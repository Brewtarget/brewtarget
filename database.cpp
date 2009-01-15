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

Database::Database()
{
   
}

bool Database::isInitialized()
{
   return initialized;
}

void Database::initialize()
{
   dbFile.open("database.xml");
   recipeFile.open("recipes.xml"); // Why are these separate from the dbFile? To prevent duplicates.
   mashFile.open("mashs.xml"); // Why are these separate from the dbFile? To prevent duplicates.
   
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
   size = tree->getNodesWithTag(nodes, "MASHSTEP");
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

   Database::initialized = true;
}
