/*
 * database.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _DATABASE_H
#define   _DATABASE_H

#include <QList>
#include <iostream>
#include <QFile>
#include <QString>
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

//! Used in the hasChanged() function. Never use \b DONOTUSE ok?
enum {DONOTUSE, DBEQUIP, DBFERM, DBHOP, DBMASH, DBMASHSTEP, DBMISC, DBRECIPE,
      DBSTYLE, DBWATER, DBYEAST, DBALL};

/*!
 * \class Database
 * This class is a singleton, meaning that there should only ever be one
 * instance of this in the whole damn program.
 *
 * It only calls hasChanged() when a new ingredient or whatever gets added,
 * not when any of them actually changed.
 */

class Database : public Observable
{
public:
   //! Don't EVER use this method to get the database!!!
   Database();
   //! This should be the ONLY way you get an instance!!!
   static Database* getDatabase();
   static void initialize();
   static bool isInitialized();
   //! Save to the persistent medium.
   static void savePersistent();

   static bool backupToDir(QString dir);
   static bool restoreFromDir(QString dirStr);
   
   /*! Adds a new Equipment. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addEquipment(Equipment* equip, bool disableNotify = false);
   /*! Adds a new Fermentable. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addFermentable(Fermentable* ferm, bool disableNotify = false);
   /*! Adds a new Hop. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addHop(Hop* hop, bool disableNotify = false);
   /*! Adds a new Mash. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addMash(Mash* mash, bool disableNotify = false);
   /*! Adds a new MashStep. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addMashStep(MashStep* mashStep, bool disableNotify = false);
   /*! Adds a new Misc. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addMisc(Misc* misc, bool disableNotify = false);
   /*! Adds a new Recipe. If \b copySubelements == \b true,
    *  also inserts the recipe's ingredients into the database.
    */
   void addRecipe(Recipe* rec, bool copySubelements);
   /*! Adds a new Style. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addStyle(Style* style, bool disableNotify = false);
   /*! Adds a new Water. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addWater(Water* water, bool disableNotify = false);
   /*! Adds a new Yeast. If \b disableNotify == \b true,
    *  does not notify any database observers of the change.
    */
   void addYeast(Yeast* yeast, bool disableNotify = false);

   // You can remove one
   void removeEquipment(Equipment* equip);
   void removeFermentable(Fermentable* ferm);
   void removeHop(Hop* hop);
   void removeMash(Mash* mash);
   void removeMashStep(MashStep* mashStep);
   void removeMisc(Misc* misc);
   void removeRecipe(Recipe* rec);
   void removeStyle(Style* style);
   void removeWater(Water* water);
   void removeYeast(Yeast* yeast);

   // Or you can remove lists
   void removeEquipment(QList<Equipment*> equip);
   void removeFermentable(QList<Fermentable*> ferm);
   void removeHop(QList<Hop*> hop);
   void removeMash(QList<Mash*> mash);
   void removeMashStep(QList<MashStep*> mashStep);
   void removeMisc(QList<Misc*> misc);
   void removeRecipe(QList<Recipe*> rec);
   void removeStyle(QList<Style*> style);
   void removeWater(QList<Water*> water);
   void removeYeast(QList<Yeast*> yeast);

   //! Sorts all the lists by their compare methods.
   void resortAll();
   void resortFermentables();
   void resortEquipments();
   void resortHops();
   void resortMiscs();
   void resortStyles();
   void resortYeasts();

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

   // These all return null on failure.
   Equipment* findEquipmentByName(QString name);
   Fermentable* findFermentableByName(QString name);
   Hop* findHopByName(QString name);
   Mash* findMashByName(QString name);
   MashStep* findMashStepByName(QString name);
   Misc* findMiscByName(QString name);
   Recipe* findRecipeByName(QString name);
   Style* findStyleByName(QString name);
   Water* findWaterByName(QString name);
   Yeast* findYeastByName(QString name);

   QList<Equipment*>::iterator getEquipmentBegin();
   QList<Equipment*>::iterator getEquipmentEnd();
   QList<Fermentable*>::iterator getFermentableBegin();
   QList<Fermentable*>::iterator getFermentableEnd();
   QList<Hop*>::iterator getHopBegin();
   QList<Hop*>::iterator getHopEnd();
   QList<Mash*>::iterator getMashBegin();
   QList<Mash*>::iterator getMashEnd();
   QList<MashStep*>::iterator getMashStepBegin();
   QList<MashStep*>::iterator getMashStepEnd();
   QList<Misc*>::iterator getMiscBegin();
   QList<Misc*>::iterator getMiscEnd();
   QList<Recipe*>::iterator getRecipeBegin();
   QList<Recipe*>::iterator getRecipeEnd();
   QList<Style*>::iterator getStyleBegin();
   QList<Style*>::iterator getStyleEnd();
   QList<Water*>::iterator getWaterBegin();
   QList<Water*>::iterator getWaterEnd();
   QList<Yeast*>::iterator getYeastBegin();
   QList<Yeast*>::iterator getYeastEnd();

private:
   static bool initialized;
   static Database* internalDBInstance;
   static QFile dbFile;
   static QString dbFileName;
   static QFile recipeFile; // Why are these separate from the dbFile? To prevent duplicates.
   static QString recipeFileName;
   static QFile mashFile; // Why are these separate from the dbFile? To prevent duplicates.
   static QString mashFileName;

   /*! Merges \b last 's BeerXML elements to \b first.
    *  Neither document should have recipes in them.
    */
   static void mergeBeerXMLDBDocs( QDomDocument& first, const QDomDocument& last );
   /*! Merges \b last 's BeerXML elements to \b first.
    *  For documents that ONLY contain recipes.
    */
   static void mergeBeerXMLRecDocs( QDomDocument& first, const QDomDocument& last );

   // The stuff we care about...
   static QList<Equipment*> equipments;
   static QList<Fermentable*> fermentables;
   static QList<Hop*> hops;
   static QList<Mash*> mashs;
   static QList<MashStep*> mashSteps;
   static QList<Misc*> miscs;
   static QList<Recipe*> recipes;
   static QList<Style*> styles;
   static QList<Water*> waters;
   static QList<Yeast*> yeasts;
};

#endif   /* _DATABASE_H */

