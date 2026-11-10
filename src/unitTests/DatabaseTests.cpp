/*======================================================================================================================
 * unitTests/DatabaseTests.cpp is part of Brewtarget, and is copyright the following authors 2026:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#include "unitTests/DatabaseTests.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMap>
#include <QScopeGuard>
#include <QSet>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include "config.h"
#include "database/Database.h"
#include "database/DatabaseSchemaHelper.h"
#include "database/ObjectStoreWrapper.h"
#include "model/BrewLog.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeUtils.h"
#include "unitTests/TestEnvironment.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(DatabaseTests)

namespace {
   /**
    * \brief File in testData that we upgrade.  This is the default database that shipped with Brewtarget 2.3.1.
    */
   char const * const brewtarget231DbFileName = "Brewtarget_2.3.1_default_database.sqlite";

   /**
    * \brief The schema version of \c brewtarget231DbFileName.  (Its settings table says "2.1.0", which
    *        \c DatabaseSchemaHelper::schemaVersion maps to 3.)
    */
   int constexpr brewtarget231SchemaVersion = 3;

   /**
    * \brief RAII wrapper for a SQLite connection that we open just for the test.  It's important to remove the
    *        connection when we're done, but Qt only allows this once all \c QSqlDatabase and \c QSqlQuery objects
    *        using the connection have gone out of scope.  So we have to be a bit careful about object lifetimes.
    */
   class TestDbConnection {
   public:
      TestDbConnection(QString const & connectionName, QString const & fileName) :
         m_connectionName{connectionName} {
         QSqlDatabase connection = QSqlDatabase::addDatabase("QSQLITE", connectionName);
         connection.setDatabaseName(fileName);
         m_opened = connection.open();
         if (!m_opened) {
            qCritical() <<
               Q_FUNC_INFO << "Unable to open" << fileName << ":" << connection.lastError().text();
         }
         return;
      }

      ~TestDbConnection() {
         {
            QSqlDatabase connection = QSqlDatabase::database(m_connectionName, false);
            connection.close();
         }
         QSqlDatabase::removeDatabase(m_connectionName);
         return;
      }

      bool isOpen() const { return m_opened; }

      QSqlDatabase connection() const { return QSqlDatabase::database(m_connectionName, false); }

   private:
      QString const m_connectionName;
      bool m_opened = false;
   };

   /**
    * \return Map from table name to the names of its columns.  We ignore SQLite's internal tables.
    *
    *         Names are converted to lower case, because SQL identifiers are case-insensitive (unless quoted, which we
    *         don't do), and some older databases have mixed-case column names (eg recipe.carbonationTemp_c in
    *         Brewtarget 2.3.1) where we now use lower case.
    */
   QMap<QString, QSet<QString>> getSchema(QSqlDatabase const & connection) {
      QMap<QString, QSet<QString>> schema;
      for (QString const & tableName : connection.tables(QSql::Tables)) {
         if (tableName.startsWith("sqlite_")) {
            continue;
         }
         QSet<QString> columnNames;
         QSqlRecord const record = connection.record(tableName);
         for (int ii = 0; ii < record.count(); ++ii) {
            columnNames.insert(record.fieldName(ii).toLower());
         }
         schema.insert(tableName.toLower(), columnNames);
      }
      return schema;
   }

   /**
    * \return Number of rows in \c tableName, or -1 if there was an error
    */
   int countRows(QSqlDatabase const & connection, QString const & tableName) {
      QSqlQuery query{connection};
      if (!query.exec(QString{"SELECT COUNT(*) FROM %1"}.arg(tableName)) || !query.next()) {
         qCritical() << Q_FUNC_INFO << "Error counting rows in" << tableName << ":" << query.lastError().text();
         return -1;
      }
      return query.value(0).toInt();
   }
}

DatabaseTests::DatabaseTests() = default;

DatabaseTests::~DatabaseTests() = default;

void DatabaseTests::testUpgradeBrewtarget231Database() {
   QDir const & tempDir = UnitTests::TestEnvironment::tempDir();

   //
   // Make a copy of the old database to upgrade, so we don't modify the one in testData.  (QFile::copy copies the
   // file's permissions, so we make sure the copy is writable, in case the original isn't.)
   //
   QString const sourceFileName = QDir{CONFIG_TEST_DATA_DIR}.filePath(brewtarget231DbFileName);
   QString const upgradedFileName = tempDir.filePath("upgradedFromBrewtarget231.sqlite");
   QVERIFY2(QFile::exists(sourceFileName), qPrintable(QString{"Missing test data file %1"}.arg(sourceFileName)));
   QVERIFY(QFile::copy(sourceFileName, upgradedFileName));
   QVERIFY(QFile::setPermissions(upgradedFileName, QFile::ReadOwner | QFile::WriteOwner));

   //
   // Make a new database at the latest schema version, so we have something to compare the upgraded one against
   //
   QString const freshFileName = tempDir.filePath("freshlyCreated.sqlite");
   QVERIFY(Database::instance(Database::DbType::SQLite).createBlank(freshFileName));

   //
   // Note the scoping here.  The schema/row counts we extract are plain data, but anything holding a QSqlDatabase
   // needs to have gone away before the TestDbConnection destructor runs.
   //
   QMap<QString, QSet<QString>> freshSchema;
   {
      TestDbConnection const freshDb{"dbTestFresh", freshFileName};
      QVERIFY(freshDb.isOpen());
      freshSchema = getSchema(freshDb.connection());
   }
   QVERIFY(!freshSchema.isEmpty());

   TestDbConnection const upgradedDb{"dbTestUpgrade", upgradedFileName};
   QVERIFY(upgradedDb.isOpen());
   QSqlDatabase connection = upgradedDb.connection();

   QCOMPARE(DatabaseSchemaHelper::schemaVersion(connection), brewtarget231SchemaVersion);

   //
   // Remember how many of the main ingredients etc there are, so we can check we didn't lose any in the upgrade.  (The
   // table names here are the same before and after the upgrade.)
   //
   QStringList const tablesToCount{"equipment", "fermentable", "hop", "misc", "style", "water", "yeast"};
   QMap<QString, int> rowCountsBefore;
   for (QString const & tableName : tablesToCount) {
      int const numRows = countRows(connection, tableName);
      QVERIFY2(numRows > 0, qPrintable(QString{"No rows in %1 before upgrade"}.arg(tableName)));
      rowCountsBefore.insert(tableName, numRows);
   }

   //
   // This is the main thing we are testing!
   //
   QVERIFY(DatabaseSchemaHelper::migrate(Database::instance(Database::DbType::SQLite),
                                         brewtarget231SchemaVersion,
                                         DatabaseSchemaHelper::latestVersion,
                                         connection));
   QCOMPARE(DatabaseSchemaHelper::schemaVersion(connection), DatabaseSchemaHelper::latestVersion);

   //
   // Every table and column in a freshly-created database should be present in the upgraded one.  (We report all the
   // missing ones before failing, as that's more helpful than stopping at the first.)
   //
   QMap<QString, QSet<QString>> const upgradedSchema = getSchema(connection);
   QStringList missing;
   // (We don't use QMap::asKeyValueRange() as it needs Qt 6.4.)
   for (auto ii = freshSchema.cbegin(); ii != freshSchema.cend(); ++ii) {
      QString       const & tableName    = ii.key();
      QSet<QString> const & freshColumns = ii.value();
      if (!upgradedSchema.contains(tableName)) {
         missing.append(QString{"table %1"}.arg(tableName));
         continue;
      }
      QSet<QString> const & upgradedColumns = upgradedSchema.value(tableName);
      for (QString const & columnName : freshColumns) {
         if (!upgradedColumns.contains(columnName)) {
            missing.append(QString{"column %1.%2"}.arg(tableName, columnName));
         }
      }
   }
   QVERIFY2(missing.isEmpty(), qPrintable("Upgraded DB is missing " + missing.join(", ")));

   //
   // Conversely, it's not necessarily an error for the upgraded database to have extra tables or columns, but it
   // probably means a migration forgot to drop something, so we log it.
   //
   for (auto ii = upgradedSchema.cbegin(); ii != upgradedSchema.cend(); ++ii) {
      QString       const & tableName       = ii.key();
      QSet<QString> const & upgradedColumns = ii.value();
      if (!freshSchema.contains(tableName)) {
         qWarning() << Q_FUNC_INFO << "Upgraded DB has extra table" << tableName;
         continue;
      }
      for (QString const & columnName : upgradedColumns - freshSchema.value(tableName)) {
         qWarning() << Q_FUNC_INFO << "Upgraded DB has extra column" << tableName << "." << columnName;
      }
   }

   //
   // Some migrations add rows (eg where data that used to be held in one table is split out into several), so we just
   // check we didn't lose any.
   //
   for (QString const & tableName : tablesToCount) {
      int const numRowsAfter = countRows(connection, tableName);
      qInfo() <<
         Q_FUNC_INFO << "Table" << tableName << "had" << rowCountsBefore.value(tableName) << "rows before upgrade and" <<
         numRowsAfter << "after";
      QVERIFY2(numRowsAfter >= rowCountsBefore.value(tableName),
               qPrintable(QString{"Rows lost from %1 in upgrade"}.arg(tableName)));
   }

   //
   // Check that the upgrade didn't leave any dangling foreign keys.  (DatabaseSchemaHelper::migrate turns off foreign
   // key enforcement during the upgrade, so this wouldn't otherwise be caught.)
   //
   QSqlQuery foreignKeyCheck{connection};
   QVERIFY(foreignKeyCheck.exec("PRAGMA foreign_key_check"));
   QStringList foreignKeyErrors;
   while (foreignKeyCheck.next()) {
      foreignKeyErrors.append(
         QString{"%1 row %2 -> %3"}.arg(foreignKeyCheck.value(0).toString(),
                                        foreignKeyCheck.value(1).toString(),
                                        foreignKeyCheck.value(2).toString())
      );
   }
   QVERIFY2(foreignKeyErrors.isEmpty(), qPrintable("Foreign key errors: " + foreignKeyErrors.join(", ")));

   return;
}

void DatabaseTests::testLockingAndSharing() {
   // We don't want the changes below to create new versions of Recipes
   RecipeUtils::SuspendRecipeVersioning suspendRecipeVersioning;

   //
   // Set up a mash, with a couple of steps, used by two recipes
   //
   auto mash = std::make_shared<Mash>("Test Shared Mash");
   ObjectStoreWrapper::insert(mash);
   for (char const * const stepName : {"Test Sacch Rest", "Test Mash Out"}) {
      mash->add(std::make_shared<MashStep>(stepName));
   }
   QCOMPARE(mash->numSteps(), 2u);

   auto recipeA = std::make_shared<Recipe>("Test Locking Recipe A");
   auto recipeB = std::make_shared<Recipe>("Test Locking Recipe B");
   ObjectStoreWrapper::insert(recipeA);
   ObjectStoreWrapper::insert(recipeB);
   recipeA->setMash(mash);
   recipeB->setMash(mash);

   QCOMPARE(RecipeUtils::recipesUsing(*mash).size(), 2);
   QVERIFY(RecipeUtils::lockReason(*mash) == RecipeUtils::LockReason::NotLocked);
   QString description = RecipeUtils::describeSharingAndLocking(*mash, recipeA.get());
   QVERIFY2(description.contains(recipeB->name()) && !description.contains(recipeA->name()), qPrintable(description));

   //
   // Locking the mash should be stored in the database (not just in the object)
   //
   mash->setLocked(true);
   QVERIFY(RecipeUtils::lockReason(*mash) == RecipeUtils::LockReason::ItemLocked);
   {
      QSqlQuery query{Database::instance().sqlDatabase()};
      query.prepare("SELECT locked FROM mash WHERE id = ?");
      query.addBindValue(mash->key());
      QVERIFY2(query.exec() && query.next(), qPrintable(query.lastError().text()));
      QVERIFY(query.value(0).toBool());
   }

   //
   // Making a copy for recipe A should give an unlocked copy, with the same steps, which only recipe A uses
   //
   std::shared_ptr<Mash> const copy = RecipeUtils::useCopyFor<Mash>(*recipeA);
   QVERIFY(copy);
   QVERIFY(copy->key() > 0 && copy->key() != mash->key());
   QVERIFY(!copy->locked());
   QCOMPARE(copy->numSteps(), mash->numSteps());
   QVERIFY2(copy->name().contains(recipeA->name()), qPrintable(copy->name()));
   QCOMPARE(recipeA->mash()->key(), copy->key());
   QCOMPARE(RecipeUtils::recipesUsing(*copy).size(), 1);
   QCOMPARE(RecipeUtils::recipesUsing(*mash).size(), 1);
   QVERIFY(RecipeUtils::lockReason(*copy) == RecipeUtils::LockReason::NotLocked);

   //
   // A mash used by a locked recipe is, in effect, locked, even though its own flag isn't set
   //
   mash->setLocked(false);
   QVERIFY(RecipeUtils::lockReason(*mash) == RecipeUtils::LockReason::NotLocked);
   recipeB->setLocked(true);
   QVERIFY(RecipeUtils::lockReason(*mash) == RecipeUtils::LockReason::UsedByLockedRecipe);
   description = RecipeUtils::describeSharingAndLocking(*mash, nullptr);
   QVERIFY2(description.contains(recipeB->name()), qPrintable(description));
   // But it doesn't affect the copy
   QVERIFY(RecipeUtils::lockReason(*copy) == RecipeUtils::LockReason::NotLocked);

   return;
}

void DatabaseTests::testAutomaticVersioning() {
   bool const savedVersioning = RecipeUtils::getAutomaticVersioningEnabled();
   RecipeUtils::setAutomaticVersioningEnabled(true);
   auto restoreVersioning = qScopeGuard([savedVersioning]() {
      RecipeUtils::setAutomaticVersioningEnabled(savedVersioning);
   });

   //
   // A recipe with something in it that we can change.  (Changes to a Recipe's own fields don't trigger versioning --
   // see NamedEntity::owningRecipe -- so we make our changes to the Misc addition.)
   //
   auto recipe = std::make_shared<Recipe>("Test Versioning Recipe");
   ObjectStoreWrapper::insert(recipe);
   auto misc = std::make_shared<Misc>("Test Versioning Misc");
   ObjectStoreWrapper::insert(misc);
   auto addition = std::make_shared<RecipeAdditionMisc>(*recipe, *misc);
   recipe->addAddition(addition);

   //
   // Changing a recipe that has not been brewed does not create a new version
   //
   addition->setName("Test Versioning Addition - Change 1");
   QVERIFY(!recipe->hasAncestors());

   //
   // Brew the recipe, then fill in the brew log.  This does not create a new version, because the brew log is not part
   // of the recipe.
   //
   auto brewLog1 = std::make_shared<BrewLog>(*recipe);
   brewLog1->populateNote(recipe.get());
   ObjectStoreWrapper::insert(brewLog1);
   QCOMPARE(recipe->brewLogs().size(), 1);
   brewLog1->setMeasuredPreBoilGravity_sg(1.042);
   brewLog1->setNotes("Test Versioning Brew Log 1");
   QVERIFY(!recipe->hasAncestors());

   //
   // Now changing the recipe should create a new version, ie the recipe gets an ancestor (aka previous version), which
   // is locked and has the brew log.
   //
   addition->setName("Test Versioning Addition - Change 2");
   QVERIFY(recipe->hasAncestors());
   QCOMPARE(recipe->ancestors().size(), 1);
   std::shared_ptr<Recipe> const firstVersion = recipe->ancestors().first();
   QVERIFY(firstVersion->locked());
   QVERIFY(!recipe->locked());
   QVERIFY(recipe->brewLogs().isEmpty());
   QCOMPARE(firstVersion->brewLogs().size(), 1);
   QCOMPARE(firstVersion->brewLogs().first()->key(), brewLog1->key());
   QCOMPARE(RecipeUtils::brewLogsForRecipeAndAncestors(*recipe).size(), 1);
   // The snapshot has the recipe as it was before the change
   QCOMPARE(firstVersion->miscAdditions().size(), 1);
   QCOMPARE(firstVersion->miscAdditions().first()->name(), QString{"Test Versioning Addition - Change 1"});

   //
   // Further changes, before the recipe is brewed again, do not create more versions
   //
   addition->setName("Test Versioning Addition - Change 3");
   addition->setName("Test Versioning Addition - Change 4");
   QCOMPARE(recipe->ancestors().size(), 1);

   //
   // Brew it again.  As before, creating and filling in the brew log does not create a new version.
   //
   auto brewLog2 = std::make_shared<BrewLog>(*recipe);
   brewLog2->populateNote(recipe.get());
   ObjectStoreWrapper::insert(brewLog2);
   brewLog2->setNotes("Test Versioning Brew Log 2");
   QCOMPARE(recipe->ancestors().size(), 1);
   QCOMPARE(recipe->brewLogs().size(), 1);

   //
   // The next change creates a second version, which gets the second brew log
   //
   addition->setName("Test Versioning Addition - Change 5");
   QCOMPARE(recipe->ancestors().size(), 2);
   std::shared_ptr<Recipe> const secondVersion = recipe->ancestors().first();
   QVERIFY(secondVersion->key() != firstVersion->key());
   QVERIFY(recipe->brewLogs().isEmpty());
   QCOMPARE(secondVersion->brewLogs().size(), 1);
   QCOMPARE(secondVersion->brewLogs().first()->key(), brewLog2->key());
   QCOMPARE(firstVersion->brewLogs().size(), 1);
   QCOMPARE(RecipeUtils::brewLogsForRecipeAndAncestors(*recipe).size(), 2);

   return;
}
