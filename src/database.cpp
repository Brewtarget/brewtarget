/*
 * database.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include <QObject>
#include <QString>

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

// Grrr... stupid C++. Have to define these outside the class AGAIN.
QList<Equipment*> Database::equipments;
QList<Fermentable*> Database::fermentables;
QList<Hop*> Database::hops;
QList<Mash*> Database::mashs;
QList<MashStep*> Database::mashSteps;
QList<Misc*> Database::miscs;
QList<Recipe*> Database::recipes;
QList<Style*> Database::styles;
QList<Water*> Database::waters;
QList<Yeast*> Database::yeasts;
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
   QDomDocument dbDoc, recDoc, mashDoc;
   QDomNodeList list;
   QString err;
   int line;
   int col;
   unsigned int i, size;

   dbFileName = (Brewtarget::getConfigDir() + "database.xml");
   recipeFileName = (Brewtarget::getConfigDir() + "recipes.xml");
   mashFileName = (Brewtarget::getConfigDir() + "mashs.xml");

   dbFile.setFileName(dbFileName);
   recipeFile.setFileName(recipeFileName);
   mashFile.setFileName(mashFileName);

   // Try to open the files.
   if( ! dbFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for reading.").arg(dbFile.fileName()));
      return;
   }
   if( ! recipeFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for reading.").arg(recipeFile.fileName()));
      return;
   }
   if( ! mashFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, QObject::tr("Could not open %1 for reading.").arg(recipeFile.fileName()));
      return;
   }

   // Parse the xml documents.
   if( ! dbDoc.setContent(&dbFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad document formatting in %1 %2:%3. %4").arg(dbFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! recDoc.setContent(&recipeFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad document formatting in %1 %2:%3. %4").arg(recipeFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! mashDoc.setContent(&mashFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QObject::tr("Bad document formatting in %1 %2:%3. %4").arg(mashFile.fileName()).arg(line).arg(col).arg(err) );

   equipments.clear();
   fermentables.clear();
   hops.clear();
   mashSteps.clear();
   miscs.clear();
   styles.clear();
   waters.clear();
   yeasts.clear();
   mashs.clear();
   recipes.clear();

   /*** Items in dbDoc ***/
   list = dbDoc.elementsByTagName("EQUIPMENT");
   size = list.size();
   for( i = 0; i < size; ++i )
      equipments.push_back(new Equipment( list.at(i) ));
   list = dbDoc.elementsByTagName("FERMENTABLE");
   size = list.size();
   for( i = 0; i < size; ++i )
      fermentables.push_back( new Fermentable( list.at(i) ) );
   list = dbDoc.elementsByTagName("HOP");
   size = list.size();
   for( i = 0; i < size; ++i )
      hops.push_back(new Hop( list.at(i) ));
   list = dbDoc.elementsByTagName("MASH_STEP");
   size = list.size();
   for( i = 0; i < size; ++i )
      mashSteps.push_back(new MashStep( list.at(i) ));
   list = dbDoc.elementsByTagName("MISC");
   size = list.size();
   for( i = 0; i < size; ++i )
      miscs.push_back(new Misc( list.at(i) ));
   list = dbDoc.elementsByTagName("STYLE");
   size = list.size();
   for( i = 0; i < size; ++i )
      styles.push_back(new Style(list.at(i)));
   list = dbDoc.elementsByTagName("WATER");
   size = list.size();
   for( i = 0; i < size; ++i )
      waters.push_back(new Water(list.at(i)));
   list = dbDoc.elementsByTagName("YEAST");
   size = list.size();
   for( i = 0; i < size; ++i )
      yeasts.push_back(new Yeast(list.at(i)));

   /*** Items in mashDoc ***/
   list = mashDoc.elementsByTagName("MASH");
   size = list.size();
   for( i = 0; i < size; ++i )
      mashs.push_back(new Mash(list.at(i)));

   /*** Items in recDoc ***/
   list = recDoc.elementsByTagName("RECIPE");
   size = list.size();
   for( i = 0; i < size; ++i )
      recipes.push_back(new Recipe(list.at(i)));

   // Sort everything by name.
   qSort(equipments.begin(), equipments.end(), EquipmentPtrLt );
   //equipments.sort(Equipment_ptr_cmp());
   qSort( fermentables.begin(), fermentables.end(), FermentablePtrLt );
   //fermentables.sort(Fermentable_ptr_cmp());
   qSort( hops.begin(), hops.end(), HopPtrLt );
   //hops.sort(Hop_ptr_cmp());
   qSort( mashs.begin(), mashs.end(), MashPtrLt );
   //mashs.sort(Mash_ptr_cmp());
   qSort( mashSteps.begin(), mashSteps.end(), MashStepPtrLt );
   //mashSteps.sort(MashStep_ptr_cmp());
   qSort( miscs.begin(), miscs.end(), MiscPtrLt );
   //miscs.sort(Misc_ptr_cmp());
   qSort( recipes.begin(), recipes.end(), RecipePtrLt );
   //recipes.sort(Recipe_ptr_cmp());
   qSort( styles.begin(), styles.end(), StylePtrLt );
   //styles.sort(Style_ptr_cmp());
   qSort( waters.begin(), waters.end(), WaterPtrLt );
   //waters.sort(Water_ptr_cmp());
   qSort( yeasts.begin(), yeasts.end(), YeastPtrLt );
   //yeasts.sort(Yeast_ptr_cmp());

   dbFile.close();
   recipeFile.close();
   mashFile.close();

   if( internalDBInstance == 0 )
      internalDBInstance = new Database();
   Database::initialized = true;
   
   internalDBInstance->hasChanged(QVariant(DBALL));
}

void Database::resortAll()
{
   // Sort everything by name.
   qSort(equipments.begin(), equipments.end(), EquipmentPtrLt );
   //equipments.sort(Equipment_ptr_cmp());
   qSort( fermentables.begin(), fermentables.end(), FermentablePtrLt );
   //fermentables.sort(Fermentable_ptr_cmp());
   qSort( hops.begin(), hops.end(), HopPtrLt );
   //hops.sort(Hop_ptr_cmp());
   qSort( mashs.begin(), mashs.end(), MashPtrLt );
   //mashs.sort(Mash_ptr_cmp());
   qSort( mashSteps.begin(), mashSteps.end(), MashStepPtrLt );
   //mashSteps.sort(MashStep_ptr_cmp());
   qSort( miscs.begin(), miscs.end(), MiscPtrLt );
   //miscs.sort(Misc_ptr_cmp());
   qSort( recipes.begin(), recipes.end(), RecipePtrLt );
   //recipes.sort(Recipe_ptr_cmp());
   qSort( styles.begin(), styles.end(), StylePtrLt );
   //styles.sort(Style_ptr_cmp());
   qSort( waters.begin(), waters.end(), WaterPtrLt );
   //waters.sort(Water_ptr_cmp());
   qSort( yeasts.begin(), yeasts.end(), YeastPtrLt );
   //yeasts.sort(Yeast_ptr_cmp());

   hasChanged(QVariant(DBALL));
}

void Database::resortEquipments()
{
   qSort(equipments.begin(), equipments.end(), EquipmentPtrLt );
   hasChanged(QVariant(DBEQUIP));
}

void Database::resortFermentables()
{
   qSort( fermentables.begin(), fermentables.end(), FermentablePtrLt );
   hasChanged(QVariant(DBFERM));
}

void Database::resortHops()
{
   qSort( hops.begin(), hops.end(), HopPtrLt );
   hasChanged(QVariant(DBHOP));
}

void Database::resortMiscs()
{
   qSort( miscs.begin(), miscs.end(), MiscPtrLt );
   hasChanged(QVariant(DBMISC));
}

void Database::resortStyles()
{
   qSort( styles.begin(), styles.end(), StylePtrLt );
   hasChanged(QVariant(DBSTYLE));
}

void Database::resortYeasts()
{
   qSort( yeasts.begin(), yeasts.end(), YeastPtrLt );
   hasChanged(QVariant(DBYEAST));
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
   dbOut << dbDoc.toString();
   /*** END dbDoc ***/
   
   /*** recDoc ***/
   recRoot = recDoc.createElement("RECIPES");
   
   QList<Recipe*>::iterator rit, rend;
   rend = recipes.end();
   for( rit = recipes.begin(); rit != rend; ++rit )
      (*rit)->toXml(recDoc, recRoot);
   
   recDoc.appendChild(recRoot);
   recipeOut << recDoc.toString();
   /*** END recDoc ***/
   
   /*** mashDoc ***/
   mashRoot = mashDoc.createElement("MASHS");
   
   QList<Mash*>::iterator mait, maend;
   maend = mashs.end();
   for( mait = mashs.begin(); mait != maend; ++mait )
      (*mait)->toXml(mashDoc, mashRoot);
   
   mashDoc.appendChild(mashRoot);
   mashOut << mashDoc.toString();
   /*** END mashDoc ***/
   
   dbFile.close();
   recipeFile.close();
   mashFile.close();
}

//=========================accessor methods=====================================

// TODO: restructure the database to use maps so that this process is fast.
void Database::addEquipment(Equipment* equip)
{
   if( equip != 0 )
   {
      equipments.push_back(equip);
      qSort(equipments.begin(), equipments.end(), EquipmentPtrLt );
      Algorithms::Instance().unDup(equipments, EquipmentPtrEq);
      //equipments.unique(Equipment_ptr_equals()); // No dups.
      hasChanged(QVariant(DBEQUIP));
   }
}

void Database::addFermentable(Fermentable* ferm)
{
   if( ferm != 0 )
   {
      fermentables.push_back(ferm);
      qSort(fermentables.begin(), fermentables.end(), FermentablePtrLt );
      Algorithms::Instance().unDup(fermentables, FermentablePtrEq);
      //fermentables.unique(Fermentable_ptr_equals());
      hasChanged(QVariant(DBFERM));
   }
}

void Database::addHop(Hop* hop)
{
   if( hop != 0 )
   {
      hops.push_back(hop);
      qSort(hops.begin(), hops.end(), HopPtrLt );
      Algorithms::Instance().unDup(hops, HopPtrEq);
      //hops.unique(Hop_ptr_equals());
      hasChanged(QVariant(DBHOP));
   }
}

void Database::addMash(Mash* mash)
{
   if( mash != 0 )
   {
      mashs.push_back(mash);
      qSort(mashs.begin(), mashs.end(), MashPtrLt );
      Algorithms::Instance().unDup(mashs, MashPtrEq);
      //mashs.unique(Mash_ptr_equals());
      hasChanged(QVariant(DBMASH));
   }
}

void Database::addMashStep(MashStep* mashStep)
{
   if( mashStep != 0 )
   {
      mashSteps.push_back(mashStep);
      qSort(mashSteps.begin(), mashSteps.end(), MashStepPtrLt );
      Algorithms::Instance().unDup(mashSteps, MashStepPtrEq);
      //mashSteps.unique(MashStep_ptr_equals());
      hasChanged(QVariant(DBMASHSTEP));
   }
}

void Database::addMisc(Misc* misc)
{
   if( misc != 0 )
   {
      miscs.push_back(misc);
      qSort(miscs.begin(), miscs.end(), MiscPtrLt );
      Algorithms::Instance().unDup(miscs, MiscPtrEq);
      //miscs.unique(Misc_ptr_equals());
      hasChanged(QVariant(DBMISC));
   }
}

void Database::addRecipe(Recipe* rec, bool copySubelements)
{
   if( rec == 0 )
      return;

   recipes.push_back(rec);
   qSort(recipes.begin(), recipes.end(), RecipePtrLt );
   Algorithms::Instance().unDup(recipes, RecipePtrEq);
   //recipes.unique(Recipe_ptr_equals());

   if( copySubelements )
   {
      unsigned int i, size;
      addEquipment(rec->getEquipment());
      addMash(rec->getMash());
      addStyle(rec->getStyle());

      size = rec->getNumFermentables();
      for( i = 0; i < size; ++i )
         addFermentable( rec->getFermentable(i) );
      size = rec->getNumHops();
      for( i = 0; i < size; ++i )
         addHop( rec->getHop(i) );
      size = rec->getNumMiscs();
      for( i = 0; i < size; ++i )
         addMisc( rec->getMisc(i) );
      size = rec->getNumWaters();
      for( i = 0; i < size; ++i )
         addWater( rec->getWater(i) );
      size = rec->getNumYeasts();
      for( i = 0; i < size; ++i )
         addYeast( rec->getYeast(i) );
   }

   hasChanged(DBRECIPE);

}

void Database::addStyle(Style* style)
{
   if( style != 0 )
   {
      styles.push_back(style);
      qSort(styles.begin(), styles.end(), StylePtrLt );
      Algorithms::Instance().unDup(styles, StylePtrEq);
      //styles.unique(Style_ptr_equals());
      hasChanged(QVariant(DBSTYLE));
   }
}

void Database::removeEquipment(Equipment* equip)
{
   equipments.removeOne(equip);
   hasChanged(QVariant(DBEQUIP));
}

void Database::removeFermentable(Fermentable* ferm)
{
   fermentables.removeOne(ferm);
   hasChanged(QVariant(DBFERM));
}

void Database::removeHop(Hop* hop)
{
   hops.removeOne(hop);
   hasChanged(QVariant(DBHOP));
}

void Database::removeMash(Mash* mash)
{
   mashs.removeOne(mash);
   hasChanged(QVariant(DBMASH));
}

void Database::removeMashStep(MashStep* mashStep)
{
   mashSteps.removeOne(mashStep);
   hasChanged(QVariant(DBMASHSTEP));
}

void Database::removeMisc(Misc* misc)
{
   miscs.removeOne(misc);
   hasChanged(QVariant(DBMISC));
}

void Database::removeRecipe(Recipe* rec)
{
   recipes.removeOne(rec);
   hasChanged(QVariant(DBRECIPE));
}

void Database::removeStyle(Style* style)
{
   styles.removeOne(style); // Wow, that was easy.
   hasChanged(QVariant(DBSTYLE));
}

void Database::removeWater(Water* water)
{
   waters.removeOne(water);
   hasChanged(QVariant(DBWATER));
}

void Database::removeYeast(Yeast* yeast)
{
   yeasts.removeOne(yeast);
   hasChanged(QVariant(DBYEAST));
}

void Database::addWater(Water* water)
{
   if( water != 0 )
   {
      waters.push_back(water);
      qSort(waters.begin(), waters.end(), WaterPtrLt );
      Algorithms::Instance().unDup(waters, WaterPtrEq);
      //waters.unique(Water_ptr_equals());
      hasChanged(QVariant(DBWATER));
   }
}

void Database::addYeast(Yeast* yeast)
{
   if( yeast != 0 )
   {
      yeasts.push_back(yeast);
      qSort(yeasts.begin(), yeasts.end(), YeastPtrLt );
      Algorithms::Instance().unDup(yeasts, YeastPtrEq);
      //yeasts.unique(Yeast_ptr_equals());
      hasChanged(QVariant(DBYEAST));
   }
}


unsigned int Database::getNumEquipments()
{
   return equipments.size();
}

unsigned int Database::getNumFermentables()
{
   return fermentables.size();
}

unsigned int Database::getNumHops()
{
   return hops.size();
}

unsigned int Database::getNumMashs()
{
   return mashs.size();
}

unsigned int Database::getNumMashSteps()
{
   return mashSteps.size();
}

unsigned int Database::getNumMiscs()
{
   return miscs.size();
}

unsigned int Database::getNumRecipes()
{
   return recipes.size();
}

unsigned int Database::getNumStyles()
{
   return styles.size();
}

unsigned int Database::getNumWaters()
{
   return waters.size();
}

unsigned int Database::getNumYeasts()
{
   return yeasts.size();
}


QList<Equipment*>::iterator Database::getEquipmentBegin()
{
   return equipments.begin();
}

QList<Equipment*>::iterator Database::getEquipmentEnd()
{
   return equipments.end();
}

QList<Fermentable*>::iterator Database::getFermentableBegin()
{
   return fermentables.begin();
}

QList<Fermentable*>::iterator Database::getFermentableEnd()
{
   return fermentables.end();
}

QList<Hop*>::iterator Database::getHopBegin()
{
   return hops.begin();
}

QList<Hop*>::iterator Database::getHopEnd()
{
   return hops.end();
}

QList<Mash*>::iterator Database::getMashBegin()
{
   return mashs.begin();
}

QList<Mash*>::iterator Database::getMashEnd()
{
   return mashs.end();
}

QList<MashStep*>::iterator Database::getMashStepBegin()
{
   return mashSteps.begin();
}

QList<MashStep*>::iterator Database::getMashStepEnd()
{
   return mashSteps.end();
}

QList<Misc*>::iterator Database::getMiscBegin()
{
   return miscs.begin();
}

QList<Misc*>::iterator Database::getMiscEnd()
{
   return miscs.end();
}

QList<Recipe*>::iterator Database::getRecipeBegin()
{
   return recipes.begin();
}

QList<Recipe*>::iterator Database::getRecipeEnd()
{
   return recipes.end();
}

QList<Style*>::iterator Database::getStyleBegin()
{
   return styles.begin();
}

QList<Style*>::iterator Database::getStyleEnd()
{
   return styles.end();
}

QList<Water*>::iterator Database::getWaterBegin()
{
   return waters.begin();
}

QList<Water*>::iterator Database::getWaterEnd()
{
   return waters.end();
}

QList<Yeast*>::iterator Database::getYeastBegin()
{
   return yeasts.begin();
}

QList<Yeast*>::iterator Database::getYeastEnd()
{
   return yeasts.end();
}

Equipment* Database::findEquipmentByName(QString name)
{
   QList<Equipment*>::iterator it, end;
   end = equipments.end();

   for( it = equipments.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }
   
   return 0;
}

Fermentable* Database::findFermentableByName(QString name)
{
   QList<Fermentable*>::iterator it, end;
   end = fermentables.end();

   for( it = fermentables.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Hop* Database::findHopByName(QString name)
{
   QList<Hop*>::iterator it, end;
   end = hops.end();

   for( it = hops.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Mash* Database::findMashByName(QString name)
{
   QList<Mash*>::iterator it, end;
   end = mashs.end();

   for( it = mashs.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

MashStep* Database::findMashStepByName(QString name)
{
   QList<MashStep*>::iterator it, end;
   end = mashSteps.end();

   for( it = mashSteps.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Misc* Database::findMiscByName(QString name)
{
   QList<Misc*>::iterator it, end;
   end = miscs.end();

   for( it = miscs.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Recipe* Database::findRecipeByName(QString name)
{
   QList<Recipe*>::iterator it, end;
   end = recipes.end();

   for( it = recipes.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Style* Database::findStyleByName(QString name)
{
   QList<Style*>::iterator it, end;
   end = styles.end();

   for( it = styles.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Water* Database::findWaterByName(QString name)
{
   QList<Water*>::iterator it, end;
   end = waters.end();

   for( it = waters.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Yeast* Database::findYeastByName(QString name)
{
   QList<Yeast*>::iterator it, end;
   end = yeasts.end();

   for( it = yeasts.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}
