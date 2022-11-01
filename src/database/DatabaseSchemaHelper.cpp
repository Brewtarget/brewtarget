/*
 * database/DatabaseSchemaHelper.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "database/DatabaseSchemaHelper.h"

#include <algorithm> // For std::sort and std::set_difference

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "Application.h"
#include "database/BtSqlQuery.h"
#include "database/Database.h"
#include "database/DbTransaction.h"
#include "database/ObjectStoreWrapper.h"
#include "model/BrewNote.h"
#include "model/Recipe.h"
#include "model/Water.h"
#include "xml/BeerXml.h"

int const DatabaseSchemaHelper::dbVersion = 10;

namespace {
   char const * const FOLDER_FOR_SUPPLIED_RECIPES = "brewtarget";

   struct QueryAndParameters {
      QString sql;
      QVector<QVariant> bindValues = {};
      bool onlyRunIfPriorQueryHadResults = false;
   };

   //
   // These migrate_to_Xyz functions are deliberately hard-coded.  Because we're migrating from version N to version
   // N+1, we don't need (or want) to refer to the generated table definitions from some later version of the schema,
   // which may be quite different.
   //
   // That said, we have rewritten history in a few places where it simplifies things.  In particular, we have omitted
   // default values that were used in earlier versions of the schema because (a) in current versions of the code they
   // are not used and (b) setting them in a way that works for SQLite and PostgreSQL is a bit painful thanks to the
   // differing ways they handle booleans ("DEFAULT true" in PostgreSQL has to be "DEFAULT 1" in SQLite etc, which is a
   // bit tedious).
   //
   bool executeSqlQueries(BtSqlQuery & q, QVector<QueryAndParameters> const & queries) {
      //
      // Sometimes whether or not we want to run a query depends on what data is in the database.  Eg, if we're trying
      // to insert into a table based on the results of a sub-query, we need to handle the case where the sub-query
      // returns no results.  This can be painful to do in SQL, so it's simpler to do a dummy-run of the sub-query (or
      // some adapted version of it) first, and then make running the real query dependent on whether the dummy-run
      // returned any results.
      //
      bool priorQueryHadResults = false;
      QString priorQuerySql = "N/A";

      for (auto & query : queries) {
         if (query.onlyRunIfPriorQueryHadResults && !priorQueryHadResults) {
            qInfo() <<
               Q_FUNC_INFO << "Skipping upgrade query \"" << query.sql << "\" as was dependent on prior upgrade "
               "query (\"" << priorQuerySql << "\") returning results, and it didn't";
            // We deliberately don't update priorQueryHadResults or priorQuerySql in this case, as it allows more than
            // one query in a row to be dependent on a single "dummy-run" query
            continue;
         }
         qDebug() << Q_FUNC_INFO << query.sql;

         q.prepare(query.sql);
         for (auto & bv : query.bindValues) {
            q.addBindValue(bv);
         }
         if (!q.exec()) {
            // If we get an error, we want to stop processing as otherwise you get "false" errors if subsequent queries
            // fail as a result of assuming that all prior queries have run OK.
            qCritical() <<
               Q_FUNC_INFO << "Error executing database upgrade/set-up query " << query.sql << ": " <<
               q.lastError().text();
            return false;
         }
         priorQueryHadResults = q.next();
         priorQuerySql = query.sql;
      }
      return true;
   }

   // This is when we first defined the settings table, and defined the version as a string.
   // In the new world, this will create the settings table and define the version as an int.
   // Since we don't set the version until the very last step of the update, I think this will be fine.
   bool migrate_to_202(Database & db, BtSqlQuery q) {
      bool ret = true;

      // Add "projected_ferm_points" to brewnote table
      QString queryString{"ALTER TABLE brewnote ADD COLUMN projected_ferm_points "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << db.getDbNativeTypeName<double>() << ";"; // Previously DEFAULT 0.0
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);
      queryString = "ALTER TABLE brewnote SET projected_ferm_points = -1.0;";
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);

      // Add the settings table
      queryString = "CREATE TABLE settings ";
      queryStringAsStream << "(\n"
         "id " << db.getDbNativePrimaryKeyDeclaration() << ",\n"
         "repopulatechildrenonnextstart " << db.getDbNativeTypeName<int>() << ",\n" // Previously DEFAULT 0
         "version " << db.getDbNativeTypeName<int>() << ");"; // Previously DEFAULT 0
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);

      return ret;
   }

   bool migrate_to_210(Database & db, BtSqlQuery & q) {
      QVector<QueryAndParameters> migrationQueries{
         {QString("ALTER TABLE equipment   ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE fermentable ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE hop         ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE misc        ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE style       ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE yeast       ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE water       ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE mash        ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE recipe      ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE brewnote    ADD COLUMN folder text")}, // Previously DEFAULT ''
         {QString("ALTER TABLE salt        ADD COLUMN folder text")}, // Previously DEFAULT ''
         // Put the "Bt:.*" recipes into /brewtarget folder
         {QString("UPDATE recipe   SET folder='/brewtarget' WHERE name LIKE 'Bt:%'")},
         // Update version to 2.1.0
         {QString("UPDATE settings SET version='2.1.0' WHERE id=1")},
         // Used to trigger the code to populate the ingredient inheritance tables
         {QString("ALTER TABLE settings ADD COLUMN repopulatechildrenonnextstart %1").arg(db.getDbNativeTypeName<int>())},
         {QString("UPDATE repopulatechildrenonnextstart integer=1")},
      };
      // Drop and re-create children tables with new UNIQUE requirement
      for (char const * baseTableName : {"equipment", "fermentable", "hop", "misc", "recipe", "style", "water", "yeast"}) {
         migrationQueries.append({QString("DROP TABLE %1_children").arg(baseTableName)});
         migrationQueries.append({QString(
                                    "CREATE TABLE %1_children (id %2, "
                                                              "child_id %3, "
                                                              "parent_id %3, "
                                                              "FOREIGN KEY(child_id) REFERENCES %1(id), "
                                                              "FOREIGN KEY(parent_id) REFERENCES %1(id));"
                                 ).arg(baseTableName,
                                       db.getDbNativePrimaryKeyDeclaration(),
                                       db.getDbNativeTypeName<int>())});
      }
      for (char const * tableName : {"fermentable_in_inventory", "hop_in_inventory", "misc_in_inventory"}) {
         migrationQueries.append({QString("DROP TABLE   %1;").arg(tableName)});
         migrationQueries.append(
            {
               QString(
                  "CREATE TABLE %1 (id %2, "
                                 "amount %3);" // Previously DEFAULT 0
               ).arg(tableName, db.getDbNativePrimaryKeyDeclaration(), db.getDbNativeTypeName<double>())
            }
         );
      }
      migrationQueries.append({QString("DROP TABLE   yeast_in_inventory")});
      migrationQueries.append(
         {
            QString("CREATE TABLE %1 (id %2, "
                                     "quanta %3);" // Previously DEFAULT 0
            ).arg("yeast_in_inventory", db.getDbNativePrimaryKeyDeclaration(), db.getDbNativeTypeName<double>())
         }
      );
      migrationQueries.append({QString("UPDATE settings VALUES(1,2)")});
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_4(Database & db, BtSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Save old settings
         {QString("ALTER TABLE settings RENAME TO oldsettings")},
         // Create new table with integer version.
         {
            QString(
               "CREATE TABLE settings (id %2, "
                                      "repopulatechildrenonnextstart %1, " // Previously DEFAULT 0
                                      "version %1);" // Previously DEFAULT 0
            ).arg(db.getDbNativeTypeName<int>()).arg(db.getDbNativePrimaryKeyDeclaration())
         },
         // Update version to 4, saving other settings
         {QString("INSERT INTO settings (id, version, repopulatechildrenonnextstart) SELECT 1, 4, repopulatechildrenonnextstart FROM oldsettings")},
         // Cleanup
         {QString("DROP TABLE oldsettings")}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_5(Database & db, BtSqlQuery q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Drop the previous bugged TRIGGER
         {QString("DROP TRIGGER dec_ins_num")},
         // Create the good trigger
            {QString("CREATE TRIGGER dec_ins_num AFTER DELETE ON instruction_in_recipe "
                 "BEGIN "
                    "UPDATE instruction_in_recipe "
                    "SET instruction_number = instruction_number - 1 "
                    "WHERE recipe_id = OLD.recipe_id "
                    "AND instruction_number > OLD.instruction_number; "
                 "END")}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   //
   bool migrate_to_6(Database & db, BtSqlQuery q) {
      bool ret = true;
      // I drop this table in version 8. There is no sense doing anything here, and it breaks other things.
      return ret;
   }

   bool migrate_to_7(Database & db, BtSqlQuery q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Add "attenuation" to brewnote table
         {"ALTER TABLE brewnote ADD COLUMN attenuation real"} // Previously DEFAULT 0.0
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_8(Database & db, BtSqlQuery q) {
      QString createTmpBrewnoteSql;
      QTextStream createTmpBrewnoteSqlStream(&createTmpBrewnoteSql);
      createTmpBrewnoteSqlStream <<
         "CREATE TABLE tmpbrewnote ("
            "id                      " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "abv                     " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 0
            "attenuation             " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "boil_off                " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "brewdate                " << db.getDbNativeTypeName<QDate>()   << ", " // Previously DEFAULT CURRENT_TIMESTAMP
            "brewhouse_eff           " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 70
            "deleted                 " << db.getDbNativeTypeName<bool>()    << ", " // Previously DEFAULT 0 / false
            "display                 " << db.getDbNativeTypeName<bool>()    << ", " // Previously DEFAULT 1 / true
            "eff_into_bk             " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 70
            "fermentdate             " << db.getDbNativeTypeName<QDate>()   << ", " // Previously DEFAULT CURRENT_TIMESTAMP
            "fg                      " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "final_volume            " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "folder                  " << db.getDbNativeTypeName<QString>() << ", " // Previously DEFAULT ''
            "mash_final_temp         " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 67
            "notes                   " << db.getDbNativeTypeName<QString>() << ", " // Previously DEFAULT ''
            "og                      " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "pitch_temp              " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 20
            "post_boil_volume        " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 0
            "projected_abv           " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_atten         " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 75
            "projected_boil_grav     " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_eff           " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_ferm_points   " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_fg            " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_mash_fin_temp " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 67
            "projected_og            " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_points        " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_strike_temp   " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 70
            "projected_vol_into_bk   " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "projected_vol_into_ferm " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 0
            "sg                      " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 1
            "strike_temp             " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 70
            "volume_into_bk          " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 0
            "volume_into_fermenter   " << db.getDbNativeTypeName<double>()  << ", " // Previously DEFAULT 0
            "recipe_id               " << db.getDbNativeTypeName<int>()     << ", "
            "FOREIGN KEY(recipe_id) REFERENCES recipe(id)"
         ");";
      QVector<QueryAndParameters> migrationQueries{
         //
         // Drop columns predicted_og and predicted_abv. They are used nowhere I can find and they are breaking things.
         //
         {createTmpBrewnoteSql},
         {QString("SELECT id FROM brewnote")}, // Dummy-run query
         {QString("INSERT INTO tmpbrewnote ("
                    "id, "
                    "abv, "
                    "attenuation, "
                    "boil_off, "
                    "brewdate, "
                    "brewhouse_eff, "
                    "deleted, "
                    "display, "
                    "eff_into_bk, "
                    "fermentdate, "
                    "fg, "
                    "final_volume, "
                    "folder, "
                    "mash_final_temp, "
                    "notes, "
                    "og, "
                    "pitch_temp, "
                    "post_boil_volume, "
                    "projected_abv, "
                    "projected_atten, "
                    "projected_boil_grav, "
                    "projected_eff, "
                    "projected_ferm_points, "
                    "projected_fg, "
                    "projected_mash_fin_temp, "
                    "projected_og, "
                    "projected_points, "
                    "projected_strike_temp, "
                    "projected_vol_into_bk, "
                    "projected_vol_into_ferm, "
                    "sg, "
                    "strike_temp, "
                    "volume_into_bk, "
                    "volume_into_fermenter, "
                    "recipe_id"
                 ") SELECT id, "
                    "abv, "
                    "attenuation, "
                    "boil_off, "
                    "brewdate, "
                    "brewhouse_eff, "
                    "deleted, "
                    "display, "
                    "eff_into_bk, "
                    "fermentdate, "
                    "fg, "
                    "final_volume, "
                    "folder, "
                    "mash_final_temp, "
                    "notes, "
                    "og, "
                    "pitch_temp, "
                    "post_boil_volume, "
                    "projected_abv, "
                    "projected_atten, "
                    "projected_boil_grav, "
                    "projected_eff, "
                    "projected_ferm_points, "
                    "projected_fg, "
                    "projected_mash_fin_temp, "
                    "projected_og, "
                    "projected_points, "
                    "projected_strike_temp, "
                    "projected_vol_into_bk, "
                    "projected_vol_into_ferm, "
                    "sg, "
                    "strike_temp, "
                    "volume_into_bk, "
                    "volume_into_fermenter, "
                    "recipe_id "
                 "FROM brewnote"),
            {},
            true // Don't run this query if the previous one had no results (ie there's nothing to insert)
         },
         {QString("drop table brewnote")},
         {QString("ALTER TABLE tmpbrewnote RENAME TO brewnote")}
      };
      //
      // Rearrange inventory
      //
      for (char const * baseTableName : {"fermentable", "hop", "misc", "yeast"}) {
         // On the yeast tables, we use "quanta" instead of "amount", which turns out to be mildly annoying in all sorts
         // of ways.  One day we'll fix it to be consistent with the other tables.  For now we have to do horrible
         // things like this.
         QString const amountColumnName{QString{baseTableName} == "yeast" ? "quanta" : "amount"};

         // This gives us the the DB-specific version of
         //    ALTER TABLE %1 ADD COLUMN inventory_id REFERENCES %1_in_inventory (id)
         // where %1 is baseTableName!
         QString inInventoryTable = QString("%1_in_inventory").arg(baseTableName);
         migrationQueries.append({QString(db.getSqlToAddColumnAsForeignKey()).arg(baseTableName,
                                                                                  "inventory_id",
                                                                                  inInventoryTable,
                                                                                  "id")});
         // It would seem we have kids with their own rows in the db. This is a freaking mess, but I need to delete
         // those rows before I can do anything else.
         migrationQueries.append({QString("DELETE FROM %1_in_inventory "
                                          "WHERE %1_in_inventory.id in ( "
                                             "SELECT %1_in_inventory.id "
                                             "FROM %1_in_inventory, %1_children, %1 "
                                             "WHERE %1.id = %1_children.child_id "
                                             "AND %1_in_inventory.%1_id = %1.id )").arg(baseTableName)});
         // This next is a dummy-run query for the subsequent insert.  We don't want to try to do the insert if this
         // query has no results as it will barf trying to insert no rows.  (AFAIK there isn't an elegant way around
         // this in SQL.)
         migrationQueries.append({QString("SELECT id FROM %1 WHERE NOT EXISTS ( "
                                             "SELECT %1_children.id "
                                             "FROM %1_children "
                                             "WHERE %1_children.child_id = %1.id "
                                          ") AND NOT EXISTS ( "
                                             "SELECT %1_in_inventory.id "
                                             "FROM %1_in_inventory "
                                             "WHERE %1_in_inventory.%1_id = %1.id"
                                          ")").arg(baseTableName)});
         migrationQueries.append({
            QString("INSERT INTO %1_in_inventory (%1_id) "
                    // Everything has an inventory row now. This will find all the parent items that don't have an
                    // inventory row.
                    "SELECT id FROM %1 WHERE NOT EXISTS ( "
                       "SELECT %1_children.id "
                       "FROM %1_children "
                       "WHERE %1_children.child_id = %1.id "
                    ") AND NOT EXISTS ( "
                       "SELECT %1_in_inventory.id "
                       "FROM %1_in_inventory "
                       "WHERE %1_in_inventory.%1_id = %1.id"
                    ")"
            ).arg(baseTableName),
            {},
            true // Don't run this query if the previous one had no results
         });
         // Once we know all parents have inventory rows, we populate inventory_id for them
         migrationQueries.append({QString("UPDATE %1 SET inventory_id = ("
                                             "SELECT %1_in_inventory.id "
                                             "FROM %1_in_inventory "
                                             "WHERE %1.id = %1_in_inventory.%1_id"
                                          ")").arg(baseTableName)});
         // Finally, we update all the kids to have the same inventory_id as their dear old paw
         migrationQueries.append({QString("UPDATE %1 SET inventory_id = ( "
                                             "SELECT tmp.inventory_id "
                                             "FROM %1 tmp, %1_children "
                                             "WHERE %1.id = %1_children.child_id "
                                             "AND tmp.id = %1_children.parent_id"
                                          ") "
                                          "WHERE inventory_id IS NULL").arg(baseTableName)});
      }
      //
      // We need to drop the appropriate columns from the inventory tables
      // Scary, innit? The changes above basically reverse the relation.
      // Instead of inventory knowing about ingredients, we now have ingredients
      // knowing about inventory. I am concerned that leaving these in place
      // will cause circular references
      //
      for (char const * baseTableName : {"fermentable", "hop", "misc", "yeast"}) {
         // See comment above for annoying use of "quanta" in yeast tables
         QString const amountColumnName{QString{baseTableName} == "yeast" ? "quanta" : "amount"};
         migrationQueries.append({QString(
                                     "CREATE TABLE tmp%1_in_inventory (id %2, %3 %4);" // Previously DEFAULT 0
                                  ).arg(baseTableName,
                                        db.getDbNativePrimaryKeyDeclaration(),
                                        amountColumnName,
                                        db.getDbNativeTypeName<double>())});
         migrationQueries.append({QString("INSERT INTO tmp%1_in_inventory (id, %2) "
                                          "SELECT id, %2 "
                                          "FROM %1_in_inventory").arg(baseTableName, amountColumnName)});
         migrationQueries.append({QString("DROP TABLE %1_in_inventory").arg(baseTableName)});
         migrationQueries.append({QString("ALTER TABLE tmp%1_in_inventory "
                                          "RENAME TO %1_in_inventory").arg(baseTableName)});
      }
      //
      // Finally, the btalltables table isn't needed, so drop it
      //
      migrationQueries.append({QString("DROP TABLE IF EXISTS bt_alltables")});

      return executeSqlQueries(q, migrationQueries);
   }

   // To support the water chemistry, I need to add two columns to water and to
   // create the salt and salt_in_recipe tables
   bool migrate_to_9(Database & db, BtSqlQuery q) {
      QString createSaltSql;
      QTextStream createSaltSqlStream(&createSaltSql);
      createSaltSqlStream <<
         "CREATE TABLE salt ( "
            "id               " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "addTo            " << db.getDbNativeTypeName<int>()     << "         , " // Previously DEFAULT 0
            "amount           " << db.getDbNativeTypeName<double>()  << "         , " // Previously DEFAULT 0
            "amount_is_weight " << db.getDbNativeTypeName<bool>()    << "         , " // Previously DEFAULT 1 / true
            "deleted          " << db.getDbNativeTypeName<bool>()    << "         , " // Previously DEFAULT 0 / false
            "display          " << db.getDbNativeTypeName<bool>()    << "         , " // Previously DEFAULT 1 / true
            "folder           " << db.getDbNativeTypeName<QString>() << "         , " // Previously DEFAULT ''
            "is_acid          " << db.getDbNativeTypeName<bool>()    << "         , " // Previously DEFAULT 0 / false
            "name             " << db.getDbNativeTypeName<QString>() << " not null, " // Previously DEFAULT ''
            "percent_acid     " << db.getDbNativeTypeName<double>()  << "         , " // Previously DEFAULT 0
            "stype            " << db.getDbNativeTypeName<int>()     << "         , " // Previously DEFAULT 0
            "misc_id          " << db.getDbNativeTypeName<int>()     << ", "
            "FOREIGN KEY(misc_id) REFERENCES misc(id)"
         ");";
      QVector<QueryAndParameters> const migrationQueries{
         {QString(
            "ALTER TABLE water ADD COLUMN wtype      %1" // Previously DEFAULT 0
          ).arg(db.getDbNativeTypeName<int>())},
         {QString(
            "ALTER TABLE water ADD COLUMN alkalinity %1" // Previously DEFAULT 0
          ).arg(db.getDbNativeTypeName<double>())},
         {QString(
            "ALTER TABLE water ADD COLUMN as_hco3    %1" // Previously DEFAULT 1 / true
          ).arg(db.getDbNativeTypeName<bool>())},
         {QString(
            "ALTER TABLE water ADD COLUMN sparge_ro  %1" // Previously DEFAULT 0
          ).arg(db.getDbNativeTypeName<double>())},
         {QString(
            "ALTER TABLE water ADD COLUMN mash_ro    %1" // Previously DEFAULT 0
          ).arg(db.getDbNativeTypeName<double>())},
         {createSaltSql},
         {QString("CREATE TABLE salt_in_recipe ( "
                    "id        %2, "
                    "recipe_id %1, "
                    "salt_id   %1, "
                    "FOREIGN KEY(recipe_id) REFERENCES recipe(id), "
                    "FOREIGN KEY(salt_id)   REFERENCES salt(id)"
                 ");").arg(db.getDbNativeTypeName<int>(), db.getDbNativePrimaryKeyDeclaration())}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_10(Database & db, BtSqlQuery q) {
      QVector<QueryAndParameters> const migrationQueries{
         // DB-specific version of ALTER TABLE recipe ADD COLUMN ancestor_id INTEGER REFERENCES recipe(id)
         {QString(db.getSqlToAddColumnAsForeignKey()).arg("recipe",
                                                          "ancestor_id",
                                                          "recipe",
                                                          "id")},
         {QString("ALTER TABLE recipe ADD COLUMN locked %1").arg(db.getDbNativeTypeName<bool>())},
         {QString("UPDATE recipe SET locked = ?"), {QVariant{false}}},
         // By default a Recipe is its own ancestor.  So, we need to set ancestor_id = id where display = true and ancestor_id is null
         {QString("UPDATE recipe SET ancestor_id = id WHERE display = ? and ancestor_id IS NULL"), {QVariant{true}}}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   /*!
    * \brief Migrate from version \c oldVersion to \c oldVersion+1
    */
   bool migrateNext(Database & database, int oldVersion, QSqlDatabase db ) {
      qDebug() << Q_FUNC_INFO << "Migrating DB schema from v" << oldVersion << "to v" << oldVersion + 1;
      BtSqlQuery sqlQuery(db);
      bool ret = true;

      // NOTE: Add a new case when adding a new schema change
      switch(oldVersion)
      {
         case 1: // == '2.0.0'
            ret &= migrate_to_202(database, sqlQuery);
            break;
         case 2: // == '2.0.2'
            ret &= migrate_to_210(database, sqlQuery);
            break;
         case 3: // == '2.1.0'
            ret &= migrate_to_4(database, sqlQuery);
            break;
         case 4:
            ret &= migrate_to_5(database, sqlQuery);
            break;
         case 5:
            ret &= migrate_to_6(database, sqlQuery);
            break;
         case 6:
            ret &= migrate_to_7(database, sqlQuery);
            break;
         case 7:
            ret &= migrate_to_8(database, sqlQuery);
            break;
         case 8:
            ret &= migrate_to_9(database, sqlQuery);
            break;
         case 9:
            ret &= migrate_to_10(database, sqlQuery);
            break;
         default:
            qCritical() << QString("Unknown version %1").arg(oldVersion);
            return false;
      }

      // Set the db version
      if( oldVersion > 3 )
      {
         QString queryString{"UPDATE settings SET version=:version WHERE id=1"};
         sqlQuery.prepare(queryString);
         QVariant bindValue{QString::number(oldVersion + 1)};
         sqlQuery.bindValue(":version", bindValue);
         ret &= sqlQuery.exec();
      }

      return ret;
   }

}
bool DatabaseSchemaHelper::upgrade = false;
// Default namespace hides functions from everything outside this file.

bool DatabaseSchemaHelper::create(Database & database, QSqlDatabase connection) {
   //--------------------------------------------------------------------------
   // NOTE: if you edit this function, increment dbVersion and edit
   // migrateNext() appropriately.
   //--------------------------------------------------------------------------

   // NOTE: none of the BeerXML property names should EVER change. This is to
   //       ensure backwards compatibility when rolling out ingredient updates to
   //       old versions.

   // NOTE: deleted=1 means the ingredient is "deleted" and should not be shown in
   //                 any list.
   //       deleted=0 means it isn't deleted and may or may not be shown.
   //       display=1 means the ingredient should be shown in a list, available to
   //                 be put into a recipe.
   //       display=0 means the ingredient is in a recipe already and should not
   //                 be shown in a list, available to be put into a recipe.

   // Start transaction
   // By the magic of RAII, this will abort if we exit this function (including by throwing an exception) without
   // having called dbTransaction.commit().
   DbTransaction dbTransaction{database, connection};

   qDebug() << Q_FUNC_INFO;
   if (!CreateAllDatabaseTables(database, connection)) {
      return false;
   }

   //
   // Create the settings table manually, since it's only used in this file
   //
   // NB: For reasons lost in the mists of time, the repopulateChildrenOnNextStart column was originally implemented as
   // an integer and not a boolean.
   //
   QVector<QueryAndParameters> const setUpQueries{
      {QString("CREATE TABLE settings (id %2, repopulatechildrenonnextstart %1, version %1)").arg(database.getDbNativeTypeName<int>(), database.getDbNativePrimaryKeyDeclaration())},
      {QString("INSERT INTO settings (repopulatechildrenonnextstart, version) VALUES (?, ?)"), {QVariant(1), QVariant(dbVersion)}}

   };
   BtSqlQuery sqlQuery{connection};

   if (!executeSqlQueries(sqlQuery, setUpQueries)) {
      return false;
   }

   // If we got here, everything went well, so we can commit the DB transaction now, otherwise it will have aborted when
   // we returned from an error branch above.
   dbTransaction.commit();

   return true;
}

bool DatabaseSchemaHelper::migrate(Database & database, int oldVersion, int newVersion, QSqlDatabase connection) {
   if( oldVersion >= newVersion || newVersion > dbVersion ) {
      qDebug() << Q_FUNC_INFO <<
         QString("Requested backwards migration from %1 to %2: You are an imbecile").arg(oldVersion).arg(newVersion);
      return false;
   }

   bool ret = true;
   qDebug() << Q_FUNC_INFO << "Migrating database schema from v" << oldVersion << "to v" << newVersion;

   // Start transaction
   // By the magic of RAII, this will abort if we exit this function (including by throwing an exception) without
   // having called dbTransaction.commit().  (It will also turn foreign keys back on either way -- whether the
   // transaction is committed or rolled back.)
   DbTransaction dbTransaction{database, connection, DbTransaction::DISABLE_FOREIGN_KEYS};

   for ( ; oldVersion < newVersion && ret; ++oldVersion ) {
      ret &= migrateNext(database, oldVersion, connection);
   }

   // If all statements executed OK, we can commit, otherwise the transaction will roll back when we exit this function
   if (ret) {
      ret &= dbTransaction.commit();
   }

   return ret;
}

int DatabaseSchemaHelper::currentVersion(QSqlDatabase db) {
   // Version was a string field in early versions of the code and then became an integer field
   // We'll read it into a QVariant and then work out whether it's a string or an integer
   BtSqlQuery q("SELECT version FROM settings WHERE id=1", db);
   QVariant ver;
   if( q.next() ) {
      ver = q.value("version");
   } else {
      // No settings table in version 2.0.0
      ver = QString("2.0.0");
   }

   // Get the string before we kill it by convert()-ing
   QString stringVer( ver.toString() );
   qDebug() << Q_FUNC_INFO << "Database schema version" << stringVer;

   // Initially, versioning was done with strings, so we need to convert
   // the old version strings to integer versions
   if ( ver.convert(QVariant::Int) ) {
      return ver.toInt();
   }

   if( stringVer == "2.0.0" ) {
      return 1;
   }

   if( stringVer == "2.0.2" ) {
      return 2;
   }

   if( stringVer == "2.1.0" ) {
      return 3;
   }

   qCritical() << "Could not find database version";
   return -1;
}

bool DatabaseSchemaHelper::copyToNewDatabase(Database & newDatabase, QSqlDatabase & connectionNew) {

   // this is to prevent us from over-writing or doing heavens knows what to an existing db
   if (connectionNew.tables().contains(QLatin1String("settings"))) {
      qWarning() << Q_FUNC_INFO << "It appears the database is already configured.";
      return false;
   }

   // The crucial bit is creating the new tables in the new DB.  Once that is done then, assuming disabling of foreign
   // keys works OK, it should be turn-the-handle to write out all the data.
   if (!DatabaseSchemaHelper::create(newDatabase, connectionNew)) {
      qCritical() << Q_FUNC_INFO << "Error creating tables in new DB";
      return false;
   }

   if (!WriteAllObjectStoresToNewDb(newDatabase, connectionNew)) {
      qCritical() << Q_FUNC_INFO << "Error writing data to new DB";
      return false;
   }

   return true;
}


/**
 * \brief Imports any new default data to the database.  This is what gets called when the user responds Yes to the
 *        dialog saying "There are new ingredients, would you like to merge?"
 *
 *        In older versions of the software, default data was copied from a SQLite database file (default_db.sqlite)
 *        into the user's database (which could be SQLite or PostgreSQL), and special tables (bt_hop, bt_fermentable,
 *        etc) kept track of which records in the user's database had been copies from the default database.  This
 *        served two purposes.  One was to know which default records were present in the user's database, so we could
 *        copy across any new ones when the default data set is augmented.  The other was to allow us to attempt to
 *        modify the user's records when corresponding records in the default data set were changed (typically to make
 *        corrections).  However, it's risky to modify the user's existing data because you might overwrite changes
 *        they've made themselves since the record was imported.  So we stopped trying to do that, and used the special
 *        tables just to track which default records had and hadn't yet been imported.
 *
 *        What we do now is store the default data in BeerXML.  (We will likely switch to storing in BeerJSON once we
 *        support that.)  Besides simplifying this function, this has a couple of advantages:
 *           - Being a text rather than a binary format, it's much easier in the source code repository to make (and
 *             see) changes to default data.
 *           - Our XML import code already does duplicate detection, so don't need the special tracking tables any more.
 *             We just try to import all the default data, and any records that the user already has will be skipped
 *             over.
 */
bool DatabaseSchemaHelper::updateDatabase(QTextStream & userMessage) {

   //
   // We'd like to put any newly-imported default Recipes in the same folder as the other default ones.  To do this, we
   // first make a note of which Recipes exist already, then, after the import, any new ones need to go in the default
   // folder.
   //
   QList<Recipe *> allRecipesBeforeImport = ObjectStoreWrapper::getAllRaw<Recipe>();
   qDebug() << Q_FUNC_INFO << allRecipesBeforeImport.size() << "Recipes before import";

   QString const defaultDataFileName = Application::getResourceDir().filePath("DefaultData.xml");
   bool succeeded = BeerXML::getInstance().importFromXML(defaultDataFileName, userMessage);

   if (succeeded) {
      //
      // Now see what Recipes exist that weren't there before the import
      //
      QList<Recipe *> allRecipesAfterImport = ObjectStoreWrapper::getAllRaw<Recipe>();
      qDebug() << Q_FUNC_INFO << allRecipesAfterImport.size() << "Recipes after import";

      //
      // Once the lists are sorted, finding the difference is just a library call
      // (Note that std::set_difference requires the longer list as its first parameter pair.)
      //
      std::sort(allRecipesBeforeImport.begin(), allRecipesBeforeImport.end());
      std::sort(allRecipesAfterImport.begin(), allRecipesAfterImport.end());
      QList<Recipe *> newlyImportedRecipes;
      std::set_difference(allRecipesAfterImport.begin(), allRecipesAfterImport.end(),
                          allRecipesBeforeImport.begin(), allRecipesBeforeImport.end(),
                          std::back_inserter(newlyImportedRecipes));
      qDebug() << Q_FUNC_INFO << newlyImportedRecipes.size() << "newly imported Recipes";
      for (auto recipe : newlyImportedRecipes) {
         recipe->setFolder(FOLDER_FOR_SUPPLIED_RECIPES);
      }
   }

   return succeeded;
}
