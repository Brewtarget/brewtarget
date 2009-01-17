/*
 * database.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _DATABASE_H
#define	_DATABASE_H

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
#include "observable.h"

class Database;

/*
 * This class is a singleton, meaning that there should only ever be one
 * instance of this in the whole damn program.
 *
 * It only calls hasChanged() when a new ingredient or whatever gets added,
 * not when any of them actually changed.
 */

class Database : public Observable
{
public:
   Database(); // Don't EVER use this method to get the database!!!
   static Database* getDatabase(); // This should be the ONLY way you get an instance!!!
   static void initialize();
   static bool isInitialized();
   static void savePersistent(); // Save to the persistent medium.

   void addEquipment(Equipment* equip);
   void addFermentable(Fermentable* ferm);
   void addHop(Hop* hop);
   void addMash(Mash* mash);
   void addMashStep(MashStep* mashStep);
   void addMisc(Misc* misc);
   void addRecipe(Recipe* rec);
   void addStyle(Style* style);
   void addWater(Water* water);
   void addYeast(Yeast* yeast);

   unsigned int getNumEquipments();
   unsigned int getNumFermentables();
   unsigned int getNumHops();
   unsigned int getNumMashs();
   unsigned int getNumMashSteps();
   unsigned int getNumMiscs();
   unsigned int getNumRecipes();
   unsigned int getNumStyles();
   unsigned int getNumWaters();
   unsigned int getNumYeasts();

   Equipment* getEquipment(unsigned int i);
   Fermentable* getFermentable(unsigned int i);
   Hop* getHop(unsigned int i);
   Mash* getMash(unsigned int i);
   MashStep* getMashStep(unsigned int i);
   Misc* getMisc(unsigned int i);
   Recipe* getRecipe(unsigned int i);
   Style* getStyle(unsigned int i);
   Water* getWater(unsigned int i);
   Yeast* getYeast(unsigned int i);

private:
   static bool initialized;
   static Database* internalDBInstance;
   static std::fstream dbFile;
   static const char* dbFileName;
   static std::fstream recipeFile; // Why are these separate from the dbFile? To prevent duplicates.
   static const char* recipeFileName;
   static std::fstream mashFile; // Why are these separate from the dbFile? To prevent duplicates.
   static const char* mashFileName;

   // The stuff we care about...
   static std::vector<Equipment*> equipments;
   static std::vector<Fermentable*> fermentables;
   static std::vector<Hop*> hops;
   static std::vector<Mash*> mashs;
   static std::vector<MashStep*> mashSteps;
   static std::vector<Misc*> miscs;
   static std::vector<Recipe*> recipes;
   static std::vector<Style*> styles;
   static std::vector<Water*> waters;
   static std::vector<Yeast*> yeasts;
};

#endif	/* _DATABASE_H */

