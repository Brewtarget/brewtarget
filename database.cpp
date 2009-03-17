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
std::fstream Database::dbFile;
QString Database::dbFileName;
std::fstream Database::recipeFile;
QString Database::recipeFileName;
std::fstream Database::mashFile;
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
   dbFileName = (Brewtarget::getDataDir() + "database.xml");
   recipeFileName = (Brewtarget::getDataDir() + "recipes.xml");
   mashFileName = (Brewtarget::getDataDir() + "mashs.xml");
   
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

   internalDBInstance = new Database();
   Database::initialized = true;
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

   hasChanged();
}

void Database::savePersistent()
{
   dbFile.open( dbFileName.toStdString().c_str(), ios::out | ios::trunc );
   recipeFile.open( recipeFileName.toStdString().c_str(), ios::out | ios::trunc );
   mashFile.open( mashFileName.toStdString().c_str(), ios::out | ios::trunc );

   dbFile << "<?xml version=\"1.0\"?>" << std::endl;
   recipeFile << "<?xml version=\"1.0\"?>" << std::endl;
   mashFile << "<?xml version=\"1.0\"?>" << std::endl;

   //=====================dbFile entries=============================

   std::list<Equipment*>::iterator eqit, eqend;
   eqend = equipments.end();
   dbFile << "<EQUIPMENTS>" << std::endl;
   for( eqit = equipments.begin(); eqit != eqend; ++eqit )
      dbFile << (*eqit)->toXml();
   dbFile << "</EQUIPMENTS>" << std::endl;

   std::list<Fermentable*>::iterator fit, fend;
   fend = fermentables.end();
   dbFile << "<FERMENTABLES>" << std::endl;
   for( fit = fermentables.begin(); fit != fend; ++fit )
      dbFile << (*fit)->toXml();
   dbFile << "</FERMENTABLES>" << std::endl;

   std::list<Hop*>::iterator hit, hend;
   hend = hops.end();
   dbFile << "<HOPS>" << std::endl;
   for( hit = hops.begin(); hit != hend; ++hit )
      dbFile << (*hit)->toXml();
   dbFile << "</HOPS>" << std::endl;

   std::list<MashStep*>::iterator msit, msend;
   msend = mashSteps.end();
   dbFile << "<MASH_STEPS>" << std::endl;
   for( msit = mashSteps.begin(); msit != msend; ++msit )
      dbFile << (*msit)->toXml();
   dbFile << "</MASH_STEPS>" << std::endl;

   std::list<Misc*>::iterator miscit, miscend;
   miscend = miscs.end();
   dbFile << "<MISCS>" << std::endl;
   for( miscit = miscs.begin(); miscit != miscend; ++miscit )
      dbFile << (*miscit)->toXml();
   dbFile << "</MISCS>" << std::endl;

   std::list<Style*>::iterator sit, send;
   send = styles.end();
   dbFile << "<STYLES>" << std::endl;
   for( sit = styles.begin(); sit != send; ++sit )
      dbFile << (*sit)->toXml();
   dbFile << "</STYLES>" << std::endl;

   std::list<Water*>::iterator wit, wend;
   wend = waters.end();
   dbFile << "<WATERS>" << std::endl;
   for( wit = waters.begin(); wit != wend; ++wit )
      dbFile << (*wit)->toXml();
   dbFile << "</WATERS>" << std::endl;

   std::list<Yeast*>::iterator yit, yend;
   yend = yeasts.end();
   dbFile << "<YEASTS>" << std::endl;
   for( yit = yeasts.begin(); yit != yend; ++yit )
      dbFile << (*yit)->toXml();
   dbFile << "</YEASTS>" << std::endl;

   //============================mashFile entries===============================
   std::list<Mash*>::iterator mait, maend;
   maend = mashs.end();
   mashFile << "<MASHS>" << std::endl;
   for( mait = mashs.begin(); mait != maend; ++mait )
      mashFile << (*mait)->toXml();
   mashFile << "</MASHS>" << std::endl;

   //==========================recipeFile entries===============================
   std::list<Recipe*>::iterator rit, rend;
   rend = recipes.end();
   recipeFile << "<RECIPES>" << std::endl;
   for( rit = recipes.begin(); rit != rend; ++rit )
      recipeFile << (*rit)->toXml();
   recipeFile << "</RECIPES>" << std::endl;

   dbFile.close();
   recipeFile.close();
   mashFile.close();
}

//=========================accessor methods=====================================
void Database::addEquipment(Equipment* equip)
{
   if( equip != 0 )
   {
      equipments.push_back(equip);
      equipments.sort(Equipment_ptr_cmp());
      hasChanged();
   }
}

void Database::addFermentable(Fermentable* ferm)
{
   if( ferm != 0 )
   {
      fermentables.push_back(ferm);
      fermentables.sort(Fermentable_ptr_cmp());
      hasChanged();
   }
}

void Database::addHop(Hop* hop)
{
   if( hop != 0 )
   {
      hops.push_back(hop);
      hops.sort(Hop_ptr_cmp());
      hasChanged();
   }
}

void Database::addMash(Mash* mash)
{
   if( mash != 0 )
   {
      mashs.push_back(mash);
      mashs.sort(Mash_ptr_cmp());
      hasChanged();
   }
}

void Database::addMashStep(MashStep* mashStep)
{
   if( mashStep != 0 )
   {
      mashSteps.push_back(mashStep);
      mashSteps.sort(MashStep_ptr_cmp());
      hasChanged();
   }
}

void Database::addMisc(Misc* misc)
{
   if( misc != 0 )
   {
      miscs.push_back(misc);
      miscs.sort(Misc_ptr_cmp());
      hasChanged();
   }
}

void Database::addRecipe(Recipe* rec, bool copySubelements)
{
   if( rec == 0 )
      return;

   recipes.push_back(rec);
   recipes.sort(Recipe_ptr_cmp());

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

   hasChanged();

}

void Database::addStyle(Style* style)
{
   if( style != 0 )
   {
      styles.push_back(style);
      styles.sort(Style_ptr_cmp());
      hasChanged();
   }
}

void Database::addWater(Water* water)
{
   if( water != 0 )
   {
      waters.push_back(water);
      waters.sort(Water_ptr_cmp());
      hasChanged();
   }
}

void Database::addYeast(Yeast* yeast)
{
   if( yeast != 0 )
   {
      yeasts.push_back(yeast);
      yeasts.sort(Yeast_ptr_cmp());
      hasChanged();
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
