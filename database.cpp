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

#include <vector>
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

// Grrr... stupid C++. Have to define these outside the class AGAIN.
std::vector<Equipment*> Database::equipments;
std::vector<Fermentable*> Database::fermentables;
std::vector<Hop*> Database::hops;
std::vector<Mash*> Database::mashs;
std::vector<MashStep*> Database::mashSteps;
std::vector<Misc*> Database::miscs;
std::vector<Recipe*> Database::recipes;
std::vector<Style*> Database::styles;
std::vector<Water*> Database::waters;
std::vector<Yeast*> Database::yeasts;
bool Database::initialized = false;
Database* Database::internalDBInstance = 0;
std::fstream Database::dbFile;
const char* Database::dbFileName = "database.xml";
std::fstream Database::recipeFile;
const char* Database::recipeFileName = "recipes.xml";
std::fstream Database::mashFile;
const char* Database::mashFileName = "mashs.xml";

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
   dbFile.open(dbFileName);
   recipeFile.open(recipeFileName); // Why are these separate from the dbFile? To prevent duplicates.
   mashFile.open(mashFileName); // Why are these separate from the dbFile? To prevent duplicates.
   
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

   internalDBInstance = new Database();
   Database::initialized = true;
}

void Database::savePersistent()
{
   dbFile.open( dbFileName, ios::out | ios::trunc );
   recipeFile.open( recipeFileName, ios::out | ios::trunc );
   mashFile.open( mashFileName, ios::out | ios::trunc );

   unsigned int i, size;
   dbFile << "<?xml version=\"1.0\"?>" << std::endl;
   recipeFile << "<?xml version=\"1.0\"?>" << std::endl;
   mashFile << "<?xml version=\"1.0\"?>" << std::endl;

   //=====================dbFile entries=============================

   size = equipments.size();
   dbFile << "<EQUIPMENTS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << equipments[i]->toXml();
   dbFile << "</EQUIPMENTS>" << std::endl;

   size = fermentables.size();
   dbFile << "<FERMENTABLES>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << fermentables[i]->toXml();
   dbFile << "</FERMENTABLES>" << std::endl;

   size = hops.size();
   dbFile << "<HOPS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << hops[i]->toXml();
   dbFile << "</HOPS>" << std::endl;

   size = mashSteps.size();
   dbFile << "<MASH_STEPS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << mashSteps[i]->toXml();
   dbFile << "</MASH_STEPS>" << std::endl;

   size = miscs.size();
   dbFile << "<MISCS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << miscs[i]->toXml();
   dbFile << "</MISCS>" << std::endl;

   size = styles.size();
   dbFile << "<STYLES>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << styles[i]->toXml();
   dbFile << "</STYLES>" << std::endl;

   size = waters.size();
   dbFile << "<WATERS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << waters[i]->toXml();
   dbFile << "</WATERS>" << std::endl;

   size = yeasts.size();
   dbFile << "<YEASTS>" << std::endl;
   for( i = 0; i < size; ++i )
      dbFile << yeasts[i]->toXml();
   dbFile << "</YEASTS>" << std::endl;

   //============================mashFile entries===============================
   size = mashs.size();
   mashFile << "<MASHS>" << std::endl;
   for( i = 0; i < size; ++i )
      mashFile << mashs[i]->toXml();
   mashFile << "</MASHS>" << std::endl;

   //==========================recipeFile entries===============================
   size = recipes.size();
   recipeFile << "<RECIPES>" << std::endl;
   for( i = 0; i < size; ++i )
      recipeFile << recipes[i]->toXml();
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
      hasChanged();
   }
}

void Database::addFermentable(Fermentable* ferm)
{
   if( ferm != 0 )
   {
      fermentables.push_back(ferm);
      hasChanged();
   }
}

void Database::addHop(Hop* hop)
{
   if( hop != 0 )
   {
      hops.push_back(hop);
      hasChanged();
   }
}

void Database::addMash(Mash* mash)
{
   if( mash != 0 )
   {
      mashs.push_back(mash);
      hasChanged();
   }
}

void Database::addMashStep(MashStep* mashStep)
{
   if( mashStep != 0 )
   {
      mashSteps.push_back(mashStep);
      hasChanged();
   }
}

void Database::addMisc(Misc* misc)
{
   if( misc != 0 )
   {
      miscs.push_back(misc);
      hasChanged();
   }
}

void Database::addRecipe(Recipe* rec)
{
   if( rec != 0 )
   {
      recipes.push_back(rec);
      hasChanged();
   }
}

void Database::addStyle(Style* style)
{
   if( style != 0 )
   {
      styles.push_back(style);
      hasChanged();
   }
}

void Database::addWater(Water* water)
{
   if( water != 0 )
   {
      waters.push_back(water);
      hasChanged();
   }
}

void Database::addYeast(Yeast* yeast)
{
   if( yeast != 0 )
   {
      yeasts.push_back(yeast);
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


Equipment* Database::getEquipment(unsigned int i)
{
   return equipments[i];
}

Fermentable* Database::getFermentable(unsigned int i)
{
   return fermentables[i];
}

Hop* Database::getHop(unsigned int i)
{
   return hops[i];
}

Mash* Database::getMash(unsigned int i)
{
   return mashs[i];
}

MashStep* Database::getMashStep(unsigned int i)
{
   return mashSteps[i];
}

Misc* Database::getMisc(unsigned int i)
{
   return miscs[i];
}

Recipe* Database::getRecipe(unsigned int i)
{
   return recipes[i];
}

Style* Database::getStyle(unsigned int i)
{
   return styles[i];
}

Water* Database::getWater(unsigned int i)
{
   return waters[i];
}

Yeast* Database::getYeast(unsigned int i)
{
   return yeasts[i];
}
