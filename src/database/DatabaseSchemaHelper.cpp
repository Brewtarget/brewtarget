/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/DatabaseSchemaHelper.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "database/ObjectStoreTyped.h"

int constexpr DatabaseSchemaHelper::latestVersion = 14;

// Default namespace hides functions from everything outside this file.
namespace {

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
         qDebug() << Q_FUNC_INFO << q.numRowsAffected() << "rows affected";
         priorQueryHadResults = q.next();
         priorQuerySql = query.sql;
      }
      return true;
   }

   // This is when we first defined the settings table, and defined the version as a string.
   // In the new world, this will create the settings table and define the version as an int.
   // Since we don't set the version until the very last step of the update, I think this will be fine.
   bool migrate_to_202(Database & db, BtSqlQuery & q) {
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
         {QString("ALTER TABLE equipment   ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE fermentable ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE hop         ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE misc        ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE style       ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE yeast       ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE water       ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE mash        ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE recipe      ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE brewnote    ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         {QString("ALTER TABLE salt        ADD COLUMN folder %1").arg(db.getDbNativeTypeName<QString>())}, // Previously DEFAULT ''
         // Put the "Bt:.*" recipes into /brewtarget folder
         {QString("UPDATE recipe   SET folder='/brewtarget' WHERE name LIKE 'Bt:%'")},
         // Update version to 2.1.0
         {QString("UPDATE settings SET version='2.1.0' WHERE id=1")},
         // Used to trigger the code to populate the ingredient inheritance tables.  Gets removed in schema version 11.
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

   bool migrate_to_5([[maybe_unused]] Database & db, BtSqlQuery & q) {
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
   bool migrate_to_6([[maybe_unused]] Database & db, [[maybe_unused]] BtSqlQuery & q) {
      // I drop this table in version 8. There is no sense doing anything here, and it breaks other things.
      return true;
   }

   bool migrate_to_7([[maybe_unused]] Database & db, BtSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Add "attenuation" to brewnote table
         {QString("ALTER TABLE brewnote ADD COLUMN attenuation %1").arg(db.getDbNativeTypeName<double>())} // Previously DEFAULT 0.0
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_8(Database & db, BtSqlQuery & q) {
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
   bool migrate_to_9(Database & db, BtSqlQuery & q) {
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

   bool migrate_to_10(Database & db, BtSqlQuery & q) {
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

   /**
    * \brief This is a lot of schema and data changes to support BeerJSON - or rather the new data structures that
    *        BeerJSON introduces over BeerXML and what else we already had.  We also try to standardise some
    *        serialisations across BeerJSON, DB and UI.
    *
    *        Where we are adding new columns (or otherwise renaming existing ones) we are starting to try to use the
    *        same convention we have for properties where the "units" of the column are appended to its name - hence
    *        names ending in "_pct" (for percent), "_l" (for liters), etc.  One day perhaps we'll rename all the
    *        relevant existing columns, but I think we've got enough other change in this update!
    */
   bool migrate_to_11(Database & db, BtSqlQuery & q) {
      //
      // Some of the bits of SQL would be too cumbersome to build up in-place inside the migrationQueries vector, so
      // we use string streams to do the string construction here.
      //
      // Note that the `temp_recipe_id` columns are used just for the initial population of the table and are then
      // dropped.  (For each row in recipe, we need to create a new row in boil and then update the row in recipe to
      // refer to it.  Temporarily putting the recipe_id on boil, without a foreign key constraint, makes this a lot
      // simpler.  Same applied to fermentation.)
      //
      QString createBoilSql;
      QTextStream createBoilSqlStream(&createBoilSql);
      createBoilSqlStream <<
         "CREATE TABLE boil ("
            "id"              " " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "name"            " " << db.getDbNativeTypeName<QString>()     << ", "
            "deleted"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "display"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "folder"          " " << db.getDbNativeTypeName<QString>()     << ", "
            "description"     " " << db.getDbNativeTypeName<QString>()     << ", "
            "notes"           " " << db.getDbNativeTypeName<QString>()     << ", "
            "pre_boil_size_l" " " << db.getDbNativeTypeName<double>()      << ", "
            "boil_time_mins"  " " << db.getDbNativeTypeName<double>()      << ", "
            "temp_recipe_id"  " " << db.getDbNativeTypeName<int>()         <<
         ");";

      QString createBoilStepSql;
      QTextStream createBoilStepSqlStream(&createBoilStepSql);
      createBoilStepSqlStream <<
         "CREATE TABLE boil_step ("
            "id"               " " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "name"             " " << db.getDbNativeTypeName<QString>()     << ", "
            "deleted"          " " << db.getDbNativeTypeName<bool>()        << ", "
            "display"          " " << db.getDbNativeTypeName<bool>()        << ", "
            "step_time_mins"   " " << db.getDbNativeTypeName<double>()      << ", "
            "end_temp_c"       " " << db.getDbNativeTypeName<double>()      << ", "
            "ramp_time_mins"   " " << db.getDbNativeTypeName<double>()      << ", "
            "step_number"      " " << db.getDbNativeTypeName<int>()         << ", "
            "boil_id"          " " << db.getDbNativeTypeName<int>()         << ", "
            "description"      " " << db.getDbNativeTypeName<QString>()     << ", "
            "start_acidity_ph" " " << db.getDbNativeTypeName<double>()      << ", "
            "end_acidity_ph"   " " << db.getDbNativeTypeName<double>()      << ", "
            "start_temp_c"     " " << db.getDbNativeTypeName<double>()      << ", "
            "start_gravity_sg" " " << db.getDbNativeTypeName<double>()      << ", "
            "end_gravity_sg"   " " << db.getDbNativeTypeName<double>()      << ", "
            "chilling_type"    " " << db.getDbNativeTypeName<QString>()     << ", "
            "FOREIGN KEY(boil_id) REFERENCES boil(id)" <<
         ");";

      QString createFermentationSql;
      QTextStream createFermentationSqlStream(&createFermentationSql);
      createFermentationSqlStream <<
         "CREATE TABLE fermentation ("
            "id"              " " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "name"            " " << db.getDbNativeTypeName<QString>()     << ", "
            "deleted"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "display"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "folder"          " " << db.getDbNativeTypeName<QString>()     << ", "
            "description"     " " << db.getDbNativeTypeName<QString>()     << ", "
            "notes"           " " << db.getDbNativeTypeName<QString>()     << ", "
            "temp_recipe_id"  " " << db.getDbNativeTypeName<int>()         <<
         ");";

      // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
      //     should not be stored in the DB or serialised.  See comment in model/Step.h.
      QString createFermentationStepSql;
      QTextStream createFermentationStepSqlStream(&createFermentationStepSql);
      createFermentationStepSqlStream <<
         "CREATE TABLE fermentation_step ("
            "id"               " " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "name"             " " << db.getDbNativeTypeName<QString>()     << ", "
            "deleted"          " " << db.getDbNativeTypeName<bool>()        << ", "
            "display"          " " << db.getDbNativeTypeName<bool>()        << ", "
            "step_time_mins"   " " << db.getDbNativeTypeName<double>()      << ", "
            "end_temp_c"       " " << db.getDbNativeTypeName<double>()      << ", "
            "step_number"      " " << db.getDbNativeTypeName<int>()         << ", "
            "fermentation_id"  " " << db.getDbNativeTypeName<int>()         << ", "
            "description"      " " << db.getDbNativeTypeName<QString>()     << ", "
            "start_acidity_ph" " " << db.getDbNativeTypeName<double>()      << ", "
            "end_acidity_ph"   " " << db.getDbNativeTypeName<double>()      << ", "
            "start_temp_c"     " " << db.getDbNativeTypeName<double>()      << ", "
            "start_gravity_sg" " " << db.getDbNativeTypeName<double>()      << ", "
            "end_gravity_sg"   " " << db.getDbNativeTypeName<double>()      << ", "
            "free_rise"        " " << db.getDbNativeTypeName<bool>()        << ", "
            "vessel"           " " << db.getDbNativeTypeName<QString>()     << ", "
            "FOREIGN KEY(fermentation_id) REFERENCES fermentation(id)" <<
         ");";

      QVector<QueryAndParameters> const migrationQueries{
         //
         // There was a bug in an old version of the code that meant inventory_id got stored as a decimal instead of
         // an integer.
         //
         {QString("UPDATE hop         SET inventory_id = CAST(inventory_id AS int) WHERE inventory_id IS NOT null")},
         {QString("UPDATE fermentable SET inventory_id = CAST(inventory_id AS int) WHERE inventory_id IS NOT null")},
         {QString("UPDATE misc        SET inventory_id = CAST(inventory_id AS int) WHERE inventory_id IS NOT null")},
         {QString("UPDATE yeast       SET inventory_id = CAST(inventory_id AS int) WHERE inventory_id IS NOT null")},
         //
         // For historical reasons, some people have a lot of indexes in their database, others do not.  Where they
         // relate to columns we are getting rid of we need to drop them if present.  Fortunately, the syntax for doing
         // this is the same for SQLite and PostgreSQL.
         //
         // We actually go a bit further and drop some indexes on columns we aren't getting rid of.  This is because the
         // indexes serve little purpose.  We load all the data from the DB into memory at start-up and then access rows
         // by primary key to make amendments etc.
         //
         // NOTE: we cannot drop indexes beginning "sqlite_autoindex_" as we would get an error "index associated with
         //       UNIQUE or PRIMARY KEY constraint cannot be dropped".
         //
         {QString("DROP INDEX IF EXISTS bt_hop_hop_id                       ")},
         {QString("DROP INDEX IF EXISTS hop_children_parent_id              ")},
         {QString("DROP INDEX IF EXISTS hop_in_recipe_recipe_id             ")},
         {QString("DROP INDEX IF EXISTS hop_in_recipe_hop_id                ")},
         {QString("DROP INDEX IF EXISTS instruction_in_recipe_recipe_id     ")},
         {QString("DROP INDEX IF EXISTS instruction_in_recipe_instruction_id")},
         {QString("DROP INDEX IF EXISTS equipment_children_parent_id        ")},
         {QString("DROP INDEX IF EXISTS misc_inventory_id                   ")},
         {QString("DROP INDEX IF EXISTS misc_children_parent_id             ")},
         {QString("DROP INDEX IF EXISTS misc_in_recipe_recipe_id            ")},
         {QString("DROP INDEX IF EXISTS misc_in_recipe_misc_id              ")},
         {QString("DROP INDEX IF EXISTS brewnote_recipe_id                  ")},
         {QString("DROP INDEX IF EXISTS bt_equipment_equipment_id           ")},
         {QString("DROP INDEX IF EXISTS bt_fermentable_fermentable_id       ")},
         {QString("DROP INDEX IF EXISTS bt_misc_misc_id                     ")},
         {QString("DROP INDEX IF EXISTS bt_style_style_id                   ")},
         {QString("DROP INDEX IF EXISTS bt_water_water_id                   ")},
         {QString("DROP INDEX IF EXISTS bt_yeast_yeast_id                   ")},
         {QString("DROP INDEX IF EXISTS fermentable_inventory_id            ")},
         {QString("DROP INDEX IF EXISTS fermentable_children_parent_id      ")},
         {QString("DROP INDEX IF EXISTS fermentable_in_recipe_recipe_id     ")},
         {QString("DROP INDEX IF EXISTS fermentable_in_recipe_fermentable_id")},
         {QString("DROP INDEX IF EXISTS hop_inventory_id                    ")},
         {QString("DROP INDEX IF EXISTS mashstep_mash_id                    ")},
         {QString("DROP INDEX IF EXISTS recipe_equipment_id                 ")},
         {QString("DROP INDEX IF EXISTS recipe_mash_id                      ")},
         {QString("DROP INDEX IF EXISTS recipe_style_id                     ")},
         {QString("DROP INDEX IF EXISTS recipe_ancestor_id                  ")},
         {QString("DROP INDEX IF EXISTS recipe_children_parent_id           ")},
         {QString("DROP INDEX IF EXISTS salt_misc_id                        ")},
         {QString("DROP INDEX IF EXISTS salt_in_recipe_salt_id              ")},
         {QString("DROP INDEX IF EXISTS salt_in_recipe_recipe_id            ")},
         {QString("DROP INDEX IF EXISTS style_children_parent_id            ")},
         {QString("DROP INDEX IF EXISTS water_children_parent_id            ")},
         {QString("DROP INDEX IF EXISTS water_in_recipe_recipe_id           ")},
         {QString("DROP INDEX IF EXISTS water_in_recipe_water_id            ")},
         {QString("DROP INDEX IF EXISTS yeast_inventory_id                  ")},
         {QString("DROP INDEX IF EXISTS yeast_children_parent_id            ")},
         {QString("DROP INDEX IF EXISTS yeast_in_recipe_recipe_id           ")},
         {QString("DROP INDEX IF EXISTS yeast_in_recipe_yeast_id            ")},
         //
         // Salt::Type is currently stored as a raw number.  We convert it to a string to bring it into line with other
         // enums.  Current values are:
         //     0 == NONE
         //     1 == CACL2
         //     2 == CACO3
         //     3 == CASO4
         //     4 == MGSO4
         //     5 == NACL
         //     6 == NAHCO3
         //     7 == LACTIC
         //     8 == H3PO4
         //     9 == ACIDMLT
         //    10 == numTypes
         //
         {QString("ALTER TABLE salt RENAME COLUMN stype TO numeric_type")},
         {QString("ALTER TABLE salt    ADD COLUMN stype %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("UPDATE salt "
                  "SET stype = "
                  "CASE "
                     "WHEN numeric_type = 1 THEN 'CaCl2'          "
                     "WHEN numeric_type = 2 THEN 'CaCO3'          "
                     "WHEN numeric_type = 3 THEN 'CaSO4'          "
                     "WHEN numeric_type = 4 THEN 'MgSO4'          "
                     "WHEN numeric_type = 5 THEN 'NaCl'           "
                     "WHEN numeric_type = 6 THEN 'NaHCO3'         "
                     "WHEN numeric_type = 7 THEN 'LacticAcid'     "
                     "WHEN numeric_type = 8 THEN 'H3PO4'          "
                     "WHEN numeric_type = 9 THEN 'AcidulatedMalt' "
                  "END")},
         {QString("ALTER TABLE salt DROP COLUMN numeric_type")},
         //
         // Hop: Extended and additional fields for BeerJSON
         //
         // We only need to update the old Hop type and form mappings.  The new ones should "just work".
         {QString(     "UPDATE hop SET htype = 'aroma'           WHERE htype = 'Aroma'"    )},
         {QString(     "UPDATE hop SET htype = 'bittering'       WHERE htype = 'Bittering'")},
         {QString(     "UPDATE hop SET htype = 'aroma/bittering' WHERE htype = 'Both'"     )},
         {QString(     "UPDATE hop SET form = 'pellet' WHERE form = 'Pellet'")},
         {QString(     "UPDATE hop SET form = 'plug'   WHERE form = 'Plug'"  )},
         {QString(     "UPDATE hop SET form = 'leaf'   WHERE form = 'Leaf'"  )},
         {QString("ALTER TABLE hop ADD COLUMN producer              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE hop ADD COLUMN product_id            %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE hop ADD COLUMN year                  %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE hop ADD COLUMN total_oil_ml_per_100g %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN farnesene_pct         %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN geraniol_pct          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN b_pinene_pct          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN linalool_pct          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN limonene_pct          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN nerol_pct             %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN pinene_pct            %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN polyphenols_pct       %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop ADD COLUMN xanthohumol_pct       %1").arg(db.getDbNativeTypeName<double >())},
         //
         // Fermentable: Extended and additional fields for BeerJSON
         //
         // We only need to update the old Fermentable type mappings.  The new ones should "just work".
         {QString("     UPDATE fermentable SET ftype = 'grain'       WHERE ftype = 'Grain'"      )},
         {QString("     UPDATE fermentable SET ftype = 'sugar'       WHERE ftype = 'Sugar'"      )},
         {QString("     UPDATE fermentable SET ftype = 'extract'     WHERE ftype = 'Extract'"    )},
         {QString("     UPDATE fermentable SET ftype = 'dry extract' WHERE ftype = 'Dry Extract'")},
         {QString("     UPDATE fermentable SET ftype = 'other'       WHERE ftype = 'Adjunct'"    )},
         {QString("ALTER TABLE fermentable ADD COLUMN grain_group                    %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable ADD COLUMN producer                       %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable ADD COLUMN product_id                     %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable RENAME COLUMN yield TO fine_grind_yield_pct")},
         {QString("ALTER TABLE fermentable ADD COLUMN coarse_grind_yield_pct         %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN potential_yield_sg             %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN alpha_amylase_dext_units       %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN kolbach_index_pct              %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN hardness_prp_glassy_pct        %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN hardness_prp_half_pct          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN hardness_prp_mealy_pct         %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN kernel_size_prp_plump_pct      %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN kernel_size_prp_thin_pct       %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN friability_pct                 %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN di_ph                          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN viscosity_cp                   %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN dmsp_ppm                       %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN fan_ppm                        %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN fermentability_pct             %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable ADD COLUMN beta_glucan_ppm                %1").arg(db.getDbNativeTypeName<double >())},
         // Also on Fermentable, diastaticPower_lintner is now optional (as it should have been all along) and we convert
         // 0 values to NULL
         {QString("     UPDATE fermentable SET diastatic_power = NULL where diastatic_power = 0.0")},
         //
         // Misc: Extended and additional fields for BeerJSON
         //
         // We only need to update the old Misc type mappings.  The new ones should "just work".
         {QString("     UPDATE misc SET mtype = 'spice'       WHERE mtype = 'Spice'      ")},
         {QString("     UPDATE misc SET mtype = 'fining'      WHERE mtype = 'Fining'     ")},
         {QString("     UPDATE misc SET mtype = 'water agent' WHERE mtype = 'Water Agent'")},
         {QString("     UPDATE misc SET mtype = 'herb'        WHERE mtype = 'Herb'       ")},
         {QString("     UPDATE misc SET mtype = 'flavor'      WHERE mtype = 'Flavor'     ")},
         {QString("     UPDATE misc SET mtype = 'other'       WHERE mtype = 'Other'      ")},
         {QString("ALTER TABLE misc ADD COLUMN producer   %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE misc ADD COLUMN product_id %1").arg(db.getDbNativeTypeName<QString>())},
         //
         // Yeast: Extended and additional fields for BeerJSON
         //
         // We only need to update the old Yeast type, form and flocculation mappings.  The new ones ("very low",
         // "medium low", "medium high") should "just work".
         //
         // For "parent" yeast records, attenuation is replaced by attenuation_min_pct and attenuation_max_pct.  (For
         // "child" ones it moves to attenuation_pct on yeast_in_recipe, which is done below.)  Although it's unlikely
         // to be strictly correct, we set attenuation_min_pct and attenuation_max_pct both to hold the same value as
         // the old attenuation column, on the grounds that this is better than nothing, except in a case where the old
         // attenuation column holds 0.
         //
         {QString("     UPDATE yeast SET ytype = 'ale'       WHERE ytype = 'Ale'      ")},
         {QString("     UPDATE yeast SET ytype = 'lager'     WHERE ytype = 'Lager'    ")},
         {QString("     UPDATE yeast SET ytype = 'other'     WHERE ytype = 'Wheat'    ")}, // NB: Wheat becomes Other
         {QString("     UPDATE yeast SET ytype = 'wine'      WHERE ytype = 'Wine'     ")},
         {QString("     UPDATE yeast SET ytype = 'champagne' WHERE ytype = 'Champagne'")},
         {QString("     UPDATE yeast SET form = 'liquid'  WHERE form = 'Liquid' ")},
         {QString("     UPDATE yeast SET form = 'dry'     WHERE form = 'Dry'    ")},
         {QString("     UPDATE yeast SET form = 'slant'   WHERE form = 'Slant'  ")},
         {QString("     UPDATE yeast SET form = 'culture' WHERE form = 'Culture'")},
         {QString("     UPDATE yeast SET flocculation = 'low'       WHERE flocculation = 'Low'      ")},
         {QString("     UPDATE yeast SET flocculation = 'medium'    WHERE flocculation = 'Medium'   ")},
         {QString("     UPDATE yeast SET flocculation = 'high'      WHERE flocculation = 'High'     ")},
         {QString("     UPDATE yeast SET flocculation = 'very high' WHERE flocculation = 'Very High'")},
         {QString("ALTER TABLE yeast ADD COLUMN alcohol_tolerance_pct        %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE yeast ADD COLUMN attenuation_min_pct          %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE yeast ADD COLUMN attenuation_max_pct          %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE yeast ADD COLUMN phenolic_off_flavor_positive %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN glucoamylase_positive        %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN killer_producing_k1_toxin    %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN killer_producing_k2_toxin    %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN killer_producing_k28_toxin   %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN killer_producing_klus_toxin  %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("ALTER TABLE yeast ADD COLUMN killer_neutral               %1").arg(db.getDbNativeTypeName<bool  >())},
         {QString("     UPDATE yeast SET attenuation_min_pct = attenuation WHERE attenuation != 0")},
         {QString("     UPDATE yeast SET attenuation_max_pct = attenuation WHERE attenuation != 0")},
         //
         // Style: Extended and additional fields for BeerJSON.  Plus fix inconsistent column name
         //
         {QString("ALTER TABLE style RENAME COLUMN s_type TO stype")},
         // We only need to update the old Style type mapping.  The new ones should "just work".
         // See comment in model/Style.h for more on the mapping here
         {QString("     UPDATE style SET stype = 'beer'  WHERE stype = 'Lager'")},
         {QString("     UPDATE style SET stype = 'beer'  WHERE stype = 'Ale'  ")},
         {QString("     UPDATE style SET stype = 'beer'  WHERE stype = 'Wheat'")},
         {QString("     UPDATE style SET stype = 'cider' WHERE stype = 'Cider'")},
         {QString("     UPDATE style SET stype = 'mead'  WHERE stype = 'Mead' ")},
         {QString("     UPDATE style SET stype = 'other' WHERE stype = 'Mixed'")},
         // Profile is split into Flavor and Aroma, so we rename Profile to Flavor before adding the other columns
         {QString("ALTER TABLE style RENAME COLUMN profile TO flavor")},
         {QString("ALTER TABLE style ADD COLUMN aroma              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE style ADD COLUMN appearance         %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE style ADD COLUMN mouthfeel          %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE style ADD COLUMN overall_impression %1").arg(db.getDbNativeTypeName<QString>())},
         //
         // Water: additional fields for BeerJSON
         //
         {QString("ALTER TABLE water ADD COLUMN carbonate_ppm %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN potassium_ppm %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN iron_ppm      %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN nitrate_ppm   %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN nitrite_ppm   %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN flouride_ppm  %1").arg(db.getDbNativeTypeName<double>())}, // Should have been fluoride_ppm!
         //
         // Equipment: Extended and additional fields for BeerJSON.  This includes changing a lot of column names as
         // BeerJSON essentially has a record per vessel ("HLT", "Mash Tun", etc)
         //
         {QString("ALTER TABLE equipment RENAME COLUMN notes             TO kettle_notes                 ")},
         {QString("ALTER TABLE equipment RENAME COLUMN real_evap_rate    TO kettle_evaporation_per_hour_l")},
         {QString("ALTER TABLE equipment RENAME COLUMN boil_size         TO kettle_boil_size_l           ")},
         {QString("ALTER TABLE equipment RENAME COLUMN tun_specific_heat TO mash_tun_specific_heat_calgc ")},
         {QString("ALTER TABLE equipment RENAME COLUMN tun_volume        TO mash_tun_volume_l            ")},
         {QString("ALTER TABLE equipment RENAME COLUMN tun_weight        TO mash_tun_weight_kg           ")},
         {QString("ALTER TABLE equipment RENAME COLUMN absorption        TO mash_tun_grain_absorption_lkg")},
         {QString("ALTER TABLE equipment RENAME COLUMN batch_size        TO fermenter_batch_size_l       ")},
         {QString("ALTER TABLE equipment RENAME COLUMN trub_chiller_loss TO kettle_trub_chiller_loss_l   ")},
         {QString("ALTER TABLE equipment RENAME COLUMN lauter_deadspace  TO lauter_tun_deadspace_loss_l  ")},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_type                       %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN mash_tun_type                  %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN lauter_tun_type                %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN kettle_type                    %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN fermenter_type                 %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN agingvessel_type               %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN packaging_vessel_type          %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_volume_l                   %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN lauter_tun_volume_l            %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN aging_vessel_volume_l          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN packaging_vessel_volume_l      %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_loss_l                     %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN mash_tun_loss_l                %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN fermenter_loss_l               %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN aging_vessel_loss_l            %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN packaging_vessel_loss_l        %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN kettle_outflow_per_minute_l    %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_weight_kg                  %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN lauter_tun_weight_kg           %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN kettle_weight_kg               %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_specific_heat_calgc        %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN lauter_tun_specific_heat_calgc %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN kettle_specific_heat_calgc     %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE equipment ADD COLUMN hlt_notes                      %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN mash_tun_notes                 %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN lauter_tun_notes               %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN fermenter_notes                %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN aging_vessel_notes             %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE equipment ADD COLUMN packaging_vessel_notes         %1").arg(db.getDbNativeTypeName<QString>())},
         //
         // MashStep
         //
         // Fix the table name for MashStep so it's consistent with most of the rest of our naming
         {QString("ALTER TABLE mashstep RENAME TO mash_step")},
         // We only need to update the old MashStep type mapping.  The new ones should "just work".
         {QString("     UPDATE mash_step SET mstype = 'infusion'       WHERE mstype = 'Infusion'   ")},
         {QString("     UPDATE mash_step SET mstype = 'temperature'    WHERE mstype = 'Temperature'")},
         {QString("     UPDATE mash_step SET mstype = 'decoction'      WHERE mstype = 'Decoction'  ")},
         {QString("     UPDATE mash_step SET mstype = 'sparge'         WHERE mstype = 'FlySparge'  ")},
         {QString("     UPDATE mash_step SET mstype = 'drain mash tun' WHERE mstype = 'BatchSparge'")},
         // The two different amount fields are unified.
         // Note that, per https://sqlite.org/changes.html, SQLite finally supports "DROP COLUMN" as of its
         // 2021-03-12 (3.35.0) release (and the teething troubles were ironed out by the 2021-04-19 (3.35.5) release!)
         {QString("ALTER TABLE mash_step RENAME COLUMN infuse_amount TO amount_l")},
         {QString("     UPDATE mash_step SET amount_l = decoction_amount WHERE mstype = 'Decoction'")},
         {QString("ALTER TABLE mash_step DROP COLUMN decoction_amount")},
         {QString("ALTER TABLE mash_step RENAME COLUMN ramp_time TO ramp_time_mins")},
         {QString("ALTER TABLE mash_step ADD COLUMN description               %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE mash_step ADD COLUMN liquor_to_grist_ratio_lkg %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE mash_step ADD COLUMN start_acidity_ph          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE mash_step ADD COLUMN end_acidity_ph            %1").arg(db.getDbNativeTypeName<double >())},
         // Now that we properly support optional fields, we can fix "zero means not set" on certain fields
         {QString("     UPDATE mash_step SET end_temp = NULL WHERE end_temp = 0")},
         // Give some other columns more consistent names
         {QString("ALTER TABLE mash_step RENAME COLUMN    end_temp TO    end_temp_c"   )},
         {QString("ALTER TABLE mash_step RENAME COLUMN infuse_temp TO infuse_temp_c"   )},
         {QString("ALTER TABLE mash_step RENAME COLUMN   step_temp TO   step_temp_c"   )},
         {QString("ALTER TABLE mash_step RENAME COLUMN   step_time TO   step_time_mins")},
         //
         // Recipe
         //
         // We only need to update the old Recipe type mapping.  The new ones should "just work".
         {QString("     UPDATE recipe SET type = 'extract'      WHERE type = 'Extract'     ")},
         {QString("     UPDATE recipe SET type = 'partial mash' WHERE type = 'Partial Mash'")},
         {QString("     UPDATE recipe SET type = 'all grain'    WHERE type = 'All Grain'   ")},
         {QString("ALTER TABLE recipe ADD COLUMN boil_id         %1 REFERENCES boil         (id)").arg(db.getDbNativeTypeName<int>())},
         {QString("ALTER TABLE recipe ADD COLUMN fermentation_id %1 REFERENCES fermentation (id)").arg(db.getDbNativeTypeName<int>())},
         {QString("ALTER TABLE recipe ADD COLUMN beer_acidity_ph          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE recipe ADD COLUMN apparent_attenuation_pct %1").arg(db.getDbNativeTypeName<double >())},
         //
         // We have to create and populate the boil and boil_step tables before we do hop_in_recipe as we need pre-boil
         // steps to attach first wort hops to.
         //
         // We also need to create and populate fermentation and fermentation_step tables.
         //
         // As noted above, we use a temporary column on the new tables to simplify populating them with data linked to
         // recipe.
         //
         {createBoilSql            },
         {createBoilStepSql        },
         {createFermentationSql    },
         {createFermentationStepSql},
         {QString("INSERT INTO boil ("
                      "name           , "
                      "deleted        , "
                      "display        , "
                      "folder         , "
                      "description    , "
                      "notes          , "
                      "pre_boil_size_l, "
                      "boil_time_mins , "
                      "temp_recipe_id   "
                  ") SELECT "
                     "'Boil for ' || name, "
                     "?, "
                     "?, "
                     "'', "
                     "'', "
                     "'', "
                     "boil_size, "
                     "boil_time, "
                     "id AS recipe_id "
                  "FROM recipe"
         ), {QVariant{false}, QVariant{true}}},

         {QString("INSERT INTO fermentation ("
                      "name           , "
                      "deleted        , "
                      "display        , "
                      "folder         , "
                      "description    , "
                      "notes          , "
                      "temp_recipe_id   "
                  ") SELECT "
                     "'Fermentation for ' || name, "
                     "?, "
                     "?, "
                     "'', "
                     "'', "
                     "'', "
                     "id AS recipe_id "
                  "FROM recipe"
         ), {QVariant{false}, QVariant{true}}},
         {QString("UPDATE recipe "
                  "SET boil_id = b.id "
                  "FROM ("
                     "SELECT id, "
                            "temp_recipe_id "
                     "FROM boil"
                  ") AS b "
                  "WHERE recipe.id = b.temp_recipe_id")},
         {QString("UPDATE recipe "
                  "SET fermentation_id = f.id "
                  "FROM ("
                     "SELECT id, "
                            "temp_recipe_id "
                     "FROM fermentation"
                  ") AS f "
                  "WHERE recipe.id = f.temp_recipe_id")},
         // Get rid of the temporary columns now that they have served their purpose.
         {QString("ALTER TABLE boil         DROP COLUMN temp_recipe_id")},
         {QString("ALTER TABLE fermentation DROP COLUMN temp_recipe_id")},
         //
         // Now we copied two recipe columns onto the boil table, we can drop them from the recipe table
         //
         {QString("ALTER TABLE recipe DROP COLUMN boil_size")},
         {QString("ALTER TABLE recipe DROP COLUMN boil_time")},
         //
         // Populate boil_steps.  We want to have a pre-boil step, a boil step, and a post-boil step as it makes the hop
         // addition stuff easier.
         //
         // The default names here are hard-coded in English, which isn't ideal (mea culpa) but this is only a one-off
         // data migration.  When the main code needs to add a new BoilStep, it does the right thing and uses tr().
         //
         // For the pre-boil step, ie ramping up from mash temperature to boil temperature, we take the end temperature
         // of the last mash step as the starting point.  This will be mash_step.end_temp_c IF SET, and
         // mash_step.step_temp_c otherwise.
         //
         // Note that, because mash_id is stored in both the mash_step and recipe tables, we don't actually have to look
         // at the mash table here.
         //
         // The PARTITION BY stuff below is a SQL window function that helps us get the max mash step number for each
         // mash ID.  As often with SQL, there are several ways to achieve this result.  The small size of our data sets
         // means we're not too anxious about performance so we prefer clarity (to the extent that's possible with
         // SQL!).
         //
         {QString("INSERT INTO boil_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "ramp_time_mins  , "
                     "step_number     , "
                     "boil_id         , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "chilling_type     "
                  ") SELECT "
                     "'Pre-boil for ' || recipe.name, "                              // name
                     "?, "                                                           // deleted
                     "?, "                                                           // display
                     "NULL, "                                                        // step_time_mins
                     "100.0, "                                                       // end_temp_c
                     "NULL, "                                                        // ramp_time_mins
                     "1, "                                                           // step_number
                     "recipe.boil_id, "                                              // boil_id
                     "'Automatically-generated pre-boil step for ' || recipe.name, " // description
                     "NULL, "                                                        // start_acidity_ph
                     "NULL, "                                                        // end_acidity_ph
                     "last_mash_step.temperature, "                                  // start_temp_c
                     "NULL, "                                                        // start_gravity_sg
                     "NULL, "                                                        // end_gravity_sg
                     "NULL "                                                         // chilling_type
                  "FROM recipe, "
                       "("
                          "SELECT mash_id, "
                                 "step_temp_c, "
                                 "end_temp_c, "
                                 "step_number, "
                                 "IIF(step_temp_c < end_temp_c, step_temp_c, end_temp_c) AS temperature, "
                                 "ROW_NUMBER() OVER ("
                                    "PARTITION BY mash_id "
                                    "ORDER BY step_number DESC"
                                 ") reversed_step_number "
                          "FROM mash_step "
                       ") AS last_mash_step "
                  "WHERE reversed_step_number = 1 "
                  "AND recipe.mash_id = last_mash_step.mash_id"
         ), {QVariant{false}, QVariant{true}}},
         //
         // But wait, we're not done on pre-boil step.  We also need to handle the case where a recipe has a mash that
         // does not have any mash steps.  Eg the supplied "Extract" recipes are like this.
         //
         // It may be there is a way to combine this with the SQL query above, but I think it's simpler not to and just
         // live with some horrible copy-and-paste here in a query that just creates a (hopefully sane) pre-boil step
         // for any recipes that don't have one.  We make a heroic assumption that the start temperature is 15°C (ie
         // about 60 °F) which is about what you might expect tap water temperature to be a lot of the time in a lot of
         // places.
         //
         {QString("INSERT INTO boil_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "ramp_time_mins  , "
                     "step_number     , "
                     "boil_id         , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "chilling_type     "
                  ") SELECT "
                     "'Pre-boil for ' || recipe.name, "                              // name
                     "?, "                                                           // deleted
                     "?, "                                                           // display
                     "NULL, "                                                        // step_time_mins
                     "100.0, "                                                       // end_temp_c
                     "NULL, "                                                        // ramp_time_mins
                     "1, "                                                           // step_number
                     "recipe.boil_id, "                                              // boil_id
                     "'Automatically-generated pre-boil step for ' || recipe.name, " // description
                     "NULL, "                                                        // start_acidity_ph
                     "NULL, "                                                        // end_acidity_ph
                     "15, "                                                          // start_temp_c
                     "NULL, "                                                        // start_gravity_sg
                     "NULL, "                                                        // end_gravity_sg
                     "NULL "                                                         // chilling_type
                  "FROM recipe "
                  "WHERE recipe.boil_id in ( "
                     "SELECT boil_id "
                     "FROM boil "
                     "WHERE boil_id NOT IN ( "
                        "SELECT boil_id "
                        "FROM boil_step "
                        "WHERE step_number = 1 "
                     ")"
                  ")"
         ), {QVariant{false}, QVariant{true}}},
         // Adding the second step for the actual boil itself is easier
         {QString("INSERT INTO boil_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "ramp_time_mins  , "
                     "step_number     , "
                     "boil_id         , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "chilling_type     "
                  ") SELECT "
                     "'Boil proper for ' || recipe.name, "                              // name
                     "?, "                                                              // deleted
                     "?, "                                                              // display
                     "NULL, "                                                           // step_time_mins
                     "100.0, "                                                          // end_temp_c
                     "NULL, "                                                           // ramp_time_mins
                     "2, "                                                              // step_number
                     "recipe.boil_id, "                                                 // boil_id
                     "'Automatically-generated boil proper step for ' || recipe.name, " // description
                     "NULL, "                                                           // start_acidity_ph
                     "NULL, "                                                           // end_acidity_ph
                     "100.0, "                                                          // start_temp_c
                     "NULL, "                                                           // start_gravity_sg
                     "NULL, "                                                           // end_gravity_sg
                     "NULL "                                                            // chilling_type
                  "FROM recipe"
         ), {QVariant{false}, QVariant{true}}},
         // For the post-boil step, we'll assume we are cooling to primary fermentation temperature, if known (ie it's
         // non-zero), or to 30°C otherwise.
         {QString("INSERT INTO boil_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "ramp_time_mins  , "
                     "step_number     , "
                     "boil_id         , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "chilling_type     "
                  ") SELECT "
                     "'Wort cooling for ' || recipe.name, "                            // name
                     "?, "                                                            // deleted
                     "?, "                                                            // display
                     "NULL, "                                                         // step_time_mins
                     "IIF(recipe.primary_temp > 0.0, recipe.primary_temp, 30.0), "    // end_temp_c
                     "NULL, "                                                         // ramp_time_mins
                     "3, "                                                            // step_number
                     "recipe.boil_id, "                                               // boil_id
                     "'Automatically-generated post-boil step for ' || recipe.name, " // description
                     "NULL, "                                                         // start_acidity_ph
                     "NULL, "                                                         // end_acidity_ph
                     "100.0, "                                                        // start_temp_c
                     "NULL, "                                                         // start_gravity_sg
                     "NULL, "                                                         // end_gravity_sg
                     "NULL "                                                          // chilling_type
                  "FROM recipe"
         ), {QVariant{false}, QVariant{true}}},
         //
         // Populate fermentation_steps.  NB that fermentation steps do not have a ramp time.  (See comment in
         // model/Step.h.)
         //
         // Note that primary_age, secondary_age, tertiary_age (which we can safely assume are not NULL as we are only
         // introducing optional fields with the BeerJSON work) are in days, but our canonical unit of time is minutes.
         //
         // We assume everything has a primary fermentation.
         //
         {QString("INSERT INTO fermentation_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "step_number     , "
                     "fermentation_id , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "free_rise       , "
                     "vessel            "
                  ") SELECT "
                     "'Primary fermentation for ' || recipe.name, "                              // name
                     "?,    "                                                                    // deleted
                     "?,    "                                                                    // display
                     "recipe.primary_age * 60 * 24, "                                            // step_time_mins
                     "recipe.primary_temp   , "                                                  // end_temp_c
                     "1,    "                                                                    // step_number
                     "recipe.fermentation_id, "                                                  // fermentation_id
                     "'Automatically-generated primary fermentation step for ' || recipe.name, " // description
                     "NULL, "                                                                    // start_acidity_ph
                     "NULL, "                                                                    // end_acidity_ph
                     "recipe.primary_temp   , "                                                  // start_temp_c
                     "NULL, "                                                                    // start_gravity_sg
                     "NULL, "                                                                    // end_gravity_sg
                     "NULL, "                                                                    // free_rise
                     "''    "                                                                    // vessel
                  "FROM recipe "
         ), {QVariant{false}, QVariant{true}}},
         // Secondary fermentation is only valid if its age is more than 0 days.
         {QString("INSERT INTO fermentation_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "step_number     , "
                     "fermentation_id , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "free_rise       , "
                     "vessel            "
                  ") SELECT "
                     "'Secondary fermentation for ' || recipe.name, "                              // name
                     "?,    "                                                                      // deleted
                     "?,    "                                                                      // display
                     "recipe.secondary_age * 60 * 24,"                                             // step_time_mins
                     "recipe.secondary_temp , "                                                    // end_temp_c
                     "2,    "                                                                      // step_number
                     "recipe.fermentation_id, "                                                    // fermentation_id
                     "'Automatically-generated secondary fermentation step for ' || recipe.name, " // description
                     "NULL, "                                                                      // start_acidity_ph
                     "NULL, "                                                                      // end_acidity_ph
                     "recipe.secondary_temp , "                                                    // start_temp_c
                     "NULL, "                                                                      // start_gravity_sg
                     "NULL, "                                                                      // end_gravity_sg
                     "NULL, "                                                                      // free_rise
                     "''    "                                                                      // vessel
                  "FROM recipe "
                  "WHERE recipe.secondary_age > 0 "
         ), {QVariant{false}, QVariant{true}}},
         // Tertiary fermentation is only valid if its age is more than 0 days AND if there was a secondary
         // fermentation.
         {QString("INSERT INTO fermentation_step ("
                     "name            , "
                     "deleted         , "
                     "display         , "
                     "step_time_mins  , "
                     "end_temp_c      , "
                     "step_number     , "
                     "fermentation_id , "
                     "description     , "
                     "start_acidity_ph, "
                     "end_acidity_ph  , "
                     "start_temp_c    , "
                     "start_gravity_sg, "
                     "end_gravity_sg  , "
                     "free_rise       , "
                     "vessel            "
                  ") SELECT "
                     "'Tertiary fermentation for ' || recipe.name, "                               // name
                     "?,    "                                                                     // deleted
                     "?,    "                                                                     // display
                     "recipe.tertiary_age * 60 * 24,"                                             // step_time_mins
                     "recipe.tertiary_temp , "                                                    // end_temp_c
                     "3,    "                                                                     // step_number
                     "recipe.fermentation_id, "                                                   // fermentation_id
                     "'Automatically-generated tertiary fermentation step for ' || recipe.name, " // description
                     "NULL, "                                                                     // start_acidity_ph
                     "NULL, "                                                                     // end_acidity_ph
                     "recipe.tertiary_temp , "                                                    // start_temp_c
                     "NULL, "                                                                     // start_gravity_sg
                     "NULL, "                                                                     // end_gravity_sg
                     "NULL, "                                                                     // free_rise
                     "''    "                                                                     // vessel
                  "FROM recipe "
                  "WHERE recipe.tertiary_age  > 0 "
                  "AND   recipe.secondary_age > 0 "
         ), {QVariant{false}, QVariant{true}}},
         //
         // Now we copied the data across, we don't need the primary/secondary/tertiary columns on recipe
         //
         {QString("ALTER TABLE recipe DROP COLUMN primary_age   ")},
         {QString("ALTER TABLE recipe DROP COLUMN primary_temp  ")},
         {QString("ALTER TABLE recipe DROP COLUMN secondary_age ")},
         {QString("ALTER TABLE recipe DROP COLUMN secondary_temp")},
         {QString("ALTER TABLE recipe DROP COLUMN tertiary_age  ")},
         {QString("ALTER TABLE recipe DROP COLUMN tertiary_temp ")},
         //
         // This field/column exists in our schema because it is part of BeerXML, but we don't expose it in the UI or
         // make any use of it internally.  So, for any recipes created in our software, its value will be meaningless.
         //
         // Going forward, Fermentation object knows the number of FermentationSteps it has, so a separate field is not
         // needed.
         //
         {QString("ALTER TABLE recipe DROP COLUMN fermentation_stages")},
         //
         // Now comes the tricky stuff where we change the hop_in_recipe, fermentable_in_recipe, misc_in_recipe,
         // yeast_in_recipe, salt_in_recipe and water_in_recipe junction tables to full-blown object tables, and remove
         // hop_children, fermentable_children, misc_children, yeast_children and water_children.  (NB: There is no
         // salt_children table!)  We do salt and water last (out of alphabetical order) because they are a bit
         // different from the other ingredients.  In particular, water additions don't have any timing info (because
         // that's in the mash / mash step data) and there is no inventory for water.
         //
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN name              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN display           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN deleted           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN quantity          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN unit              %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN stage             %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN step              %1").arg(db.getDbNativeTypeName<int    >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN add_at_time_mins  %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN add_at_gravity_sg %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN add_at_acidity_ph %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE hop_in_recipe ADD COLUMN duration_mins     %1").arg(db.getDbNativeTypeName<double >())},
         {QString("     UPDATE hop_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE hop_in_recipe SET deleted = ?"), {QVariant{false}}},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN name              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN display           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN deleted           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN quantity          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN unit              %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN stage             %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN step              %1").arg(db.getDbNativeTypeName<int    >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN add_at_time_mins  %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN add_at_gravity_sg %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN add_at_acidity_ph %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE fermentable_in_recipe ADD COLUMN duration_mins     %1").arg(db.getDbNativeTypeName<double >())},
         {QString("     UPDATE fermentable_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE fermentable_in_recipe SET deleted = ?"), {QVariant{false}}},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN name              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN display           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN deleted           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN quantity          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN unit              %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN stage             %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN step              %1").arg(db.getDbNativeTypeName<int    >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN add_at_time_mins  %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN add_at_gravity_sg %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN add_at_acidity_ph %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE misc_in_recipe ADD COLUMN duration_mins     %1").arg(db.getDbNativeTypeName<double >())},
         {QString("     UPDATE misc_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE misc_in_recipe SET deleted = ?"), {QVariant{false}}},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN name                %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN display             %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN deleted             %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN quantity            %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN unit                %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN stage               %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN step                %1").arg(db.getDbNativeTypeName<int    >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN add_at_time_mins    %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN add_at_gravity_sg   %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN add_at_acidity_ph   %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN duration_mins       %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN attenuation_pct     %1").arg(db.getDbNativeTypeName<double >())}, // NB: Extra column for yeast_in_recipe
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN times_cultured      %1").arg(db.getDbNativeTypeName<int    >())}, // NB: Extra column for yeast_in_recipe
         {QString("ALTER TABLE yeast_in_recipe ADD COLUMN cell_count_billions %1").arg(db.getDbNativeTypeName<int    >())}, // NB: Extra column for yeast_in_recipe
         {QString("     UPDATE yeast_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE yeast_in_recipe SET deleted = ?"), {QVariant{false}}},
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN name              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN display           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN deleted           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN quantity          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN unit              %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("ALTER TABLE salt_in_recipe ADD COLUMN when_to_add       %1").arg(db.getDbNativeTypeName<QString>())}, // Enums are stored as strings
         {QString("     UPDATE salt_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE salt_in_recipe SET deleted = ?"), {QVariant{false}}},
         {QString("ALTER TABLE water_in_recipe ADD COLUMN name              %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE water_in_recipe ADD COLUMN display           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE water_in_recipe ADD COLUMN deleted           %1").arg(db.getDbNativeTypeName<bool   >())},
         {QString("ALTER TABLE water_in_recipe ADD COLUMN volume_l          %1").arg(db.getDbNativeTypeName<double >())},
         {QString("     UPDATE water_in_recipe SET display = ?"), {QVariant{true}}},
         {QString("     UPDATE water_in_recipe SET deleted = ?"), {QVariant{false}}},

         //
         // Bring the amounts across from the hop and fermentable tables.  At the outset, all amounts are going to be
         // weights, because the previous schemas did not support volumes for hop or fermentable additions.
         //
         // Although we mostly try to avoid it, we are using non-standard UPDATE FROM syntax here (see
         // https://www.sqlite.org/lang_update.html#update_from).  Fortunately, SQLite follows PostgreSQL for this, so
         // the same query should work on both databases.
         //
         // (See Measurement::Units::unitStringMapping for mapping of "kilograms" to Measurement::Units::kilograms etc.)
         //
         // It's not strictly needed, but we'll give obvious ("Addition of...") names to the hop/fermentable/etc
         // additions at the same time because it makes the DB easier to browse.  I guess in a perfect world we should
         // translate these but, for now at least, the name of the addition object (ie the RecipeAdditionHop etc object)
         // is not shown in the UI, so it's not a big deal that there's an English name in the DB.
         //
         {QString("UPDATE hop_in_recipe "
                  "SET quantity = h.amount, "
                      "unit = 'kilograms', "
                      "name = 'Addition of ' || h.name "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount "
                     "FROM hop"
                  ") AS h "
                  "WHERE hop_in_recipe.hop_id = h.id")},
         {QString("UPDATE fermentable_in_recipe "
                  "SET quantity = f.amount, "
                      "unit = 'kilograms', "
                      "name = 'Addition of ' || f.name "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount "
                     "FROM fermentable"
                  ") AS f "
                  "WHERE fermentable_in_recipe.fermentable_id = f.id")},
         //
         // Now do the same for misc and yeast tables.  Here, the existing schema _does_ support weight and volume, so
         // we have to account for that.
         //
         // We also bring across yeast.times_cultured to yeast_in_recipe.times_cultured, as that's where it now lives.
         //
         // TBD: How do "quanta" (ie number of packets) of yeast get stored in DB?
         //
         {QString("UPDATE misc_in_recipe "
                  "SET quantity = m.amount, "
                      "unit = m.unit, "
                      "name = 'Addition of ' || m.name "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount, "
                            "CASE WHEN amount_is_weight THEN 'kilograms' ELSE 'liters' END AS unit "
                     "FROM misc"
                  ") AS m "
                  "WHERE misc_in_recipe.misc_id = m.id")},
         {QString("UPDATE yeast_in_recipe "
                  "SET quantity = y.amount, "
                      "unit = y.unit, "
                      "name = 'Addition of ' || y.name, "
                      "times_cultured = y.times_cultured "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount, "
                            "CASE WHEN amount_is_weight THEN 'kilograms' ELSE 'liters' END AS unit, "
                            "times_cultured "
                     "FROM yeast"
                  ") AS y "
                  "WHERE yeast_in_recipe.yeast_id = y.id")},
         //
         // Salt is similar to misc and yeast, except we have the possibility of "WhenToAdd == Never" (addTo == 0) which
         // means don't add the salt at all(!)
         //
         // It's convenient to bring the WhenToAdd property across at the same time as the quantity.  It's stored
         // numerically in the salt.addTo column:
         //    0 == Never  == Do not add at all
         //    1 == Mash   == Add at start of mash
         //    2 == Sparge == Add to sparge water (at end of mash)
         //    3 == Ratio  == Add at mash and sparge, pro rata to the amounts of water (I think!)
         //    4 == Equal  == Add at mash and sparge, equal amounts (I think!)
         //
         // We skip the "Never" value (see below) and store in when_to_add as a string.
         //
         {QString("UPDATE salt_in_recipe "
                  "SET quantity    = s.amount, "
                      "unit        = s.unit, "
                      "name = 'Addition of ' || s.name, "
                      "when_to_add = s.when_to_add "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount, "
                            "addTo, "
                            "CASE "
                               "WHEN amount_is_weight THEN 'kilograms' "
                               "ELSE 'liters' "
                            "END AS unit, "
                            "CASE "
                               "WHEN addTo = 1 THEN 'Mash'   "
                               "WHEN addTo = 2 THEN 'Sparge' "
                               "WHEN addTo = 3 THEN 'Ratio'  "
                               "WHEN addTo = 4 THEN 'Equal'  "
                            "END AS when_to_add "
                     "FROM salt"
                  ") AS s "
                  "WHERE salt_in_recipe.salt_id = s.id "
                  "AND s.addTo != 0")},
         //
         // Now we have to do something with the salts marked as "do not add".  Since salt is not mentioned in either
         // BeerXML or BeerJSON, we don't have a lot of guidance here.  AFAICT the "Never" value was just a convenience
         // in the UI for when you created a salt addition but hadn't yet set all its properties.  I _think_ we could
         // safely just delete Salts with addTo == 0.  However, rather than risk losing data, we will instead mark them
         // as deleted.  We have to set some valid value for when_to_add, so we arbitrarily choose "Equal", but make
         // clear in the Name that original value was "Never".
         //
         {QString("UPDATE salt_in_recipe "
                  "SET quantity    = s.amount, "
                      "unit        = s.unit, "
                      "name = 'Deleted addition of \"Never add\" ' || s.name, "
                      "display = ?, "
                      "deleted = ?, "
                      "when_to_add = 'Equal' "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount, "
                            "addTo, "
                            "CASE "
                               "WHEN amount_is_weight THEN 'kilograms' "
                               "ELSE 'liters' "
                            "END AS unit "
                     "FROM salt"
                  ") AS s "
                  "WHERE salt_in_recipe.salt_id = s.id "
                  "AND s.addTo == 0"), {QVariant{false}, QVariant{true}}},
         //
         // For water both the source and target are volume in liters
         //
         {QString("UPDATE water_in_recipe "
                  "SET volume_l = w.amount, "
                      "name = 'Use of ' || w.name "
                  "FROM ("
                     "SELECT id, "
                            "name, "
                            "amount "
                     "FROM water"
                  ") AS w "
                  "WHERE water_in_recipe.water_id = w.id")},
         //
         // Now we brought the amounts across, we can drop them on the hop, fermentable, misc, yeast, salt and water
         // tables.
         //
         // NB: Do NOT drop the amount_is_weight columns yet!  We need them below to update inventory.
         //
         // Technically we are losing some data here, because we lose the amount field for "parent" hops/fermentables
         // (ie those rows that do not correspond to "use of hop/fermentable in a recipe".  However, this is meaningless
         // data, which is why it isn't in the new schema, and the user has a backup of the old DB, so it should be OK.
         // (Note that inventory amounts are stored in different tables - hop_in_inventory, fermentable_in_inventory.)
         //
         {QString("ALTER TABLE hop         DROP COLUMN amount")},
         {QString("ALTER TABLE fermentable DROP COLUMN amount")},
         {QString("ALTER TABLE misc        DROP COLUMN amount")},
         {QString("ALTER TABLE salt        DROP COLUMN amount")},
         {QString("ALTER TABLE yeast       DROP COLUMN amount")},
         {QString("ALTER TABLE water       DROP COLUMN amount")},
         // Also drop the other column on yeast that we brought across
         {QString("ALTER TABLE yeast       DROP COLUMN times_cultured")},
         //
         // Bring the addition times across from the hop and misc tables.  Do this before setting stage etc, as it's
         // similar to the queries we've just done.
         //
         {QString("UPDATE hop_in_recipe "
                  "SET add_at_time_mins = h.time "
                  "FROM ("
                     "SELECT id, "
                            "time "
                     "FROM hop"
                  ") AS h "
                  "WHERE hop_in_recipe.hop_id = h.id")},
         {QString("UPDATE misc_in_recipe "
                  "SET add_at_time_mins = m.time "
                  "FROM ("
                     "SELECT id, "
                            "time "
                     "FROM misc"
                  ") AS m "
                  "WHERE misc_in_recipe.misc_id = m.id")},
         //
         // Existing data doesn't have an addition time for fermentable or yeast to 0 for both of them.  (Note that
         // salt_in_recipe and water_in_recipe do not have the add_at_time_mins column.)
         //
         {QString("UPDATE fermentable_in_recipe "
                  "SET add_at_time_mins = 0.0 ")},
         {QString("UPDATE yeast_in_recipe "
                  "SET add_at_time_mins = 0.0 ")},
         //
         // And, as above, drop the time column on the hop and misc tables now we pulled the data across.
         //
         {QString("ALTER TABLE hop  DROP COLUMN time")},
         {QString("ALTER TABLE misc DROP COLUMN time")},
         //
         // NB: No addition times or stages etc for water uses (as this is handled in mash steps etc).
         //
         // We need to map from old Hop::Use {Mash, First_Wort, Boil, Aroma, Dry_Hop} to new RecipeAddition::Stage
         // {Mash, Boil, Fermentation, Packaging}.  NB: The equivalent logic is also in RecipeAdditionHop::use() and
         // RecipeAdditionHop::setUse().
         //
         // Hop::Use::Mash -> RecipeAddition::Stage::Mash
         //
         {QString("UPDATE hop_in_recipe "
                  "SET stage = 'add_to_mash' "
                  "WHERE hop_id IN ("
                     "SELECT id "
                     "FROM hop "
                     "WHERE lower(hop.use) = 'mash'"
                  ")")},
         //
         // Hop::Use::First_Wort -> RecipeAddition::Stage::Boil + RecipeAddition::step = 1 (because we made sure above
         // that every boil has a pre-boil step
         //
         {QString("UPDATE hop_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 1 "
                  "WHERE hop_id IN ("
                     "SELECT id "
                     "FROM hop "
                     "WHERE lower(hop.use) = 'first wort'"
                  ")")},
         //
         // Hop::Use::Boil -> RecipeAddition::Stage::Boil + RecipeAddition::step = 2 (because we made sure above that
         // every boil has a "boil proper" step
         //
         {QString("UPDATE hop_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 2 "
                  "WHERE hop_id IN ("
                     "SELECT id "
                     "FROM hop "
                     "WHERE lower(hop.use) = 'boil'"
                  ")")},
         //
         // Hop::Use::Aroma -> RecipeAddition::Stage::Boil + RecipeAddition::step = 3 (because we made sure above that
         // every boil has a post-boil step
         //
         {QString("UPDATE hop_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 3 "
                  "WHERE hop_id IN ("
                     "SELECT id "
                     "FROM hop "
                     "WHERE lower(hop.use) = 'aroma'"
                  ")")},
         //
         // Hop::Use::Dry_Hop -> RecipeAddition::Stage::Fermentation
         //
         {QString("UPDATE hop_in_recipe "
                  "SET stage = 'add_to_fermentation' "
                  "WHERE hop_id IN ("
                     "SELECT id "
                     "FROM hop "
                     "WHERE lower(hop.use) = 'dry hop'"
                  ")")},
         //
         // Misc is similar to Hop, except that the old Misc::Use values are {Mash, Boil, Primary, Secondary, Bottling}.
         // Again, parallel logic is in RecipeAdditionMisc::use() and RecipeAdditionMisc::setUse().
         //
         //
         // Misc::Use::Mash -> RecipeAddition::Stage::Mash
         //
         {QString("UPDATE misc_in_recipe "
                  "SET stage = 'add_to_mash' "
                  "WHERE misc_id IN ("
                     "SELECT id "
                     "FROM misc "
                     "WHERE lower(misc.use) = 'mash'"
                  ")")},
         //
         // Misc::Use::Boil -> RecipeAddition::Stage::Boil + RecipeAddition::step = 2 (because we made sure above that
         // every boil has a "boil proper" step
         //
         {QString("UPDATE misc_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 2 "
                  "WHERE misc_id IN ("
                     "SELECT id "
                     "FROM misc "
                     "WHERE lower(misc.use) = 'boil'"
                  ")")},
         //
         // Misc::Use::Primary, Misc::Use::Secondary -> RecipeAddition::Stage::Fermentation
         //
         // TBD: Should we create two fermentation steps?
         //
         {QString("UPDATE misc_in_recipe "
                  "SET stage = 'add_to_fermentation' "
                  "WHERE misc_id IN ("
                     "SELECT id "
                     "FROM misc "
                     "WHERE lower(misc.use) = 'primary' "
                        "OR lower(misc.use) = 'secondary' "
                  ")")},
         //
         // Misc::Use::Bottling -> RecipeAddition::Stage::Packaging
         //
         {QString("UPDATE misc_in_recipe "
                  "SET stage = 'add_to_package' "
                  "WHERE misc_id IN ("
                     "SELECT id "
                     "FROM misc "
                     "WHERE lower(misc.use) = 'bottling'"
                  ")")},
         //
         // Now we pulled the info from the hop.use column into the hop_in_recipe table, we can drop the column.  Same
         // goes for misc.use into misc_in_recipe.
         //
         {QString("ALTER TABLE hop  DROP COLUMN use")},
         {QString("ALTER TABLE misc DROP COLUMN use")},
         //
         // Fermentable additions are a bit simpler.  They are either "is_mashed" or "add_after_boil" or neither.
         // (Obviously doesn't make sense to be both!)  In the case of neither (ie not mashed and not added after boil,
         // we assume added at the start of the boil).
         //
         // NOTE: For historical reasons, there are a lot of places in the database where a Boolean value could be
         //       stored as "true" / "false" rather than 1 / 0 (depending on the exact version of the software that a
         //       particular field was written with).  According to https://sqlite.org/datatype3.html:
         //
         //          SQLite does not have a separate Boolean storage class. Instead, Boolean values are stored as
         //          integers 0 (false) and 1 (true).
         //
         //          SQLite recognizes the keywords "TRUE" and "FALSE", as of version 3.23.0 (2018-04-02) but those
         //          keywords are really just alternative spellings for the integer literals 1 and 0 respectively.
         //
         //       So it should be OK to ignore this difference and trust SQLite to "do the right thing" when we have a
         //       Boolean value in a WHERE clause.
         //
         //       Note that we start by setting _everything_ to be added to the mash, then overwrite the boil cases
         //       afterwards.  This guarantees that every entry in fermentable_in_recipe has stage set -- even if there
         //       are odd null values in the is_mashed and/or add_after_boil columns of the fermentable table.
         //
         {QString("UPDATE fermentable_in_recipe "
                  "SET stage = 'add_to_mash' ")},
         {QString("UPDATE fermentable_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 1 "
                  "WHERE fermentable_id IN ("
                     "SELECT id "
                     "FROM fermentable "
                     "WHERE is_mashed = ? "
                     "AND add_after_boil = ?"
                  ")"), {QVariant{false}, QVariant{false}}},
         {QString("UPDATE fermentable_in_recipe "
                  "SET stage = 'add_to_boil', "
                      "step  = 3 "
                  "WHERE fermentable_id IN ("
                     "SELECT id "
                     "FROM fermentable "
                     "WHERE add_after_boil = ?"
                  ")"), {QVariant{true}}},
         //
         // We can drop the is_mashed and add_after_boil columns on fermentable as we just pulled the information from
         // them into the fermentable_in_recipe table.
         //
         {QString("ALTER TABLE fermentable DROP COLUMN is_mashed")},
         {QString("ALTER TABLE fermentable DROP COLUMN add_after_boil")},
         //
         // Yeast additions are either add_to_secondary or not (ie add to primary).  The BeerXML specifications explain
         // that add_to_secondary is a "flag denoting that this yeast was added for a secondary (or later) fermentation
         // as opposed to the primary fermentation.  Useful if one uses two or more yeast strains for a single brew (eg:
         // Lambic).  Default value is FALSE".
         //
         // In BeerJSON, the TimingType of an addition includes the "step" field, which is "used to indicate what step
         // this ingredient timing addition is referencing. EG A value of 2 for add_to_fermentation would mean to add
         // during the second fermentation step".
         //
         // Again, we start by setting _everything_ to be add-to-primary and then overwrite the add-to-secondary cases
         // afterwards, as this covers any null data in the add_to_secondary column of the yeast table.
         //
         {QString("UPDATE yeast_in_recipe "
                  "SET stage = 'add_to_fermentation', "
                      "step = 1 ")},
         {QString("UPDATE yeast_in_recipe "
                  "SET stage = 'add_to_fermentation', "
                      "step = 2 "
                  "WHERE yeast_id IN ("
                     "SELECT id "
                     "FROM yeast "
                     "WHERE add_to_secondary = ?"
                  ")"), {QVariant{true}}},
         //
         // We can drop the add_to_secondary column on yeast now that we have transferred the information from it across
         // to yeast_in_recipe.
         //
         {QString("ALTER TABLE yeast DROP COLUMN add_to_secondary")},
         //
         // For "child" yeast records, attenuation percent moves from being a Yeast property to a RecipeAdditionYeast
         // one.
         //
         {QString("UPDATE yeast_in_recipe "
                  "SET attenuation_pct = y.attenuation "
                  "FROM ("
                     "SELECT id, "
                            "attenuation "
                     "FROM yeast"
                  ") AS y "
                  "WHERE yeast_in_recipe.yeast_id = y.id")},
         // Now we moved the attenuation column across, we can drop it
         {QString("ALTER TABLE yeast DROP COLUMN attenuation")},

         //
         // Entries in hop_in_recipe will still be pointing to the "child" hop.  We need to point to the parent one.
         // Same for fermentable_in_recipe, misc_in_recipe, yeast_in_recipe and water_in_recipe.
         //
         {QString("UPDATE hop_in_recipe "
                  "SET hop_id = hc.parent_id "
                  "FROM ("
                     "SELECT parent_id, "
                            "child_id "
                     "FROM hop_children"
                  ") AS hc "
                  "WHERE hop_in_recipe.hop_id = hc.child_id")},
         {QString("UPDATE fermentable_in_recipe "
                  "SET fermentable_id = fc.parent_id "
                  "FROM ("
                     "SELECT parent_id, "
                            "child_id "
                     "FROM fermentable_children"
                  ") AS fc "
                  "WHERE fermentable_in_recipe.fermentable_id = fc.child_id")},
         {QString("UPDATE misc_in_recipe "
                  "SET misc_id = mc.parent_id "
                  "FROM ("
                     "SELECT parent_id, "
                            "child_id "
                     "FROM misc_children"
                  ") AS mc "
                  "WHERE misc_in_recipe.misc_id = mc.child_id")},
         {QString("UPDATE yeast_in_recipe "
                  "SET yeast_id = yc.parent_id "
                  "FROM ("
                     "SELECT parent_id, "
                            "child_id "
                     "FROM yeast_children"
                  ") AS yc "
                  "WHERE yeast_in_recipe.yeast_id = yc.child_id")},
         {QString("UPDATE water_in_recipe "
                  "SET water_id = wc.parent_id "
                  "FROM ("
                     "SELECT parent_id, "
                            "child_id "
                     "FROM water_children"
                  ") AS wc "
                  "WHERE water_in_recipe.water_id = wc.child_id")},
         //
         // Now we can mark the child hops, fermentables, miscs, yeasts, waters as deleted
         //
         {QString("UPDATE hop "
                  "SET deleted = ?, "
                      "display = ? "
                  "WHERE hop.id IN (SELECT child_id FROM hop_children)"), {QVariant{true}, QVariant{false}}},
         {QString("UPDATE fermentable "
                  "SET deleted = ?, "
                      "display = ? "
                  "WHERE fermentable.id IN (SELECT child_id FROM fermentable_children)"), {QVariant{true}, QVariant{false}}},
         {QString("UPDATE misc "
                  "SET deleted = ?, "
                      "display = ? "
                  "WHERE misc.id IN (SELECT child_id FROM misc_children)"), {QVariant{true}, QVariant{false}}},
         {QString("UPDATE yeast "
                  "SET deleted = ?, "
                      "display = ? "
                  "WHERE yeast.id IN (SELECT child_id FROM yeast_children)"), {QVariant{true}, QVariant{false}}},
         {QString("UPDATE water "
                  "SET deleted = ?, "
                      "display = ? "
                  "WHERE water.id IN (SELECT child_id FROM water_children)"), {QVariant{true}, QVariant{false}}},
         //
         // So we don't need the hop_children, fermentable_children, misc_children, yeast_children, water_children
         // tables any more.
         //
         {QString("DROP TABLE         hop_children")},
         {QString("DROP TABLE fermentable_children")},
         {QString("DROP TABLE        misc_children")},
         {QString("DROP TABLE       yeast_children")},
         {QString("DROP TABLE       water_children")},

         //
         // Whilst we're here, there are some unused columns on hop, and various other tables, that we should get rid
         // of.  These were added a long time ago for a feature that was dropped (see
         // https://github.com/Brewtarget/brewtarget/issues/557) so safe to delete.
         //
         // Don't forget we renamed mashstep to mash_step above!
         //
         {QString("ALTER TABLE hop         DROP COLUMN display_unit")},
         {QString("ALTER TABLE hop         DROP COLUMN display_scale")},
         {QString("ALTER TABLE fermentable DROP COLUMN display_unit")},
         {QString("ALTER TABLE fermentable DROP COLUMN display_scale")},
         {QString("ALTER TABLE mash_step   DROP COLUMN display_unit")},
         {QString("ALTER TABLE mash_step   DROP COLUMN display_scale")},
         {QString("ALTER TABLE mash_step   DROP COLUMN display_temp_unit")},
         {QString("ALTER TABLE misc        DROP COLUMN display_unit")},
         {QString("ALTER TABLE misc        DROP COLUMN display_scale")},
         {QString("ALTER TABLE yeast       DROP COLUMN display_unit")},
         {QString("ALTER TABLE yeast       DROP COLUMN display_scale")},
         //
         // The salt table has a column misc_id that is a foreign key to the misc table.  AFAIK this is currently
         // unused.  It's a bit of a palava to drop a foreign key column, as you have to make a new table and copy all
         // the data over etc, then drop and rename tables.  For the moment, we'll leave it alone as it seems to do no
         // harm.
         //
         //
         // Now we sort out inventory.  We move the hop ID and by-volume/by-mass info from the hop table to the
         // inventory table.  Same thing for fermentables, misc and, to some extent, yeast.
         //
         // NB: Inventory table for salt is brand new -- ie there never used to be one -- and gets created below just
         //     after we fix up the foreign keys on the other inventory tables.
         //
         // NB: No inventory for water!
         //
         {QString("ALTER TABLE         hop_in_inventory RENAME COLUMN amount TO quantity")},
         {QString("ALTER TABLE fermentable_in_inventory RENAME COLUMN amount TO quantity")},
         {QString("ALTER TABLE        misc_in_inventory RENAME COLUMN amount TO quantity")},
         {QString("ALTER TABLE       yeast_in_inventory RENAME COLUMN quanta TO quantity")},
         {QString("ALTER TABLE         hop_in_inventory ADD COLUMN         hop_id %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable_in_inventory ADD COLUMN fermentable_id %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE        misc_in_inventory ADD COLUMN        misc_id %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE       yeast_in_inventory ADD COLUMN       yeast_id %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE         hop_in_inventory ADD COLUMN unit   %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE fermentable_in_inventory ADD COLUMN unit   %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE        misc_in_inventory ADD COLUMN unit   %1").arg(db.getDbNativeTypeName<QString>())},
         {QString("ALTER TABLE       yeast_in_inventory ADD COLUMN unit   %1").arg(db.getDbNativeTypeName<QString>())},
         //
         // For historical reasons, some users have some duff entries in the xxx_in_inventory tables.  It's worth
         // cleaning them up here.  Note that empirical testing shows the "WHERE inventory_id IS NOT NULL" bit is needed
         // here.
         //
         {QString("DELETE FROM         hop_in_inventory WHERE id NOT IN (SELECT inventory_id FROM         hop WHERE inventory_id IS NOT NULL)")},
         {QString("DELETE FROM fermentable_in_inventory WHERE id NOT IN (SELECT inventory_id FROM fermentable WHERE inventory_id IS NOT NULL)")},
         {QString("DELETE FROM        misc_in_inventory WHERE id NOT IN (SELECT inventory_id FROM        misc WHERE inventory_id IS NOT NULL)")},
         {QString("DELETE FROM       yeast_in_inventory WHERE id NOT IN (SELECT inventory_id FROM       yeast WHERE inventory_id IS NOT NULL)")},
         //
         // At this point, all hop and fermentable amounts will be weights, because prior versions of the DB did not
         // support measuring them by volume.
         //
         // There is a bit of a gotcha waiting for us here.  In the old schema, where each hop refers to a
         // hop_in_inventory row, there can and will be multiple hops sharing the same inventory row.  In the new
         // schema, where each hop_in_inventory row refers to a hop, different hops cannot share an inventory.  (This is
         // all by design, as we ultimately want to be able to manage multiple inventory entries per hop.  That way you
         // can separately track the Fuggles that you bought last year (and need to use up) from the ones you bought
         // last week (so you won't run out when you use up last year's).
         //
         // Although we have, by this point, deleted the "child" entries in hop that signified "use of hop in recipe"
         // (replacing them with entries in hop_in_recipe).  There can still be multiple entries for "the same" hop in
         // the hop table, all sharing the same hop_in_inventory row.  Specifically this is because of hops marked
         // deleted or not displayable.  The simple answer would be to ignore deleted and non-displayable hops.  But
         // this creates another problem because there can be inventory entries for hops that are only deleted.  And,
         // whilst we might have a natural instinct to just delete the hop_in_inventory rows that have no corresponding
         // displayable not-deleted hop, that feels wrong as we would be throwing data away that might one day be
         // needed (eg if someone wants to undelete a hop that they deleted in error).
         //
         // So, we do something "clever".  For the "update hop_in_inventory rows to point at hop rows" query, we feed it
         // the list of hops in a sort of reverse order, specifically such that the deleted and non-displayable ones
         // occur before the others.  Thus, where there are multiple hop rows pointing to the same hop_in_inventory row,
         // the latter will get updated multiple times and, if there is a corresponding displayable non-deleted hop,
         // that will be the last one that the hop_in_inventory row is updated to point to, so it will "win" over the
         // others.  Despite sounding a bit complicated, it's makes the SQL simpler than a lot of other approaches!
         //
         // It _should_ be the case that there is at most one displayable non-deleted hop per row of hop_in_inventory.
         // However, just in case this is ever not true, we go a bit further and say that, after ordering hops so that
         // all the deleted and non-displayable ones are parsed first, the remaining ones are put in reverse ID order,
         // which means the oldest entries (with the smallest IDs) get processed last and are most likely to "win" in
         // conflicts.  At the moment at least, it seems the most reasonable tie-breaker.
         //
         // Of course, all the above applies, mutatis mutandis, to fermentables, miscs, and yeasts as well.
         //
         {QString("UPDATE hop_in_inventory "
                  "SET hop_id = h.id, "
                      "unit = 'kilograms' "
                  "FROM ("
                     "SELECT id, "
                            "inventory_id "
                     "FROM hop "
                     "ORDER BY NOT deleted, display, id DESC "
                  ") AS h "
                  "WHERE hop_in_inventory.id = h.inventory_id")},
         {QString("UPDATE fermentable_in_inventory "
                  "SET fermentable_id = f.id, "
                      "unit = 'kilograms' "
                  "FROM ("
                     "SELECT id, "
                            "inventory_id "
                     "FROM fermentable "
                     "ORDER BY NOT deleted, display, id DESC "
                  ") AS f "
                  "WHERE fermentable_in_inventory.id = f.inventory_id ")},
         // For misc, we need to support both weights and volumes.
         {QString("UPDATE misc_in_inventory "
                  "SET misc_id = m.id, "
                      "unit = m.unit "
                  "FROM ("
                     "SELECT id, "
                            "inventory_id, "
                            "CASE WHEN amount_is_weight THEN 'kilograms' ELSE 'liters' END AS unit "
                     "FROM misc "
                     "ORDER BY NOT deleted, display, id DESC "
                  ") AS m "
                  "WHERE misc_in_inventory.id = m.inventory_id")},
         // HOWEVER, for inventory purposes, yeast is actually stored as "number of packets" (aka quanta in various old
         // bits of the code).
         {QString("UPDATE yeast_in_inventory "
                  "SET yeast_id = y.id, "
                      "unit = 'number_of' "
                  "FROM ("
                     "SELECT id, "
                            "inventory_id "
                     "FROM yeast "
                     "ORDER BY NOT deleted, display, id DESC "
                  ") AS y "
                  "WHERE yeast_in_inventory.id = y.inventory_id")},
         // Now we transferred info across, we don't need the inventory_id column on hop or fermentable
         {QString("ALTER TABLE hop         DROP COLUMN inventory_id")},
         {QString("ALTER TABLE fermentable DROP COLUMN inventory_id")},
         {QString("ALTER TABLE misc        DROP COLUMN inventory_id")},
         {QString("ALTER TABLE yeast       DROP COLUMN inventory_id")},
         // And we're finally done with the amount_is_weight columns
         {QString("ALTER TABLE misc        DROP COLUMN amount_is_weight")},
         {QString("ALTER TABLE yeast       DROP COLUMN amount_is_weight")},
         //
         // We would like hop_in_inventory.hop_id to be a foreign key to hop.id.  SQLite only lets you create foreign
         // key constraints at the time that you create the table, so this is a bit of a palava.
         //
         {QString("CREATE TABLE tmp_hop_in_inventory ( "
                    "id        %1, "
                    "hop_id    %2, "
                    "quantity  %3, "
                    "unit      %4, "
                    "FOREIGN KEY(hop_id)   REFERENCES hop(id)"
                 ");").arg(db.getDbNativePrimaryKeyDeclaration(),
                           db.getDbNativeTypeName<int    >(),
                           db.getDbNativeTypeName<double >(),
                           db.getDbNativeTypeName<QString>())},
         {QString("INSERT INTO tmp_hop_in_inventory (id, hop_id, quantity, unit) "
                  "SELECT id, hop_id, quantity, unit "
                  "FROM hop_in_inventory")},
         {QString("DROP TABLE hop_in_inventory")},
         {QString("ALTER TABLE tmp_hop_in_inventory "
                  "RENAME TO hop_in_inventory")},
         // Same palava for fermentable_in_inventory.fermentable_id to be a foreign key to fermentable.id
         {QString("CREATE TABLE tmp_fermentable_in_inventory ( "
                    "id             %1, "
                    "fermentable_id %2, "
                    "quantity       %3, "
                    "unit           %4, "
                    "FOREIGN KEY(fermentable_id)   REFERENCES fermentable(id)"
                 ");").arg(db.getDbNativePrimaryKeyDeclaration(),
                           db.getDbNativeTypeName<int    >(),
                           db.getDbNativeTypeName<double >(),
                           db.getDbNativeTypeName<QString>())},
         {QString("INSERT INTO tmp_fermentable_in_inventory (id, fermentable_id, quantity, unit) "
                  "SELECT id, fermentable_id, quantity, unit "
                  "FROM fermentable_in_inventory")},
         {QString("DROP TABLE fermentable_in_inventory")},
         {QString("ALTER TABLE tmp_fermentable_in_inventory "
                  "RENAME TO fermentable_in_inventory")},
         // Same palava for misc_in_inventory.misc_id to be a foreign key to misc.id
         {QString("CREATE TABLE tmp_misc_in_inventory ( "
                    "id             %1, "
                    "misc_id %2, "
                    "quantity       %3, "
                    "unit           %4, "
                    "FOREIGN KEY(misc_id)   REFERENCES misc(id)"
                 ");").arg(db.getDbNativePrimaryKeyDeclaration(),
                           db.getDbNativeTypeName<int    >(),
                           db.getDbNativeTypeName<double >(),
                           db.getDbNativeTypeName<QString>())},
         {QString("INSERT INTO tmp_misc_in_inventory (id, misc_id, quantity, unit) "
                  "SELECT id, misc_id, quantity, unit "
                  "FROM misc_in_inventory")},
         {QString("DROP TABLE misc_in_inventory")},
         {QString("ALTER TABLE tmp_misc_in_inventory "
                  "RENAME TO misc_in_inventory")},
         // Same palava for yeast_in_inventory.yeast_id to be a foreign key to yeast.id
         {QString("CREATE TABLE tmp_yeast_in_inventory ( "
                    "id             %1, "
                    "yeast_id %2, "
                    "quantity       %3, "
                    "unit           %4, "
                    "FOREIGN KEY(yeast_id)   REFERENCES yeast(id)"
                 ");").arg(db.getDbNativePrimaryKeyDeclaration(),
                           db.getDbNativeTypeName<int    >(),
                           db.getDbNativeTypeName<double >(),
                           db.getDbNativeTypeName<QString>())},
         {QString("INSERT INTO tmp_yeast_in_inventory (id, yeast_id, quantity, unit) "
                  "SELECT id, yeast_id, quantity, unit "
                  "FROM yeast_in_inventory")},
         {QString("DROP TABLE yeast_in_inventory")},
         {QString("ALTER TABLE tmp_yeast_in_inventory "
                  "RENAME TO yeast_in_inventory")},
         // Note that there isn't yet a salt_in_inventory table so we don't have to change it -- just create it!
         {QString("CREATE TABLE salt_in_inventory ( "
                    "id        %1, "
                    "salt_id   %2, "
                    "quantity  %3, "
                    "unit      %4, "
                    "FOREIGN KEY(salt_id)   REFERENCES salt(id)"
                 ");").arg(db.getDbNativePrimaryKeyDeclaration(),
                           db.getDbNativeTypeName<int    >(),
                           db.getDbNativeTypeName<double >(),
                           db.getDbNativeTypeName<QString>())},
         //
         // As part of removing code duplication in the model classes, we introduced FolderBase for classes that live in
         // folders, and removed the folder attribute from BrewNote.  (A BrewNote belongs to a Recipe.  The Recipe has a
         // folder, but it does not make sense for the BrewNote to have its own folder.)
         //
         {QString("ALTER TABLE brewnote DROP COLUMN folder")},
         //
         // Finally, since we're doing a big update and a bit of a clean up, it is time to drop tables that have not
         // been used for a long time and are not mentioned anywhere else in the current code base.
         //
         {QString("DROP TABLE bt_equipment")},
         {QString("DROP TABLE bt_fermentable")},
         {QString("DROP TABLE bt_hop")},
         {QString("DROP TABLE bt_misc")},
         {QString("DROP TABLE bt_style")},
         {QString("DROP TABLE bt_water")},
         {QString("DROP TABLE bt_yeast")},
         {QString("DROP TABLE recipe_children")},
         //
         // Finally finally, we update the "settings" table to drop columns we no longer use and support multiple files
         // for new "default content".
         //
         // It would be nice to rename the "version" column to "schema_version" but doing so would create a Catch-22
         // situation where, in order to know what the schema version column name is in the DB we're reading, we need to
         // know the contents of that same column.  It's solvable but the ugliness of doing so isn't worth the benefit
         // of having the better column name IMHO.
         //
         {QString("ALTER TABLE settings DROP COLUMN repopulatechildrenonnextstart")},
         {QString("ALTER TABLE settings  ADD COLUMN default_content_version %1").arg(db.getDbNativeTypeName<unsigned int>())},
         {QString("UPDATE settings SET default_content_version = 0")},
      };

      return executeSqlQueries(q, migrationQueries);
   }

   /**
    * \brief This is not actually a schema change, but rather data fixes that were missed from migrate_to_11 - mostly
    *        things where we should have been less case-sensitive.
    */
   bool migrate_to_12([[maybe_unused]] Database & db, BtSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         {QString(     "UPDATE hop SET htype = 'aroma/bittering' WHERE lower(htype) = 'both'"     )},
         {QString("     UPDATE fermentable SET ftype = 'dry extract' WHERE lower(ftype) = 'dry extract'")},
         {QString("     UPDATE fermentable SET ftype = 'dry extract' WHERE lower(ftype) = 'dryextract'" )},
         {QString("     UPDATE fermentable SET ftype = 'other'       WHERE lower(ftype) = 'adjunct'"    )},
         {QString("     UPDATE misc SET mtype = 'water agent' WHERE lower(mtype) = 'water agent'")},
         {QString("     UPDATE misc SET mtype = 'water agent' WHERE lower(mtype) = 'wateragent'" )},
         {QString("     UPDATE yeast SET ytype = 'other'     WHERE lower(ytype) = 'wheat'    ")},
         {QString("     UPDATE yeast SET flocculation = 'very high' WHERE lower(flocculation) = 'very high'")},
         {QString("     UPDATE yeast SET flocculation = 'very high' WHERE lower(flocculation) = 'veryhigh'" )},
         {QString("     UPDATE style SET stype = 'beer'  WHERE lower(stype) = 'lager'")},
         {QString("     UPDATE style SET stype = 'beer'  WHERE lower(stype) = 'ale'  ")},
         {QString("     UPDATE style SET stype = 'beer'  WHERE lower(stype) = 'wheat'")},
         {QString("     UPDATE style SET stype = 'other' WHERE lower(stype) = 'mixed'")},
         {QString("     UPDATE mash_step SET mstype = 'sparge'         WHERE lower(mstype) = 'flysparge'  ")},
         {QString("     UPDATE mash_step SET mstype = 'sparge'         WHERE lower(mstype) = 'fly sparge'  ")},
         {QString("     UPDATE mash_step SET mstype = 'drain mash tun' WHERE lower(mstype) = 'batchsparge'")},
         {QString("     UPDATE mash_step SET mstype = 'drain mash tun' WHERE lower(mstype) = 'batch sparge'")},
         {QString("     UPDATE recipe SET type = 'partial mash' WHERE lower(type) = 'partial mash'")},
         {QString("     UPDATE recipe SET type = 'partial mash' WHERE lower(type) = 'partialmash'")},
         {QString("     UPDATE recipe SET type = 'all grain'    WHERE lower(type) = 'all grain'   ")},
         {QString("     UPDATE recipe SET type = 'all grain'    WHERE lower(type) = 'allgrain'   ")},
      };
      return executeSqlQueries(q, migrationQueries);
   }

   /**
    * \brief Small schema change to support measuring diameter of boil kettle for new IBU calculations.  Plus, some
    *        tidy-ups to Salt and Boil / BoilStep which we didn't get to before.
    */
   bool migrate_to_13([[maybe_unused]] Database & db, BtSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         {QString("ALTER TABLE equipment  ADD COLUMN kettleInternalDiameter_cm %1").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE equipment  ADD COLUMN kettleOpeningDiameter_cm  %1").arg(db.getDbNativeTypeName<double>())},
         // The is_acid column is unnecessary as we know whether it's an acide from the stype column
         {QString("ALTER TABLE salt DROP COLUMN is_acid")},
         // The addTo column is not needed as it is now replaced by salt_in_recipe.when_to_add and the contents brought
         // over in migrate_to_11 above.  Same goes for amount_is_weight, which is replaced by salt_in_recipe.unit and
         // salt_in_inventory.unit.
         {QString("ALTER TABLE salt DROP COLUMN addTo")},
         {QString("ALTER TABLE salt DROP COLUMN amount_is_weight")},
         //
         // The boil.boil_time_mins column is, in reality the length of the boil proper, so it should have gone straight
         // to the relevant boil_step.  We correct that here and do away with the column on boil.
         //
         // Per the comment in model/Boil.h on minimumBoilTemperature_c, 81.0°C is the temperature above which we assume
         // a step is a boil.
         //
         {QString("UPDATE boil_step "
                  "SET step_time_mins = b.boil_time_mins "
                  "FROM ("
                     "SELECT id, "
                            "boil_time_mins "
                     "FROM boil"
                  ") AS b "
                  "WHERE boil_step.step_time_mins IS NULL "
                    "AND boil_step.start_temp_c >= 81.0 "
                    "AND boil_step.end_temp_c >= 81.0 "
                    "AND boil_step.boil_id = b.id ")},
         {QString("ALTER TABLE boil DROP COLUMN boil_time_mins")},
      };
      return executeSqlQueries(q, migrationQueries);
   }

   /**
    * \brief Correct flouride to fluoride on water table
    *        Fix Instructions to be stored the same way as BrewNotes, RecipeAdditions etc
    *           // TODO: On next DB update, correct water.flouride_ppm to water.fluoride_ppm
.
    */
   bool migrate_to_14([[maybe_unused]] Database & db, BtSqlQuery & q) {
      //
      // See below for why we create a new version of the instruction table here.  While we're at it, we make the
      // column names snake_case, and add units, to be more consistent with the rest of the DB schema.
      //
      // NOTE I am not convinced that has_timer, timer_value and completed columns are actually used
      //
      QString createNewInstructionsSql;
      QTextStream createNewInstructionsSqlStream(&createNewInstructionsSql);
      createNewInstructionsSqlStream <<
         "CREATE TABLE new_instruction ("
            "id"              " " << db.getDbNativePrimaryKeyDeclaration() << ", "
            "name"            " " << db.getDbNativeTypeName<QString>()     << ", "
            "display"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "deleted"         " " << db.getDbNativeTypeName<bool>()        << ", "
            "recipe_id"       " " << db.getDbNativeTypeName<int>()         << ", "
            "step_number"     " " << db.getDbNativeTypeName<int>()         << ", "
            "directions"      " " << db.getDbNativeTypeName<QString>()     << ", "
            "has_timer"       " " << db.getDbNativeTypeName<bool>()        << ", "
            "timer_value"     " " << db.getDbNativeTypeName<QString>()     << ", "
            "completed"       " " << db.getDbNativeTypeName<bool>()        << ", "
            "interval_mins"   " " << db.getDbNativeTypeName<double>()      << ", "
            "FOREIGN KEY(recipe_id) REFERENCES recipe(id) "
         ");";

      QVector<QueryAndParameters> const migrationQueries{
         //
         // This is a naming error in BeerJSON that we copied-and-pasted into the code.  BeerJSON will get a fix at some
         // point -- see https://github.com/beerjson/beerjson/issues/214.  But we can fix our DB column name in the
         // meantime.
         //
         {QString("ALTER TABLE water RENAME COLUMN flouride_ppm TO fluoride_ppm")},
         //
         // The existence of the instruction_in_recipe table implies that Instruction objects can be shared between
         // multiple Recipe objects.  However, since they are only managed inside each Recipe, and are often
         // automatically generated, it doesn't make a whole lot of sense for an Instruction to have a separate
         // existence outside of Recipe.  So we try to make them more like BrewNotes here.
         //
         // At the risk of being overly cautious, we assume it is at least conceivable there could be some user
         // databases where instructions are shared between recipes (even though this should not happen).   To avoid any
         // potential data loss, we therefore allow for the possibility that we might need to create copies of existing
         // "shared" instructions (since, after the schema update it will be impossible for one instruction row to
         // related to more than one recipe row).  This being the case, it seems simpler to create a new table rather
         // than fill in the blanks on the existing one.
         //
         {createNewInstructionsSql},
         {QString("INSERT INTO new_instruction ( "
            "name       , "
            "display    , "
            "deleted    , "
            "recipe_id  , "
            "step_number, "
            "directions , "
            "has_timer  , "
            "timer_value, "
            "completed  , "
            "interval_mins "
          ") "
          "SELECT ii.name              , "
                 "ii.display           , "
                 "ii.deleted           , "
                 "jj.recipe_id         , "
                 "jj.instruction_number, "
                 "ii.directions        , "
                 "ii.hasTimer          , " // NB: hasTimer -> has_timer
                 "ii.timerValue        , " // NB: timerValue -> timer_value
                 "ii.completed         , "
                 "ii.interval "            // NB: interval -> interval_mins
          "FROM instruction AS ii, "
               "instruction_in_recipe AS jj "
          "WHERE jj.instruction_id = ii.id")},
         // Now we've copied all the data to the new table, we can safely throw the old tables away
         {QString("DROP TABLE instruction_in_recipe")},
         {QString("DROP TABLE instruction")},
         // And finally we can rename the new table replace the old one
         {QString("ALTER TABLE new_instruction RENAME TO instruction")},

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
      switch(oldVersion) {
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
         case 10:
            ret &= migrate_to_11(database, sqlQuery);
            break;
         case 11:
            ret &= migrate_to_12(database, sqlQuery);
            break;
         case 12:
            ret &= migrate_to_13(database, sqlQuery);
            break;
         case 13:
            ret &= migrate_to_14(database, sqlQuery);
            break;
         default:
            qCritical() << QString("Unknown version %1").arg(oldVersion);
            return false;
      }

      //
      // Set the db version
      //
      if (oldVersion > 3) {
         QString const queryString{"UPDATE settings SET version=:version WHERE id=1"};
         sqlQuery.prepare(queryString);
         QVariant bindValue{QString::number(oldVersion + 1)};
         sqlQuery.bindValue(":version", bindValue);
         ret &= sqlQuery.exec();
      }

      return ret;
   }

}


int DatabaseSchemaHelper::getDefaultContentVersionFromDb(QSqlDatabase & db) {
   BtSqlQuery sqlQuery("SELECT default_content_version FROM settings WHERE id=1", db);
   if (sqlQuery.next() ) {
      QVariant dc = sqlQuery.value("default_content_version");
      return dc.toInt();
   }
   return -1;
}

bool DatabaseSchemaHelper::setDefaultContentVersionFromDb(QSqlDatabase & db, int val) {
   BtSqlQuery sqlQuery{db};
   QString const queryString{"UPDATE settings SET default_content_version=:version WHERE id=1"};
   sqlQuery.prepare(queryString);
   QVariant bindValue{QString::number(val)};
   sqlQuery.bindValue(":version", bindValue);
   bool ret = sqlQuery.exec();
   return ret;
}


bool DatabaseSchemaHelper::create(Database & database, QSqlDatabase connection) {
   //--------------------------------------------------------------------------
   // NOTE: if you edit this function, increment DatabaseSchemaHelper::latestVersion and edit
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
   // Create the settings table manually, since it's only used in a couple of places (this file and
   // DefaultContentLoader.cpp).
   //
   // Note that we changed the settings table in version 11 of the schema (DatabaseSchemaHelper::latestVersion == 11).  What
   // we create here is the new version of that table.
   //
   QVector<QueryAndParameters> const setUpQueries{
      {QString("CREATE TABLE settings (id %1, "
                                      "version %2, "
                                      "default_content_version %2)").arg(database.getDbNativePrimaryKeyDeclaration(),
                                                                         database.getDbNativeTypeName<int>        ())},
      {QString("INSERT INTO settings (version, "
                                     "default_content_version) "
               "VALUES (?, ?)"), {QVariant(DatabaseSchemaHelper::latestVersion), QVariant(0)}}
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
   if (oldVersion >= newVersion || newVersion > DatabaseSchemaHelper::latestVersion ) {
      qCritical() << Q_FUNC_INFO <<
         "Requested backwards migration from" << oldVersion << "to" << newVersion << ".  Assuming this is a coding "
         "error and therefore doing nothing!";
      return false;
   }

   bool ret = true;
   qDebug() << Q_FUNC_INFO << "Migrating database schema from v" << oldVersion << "to v" << newVersion;

   // Start transaction
   // By the magic of RAII, this will abort if we exit this function (including by throwing an exception) without
   // having called dbTransaction.commit().  (It will also turn foreign keys back on either way -- whether the
   // transaction is committed or rolled back.)
   DbTransaction dbTransaction{database, connection, "Migrate", DbTransaction::DISABLE_FOREIGN_KEYS};

   for ( ; oldVersion < newVersion && ret; ++oldVersion ) {
      ret &= migrateNext(database, oldVersion, connection);
   }

   // If all statements executed OK, we can commit, otherwise the transaction will roll back when we exit this function
   if (ret) {
      ret &= dbTransaction.commit();
   }

   return ret;
}

int DatabaseSchemaHelper::schemaVersion(QSqlDatabase & db) {
   // Version was a string field in early versions of the code and then became an integer field
   // We'll read it into a QVariant and then work out whether it's a string or an integer
   BtSqlQuery q("SELECT version FROM settings WHERE id=1", db);
   QVariant ver;
   if (q.next() ) {
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
   if (ver.canConvert<int>()) {
      return ver.toInt();
   }

   if (stringVer == "2.0.0") {
      return 1;
   }

   if (stringVer == "2.0.2") {
      return 2;
   }

   if (stringVer == "2.1.0") {
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
