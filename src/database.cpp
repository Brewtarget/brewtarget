/*
 * database.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "database.h"

#include <QList>
#include <iostream>
#include <fstream>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QTextCodec>
#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>

#include "Algorithms.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "yeast.h"

#include "config.h"
#include "brewtarget.h"

// Static members.
QFile Database::dbFile;
QString Database::dbFileName;
QHash<DBTable,QString> Database::tableNames = tableNamesHash();

Database::Database()
{
   QFile dataDbFile;
   QString dataDbFileName;
   bool dbIsOpen;

   commandStack.setUndoLimit(100);
   
   load();
}

void Database::load()
{
   // Set file names.
   dbFileName = (Brewtarget::getUserDataDir() + "database.sqlite");
   dbFile.setFilename(dbFileName);
   dataDbFileName = (Brewtarget::getDataDir() + "database.sqlite");
   dataDbFile.setFilename(dataDbFileName);
   
   // Open SQLite db.
   // http://www.developer.nokia.com/Community/Wiki/CS001504_-_Creating_an_SQLite_database_in_Qt
   sqldb.close();
   sqldb = QSqlDatabase::addDatabase("QSQLITE");
   sqldb.setDatabaseName(dbFileName);
   dbIsOpen = sqldb.open();
   if( ! dbIsOpen )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.").arg(dbFileName));
      // TODO: if we can't open the database, what should we do?
      return;
   }
   
   // See if there are new ingredients that we need to merge from the data-space db.
   if( dataDbFile.fileName() != dbFile.fileName()
      && ! Brewtarget::userDatabaseDidNotExist // Don't do this if we JUST copied the dataspace database.
      && QFileInfo(dataDbFile).lastModified() > Brewtarget::lastDbMergeRequest )
   {
      
      if( QMessageBox::question(0,
         QObject::tr("Merge Database"),
                                QObject::tr("There may be new ingredients and recipes available. Would you like to add these to your database?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes)
         == QMessageBox::Yes
         )
      {
         // TODO: implement merging here.
      }
      
      // Update this field.
      Brewtarget::lastDbMergeRequest = QDateTime::currentDateTime();
   }
   
   // Set up the tables.
   tableModel = QSqlTableModel( 0, sqldb );
   tables.clear();
   
   equipments_tm = tableModel;
   equipments_tm.setTable(tableNames[EQUIPTABLE]);
   tables[EQUIPTABLE] = &equipments_tm;
   
   fermentables_tm = tableModel;
   fermentables_tm.setTable(tableNames[FERMTABLE]);
   tables[FERMTABLE] = &fermentables_tm;
   
   hops_tm = tableModel;
   hops_tm.setTable(tableNames[HOPTABLE]);
   tables[HOPTABLE] = &hops_tm;
   
   instructions_tm = tableModel;
   instructions_tm.setTable(tableNames[INSTRUCTIONTABLE]);
   tables[INSTRUCTIONTABLE] = &instructions_tm;
   
   mashs_tm = tableModel;
   mashs_tm.setTable(tableNames[MASHTABLE]);
   tables[MASHTABLE] = &mashs_tm;
   
   mashSteps_tm = tableModel;
   mashSteps_tm.setTable(tableNames[MASHSTEPTABLE]);
   tables[MASHSTEPTABLE] = &mashSteps_tm;
   
   miscs_tm = tableModel;
   miscs_tm.setTable(tableNames[MISCTABLE]);
   tables[MISCTABLE] = &miscs_tm;
   
   recipes_tm = tableModel;
   recipes_tm.setTable(tableNames[RECTABLE]);
   tables[RECTABLE] = &recipes_tm;
   
   styles_tm = tableModel;
   styles_tm.setTable(tableNames[STYLETABLE]);
   tables[STYLETABLE] = &styles_tm;
   
   waters_tm = tableModel;
   waters_tm.setTable(tableNames[WATERTABLE]);
   tables[WATERTABLE] = &waters_tm;
   
   yeasts_tm = tableModel;
   yeasts_tm.setTable(tableNames[YEASTTABLE]);
   tables[YEASTTABLE] = &yeasts_tm;
   
   // TODO: set relations?
   
   // Create and store all pointers.
   populateElements( allEquipments, equipments_tm, EQUIPTABLE );
   populateElements( allFermentables, fermentables_tm, FERMTABLE );
   populateElements( allHops, hops_tm, HOPTABLE );
   populateElements( allInstructions, instructions_tm, INSTRUCTIONTABLE );
   populateElements( allMashs, mashs_tm, MASHTABLE );
   populateElements( allMashSteps, mashSteps_tm, MASHSTEPTABLE );
   populateElements( allMiscs, miscs_tm, MISCTABLE );
   populateElements( allRecipes, recipes_tm, RECTABLE );
   populateElements( allStyles, styles_tm, STYLETABLE );
   populateElements( allWaters, waters_tm, WATERTABLE );
   populateElements( allYeasts, yeasts_tm, YEASTTABLE );
}

Database& Database::instance()
{
   static Database dbSingleton;
      return dbSingleton;
}

bool Database::backupToDir(QString dir)
{
   // Make sure the singleton exists.
   instance();
   
   bool success = true;
   QString prefix = dir + "/";
   QString newDbFileName = prefix + "database.sqlite";
   
   // Remove the files if they already exist so that
   // the copy() operation will succeed.
   QFile::remove(newDbFileName);
   
   success = dbFile.copy( newDbFileName );
   
   return success;
}

bool Database::restoreFromDir(QString dirStr)
{
   bool success = true;
   
   // Ensure singleton exists.
   instance();
   
   QString prefix = dirStr + "/";
   QString newDbFileName = prefix + "database.sqlite";
   QFile newDbFile(newDbFileName);
   
   // Fail if we can't find file.
   if( !newDbFile.exists() )
      return false;
   
   // TODO: there are probably concurrency issues here. What if a query happens
   // between these two lines?
   success &= dbFile.remove();   
   success &= newDbFile.copy(dbFile.fileName());
   
   // Reload everything.
   instance().load();
   
   return success;
}

/*
Equipment* Database::equipment(int key)
{
}

Fermentable* Database::fermentable(int key)
{
}

Hop* Database::hop(int key)
{
}

Mash* Database::mash(int key)
{
}

MashStep* Database::mashStep(int key)
{
}

Misc* Database::misc(int key)
{
}

Recipe* Database::recipe(int key)
{
}

Style* Database::style(int key)
{
}

Water* Database::water(int key)
{
}

Yeast* Database::yeast(int key)
{
}
*/

void removeFromRecipe( Recipe* rec, Hop* hop )
{
   removeIngredientFromRecipe( rec, hop, "hops", "hop_in_recipe", "hop_id" );
}

void removeFromRecipe( Recipe* rec, Fermentable* ferm )
{
   removeIngredientFromRecipe( rec, ferm, "fermentables", "fermentable_in_recipe", "fermentable_id" );
}

void removeFromRecipe( Recipe* rec, Misc* m )
{
   removeIngredientFromRecipe( rec, m, "miscs", "misc_in_recipe", "misc_id" );
}

void removeFromRecipe( Recipe* rec, Yeast* y )
{
   removeIngredientFromRecipe( rec, y, "yeasts", "yeast_in_recipe", "yeast_id" );
}

void removeFromRecipe( Recipe* rec, Water* w )
{
   removeIngredientFromRecipe( rec, w, "waters", "water_in_recipe", "water_id" );
}

void removeFromRecipe( Recipe* rec, Instruction* ins )
{
   // TODO: encapsulate in QUndoCommand.
   // NOTE: is this the right thing to do?
   QSqlQuery q( QString("SELECT * FROM instruction WHERE iid = %1").arg(ins->key),
                sqldb );
   if( q.next() )
      q.record().setValue( "deleted", true );
}


int Database::insertNewRecord( DBTable table )
{
   // TODO: encapsulate this in a QUndoCommand so we can undo it.
   tableModel.setTable(tableNames[table]);
   tableModel.insertRow(-1,tableModel.record());
   return tableModel.query().lastInsertId().toInt();
}

Equipment* Database::newEquipment()
{
   Equipment* tmp = new Equipment();
   tmp->_key = insertNewRecord(EQUIPTABLE);
   tmp->_table = EQUIPTABLE;
   allEquipments.insert(tmp->_key,tmp);
   return tmp;
}

Fermentable* Database::newFermentable()
{
   Fermentable* tmp = new Fermentable();
   tmp->_key = insertNewRecord(FERMTABLE);
   tmp->_table = FERMTABLE;
   allFermentables.insert(tmp->_key,tmp);
   return tmp;
}

Hop* Database::newHop()
{
   Hop* tmp = new Hop();
   tmp->_key = insertNewRecord(HOPTABLE);
   tmp->_table = HOPTABLE;
   allHops.insert(tmp->_key,tmp);
   return tmp;
}

Instruction* newInstruction(Recipe* rec)
{
   // TODO: encapsulate in QUndoCommand.
   Instruction* tmp = new Instruction();
   tmp->_key = insertNewRecord(INSTRUCTIONTABLE);
   tmp->_table = INSTRUCTIONTABLE;
   QSqlQuery q( QString("SELECT * FROM instruction WHERE iid = %1").arg(tmp->key),
                sqldb );
   q.next();
   q.record().setValue( "recipe_id", rec->key );
   allInstructions.insert(tmp->_key,tmp);
   return tmp;
}

Mash* Database::newMash()
{
   Mash* tmp = new Mash();
   tmp->_key = insertNewRecord(MASHTABLE);
   tmp->_table = MASHTABLE;
   allMashs.insert(tmp->_key,tmp);
   return tmp;
}

MashStep* Database::newMashStep()
{
   MashStep* tmp = new MashStep();
   tmp->_key = insertNewRecord(MASHSTEPTABLE);
   tmp->_table = MASHSTEPTABLE;
   allMashSteps.insert(tmp->_key,tmp);
   return tmp;
}

Misc* Database::newMisc()
{
   Misc* tmp = new Misc();
   tmp->_key = insertNewRecord(MISCTABLE);
   tmp->_table = MISCTABLE;
   allMiscs.insert(tmp->_key,tmp);
   return tmp;
}

Recipe* Database::newRecipe()
{
   Recipe* tmp = new Recipe();
   tmp->_key = insertNewRecord(RECTABLE);
   tmp->_table = RECTABLE;
   allRecipes.insert(tmp->_key,tmp);
   return tmp;
}

Style* Database::newStyle()
{
   Style* tmp = new Style();
   tmp->_key = insertNewRecord(STYLETABLE);
   tmp->_table = STYLETABLE;
   allStyles.insert(tmp->_key,tmp);
   return tmp;
}

Water* Database::newWater()
{
   Water* tmp = new Water();
   tmp->_key = insertNewRecord(WATERTABLE);
   tmp->_table = WATERTABLE;
   allWaters.insert(tmp->_key,tmp);
   return tmp;
}

Yeast* Database::newYeast()
{
   Yeast* tmp = new Yeast();
   tmp->_key = insertNewRecord(YEASTTABLE);
   tmp->_table = YEASTTABLE;
   allYeasts.insert(tmp->_key,tmp);
   return tmp;
}

int Database::deleteRecord( DBTable table, BeerXMLElement* object )
{
   // Assumes the table has a column called 'deleted'.
   SetterCommand command(tables[table], keyName(table), key, "deleted", true, object->property("deleted"), object);
   // For now, immediately execute the command.
   command.redo();
   // Push the command on the undo stack.
   commandStack.push(command);
}

void Database::removeEquipment(Equipment* equip)
{
   deleteRecord(EQUIPTABLE,equip);
}

void Database::removeEquipment(QList<Equipment*> equip)
{
   QList<Equipment*>::Iterator it = equip.begin();
   while( it != equip.end() )
   {
      removeEquipment(*it);
      it++;
   }
}

void Database::removeFermentable(Fermentable* ferm)
{
   deleteRecord(FERMTABLE,ferm);
}

void Database::removeFermentable(QList<Fermentable*> ferm)
{
   QList<Fermentable*>::Iterator it = ferm.begin();
   while( it != ferm.end() )
   {
      removeFermentable(*it);
      it++;
   }
}

void Database::removeHop(Hop* hop)
{
   deleteRecord(HOPTABLE,hop);
}

void Database::removeHop(QList<Hop*> hop)
{
   QList<Hop*>::Iterator it = hop.begin();
   while( it != hop.end() )
   {
      removeHop(*it);
      it++;
   }
}

void Database::removeMash(Mash* mash)
{
   deleteRecord(MASHTABLE,mash);
}

void Database::removeMash(QList<Mash*> mash)
{
   QList<Mash*>::Iterator it = mash.begin();
   while( it != mash.end() )
   {
      removeMash(*it);
      it++;
   }
}

void Database::removeMashStep(MashStep* mashStep)
{
   deleteRecord(MASHSTEPTABLE,mashStep);
}

void Database::removeMashStep(QList<MashStep*> mashStep)
{
   QList<MashStep*>::Iterator it = mashStep.begin();
   while( it != mashStep.end() )
   {
      removeMashStep(*it);
      it++;
   }
}

void Database::removeMisc(Misc* misc)
{
   deleteRecord(MISCTABLE,misc);
}

void Database::removeMisc(QList<Misc*> misc)
{
   QList<Misc*>::Iterator it = misc.begin();
   while( it != misc.end() )
   {
      removeMisc(*it);
      it++;
   }
}

void Database::removeRecipe(Recipe* rec)
{
   deleteRecord(RECTABLE,rec);
}

void Database::removeRecipe(QList<Recipe*> rec)
{
   QList<Recipe*>::Iterator it = rec.begin();
   while( it != rec.end() )
   {
      removeRecipe(*it);
      it++;
   }
}

void Database::removeStyle(Style* style)
{
   deleteRecord(STYLETABLE,style);
}

void Database::removeStyle(QList<Style*> style)
{
   QList<Style*>::Iterator it = style.begin();
   while( it != style.end() )
   {
      removeStyle(*it);
      it++;
   }
}

void Database::removeWater(Water* water)
{
   deleteRecord(WATERTABLE,water);
}

void Database::removeWater(QList<Water*> water)
{
   QList<Water*>::Iterator it = water.begin();
   while( it != water.end() )
   {
      removeWater(*it);
      it++;
   }
}

void Database::removeYeast(Yeast* yeast)
{
   deleteRecord(YEASTTABLE,yeast);
}

void Database::removeYeast(QList<Yeast*> yeast)
{
   QList<Yeast*>::Iterator it = yeast.begin();
   while( it != yeast.end() )
   {
      removeYeast(*it);
      it++;
   }
}

QString Database::getDbFileName()
{
   // Ensure instance exists.
   instance();
   
   return dbFileName;
}

void Database::updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object )
{
   SetterCommand command(tables[table], keyName(table), key, col_name, value, prop, object);
   // For now, immediately execute the command.
   command.redo();
   
   // Push the command on the undo stack.
   //commandStack.beginMacro("Change an entry");
   commandStack.push(command);
   //commandStack.endMacro();
}

QString Database::keyName( DBTable table )
{
   return tables[table].primaryKey().name();
}

void Database::removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName )
{
   // TODO: encapsulate this in a QUndoCommand.
   
   tableModel.setTable(relTableName);
   QString filter = tableModel.filter();
   
   // Find the row in the relational db that connects the ingredient to the recipe.
   tableModel.setFilter( QString("%1=%2 AND recipe_id=%3").arg(ingKeyName).arg(ing->key).arg(rec->key) );
   tableModel.select();
   if( tableModel.rowCount() > 0 )
      tableModel.removeRows(0,1);
   
   // Restore the old filter.
   tableModel.setFilter(filter);
   tableModel.select();
   
   emit rec->changed( rec->property(propName) );
}

void Database::addIngredientToRecipe( BeerXMLElement* ing, Recipe* rec, QString propName, QString relTableName, QString ingKeyName )
{
   // TODO: encapsulate this in a QUndoCommand.
   
   tableModel.setTable(relTableName);
   
   // Ensure this ingredient is not already in the recipe.
   /*
   QSqlQuery q(
      QString("SELECT * from %1 WHERE %2 = %3 AND recipe_id = %4")
        .arg(relTableName).arg(ingKeyName).arg(ing->key).arg(rec->key), sqldb);
   if( q.next() )
   {
      Brewtarget::logW( "Ingredient already exists in recipe." );
      return;
   }
   */
   
   QString filter = tableModel.filter();
   
   // Ensure this ingredient is not already in the recipe.
   tableModel.setFilter(QString("%1=%2 AND recipe_id=%3").arg(ingKeyName).arg(ing->key).arg(rec->key));
   tableModel.select();
   if( tableModel.rowCount() > 0 )
   {
      Brewtarget::logW( "Ingredient already exists in recipe." );
      return;
   }
   
   tableModel.setFilter(filter);
   tableModel.select();
   
   // Put this (rec,hop) pair in the hop_in_recipe table.
   QSqlRecord r = tableModel.record();
   r.setValue(ingKeyName, ing->key);
   r.setValue("recipe_id", rec->key);
   tableModel.insert(-1,r);
   
   emit rec->changed( rec->property(propName) );
}

void addToRecipe( Recipe* rec, Hop* hop )
{
   addIngredientToRecipe( rec, hop, "hops", "hop_in_recipe", "hop_id" );
}

void addToRecipe( Recipe* rec, Fermentable* ferm )
{
   addIngredientToRecipe( rec, ferm, "ferms", "fermentable_in_recipe", "fermentable_id" );
}

void addToRecipe( Recipe* rec, Misc* m )
{
   addIngredientToRecipe( rec, m, "miscs", "misc_in_recipe", "misc_id" );
}

void addToRecipe( Recipe* rec, Yeast* y )
{
   addIngredientToRecipe( rec, y, "yeasts", "yeast_in_recipe", "yeast_id" );
}

void addToRecipe( Recipe* rec, Water* w )
{
   addIngredientToRecipe( rec, w, "waters", "water_in_recipe", "water_id" );
}

void Database::getEquipments( QList<Equipment*>& list, QString filter )
{
   getElements( list, filter, equipments_tm, EQUIPTABLE, allEquipments );
}

void Database::getFermentables( QList<Fermentable*>& list, QString filter )
{
   getElements( list, filter, fermentables_tm, FERMTABLE, allFermentables );
}

void Database::getHops( QList<Hop*>& list, QString filter )
{
   getElements( list, filter, hops_tm, HOPTABLE, allHops );
}

void Database::getMashs( QList<Mash*>& list, QString filter )
{
   getElements( list, filter, mashs_tm, MASHTABLE, allMashs );
}

void Database::getMashSteps( QList<MashStep*>& list, QString filter )
{
   getElements( list, filter, mashsteps_tm, MASHSTEPTABLE, allMashSteps );
}

void Database::getMiscs( QList<Misc*>& list, QString filter )
{
   getElements( list, filter, miscs_tm, MISCTABLE, allMiscs );
}

void Database::getRecipes( QList<Recipe*>& list, QString filter )
{
   getElements( list, filter, recipes_tm, RECTABLE, allRecipes );
}

void Database::getStyles( QList<Style*>& list, QString filter )
{
   getElements( list, filter, styles_tm, STYLETABLE, allStyles );
}

void Database::getWaters( QList<Water*>& list, QString filter )
{
   getElements( list, filter, waters_tm, WATERTABLE, allWaters );
}

void Database::getYeasts( QList<Yeast*>& list, QString filter )
{
   getElements( list, filter, yeasts_tm, YEASTTABLE, allYeasts );
}

QHash<DBTable,QString> Database::tableNamesHash()
{
   QHash<DBTable,QString> tmp;
   
   tmp[ BREWNOTETABLE ] = "brewnote"
   tmp[ EQUIPTABLE ] = "equipment";
   tmp[ FERMTABLE ] = "fermentable";
   tmp[ HOPTABLE ] = "hop";
   tmp[ INSTRUCTIONTABLE ] = "instruction";
   tmp[ MASHSTEPTABLE ] = "mashstep";
   tmp[ MASHTABLE ] = "mash";
   tmp[ MISCTABLE ] = "misc";
   tmp[ RECTABLE ] = "recipe";
   tmp[ STYLETABLE ] = "style";
   tmp[ WATERTABLE ] = "water";
   tmp[ YEASTTABLE ] = "yeast";
   
   return tmp;
}

const Database::QSqlRelationalTableModel getModel( DBTable table )
{
   return *(tables[table]);
}

// Do this to pacify the READ in Q_PROPERTY.
QList<Equipment*>& Database::equipments()
{
   QList<Equipment*>* tmp = new QList<Equipment*>;
   getEquipments( *tmp );
   return *tmp;
}

QList<Fermentable*>& Database::fermentables()
{
   QList<Fermentable*>* tmp = new QList<Fermentable*>;
   getFermentables( *tmp );
   return *tmp;
}

QList<Hop*>& Database::hops()
{
   QList<Hop*>* tmp = new QList<Hop*>;
   getHops( *tmp );
   return *tmp;
}

QList<Mash*>& Database::mashs()
{
   QList<Mash*>* tmp = new QList<Mash*>;
   getMashs( *tmp );
   return *tmp;
}

QList<MashStep*>& Database::mashSteps()
{
   QList<MashStep*>* tmp = new QList<MashStep*>;
   getMashSteps( *tmp );
   return *tmp;
}

QList<Misc*>& Database::miscs()
{
   QList<Misc*>* tmp = new QList<Misc*>;
   getMiscs( *tmp );
   return *tmp;
}

QList<Recipe*>& Database::recipes()
{
   QList<Recipe*>* tmp = new QList<Recipe*>;
   getRecipes( *tmp );
   return *tmp;
}

QList<Style*>& Database::styles()
{
   QList<Style*>* tmp = new QList<Style*>;
   getStyles( *tmp );
   return *tmp;
}

QList<Water*>& Database::waters()
{
   QList<Water*>* tmp = new QList<Water*>;
   getWaters( *tmp );
   return *tmp;
}

QList<Yeast*>& Database::yeasts()
{
   QList<Yeast*>* tmp = new QList<Yeast*>;
   getYeasts( *tmp );
   return *tmp;
}