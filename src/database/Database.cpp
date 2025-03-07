/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/Database.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Chris Pavetto <chrispavetto@gmail.com>
 *   • Chris Speck <cgspeck@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • David Grundberg <individ@acc.umu.se>
 *   • Greg Greenaae <ggreenaae@gmail.com>
 *   • Jamie Daws <jdelectronics1@gmail.com>
 *   • Jean-Baptiste Wons <wonsjb@gmail.com>
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
 *   • Luke Vincent <luke.r.vincent@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "database/Database.h"

#include <filesystem>
#include <iostream> // For writing to std::cerr in destructor
#include <mutex>    // For std::once_flag etc

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QString>
#include <QThread>

#include "Application.h"
#include "config.h"
#include "database/BtSqlQuery.h"
#include "database/DefaultContentLoader.h"
#include "database/DatabaseSchemaHelper.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"
#include "utils/EnumStringMapping.h"
#include "utils/ErrorCodeToStream.h"

namespace {
   EnumStringMapping const dbTypeToName {
      {Database::DbType::NODB  , Database::tr("NODB"  )},
      {Database::DbType::SQLITE, Database::tr("SQLITE")},
      {Database::DbType::PGSQL , Database::tr("PGSQL" )},
      {Database::DbType::ALLDB , Database::tr("ALLDB" )},
   };

   //
   // Constants for DB native type names etc
   //
   struct DbNativeVariants {
      char const * const sqliteName;
      char const * const postgresqlName;
      // GCC will let you get away without it, but some C++ compilers are more strict about the need for a non-default
      // constructor when you have const members in a struct
      DbNativeVariants(char const * const sqliteName = nullptr, char const * const postgresqlName = nullptr) :
         sqliteName{sqliteName},
         postgresqlName{postgresqlName} {
         return;
      }
   };

   DbNativeVariants const displayableDbType {
      "SQLite",
      "PostgreSQL"
   };

   //
   // SQLite actually lets you store any type in any column, and only offers five "affinities" for "the recommended
   // type for data stored in a column".   There are no special types for boolean or date.  However, it also allows you
   // to use traditional SQL type names in create table statements etc (and does the mapping down to the affinities
   // under the hood) and retains those names when you're browsing the database.   We therefore use those traditional
   // SQL typenames here as it makes the intent clearer for anyone looking directly at the database.
   //
   // NOTE HOWEVER, using BOOLEAN as a column type on a table in SQLite precludes us from creating that table as a
   // "STRICT Table" (see https://www.sqlite.org/stricttables.html).
   //
   // PostgreSQL is the other extreme and has all sorts of specialised types (including for networking addresses,
   // geometric shapes and XML).  We need only a small subset of these.
   //
   template<typename T> DbNativeVariants const nativeTypeNames;
   //                                                                SQLite    PostgreSQL
   template<> DbNativeVariants const nativeTypeNames<bool>         {"BOOLEAN", "BOOLEAN"         };
   template<> DbNativeVariants const nativeTypeNames<int>          {"INTEGER", "INTEGER"         };
   template<> DbNativeVariants const nativeTypeNames<unsigned int> {"INTEGER", "INTEGER"         };
   template<> DbNativeVariants const nativeTypeNames<double>       {"DOUBLE",  "DOUBLE PRECISION"};
   template<> DbNativeVariants const nativeTypeNames<QString>      {"TEXT",    "TEXT"            };
   template<> DbNativeVariants const nativeTypeNames<QDate>        {"DATE",    "DATE"            };

   // Note that, per https://www.sqlite.org/autoinc.html, SQLite explicitly recommends against using AUTOINCREMENT for
   // integer primary keys, as specifying "PRIMARY KEY" alone will result in automatic generation of primary keys with
   // less overhead.
   DbNativeVariants const nativeIntPrimaryKeyDeclaration {
      "INTEGER PRIMARY KEY", // SQLite
      "SERIAL PRIMARY KEY"   // PostgreSQL
   };

   DbNativeVariants const sqlToAddColumnAsForeignKey {
      "ALTER TABLE %1 ADD COLUMN %2 INTEGER REFERENCES %3(%4);", // SQLite
      "ALTER TABLE %1 ADD COLUMN %2 INTEGER REFERENCES %3(%4);"  // PostgreSQL
      // MySQL would be ALTER TABLE %1 ADD COLUMN %2 int, FOREIGN KEY (%2) REFERENCES %3(%4)
   };

   char const * getDbNativeName(DbNativeVariants const & dbNativeVariants, Database::DbType dbType) {
      switch (dbType) {
         case Database::DbType::SQLITE: return dbNativeVariants.sqliteName;
         case Database::DbType::PGSQL:  return dbNativeVariants.postgresqlName;
         default:
            // It's a coding error if we get here
            qCritical() << Q_FUNC_INFO << "Unrecognised DB type:" << dbType;
            Q_ASSERT(false);
            break;
      }
      return "NotSupported";
   }

   //
   // Each thread has its own connection to the database, and each connection has to have a unique name (otherwise,
   // calling QSqlDatabase::addDatabase() with the same name as an existing connection will replace that existing
   // connection with the new one created by that function).  We can create a unique connection name from the thread
   // ID in a similar way as we do in the Logging module.  The difference is that we need each _instance_ of Database to
   // have a separate connection name for each _thread_ because, eg, when you're switching between SQLite and
   // PostgreSQL, a single thread will have two separate connections open (one to current and one to new DB).
   //
   // We only need to store the name of the connection here.  (See header file comment for Database::sqlDatabase() for
   // more details of why it would be unhelpful to store a QSqlDatabase object in thread-local storage.)
   //
   // Since C++11, we can use thread_local to define thread-specific variables that are initialized "before first use"
   //
   thread_local QMap<Database::DbType, QString> const dbConnectionNamesForThisThread {
      {Database::DbType::SQLITE, QString{"%1-%2"}.arg(getDbNativeName(displayableDbType, Database::DbType::SQLITE)).arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 36)},
      {Database::DbType::PGSQL,  QString{"%1-%2"}.arg(getDbNativeName(displayableDbType, Database::DbType::PGSQL)).arg(reinterpret_cast<quintptr>(QThread::currentThreadId()), 0, 36)}
   };

   //
   // At start-up, we know what type of database to talk to (and thus what type of Database object to return from
   // Database::instance()) by looking in PersistentSettings (and defaulting to SQLite if nothing is marked there).  But
   // we need to remember this value and not keep looking in PersistentSettings because those settings can change (if
   // the user wants to switch to another database) and we don't want other bits of the program to suddenly get a
   // different object back from Database::instance() before we're ready for it.  Hence this variable.
   //
   Database::DbType currentDbType = Database::DbType::NODB;

   // May St. Stevens intercede on my behalf.
   //
   //! \brief opens an SQLite db for transfer
   QSqlDatabase openSQLite(QString filePath) {
      QSqlDatabase newConnection = QSqlDatabase::addDatabase("QSQLITE", "altdb");

      try {
///         dbFile.setFileName(dbFileName);

         if (filePath.isEmpty()) {
            throw QString("Could not read the database file (%1)").arg(filePath);
         }

         newConnection.setDatabaseName(filePath);

         if (!newConnection.open()) {
            throw QString("Could not open %1 : %2").arg(filePath).arg(newConnection.lastError().text());
         }
      } catch (QString e) {
         qCritical() << Q_FUNC_INFO << e;
         throw;
      }

      return newConnection;
   }

   //! \brief opens a PostgreSQL db for transfer
   QSqlDatabase openPostgres(QString const& Hostname, QString const& DbName,
                             QString const& Username, QString const& Password,
                             int Portnum) {
      QSqlDatabase newConnection = QSqlDatabase::addDatabase("QPSQL", "altdb");

      try {
         newConnection.setHostName(Hostname);
         newConnection.setDatabaseName(DbName);
         newConnection.setUserName(Username);
         newConnection.setPort(Portnum);
         newConnection.setPassword(Password);

         if (!newConnection.open()) {
            throw QString("Could not open %1 : %2").arg(Hostname).arg(newConnection.lastError().text());
         }
      } catch (QString e) {
         qCritical() << Q_FUNC_INFO << e;
         throw;
      }
      return newConnection;
   }

}

//
// This private implementation class holds all private non-virtual members of Database
//
class Database::impl {
public:

   /**
    * Constructor
    */
   impl(Database::DbType dbType) : dbType{dbType},
                                   dbConName{},
                                   loaded{false},
                                   loadWasSuccessful{false},
                                   mutex{},
                                   userDatabaseDidNotExist{false} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // Don't know where to put this, so it goes here for right now
   bool loadSQLite(Database & database) {
      qDebug() << "Loading SQLITE...";

      // Set file names.
      this->dbFileName = PersistentSettings::getUserDataDir().filePath("database.sqlite");
///      this->dataDbFileName = Application::getResourceDir().filePath("default_db.sqlite");
///      qInfo().noquote() <<
///         Q_FUNC_INFO << "dbFileName = \"" << this->dbFileName << "\"\ndataDbFileName=\"" << this->dataDbFileName << "\"";
      qInfo().noquote() << Q_FUNC_INFO << "dbFileName =" << this->dbFileName;
      // Set the files.
      this->dbFile.setFileName(this->dbFileName);
///      this->dataDbFile.setFileName(this->dataDbFileName);

      // If user restored the database from a backup, make the backup into the primary.
      {
         QFile newdb(QString("%1.new").arg(this->dbFileName));
         if (newdb.exists()) {
            this->dbFile.remove();
            newdb.copy(this->dbFileName);
            QFile::setPermissions(this->dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );
            newdb.remove();
         }
      }

///      // If there's no dbFile, try to copy from dataDbFile.
///      if (!this->dbFile.exists()) {
///         userDatabaseDidNotExist = true;
///
///         // Have to wait until db is open before creating from scratch.
///         if (this->dataDbFile.exists()) {
///            this->dataDbFile.copy(this->dbFileName);
///            QFile::setPermissions(this->dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup);
///         }
///      }

      // Open SQLite DB
      // It's a coding error if we didn't already establish that SQLite is the type of DB we're talking to, so assert
      // that and then call the generic code to get a connection
      Q_ASSERT(this->dbType == Database::DbType::SQLITE);
      QSqlDatabase connection = database.sqlDatabase();

      this->dbConName = connection.connectionName();
      qDebug() << Q_FUNC_INFO << "dbConName=" << this->dbConName;

      //
      // It's quite useful to record the DB version in the logs
      //
      BtSqlQuery sqlQuery(connection);
      QString queryString{"SELECT sqlite_version() AS version;"};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec() || !sqlQuery.next()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }
      QVariant fieldValue = sqlQuery.value("version");
      qInfo() << Q_FUNC_INFO << "SQLite version" << fieldValue;

      // NOTE: synchronous=off reduces query time by an order of magnitude!
      BtSqlQuery pragma(connection);
      if ( ! pragma.exec( "PRAGMA synchronous = off" ) ) {
         qCritical() << Q_FUNC_INFO << "Could not disable synchronous writes: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec( "PRAGMA foreign_keys = on")) {
         qCritical() << Q_FUNC_INFO << "Could not enable foreign keys: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec( "PRAGMA locking_mode = EXCLUSIVE")) {
         qCritical() << Q_FUNC_INFO << "Could not enable exclusive locks: " << pragma.lastError().text();
         return false;
      }
      if ( ! pragma.exec("PRAGMA temp_store = MEMORY") ) {
         qCritical() << Q_FUNC_INFO << "Could not enable temporary memory: " << pragma.lastError().text();
         return false;
      }

      // older sqlite databases may not have a settings table. I think I will
      // just check to see if anything is in there.
      this->createFromScratch = connection.tables().size() == 0;

      return true;
   }

   bool loadPgSQL(Database & database) {

      this->dbHostname = PersistentSettings::value(PersistentSettings::Names::dbHostname).toString();
      this->dbPortnum  = PersistentSettings::value(PersistentSettings::Names::dbPortnum).toInt();
      this->dbName     = PersistentSettings::value(PersistentSettings::Names::dbName).toString();
      this->dbSchema   = PersistentSettings::value(PersistentSettings::Names::dbSchema).toString();

      this->dbUsername = PersistentSettings::value(PersistentSettings::Names::dbUsername).toString();

      if (PersistentSettings::contains(PersistentSettings::Names::dbPassword)) {
         this->dbPassword = PersistentSettings::value(PersistentSettings::Names::dbPassword).toString();
      } else {
         bool isOk = false;

         // prompt for the password until we get it? I don't think this is a good
         // idea?
         while (!isOk) {
            this->dbPassword = QInputDialog::getText(nullptr,
                                                     tr("Database password"),
                                                     tr("Password"),
                                                     QLineEdit::Password,
                                                     QString(),
                                                     &isOk);
            if (isOk) {
               isOk = verifyDbConnection(Database::DbType::PGSQL,
                                         this->dbHostname,
                                         this->dbPortnum,
                                         this->dbSchema,
                                         this->dbName,
                                         this->dbUsername,
                                         this->dbPassword);
            }
         }
      }

      // It's a coding error if we didn't already establish that PostgreSQL is the type of DB we're talking to, so
      // assert that and then call the generic code to get a connection
      Q_ASSERT(this->dbType == Database::DbType::PGSQL);
      QSqlDatabase connection = database.sqlDatabase();

      this->dbConName = connection.connectionName();
      qDebug() << Q_FUNC_INFO << "dbConName=" << this->dbConName;

      //
      // It's quite useful to record the DB version in the logs
      //
      BtSqlQuery sqlQuery(connection);
      QString queryString{"SELECT version() AS version;"};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec() || !sqlQuery.next()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }
      QVariant fieldValue = sqlQuery.value("version");
      qInfo() << Q_FUNC_INFO << "PostgreSQL version" << fieldValue;

      // by the time we had pgsql support, there is a settings table
      this->createFromScratch = ! connection.tables().contains("settings");

      return true;
   }

   // Returns true if the schema gets updated, false otherwise.
   // If err != 0, set it to true if an error occurs, false otherwise.
   bool updateSchema(Database & database, bool* err = nullptr) {
      auto connection = database.sqlDatabase();
      int dbSchemaVersion = DatabaseSchemaHelper::schemaVersion(connection);
      int latestSchemaVersion = DatabaseSchemaHelper::latestVersion;
      qInfo() <<
         Q_FUNC_INFO << "Schema version in DB:" << dbSchemaVersion << ", current schema version in code:" << latestSchemaVersion;

      bool doUpdate = dbSchemaVersion < latestSchemaVersion;
      if (doUpdate) {
         //
         // Before we do a DB upgrade, we should back-up the DB (if we can).
         //
         // If we're in interactive mode (rather than, eg, running unit tests), we should tell the user, including
         // giving them a chance to abort.
         //
         QString backupDir = PersistentSettings::value(
            PersistentSettings::Names::directory,
            PersistentSettings::getUserDataDir().canonicalPath(),
            PersistentSettings::Sections::backups
         ).toString();
         //
         // It's probably enough for most users to put the date on the backup file name to make it unique.  But we put
         // the time too just in case.  Note that, even though it is done in the ISO 8601 standard, we cannot format the
         // time with colons (eg as hh:mm:ss) because Windows does not accept colons in filenames.  We could use the
         // ratio symbol (∶) which looks almost the same.  There is a precendent for doing this:
         // https://web.archive.org/web/20190108033419/https://blogs.msdn.microsoft.com/oldnewthing/20180913-00/?p=99725
         // However, at least on KDE desktop, this looks a bit odd in filenames as the character is padded with a lot of
         // space.  So, instead, we use the raised colon (˸), which gives tighter spacing in proportional fonts.
         //
         // NOTE: We do not currently check whether the file we are creating already exists...
         //
         QString backupName = QString(
            "%1 database.sqlite backup (before upgrade from v%2 to v%3)"
         ).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh˸mm˸ss")).arg(dbSchemaVersion).arg(latestSchemaVersion);
         bool succeeded = database.backupToDir(backupDir, backupName);
         if (!succeeded) {
            qCritical() << Q_FUNC_INFO << "Unable to create DB backup";
            if (Application::isInteractive()) {
               QMessageBox upgradeBackupFailedMessageBox;
               upgradeBackupFailedMessageBox.setIcon(QMessageBox::Icon::Critical);
               upgradeBackupFailedMessageBox.setWindowTitle(tr("Unable to back up database before upgrading"));
               upgradeBackupFailedMessageBox.setText(
                  tr("Could not backup database prior to required upgrade.  See logs for more details.")
               );
               upgradeBackupFailedMessageBox.exec();
            }
            exit(1);
         }

         if (Application::isInteractive()) {
            QMessageBox dbUpgradeMessageBox;
            dbUpgradeMessageBox.setWindowTitle(tr("Software Upgraded"));
            dbUpgradeMessageBox.setText(
               tr("Before continuing, %1 %2 needs to upgrade your database schema "
                  "(from v%3 to v%4).\n").arg(CONFIG_APPLICATION_NAME_UC).arg(CONFIG_VERSION_STRING).arg(dbSchemaVersion).arg(latestSchemaVersion)
            );
            dbUpgradeMessageBox.setInformativeText(
               tr("DON'T PANIC: Your existing data will be retained!")
            );
            if (this->dbType == Database::DbType::PGSQL) {
               dbUpgradeMessageBox.setDetailedText(
                  tr("The upgrade should retain all your existing data.\n\nEven so, it's a good idea to make a manual "
                     "backup of your PostgreSQL database just in case.\n\nIf you didn't yet do this, click Abort.")
               );
            } else {
               dbUpgradeMessageBox.setDetailedText(
                  tr("Pre-upgrade database backup is in:\n%1").arg(backupDir)
               );
            }
            dbUpgradeMessageBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Abort);
            dbUpgradeMessageBox.setDefaultButton(QMessageBox::Ok);
            int ret = dbUpgradeMessageBox.exec();
            if (ret == QMessageBox::Abort) {
               qDebug() << Q_FUNC_INFO << "User clicked \"Abort\".  Exiting.";
               // Ask the application nicely to quit
               QCoreApplication::quit();
               // If it didn't, we have to insist!
               QCoreApplication::exit(1);
               // If insisting doesn't work, there's one final option
               exit(1);
            }
         }

         bool success = DatabaseSchemaHelper::migrate(database, dbSchemaVersion, latestSchemaVersion, database.sqlDatabase() );
         if (!success) {
            qCritical() << Q_FUNC_INFO << QString("Database migration %1->%2 failed").arg(dbSchemaVersion).arg(latestSchemaVersion);
            if (err) {
               *err = true;
            }
            return false;
         }
      }

      return doUpdate;
   }

   void automaticBackup(Database & database) {
      int count = PersistentSettings::value(PersistentSettings::Names::count, 0, PersistentSettings::Sections::backups).toInt() + 1;
      int frequency = PersistentSettings::value(PersistentSettings::Names::frequency, 4, PersistentSettings::Sections::backups).toInt();
      int maxBackups = PersistentSettings::value(PersistentSettings::Names::maximum, 10, PersistentSettings::Sections::backups).toInt();

      // The most common case is update the counter and nothing else
      // A frequency of 1 means backup every time. Which this statisfies
      if ( count % frequency != 0 ) {
         PersistentSettings::insert(PersistentSettings::Names::count, count, PersistentSettings::Sections::backups);
         return;
      }

      // If the user has selected 0 max backups, we just return. There's a weird
      // case where they have a frequency of 1 and a maxBackup of 0. In that
      // case, maxBackup wins
      if ( maxBackups == 0 ) {
         return;
      }

      QString backupDir = PersistentSettings::value(PersistentSettings::Names::directory, PersistentSettings::getUserDataDir().canonicalPath(), PersistentSettings::Sections::backups).toString();
      QString listOfFiles = PersistentSettings::value(PersistentSettings::Names::files, QVariant(), PersistentSettings::Sections::backups).toString();
      QStringList fileNames = listOfFiles.split(",", Qt::SkipEmptyParts);

      QString halfName = QString("%1.%2").arg("databaseBackup").arg(QDate::currentDate().toString("yyyyMMdd"));
      QString newName = halfName;
      // Unique filenames are a pain in the ass. In the case you open the application twice in a day, this loop makes
      // sure we don't over write (or delete) the wrong thing.
      int foobar = 0;
      while ( foobar < 10000 && QFile::exists( backupDir + "/" + newName ) ) {
         foobar++;
         newName = QString("%1_%2").arg(halfName).arg(foobar,4,10,QChar('0'));
         if ( foobar > 9999 ) {
            qWarning() << QString("%1 : could not find a unique name in 10000 tries. Overwriting %2").arg(Q_FUNC_INFO).arg(halfName);
            newName = halfName;
         }
      }
      // backup the file first
      database.backupToDir(backupDir, newName);

      // If we have maxBackups == -1, it means never clean. It also means we
      // don't track the filenames.
      if ( maxBackups == -1 )  {
         PersistentSettings::remove(PersistentSettings::Names::files, PersistentSettings::Sections::backups);
         return;
      }

      fileNames.append(newName);

      // If we have too many backups. This is in a while loop because we need to
      // handle the case where a user decides they only want 4 backups, not 10.
      // The while loop will clean that up properly.
      while ( fileNames.size() > maxBackups ) {
         // takeFirst() removes the file from the list, which is important
         QString victim = backupDir + "/" + fileNames.takeFirst();
         QFile *file = new QFile(victim);
         QFileInfo *fileThing = new QFileInfo(victim);

         // Make sure it exists, and make sure it is a file before we
         // try remove it
         if ( fileThing->exists() && fileThing->isFile() ) {
            qInfo() <<
               Q_FUNC_INFO << "Removing oldest database backup file," << victim << "as more than" << maxBackups <<
               "files in" << backupDir;
            // If we can't remove it, give a warning.
            if (! file->remove() ) {
               qWarning() <<
                  Q_FUNC_INFO << "Could not remove old database backup file " << victim << ".  Error:" << file->error();
            }
         }
      }

      // re-encode the list
      listOfFiles = fileNames.join(",");

      // finally, reset the counter and save the new list of files
      PersistentSettings::insert(PersistentSettings::Names::count, 0, PersistentSettings::Sections::backups);
      PersistentSettings::insert(PersistentSettings::Names::files, listOfFiles, PersistentSettings::Sections::backups);

      return;
   }

   //============================================== impl member variables ==============================================

   Database::DbType dbType;
   QString dbConName;

   bool loaded;

   // Instance variables.
   bool loadWasSuccessful;
   bool createFromScratch;
   bool schemaUpdated;

   // Used for locking member functions that must be single-threaded
   QMutex mutex;

   bool userDatabaseDidNotExist;


   // These are for SQLite databases
   QFile dbFile;
   QString dbFileName;
///   QFile dataDbFile;
///   QString dataDbFileName;

   // And these are for Postgres databases
   QString dbHostname;
   int dbPortnum;
   QString dbName;
   QString dbSchema;
   QString dbUsername;
   QString dbPassword;
};


Database::Database(Database::DbType dbType) : pimpl{std::make_unique<impl>(dbType)} {
   return;
}

Database::~Database() {
   // Don't try and log in this function as it's called pretty close to the program exiting, at the end of main(), at
   // which point the objects used by the logging module may be in a weird state.

   // Similarly, trying to close DB connections etc here can be tricky as some bits of Qt may already have terminated.
   // It's therefore safer, albeit less elegant to rely on main() or similar to call unload() rather than try to do it
   // here.
   if (this->pimpl->loaded) {
      std::cerr <<
         "Warning: Destructor on Database object object for " <<
         getDbNativeName(displayableDbType, this->pimpl->dbType) << " called before unload()";
   }

   return;
}

QSqlDatabase Database::sqlDatabase() const {
   // Need a unique database connection for each thread.
   //http://www.linuxjournal.com/article/9602

   //
   // If we already created a valid DB connection for this thread, this call will get it, and we can just return it to
   // the caller.  Otherwise, we'll just get an invalid connection.
   //
   Q_ASSERT(this->pimpl->dbType != Database::DbType::NODB);
   Q_ASSERT(dbConnectionNamesForThisThread.contains(this->pimpl->dbType));
   QString connectionName = dbConnectionNamesForThisThread.value(this->pimpl->dbType);
   Q_ASSERT(!connectionName.isEmpty());
   QSqlDatabase connection = QSqlDatabase::database(connectionName);
   if (connection.isValid()) {
      qDebug() << Q_FUNC_INFO << "Returning connection " << connectionName;
      return connection;
   }

   //
   // Create a new connection in Qt's register of connections.  (NB: The call to QSqlDatabase::addDatabase() is thread-
   // safe, so we don't need to worry about mutexes here.)
   //
   QString driverType{this->pimpl->dbType == Database::DbType::PGSQL ? "QPSQL" : "QSQLITE"};
   qDebug() <<
      Q_FUNC_INFO << "Creating connection " << connectionName << " with " << driverType << " driver";
   connection = QSqlDatabase::addDatabase(driverType, connectionName);
   if (!connection.isValid()) {
      //
      // If the connection is not valid, it means the specified driver type is not available or could not be loaded
      // Log an error here in the knowledge that we'll also throw an exception below
      //
      qCritical() << Q_FUNC_INFO << "Unable to load " << driverType << " database driver";
   }

   qDebug() << Q_FUNC_INFO << "Created connection of type" << connection.driver()->handle().typeName();

   //
   // Initialisation parameters depend on the DB type
   //
   if (this->pimpl->dbType == Database::DbType::PGSQL) {
      connection.setHostName    (this->pimpl->dbHostname);
      connection.setDatabaseName(this->pimpl->dbName);
      connection.setUserName    (this->pimpl->dbUsername);
      connection.setPort        (this->pimpl->dbPortnum);
      connection.setPassword    (this->pimpl->dbPassword);
   } else {
      connection.setDatabaseName(this->pimpl->dbFileName);
   }

   //
   // The moment of truth is when we try to open the new connection
   //
   if (!connection.open()) {
      QString errorMessage;
      if (this->pimpl->dbType == Database::DbType::PGSQL) {
         errorMessage = QString{
            QObject::tr("Could not open PostgreSQL DB connection to %1.\n%2")
         }.arg(this->pimpl->dbHostname).arg(connection.lastError().text());
      } else {
         errorMessage = QString{
            QObject::tr("Could not open SQLite DB file %1.\n%2")
         }.arg(this->pimpl->dbFileName).arg(connection.lastError().text());
      }
      qCritical() << Q_FUNC_INFO << errorMessage;

      if (Application::isInteractive()) {
         QMessageBox::critical(nullptr,
                               QObject::tr("Database Failure"),
                               errorMessage);
      }

      // If we can't talk to the DB, there's not much we can do to recover
      throw errorMessage;
   }

   return connection;
}

bool Database::load() {
   this->pimpl->createFromScratch = false;
   this->pimpl->schemaUpdated = false;
   this->pimpl->loadWasSuccessful = false;

   // We have had problems on Windows with the DB driver not being found in certain circumstances.  This is some extra
   // diagnostic to help resolve that.
   qInfo() << Q_FUNC_INFO << "Known DB drivers: " << QSqlDatabase::drivers();

   bool dbIsOpen;
   if (this->dbType() == Database::DbType::PGSQL ) {
      dbIsOpen = this->pimpl->loadPgSQL(*this);
   } else {
      dbIsOpen = this->pimpl->loadSQLite(*this);
   }

   if (!dbIsOpen) {
      return false;
   }

   this->pimpl->loaded = true;

   QSqlDatabase sqldb = this->sqlDatabase();

   // This should work regardless of the db being used.
   if (this->pimpl->createFromScratch) {
      if (!DatabaseSchemaHelper::create(*this, sqldb)) {
         qCritical() << Q_FUNC_INFO << "DatabaseSchemaHelper::create() failed";
         return false;
      }
   }

   // Update the database if need be. This has to happen before we do anything
   // else or we dump core
   bool schemaErr = false;
   this->pimpl->schemaUpdated = this->pimpl->updateSchema(*this, &schemaErr);

   if (schemaErr ) {
      if (Application::isInteractive()) {
         QMessageBox::critical(
            nullptr,
            QObject::tr("Database Failure"),
            QObject::tr("Failed to update the database.\n\nSee log file for details.\n\nProgram will now exit.")
         );
      }
      return false;
   }

   this->pimpl->loadWasSuccessful = true;
   return this->pimpl->loadWasSuccessful;
}

void Database::checkForNewDefaultData() {
   // See if there are new ingredients that we need to merge from the data-space db.
///   // Don't do this if we JUST copied the default database.
///   qDebug() <<
///      Q_FUNC_INFO << "dataDbFile:" << this->pimpl->dataDbFile.fileName() << ", dbFile:" <<
///      this->pimpl->dbFile.fileName() << ", userDatabaseDidNotExist: " <<
///      (this->pimpl->userDatabaseDidNotExist ? "True" : "False") << ", dataDbFile.lastModified:" <<
///      QFileInfo(this->pimpl->dataDbFile).lastModified();
   qDebug() <<
      Q_FUNC_INFO << "dbFile:" << this->pimpl->dbFile.fileName() << ", userDatabaseDidNotExist: " <<
      (this->pimpl->userDatabaseDidNotExist ? "True" : "False");
///   if (this->pimpl->dataDbFile.fileName() != this->pimpl->dbFile.fileName() &&
///       !this->pimpl->userDatabaseDidNotExist) {
   if (!this->pimpl->userDatabaseDidNotExist) {
      auto connection = this->sqlDatabase();
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      auto result = DefaultContentLoader::updateContentIfNecessary(connection, userMessageAsStream);
      if (result != DefaultContentLoader::UpdateResult::NothingToDo) {
         bool succeeded = (
            result == DefaultContentLoader::UpdateResult::Succeeded
         );
         QString messageBoxTitle{succeeded ? tr("Success!") : tr("ERROR")};
         QString messageBoxText;
         if (succeeded) {
            // The userMessage parameter will tell how many files were imported/exported and/or skipped (as duplicates)
            // Do separate messages for import and export as it makes translations easier
            messageBoxText = QString(
               tr("Successfully read new default data\n\n%1").arg(userMessage)
            );
         } else {
            messageBoxText = QString(
               tr("Unable to import some or all of new default data\n\n"
                  "%1\n\n"
                  "Log file may contain more details.").arg(userMessage)
            );
            qCritical() << Q_FUNC_INFO << userMessage;
         }
         qDebug() << Q_FUNC_INFO << "Message box text : " << messageBoxText;
         QMessageBox msgBox{succeeded ? QMessageBox::Information : QMessageBox::Critical,
                           messageBoxTitle,
                           messageBoxText};
         msgBox.exec();
      }


   }

   return;
}

bool Database::createBlank(QString const& filename) {
   {
      QSqlDatabase sqldb = QSqlDatabase::addDatabase("QSQLITE", "blank");
      sqldb.setDatabaseName(filename);
      bool dbIsOpen = sqldb.open();
      if (! dbIsOpen )
      {
         qWarning() << QString("Database::createBlank(): could not open '%1'").arg(filename);
         return false;
      }

      DatabaseSchemaHelper::create(Database::instance(Database::DbType::SQLITE), sqldb);

      sqldb.close();
   } // sqldb gets destroyed as it goes out of scope before removeDatabase()

   QSqlDatabase::removeDatabase( "blank" );
   return true;
}

bool Database::copyDataFiles(const QDir newPath) {
   QString dbFileName = "database.sqlite";
   return QFile::copy(PersistentSettings::getUserDataDir().filePath(dbFileName), newPath.filePath(dbFileName));
}


bool Database::loadSuccessful() {
   return this->pimpl->loadWasSuccessful;
}

void Database::unload() {

   // We really don't want this function to be called twice on the same object or when we didn't get as far as making a
   // connection to the DB etc.
   if (!this->pimpl->loaded) {
      qDebug() <<
         Q_FUNC_INFO << "Nothing to do for Database object for" <<
         getDbNativeName(displayableDbType, this->pimpl->dbType) << "as not loaded";
      return;
   }

   // This RAII wrapper does all the hard work on mutex.lock() and mutex.unlock() in an exception-safe way
   QMutexLocker locker(&this->pimpl->mutex);

   // We only want to close connections that relate to this instance of Database
   QString ourConnectionPrefix = QString{"%1-"}.arg(getDbNativeName(displayableDbType, this->pimpl->dbType));

   // So far, it seems we only create one connection to the db per database type, so this is likely overkill
   QStringList allConnectionNames{QSqlDatabase::connectionNames()};
   for (QString conName : allConnectionNames) {
      if (0 == conName.indexOf(ourConnectionPrefix)) {
         qDebug() << Q_FUNC_INFO << "Closing connection " << conName;
         {
            //
            // Extra braces here are to ensure that this QSqlDatabase object is out of scope before the call to
            // QSqlDatabase::removeDatabase() below
            //
            QSqlDatabase connectionToClose = QSqlDatabase::database(conName, false);
            if (connectionToClose.isOpen()) {
               connectionToClose.rollback();
               connectionToClose.close();
            }
         }
         QSqlDatabase::removeDatabase(conName);
      } else {
         qDebug() <<
            Q_FUNC_INFO << "Ignoring connection" << conName << "as does not start with" << ourConnectionPrefix;
      }
   }

   qDebug() << Q_FUNC_INFO << "DB connections all closed";

   if (this->pimpl->loadWasSuccessful && this->dbType() == Database::DbType::SQLITE ) {
      this->pimpl->dbFile.close();
      this->pimpl->automaticBackup(*this);
   }

   this->pimpl->loaded = false;
   this->pimpl->loadWasSuccessful = false;

   qDebug() << Q_FUNC_INFO << "Drop Instance done";

   return;
}

Database& Database::instance(Database::DbType dbType) {
   //
   // For the moment, with only two types of database supported, we don't do anything too sophisticated here, but we
   // should probably change that if we end up supporting more.
   //
   if (Database::DbType::NODB == dbType) {
      // The first time we are asked for the default type of Database, we look in PersistentSettings.  We then want to
      // remember that value for future requests in case PersistentSettings changes (see comment at definition of
      // currentDbType).
      if (Database::DbType::NODB == currentDbType) {
         currentDbType = static_cast<Database::DbType>(
            PersistentSettings::value(PersistentSettings::Names::dbType,
                                      static_cast<int>(Database::DbType::SQLITE)).toInt()
         );
      }
      dbType = currentDbType;
   }

   //
   // As of C++11, simple "Meyers singleton" is now thread-safe -- see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime
   //
   static Database dbSingleton_SQLite{Database::DbType::SQLITE}, dbSingleton_PostgresSQL{Database::DbType::PGSQL};

   //
   // And C++11 also provides a thread-safe way to ensure a function is called exactly once
   //
   // (See http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf for why user-implemented efforts to do this via
   // double-checked locking often come unstuck in the face of compiler optimisations, especially on multi-processor
   // platforms, back in the days when the C++ language had "no notion of threading (or any other form of concurrency)".
   //
   static std::once_flag initFlag_SQLite, initFlag_PostgresSQL;

   if (dbType == Database::DbType::SQLITE) {
      std::call_once(initFlag_SQLite, &Database::load, &dbSingleton_SQLite);
      return dbSingleton_SQLite;
   }

   std::call_once(initFlag_PostgresSQL, &Database::load, &dbSingleton_PostgresSQL);
   return dbSingleton_PostgresSQL;
}

char const * Database::getDefaultBackupFileName() {
    return "database.sqlite";
}

bool Database::backupToFile(QString const & newDbFileName) {
   QString const curDbFileName = this->pimpl->dbFile.fileName();

   qDebug() << Q_FUNC_INFO << "Database backup from" << curDbFileName << "to" << newDbFileName;

   //
   // In earlier versions of the code, we just used the copy() member function of QFile.  When this works it is fine,
   // but when there is an error, the diagnostics are not always very helpful.  Eg getting QFileDevice::CopyError back
   // from the error() member function doesn't really tell us why the file could not be copied.
   //
   // Using the Filesystem library from the C++ standard library gives us better (albeit not perfect) diagnostics when
   // things go wrong.
   //
   // The std::filesystem functions typically come in two versions: one using an error code and one throwing an
   // exception.  Since we're going to abort on the first error, the exception-throwing versions make the code a bit
   // simpler.  The `operation` variable keeps track of what we were doing when the exception occurred so we can give a
   // clue about what caused the error.
   //
   this->pimpl->dbFile.close();
   char const * operation = "Allocate";
   try {
      std::filesystem::path source{curDbFileName.toStdString()};
      std::filesystem::path target{newDbFileName.toStdString()};
      // Note that std::filesystem::canonical needs its parameter to exist, whereas std::filesystem::weakly_canonical
      // does not.
      source = std::filesystem::canonical(source);
      target = std::filesystem::weakly_canonical(target);
      operation = "See if target already exists";
      if (std::filesystem::exists(target)) {
         // AFAICT std::filesystem::copy should overwrite its target if it exists, but it's helpful for diagnostics to
         // pull that case out as a separate step.
         operation = "Remove existing target";
         qInfo() <<
            Q_FUNC_INFO << "Removing existing file" << newDbFileName << "before copying" << curDbFileName;
         std::filesystem::remove(target);
      }
      operation = "Copy";
      std::filesystem::copy(source, target);
   } catch (std::filesystem::filesystem_error & fsError) {
      std::error_code errorCode = fsError.code();
      qWarning() <<
         Q_FUNC_INFO << "Error backing up database file " << curDbFileName << "to" << newDbFileName << ":" <<
         operation << "failed with" << errorCode << ".  Error message:" << fsError.what();
      return false;
   } catch (std::exception & exception) {
      // Most probably this would be std::bad_alloc, in which case we'd probably even have difficulty logging, but we
      // might as well try!
      qCritical() <<
         Q_FUNC_INFO << "Unexpected error backing up database file " << curDbFileName << "to" << newDbFileName << ":" <<
         operation << "failed:" << exception.what();
      return false;
   }

   return true;
}

bool Database::backupToDir(QString dir, QString filename) {
   QString prefix = dir + "/";
   QString newDbFileName = prefix + getDefaultBackupFileName();

   if ( !filename.isEmpty() ) {
      newDbFileName = prefix + filename;
   }

   return this->backupToFile( newDbFileName );
}

bool Database::restoreFromFile(QString newDbFileStr) {
   QFile newDbFile(newDbFileStr);
   // Fail if we can't find file.
   if (!newDbFile.exists()) {
      return false;
   }

   bool success = newDbFile.copy(QString("%1.new").arg(this->pimpl->dbFile.fileName()));
   QFile::setPermissions( newDbFile.fileName(), QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );

   return success;
}

// .:TBD:. What should we be doing, if anything, with schema?
bool Database::verifyDbConnection(Database::DbType testDb,
                                  QString const &  hostname,
                                  int              portnum,
                                  [[maybe_unused]] QString const &  schema,
                                  QString const &  database,
                                  QString const &  username,
                                  QString const &  password) {
   QString const testConnectionName{"testConnDb"};

   QString driverName;
   switch (testDb) {
      case Database::DbType::PGSQL:
         driverName = "QPSQL";
         break;
      default:
         driverName = "QSQLITE";
   }

   bool results = false;

   {
      // Extra braces here are to ensure that this QSqlDatabase object is out of scope before the call to
      // QSqlDatabase::removeDatabase() below
      QSqlDatabase connDb = QSqlDatabase::addDatabase(driverName, testConnectionName);

      switch (testDb) {
         case Database::DbType::PGSQL:
            connDb.setHostName(hostname);
            connDb.setPort(portnum);
            connDb.setDatabaseName(database);
            connDb.setUserName(username);
            connDb.setPassword(password);
            break;
         default:
            connDb.setDatabaseName(hostname);
      }

      results = connDb.open();

      if (results) {
         connDb.close();
      } else {
         QMessageBox::critical(
            nullptr,
            tr("Connection failed"),
            QString(tr("Could not connect to %1 : %2")).arg(hostname).arg(connDb.lastError().text())
         );
      }
   }

   QSqlDatabase::removeDatabase(testConnectionName);

   return results;
}

void Database::convertDatabase(QString const& Hostname, QString const& DbName,
                               QString const& Username, QString const& Password,
                               int Portnum, Database::DbType newType) {
   QSqlDatabase connectionNew;

   try {
      if (newType == Database::DbType::NODB) {
         throw QString("No type found for the new database.");
      }

      switch( newType ) {
         case Database::DbType::PGSQL:
            connectionNew = openPostgres(Hostname, DbName, Username, Password, Portnum);
            break;
         default:
            // .:TBD:. Feels like we should have filePath passed in rather than coming from PersistentSettings
            QString filePath = PersistentSettings::getUserDataDir().filePath("database.sqlite");
            connectionNew = openSQLite(filePath);
      }

      if ( ! connectionNew.isOpen() ) {
         throw QString("Could not open new database: %1").arg(connectionNew.lastError().text());
      }

      // Don't get newDatabase via Database::instance() as we don't want to use the connection details from
      // PersistentSettings (or to attempt to read data from newDatabase)
      Database newDatabase{newType};
      DatabaseSchemaHelper::copyToNewDatabase(newDatabase, connectionNew);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      throw;
   }
}

Database::DbType Database::dbType() const {
   return this->pimpl->dbType;
}

void Database::setForeignKeysEnabled(bool enabled, QSqlDatabase connection, Database::DbType type) {
   if (type == Database::DbType::NODB) {
      type = this->dbType();
   }

   QString queryString{""};
   switch (type) {
      case Database::DbType::SQLITE:
         queryString = QString{"PRAGMA foreign_keys=%1"}.arg(enabled ? "on": "off");
         break;
      case Database::DbType::PGSQL:
         // This is a bit of a hack, and needs you to be connected as super user, but seems more robust than
         // "SET CONSTRAINTS ALL DEFERRED" which requires foreign keys to have been set up in a particular way in the
         // first place (see https://www.postgresql.org/docs/13/sql-set-constraints.html).
         queryString = QString{"SET session_replication_role TO '%1'"}.arg(enabled ? "origin": "replica");
         break;
      default:
         // It's a coding error (somewhere) if we get here!
         Q_ASSERT(false);
   }

   BtSqlQuery sqlQuery{connection};
   sqlQuery.prepare(queryString);
   if (!sqlQuery.exec()) {
      qCritical() <<
         Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
      return;
   }

   return;
}

template<typename T> char const * Database::getDbNativeTypeName() const {
   return getDbNativeName(nativeTypeNames<T>, this->pimpl->dbType);
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template char const * Database::getDbNativeTypeName<bool>() const;
template char const * Database::getDbNativeTypeName<int>() const;
template char const * Database::getDbNativeTypeName<unsigned int>() const;
template char const * Database::getDbNativeTypeName<double>() const;
template char const * Database::getDbNativeTypeName<QString>() const;
template char const * Database::getDbNativeTypeName<QDate>() const;

char const * Database::getDbNativePrimaryKeyDeclaration() const {
   return getDbNativeName(nativeIntPrimaryKeyDeclaration, this->pimpl->dbType);
}

char const * Database::getSqlToAddColumnAsForeignKey() const {
   return getDbNativeName(sqlToAddColumnAsForeignKey, this->pimpl->dbType);
}

QList<QPair<QString, QString>> Database::displayableConnectionParms() const {
   switch (this->pimpl->dbType) {
      case Database::DbType::SQLITE:
         return {
            {tr("Filename"), this->pimpl->dbFileName}
         };
      case Database::DbType::PGSQL:
         return {
            { tr("Host & Port"), QString("%1:%2").arg(this->pimpl->dbHostname, this->pimpl->dbPortnum) },
            { tr("Database"),    this->pimpl->dbName     },
            { tr("Schema"),      this->pimpl->dbSchema   },
            { tr("Username"),    this->pimpl->dbUsername }
         };
      default:
         // It's a coding error (somewhere) if we get here!
         Q_ASSERT(false);
   }

   return {};
}

bool Database::updatePrimaryKeySequenceIfNecessary(QSqlDatabase & connection,
                                                   BtStringConst const & tableName,
                                                   BtStringConst const & columnName) const {
   switch (this->pimpl->dbType) {
      case Database::DbType::SQLITE:
         // Nothing to do for SQLite
         break;
      case Database::DbType::PGSQL:
         {
            //
            // https://wiki.postgresql.org/wiki/Fixing_Sequences has a big scary query you can run that will fix all
            // sequences in the database.  But to fix one sequence on one table, the work is more comprehensible.
            //
            // Per https://www.postgresql.org/docs/current/functions-info.html,
            // pg_get_serial_sequence(table_name, column_name) gets the name of the sequence that a serial, smallserial
            // or bigserial column uses.  (Usually, for column "id" on table "foo", the sequence will be called
            // "foo_id_seq".)  Note the need to quote the parameters.
            //
            // Per https://www.postgresql.org/docs/current/functions-sequence.html, setval(seq, val, advance)
            // will update the last_value field on sequence seq to val and, depending on whether advance is true or
            // false, will or won't increment the last_value field before the next call to nextval().  Note that, since
            // PostgreSQL 8.1, we _don't_ need to quote the sequence name.
            //
            // The result returned by setval is just the value of its second argument.  We only need FROM in the
            // statement below so that the MAX function will give us the current maximum value of the primary column
            // whose sequence we are updating.
            //
            // COALESCE covers the case where the table is empty (so MAX would return NULL).
            //
            // We use "setval(..., COALESCE(MAX(%2) + 1, 1), false)" to set the sequence to the next value that should
            // be used (and don't advance sequence before next insertion), rather than
            // "setval(..., COALESCE(MAX(%2), 0), true)" to set the sequence to the last insered value (and advance the
            // sequence before next insertion) because, in the case the table is empty, we don't want to set the
            // sequence to 0 (an invalid ID) in case it causes problems.
            //
            BtSqlQuery query{connection};
            query.prepare(
               QString("SELECT setval(pg_get_serial_sequence('%1', '%2'), COALESCE(MAX(%2) + 1, 1), false) "
                       "FROM %1;").arg(*tableName, *columnName)
            );
            if (!query.exec()) {
               qCritical() <<
                  Q_FUNC_INFO << "Error updating sequence value for column" << columnName << "on table" << tableName <<
                  "using SQL \"" << query.lastQuery() << "\":" << query.lastError().text();
               return false;
            }
            if (query.next()) {
               qInfo() <<
                  Q_FUNC_INFO << "Updated sequence value for column" << columnName << "on table" << tableName << "to" <<
                  query.value(0);
            }
         }
         break;
      default:
         // It's a coding error (somewhere) if we get here!
         Q_ASSERT(false);
         return false;
   }
   return true;
}

template<class S>
S & operator<<(S & stream, Database::DbType const dbType) {
   std::optional<QString> dbTypeAsString = dbTypeToName.enumToString(dbType);
   if (dbTypeAsString) {
      stream << *dbTypeAsString;
   } else {
      // This is a coding error
      stream << "Unrecognised database type: " << static_cast<int>(dbType);
   }
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug &      operator<<(QDebug &      stream, Database::DbType const dbType);
template QTextStream & operator<<(QTextStream & stream, Database::DbType const dbType);

//======================================================================================================================
//====================================== Start of Functions in Helper Namespace ========================================
//======================================================================================================================
char const * DatabaseHelper::getNameFromDbTypeName(Database::DbType whichDb) {
   return getDbNativeName(displayableDbType, whichDb);
}
