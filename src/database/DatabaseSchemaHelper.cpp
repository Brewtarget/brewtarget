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
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "brewtarget.h"
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

   //! \brief converts sqlite values (mostly booleans) into something postgres wants
/*   QVariant convertValue(Database::DbType newType, QSqlField field) {
      QVariant retVar = field.value();
      if ( field.type() == QVariant::Bool ) {
         switch(newType) {
            case Database::PGSQL:
               retVar = field.value().toBool();
               break;
            default:
               retVar = field.value().toInt();
               break;
         }
      } else if ( field.name() == PropertyNames::BrewNote::fermentDate && field.value().toString() == "CURRENT_DATETIME" ) {
         retVar = "'now()'";
      }
      return retVar;
   }*/

   struct QueryAndParameters {
      QString sql;
      QVector<QVariant> bindValues = {};
   };

   //
   // These migrate_to_Xyz functions are deliberately hard-coded.  Because we're migrating from version N to version
   // N+1, we don't need (or want) to refer to the generated table definitions from some later version of the schema,
   // which may be quite different.
   //
   // For the moment, it mostly suffices to execute a list of queries.  A possible future enhancement might be to
   // attach to each query a (usually empty) list of bind parameters, but it's probably not necessary.
   //
   bool executeSqlQueries(QSqlQuery & q, QVector<QueryAndParameters> const & queries) {
      // If we get an error, we want to stop processing as otherwise you get "false" errors if subsequent queries fail
      // as a result of assuming that all prior queries have run OK.
      for (auto & query : queries) {
         qDebug() << Q_FUNC_INFO << query.sql;
         q.prepare(query.sql);
         for (auto & bv : query.bindValues) {
            q.addBindValue(bv);
         }
         if (!q.exec()) {
            qCritical() <<
               Q_FUNC_INFO << "Error executing database upgrade/set-up query " << query.sql << ": " <<
               q.lastError().text();
            return false;
         }
      }
      return true;
   }

   // This is when we first defined the settings table, and defined the version as a string.
   // In the new world, this will create the settings table and define the version as an int.
   // Since we don't set the version until the very last step of the update, I think this will be fine.
   bool migrate_to_202(Database & db, QSqlQuery q) {
      bool ret = true;

      // Add "projected_ferm_points" to brewnote table
      QString queryString{"ALTER TABLE brewnote ADD COLUMN projected_ferm_points "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << db.getDbNativeTypeName<double>() << " DEFAULT 0.0;";
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);
      queryString = "ALTER TABLE brewnote SET projected_ferm_points = -1.0;";
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);

      // Add the settings table
      queryString = "CREATE TABLE settings ";
      queryStringAsStream << "(\n"
         "id " << db.getDbNativeTypeName<int>() << " " << db.getDbNativeIntPrimaryKeyModifier() << ",\n"
         "repopulatechildrenonnextstart " << db.getDbNativeTypeName<int>() << " DEFAULT 0,\n"
         "version " << db.getDbNativeTypeName<int>() << " DEFAULT 0);";
      qDebug() << Q_FUNC_INFO << queryString;
      ret &= q.exec(queryString);

      return ret;
   }

   bool migrate_to_210(Database & db, QSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         {QString("ALTER TABLE equipment   ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE fermentable ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE hop         ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE misc        ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE style       ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE yeast       ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE water       ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE mash        ADD COLUMN folder text DEFAULT ''")},
         //{QString("ALTER TABLE mashstep ADD COLUMN   DEFAULT ''")},
         {QString("ALTER TABLE recipe      ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE brewnote    ADD COLUMN folder text DEFAULT ''")},
         {QString("ALTER TABLE instruction ADD COLUMN   DEFAULT ''")},
         {QString("ALTER TABLE salt        ADD COLUMN folder text DEFAULT ''")},
         // Put the "Bt:.*" recipes into /brewtarget folder
         {QString("UPDATE recipe   SET folder='/brewtarget' WHERE name LIKE 'Bt:%'")},
         // Update version to 2.1.0
         {QString("UPDATE settings SET version='2.1.0' WHERE id=1")},
         // Used to trigger the code to populate the ingredient inheritance tables
         {QString("ALTER TABLE settings ADD COLUMN repopulatechildrenonnextstart %1").arg(db.getDbNativeTypeName<int>())},
         {QString("UPDATE repopulatechildrenonnextstart integer=1")},
         // Drop and re-create children tables with new UNIQUE requirement
         {QString("DROP TABLE   equipment_children")},
         {QString("CREATE TABLE equipment_children (id %1 %2, "
                                                  "child_id %1, "
                                                  "parent_id %1, "
                                                  "FOREIGN KEY(child_id) REFERENCES equipment(id), "
                                                  "FOREIGN KEY(parent_id) REFERENCES equipment(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   fermentable_children")},
         {QString("CREATE TABLE fermentable_children (id %1 %2, "
                                                    "child_id %1, "
                                                    "parent_id %1, "
                                                    "FOREIGN KEY(child_id) REFERENCES fermentable(id), "
                                                    "FOREIGN KEY(parent_id) REFERENCES fermentable(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   hop_children")},
         {QString("CREATE TABLE hop_children (id %1 %2, "
                                            "child_id %1, "
                                            "parent_id %1, "
                                            "FOREIGN KEY(child_id) REFERENCES hop(id), "
                                            "FOREIGN KEY(parent_id) REFERENCES hop(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   misc_children")},
         {QString("CREATE TABLE misc_children (id %1 %2, "
                                             "child_id %1, "
                                             "parent_id %1, "
                                             "FOREIGN KEY(child_id) REFERENCES misc(id), "
                                             "FOREIGN KEY(parent_id) REFERENCES misc(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   recipe_children")},
         {QString("CREATE TABLE recipe_children (id %1 %2, "
                                               "child_id %1, "
                                               "parent_id %1, "
                                               "FOREIGN KEY(child_id) REFERENCES recipe(id), "
                                               "FOREIGN KEY(parent_id) REFERENCES recipe(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   style_children")},
         {QString("CREATE TABLE style_children (id %1 %2, "
                                              "child_id %1, "
                                              "parent_id %1, "
                                              "FOREIGN KEY(child_id) REFERENCES style(id), "
                                              "FOREIGN KEY(parent_id) REFERENCES style(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   water_children")},
         {QString("CREATE TABLE water_children (id %1 %2, "
                                              "child_id %1, "
                                              "parent_id %1, "
                                              "FOREIGN KEY(child_id) REFERENCES water(id), "
                                              "FOREIGN KEY(parent_id) REFERENCES water(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   yeast_children")},
         {QString("CREATE TABLE yeast_children (id %1 %2, "
                                              "child_id %1, "
                                              "parent_id %1, "
                                              "FOREIGN KEY(child_id) REFERENCES yeast(id), "
                                              "FOREIGN KEY(parent_id) REFERENCES yeast(id));").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())},
         {QString("DROP TABLE   fermentable_in_inventory")},
         {QString("CREATE TABLE fermentable_in_inventory (id %1 %2, "
                                                        "amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("DROP TABLE   hop_in_inventory")},
         {QString("CREATE TABLE hop_in_inventory (id %1 %2, "
                                                "amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("DROP TABLE   misc_in_inventory")},
         {QString("CREATE TABLE misc_in_inventory (id %1 %2, "
                                                 "amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("DROP TABLE   yeast_in_inventory")},
         {QString("CREATE TABLE yeast_in_inventory (id %1 %2, "
                                                  "quanta %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("UPDATE settings VALUES(1,2)")}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_4(Database & db, QSqlQuery & q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Save old settings
         {QString("ALTER TABLE settings RENAME TO oldsettings")},
         // Create new table with integer version.
         {QString("CREATE TABLE settings (id %1 %2, "
                                        "repopulatechildrenonnextstart %1 DEFAULT 0, "
                                        "version %1 DEFAULT 0);").arg(db.getDbNativeTypeName<int>()).arg(db.getDbNativeIntPrimaryKeyModifier())},
         // Update version to 4, saving other settings
         {QString("INSERT INTO settings (id, version, repopulatechildrenonnextstart) SELECT 1, 4, repopulatechildrenonnextstart FROM oldsettings")},
         // Cleanup
         {QString("DROP TABLE oldsettings")}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_5(Database & db, QSqlQuery q) {
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
   bool migrate_to_6(Database & db, QSqlQuery q) {
      bool ret = true;
      // I drop this table in version 8. There is no sense doing anything here, and it breaks other things.
      return ret;
   }

   bool migrate_to_7(Database & db, QSqlQuery q) {
      QVector<QueryAndParameters> const migrationQueries{
         // Add "attenuation" to brewnote table
         {"ALTER TABLE brewnote ADD COLUMN attenuation real DEFAULT 0.0"}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_8(Database & db, QSqlQuery q) {
      QString createTmpBrewnoteSql;
      QTextStream createTmpBrewnoteSqlStream(&createTmpBrewnoteSql);
      createTmpBrewnoteSqlStream <<
         "CREATE TABLE tmpbrewnote ("
            "id                      " << db.getDbNativeTypeName<int>()     << " " << db.getDbNativeIntPrimaryKeyModifier() << ", "
            "abv                     " << db.getDbNativeTypeName<double>()  << " DEFAULT 0, "
            "attenuation             " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "boil_off                " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "brewdate                " << db.getDbNativeTypeName<QDate>()   << " DEFAULT CURRENT_TIMESTAMP, "
            "brewhouse_eff           " << db.getDbNativeTypeName<double>()  << " DEFAULT 70, "
            "deleted                 " << db.getDbNativeTypeName<bool>()    << " DEFAULT 0, "
            "display                 " << db.getDbNativeTypeName<bool>()    << " DEFAULT 1, "
            "eff_into_bk             " << db.getDbNativeTypeName<double>()  << " DEFAULT 70, "
            "fermentdate             " << db.getDbNativeTypeName<QDate>()   << " DEFAULT CURRENT_TIMESTAMP, "
            "fg                      " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "final_volume            " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "folder                  " << db.getDbNativeTypeName<QString>() << " DEFAULT '', "
            "mash_final_temp         " << db.getDbNativeTypeName<double>()  << " DEFAULT 67, "
            "notes                   " << db.getDbNativeTypeName<QString>() << " DEFAULT '', "
            "og                      " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "pitch_temp              " << db.getDbNativeTypeName<double>()  << " DEFAULT 20, "
            "post_boil_volume        " << db.getDbNativeTypeName<double>()  << " DEFAULT 0, "
            "projected_abv           " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_atten         " << db.getDbNativeTypeName<double>()  << " DEFAULT 75, "
            "projected_boil_grav     " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_eff           " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_ferm_points   " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_fg            " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_mash_fin_temp " << db.getDbNativeTypeName<double>()  << " DEFAULT 67, "
            "projected_og            " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_points        " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_strike_temp   " << db.getDbNativeTypeName<double>()  << " DEFAULT 70, "
            "projected_vol_into_bk   " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "projected_vol_into_ferm " << db.getDbNativeTypeName<double>()  << " DEFAULT 0, "
            "sg                      " << db.getDbNativeTypeName<double>()  << " DEFAULT 1, "
            "strike_temp             " << db.getDbNativeTypeName<double>()  << " DEFAULT 70, "
            "volume_into_bk          " << db.getDbNativeTypeName<double>()  << " DEFAULT 0, "
            "volume_into_fermenter   " << db.getDbNativeTypeName<double>()  << " DEFAULT 0, "
            "recipe_id               " << db.getDbNativeTypeName<int>()     << ", "
            "FOREIGN KEY(recipe_id) REFERENCES recipe(id)"
         ");";
      QVector<QueryAndParameters> const migrationQueries{
         //
         // Drop columns predicted_og and predicted_abv. They are used nowhere I can find and they are breaking things.
         //
         {createTmpBrewnoteSql},
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
                 "FROM brewnote")},
         {QString("drop table brewnote")},
         {QString("ALTER TABLE tmpbrewnote RENAME TO brewnote")},
         //
         // Rearrange inventory - fermentable
         //
         {QString("ALTER TABLE fermentable ADD COLUMN inventory_id REFERENCES fermentable_in_inventory (id)")},
         // It would seem we have kids with their own rows in the db. This is a freaking mess, but I need to delete those rows
         // before I can do anything else.
         {QString("DELETE FROM fermentable_in_inventory "
                 "WHERE fermentable_in_inventory.id in ( "
                    "SELECT fermentable_in_inventory.id "
                    "FROM fermentable_in_inventory, fermentable_children, fermentable "
                    "WHERE fermentable.id = fermentable_children.child_id "
                    "AND fermentable_in_inventory.fermentable_id = fermentable.id "
                 ")")},
         {QString("INSERT INTO fermentable_in_inventory (fermentable_id) VALUES ( "
                    // Everything has an inventory row now. This will find all the parent items that don't have an inventory row.
                    "SELECT id FROM fermentable WHERE NOT EXISTS ( "
                       "SELECT fermentable_children.id "
                       "FROM fermentable_children "
                       "WHERE fermentable_children.child_id = fermentable.id "
                    ") AND NOT EXISTS ( "
                       "SELECT fermentable_in_inventory.id "
                       "FROM fermentable_in_inventory "
                       "WHERE fermentable_in_inventory.fermentable_id = fermentable.id"
                    ") "
                 ")")},
         // Once we know all parents have inventory rows, we populate inventory_id for them
         {QString("UPDATE fermentable SET inventory_id = ("
                    "SELECT fermentable_in_inventory.id "
                    "FROM fermentable_in_inventory "
                    "WHERE fermentable.id = fermentable_in_inventory.fermentable_id"
                 ")")},
         // Finally, we update all the kids to have the same inventory_id as their dear old paw
         {QString("UPDATE fermentable SET inventory_id = ( "
                    "SELECT tmp.inventory_id "
                    "FROM fermentable tmp, fermentable_children "
                    "WHERE fermentable.id = fermentable_children.child_id "
                    "AND tmp.id = fermentable_children.parent_id"
                 ") "
                 "WHERE inventory_id IS NULL")},
         //
         // Rearrange inventory - hop
         //
         {QString("ALTER TABLE hop ADD COLUMN inventory_id REFERENCES hop_in_inventory (id)")},
         // It would seem we have kids with their own rows in the db. This is a freaking mess, but I need to delete those rows
         // before I can do anything else.
         {QString("DELETE FROM hop_in_inventory "
                 "WHERE hop_in_inventory.id in ( "
                    "SELECT hop_in_inventory.id "
                    "FROM hop_in_inventory, hop_children, hop "
                    "WHERE hop.id = hop_children.child_id "
                    "AND hop_in_inventory.hop_id = hop.id "
                 ")")},
         {QString("INSERT INTO hop_in_inventory (hop_id) VALUES ( "
                    // Everything has an inventory row now. This will find all the parent items that don't have an inventory row.
                    "SELECT id FROM hop WHERE NOT EXISTS ( "
                       "SELECT hop_children.id "
                       "FROM hop_children "
                       "WHERE hop_children.child_id = hop.id "
                    ") AND NOT EXISTS ( "
                       "SELECT hop_in_inventory.id "
                       "FROM hop_in_inventory "
                       "WHERE hop_in_inventory.hop_id = hop.id"
                    ") "
                 ")")},
         // Once we know all parents have inventory rows, we populate inventory_id for them
         {QString("UPDATE hop SET inventory_id = ("
                    "SELECT hop_in_inventory.id "
                    "FROM hop_in_inventory "
                    "WHERE hop.id = hop_in_inventory.hop_id"
                 ")")},
         // Finally, we update all the kids to have the same inventory_id as their dear old paw
         {QString("UPDATE hop SET inventory_id = ( "
                    "SELECT tmp.inventory_id "
                    "FROM hop tmp, hop_children "
                    "WHERE hop.id = hop_children.child_id "
                    "AND tmp.id = hop_children.parent_id"
                 ") "
                 "WHERE inventory_id IS NULL")},
         //
         // Rearrange inventory - misc
         //
         {QString("ALTER TABLE misc ADD COLUMN inventory_id REFERENCES misc_in_inventory (id)")},
         // It would seem we have kids with their own rows in the db. This is a freaking mess, but I need to delete those rows
         // before I can do anything else.
         {QString("DELETE FROM misc_in_inventory "
                 "WHERE misc_in_inventory.id in ( "
                    "SELECT misc_in_inventory.id "
                    "FROM misc_in_inventory, misc_children, misc "
                    "WHERE misc.id = misc_children.child_id "
                    "AND misc_in_inventory.misc_id = misc.id "
                 ")")},
         {QString("INSERT INTO misc_in_inventory (misc_id) VALUES ( "
                    // Everything has an inventory row now. This will find all the parent items that don't have an inventory row.
                    "SELECT id FROM misc WHERE NOT EXISTS ( "
                       "SELECT misc_children.id "
                       "FROM misc_children "
                       "WHERE misc_children.child_id = misc.id "
                    ") AND NOT EXISTS ( "
                       "SELECT misc_in_inventory.id "
                       "FROM misc_in_inventory "
                       "WHERE misc_in_inventory.misc_id = misc.id"
                    ") "
                 ")")},
         // Once we know all parents have inventory rows, we populate inventory_id for them
         {QString("UPDATE misc SET inventory_id = ("
                    "SELECT misc_in_inventory.id "
                    "FROM misc_in_inventory "
                    "WHERE misc.id = misc_in_inventory.misc_id"
                 ")")},
         // Finally, we update all the kids to have the same inventory_id as their dear old paw
         {QString("UPDATE misc SET inventory_id = ( "
                    "SELECT tmp.inventory_id "
                    "FROM misc tmp, misc_children "
                    "WHERE misc.id = misc_children.child_id "
                    "AND tmp.id = misc_children.parent_id"
                 ") "
                 "WHERE inventory_id IS NULL")},
         //
         // Rearrange inventory - yeast
         //
         {QString("ALTER TABLE yeast ADD COLUMN inventory_id REFERENCES yeast_in_inventory (id)")},
         // It would seem we have kids with their own rows in the db. This is a freaking mess, but I need to delete those rows
         // before I can do anything else.
         {QString("DELETE FROM yeast_in_inventory "
                 "WHERE yeast_in_inventory.id in ( "
                    "SELECT yeast_in_inventory.id "
                    "FROM yeast_in_inventory, yeast_children, yeast "
                    "WHERE yeast.id = yeast_children.child_id "
                    "AND yeast_in_inventory.yeast_id = yeast.id "
                 ")")},
         {QString("INSERT INTO yeast_in_inventory (yeast_id) VALUES ( "
                    // Everything has an inventory row now. This will find all the parent items that don't have an inventory row.
                    "SELECT id FROM yeast WHERE NOT EXISTS ( "
                       "SELECT yeast_children.id "
                       "FROM yeast_children "
                       "WHERE yeast_children.child_id = yeast.id "
                    ") AND NOT EXISTS ( "
                       "SELECT yeast_in_inventory.id "
                       "FROM yeast_in_inventory "
                       "WHERE yeast_in_inventory.yeast_id = yeast.id"
                    ") "
                 ")")},
         // Once we know all parents have inventory rows, we populate inventory_id for them
         {QString("UPDATE yeast SET inventory_id = ("
                    "SELECT yeast_in_inventory.id "
                    "FROM yeast_in_inventory "
                    "WHERE yeast.id = yeast_in_inventory.yeast_id"
                 ")")},
         // Finally, we update all the kids to have the same inventory_id as their dear old paw
         {QString("UPDATE yeast SET inventory_id = ( "
                    "SELECT tmp.inventory_id "
                    "FROM yeast tmp, yeast_children "
                    "WHERE yeast.id = yeast_children.child_id "
                    "AND tmp.id = yeast_children.parent_id"
                 ") "
                 "WHERE inventory_id IS NULL")},
         //
         // We need to drop the appropriate columns from the inventory tables
         // Scary, innit? The changes above basically reverse the relation.
         // Instead of inventory knowing about ingredients, we now have ingredients
         // knowing about inventory. I am concerned that leaving these in place
         // will cause circular references
         //
         // Dropping inventory columns - fermentable
         //
         {QString("CREATE TABLE tmpfermentable_in_inventory (id %1 %2, amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("INSERT INTO tmpfermentable_in_inventory (id, amount) SELECT id, amount FROM fermentable_in_inventory")},
         {QString("DROP TABLE fermentable_in_inventory")},
         {QString("ALTER TABLE tmpfermentable_in_inventory RENAME TO fermentable_in_inventory")},
         //
         // Dropping inventory columns - hop
         //
         {QString("CREATE TABLE tmphop_in_inventory (id %1 %2, amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("INSERT INTO tmphop_in_inventory (id, amount) SELECT id, amount FROM hop_in_inventory")},
         {QString("DROP TABLE hop_in_inventory")},
         {QString("ALTER TABLE tmphop_in_inventory RENAME TO hop_in_inventory")},
         //
         // Dropping inventory columns - misc
         //
         {QString("CREATE TABLE tmpmisc_in_inventory (id %1 %2, amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("INSERT INTO tmpmisc_in_inventory (id, amount) SELECT id, amount FROM misc_in_inventory")},
         {QString("DROP TABLE misc_in_inventory")},
         {QString("ALTER TABLE tmpmisc_in_inventory RENAME TO misc_in_inventory")},
         //
         // Dropping inventory columns - yeast
         //
         {QString("CREATE TABLE tmpyeast_in_inventory (id %1 %2, amount %3 DEFAULT 0);").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier(), db.getDbNativeTypeName<double>())},
         {QString("INSERT INTO tmpyeast_in_inventory (id, amount) SELECT id, amount FROM yeast_in_inventory")},
         {QString("DROP TABLE yeast_in_inventory")},
         {QString("ALTER TABLE tmpyeast_in_inventory RENAME TO yeast_in_inventory")},
         //
         // Finally, the btalltables table isn't needed, so drop it
         //
         {QString("DROP TABLE IF EXISTS bt_alltables")}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   // To support the water chemistry, I need to add two columns to water and to
   // create the salt and salt_in_recipe tables
   bool migrate_to_9(Database & db, QSqlQuery q) {
      QString createSaltSql;
      QTextStream createSaltSqlStream(&createSaltSql);
      createSaltSqlStream <<
         "CREATE TABLE salt ( "
            "id               " << db.getDbNativeTypeName<int>()     << " " << db.getDbNativeIntPrimaryKeyModifier() << ", "
            "addTo            " << db.getDbNativeTypeName<int>()     << "          DEFAULT 0, "
            "amount           " << db.getDbNativeTypeName<double>()  << "          DEFAULT 0, "
            "amount_is_weight " << db.getDbNativeTypeName<bool>()    << "          DEFAULT 1, "
            "deleted          " << db.getDbNativeTypeName<bool>()    << "          DEFAULT 0, "
            "display          " << db.getDbNativeTypeName<bool>()    << "          DEFAULT 1, "
            "folder           " << db.getDbNativeTypeName<QString>() << "          DEFAULT '', "
            "is_acid          " << db.getDbNativeTypeName<bool>()    << "          DEFAULT 0, "
            "name             " << db.getDbNativeTypeName<QString>() << " not null DEFAULT '', "
            "percent_acid     " << db.getDbNativeTypeName<double>()  << "          DEFAULT 0, "
            "stype            " << db.getDbNativeTypeName<int>()     << "          DEFAULT 0, "
            "misc_id          " << db.getDbNativeTypeName<int>()     << ", "
            "FOREIGN KEY(misc_id) REFERENCES misc(id)"
         ");";
      QVector<QueryAndParameters> const migrationQueries{
         {QString("ALTER TABLE water ADD COLUMN wtype      %1 DEFAULT    0").arg(db.getDbNativeTypeName<int>())},
         {QString("ALTER TABLE water ADD COLUMN alkalinity %1 DEFAULT    0").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN as_hco3    %1 DEFAULT true").arg(db.getDbNativeTypeName<bool>())},
         {QString("ALTER TABLE water ADD COLUMN sparge_ro  %1 DEFAULT    0").arg(db.getDbNativeTypeName<double>())},
         {QString("ALTER TABLE water ADD COLUMN mash_ro    %1 DEFAULT    0").arg(db.getDbNativeTypeName<double>())},
         {createSaltSql},
         {QString("CREATE TABLE salt_in_recipe ( "
                    "id        %1 %2, "
                    "recipe_id %1, "
                    "salt_id   %1, "
                    "FOREIGN KEY(recipe_id) REFERENCES recipe(id), "
                    "FOREIGN KEY(salt_id)   REFERENCES salt(id)"
                 ");").arg(db.getDbNativeTypeName<int>(), db.getDbNativeIntPrimaryKeyModifier())}
      };
      return executeSqlQueries(q, migrationQueries);
   }

   bool migrate_to_10(Database & db, QSqlQuery q) {
      QVector<QueryAndParameters> const migrationQueries{
         {QString("ALTER TABLE recipe ADD COLUMN ancestor_id %1 REFERENCES recipe(id)").arg(db.getDbNativeTypeName<int>())},
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
      QSqlQuery sqlQuery(db);
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

   // .:TODO-DATABASE:. Need to look at the default data population stuff below ALSO move repopulate stuff out of Database class into this one
   bool ret = true;
   qDebug() << Q_FUNC_INFO;
   ret &= CreateAllDatabaseTables(database, connection);

   // Create the settings table manually, since it's only used in this file
   QVector<QueryAndParameters> const setUpQueries{
      {QString("CREATE TABLE settings (id %1 %2, repopulatechildrenonnextstart %1, version %1)").arg(database.getDbNativeTypeName<int>(), database.getDbNativeIntPrimaryKeyModifier())},
      {QString("INSERT INTO settings (repopulatechildrenonnextstart, version) VALUES (?, ?)"), {QVariant(true), QVariant(dbVersion)}}

   };
   QSqlQuery sqlQuery{connection};

   ret &= executeSqlQueries(sqlQuery, setUpQueries);

   // If everything went well, we can commit the DB transaction now, otherwise it will abort when this function returns
   if (ret) {
      dbTransaction.commit();
   }

   return ret;
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
   QSqlQuery q("SELECT version FROM settings WHERE id=1", db);
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

void DatabaseSchemaHelper::copyDatabase(Database::DbType oldType, Database::DbType newType, QSqlDatabase connectionNew) {
   Database & oldDatabase = Database::instance(oldType);
   Database & newDatabase = Database::instance(newType);

   // this is to prevent us from over-writing or doing heavens knows what to an existing db
   if( connectionNew.tables().contains(QLatin1String("settings")) ) {
      qWarning() << Q_FUNC_INFO << "It appears the database is already configured.";
      return;
   }

   // The crucial bit is creating the new tables in the new DB.  Once that is done then, assuming disabling of foreign
   // keys works OK, it should be turn-the-handle to run through all the tables and copy each record from old DB to new
   // one.
   if (!CreateAllDatabaseTables(newDatabase, connectionNew)) {
      qCritical() << Q_FUNC_INFO << "Error creating tables in new DB";
      return;
   }

   //
   // Start transaction
   // By the magic of RAII, this will abort if we exit this function (including by throwing an exception) without
   // having called dbTransaction.commit().  (It will also turn foreign keys back on either way -- whether the
   // transaction is committed or rolled back.)
   //
   DbTransaction dbTransaction{newDatabase, connectionNew, DbTransaction::DISABLE_FOREIGN_KEYS};

   QSqlDatabase connectionOld = oldDatabase.sqlDatabase();
   QSqlQuery readOld(connectionOld);
   QSqlQuery upsertNew(connectionNew); // we will prepare this in a bit

   QVector<ObjectStore const *> objectStores = GetAllObjectStores();
   for (ObjectStore const * objectStore : objectStores) {
      QList<QString> tableNames = objectStore->getAllTableNames();
      for (QString tableName : tableNames) {
         QString findAllQuery = QString("SELECT * FROM %1").arg(tableName);
         qDebug() << Q_FUNC_INFO << "FIND ALL:" << findAllQuery;
         if (! readOld.exec(findAllQuery) ) {
            qCritical() << Q_FUNC_INFO << "Error reading record from DB with SQL" << readOld.lastQuery() << ":" << readOld.lastError().text();
            return;
         }

         //
         // We do SELECT * on the old DB table and then look at the records that come back to work out what the INSERT
         // into the new DB table should look like.  Of course, we're assuming that there aren't any secret extra
         // fields on the old DB table, otherwise things will break.  But, all being well, this saves a lot of special-
         // case code either inside ObjectStore or messing with its internal data structures.
         //
         bool upsertQueryCreated{false};
         QString fieldNames;
         QTextStream fieldNamesAsStream{&fieldNames};
         QString bindNames;
         QTextStream bindNamesAsStream{&bindNames};
         QString upsertQuery{"INSERT INTO "};
         QTextStream upsertQueryAsStream{&upsertQuery};

         // Start reading the records from the old db
         while (readOld.next()) {
            QSqlRecord here = readOld.record();
            if (!upsertQueryCreated) {
               // Loop through all the fields in the record.  Order shouldn't matter.
               for (int ii = 0; ii < here.count(); ++ii) {
                  QSqlField field = here.field(ii);
                  if (ii != 0) {
                     fieldNamesAsStream << ", ";
                     bindNamesAsStream << ", ";
                  }
                  fieldNamesAsStream << field.name();
                  bindNamesAsStream << ":" << field.name();
               }
               upsertQueryAsStream << tableName << " (" << fieldNames << ") VALUES (" << bindNames << ");";
               upsertNew.prepare(upsertQuery);
               upsertQueryCreated = true;
            }

            for (int ii = 0; ii < here.count(); ++ii) {
               QSqlField field = here.field(ii);
               QString bindName = QString(":%1").arg(field.name());
               QVariant bindValue = here.value(field.name());
               //
               // QVariant should handle all the problems of different types for us here.  Eg, in SQLite, there is no
               // native bool type, so we'll get back 0 or 1 on a field we store bools in, but this should still
               // convert to the right thing in, say, PostgreSQL, when we try to insert it into a field of type
               // BOOLEAN.
               //
               upsertNew.bindValue(bindName, bindValue);
            }

            if (!upsertNew.exec()) {
               qCritical() <<
                  Q_FUNC_INFO << "Error writing record to DB with SQL" << upsertNew.lastQuery() << ":" <<
                  upsertNew.lastError().text();
               return;
            }
         }
      }
   }

   dbTransaction.commit();
   return;
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

   QString const defaultDataFileName = Brewtarget::getResourceDir().filePath("DefaultData.xml");
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
