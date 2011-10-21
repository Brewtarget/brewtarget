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

bool Database::initialized = false;
Database* Database::internalDBInstance = 0;
QFile Database::dbFile;
QString Database::dbFileName;
QFile Database::recipeFile;
QString Database::recipeFileName;
QFile Database::mashFile;
QString Database::mashFileName;

Database::Database()
{
   // Don't EVER use this method to get the database!!!
}

Database* Database::getDatabase()
{
   if( initialized )
      return internalDBInstance;
   else
      return 0;
}

bool Database::isInitialized()
{
   return initialized;
}

void Database::initialize()
{
   // These are the user-space database, recipe and mash documents.
   QDomDocument dbDoc, recDoc, mashDoc, tmpDoc;
   QFile origDbFile, origRecFile, origMashFile;
   QDomNodeList list;
   QString err;
   int line;
   int col;
   unsigned int i, size;

   commandStack.setUndoLimit(100);
   
   dbFileName = (Brewtarget::getUserDataDir() + "database.xml");
   recipeFileName = (Brewtarget::getUserDataDir() + "recipes.xml");
   mashFileName = (Brewtarget::getUserDataDir() + "mashs.xml");

   dbFile.setFileName(dbFileName);
   recipeFile.setFileName(recipeFileName);
   mashFile.setFileName(mashFileName);

   // Try to open the files.
   if( ! dbFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not open %1 for reading.").arg(dbFile.fileName()));
      return;
   }
   if( ! recipeFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not open %1 for reading.").arg(recipeFile.fileName()));
      return;
   }
   if( ! mashFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QString("Could not open %1 for reading.").arg(recipeFile.fileName()));
      return;
   }

   // Parse the xml documents.
   if( ! dbDoc.setContent(&dbFile, false, &err, &line, &col) )
      Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(dbFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! recDoc.setContent(&recipeFile, false, &err, &line, &col) )
      Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(recipeFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! mashDoc.setContent(&mashFile, false, &err, &line, &col) )
      Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(mashFile.fileName()).arg(line).arg(col).arg(err) );

   // See if we should merge the dataspace database with the userspace
   // ones.
   origDbFile.setFileName( Brewtarget::getDataDir() + "database.xml" );
   origRecFile.setFileName( Brewtarget::getDataDir() + "recipes.xml" );
   origMashFile.setFileName( Brewtarget::getDataDir() + "mashs.xml" );
   if( origDbFile.open(QIODevice::ReadOnly)
       && origRecFile.open(QIODevice::ReadOnly)
       && origMashFile.open(QIODevice::ReadOnly)
       && origDbFile.fileName() != dbFile.fileName()
       && origRecFile.fileName() != recipeFile.fileName()
       && origMashFile.fileName() != mashFile.fileName()
       && ! Brewtarget::userDatabaseDidNotExist // Don't do this if we JUST copied the dataspace database.
       && QFileInfo(origDbFile).lastModified() > Brewtarget::lastDbMergeRequest )
   {
      // This works because the combination of stable sorting and undup's in this
      // class will ensure that there is exactly 1 entry with a certain name, and
      // if there is an ingredient in the dataspace database with the same name
      // as one in the userspace, then the userspace one is kept.

      if( QMessageBox::question(0,
                                QObject::tr("Merge Database"),
                                QObject::tr("There may be new ingredients and recipes available. Would you like to add these to your database?"),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes)
         == QMessageBox::Yes
         )
      {
         if( ! tmpDoc.setContent(&origDbFile, false, &err, &line, &col) )
            Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(origDbFile.fileName()).arg(line).arg(col).arg(err) );
         mergeBeerXMLDBDocs(dbDoc, tmpDoc);

         if( ! tmpDoc.setContent(&origRecFile, false, &err, &line, &col) )
            Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(origRecFile.fileName()).arg(line).arg(col).arg(err) );
         mergeBeerXMLRecDocs(recDoc, tmpDoc);

         if( ! tmpDoc.setContent(&origMashFile, false, &err, &line, &col) )
            Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(origMashFile.fileName()).arg(line).arg(col).arg(err) );
         mergeBeerXMLDBDocs(mashDoc, tmpDoc);
      }

      // Update this field.
      Brewtarget::lastDbMergeRequest = QDateTime::currentDateTime();
   }

   // Close the "orig" files.
   origDbFile.close();
   origRecFile.close();
   origMashFile.close();

   // TODO: implement the rest of this.
   
   // Setup equipments, fermentables, and all other table models.
   
   /*
   list = dbDoc.elementsByTagName("EQUIPMENT");
   size = list.size();
   for( i = 0; i < size; ++i )
      equipments.push_back(new Equipment(list.at(i)));
   */
   
   dbFile.close();
   recipeFile.close();
   mashFile.close();

   if( internalDBInstance == 0 )
      internalDBInstance = new Database();
   Database::initialized = true;
}

void Database::mergeBeerXMLRecDocs( QDomDocument& first, const QDomDocument& last )
{
  QDomNode root = first.elementsByTagName("RECIPES").at(0);;
   QDomNodeList list;
   int i, size;

   /*** Items in recDoc ***/
   list = last.elementsByTagName("RECIPE");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));
}

void Database::mergeBeerXMLDBDocs( QDomDocument& first, const QDomDocument& last )
{
   QDomNode root = first.elementsByTagName("DATABASE").at(0);
   QDomNodeList list;
   int i, size;

   /*** Items in dbDoc ***/
   list = last.elementsByTagName("EQUIPMENT");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("FERMENTABLE");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("HOP");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("MASH_STEP");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("MISC");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("STYLE");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("WATER");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   list = last.elementsByTagName("YEAST");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));

   /*** Items in mashDoc ***/
   list = last.elementsByTagName("MASH");
   size = list.size();
   for( i = 0; i < size; ++i )
      root.appendChild(list.at(i));
   
}

bool Database::backupToDir(QString dir)
{
   if( ! isInitialized() )
      initialize();
   
   savePersistent();
   bool success = true;
   QString prefix = dir + "/";
   QString dbFileName = prefix + "database.xml";
   QString recipeFileName = prefix + "recipes.xml";
   QString mashFileName = prefix + "mashs.xml";
   
   // Remove the files if they already exist so that
   // the copy() operation will succeed.
   QFile::remove(dbFileName);
   QFile::remove(recipeFileName);
   QFile::remove(mashFileName);
   
   success &= dbFile.copy( prefix + "database.xml" );
   success &= recipeFile.copy( prefix + "recipes.xml" );
   success &= mashFile.copy( prefix + "mashs.xml" );
   
   return success;
}

bool Database::restoreFromDir(QString dirStr)
{
   if( ! isInitialized() )
      initialize();
   
   bool success = true;
   QString prefix = dirStr + "/";
   QString dbFileName = prefix + "database.xml";
   QString recipeFileName = prefix + "recipes.xml";
   QString mashFileName = prefix + "mashs.xml";
   
   QFile newDbFile(dbFileName);
   QFile newRecipeFile(recipeFileName);
   QFile newMashFile(mashFileName);
   
   // Fail if we can't find even one of the required files.
   if( !newDbFile.exists() || !newRecipeFile.exists() || !newMashFile.exists() )
      return false;
   
   success &= dbFile.remove();
   success &= recipeFile.remove();
   success &= mashFile.remove();
   
   success &= newDbFile.copy(dbFile.fileName());
   success &= newRecipeFile.copy(recipeFile.fileName());
   success &= newMashFile.copy(mashFile.fileName());
   
   initialize();
   
   return success;
}

void Database::savePersistent()
{
   QDomDocument dbDoc, recDoc, mashDoc;
   QDomElement dbRoot, recRoot, mashRoot;
   
   if( ! dbFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for writing.").arg(dbFile.fileName()));
      return;
   }
   if( ! recipeFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for writing.").arg(recipeFile.fileName()));
      return;
   }
   if( ! mashFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for writing.").arg(mashFile.fileName()));
      return;
   }

   QTextStream dbOut(&dbFile);
   QTextStream recipeOut(&recipeFile);
   QTextStream mashOut(&mashFile);

   dbOut.setCodec( "UTF-8" );
   recipeOut.setCodec( "UTF-8" );
   mashOut.setCodec( "UTF-8" );

   /*** dbDoc ***/
   dbRoot = dbDoc.createElement("DATABASE");
   
   QList<Equipment*>::iterator eqit, eqend;
   eqend = equipments.end();
   for( eqit = equipments.begin(); eqit != eqend; ++eqit )
      (*eqit)->toXml(dbDoc, dbRoot);
   
   QList<Fermentable*>::iterator fit, fend;
   fend = fermentables.end();
   for( fit = fermentables.begin(); fit != fend; ++fit )
      (*fit)->toXml(dbDoc, dbRoot);
   
   QList<Hop*>::iterator hit, hend;
   hend = hops.end();
   for( hit = hops.begin(); hit != hend; ++hit )
      (*hit)->toXml(dbDoc, dbRoot);
   
   QList<MashStep*>::iterator msit, msend;
   msend = mashSteps.end();
   for( msit = mashSteps.begin(); msit != msend; ++msit )
      (*msit)->toXml(dbDoc, dbRoot);
   
   QList<Misc*>::iterator miscit, miscend;
   miscend = miscs.end();
   for( miscit = miscs.begin(); miscit != miscend; ++miscit )
      (*miscit)->toXml(dbDoc, dbRoot);
   
   QList<Style*>::iterator sit, send;
   send = styles.end();
   for( sit = styles.begin(); sit != send; ++sit )
      (*sit)->toXml(dbDoc, dbRoot);
   
   QList<Water*>::iterator wit, wend;
   wend = waters.end();
   for( wit = waters.begin(); wit != wend; ++wit )
      (*wit)->toXml(dbDoc, dbRoot);
   
   QList<Yeast*>::iterator yit, yend;
   yend = yeasts.end();
   for( yit = yeasts.begin(); yit != yend; ++yit )
      (*yit)->toXml(dbDoc, dbRoot);
   
   dbDoc.appendChild(dbRoot);
   dbOut <<  "<?xml version=\"1.0\" encoding=\"" << dbOut.codec()->name() << "\"?>\n";
   dbOut << dbDoc.toString();
   /*** END dbDoc ***/
   
   /*** recDoc ***/
   recRoot = recDoc.createElement("RECIPES");
   
   QList<Recipe*>::iterator rit, rend;
   rend = recipes.end();
   for( rit = recipes.begin(); rit != rend; ++rit )
      (*rit)->toXml(recDoc, recRoot);
   
   recDoc.appendChild(recRoot);
   recipeOut <<  "<?xml version=\"1.0\" encoding=\"" << recipeOut.codec()->name() << "\"?>\n";
   recipeOut << recDoc.toString();
   /*** END recDoc ***/
   
   /*** mashDoc ***/
   mashRoot = mashDoc.createElement("MASHS");
   
   QList<Mash*>::iterator mait, maend;
   maend = mashs.end();
   for( mait = mashs.begin(); mait != maend; ++mait )
      (*mait)->toXml(mashDoc, mashRoot);
   
   mashDoc.appendChild(mashRoot);
   mashOut <<  "<?xml version=\"1.0\" encoding=\"" << mashOut.codec()->name() << "\"?>\n";
   mashOut << mashDoc.toString();
   /*** END mashDoc ***/
   
   dbFile.close();
   recipeFile.close();
   mashFile.close();
}

Equipment* Database::newEquipment()
{
   // TODO: implement.
   return 0;
}

Fermentable* Database::newFermentable()
{
   // TODO: implement.
   return 0;
}

Hop* Database::newHop()
{
   // TODO: implement.
   return 0;
}

Hop* Database::newHop()
{
   // TODO: implement.
   return 0;
}

MashStep* Database::newMashStep()
{
   // TODO: implement.
   return 0;
}

Misc* Database::newMisc()
{
   // TODO: implement.
   return 0;
}

Recipe* Database::newRecipe()
{
   // TODO: implement.
   return 0;
}

Style* Database::newStyle()
{
   // TODO: implement.
   return 0;
}

Water* Database::newWater()
{
   // TODO: implement.
   return 0;
}

Yeast* Database::newYeast()
{
   // TODO: implement.
   return 0;
}

void Database::removeEquipment(Equipment* equip)
{
   // TODO: implement.
}

void Database::removeEquipment(QList<Equipment*> equip)
{
   // TODO: implement.
}

void Database::removeFermentable(Fermentable* ferm)
{
   // TODO: implement.
}

void Database::removeFermentable(QList<Fermentable*> ferm)
{
   // TODO: implement.
}

void Database::removeHop(Hop* hop)
{
   // TODO: implement.
}

void Database::removeHop(QList<Hop*> hop)
{
   // TODO: implement.
}

void Database::removeMash(Mash* mash)
{
   // TODO: implement.
}

void Database::removeMash(QList<Mash*> mash)
{
   // TODO: implement.
}

void Database::removeMashStep(MashStep* mashStep)
{
   // TODO: implement.
}

void Database::removeMashStep(QList<MashStep*> mashStep)
{
   // TODO: implement.
}

void Database::removeMisc(Misc* misc)
{
   // TODO: implement.
}

void Database::removeMisc(QList<Misc*> misc)
{
   // TODO: implement.
}

void Database::removeRecipe(Recipe* rec)
{
   // TODO: implement.
}

void Database::removeRecipe(QList<Recipe*> rec)
{
   // TODO: implement.
}

void Database::removeStyle(Style* style)
{
   // TODO: implement.
}

void Database::removeStyle(QList<Style*> style)
{
   // TODO: implement.
}

void Database::removeWater(Water* water)
{
  // TODO: implement.
}

void Database::removeWater(QList<Water*> water)
{
   // TODO: implement.
}

void Database::removeYeast(Yeast* yeast)
{
   // TODO: implement.
}

void Database::removeYeast(QList<Yeast*> yeast)
{
   // TODO: implement.
}

unsigned int Database::getNumEquipments()
{
   // TODO: implement.
}

unsigned int Database::getNumFermentables()
{
   // TODO: implement.
}

unsigned int Database::getNumHops()
{
   // TODO: implement.
}

unsigned int Database::getNumMashs()
{
   // TODO: implement.
}

unsigned int Database::getNumMashSteps()
{
   // TODO: implement.
}

unsigned int Database::getNumMiscs()
{
   // TODO: implement.
}

unsigned int Database::getNumRecipes()
{
   // TODO: implement.
}

unsigned int Database::getNumStyles()
{
   // TODO: implement.
}

unsigned int Database::getNumWaters()
{
   // TODO: implement.
}

unsigned int Database::getNumYeasts()
{
   // TODO: implement.
}


QList<Equipment*>::iterator Database::getEquipmentBegin()
{
   // TODO: implement.
}

QList<Equipment*>::iterator Database::getEquipmentEnd()
{
   // TODO: implement.
}

QList<Fermentable*>::iterator Database::getFermentableBegin()
{
   // TODO: implement.
}

QList<Fermentable*>::iterator Database::getFermentableEnd()
{
   // TODO: implement.
}

QList<Hop*>::iterator Database::getHopBegin()
{
   // TODO: implement.
}

QList<Hop*>::iterator Database::getHopEnd()
{
   // TODO: implement.
}

QList<Mash*>::iterator Database::getMashBegin()
{
   // TODO: implement.
}

QList<Mash*>::iterator Database::getMashEnd()
{
   // TODO: implement.
}

QList<MashStep*>::iterator Database::getMashStepBegin()
{
   // TODO: implement.
}

QList<MashStep*>::iterator Database::getMashStepEnd()
{
   // TODO: implement.
}

QList<Misc*>::iterator Database::getMiscBegin()
{
   // TODO: implement.
}

QList<Misc*>::iterator Database::getMiscEnd()
{
   // TODO: implement.
}

QList<Recipe*>::iterator Database::getRecipeBegin()
{
   // TODO: implement.
}

QList<Recipe*>::iterator Database::getRecipeEnd()
{
   // TODO: implement.
}

QList<Style*>::iterator Database::getStyleBegin()
{
   // TODO: implement.
}

QList<Style*>::iterator Database::getStyleEnd()
{
   // TODO: implement.
}

QList<Water*>::iterator Database::getWaterBegin()
{
   // TODO: implement.
}

QList<Water*>::iterator Database::getWaterEnd()
{
   // TODO: implement.
}

QList<Yeast*>::iterator Database::getYeastBegin()
{
   // TODO: implement.
}

QList<Yeast*>::iterator Database::getYeastEnd()
{
   // TODO: implement.
}

QString Database::getDbFileName()
{
   return dbFileName;
}

QString Database::getRecipeFileName()
{
   return recipeFileName;
}

void Database::updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object )
{
   SetterCommand command(table, keyName(table), key, col_name, value, prop, object);
   // For now, immediately execute the command.
   command.redo();
   
   // Push the command on the undo stack.
   //commandStack.beginMacro("Change an entry");
   commandStack.push(command);
   //commandStack.endMacro();
}
