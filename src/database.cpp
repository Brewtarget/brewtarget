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

#include <list>
#include <iostream>
#include <fstream>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>

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

#include "xmlnode.h"
#include "xmltree.h"
#include "config.h"
#include "brewtarget.h"

// Grrr... stupid C++. Have to define these outside the class AGAIN.
std::list<Equipment*> Database::equipments;
std::list<Fermentable*> Database::fermentables;
std::list<Hop*> Database::hops;
std::list<Mash*> Database::mashs;
std::list<MashStep*> Database::mashSteps;
std::list<Misc*> Database::miscs;
std::list<Recipe*> Database::recipes;
std::list<Style*> Database::styles;
std::list<Water*> Database::waters;
std::list<Yeast*> Database::yeasts;
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

   if (!dbFile.exists())
      QFile::copy(Brewtarget::getDataDir() + "database.xml", dbFileName);
   if (!recipeFile.exists())
      QFile::copy(Brewtarget::getDataDir() + "recipes.xml", recipeFileName);
   if (!mashFile.exists())
      QFile::copy(Brewtarget::getDataDir() + "mashs.xml", mashFileName);

   // Try to open the files.
   if( ! dbFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + dbFile.fileName().toStdString() + " for reading.");
      return;
   }
   if( ! recipeFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + recipeFile.fileName().toStdString() + " for reading.");
      return;
   }
   if( ! mashFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + recipeFile.fileName().toStdString() + " for reading.");
      return;
   }

   // Parse the xml documents.
   if( ! dbDoc.setContent(&dbFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QString("Bad document formatting in %1 %2:%3. %4").arg(dbFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! recDoc.setContent(&recipeFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QString("Bad document formatting in %1 %2:%3. %4").arg(recipeFile.fileName()).arg(line).arg(col).arg(err) );
   if( ! mashDoc.setContent(&mashFile, false, &err, &line, &col) )
      Brewtarget::log(Brewtarget::WARNING, QString("Bad document formatting in %1 %2:%3. %4").arg(mashFile.fileName()).arg(line).arg(col).arg(err) );

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
   equipments.sort(Equipment_ptr_cmp());
   fermentables.sort(Fermentable_ptr_cmp());
   hops.sort(Hop_ptr_cmp());
   mashs.sort(Mash_ptr_cmp());
   mashSteps.sort(MashStep_ptr_cmp());
   miscs.sort(Misc_ptr_cmp());
   recipes.sort(Recipe_ptr_cmp());
   styles.sort(Style_ptr_cmp());
   waters.sort(Water_ptr_cmp());
   yeasts.sort(Yeast_ptr_cmp());

   dbFile.close();
   recipeFile.close();
   mashFile.close();

   internalDBInstance = new Database();
   Database::initialized = true;
   
   /*
   dbFile.open(dbFileName.toStdString().c_str());
   recipeFile.open(recipeFileName.toStdString().c_str()); // Why are these separate from the dbFile? To prevent duplicates.
   mashFile.open(mashFileName.toStdString().c_str()); // Why are these separate from the dbFile? To prevent duplicates.
   
   unsigned int i, size;
   std::vector<XmlNode*> nodes;
   XmlTree* tree = new XmlTree(dbFile);

   size = tree->getNodesWithTag(nodes, "EQUIPMENT");
   for( i = 0; i < size; ++i )
      equipments.push_back(new Equipment(nodes[i]));
   size = tree->getNodesWithTag(nodes, "FERMENTABLE");
   for( i = 0; i < size; ++i )
      fermentables.push_back(new Fermentable(nodes[i]));
   size = tree->getNodesWithTag(nodes, "HOP");
   for( i = 0; i < size; ++i )
      hops.push_back(new Hop(nodes[i]));
   size = tree->getNodesWithTag(nodes, "MASH_STEP");
   for( i = 0; i < size; ++i )
      mashSteps.push_back(new MashStep(nodes[i]));
   size = tree->getNodesWithTag(nodes, "MISC");
   for( i = 0; i < size; ++i )
      miscs.push_back(new Misc(nodes[i]));
   size = tree->getNodesWithTag(nodes, "STYLE");
   for( i = 0; i < size; ++i )
      styles.push_back(new Style(nodes[i]));
   size = tree->getNodesWithTag(nodes, "WATER");
   for( i = 0; i < size; ++i )
      waters.push_back(new Water(nodes[i]));
   size = tree->getNodesWithTag(nodes, "YEAST");
   for( i = 0; i < size; ++i )
      yeasts.push_back(new Yeast(nodes[i]));

   delete tree;

   tree = new XmlTree(mashFile);

   size = tree->getNodesWithTag(nodes, "MASH");
   for( i = 0; i < size; ++i )
      mashs.push_back(new Mash(nodes[i]));

   delete tree;

   tree = new XmlTree(recipeFile);

   size = tree->getNodesWithTag(nodes, "RECIPE");
   for( i = 0; i < size; ++i )
      recipes.push_back(new Recipe(nodes[i]));

   delete tree;
   dbFile.close();
   recipeFile.close();
   mashFile.close();

   // Sort everything by name.
   equipments.sort(Equipment_ptr_cmp());
   fermentables.sort(Fermentable_ptr_cmp());
   hops.sort(Hop_ptr_cmp());
   mashs.sort(Mash_ptr_cmp());
   mashSteps.sort(MashStep_ptr_cmp());
   miscs.sort(Misc_ptr_cmp());
   recipes.sort(Recipe_ptr_cmp());
   styles.sort(Style_ptr_cmp());
   waters.sort(Water_ptr_cmp());
   yeasts.sort(Yeast_ptr_cmp());

    */
}

void Database::resortAll()
{
   // Sort everything by name.
   equipments.sort(Equipment_ptr_cmp());
   fermentables.sort(Fermentable_ptr_cmp());
   hops.sort(Hop_ptr_cmp());
   mashs.sort(Mash_ptr_cmp());
   mashSteps.sort(MashStep_ptr_cmp());
   miscs.sort(Misc_ptr_cmp());
   recipes.sort(Recipe_ptr_cmp());
   styles.sort(Style_ptr_cmp());
   waters.sort(Water_ptr_cmp());
   yeasts.sort(Yeast_ptr_cmp());

   hasChanged(QVariant(DBALL));
}

void Database::resortEquipments()
{
   equipments.sort(Equipment_ptr_cmp());
   hasChanged(QVariant(DBEQUIP));
}

void Database::resortFermentables()
{
   fermentables.sort(Fermentable_ptr_cmp());
   hasChanged(QVariant(DBFERM));
}

void Database::resortHops()
{
   hops.sort(Hop_ptr_cmp());
   hasChanged(QVariant(DBHOP));
}

void Database::resortMiscs()
{
   miscs.sort(Misc_ptr_cmp());
   hasChanged(QVariant(DBMISC));
}

void Database::resortStyles()
{
   styles.sort(Style_ptr_cmp());
   hasChanged(QVariant(DBSTYLE));
}

void Database::resortYeasts()
{
   yeasts.sort(Yeast_ptr_cmp());
   hasChanged(QVariant(DBYEAST));
}

void Database::savePersistent()
{
   QDomDocument dbDoc, recDoc, mashDoc;
   QDomElement dbRoot, recRoot, mashRoot;
   
   if( ! dbFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + dbFile.fileName().toStdString() + " for writing.");
      return;
   }
   if( ! recipeFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + recipeFile.fileName().toStdString() + " for writing.");
      return;
   }
   if( ! mashFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) )
   {
      Brewtarget::log(Brewtarget::ERROR, "Could not open " + mashFile.fileName().toStdString() + " for writing.");
      return;
   }

   QTextStream dbOut(&dbFile);
   QTextStream recipeOut(&recipeFile);
   QTextStream mashOut(&mashFile);

   /*** dbDoc ***/
   dbRoot = dbDoc.createElement("DATABASE");
   
   std::list<Equipment*>::iterator eqit, eqend;
   eqend = equipments.end();
   for( eqit = equipments.begin(); eqit != eqend; ++eqit )
      (*eqit)->toXml(dbDoc, dbRoot);
   
   std::list<Fermentable*>::iterator fit, fend;
   fend = fermentables.end();
   for( fit = fermentables.begin(); fit != fend; ++fit )
      (*fit)->toXml(dbDoc, dbRoot);
   
   std::list<Hop*>::iterator hit, hend;
   hend = hops.end();
   for( hit = hops.begin(); hit != hend; ++hit )
      (*hit)->toXml(dbDoc, dbRoot);
   
   std::list<MashStep*>::iterator msit, msend;
   msend = mashSteps.end();
   for( msit = mashSteps.begin(); msit != msend; ++msit )
      (*msit)->toXml(dbDoc, dbRoot);
   
   std::list<Misc*>::iterator miscit, miscend;
   miscend = miscs.end();
   for( miscit = miscs.begin(); miscit != miscend; ++miscit )
      (*miscit)->toXml(dbDoc, dbRoot);
   
   std::list<Style*>::iterator sit, send;
   send = styles.end();
   for( sit = styles.begin(); sit != send; ++sit )
      (*sit)->toXml(dbDoc, dbRoot);
   
   std::list<Water*>::iterator wit, wend;
   wend = waters.end();
   for( wit = waters.begin(); wit != wend; ++wit )
      (*wit)->toXml(dbDoc, dbRoot);
   
   std::list<Yeast*>::iterator yit, yend;
   yend = yeasts.end();
   for( yit = yeasts.begin(); yit != yend; ++yit )
      (*yit)->toXml(dbDoc, dbRoot);
   
   dbDoc.appendChild(dbRoot);
   dbOut << dbDoc.toString();
   /*** END dbDoc ***/
   
   /*** recDoc ***/
   recRoot = recDoc.createElement("RECIPES");
   
   std::list<Recipe*>::iterator rit, rend;
   rend = recipes.end();
   for( rit = recipes.begin(); rit != rend; ++rit )
      (*rit)->toXml(recDoc, recRoot);
   
   recDoc.appendChild(recRoot);
   recipeOut << recDoc.toString();
   /*** END recDoc ***/
   
   /*** mashDoc ***/
   mashRoot = mashDoc.createElement("MASHS");
   
   std::list<Mash*>::iterator mait, maend;
   maend = mashs.end();
   for( mait = mashs.begin(); mait != maend; ++mait )
      (*mait)->toXml(mashDoc, mashRoot);
   
   mashDoc.appendChild(mashRoot);
   mashOut << mashDoc.toString();
   /*** END mashDoc ***/
   
   /*
   dbOut << QString("<?xml version=\"1.0\"?>\n");
   recipeOut << QString("<?xml version=\"1.0\"?>\n");
   mashOut << QString("<?xml version=\"1.0\"?>\n");

   //=====================dbOut entries=============================

   std::list<Equipment*>::iterator eqit, eqend;
   eqend = equipments.end();
   dbOut << QString("<EQUIPMENTS>\n");
   for( eqit = equipments.begin(); eqit != eqend; ++eqit )
      dbOut << QString((*eqit)->toXml().c_str());
   dbOut << QString("</EQUIPMENTS>\n");

   std::list<Fermentable*>::iterator fit, fend;
   fend = fermentables.end();
   dbOut << QString("<FERMENTABLES>\n");
   for( fit = fermentables.begin(); fit != fend; ++fit )
      dbOut << QString((*fit)->toXml().c_str());
   dbOut << QString("</FERMENTABLES>\n");

   std::list<Hop*>::iterator hit, hend;
   hend = hops.end();
   dbOut << QString("<HOPS>\n");
   for( hit = hops.begin(); hit != hend; ++hit )
      dbOut << QString((*hit)->toXml().c_str());
   dbOut << QString("</HOPS>\n");

   std::list<MashStep*>::iterator msit, msend;
   msend = mashSteps.end();
   dbOut << QString("<MASH_STEPS>\n");
   for( msit = mashSteps.begin(); msit != msend; ++msit )
      dbOut << QString((*msit)->toXml().c_str());
   dbOut << QString("</MASH_STEPS>\n");

   std::list<Misc*>::iterator miscit, miscend;
   miscend = miscs.end();
   dbOut << QString("<MISCS>\n");
   for( miscit = miscs.begin(); miscit != miscend; ++miscit )
      dbOut << QString((*miscit)->toXml().c_str());
   dbOut << QString("</MISCS>\n");

   std::list<Style*>::iterator sit, send;
   send = styles.end();
   dbOut << QString("<STYLES>\n");
   for( sit = styles.begin(); sit != send; ++sit )
      dbOut << QString((*sit)->toXml().c_str());
   dbOut << QString("</STYLES>\n");

   std::list<Water*>::iterator wit, wend;
   wend = waters.end();
   dbOut << QString("<WATERS>\n");
   for( wit = waters.begin(); wit != wend; ++wit )
      dbOut << QString((*wit)->toXml().c_str());
   dbOut << QString("</WATERS>\n");

   std::list<Yeast*>::iterator yit, yend;
   yend = yeasts.end();
   dbOut << QString("<YEASTS>\n");
   for( yit = yeasts.begin(); yit != yend; ++yit )
      dbOut << QString((*yit)->toXml().c_str());
   dbOut << QString("</YEASTS>\n");

   //============================mashOut entries===============================
   std::list<Mash*>::iterator mait, maend;
   maend = mashs.end();
   mashOut << QString("<MASHS>\n");
   for( mait = mashs.begin(); mait != maend; ++mait )
      mashOut << QString((*mait)->toXml().c_str());
   mashOut << QString("</MASHS>\n");

   //==========================recipeOut entries===============================
   std::list<Recipe*>::iterator rit, rend;
   rend = recipes.end();
   recipeOut << QString("<RECIPES>\n");
   for( rit = recipes.begin(); rit != rend; ++rit )
      recipeOut << QString((*rit)->toXml().c_str());
   recipeOut << QString("</RECIPES>\n");

   */
   
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
      equipments.sort(Equipment_ptr_cmp());
      equipments.unique(Equipment_ptr_equals()); // No dups.
      hasChanged(QVariant(DBEQUIP));
   }
}

void Database::addFermentable(Fermentable* ferm)
{
   if( ferm != 0 )
   {
      fermentables.push_back(ferm);
      fermentables.sort(Fermentable_ptr_cmp());
      fermentables.unique(Fermentable_ptr_equals());
      hasChanged(QVariant(DBFERM));
   }
}

void Database::addHop(Hop* hop)
{
   if( hop != 0 )
   {
      hops.push_back(hop);
      hops.sort(Hop_ptr_cmp());
      hops.unique(Hop_ptr_equals());
      hasChanged(QVariant(DBHOP));
   }
}

void Database::addMash(Mash* mash)
{
   if( mash != 0 )
   {
      mashs.push_back(mash);
      mashs.sort(Mash_ptr_cmp());
      mashs.unique(Mash_ptr_equals());
      hasChanged(QVariant(DBMASH));
   }
}

void Database::addMashStep(MashStep* mashStep)
{
   if( mashStep != 0 )
   {
      mashSteps.push_back(mashStep);
      mashSteps.sort(MashStep_ptr_cmp());
      mashSteps.unique(MashStep_ptr_equals());
      hasChanged(QVariant(DBMASHSTEP));
   }
}

void Database::addMisc(Misc* misc)
{
   if( misc != 0 )
   {
      miscs.push_back(misc);
      miscs.sort(Misc_ptr_cmp());
      miscs.unique(Misc_ptr_equals());
      hasChanged(QVariant(DBMISC));
   }
}

void Database::addRecipe(Recipe* rec, bool copySubelements)
{
   if( rec == 0 )
      return;

   recipes.push_back(rec);
   recipes.sort(Recipe_ptr_cmp());
   recipes.unique(Recipe_ptr_equals());

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
      styles.sort(Style_ptr_cmp());
      styles.unique(Style_ptr_equals());
      hasChanged(QVariant(DBSTYLE));
   }
}

void Database::removeEquipment(Equipment* equip)
{
   equipments.remove(equip);
   hasChanged(QVariant(DBEQUIP));
}

void Database::removeFermentable(Fermentable* ferm)
{
   fermentables.remove(ferm);
   hasChanged(QVariant(DBFERM));
}

void Database::removeHop(Hop* hop)
{
   hops.remove(hop);
   hasChanged(QVariant(DBHOP));
}

void Database::removeMash(Mash* mash)
{
   mashs.remove(mash);
   hasChanged(QVariant(DBMASH));
}

void Database::removeMashStep(MashStep* mashStep)
{
   mashSteps.remove(mashStep);
   hasChanged(QVariant(DBMASHSTEP));
}

void Database::removeMisc(Misc* misc)
{
   miscs.remove(misc);
   hasChanged(QVariant(DBMISC));
}

void Database::removeRecipe(Recipe* rec)
{
   recipes.remove(rec);
   hasChanged(QVariant(DBRECIPE));
}

void Database::removeStyle(Style* style)
{
   styles.remove(style); // Wow, that was easy.
   hasChanged(QVariant(DBSTYLE));
}

void Database::removeWater(Water* water)
{
   waters.remove(water);
   hasChanged(QVariant(DBWATER));
}

void Database::removeYeast(Yeast* yeast)
{
   yeasts.remove(yeast);
   hasChanged(QVariant(DBYEAST));
}

void Database::addWater(Water* water)
{
   if( water != 0 )
   {
      waters.push_back(water);
      waters.sort(Water_ptr_cmp());
      waters.unique(Water_ptr_equals());
      hasChanged(QVariant(DBWATER));
   }
}

void Database::addYeast(Yeast* yeast)
{
   if( yeast != 0 )
   {
      yeasts.push_back(yeast);
      yeasts.sort(Yeast_ptr_cmp());
      yeasts.unique(Yeast_ptr_equals());
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


std::list<Equipment*>::iterator Database::getEquipmentBegin()
{
   return equipments.begin();
}

std::list<Equipment*>::iterator Database::getEquipmentEnd()
{
   return equipments.end();
}

std::list<Fermentable*>::iterator Database::getFermentableBegin()
{
   return fermentables.begin();
}

std::list<Fermentable*>::iterator Database::getFermentableEnd()
{
   return fermentables.end();
}

std::list<Hop*>::iterator Database::getHopBegin()
{
   return hops.begin();
}

std::list<Hop*>::iterator Database::getHopEnd()
{
   return hops.end();
}

std::list<Mash*>::iterator Database::getMashBegin()
{
   return mashs.begin();
}

std::list<Mash*>::iterator Database::getMashEnd()
{
   return mashs.end();
}

std::list<MashStep*>::iterator Database::getMashStepBegin()
{
   return mashSteps.begin();
}

std::list<MashStep*>::iterator Database::getMashStepEnd()
{
   return mashSteps.end();
}

std::list<Misc*>::iterator Database::getMiscBegin()
{
   return miscs.begin();
}

std::list<Misc*>::iterator Database::getMiscEnd()
{
   return miscs.end();
}

std::list<Recipe*>::iterator Database::getRecipeBegin()
{
   return recipes.begin();
}

std::list<Recipe*>::iterator Database::getRecipeEnd()
{
   return recipes.end();
}

std::list<Style*>::iterator Database::getStyleBegin()
{
   return styles.begin();
}

std::list<Style*>::iterator Database::getStyleEnd()
{
   return styles.end();
}

std::list<Water*>::iterator Database::getWaterBegin()
{
   return waters.begin();
}

std::list<Water*>::iterator Database::getWaterEnd()
{
   return waters.end();
}

std::list<Yeast*>::iterator Database::getYeastBegin()
{
   return yeasts.begin();
}

std::list<Yeast*>::iterator Database::getYeastEnd()
{
   return yeasts.end();
}

Equipment* Database::findEquipmentByName(std::string name)
{
   std::list<Equipment*>::iterator it, end;
   end = equipments.end();

   for( it = equipments.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }
   
   return 0;
}

Fermentable* Database::findFermentableByName(std::string name)
{
   std::list<Fermentable*>::iterator it, end;
   end = fermentables.end();

   for( it = fermentables.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Hop* Database::findHopByName(std::string name)
{
   std::list<Hop*>::iterator it, end;
   end = hops.end();

   for( it = hops.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Mash* Database::findMashByName(std::string name)
{
   std::list<Mash*>::iterator it, end;
   end = mashs.end();

   for( it = mashs.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

MashStep* Database::findMashStepByName(std::string name)
{
   std::list<MashStep*>::iterator it, end;
   end = mashSteps.end();

   for( it = mashSteps.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Misc* Database::findMiscByName(std::string name)
{
   std::list<Misc*>::iterator it, end;
   end = miscs.end();

   for( it = miscs.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Recipe* Database::findRecipeByName(std::string name)
{
   std::list<Recipe*>::iterator it, end;
   end = recipes.end();

   for( it = recipes.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Style* Database::findStyleByName(std::string name)
{
   std::list<Style*>::iterator it, end;
   end = styles.end();

   for( it = styles.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Water* Database::findWaterByName(std::string name)
{
   std::list<Water*>::iterator it, end;
   end = waters.end();

   for( it = waters.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}

Yeast* Database::findYeastByName(std::string name)
{
   std::list<Yeast*>::iterator it, end;
   end = yeasts.end();

   for( it = yeasts.begin(); it != end; it++ )
   {
      if( (*it)->getName() == name )
         return *it;
   }

   return 0;
}
