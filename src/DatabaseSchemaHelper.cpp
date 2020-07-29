/*
 * DatabaseSchemaHelper.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip G. Lee <rocketman768@gmail.com>
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


#include "brewtarget.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QSqlError>

#include "DatabaseSchemaHelper.h"
#include "TableSchema.h"
#include "TableSchemaConst.h"
#include "BrewnoteSchema.h"
#include "SettingsSchema.h"
#include "WaterSchema.h"

const int DatabaseSchemaHelper::dbVersion = 9;

// Commands and keywords
QString DatabaseSchemaHelper::CREATETABLE("CREATE TABLE");
QString DatabaseSchemaHelper::ALTERTABLE("ALTER TABLE");
QString DatabaseSchemaHelper::DROPTABLE("DROP TABLE");
QString DatabaseSchemaHelper::ADDCOLUMN("ADD COLUMN");
QString DatabaseSchemaHelper::DROPCOLUMN("DROP COLUMN");
QString DatabaseSchemaHelper::UPDATE("UPDATE");
QString DatabaseSchemaHelper::SET("SET");
QString DatabaseSchemaHelper::INSERTINTO("INSERT INTO");
QString DatabaseSchemaHelper::DEFAULT("DEFAULT");
QString DatabaseSchemaHelper::SELECT("SELECT");
QString DatabaseSchemaHelper::SEP(" ");
QString DatabaseSchemaHelper::COMMA(",");
QString DatabaseSchemaHelper::OPENPAREN("(");
QString DatabaseSchemaHelper::CLOSEPAREN(")");
QString DatabaseSchemaHelper::END(";");

QString DatabaseSchemaHelper::UNIQUE("UNIQUE");

bool DatabaseSchemaHelper::upgrade = false;
// Default namespace hides functions from everything outside this file.

bool DatabaseSchemaHelper::create(QSqlDatabase db, DatabaseSchema* defn, Brewtarget::DBTypes dbType )
{
   //--------------------------------------------------------------------------
   // NOTE: if you edit this function, increment dbVersion and edit
   // migrateNext() appropriately.
   //--------------------------------------------------------------------------

   // NOTE: none of the BeerXML property names should EVER change. This is to
   //       ensure backwards compatability when rolling out ingredient updates to
   //       old versions.

   // NOTE: deleted=1 means the ingredient is "deleted" and should not be shown in
   //                 any list.
   //       deleted=0 means it isn't deleted and may or may not be shown.
   //       display=1 means the ingredient should be shown in a list, available to
   //                 be put into a recipe.
   //       display=0 means the ingredient is in a recipe already and should not
   //                 be shown in a list, available to be put into a recipe.

   bool hasTransaction = db.transaction();

   QSqlQuery q(db);
   bool ret = true;

   foreach( TableSchema* table, defn->allTables() ) {
      QString createTable = table->generateCreateTable(dbType);
      if ( ! q.exec(createTable) ) {
         throw QString("Could not create %1 : %2")
               .arg(table->tableName())
               .arg(q.lastError().text());
      }
      // We need to create the increment and decrement things for the instructions_in_recipe table.
      if ( table->dbTable() == Brewtarget::INSTINRECTABLE ) {
         q.exec(table->generateIncrementTrigger(dbType));
         q.exec(table->generateDecrementTrigger(dbType));
      }

   }
   TableSchema* settings = defn->table(Brewtarget::SETTINGTABLE);

   // since we create from scratch, it will always be at the most
   // recent version
   QString insSetting = QString("INSERT INTO settings (%1,%2) values (%3,%4)")
         .arg( settings->propertyToColumn(kpropSettingsRepopulate))
         .arg(settings->propertyToColumn(kpropSettingsVersion))
         .arg(Brewtarget::dbTrue())
         .arg(dbVersion);

   if ( !q.exec(insSetting) ) {
      throw QString("Could not insert into %1 : %2 (%3)")
               .arg(settings->tableName())
               .arg(q.lastError().text())
               .arg(q.lastQuery());
   }
   // Commit transaction
   if( hasTransaction )
      ret = db.commit();

   if ( ! ret ) {
      Brewtarget::logE("db.commit() failed");
   }

   return ret;
}

bool DatabaseSchemaHelper::migrateNext(int oldVersion, QSqlDatabase db )
{
   QSqlQuery q(db);
   bool ret = true;
   DatabaseSchema* defn = new DatabaseSchema();
   TableSchema* tbl = defn->table(Brewtarget::SETTINGTABLE);

   // NOTE: use this to debug your migration
#define CHECKQUERY if(!ret) qDebug() << QString("ERROR: %1\nQUERY: %2").arg(q.lastError().text()).arg(q.lastQuery());

   // NOTE: Add a new case when adding a new schema change
   switch(oldVersion)
   {
      case 1: // == '2.0.0'
         ret &= migrate_to_202(q,defn);
         break;
      case 2: // == '2.0.2'
         ret &= migrate_to_210(q,defn);
         break;
      case 3: // == '2.1.0'
         ret &= migrate_to_4(q,defn);
         break;
      case 4:
         ret &= migrate_to_5(q,defn);
         break;
      case 5:
         ret &= migrate_to_6(q,defn);
         break;
      case 6:
         ret &= migrate_to_7(q,defn);
         break;
      case 7:
         ret &= migrate_to_8(q,defn);
         break;
      case 8:
         ret &= migrate_to_9(q,defn);
         break;
      default:
         Brewtarget::logE(QString("Unknown version %1").arg(oldVersion));
         return false;
   }

   // Set the database version
   if( oldVersion > 3 )
   {
      ret &= q.exec(
         UPDATE + SEP + tbl->tableName() +
         " SET " + tbl->propertyToColumn(kpropSettingsVersion) + "=" + QString::number(oldVersion+1) + " WHERE id=1"
      );
   }

   return ret;
#undef CHECKQUERY
}

bool DatabaseSchemaHelper::migrate(int oldVersion, int newVersion, QSqlDatabase db)
{
   if( oldVersion >= newVersion || newVersion > dbVersion )
   {
      qDebug() << QString("DatabaseSchemaHelper::migrate(%1, %2): You are an imbecile").arg(oldVersion).arg(newVersion);
      return false;
   }

   bool ret = true;

   // turning the foreign_keys on or off can only happen *outside* a
   // transaction boundary. Eeewwww.
   if ( Brewtarget::dbType() == Brewtarget::SQLITE ) {
      db.exec("PRAGMA foreign_keys=off");
   }

   // Start a transaction
   db.transaction();
   for( ; oldVersion < newVersion && ret; ++oldVersion )
      ret &= migrateNext(oldVersion, db);

   if ( Brewtarget::dbType() == Brewtarget::SQLITE ) {
      db.exec("PRAGMA foreign_keys=on");
   }
   // If any statement failed to execute, rollback database to last good state.
   if( ret )
      ret &= db.commit();
   else
   {
      Brewtarget::logE("Rolling back");
      db.rollback();
   }

   return ret;
}

int DatabaseSchemaHelper::currentVersion(QSqlDatabase db)
{
   QVariant ver;
   TableSchema* tbl = new TableSchema(Brewtarget::SETTINGTABLE);
   QSqlQuery q(
      SELECT + SEP + tbl->propertyToColumn(kpropSettingsVersion) + " FROM " + tbl->tableName() + " WHERE id=1",
      db
   );

   // No settings table in version 2.0.0
   if( q.next() )
   {
      int field = q.record().indexOf(tbl->propertyToColumn(kpropSettingsVersion));
      ver = q.value(field);
   }
   else
      ver = QString("2.0.0");

   // Get the string before we kill it by convert()-ing
   QString stringVer( ver.toString() );

   // Initially, versioning was done with strings, so we need to convert
   // the old version strings to integer versions
   if( ver.convert(QVariant::Int) )
      return ver.toInt();
   else
   {
      if( stringVer == "2.0.0" )
         return 1;
      else if( stringVer == "2.0.2" )
         return 2;
      else if( stringVer == "2.1.0" )
         return 3;
   }

   Brewtarget::logE("Could not find database version");
   return -1;
}

// This is when we first defined the settings table, and defined the version as a string.
// In the new world, this will create the settings table and define the version as an int.
// Since we don't set the version until the very last step of the update, I think this will be fine.
bool DatabaseSchemaHelper::migrate_to_202(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   TableSchema *tbl = defn->table(Brewtarget::BREWNOTETABLE);

   // Add "projected_ferm_points" to brewnote table
   ret &= q.exec(
      ALTERTABLE + SEP + tbl->tableName() + SEP +
      ADDCOLUMN + SEP + tbl->propertyToColumn(kpropProjFermPnts) + SEP +
            tbl->propertyColumnType(kpropProjFermPnts) + SEP + DEFAULT + SEP + "0.0"
   );

   ret &= q.exec(
      UPDATE + SEP + tbl->tableName() + SEP +
      SET + SEP + tbl->propertyColumnType(kpropProjFermPnts) + " = -1.0"
   );

   tbl = defn->table(Brewtarget::SETTINGTABLE);

   // Add the settings table
   ret &= q.exec(tbl->generateCreateTable());

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_210(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;

   foreach( TableSchema* tbl, defn->baseTables() ) {
      ret &= q.exec(
               ALTERTABLE + SEP + tbl->tableName() + SEP +
               ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropFolder) + SEP +
               tbl->propertyColumnType(kpropFolder) + SEP + DEFAULT + " ''"
            );
   }

   TableSchema* tbl = defn->table(Brewtarget::RECTABLE);
   // Put the "Bt:.*" recipes into /brewtarget folder
   ret &= q.exec(
      UPDATE + SEP + tbl->tableName() + SEP +
      SET + SEP + tbl->propertyToColumn(kpropFolder) + "='/brewtarget' WHERE name LIKE 'Bt:%'"
   );

   tbl = defn->table(Brewtarget::SETTINGTABLE);
   // Update version to 2.1.0
   ret &= q.exec(
      UPDATE + SEP + tbl->tableName() + SEP + SET + SEP +
            tbl->propertyToColumn(kpropSettingsVersion) + "='2.1.0' WHERE " + tbl->keyName() +"=1"
   );

   // Used to trigger the code to populate the ingredient inheritance tables
   ret &= q.exec(
      ALTERTABLE + SEP + tbl->tableName() + SEP +
      ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropSettingsRepopulate)
                 + SEP + tbl->propertyColumnType(kpropSettingsRepopulate)
   );

   ret &= q.exec(
      UPDATE + SEP + tbl->propertyToColumn(kpropSettingsRepopulate)
             + SEP + tbl->propertyColumnType(kpropSettingsRepopulate) + "=1"
   );

   // Drop and re-create children tables with new UNIQUE requirement
   foreach(TableSchema *dead, defn->childTables()) {
      ret &= q.exec( DROPTABLE + SEP + dead->tableName() );
      if ( ret )
         ret &= q.exec(dead->generateCreateTable());

      if ( ! ret ) {
         throw QString("Could not drop/recreate %1: %2").arg(dead->tableName()).arg(q.lastError().text());
      }
   }

   foreach(TableSchema *dead, defn->inventoryTables()) {
      ret &= q.exec(DROPTABLE + SEP + dead->tableName() );
      if ( ret )
         ret &= q.exec(dead->generateCreateTable());

      if ( ! ret ) {
         throw QString("Could not drop/recreate %1: %2").arg(dead->tableName()).arg(q.lastError().text());
      }
   }

   ret &= q.exec(UPDATE + SEP + tbl->tableName() + " VALUES(1,2)");
   return ret;
}

bool DatabaseSchemaHelper::migrate_to_4(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   TableSchema* tbl = defn->table(Brewtarget::SETTINGTABLE);

   // Save old settings
   ret &= q.exec( ALTERTABLE + SEP + tbl->tableName() + SEP + "RENAME TO oldsettings");

   // create new table with intever version.
   ret &= q.exec( tbl->generateCreateTable() );

   // Update version to 4, saving other settings
   QString copySettings =  INSERTINTO + SEP + tbl->tableName() + SEP +
         QString("(%1,%2,%3)").arg(tbl->keyName())
                            .arg(tbl->propertyToColumn(kpropSettingsVersion))
                            .arg(tbl->propertyToColumn(kpropSettingsRepopulate)) + " " +
         QString("SELECT 1, 4, %1 FROM oldsettings").arg(tbl->propertyToColumn(kpropSettingsRepopulate));
   ret &= q.exec(copySettings);

   // Cleanup
   ret &= q.exec( DROPTABLE + SEP + "oldsettings" );

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_5(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   TableSchema *tbl = defn->table(Brewtarget::INSTINRECTABLE);
   // Drop the previous bugged TRIGGER
   ret &= q.exec( QString("DROP TRIGGER dec_ins_num") );

   // Create the good trigger
   QString trigger = tbl->generateDecrementTrigger( Brewtarget::dbType());
   ret &= q.exec(trigger);

   return ret;
}

//
bool DatabaseSchemaHelper::migrate_to_6(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   // I drop this table in version 8. There is no sense doing anything here, and it breaks other things.
   return ret;
}

bool DatabaseSchemaHelper::migrate_to_7(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   TableSchema* tbl = defn->table(Brewtarget::BREWNOTETABLE);

   // Add "attenuation" to brewnote table
   ret &= q.exec(
      ALTERTABLE + SEP + tbl->tableName() + SEP +
      ADDCOLUMN +  SEP + tbl->propertyToColumn(kpropAtten) +
                   SEP +  tbl->propertyColumnType(kpropAtten) +
                   SEP + DEFAULT + SEP + "0.0"
   );

   return ret;
}

// Note. I think we need to drop this hop_id column (and it's brethern in the other tables)
// when we are done. It is circular and an inventory row never really cares who points to it?
bool DatabaseSchemaHelper::migration_aide_8(QSqlQuery q, DatabaseSchema *defn, Brewtarget::DBTable table )
{
   // get all the tables first
   bool ret = true;
   TableSchema* tbl = defn->table(table);
   TableSchema* cld = defn->childTable(table);
   TableSchema* inv = defn->invTable(table);
   // the inventory indexes are mostly dead, so we cannot rely on the TableSchema
   QString invIdxName = QString("%1_id").arg(tbl->tableName());

   // First, we add the column. It got hard because postgres is sometimes.
   QString addColumn;

   // it would seem we have kids with their own rows in the database. This is a freaking mess, but I need to delete those rows
   // before I can do anything else.
   // delete hop_in_inventory where hop_in_inventory.id in ( select hop_in_inventory.id from hop_in_inventory, hop_children where
   //  hop_children.child_id = hop_in_inventory.hop_id)"
   QString deleteKids = QString( "delete from %1 where %1.%2 in ( select %1.%2 from %1,%3,%4 where %4.%5 = %3.%6 and %1.%7 = %4.%5 )")
         .arg(inv->tableName())
         .arg(inv->keyName())
         .arg(cld->tableName())
         .arg(tbl->tableName())
         .arg(tbl->keyName())
         .arg(cld->childIndexName())
         .arg(invIdxName);

   // Everything has an inventory row now. This will find all the parent items that don't have an inventory row.
   // select hop.id from hop where not exists ( select hop_children.id from hop_children where hop_children.child_id = hop.id ) and
   //       not exists ( select hop_in_inventory.id from hop_in_inventory where hop_in_inventory.hop_id = hop.id );
   QString noInventory = QString( "select %1 from %2 where not exists ( select %3.%4 from %3 where %3.%5 = %2.%1 ) and "
                              "not exists( select %6.%7 from %6 where %6.%8 = %2.%1)")
         .arg(tbl->keyName())
         .arg(tbl->tableName())
         .arg(cld->tableName())
         .arg(cld->keyName())
         .arg(cld->childIndexName())
         .arg(inv->tableName())
         .arg(inv->keyName())
         .arg(invIdxName);

   // Once we know all parents have inventory rows, we populate inventory_id for them
   // update hop set inventory_id = (select hop_in_inventory.id from hop_in_inventory where hop.id = hop_in_inventory.hop_id)
   QString updateParents = QString("UPDATE %1 SET %2 = (SELECT %3.%4 from %3 where %1.%5 = %3.%6)")
         .arg(tbl->tableName())
         .arg(tbl->foreignKeyToColumn())
         .arg(inv->tableName())
         .arg(inv->keyName())
         .arg(tbl->keyName())
         .arg(invIdxName);

   // Finally, we update all the kids to have the same inventory_id as their dear old paw
   // update hop[%1] set inventory_id[%2] = (
   //   select tmp.inventory_id[%2] from hop[%1] tmp, hop_children[%3] where
   //      hop[%1].id[%4] = hop_children[%3].child_id[%5] and
   //      tmp.id[%4] = hop_children[%3].parent_id[%6]
   //   )
   // where inventory_id[%2] is null
   QString updateKids = QString("UPDATE %1 SET %2 = ( "
                                "select tmp.%2 from %1 tmp, %3 where"
                                " %1.%4 = %3.%5 and tmp.%4 = %3.%6"
                                ") where %2 is null")
         .arg(tbl->tableName())            // 1
         .arg(tbl->foreignKeyToColumn())   // 2
         .arg(cld->tableName())            // 3
         .arg(tbl->keyName())              // 4
         .arg(cld->childIndexName())       // 5
         .arg(cld->parentIndexName());     // 6


   // add the column and the constraint. postgres is verbose at times
   if ( Brewtarget::dbType() == Brewtarget::SQLITE ) {
      // sqlite can add the column and the constraint at the same time
      addColumn = QString("ALTER TABLE %1 ADD COLUMN %2 REFERENCES %3 (%4)")
         .arg( tbl->tableName() )
         .arg( tbl->foreignKeyToColumn())
         .arg( inv->tableName() )
         .arg( inv->keyName());
      ret = q.exec(addColumn);
   }
   else if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
      // postgres needs to add the column first
      addColumn = QString("ALTER TABLE %1 ADD COLUMN %2 INTEGER")
         .arg( tbl->tableName() )
         .arg( tbl->foreignKeyToColumn());

      ret = q.exec(addColumn);
      if ( ret ) {
         // and then the constraint
         addColumn = QString("ALTER TABLE %1 ADD CONSTRAINT %2_%3_fk FOREIGN KEY (%4) REFERENCES %2(%3)")
               .arg(tbl->tableName())
               .arg(inv->tableName())
               .arg(inv->keyName())
               .arg(tbl->foreignKeyToColumn());
         ret = q.exec(addColumn);
      }
   }
   if ( !ret ) {
      return ret;
   }

   // remove kids from the inventory row
   ret = q.exec(deleteKids);
   if ( !ret ) {
      return ret;
   }

   // create new inventory rows for parents who have no inventory
   QSqlQuery i(q);
   ret = q.exec(noInventory);

   while ( q.next() ) {
      int idx = q.record().value(tbl->keyName()).toInt();
      // add an inventory row
      // this is weird, but they aren't quite dead yet
      QString newInv = QString("INSERT into %1 (%2) VALUES(%3)")
            .arg(inv->tableName())
            .arg(invIdxName)
            .arg(idx);
      ret = i.exec(newInv);
      if ( !ret ) {
         return ret;
      }
   }

   // now that all parents have inventory rows
   // we can update parents to have inventory_id
   ret = q.exec(updateParents);
   if ( !ret ) {
      return ret;
   }

   // finally, point all the kids to their parent's inventory row
   ret = q.exec(updateKids);

   return ret;
}

bool DatabaseSchemaHelper::drop_columns(QSqlQuery q, TableSchema *tbl, QStringList colNames)
{
   bool ret = true;

   if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
      foreach(QString column, colNames ) {
         ret &= q.exec(
                  ALTERTABLE + SEP + tbl->tableName() + SEP +
                  DROPCOLUMN + SEP + "IF EXISTS " + column
               );
      }
   }
   else {
      // Create a temporary table
      QString tmptable = QString("tmp%1").arg(tbl->tableName());
      QString createTemp = tbl->generateCreateTable(Brewtarget::SQLITE, tmptable);
      ret &= q.exec( createTemp );

      // copy the old to the new, less bad columns
      QString copySql = tbl->generateCopyTable(tmptable, Brewtarget::SQLITE );
      ret &= q.exec( copySql );

      // drop the old
      QString dropOld = QString("drop table %1").arg(tbl->tableName());
      ret &= q.exec(dropOld);

      // rename the new
      QString rename = QString("alter table %1 rename to %2").arg(tmptable).arg(tbl->tableName());
      ret &= q.exec( rename );
   }

   return ret;
}

bool DatabaseSchemaHelper::migrate_to_8(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;

   // these columns are used nowhere I can find and they are breaking things.
   ret = drop_columns(q,defn->table(Brewtarget::BREWNOTETABLE),QStringList() << "predicted_og" << "predicted_abv");

   // Now that we've had that fun, let's have this fun
   Brewtarget::logI(QString("rearranging inventory"));
   ret &= migration_aide_8(q, defn, Brewtarget::FERMTABLE);
   if ( ret )
      ret &= migration_aide_8(q, defn, Brewtarget::HOPTABLE);
   if ( ret )
      ret &= migration_aide_8(q, defn, Brewtarget::MISCTABLE);
   if ( ret )
      ret &= migration_aide_8(q, defn, Brewtarget::YEASTTABLE);

   // We need to drop the appropriate columns from the inventory tables
   // Scary, innit? The changes above basically reverse the relation.
   // Instead of inventory knowing about ingredients, we now have ingredients
   // knowing about inventory. I am concerned that leaving these in place
   // will cause circular references
   Brewtarget::logI(QString("dropping inventory columns"));
   if ( ret ) {
      ret &= drop_columns(q, defn->table(Brewtarget::FERMINVTABLE),  QStringList() << "fermentable_id");
   }
   if ( ret ) {
      ret &= drop_columns(q, defn->table(Brewtarget::HOPINVTABLE),   QStringList() << "hop_id");
   }
   if ( ret ) {
      ret &= drop_columns(q, defn->table(Brewtarget::MISCINVTABLE),  QStringList() << "misc_id");
   }
   if ( ret ) {
      ret &= drop_columns(q, defn->table(Brewtarget::YEASTINVTABLE), QStringList() << "yeast_id");
   }

   // Finally, the btalltables table isn't needed, so drop it
   Brewtarget::logI(QString("dropping bt_alltables"));
   if ( ret )
      ret &= q.exec( DROPTABLE + SEP + "IF EXISTS bt_alltables");

   return ret;
}

// To support the water chemistry, I need to add two columns to water and to
// create the salt and salt_in_recipe tables
bool DatabaseSchemaHelper::migrate_to_9(QSqlQuery q, DatabaseSchema* defn)
{
   bool ret = true;
   TableSchema* tbl = defn->table(Brewtarget::WATERTABLE);
   ret &= q.exec(
            ALTERTABLE + SEP + tbl->tableName() + SEP +
            ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropType) +
                         SEP + tbl->propertyColumnType(kpropType) +
                         SEP + DEFAULT + SEP + tbl->propertyColumnDefault(kpropType).toString()
   );
   ret &= q.exec(
            ALTERTABLE + SEP + tbl->tableName() + SEP +
            ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropAlkalinity) +
                         SEP + tbl->propertyColumnType(kpropAlkalinity) +
                         SEP + DEFAULT + SEP + tbl->propertyColumnDefault(kpropAlkalinity).toString()
   );
   ret &= q.exec(
            ALTERTABLE + SEP + tbl->tableName() + SEP +
            ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropAsHCO3) +
                         SEP + tbl->propertyColumnType(kpropAsHCO3) +
                         SEP + DEFAULT + SEP + tbl->propertyColumnDefault(kpropAsHCO3).toString()
   );
   ret &= q.exec(
            ALTERTABLE + SEP + tbl->tableName() + SEP +
            ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropSpargeRO) +
                         SEP + tbl->propertyColumnType(kpropSpargeRO) +
                         SEP + DEFAULT + SEP + tbl->propertyColumnDefault(kpropSpargeRO).toString()
   );
   ret &= q.exec(
            ALTERTABLE + SEP + tbl->tableName() + SEP +
            ADDCOLUMN  + SEP + tbl->propertyToColumn(kpropMashRO) +
                         SEP + tbl->propertyColumnType(kpropMashRO) +
                         SEP + DEFAULT + SEP + tbl->propertyColumnDefault(kpropMashRO).toString()
   );
   ret &= q.exec(defn->generateCreateTable(Brewtarget::SALTTABLE));
   ret &= q.exec(defn->generateCreateTable(Brewtarget::SALTINRECTABLE));

   return ret;
}
