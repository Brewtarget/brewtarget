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
#include <QSqlQuery>
#include <QSqlIndex>
#include <QSqlError>
#include <QThread>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QPushButton>

#include "Algorithms.h"
#include "brewnote.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "instruction.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "yeast.h"

#include "config.h"
#include "brewtarget.h"
#include "QueuedMethod.h"
#include "SetterCommand.h"
#include "SetterCommandStack.h"

// Static members.
Database* Database::dbInstance = 0;
QFile Database::dbFile;
QString Database::dbFileName;
QFile Database::dataDbFile;
QString Database::dataDbFileName;
QFile Database::dbTempBackupFile;
QString Database::dbTempBackupFileName;
QString Database::dbConName;
QHash<Brewtarget::DBTable,QString> Database::tableNames = Database::tableNamesHash();
QHash<QString,Brewtarget::DBTable> Database::classNameToTable = Database::classNameToTableHash();
const QList<TableParams> Database::tableParams = Database::makeTableParams();

QHash< QThread*, QString > Database::_threadToConnection;
QMutex Database::_threadToConnectionMutex;

Database::Database()
   : //_setterCommandStack( new SetterCommandStack() ),
     skipEmitChanged(false), needRecalc(true)
{
   //.setUndoLimit(100);
   // Lock this here until we actually construct the first database connection.
   _threadToConnectionMutex.lock();
   
   loadWasSuccessful = load();
}

Database::~Database()
{
   unload();
}

bool Database::load()
{
   bool dbIsOpen;
   bool createFromScratch = false;
   
   // Set file names.
   dbFileName = (Brewtarget::getUserDataDir() + "database.sqlite");
   dataDbFileName = (Brewtarget::getDataDir() + "database.sqlite");
   dbTempBackupFileName = (Brewtarget::getUserDataDir() + "tempBackupDatabase.sqlite");
   
   // Cleanup the backup database if there was a previous error.
   if( !cleanupBackupDatabase() )
      return false;

   // Set the files.
   dbFile.setFileName(dbFileName);
   dataDbFile.setFileName(dataDbFileName);
   dbTempBackupFile.setFileName(dbTempBackupFileName);
   
   // If there's no dbFile, try to copy from dataDbFile.
   if( !dbFile.exists() )
   {
      // If there's no dataDbFile, create dbFile from scratch.
      if( !dataDbFile.exists() )
         createFromScratch = true;
      else
         dataDbFile.copy(dbFileName);
   }

   // NOTE: I don't think this will be necessary anymore, but leaving it
   // commented just in case.
   // Sometimes, the database file gets created but is empty. This fixes that
   // problem
   //if ( dbFile.size() == 0 )
   //   createFromScratch = true;
   
   // If we need to create from scratch, do it.
   if( createFromScratch )
   {
      QFile::copy( ":/blankdb.sqlite", dbFileName );
      QFile::setPermissions( dbFileName, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup );
   }

   // Create a copy of the database to revert to if the user decides not to make changes.
   dbFile.copy(dbTempBackupFileName);
   
   // Open SQLite db.
   // http://www.developer.nokia.com/Community/Wiki/CS001504_-_Creating_an_SQLite_database_in_Qt
   QSqlDatabase sqldb = QSqlDatabase::addDatabase("QSQLITE");
   sqldb.setDatabaseName(dbFileName);
   dbIsOpen = sqldb.open();
   dbConName = sqldb.connectionName();
   if( ! dbIsOpen )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
      QMessageBox::critical(0,
                            QObject::tr("Database Failure"),
                            QString(QObject::tr("Failed to open the database '%1'.").arg(dbFileName)));

      // TODO: if we can't open the database, what should we do?
      return false;
   }
   // Associate this db with the current thread.
   _threadToConnection.insert(QThread::currentThread(), sqldb.connectionName());
   _threadToConnectionMutex.unlock();
   
   // NOTE: synchronous=off reduces query time by an order of magnitude!
   QSqlQuery( "PRAGMA synchronous = off", sqlDatabase());
   QSqlQuery( "PRAGMA foreign_keys = on", sqlDatabase());
   QSqlQuery( "PRAGMA locking_mode = EXCLUSIVE", sqlDatabase());
   // Store temporary tables in memory.
   QSqlQuery( "PRAGMA temp_store = MEMORY", sqlDatabase());
   
   // Initialize the SELECT * query hashes.
   selectAll = Database::selectAllHash();
   
   // See if there are new ingredients that we need to merge from the data-space db.
   if( dataDbFile.fileName() != dbFile.fileName()
      && ! Brewtarget::userDatabaseDidNotExist // Don't do this if we JUST copied the dataspace database.
      && QFileInfo(dataDbFile).lastModified() > Brewtarget::lastDbMergeRequest )
   {
      
      if(
         QMessageBox::question(
            0,
            tr("Merge Database"),
            tr("There may be new ingredients and recipes available. Would you like to add these to your database?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
         )
         == QMessageBox::Yes
      )
      {
         updateDatabase(dataDbFile.fileName());
      }
      
      // Update this field.
      Brewtarget::lastDbMergeRequest = QDateTime::currentDateTime();
   }
   
   // Create and store all pointers.
   populateElements( allBrewNotes, Brewtarget::BREWNOTETABLE );
   populateElements( allEquipments, Brewtarget::EQUIPTABLE );
   populateElements( allFermentables, Brewtarget::FERMTABLE );
   populateElements( allHops, Brewtarget::HOPTABLE );
   populateElements( allInstructions, Brewtarget::INSTRUCTIONTABLE );
   populateElements( allMashs, Brewtarget::MASHTABLE );
   populateElements( allMashSteps, Brewtarget::MASHSTEPTABLE );
   populateElements( allMiscs, Brewtarget::MISCTABLE );
   populateElements( allStyles, Brewtarget::STYLETABLE );
   populateElements( allWaters, Brewtarget::WATERTABLE );
   populateElements( allYeasts, Brewtarget::YEASTTABLE );
   
   populateElements( allRecipes, Brewtarget::RECTABLE );
   
   // Connect fermentable,hop changed signals to their parent recipe.
   QHash<int,Recipe*>::iterator i;
   QList<Fermentable*>::iterator j;
   QList<Hop*>::iterator k;
   QList<Mash*>::iterator l;
   QList<MashStep*>::iterator m;


   for( i = allRecipes.begin(); i != allRecipes.end(); i++ )
   {
      QList<Fermentable*> tmpF = fermentables(*i);
      for( j = tmpF.begin(); j != tmpF.end(); j++ )
         connect( *j, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
      
      QList<Hop*> tmpH = hops(*i);
      for( k = tmpH.begin(); k != tmpH.end(); ++k )
         connect( *k, SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptHopChange(QMetaProperty,QVariant)) );
      
      connect( mash(*i), SIGNAL(changed(QMetaProperty,QVariant)), *i, SLOT(acceptMashChange(QMetaProperty,QVariant)) );
   }
   
   QList<Mash*> tmpM = mashs();
   for( l = tmpM.begin(); l != tmpM.end(); ++l)
   {
      QList<MashStep*> tmpMS = mashSteps(*l);
      for( m=tmpMS.begin(); m != tmpMS.end(); ++m)
         connect( *m, SIGNAL(changed(QMetaProperty,QVariant)), *l, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );
   }

   return true;
}

bool Database::loadSuccessful()
{
   return loadWasSuccessful;
}

QSqlDatabase Database::sqlDatabase()
{
   // Need a unique database connection for each thread.
   //http://www.linuxjournal.com/article/9602
   
   QThread* t = QThread::currentThread();
   
   _threadToConnectionMutex.lock();
   // If this thread already has a connection, return it.
   if( _threadToConnection.contains(t) )
   {
      QSqlDatabase ret = QSqlDatabase::database(_threadToConnection[t]);
      _threadToConnectionMutex.unlock();
      return ret;
   }
   // Create a unique connection name, just containing the addy of the thread.
   QString conName = QString("0x%1").arg(reinterpret_cast<uintptr_t>(t), 0, 16);
   
   // Create the new connection.
   QSqlDatabase sqldb = QSqlDatabase::addDatabase("QSQLITE",conName);
   sqldb.setDatabaseName(dbFileName);
   if( ! sqldb.open() )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(dbFileName).arg(sqldb.lastError().text()));
      // TODO: what to do if we can't open?
   }
   
   // Put new connection in the hash.
   _threadToConnection.insert(t,conName);
   _threadToConnectionMutex.unlock();
   return sqldb;
}

void Database::unload()
{
   // Delete all the ingredients floating around.
   qDeleteAll(allBrewNotes);
   qDeleteAll(allEquipments);
   qDeleteAll(allFermentables);
   qDeleteAll(allHops);
   qDeleteAll(allInstructions);
   qDeleteAll(allMashSteps);
   qDeleteAll(allMashs);
   qDeleteAll(allMiscs);
   qDeleteAll(allStyles);
   qDeleteAll(allWaters);
   qDeleteAll(allYeasts);
   qDeleteAll(allRecipes);

   QSqlDatabase::database( dbConName, false ).close();
   QSqlDatabase::removeDatabase( dbConName );

   if (!loadWasSuccessful)
   {
      // If load() failed, then
      // just keep the database and don't revert to the backup.
      if (dbFile.exists())
         dbTempBackupFile.remove();
   }
   else 
   {
      // If the database has a more recent modified date than the backup, ask
      // the user if they want to save changes.
      if (QFileInfo(dbFileName).lastModified() > QFileInfo(dbTempBackupFileName).lastModified() && 
            QMessageBox::question(0,
                                  QObject::tr("Save Database Changes"),
                                  QObject::tr("Would you like to save the changes you made?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes) == QMessageBox::Yes)
      {
         // If the user wants to save changes, just remove the backup database.
         dbTempBackupFile.remove();
      }
      else
      {
         // If the user doesn't want to save changes, remove the active database
         // and restore the backup.
         dbFile.remove();
         dbTempBackupFile.rename(dbFileName);
      }
   }
}

Database& Database::instance()
{
   
   // Not thread-safe
   //static Database dbSingleton;
   //return dbSingleton;
   
   // This is not safe either. This is the double-check pattern that
   // avoids acquiring the lock unless we need to make a new instance.
   // The problem is that it's not safe. Should replace this lazy
   // initialization with eager initialization, or just do a single
   // check lock.
   // http://www.aristeia.com/Papers/DDJ_Jul_Aug_2004_revised.pdf
   static QMutex mutex;
   if( ! dbInstance )
   {
      mutex.lock();
      
      if( ! dbInstance )
         dbInstance = new Database();
      
      mutex.unlock();
   }
   
   return *dbInstance;
}

void Database::dropInstance()
{
   static QMutex mutex;
   
   mutex.lock();
   delete dbInstance;
   dbInstance=0;
   mutex.unlock();
   
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

// removeFromRecipe ===========================================================
void Database::removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName )
{
   /*
   tableModel->setTable(relTableName);
   QString filter = tableModel->filter();
   
   // Find the row in the relational db that connects the ingredient to the recipe.
   tableModel->setFilter( QString("%1=%2 AND recipe_id=%3").arg(ingKeyName).arg(ing->_key).arg(rec->_key) );
   tableModel->select();
   if( tableModel->rowCount() > 0 )
      tableModel->removeRows(0,1);
   
   // Restore the old filter.
   tableModel->setFilter(filter);
   tableModel->select();
   */
   
   QSqlQuery q(sqlDatabase());
   q.setForwardOnly(true);
   q.prepare( QString("DELETE FROM `%1` WHERE `%2`='%3' AND recipe_id='%4'").arg(relTableName).arg(ingKeyName).arg(ing->_key).arg(rec->_key) );
   q.exec();
   q.finish();
   
   emit rec->changed( rec->metaProperty(propName), QVariant() );
}

void Database::removeFromRecipe( Recipe* rec, BrewNote* b )
{
   // Just mark the brew note as deleted.
   sqlUpdate( Brewtarget::BREWNOTETABLE,
              "deleted=1",
              QString("id=%1").arg(b->_key) );
}

void Database::removeFromRecipe( Recipe* rec, Hop* hop )
{
   removeIngredientFromRecipe( rec, hop, "hops", "hop_in_recipe", "hop_id" );
   disconnect( hop, 0, rec, 0 );
   rec->recalcAll();
}

void Database::removeFromRecipe( Recipe* rec, Fermentable* ferm )
{
   removeIngredientFromRecipe( rec, ferm, "fermentables", "fermentable_in_recipe", "fermentable_id" );
   disconnect( ferm, 0, rec, 0 );
   rec->recalcAll();
}

void Database::removeFromRecipe( Recipe* rec, Misc* m )
{
   removeIngredientFromRecipe( rec, m, "miscs", "misc_in_recipe", "misc_id" );
   rec->recalcAll();
}

void Database::removeFromRecipe( Recipe* rec, Yeast* y )
{
   removeIngredientFromRecipe( rec, y, "yeasts", "yeast_in_recipe", "yeast_id" );
   rec->recalcAll();
}

void Database::removeFromRecipe( Recipe* rec, Water* w )
{
   removeIngredientFromRecipe( rec, w, "waters", "water_in_recipe", "water_id" );
   rec->recalcAll();
}

void Database::removeFromRecipe( Recipe* rec, Instruction* ins )
{
   // NOTE: is this the right thing to do?
   // --maf-- No it isn't. Instructions just need to get whacked.
//   sqlUpdate( Brewtarget::INSTRUCTIONTABLE,
//              "deleted=1",
//              QString("%1=%2").arg(keyNames[Brewtarget::INSTRUCTIONTABLE]).arg(ins->_key) );

   sqlDelete( Brewtarget::INSTRUCTIONTABLE,
              QString("id=%1").arg(ins->_key) );

}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::removeFrom( Mash* mash, MashStep* step )
{
   // Just mark the step as deleted.
   sqlUpdate( Brewtarget::MASHSTEPTABLE,
              "deleted=1",
              QString("id=%1").arg(step->_key) );
   emit mash->changed( mash->metaProperty("mashSteps"), QVariant() );
   emit mash->mashStepsChanged();
}

Recipe* Database::getParentRecipe( BrewNote const* note )
{
   int key;
   QSqlQuery q( QString("SELECT recipe_id FROM brewnote WHERE id = %1").arg(note->_key),
                sqlDatabase());//sqldb );
   q.next();
   key = q.record().value("recipe_id").toInt();
   q.finish();
   
   return allRecipes[key];
}

Recipe* Database::recipe(int key)
{
   Recipe* ret;
   if( allRecipes.contains(key) )
   {
      ret = allRecipes[key];
      //if( ret->_uninitializedCalcs )
      //   ret->recalcAll();
      return ret;
   }
   else
      return 0;
}

Equipment* Database::equipment(int key)
{
   if( allEquipments.contains(key) )
      return allEquipments[key];
   else
      return 0;
}

Mash* Database::mash(int key)
{
   if( allMashs.contains(key) )
      return allMashs[key];
   else
      return 0;
}

Style* Database::style(int key)
{
   if( allStyles.contains(key) )
      return allStyles[key];
   else
      return 0;
}

void Database::swapMashStepOrder(MashStep* m1, MashStep* m2)
{
   // TODO: encapsulate in QUndoCommand.
   QSqlQuery q( QString("UPDATE mashstep SET step_number = CASE msid WHEN %1 then %2 when %3 then %4 END WHERE msid IN (%5,%6)")
                .arg(m1->_key).arg(m2->_key).arg(m2->_key).arg(m1->_key).arg(m1->_key).arg(m2->_key),
                sqlDatabase());//sqldb );
   q.finish();
   
   emit m1->changed( m1->metaProperty("stepNumber") );
   emit m2->changed( m2->metaProperty("stepNumber") );
}

void Database::swapInstructionOrder(Instruction* in1, Instruction* in2)
{
   // TODO: encapsulate in QUndoCommand.
   QSqlQuery q( QString("UPDATE instruction SET instruction_number = CASE id WHEN %1 then %2 when %3 then %4 END WHERE id IN (%5,%6)")
                .arg(in1->_key).arg(in2->_key).arg(in2->_key).arg(in1->_key).arg(in1->_key).arg(in2->_key),
                sqlDatabase());//sqldb );
   q.finish();
   
   emit in1->changed( in1->metaProperty("instructionNumber") );
   emit in2->changed( in2->metaProperty("instructionNumber") );
}

void Database::insertInstruction(Instruction* in, int pos)
{
   int parentRecipeKey;
   QSqlQuery q( QString("SELECT recipe_id FROM %1 WHERE id=%2")
                   .arg(tableNames[Brewtarget::INSTRUCTIONTABLE])
                   .arg(in->_key),
                sqlDatabase());//sqldb);
   q.next();
   parentRecipeKey = q.record().value("recipe_id").toInt();
   q.finish();
   
   // Increment all instruction positions greater or equal to pos.
   sqlUpdate( Brewtarget::INSTRUCTIONTABLE,
              QString("instruction_number=instruction_number+1"),
              QString("recipe_id=%1 AND instruction_number>=%2")
                 .arg(parentRecipeKey)
                 .arg(pos) );
              
   // Change in's position to pos.
   sqlUpdate( Brewtarget::INSTRUCTIONTABLE,
              QString("instruction_number=%1").arg(pos),
              QString("id=%1").arg(in->_key) );

   // emit changed
   emit in->changed( in->metaProperty("instructionNumber"), pos );
}

QList<BrewNote*> Database::brewNotes(Recipe const* parent)
{
   QList<BrewNote*> ret;
   QString filterString = QString("recipe_id = %1 AND deleted = 0 and display = 1").arg(parent->_key);
   
   getBrewNotes(ret, filterString);

   return ret;
}

QList<Fermentable*> Database::fermentables(Recipe const* parent)
{
   QList<Fermentable*> ret;
   QString queryString = QString("SELECT fermentable_id FROM fermentable_in_recipe WHERE recipe_id = %1").arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allFermentables[q.record().value("fermentable_id").toInt()]);
   q.finish();
   
   return ret;
}

QList<Hop*> Database::hops(Recipe const* parent)
{
   QList<Hop*> ret;
   QString queryString = QString("SELECT hop_id FROM hop_in_recipe WHERE recipe_id = %1").arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allHops[q.record().value("hop_id").toInt()]);
   q.finish();
   
   return ret;
}

QList<Misc*> Database::miscs(Recipe const* parent)
{
   QList<Misc*> ret;
   QString queryString = QString("SELECT misc_id FROM misc_in_recipe WHERE recipe_id = %1").arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allMiscs[q.record().value("misc_id").toInt()]);
   q.finish();
   
   return ret;
}

Mash* Database::mash( Recipe const* parent )
{
   int mashId = get( Brewtarget::RECTABLE, parent->key(), "mash_id" ).toInt();
   
   if( allMashs.contains(mashId) )
      return allMashs[mashId];
   else
      return 0;
}

QList<MashStep*> Database::mashSteps(Mash const* parent)
{
   QList<MashStep*> ret;
   QString filterString = QString("mash_id = %1 AND deleted = 0 AND display = 1").arg(parent->_key);
  
   getMashSteps(ret, filterString);
   
   return ret;
}

QList<Instruction*> Database::instructions( Recipe const* parent )
{
   QList<Instruction*> ret;
   QString queryString = QString("SELECT id FROM %1 WHERE recipe_id = %2 ORDER BY instruction_number ASC")
                            .arg(tableNames[Brewtarget::INSTRUCTIONTABLE])
                            .arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allInstructions[q.record().value("id").toInt()]);
   q.finish();
   
   return ret;
}

QList<Water*> Database::waters(Recipe const* parent)
{
   QList<Water*> ret;
   QString queryString = QString("SELECT water_id FROM water_in_recipe WHERE recipe_id = %1").arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allWaters[q.record().value("water_id").toInt()]);
   q.finish();
   
   return ret;
}

QList<Yeast*> Database::yeasts(Recipe const* parent)
{
   QList<Yeast*> ret;
   QString queryString = QString("SELECT yeast_id FROM yeast_in_recipe WHERE recipe_id = %1").arg(parent->_key);
   QSqlQuery q( queryString, sqlDatabase() );//, sqldb );
   
   while( q.next() )
      ret.append(allYeasts[q.record().value("yeast_id").toInt()]);
   q.finish();
   
   return ret;
}

// Named constructors =========================================================

int Database::insertNewDefaultRecord( Brewtarget::DBTable table )
{   
   int key;

   QSqlQuery q(sqlDatabase());
   q.exec( QString("INSERT INTO `%1` DEFAULT VALUES")
              .arg(tableNames[table])
         );

   if( q.numRowsAffected() < 1 )
   {
      Brewtarget::logE( QString("Database::insertNewDefaultRecord: could not insert a record into %1. %2").arg(tableNames[table]).arg(q.lastError().text()) );
      key = -42;
   }
   else
      key = q.lastInsertId().toInt();
   q.finish();
   
   //if( q.lastError().isValid() )
   //   Brewtarget::logE( QString("Database::insertNewDefaultRecord: %1").arg(q.lastError().text()) );
   
   return key;
}

int Database::insertNewMashStepRecord( Mash* parent )
{
   int key;
   
   QSqlQuery q(sqlDatabase());//sqldb );
   q.setForwardOnly(true);
   q.exec( QString("INSERT INTO `%1` DEFAULT VALUES")
              .arg(tableNames[Brewtarget::MASHSTEPTABLE])
         );
   if( q.numRowsAffected() < 1 )
   {
      Brewtarget::logE( QString("Database::insertNewDefaultRecord: could not insert a record into %1.").arg(tableNames[Brewtarget::MASHSTEPTABLE]) );
      key = -42;
   }
   else
      key = q.lastInsertId().toInt();
   q.finish();
   
   // I *think* we need to set the mash_id first
   sqlUpdate( Brewtarget::MASHSTEPTABLE,
              QString("`mash_id`='%1' ").arg(parent->_key),
              QString("id='%1'").arg(key)
            );
   // Just sets the step number within the mash to the next available number.
   sqlUpdate( Brewtarget::MASHSTEPTABLE,
              QString( "`step_number` = (SELECT IFNULL(MAX(`step_number`)+1,0) FROM `%1` WHERE deleted=0 AND mash_id='%2' )")
                      .arg(tableNames[Brewtarget::MASHSTEPTABLE])
                      .arg(parent->_key),
              QString("id='%1'").arg(key)
            );
   return key;
}

BrewNote* Database::newBrewNote(BrewNote* other, bool signal)
{
   int newKey;
   BrewNote* tmp = new BrewNote();
   
   QSqlRecord r = copy<BrewNote>(other);
   newKey = r.value("id").toInt();
   tmp->_key = newKey;
   tmp->_table = Brewtarget::BREWNOTETABLE;
   allBrewNotes.insert( newKey, tmp );
  
   if ( signal )
      emit changed( metaProperty("brewNotes"), QVariant() );
   return tmp;
}

BrewNote* Database::newBrewNote(Recipe* parent, bool signal)
{
   BrewNote* tmp = new BrewNote();

   tmp->_key = insertNewDefaultRecord(Brewtarget::BREWNOTETABLE);
   tmp->_table = Brewtarget::BREWNOTETABLE;

   allBrewNotes.insert(tmp->_key,tmp);
   sqlUpdate( Brewtarget::BREWNOTETABLE,
              QString("recipe_id=%1").arg(parent->_key),
              QString("id=%2").arg(tmp->_key) );

   if ( signal ) 
      sendEmitchanged( metaProperty("brewNotes"), QVariant() );
   return tmp;
}

Equipment* Database::newEquipment()
{
   Equipment* tmp = new Equipment();
   tmp->_key = insertNewDefaultRecord(Brewtarget::EQUIPTABLE);
   tmp->_table = Brewtarget::EQUIPTABLE;
   allEquipments.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("equipments"), QVariant() );
   emit newEquipmentSignal(tmp);
   
   return tmp;
}

Equipment* Database::newEquipment(Equipment* other)
{
   Equipment* tmp = new Equipment();
   tmp->_key = copy<Equipment>(other).value("id").toInt();
   tmp->_table = Brewtarget::EQUIPTABLE;
   allEquipments.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("equipments"), QVariant() );
   emit newEquipmentSignal(tmp);
   
   return tmp;
}

Fermentable* Database::newFermentable()
{
   Fermentable* tmp = new Fermentable();
   tmp->_key = insertNewDefaultRecord(Brewtarget::FERMTABLE);
   tmp->_table = Brewtarget::FERMTABLE;
   allFermentables.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("fermentables"), QVariant() );
   emit newFermentableSignal(tmp);
   
   return tmp;
}

Fermentable* Database::newFermentable(Fermentable* other)
{
   Fermentable* tmp = new Fermentable();
   tmp->_key = copy<Fermentable>(other).value("id").toInt();
   tmp->_table = Brewtarget::FERMTABLE;
   allFermentables.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("fermentables"), QVariant() );
   emit newFermentableSignal(tmp);
   
   return tmp;
}

Hop* Database::newHop()
{
   Hop* tmp = new Hop();
   tmp->_key = insertNewDefaultRecord(Brewtarget::HOPTABLE);
   tmp->_table = Brewtarget::HOPTABLE;
   allHops.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("hops"), QVariant() );
   emit newHopSignal(tmp);
   
   return tmp;
}

Hop* Database::newHop(Hop* other)
{
   Hop* tmp = new Hop();
   tmp->_key = copy<Hop>(other).value("id").toInt();
   tmp->_table = Brewtarget::HOPTABLE;
   allHops.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("hops"), QVariant() );
   emit newHopSignal(tmp);
   
   return tmp;
}

Instruction* Database::newInstruction(Recipe* rec)
{
   // TODO: encapsulate in QUndoCommand.
   Instruction* tmp = new Instruction();
   tmp->_key = insertNewDefaultRecord(Brewtarget::INSTRUCTIONTABLE);
   tmp->_table = Brewtarget::INSTRUCTIONTABLE;

   sqlUpdate( Brewtarget::INSTRUCTIONTABLE,
              QString("`recipe_id`='%1'").arg(rec->_key),
              QString("`id`='%1'").arg(tmp->_key) );
   allInstructions.insert(tmp->_key,tmp);
   
   // Database's instructions have changed.
   sendEmitchanged( metaProperty("instructions"), QVariant() );
   // Do not emit the recipe's changed() signal. It is up to the recipe to
   // decide when it wants its signals emitted... that sounds so kinky.
   return tmp;
}

Mash* Database::newMash()
{
   Mash* tmp = new Mash();
   tmp->_key = insertNewDefaultRecord(Brewtarget::MASHTABLE);
   tmp->_table = Brewtarget::MASHTABLE;
   allMashs.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);
   
   return tmp;
}

Mash* Database::newMash(Recipe* parent)
{
   Mash* tmp = new Mash();
   tmp->_key = insertNewDefaultRecord(Brewtarget::MASHTABLE);
   tmp->_table = Brewtarget::MASHTABLE;
   allMashs.insert(tmp->_key,tmp);
   
   // Connect tmp to parent, removing any existing mash in parent.
   sqlUpdate( Brewtarget::RECTABLE,
              QString("mash_id=%1").arg(tmp->_key),
              QString("id=%1").arg(parent->_key) );
   
   sendEmitchanged( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);
   
   return tmp;
}

Mash* Database::newMash(Mash* other, bool displace)
{
   Mash* tmp = new Mash();
   tmp->_key = copy<Mash>(other).value("id").toInt();
   tmp->_table = Brewtarget::MASHTABLE;
   allMashs.insert(tmp->_key,tmp);
   
   // Connect tmp to parent, removing any existing mash in parent.
   if( displace )
   {
      sqlUpdate( Brewtarget::RECTABLE,
                 QString("mash_id=%1").arg(tmp->_key),
                 QString("mash_id=%1").arg(other->_key) );
   }
   
   sendEmitchanged( metaProperty("mashs"), QVariant() );
   emit newMashSignal(tmp);
   
   return tmp;
}

MashStep* Database::newMashStep(Mash* mash)
{
   // TODO: encapsulate in QUndoCommand.
   // NOTE: we have unique(mash_id,step_number) constraints on this table,
   // so may have to pay special attention when creating the new record.
   MashStep* tmp = new MashStep();
   tmp->_key = insertNewMashStepRecord(mash);
   tmp->_table = Brewtarget::MASHSTEPTABLE;

   allMashSteps.insert(tmp->_key,tmp);
   connect( tmp, SIGNAL(changed(QMetaProperty,QVariant)), mash, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );
   // Database's steps have changed.
   sendEmitchanged( metaProperty("mashSteps"), QVariant() );
   // Mash's steps have changed.
   emit mash->changed( mash->metaProperty("mashSteps"), QVariant() );
   emit mash->mashStepsChanged();
   return tmp;
}

Misc* Database::newMisc()
{
   Misc* tmp = new Misc();
   tmp->_key = insertNewDefaultRecord(Brewtarget::MISCTABLE);
   tmp->_table = Brewtarget::MISCTABLE;
   allMiscs.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("miscs"), QVariant() );
   emit newMiscSignal(tmp);
   
   return tmp;
}

Misc* Database::newMisc(Misc* other)
{
   Misc* tmp = new Misc();
   tmp->_key = copy<Misc>(other).value("id").toInt();
   tmp->_table = Brewtarget::MISCTABLE;
   allMiscs.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("miscs"), QVariant() );
   emit newMiscSignal(tmp);
   
   return tmp;
}

Recipe* Database::newRecipe()
{
   Recipe* tmp = new Recipe();
   tmp->_key = insertNewDefaultRecord(Brewtarget::RECTABLE);
   tmp->_table = Brewtarget::RECTABLE;
   allRecipes.insert(tmp->_key,tmp);
   
   // Now, need to create a new mash for the recipe.
   newMash( tmp );
   
   sendEmitchanged( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(tmp);
   
   return tmp;
}

Recipe* Database::newRecipe(Recipe* other)
{
   Recipe* tmp = new Recipe();
   tmp->_key = copy<Recipe>(other).value("id").toInt();
   tmp->_table = Brewtarget::RECTABLE;
   allRecipes.insert( tmp->_key, tmp );
   
   sendEmitchanged( metaProperty("recipes"), QVariant() );
   emit newRecipeSignal(tmp);
   
   return tmp;
}

Style* Database::newStyle()
{
   Style* tmp = new Style();
   tmp->_key = insertNewDefaultRecord(Brewtarget::STYLETABLE);
   tmp->_table = Brewtarget::STYLETABLE;
   allStyles.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("styles"), QVariant() );
   emit newStyleSignal(tmp);
   
   return tmp;
}

Water* Database::newWater()
{
   Water* tmp = new Water();
   tmp->_key = insertNewDefaultRecord(Brewtarget::WATERTABLE);
   tmp->_table = Brewtarget::WATERTABLE;
   allWaters.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("waters"), QVariant() );
   emit newWaterSignal(tmp);
   
   return tmp;
}

Yeast* Database::newYeast()
{
   Yeast* tmp = new Yeast();
   tmp->_key = insertNewDefaultRecord(Brewtarget::YEASTTABLE);
   tmp->_table = Brewtarget::YEASTTABLE;
   allYeasts.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("yeasts"), QVariant() );
   emit newYeastSignal(tmp);
   
   return tmp;
}

Yeast* Database::newYeast(Yeast* other)
{
   Yeast* tmp = new Yeast();
   tmp->_key = copy<Yeast>(other).value("id").toInt();
   tmp->_table = Brewtarget::YEASTTABLE;
   allYeasts.insert(tmp->_key,tmp);
   
   sendEmitchanged( metaProperty("yeasts"), QVariant() );
   emit newYeastSignal(tmp);
   
   return tmp;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Database::deleteRecord( Brewtarget::DBTable table, BeerXMLElement* object )
{
   // Assumes the table has a column called 'deleted'.
   SetterCommand* command;
   command = new SetterCommand(table,
                         object->_key,
                         "deleted",
                         QVariant(1),
                         object->metaProperty("deleted"),
                         object,
                         true);
   // For now, immediately execute the command.
   command->redo();
   
   // Push the command on the undo stack.
   //commandStack.push(command);
}

void Database::removeEquipment(Equipment* equip)
{
   deleteRecord(Brewtarget::EQUIPTABLE,equip);
   
   emit changed( metaProperty("equipments"), QVariant() );
   emit deletedEquipmentSignal(equip);
}

void Database::removeEquipment(QList<Equipment*> equip)
{
   if ( equip.empty() )
      return;

   QList<Equipment*>::Iterator it = equip.begin();
   while( it != equip.end() )
   {
      deleteRecord(Brewtarget::EQUIPTABLE,*it);
      emit deletedEquipmentSignal(*it);
      it++;
   }

   emit changed( metaProperty("equipments"), QVariant() );
}

void Database::removeFermentable(Fermentable* ferm)
{
   deleteRecord(Brewtarget::FERMTABLE,ferm);
   
   emit changed( metaProperty("fermentables"), QVariant());
   emit deletedFermentableSignal(ferm);
}

void Database::removeFermentable(QList<Fermentable*> ferm)
{
   if ( ferm.empty() )
      return;

   QList<Fermentable*>::Iterator it = ferm.begin();
   while( it != ferm.end() )
   {
      deleteRecord(Brewtarget::FERMTABLE,*it);
      emit deletedFermentableSignal(*it);
      
      it++;
   }
   
   emit changed( metaProperty("fermentables"), QVariant());
}

void Database::removeHop(Hop* hop)
{
   deleteRecord(Brewtarget::HOPTABLE,hop);
   
   emit changed( metaProperty("hops"), QVariant() );
   emit deletedHopSignal(hop);
}

void Database::removeHop(QList<Hop*> hop)
{
   if ( hop.empty() )
      return;

   QList<Hop*>::Iterator it = hop.begin();
   while( it != hop.end() )
   {
      deleteRecord(Brewtarget::HOPTABLE,*it);
      emit deletedHopSignal(*it);
      
      it++;
   }

   emit changed( metaProperty("hops"), QVariant() );
}

void Database::removeMash(Mash* mash)
{
   deleteRecord(Brewtarget::MASHTABLE,mash);
   
   emit changed( metaProperty("mashs"), QVariant() );
   emit deletedMashSignal(mash);
}

void Database::removeMash(QList<Mash*> mash)
{
   if ( mash.empty() )
      return;

   QList<Mash*>::Iterator it = mash.begin();
   while( it != mash.end() )
   {
      deleteRecord(Brewtarget::MASHTABLE,*it);
      emit deletedMashSignal(*it);
      it++;
   }
   emit changed( metaProperty("mashs"), QVariant() );
}

void Database::removeMashStep(MashStep* mashStep)
{
   deleteRecord(Brewtarget::MASHSTEPTABLE,mashStep);
   
   emit changed( metaProperty("mashSteps"), QVariant() );
}

void Database::removeMashStep(QList<MashStep*> mashStep)
{
   if ( mashStep.empty() )
      return;

   QList<MashStep*>::Iterator it = mashStep.begin();
   while( it != mashStep.end() )
   {
      deleteRecord(Brewtarget::MASHSTEPTABLE,*it);
      it++;
   }
   emit changed( metaProperty("mashSteps"), QVariant() );
}

void Database::removeMisc(Misc* misc)
{
   deleteRecord(Brewtarget::MISCTABLE,misc);
   
   emit changed( metaProperty("miscs"), QVariant());
   emit deletedMiscSignal(misc);
}

void Database::removeMisc(QList<Misc*> misc)
{
   if ( misc.empty() )
      return;

   QList<Misc*>::Iterator it = misc.begin();
   while( it != misc.end() )
   {
      deleteRecord(Brewtarget::MISCTABLE,*it);
      emit deletedMiscSignal(*it);
      it++;
   }
   emit changed( metaProperty("miscs"), QVariant());
}

void Database::removeRecipe(Recipe* rec)
{
   deleteRecord(Brewtarget::RECTABLE,rec);
   
   emit changed( metaProperty("recipes"), QVariant() );
   emit deletedRecipeSignal(rec);
}

void Database::removeRecipe(QList<Recipe*> rec)
{
   if ( rec.empty() )
      return;

   QList<Recipe*>::Iterator it = rec.begin();
   while( it != rec.end() )
   {
      deleteRecord(Brewtarget::RECTABLE,*it);
      emit deletedRecipeSignal(*it);
      it++;
   }
   emit changed( metaProperty("recipes"), QVariant() );
}

void Database::removeStyle(Style* style)
{
   deleteRecord(Brewtarget::STYLETABLE,style);
   
   emit changed( metaProperty("styles"), QVariant() );
   emit deletedStyleSignal(style);
}

void Database::removeStyle(QList<Style*> style)
{
   if ( style.empty() )
      return;

   QList<Style*>::Iterator it = style.begin();
   while( it != style.end() )
   {
      deleteRecord(Brewtarget::STYLETABLE,*it);
      emit deletedStyleSignal(*it);
      it++;
   }
   emit changed( metaProperty("styles"), QVariant() );
}

void Database::removeWater(Water* water)
{
   deleteRecord(Brewtarget::WATERTABLE,water);
   
   emit changed( metaProperty("waters"), QVariant());
   emit deletedWaterSignal(water);
}

void Database::removeWater(QList<Water*> water)
{
   if ( water.empty() )
      return;

   QList<Water*>::Iterator it = water.begin();
   while( it != water.end() )
   {
      deleteRecord(Brewtarget::WATERTABLE,*it);
      emit deletedWaterSignal(*it);
      it++;
   }
   emit changed( metaProperty("waters"), QVariant());
}

void Database::removeYeast(Yeast* yeast)
{
   deleteRecord(Brewtarget::YEASTTABLE,yeast);
   
   emit changed( metaProperty("yeasts"), QVariant());
   emit deletedYeastSignal(yeast);
}

void Database::removeYeast(QList<Yeast*> yeast)
{
   if ( yeast.empty() )
      return;

   QList<Yeast*>::Iterator it = yeast.begin();
   while( it != yeast.end() )
   {
      deleteRecord(Brewtarget::YEASTTABLE,*it);
      emit deletedYeastSignal(*it);
      it++;
   }
   emit changed( metaProperty("yeasts"), QVariant());
}

QString Database::getDbFileName()
{
   // Ensure instance exists.
   instance();
   
   return dbFileName;
}

void Database::updateEntry( Brewtarget::DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify )
{
   SetterCommand* command;
   command = new SetterCommand(table,
                               key,
                               col_name,
                               value,
                               prop,
                               object,
                               notify);

   // Push onto custom stack.
   //_setterCommandStack->push(command);
   
   // Just execute the damn thing.
   command->redo();
}

void Database::sendEmitchanged(QMetaProperty prop, QVariant value)
{
  if ( !skipEmitChanged )
  {
    emit changed( prop, value );
  }
}

void Database::sendEmitchanged(Recipe* rec, QMetaProperty prop, QVariant value)
{
  if ( !skipEmitChanged )
  {
    emit rec->changed( prop, value );
  }
}


// Add to recipe ==============================================================
void Database::addToRecipe( Recipe* rec, Hop* hop, bool noCopy )
{
   int key = addIngredientToRecipe<Hop>( rec, hop,
                                         "hops",
                                         "hop_in_recipe",
                                         "hop_id",
                                         noCopy, &allHops );
   connect( allHops[key], SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptHopChange(QMetaProperty,QVariant)));
   rec->recalcIBU();
}

void Database::addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy )
{
   if ( ferm == 0 )
      return;

   int key = addIngredientToRecipe<Fermentable>( rec, ferm,
                                                 "fermentables",
                                                 "fermentable_in_recipe",
                                                 "fermentable_id",
                                                 noCopy, &allFermentables );
   connect( allFermentables[key], SIGNAL(changed(QMetaProperty,QVariant)), rec, SLOT(acceptFermChange(QMetaProperty,QVariant)) );
   rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Misc* m, bool noCopy )
{
   addIngredientToRecipe<Misc>( rec, m, "miscs", "misc_in_recipe", "misc_id", noCopy, &allMiscs );
   rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Yeast* y, bool noCopy )
{
   addIngredientToRecipe<Yeast>( rec, y, "yeasts", "yeast_in_recipe", "yeast_id", noCopy, &allYeasts );
   rec->recalcOgFg();
}

void Database::addToRecipe( Recipe* rec, Water* w, bool noCopy )
{
   addIngredientToRecipe<Water>( rec, w, "waters", "water_in_recipe", "water_id", noCopy, &allWaters );
   rec->recalcAll();
}

void Database::addToRecipe( Recipe* rec, Mash* m, bool noCopy )
{
   QSqlRecord c;
   int newKey;

   // Make a copy of mash.
   if ( ! noCopy ) 
   {
      c = copy<Mash>(m, false, &allMashs);
      newKey = c.value("id").toInt();
   }
   else 
   {
      newKey = m->_key;
   }
   
   // Update mash_id
   sqlUpdate(Brewtarget::RECTABLE,
             QString("`mash_id`='%1'").arg(newKey),
             QString("id='%1'").arg(rec->_key));
   
   // Emit a changed signal.
   sendEmitchanged( rec, rec->metaProperty("mash"), QVariant() );
}

void Database::addToRecipe( Recipe* rec, Equipment* e, bool noCopy )
{
   QSqlRecord c;
   int newKey;
   Equipment* newEquip;

   if( e == 0 )
      return;
  
   // Make a copy of equipment.
   if ( ! noCopy )
   {
      c = copy<Equipment>(e,false,&allEquipments);
      newKey = c.value("id").toInt();
      
      newEquip = allEquipments[newKey];
   }
   else 
   {
      newKey = e->_key;
      newEquip = e;
   }

   
   // Update equipment_id
   sqlUpdate(Brewtarget::RECTABLE,
             QString("`equipment_id`='%1'").arg(newKey),
             QString("id='%1'").arg(rec->_key));

   newEquip->setDisplay(false);
   // Emit a changed signal.
   sendEmitchanged( rec, rec->metaProperty("equipment"), BeerXMLElement::qVariantFromPtr(newEquip) );
}

void Database::addToRecipe( Recipe* rec, Style* s)
{
   // Just add the style directly. No need to copy I think.
   sqlUpdate(Brewtarget::RECTABLE,
             QString("`style_id`='%1'").arg(s->_key),
             QString("id='%1'").arg(rec->_key));

   // We do not need to set s->display() to false here, because we are using
   // the styles directly out of the style table. If we did that, then we could
   // only use a style once before it disappeared.
   
   // Emit a changed signal.
   // TODO: this is calling the SLOT, not the SIGNAL...erm...What do we do?
   sendEmitchanged( rec, rec->metaProperty("style"), BeerXMLElement::qVariantFromPtr(s) );
}
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Database::sqlUpdate( Brewtarget::DBTable table, QString const& setClause, QString const& whereClause )
{
   QSqlQuery q( QString("UPDATE `%1` SET %2 WHERE %3")
                .arg(tableNames[table])
                .arg(setClause)
                .arg(whereClause),
                sqlDatabase());
   if( q.lastError().isValid() )
      Brewtarget::logE( QString("Database::sqlUpdate(): %1").arg(q.lastError().text()) );
   q.finish();
}

void Database::sqlDelete( Brewtarget::DBTable table, QString const& whereClause )
{
   QSqlQuery q( QString("DELETE FROM `%1` WHERE %2")
                .arg(tableNames[table])
                .arg(whereClause),
                sqlDatabase());
   q.finish();
}

void Database::getBrewNotes( QList<BrewNote*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::BREWNOTETABLE, allBrewNotes );
}

void Database::getEquipments( QList<Equipment*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::EQUIPTABLE, allEquipments );
}

void Database::getFermentables( QList<Fermentable*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::FERMTABLE, allFermentables );
}

void Database::getHops( QList<Hop*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::HOPTABLE, allHops );
}

void Database::getMashs( QList<Mash*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::MASHTABLE, allMashs );
}

void Database::getMashSteps( QList<MashStep*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::MASHSTEPTABLE, allMashSteps );
}

void Database::getMiscs( QList<Misc*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::MISCTABLE, allMiscs );
}

void Database::getRecipes( QList<Recipe*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::RECTABLE, allRecipes );
}

void Database::getStyles( QList<Style*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::STYLETABLE, allStyles );
}

void Database::getWaters( QList<Water*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::WATERTABLE, allWaters );
}

void Database::getYeasts( QList<Yeast*>& list, QString filter )
{
   getElements( list, filter, Brewtarget::YEASTTABLE, allYeasts );
}

QHash<Brewtarget::DBTable,QSqlQuery> Database::selectAllHash()
{
   QHash<Brewtarget::DBTable,QSqlQuery> ret;
   QHash<Brewtarget::DBTable,QString> names = Database::tableNamesHash();
   
   foreach( Brewtarget::DBTable table, names.keys() )
   {
      QSqlQuery q(sqlDatabase());
      q.prepare( QString("SELECT * FROM `%1` WHERE `id`=:id").arg(names[table]) );
      
      ret[table] = q;
   }
   
   return ret;
}

QHash<Brewtarget::DBTable,QString> Database::tableNamesHash()
{
   QHash<Brewtarget::DBTable,QString> tmp;
   
   tmp[ Brewtarget::BREWNOTETABLE ] = "brewnote";
   tmp[ Brewtarget::EQUIPTABLE ] = "equipment";
   tmp[ Brewtarget::FERMTABLE ] = "fermentable";
   tmp[ Brewtarget::HOPTABLE ] = "hop";
   tmp[ Brewtarget::INSTRUCTIONTABLE ] = "instruction";
   tmp[ Brewtarget::MASHSTEPTABLE ] = "mashstep";
   tmp[ Brewtarget::MASHTABLE ] = "mash";
   tmp[ Brewtarget::MISCTABLE ] = "misc";
   tmp[ Brewtarget::RECTABLE ] = "recipe";
   tmp[ Brewtarget::STYLETABLE ] = "style";
   tmp[ Brewtarget::WATERTABLE ] = "water";
   tmp[ Brewtarget::YEASTTABLE ] = "yeast";
   
   return tmp;
}

QHash<QString,Brewtarget::DBTable> Database::classNameToTableHash()
{
   QHash<QString,Brewtarget::DBTable> tmp;
   
   tmp["BrewNote"] = Brewtarget::BREWNOTETABLE;
   tmp["Equipment"] = Brewtarget::EQUIPTABLE;
   tmp["Fermentable"] = Brewtarget::FERMTABLE;
   tmp["Hop"] = Brewtarget::HOPTABLE;
   tmp["Instruction"] = Brewtarget::INSTRUCTIONTABLE;
   tmp["MashStep"] = Brewtarget::MASHSTEPTABLE;
   tmp["Mash"] = Brewtarget::MASHTABLE;
   tmp["Misc"] = Brewtarget::MISCTABLE;
   tmp["Recipe"] = Brewtarget::RECTABLE;
   tmp["Style"] = Brewtarget::STYLETABLE;
   tmp["Water"] = Brewtarget::WATERTABLE;
   tmp["Yeast"] = Brewtarget::YEASTTABLE;
   
   return tmp;
}

QList<BrewNote*> Database::brewNotes()
{
   QList<BrewNote*> tmp;

   getBrewNotes( tmp, "deleted=0 AND display=1" );
   return tmp;
}

QList<Equipment*> Database::equipments()
{
   QList<Equipment*> tmp;
   getEquipments( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Fermentable*> Database::fermentables()
{
   QList<Fermentable*> tmp;
   getFermentables( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Hop*> Database::hops()
{
   QList<Hop*> tmp;
   getHops( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Mash*> Database::mashs()
{
   QList<Mash*> tmp;
   getMashs( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<MashStep*> Database::mashSteps()
{
   QList<MashStep*> tmp;
   getMashSteps( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Misc*> Database::miscs()
{
   QList<Misc*> tmp;
   getMiscs( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Recipe*> Database::recipes()
{
   QList<Recipe*> tmp;
   getRecipes( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Style*> Database::styles()
{
   QList<Style*> tmp;
   getStyles( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Water*> Database::waters()
{
   QList<Water*> tmp;
   getWaters( tmp, "deleted=0 and display=1");
   return tmp;
}

QList<Yeast*> Database::yeasts()
{
   QList<Yeast*> tmp;
   getYeasts( tmp, "deleted=0 and display=1");
   return tmp;
}

void Database::importFromXML(const QString& filename)
{
   unsigned int count;
   int line, col;
   QDomDocument xmlDoc;
   QDomElement root;
   QDomNodeList list;
   QString err;
   QFile inFile;
   QStringList tags = QStringList() << "EQUIPMENT" << "FERMENTABLE" << "HOP" << "MISC" << "STYLE" << "YEAST" << "WATER";
   inFile.setFileName(filename);
   
   if( ! inFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::logW(QString("Database::importFromXML: Could not open %1 for reading.").arg(filename));
      return;
   }

   if( ! xmlDoc.setContent(&inFile, false, &err, &line, &col) )
      Brewtarget::logW(QString("Database::importFromXML: Bad document formatting in %1 %2:%3. %4").arg(filename).arg(line).arg(col).arg(err) );

   list = xmlDoc.elementsByTagName("RECIPE");
   if ( list.count() )
   {
      skipEmitChanged = true;
      for(int i = 0; i < list.count(); ++i )
      {
         if ( i  == list.count() -1 )
            skipEmitChanged = false;
         recipeFromXml( list.at(i) );
      }
      skipEmitChanged = false;
   }
   else
   {
      foreach (QString tag, tags)
      {
         list = xmlDoc.elementsByTagName(tag);
         count = list.size();

         if ( count > 0 ) 
         {
            // Tell how many there were in the status bar.
            //statusBar()->showMessage( tr("Found %1 %2.").arg(count).arg(tag.toLower()), 5000 );

            if (tag == "RECIPE")
            {
            }
            else if ( tag == "EQUIPMENT" )
            {
               for(int i = 0; i < list.count(); ++i )
                  equipmentFromXml( list.at(i) );
            }
            else if( tag == "FERMENTABLE" )
            {
               for( int i = 0; i < list.count(); ++i )
                  fermentableFromXml( list.at(i) );
            }
            else if (tag == "HOP")
            {
               for(int i = 0; i < list.count(); ++i )
                  hopFromXml( list.at(i) );
            }
            else if (tag == "MISC")
            {
               for(int i = 0; i < list.count(); ++i )
                  miscFromXml( list.at(i) );
            }
            else if( tag == "STYLE" )
            {
               for( int i = 0; i < list.count(); ++i )
                  styleFromXml( list.at(i) );
            }
            else if (tag == "YEAST")
            {
               for(int i = 0; i < list.count(); ++i )
                  yeastFromXml( list.at(i) );
            }
            else if( tag == "WATER" )
            {
               for( int i = 0; i < list.count(); ++i )
                  waterFromXml( list.at(i) );
            }
         }
      }
   }
}

void Database::toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent )
{
   // TODO: implement
}

void Database::toXml( Equipment* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement equipNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   equipNode = doc.createElement("EQUIPMENT");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilSize_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->batchSize_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunVolume_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunWeight_kg()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_SPECIFIC_HEAT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunSpecificHeat_calGC()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TOP_UP_WATER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->topUpWater_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TRUB_CHILLER_LOSS");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->trubChillerLoss_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("EVAP_RATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->evapRate_pctHr()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("REAL_EVAP_RATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->evapRate_lHr()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilTime_min()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CALC_BOIL_VOLUME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calcBoilVolume()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("LAUTER_DEADSPACE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->lauterDeadspace_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TOP_UP_KETTLE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->topUpKettle_l()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HOP_UTILIZATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hopUtilization_pct()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);

   // My extensions below
   tmpNode = doc.createElement("ABSORPTION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->grainAbsorption_LKg()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOILING_POINT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilingPoint_c()));
   tmpNode.appendChild(tmpText);
   equipNode.appendChild(tmpNode);
   parent.appendChild(equipNode);
}

void Database::toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement fermNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   fermNode = doc.createElement("FERMENTABLE");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(Fermentable::types.at(a->type()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_kg()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("YIELD");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->yield_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COLOR");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->color_srm()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ADD_AFTER_BOIL");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->addAfterBoil()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(a->origin());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SUPPLIER");
   tmpText = doc.createTextNode(a->supplier());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COARSE_FINE_DIFF");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->coarseFineDiff_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MOISTURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->moisture_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DIASTATIC_POWER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->diastaticPower_lintner()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PROTEIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->protein_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MAX_IN_BATCH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxInBatch_pct()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("RECOMMEND_MASH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->recommendMash()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("IS_MASHED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->isMashed()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("IBU_GAL_PER_LB");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuGalPerLb()));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   parent.appendChild(fermNode);
}

void Database::toXml( Hop* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement hopNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   hopNode = doc.createElement("HOP");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ALPHA");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->alpha_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_kg()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(a->useString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->time_min()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FORM");
   tmpText = doc.createTextNode(a->formString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BETA");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->beta_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HSI");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hsi_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(a->origin());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SUBSTITUTES");
   tmpText = doc.createTextNode(a->substitutes());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HUMULENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->humulene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARYOPHYLLENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->caryophyllene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COHUMULONE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->cohumulone_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MYRCENE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->myrcene_pct()));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   parent.appendChild(hopNode);
}

void Database::toXml( Instruction* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement insNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   insNode = doc.createElement("INSTRUCTION");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DIRECTIONS");
   tmpText = doc.createTextNode(a->directions());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HAS_TIMER");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->hasTimer()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIMER_VALUE");
   tmpText = doc.createTextNode(a->timerValue());
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COMPLETED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->completed()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INTERVAL");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->interval()));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   parent.appendChild(insNode);
}

void Database::toXml( Mash* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement mashNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   int i, size;
   
   mashNode = doc.createElement("MASH");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("GRAIN_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->grainTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MASH_STEPS");
   QList<MashStep*> mashSteps = a->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
      toXml( mashSteps[i], doc, tmpNode);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SPARGE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->spargeTemp_c()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ph()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunWeight_kg()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TUN_SPECIFIC_HEAT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tunSpecificHeat_calGC()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("EQUIP_ADJUST");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->equipAdjust()));
   tmpNode.appendChild(tmpText);
   mashNode.appendChild(tmpNode);
   
   parent.appendChild(mashNode);
}

void Database::toXml( MashStep* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement mashStepNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   mashStepNode = doc.createElement("MASH_STEP");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("INFUSE_AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->infuseAmount_l()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("STEP_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->stepTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("STEP_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->stepTime_min()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("RAMP_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->rampTime_min()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("END_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->endTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("INFUSE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->infuseTemp_c()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DECOCTION_AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->decoctionAmount_l()));
   tmpNode.appendChild(tmpText);
   mashStepNode.appendChild(tmpNode);
   
   parent.appendChild(mashStepNode);
}

void Database::toXml( Misc* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement miscNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   miscNode = doc.createElement("MISC");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(a->useString());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->time()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amountIsWeight()));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE_FOR");
   tmpText = doc.createTextNode(a->useFor());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   parent.appendChild(miscNode);
}

void Database::toXml( Recipe* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement recipeNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   int i;
   
   recipeNode = doc.createElement("RECIPE");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->type());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   Style* style = a->style();
   if( style != 0 )
      toXml( style, doc, recipeNode);
   
   tmpNode = doc.createElement("BREWER");
   tmpText = doc.createTextNode(a->brewer());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BATCH_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->batchSize_l()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_SIZE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilSize_l()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BOIL_TIME");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->boilTime_min()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("EFFICIENCY");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->efficiency_pct()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HOPS");
   QList<Hop*> hops = a->hops();
   for( i = 0; i < hops.size(); ++i )
      toXml( hops[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FERMENTABLES");
   QList<Fermentable*> ferms = a->fermentables();
   for( i = 0; i < ferms.size(); ++i )
      toXml( ferms[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MISCS");
   QList<Misc*> miscs = a->miscs();
   for( i = 0; i < miscs.size(); ++i )
      toXml( miscs[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("YEASTS");
   QList<Yeast*> yeasts = a->yeasts();
   for( i = 0; i < yeasts.size(); ++i )
      toXml( yeasts[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("WATERS");
   QList<Water*> waters = a->waters();
   for( i = 0; i < waters.size(); ++i )
      toXml( waters[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);
   
   Mash* mash = a->mash();
   if( mash != 0 )
      toXml( mash, doc, recipeNode);
   
   tmpNode = doc.createElement("INSTRUCTIONS");
   QList<Instruction*> instructions = a->instructions();
   for( i = 0; i < instructions.size(); ++i )
      toXml( instructions[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BREWNOTES");
   QList<BrewNote*> brewNotes = a->brewNotes();
   for(i=0; i < brewNotes.size(); ++i)
      toXml(brewNotes[i], doc, tmpNode);
   recipeNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ASST_BREWER");
   tmpText = doc.createTextNode(a->asstBrewer());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   Equipment* equip = a->equipment();
   if( equip )
      toXml( equip, doc, recipeNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TASTE_NOTES");
   tmpText = doc.createTextNode(a->tasteNotes());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TASTE_RATING");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tasteRating()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("OG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->og()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FG");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fg()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FERMENTATION_STAGES");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fermentationStages()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SECONDARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->secondaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SECONDARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->secondaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TERTIARY_AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tertiaryAge_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TERTIARY_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->tertiaryTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AGE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->age_days()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AGE_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ageTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->date()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARBONATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbonation_vols()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FORCED_CARBONATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->forcedCarbonation()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMING_SUGAR_NAME");
   tmpText = doc.createTextNode(a->primingSugarName());
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARBONATION_TEMP");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbonationTemp_c()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PRIMING_SUGAR_EQUIV");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->primingSugarEquiv()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("KEG_PRIMING_FACTOR");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->kegPrimingFactor()));
   tmpNode.appendChild(tmpText);
   recipeNode.appendChild(tmpNode);
   
   parent.appendChild(recipeNode);
}

void Database::toXml( Style* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement styleNode;
   QDomElement tmpNode;
   QDomText tmpText;

   styleNode = doc.createElement("STYLE");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY");
   tmpText = doc.createTextNode(a->category());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CATEGORY_NUMBER");
   tmpText = doc.createTextNode(a->categoryNumber());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_LETTER");
   tmpText = doc.createTextNode(a->styleLetter());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("STYLE_GUIDE");
   tmpText = doc.createTextNode(a->styleGuide());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(a->typeString());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ogMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("OG_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ogMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fgMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("FG_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->fgMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuMin()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("IBU_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ibuMax()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->colorMin_srm()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("COLOR_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->colorMax_srm()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->abvMin_pct()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("ABV_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->abvMax_pct()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MIN");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbMin_vol()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CARB_MAX");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->carbMax_vol()));
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PROFILE");
   tmpText = doc.createTextNode(a->profile());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INGREDIENTS");
   tmpText = doc.createTextNode(a->ingredients());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   tmpNode = doc.createElement("EXAMPLES");
   tmpText = doc.createTextNode(a->examples());
   tmpNode.appendChild(tmpText);
   styleNode.appendChild(tmpNode);

   parent.appendChild(styleNode);
}

void Database::toXml( Water* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement waterNode;
   QDomElement tmpNode;
   QDomText tmpText;

   waterNode = doc.createElement("WATER");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount_l()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CALCIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->calcium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BICARBONATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->bicarbonate_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SULFATE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->sulfate_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CHLORIDE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->chloride_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SODIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->sodium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MAGNESIUM");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->magnesium_ppm()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PH");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->ph()));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   parent.appendChild(waterNode);
}

void Database::toXml( Yeast* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement yeastNode;
   QDomElement tmpElement;
   QDomText tmpText;

   yeastNode = doc.createElement("YEAST");
   
   tmpElement = doc.createElement("NAME");
   tmpText = doc.createTextNode(a->name());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);
   
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->version()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TYPE");
   tmpText = doc.createTextNode(Yeast::types.at(a->type()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FORM");
   tmpText = doc.createTextNode(Yeast::forms.at(a->form()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amount()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->amountIsWeight()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("LABORATORY");
   tmpText = doc.createTextNode(a->laboratory());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PRODUCT_ID");
   tmpText = doc.createTextNode(a->productID());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MIN_TEMPERATURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->minTemperature_c()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_TEMPERATURE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxTemperature_c()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FLOCCULATION");
   tmpText = doc.createTextNode(Yeast::flocculations.at(a->flocculation()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ATTENUATION");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->attenuation_pct()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("NOTES");
   tmpText = doc.createTextNode(a->notes());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("BEST_FOR");
   tmpText = doc.createTextNode(a->bestFor());
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TIMES_CULTURED");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->timesCultured()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_REUSE");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->maxReuse()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ADD_TO_SECONDARY");
   tmpText = doc.createTextNode(BeerXMLElement::text(a->addToSecondary()));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   parent.appendChild(yeastNode);
}

// fromXml ====================================================================
void Database::fromXml( BeerXMLElement* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode )
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;
  
   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;
      
      xmlTag = node.nodeName();
      textNode = child.toText();
         
      if( xmlTagsToProperties.contains(xmlTag) )
      {
         switch( element->metaProperty(xmlTagsToProperties[xmlTag]).type() )
         {
            case QVariant::Bool:
               boolVal = BeerXMLElement::getBool(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = BeerXMLElement::getDouble(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = BeerXMLElement::getInt(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = BeerXMLElement::getDateTime(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = BeerXMLElement::getDate(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateVal);
               break;
            // NOTE: I believe that enum types like Fermentable::Type will go
            // here since Q_ENUMS() converts enums to strings. So, need to make
            // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = BeerXMLElement::getString(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), stringVal);
               break;
            default:
               Brewtarget::logW("Database::fromXML: don't understand property type.");
               break;
         }
      }
      else
      {
         //if( showWarnings )
         //   Brewtarget::logW(QString("Database::fromXML: Unsupported property: %1. Line %2").arg(xmlTag).arg(node.lineNumber()) );
      }
   }
}

BrewNote* Database::brewNoteFromXml( QDomNode const& node, Recipe* parent )
{
   BrewNote* ret = newBrewNote(parent);  
   fromXml( ret, BrewNote::tagToProp, node );
   //_setterCommandStack->flush();

   return ret;
}

Equipment* Database::equipmentFromXml( QDomNode const& node, Recipe* parent )
{
   Equipment* ret = newEquipment();

   fromXml( ret, Equipment::tagToProp, node );
   //_setterCommandStack->flush();

   if( parent )
      addToRecipe( parent, ret, true );

   return ret;
}

Fermentable* Database::fermentableFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Fermentable* ret = newFermentable();
   fromXml( ret, Fermentable::tagToProp, node );

   
   // Handle enums separately.
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<Fermentable::Type>(
                    Fermentable::types.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   //_setterCommandStack->flush();
   if( parent )
      addToRecipe( parent, ret, true );
   return ret;
}

int Database::getQualifiedHopTypeIndex(QString type, Hop* hop)
{
  if ( Hop::types.indexOf(type) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT htype FROM hop WHERE name='%1' AND htype != ''").arg(hop->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString htype = q.record().value(0).toString();
      q.finish();
      if ( htype != "" )
      {
         if ( Hop::types.indexOf(htype) >= 0 )
         {
            return Hop::types.indexOf(htype);
         }
      }
    }
    // out of ideas at this point so default to Both
    return Hop::types.indexOf(QString("Both"));
  }
  else
  {
     return Hop::types.indexOf(type);
  }
}

int Database::getQualifiedHopUseIndex(QString use, Hop* hop)
{
  if ( Hop::uses.indexOf(use) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT use FROM hop WHERE name='%1' AND use != ''").arg(hop->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString hUse = q.record().value(0).toString();
      q.finish();
      if ( hUse != "" )
         if ( Hop::uses.indexOf(hUse) >= 0 )
            return Hop::uses.indexOf(hUse);
    }
    // out of ideas at this point so default to Flavor
    return Hop::uses.indexOf(QString("Flavor"));
  }
  else
  {
     return Hop::uses.indexOf(use);
  }
}

Hop* Database::hopFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;

   Hop* ret = newHop();
   fromXml( ret, Hop::tagToProp, node );
   //_setterCommandStack->flush();
  
   // Handle enums separately.
   n = node.firstChildElement("USE");
   ret->setUse( static_cast<Hop::Use>(getQualifiedHopUseIndex(n.firstChild().toText().nodeValue(), ret)));
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<Hop::Type>(getQualifiedHopTypeIndex(n.firstChild().toText().nodeValue(), ret)));
   n = node.firstChildElement("FORM");
   ret->setForm( static_cast<Hop::Form>(
                    Hop::forms.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   
   if( parent )
      addToRecipe( parent, ret, true );
   return ret;
}

Instruction* Database::instructionFromXml( QDomNode const& node, Recipe* parent )
{
   Instruction* ret = newInstruction(parent);
   
   fromXml( ret, Instruction::tagToProp, node );
   //_setterCommandStack->flush();
   return ret;
}

Mash* Database::mashFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Mash* ret;
   if( parent )
      ret = newMash(parent);
   else
      ret = newMash();
   
   // First, get all the standard properties.
   fromXml( ret, Mash::tagToProp, node );
   //_setterCommandStack->flush();
   
   // Now, get the individual mash steps.
   n = node.firstChildElement("MASH_STEPS");
   if( n.isNull() )
      return ret;
   // Iterate through all the mash steps.
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      mashStepFromXml( n, ret );

   return ret;
}

MashStep* Database::mashStepFromXml( QDomNode const& node, Mash* parent )
{
   QDomNode n;
   MashStep* ret = newMashStep(parent);
   
   fromXml( ret, MashStep::tagToProp, node );
   
   // Handle enums separately.
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<MashStep::Type>(
                    MashStep::types.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   
   //_setterCommandStack->flush();
   return ret;
}

int Database::getQualifiedMiscTypeIndex(QString type, Misc* misc)
{
  if ( Misc::types.indexOf(type) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT mtype FROM misc WHERE name='%1' AND mtype != ''").arg(misc->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString mtype = q.record().value(0).toString();
      q.finish();
      if ( mtype != "" )
      {
         if ( Misc::types.indexOf(mtype) >= 0 )
         {
            return Misc::types.indexOf(mtype);
         }
      }
    }
    // out of ideas at this point so default to Flavor
    return Misc::types.indexOf(QString("Flavor"));
  }
  else
  {
     return Misc::types.indexOf(type);
  }
}

int Database::getQualifiedMiscUseIndex(QString use, Misc* misc)
{
  if ( Misc::uses.indexOf(use) < 0 )
  {
    // look for a valid hop type from our database to use
    QSqlQuery q(QString("SELECT use FROM misc WHERE name='%1' AND use != ''").arg(misc->name()), sqlDatabase());
    q.first();
    if ( q.isValid() )
    {
      QString mUse = q.record().value(0).toString();
      q.finish();
      if ( mUse != "" )
         if ( Misc::uses.indexOf(mUse) >= 0 )
            return Misc::uses.indexOf(mUse);
    }
    // out of ideas at this point so default to Flavor
    return Misc::uses.indexOf(QString("Flavor"));
  }
  else
  {
     return Misc::uses.indexOf(use);
  }
}

Misc* Database::miscFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Misc* ret = newMisc();
   
   fromXml( ret, Misc::tagToProp, node );
   //_setterCommandStack->flush();
   
   // Handle enums separately.
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<Misc::Type>(getQualifiedMiscTypeIndex(n.firstChild().toText().nodeValue(), ret)));
   n = node.firstChildElement("USE");
   ret->setUse(static_cast<Misc::Use>(getQualifiedMiscUseIndex(n.firstChild().toText().nodeValue(), ret)));
   
   if( parent )
      addToRecipe( parent, ret, true );
   return ret;
}

Recipe* Database::recipeFromXml( QDomNode const& node )
{
   QDomNode n;
   Recipe* ret = newRecipe();
  
   // Get standard properties.
   fromXml( ret, Recipe::tagToProp, node );
   //_setterCommandStack->flush();
   
   // Get style. Note: styleFromXml requires the entire node, not just the
   // firstchild of the node.
   n = node.firstChildElement("STYLE");
   styleFromXml(n, ret);
   
   // Get equipment. equipmentFromXml requires the entire node, not just the
   // first child
   n = node.firstChildElement("EQUIPMENT");
   equipmentFromXml(n, ret);
   
   // Get hops.
   n = node.firstChildElement("HOPS");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      hopFromXml(n, ret);
   
   // Get ferms.
   n = node.firstChildElement("FERMENTABLES");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      fermentableFromXml(n, ret);
   
   // get mashes. There is only one mash per recipe, so this needs the entire
   // node.
   n = node.firstChildElement("MASH");
   mashFromXml(n, ret);

   // Get miscs.
   n = node.firstChildElement("MISCS");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      miscFromXml(n, ret);
   
   // Get yeasts.
   n = node.firstChildElement("YEASTS");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      yeastFromXml(n, ret);
   
   // Get waters.
   n = node.firstChildElement("WATERS");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      waterFromXml(n, ret);
   
   // Get instructions.
   n = node.firstChildElement("INSTRUCTIONS");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      instructionFromXml(n, ret);

   // Get brew notes

   n = node.firstChildElement("BREWNOTES");
   for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      brewNoteFromXml(n, ret);

   return ret;
}

Style* Database::styleFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Style* ret = newStyle();

   fromXml( ret, Style::tagToProp, node );

   // Handle enums separately.
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<Style::Type>(
                    Style::types.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   
   //_setterCommandStack->flush();
   if( parent )
      addToRecipe( parent, ret );
   return ret;
}

Water* Database::waterFromXml( QDomNode const& node, Recipe* parent )
{
   Water* ret = newWater();
   fromXml( ret, Water::tagToProp, node );
   if( parent )
      addToRecipe( parent, ret, true );
   return ret;
}

Yeast* Database::yeastFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Yeast* ret = newYeast();
   fromXml( ret, Yeast::tagToProp, node );

   // Handle enums separately.
   n = node.firstChildElement("TYPE");
   ret->setType( static_cast<Yeast::Type>(
                    Yeast::types.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   // Handle enums separately.
   n = node.firstChildElement("FORM");
   ret->setForm( static_cast<Yeast::Form>(
                    Yeast::forms.indexOf(
                       n.firstChild().toText().nodeValue()
                    )
                 ) );
   // Handle enums separately.
   n = node.firstChildElement("FLOCCULATION");
   ret->setFlocculation( static_cast<Yeast::Flocculation>(
                            Yeast::flocculations.indexOf(
                               n.firstChild().toText().nodeValue()
                            )
                         ) );
   //_setterCommandStack->flush();
   if( parent )
      addToRecipe( parent, ret, true );
   return ret;
}

/*
 * Cleans up the backup database if an error occurred
 * during the previous Brewtarget session.
 */
bool Database::cleanupBackupDatabase()
{
   // Check if the temporary backup database exists.
   if (QFile::exists(dbTempBackupFileName))
   {
      // Check if the primary database also exists.
      if (QFile::exists(dbFileName))
      {
         // If it does, prompt the user as to whether they'd like to keep their changes
         // from the last session (keep the primary DB), or revert back to where they
         // were before their changes (overwrite the primary DB with the backup).
         QMessageBox messageBox;
         messageBox.setIcon(QMessageBox::Question);
         messageBox.setWindowTitle(QObject::tr("Multiple Databases Found"));
         messageBox.setText(QObject::tr("Multiple databases were found.  Do you want to "
                                                   "restore the changes you made during your last "
                                                   "Brewtarget session, or rollback to before last session's changes?"));

         // Add the Restore and Rollback buttons.
         QPushButton *restoreButton = messageBox.addButton(QObject::tr("Restore"), QMessageBox::AcceptRole);
         messageBox.addButton(QObject::tr("Rollback"), QMessageBox::RejectRole);
         messageBox.setDefaultButton(restoreButton);
         
         // Display the message box.
         messageBox.exec();

         // Check which button the user clicked.
         if (messageBox.clickedButton() == restoreButton)
         {
            // If they clicked Restore, then simply remove the backup database and keep
            // the primary.  If that fails, display a message as something likely has
            // the file locked, and it needs to be resolved outside Brewtarget.
            if (!QFile::remove(dbTempBackupFileName))
            {
               QMessageBox::critical(0,
                                     "Database Restore Failure",
                                     QString(QObject::tr("Failed to remove the temporary backup database.  "
                                                         "Navigate to '%1' and remove "
                                                         "'tempBackupDatabase.sqlite'.")).arg(Brewtarget::getUserDataDir()));

               return false;
            }
         }
         else
         {
            // If they clicked Rollback, replace the primary DB with the temporary backup
            // database.  If that fails, display a message as something likely has the
            // file locked, and it needs to be cleaned up outside Brewtarget.
            QFile::remove(dbFileName);
            if (!QFile::rename(dbTempBackupFileName, dbFileName))
            {
               QMessageBox::critical(0,
                                     "Database Rollback Failure",
                                     QString(QObject::tr("Failed to rollback to the backup database.  "
                                                         "Navigate to '%1', remove 'database.sqlite' if it exists, "
                                                         "and rename 'tempBackupDatabase.sqlite' to "
                                                         "'database.sqlite'.")).arg(Brewtarget::getUserDataDir()));

               return false;
            }
         }
      }
      else
      {
         // If the primary DB doesn't exist, then restore the backup database as the
         // primary.  If that fails, display a message as something likely has the
         // file locked, and it needs to be cleaned up outside Brewtarget.
         if (!QFile::rename(dbTempBackupFileName, dbFileName))
         {
            QMessageBox::critical(0,
                                  QObject::tr("Database Restore Failure"),
                                  QString(QObject::tr("Failed to restore the backup database. Navigate to '%1' "
                                                      "and rename 'tempBackupDatabase.sqlite' to "
                                                      "'database.sqlite'.").arg(Brewtarget::getUserDataDir())));

            return false;
         }
      }
   }

   return true;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QList<TableParams> Database::makeTableParams()
{
   typedef BeerXMLElement* (Database::*NewIngFunc)(void);
   
   QList<TableParams> ret;
   TableParams tmp;
   
   //=============================Equipment====================================
   
   tmp.tableName = "equipment";
   tmp.propName = QStringList() <<
      "name" << "boil_size" << "batch_size" << "tun_volume" << "tun_weight" <<
      "tun_specific_heat" << "top_up_water" << "trub_chiller_loss" <<
      "evap_rate" << "real_evap_rate" << "boil_time" << "calc_boil_volume" <<
      "lauter_deadspace" << "top_up_kettle" << "hop_utilization" <<
      "notes";
   tmp.newElement =
      (NewIngFunc)
      (Equipment*(Database::*)(void))
      &Database::newEquipment;
   
   ret.append(tmp);
   //==============================Fermentables================================
   
   tmp.tableName = "fermentable";
   tmp.propName = QStringList() <<
      "name" << "ftype" << "amount" << "yield" << "color" <<
      "add_after_boil" << "origin" << "supplier" << "notes" <<
      "coarse_fine_diff" << "moisture" << "diastatic_power" << "protein" <<
      "max_in_batch" << "recommend_mash" << "ibu_gal_per_lb";
   tmp.newElement =
      (NewIngFunc)
      (Fermentable*(Database::*)(void))
      &Database::newFermentable;
   
   //==============================Hops=============================
   tmp.tableName = "hop";
   tmp.propName = QStringList() <<
      "name" << "alpha" << "amount" << "use" << "time" << "notes" << "htype" <<
      "form" << "beta" << "hsi" << "origin" << "substitutes" << "humulene" <<
      "caryophyllene" << "cohumulone" << "myrcene",
   // First cast specifies which newHop() I want, since it is overloaded.
   // Second cast is to force the conversion of the function pointer.
   tmp.newElement =
      (NewIngFunc)
      (Hop*(Database::*)(void))
      &Database::newHop;
   
   ret.append(tmp);
   
   //==================================Miscs===================================
   
   tmp.tableName = "misc";
   tmp.propName = QStringList() <<
      "name" << "mtype" << "use" << "time" << "amount" << "amount_is_weight" <<
      "use_for" << "notes";
   tmp.newElement =
      (NewIngFunc)
      (Misc*(Database::*)(void))
      &Database::newMisc;
   
   ret.append(tmp);
   //==================================Styles==================================
   
   tmp.tableName = "style";
   tmp.propName = QStringList() <<
      "name" << "s_type" << "category" << "category_number" <<
      "style_letter" << "style_guide" << "og_min" << "og_max" << "fg_min" <<
      "fg_max" << "ibu_min" << "ibu_max" << "color_min" << "color_max" <<
      "abv_min" << "abv_max" << "carb_min" << "carb_max" << "notes" <<
      "profile" << "ingredients" << "examples";
   tmp.newElement =
      (NewIngFunc)
      (Style*(Database::*)(void))
      &Database::newStyle;
   
   ret.append(tmp);
   
   //==================================Yeasts==================================
   
   tmp.tableName = "yeast";
   tmp.propName = QStringList() <<
      "name" << "ytype" << "form" << "amount" << "amount_is_weight" <<
      "laboratory" << "product_id" << "min_temperature" << "max_temperature" <<
      "flocculation" << "attenuation" << "notes" << "best_for" <<
      "times_cultured" << "max_reuse" << "add_to_secondary";
   tmp.newElement =
      (NewIngFunc)
      (Yeast*(Database::*)(void))
      &Database::newYeast;
   
   ret.append(tmp);
   
   //===================================Waters=================================
   
   tmp.tableName = "water";
   tmp.propName = QStringList() <<
      "name" << "amount" << "calcium" << "bicarbonate" << "sulfate" <<
      "chloride" << "sodium" << "magnesium" << "ph" << "notes";
   tmp.newElement =
      (NewIngFunc)
      (Water*(Database::*)(void))
      &Database::newWater;
   
   ret.append(tmp);
   
   return ret;
}

void Database::updateDatabase(QString const& filename)
{
   // In the naming here "old" means our local database, and
   // "new" means the database coming from 'filename'.

   QVariant btid, newid, oldid;
   QVariant zero(0);
   
   QList<QVariant> propVal;
   QStringList varAndHolder;

   QString newCon("newSqldbCon");
   QSqlDatabase newSqldb = QSqlDatabase::addDatabase("QSQLITE", newCon);
   newSqldb.setDatabaseName(filename);
   if( ! newSqldb.open() )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.\n%2").arg(filename).arg(newSqldb.lastError().text()));
      QMessageBox::critical(0,
                            QObject::tr("Database Failure"),
                            QString(QObject::tr("Failed to open the database '%1'.").arg(filename)));
      return;
   }
   
   // This is the basic gist...
   // For each (id, hop_id) in newSqldb.bt_hop...
   
   // Call this newRecord
   // SELECT * FROM newSqldb.hop WHERE id=<hop_id>
   
   // UPDATE hop SET name=:name, alpha=:alpha,... WHERE id=(SELECT hop_id FROM bt_hop WHERE id=:bt_id)
   
   // Bind :bt_id from <id>
   // Bind :name, :alpha, ..., from newRecord.
   
   // Execute.

   foreach( TableParams tp, tableParams)
   {
      QSqlQuery qNewBtIng(
         QString("SELECT * FROM bt_%1").arg(tp.tableName),
         newSqldb );
                  
      QSqlQuery qNewIng( newSqldb );
      qNewIng.prepare(QString("SELECT * FROM %1 WHERE id=:id").arg(tp.tableName));
      
      // Construct the big update query.
      QSqlQuery qUpdateOldIng( sqlDatabase() );
      QString updateString = QString("UPDATE %1 SET ").arg(tp.tableName);
      varAndHolder.clear();
      foreach( QString pn, tp.propName)
         varAndHolder.append(QString("`%1`=:%2").arg(pn).arg(pn));
      updateString.append(varAndHolder.join(", "));
      // Un-delete it if it is somehow deleted.
      updateString.append(", `deleted`=:zero WHERE `id`=:id");
      qUpdateOldIng.prepare(updateString);
      qUpdateOldIng.bindValue( ":zero", zero );
      
      QSqlQuery qOldBtIng( sqlDatabase() );
      qOldBtIng.prepare(
         QString("SELECT * FROM bt_%1 WHERE `id`=:btid").arg(tp.tableName) );
      
      QSqlQuery qOldBtIngInsert( sqlDatabase() );
      qOldBtIngInsert.prepare(
         QString("INSERT INTO bt_%1 `id`=:id `%2_id`=:%3_id")
            .arg(tp.tableName)
            .arg(tp.tableName)
            .arg(tp.tableName) );
      
      // Resize propVal appropriately for current table.
      propVal.clear();
      foreach( QString pn, tp.propName )
         propVal.append(QVariant());
      
      while( qNewBtIng.next() )
      {
         btid = qNewBtIng.record().value("id");
         newid = qNewBtIng.record().value(QString("%1_id").arg(tp.tableName));
         
         qNewIng.bindValue(":id", newid);
         qNewIng.exec();
         if( !qNewIng.next() )
         {
            Brewtarget::logE(QString("Oops. %1").arg(qNewIng.lastError().text()));
            return;
         }
         
         QList<QVariant>::iterator it = propVal.begin();
         foreach( QString pn, tp.propName )
         {
            // Get new value.
            *it = qNewIng.record().value(pn);
            // Bind it to the old ingredient.
            qUpdateOldIng.bindValue(
               QString(":%1").arg(pn),
               *it );
            ++it;
         }
         
         // Done retrieving new ingredient data.
         qNewIng.finish();
         
         // Find the bt_<ingredient> record in the local table.
         qOldBtIng.bindValue( ":btid", btid );
         qOldBtIng.exec();
         
         // If the btid exists in the old bt_hop table, do an update.
         if( qOldBtIng.next() )
         {
            oldid = qOldBtIng.record().value(
               QString("%1_id").arg(tp.tableName) );
            qOldBtIng.finish();
            
            qUpdateOldIng.bindValue( ":id", oldid );
            
            qUpdateOldIng.exec();
            if( qUpdateOldIng.lastError().isValid() )
            {
               Brewtarget::logE(
                  QString("Database::updateDatabase(): %1")
                  .arg(qUpdateOldIng.lastError().text()) );
            }
         }
         // If the btid doesn't exist in the old bt_hop table, do an insert into
         // the old hop table, then into the old bt_hop table.
         else
         {
            // Create a new ingredient.
            oldid = (this->*(tp.newElement))()->_key;
            // Copy in the new data.
            qUpdateOldIng.bindValue( ":id", oldid );
            qUpdateOldIng.exec();
            if( qUpdateOldIng.lastError().isValid() )
            {
               Brewtarget::logE(
                  QString("Database::updateDatabase(): %1")
                  .arg(qUpdateOldIng.lastError().text()) );
            }
            
            // Insert an entry into our bt_<ingredient> table.
            qOldBtIngInsert.bindValue( ":id", btid );
            qOldBtIngInsert.bindValue( QString(":%1_id").arg(tp.tableName), oldid );
            qOldBtIngInsert.exec();
            if( qOldBtIng.lastError().isValid() )
            {
               Brewtarget::logE(
                  QString("Database::updateDatabase(): %1")
                  .arg(qOldBtIng.lastError().text()) );
            }
         }
      }
   }
}
