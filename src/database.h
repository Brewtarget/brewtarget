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
#include <QSqlRelationalTableModel>
#include <QMetaProperty>
#include <QUndoStack>
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

class Database;

/*!
 * \class Database
 * This class is a singleton, meaning that there should only ever be one
 * instance of this in the whole damn program.
 *
 * It only calls hasChanged() when a new ingredient or whatever gets added,
 * not when any of them actually changed.
 */

class Database
{
public:
   enum DBTable{ NOTABLE, EQUIPTABLE, FERMTABLE, HOPTABLE, MASHTABLE, MISCTABLE,
                 RECTABLE, STYLETABLE, WATERTABLE, YEASTTABLE };

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

   /*! Schedule an update of the entry, and call the notification when complete.
    *  Should create an appropriate QUndoCommand and put it into a list somewhere.
    */
   void updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object );
   
   //! Get the contents of the cell specified by table/key/col_name.
   QVariant get( DBTable table, int key, const char* col_name );
   
   // Create a new ingredient.
   Equipment* newEquipment();
   Fermentable* newFermentable();
   Hop* newHop();
   Mash* newMash();
   MashStep* newMashStep();
   Misc* newMisc();
   Recipe* newRecipe();
   Style* newStyle();
   Water* newWater();
   Yeast* newYeast();
   
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

   // QUESTION: obsolete?.
   /*
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
   */

   // QUESTION: obsolete?
   /*
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
   */

   /*! Merges \b last 's BeerXML elements to \b first.
   *  Neither document should have recipes in them. If
   *  \b undup is true, removes duplicate entries preferring to remove
   *  items from \b last first.
   */
   static void mergeBeerXMLDBDocs( QDomDocument& first, const QDomDocument& last );
   
   /*! Merges \b last 's BeerXML elements to \b first.
   *  For documents that ONLY contain recipes. If
   *  \b undup is true, removes duplicate entries preferring to remove
   *  items from \b last first.
   */
   static void mergeBeerXMLRecDocs( QDomDocument& first, const QDomDocument& last );
   
   //! Get the file where this database was loaded from.
   static QString getDbFileName();
   
   //! Get the recipe file this database was loaded from.
   static QString getRecipeFileName();
   
private:
   static bool initialized;
   static Database* internalDBInstance;
   static QFile dbFile;
   static QString dbFileName;
   static QFile recipeFile; // Why are these separate from the dbFile? To prevent duplicates.
   static QString recipeFileName;
   static QFile mashFile; // Why are these separate from the dbFile? To prevent duplicates.
   static QString mashFileName;
   
   QSqlRelationalTableModel equipments;
   QSqlRelationalTableModel fermentables;
   QSqlRelationalTableModel hops;
   QSqlRelationalTableModel mashs;
   QSqlRelationalTableModel miscs;
   QSqlRelationalTableModel recipes;
   QSqlRelationalTableModel styles;
   QSqlRelationalTableModel waters;
   QSqlRelationalTableModel yeasts;
   
   QUndoStack commandStack;
};

#endif   /* _DATABASE_H */

