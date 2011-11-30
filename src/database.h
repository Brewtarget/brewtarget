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
#include <QHash>
#include <iostream>
#include <QFile>
#include <QString>
#include <QSqlDatabase>
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
 * instance of this floating around.
 *
 * It only calls hasChanged() when a new ingredient or whatever gets added,
 * not when any of them actually changed.
 */

class Database
{
public:
   enum DBTable{ NOTABLE, BREWNOTETABLE, EQUIPTABLE, FERMTABLE, HOPTABLE,
   INSTRUCTIONTABLE, MASHSTEPTABLE, MASHTABLE, MISCTABLE, RECTABLE, STYLETABLE, WATERTABLE, YEASTTABLE  };

   //! This should be the ONLY way you get an instance.
   static Database& instance(); // DONE
   // //! Save to the persistent medium.
   //static void savePersistent(); // OBSOLETE

   static bool backupToDir(QString dir); // DONE
   static bool restoreFromDir(QString dirStr); // DONE

   /*! Schedule an update of the entry, and call the notification when complete.
    *  Should create an appropriate QUndoCommand and put it into a list somewhere.
    */
   void updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object ); // DONE
   
   //! Get the contents of the cell specified by table/key/col_name.
   QVariant get( DBTable table, int key, const char* col_name );
   
   //! Get the name of the key column for the given table.
   QString keyName( DBTable table );
   
   //! Get a table view.
   QTableView* createView( DBTable table );
   
   // Create a new ingredient.
   Equipment* newEquipment();
   Fermentable* newFermentable();
   Hop* newHop();
   Mash* newMash();
   MashStep* newMashStep();
   Misc* newMisc(); // DONE
   Recipe* newRecipe();
   Style* newStyle();
   Water* newWater();
   Yeast* newYeast();
   
   // Get ingredients by key value.
   Equipment* equipment(int key);
   Fermentable* fermentable(int key);
   Hop* hop(int key);
   Mash* mash(int key);
   MashStep* mashStep(int key);
   Misc* misc(int key);
   Recipe* recipe(int key);
   Style* style(int key);
   Water* water(int key);
   Yeast* yeast(int key);
   
   // Add these to a recipe, then call the changed()
   // signal corresponding to the appropriate QVector
   // of ingredients in rec.
   void addToRecipe( Recipe* rec, Hop* hop );
   void addToRecipe( Recipe* rec, Fermentable* ferm );
   void addToRecipe( Recipe* rec, Misc* m );
   void addToRecipe( Recipe* rec, Yeast* y );
   void addToRecipe( Recipe* rec, Water* w );
   void addToRecipe( Recipe* rec, Instruction* ins );
   
   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QVector
   // of ingredients in rec.
   void removeFromRecipe( Recipe* rec, Hop* hop );
   void removeFromRecipe( Recipe* rec, Fermentable* ferm );
   void removeFromRecipe( Recipe* rec, Misc* m );
   void removeFromRecipe( Recipe* rec, Yeast* y );
   void removeFromRecipe( Recipe* rec, Water* w );
   void removeFromRecipe( Recipe* rec, Instruction* ins );
   
   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QVector
   // of ingredients in rec.
   void removeFromRecipe( Recipe* rec, QVector<Hop*> hop );
   void removeFromRecipe( Recipe* rec, QVector<Fermentable*> ferm );
   void removeFromRecipe( Recipe* rec, QVector<Misc*> m );
   void removeFromRecipe( Recipe* rec, QVector<Yeast*> y );
   void removeFromRecipe( Recipe* rec, QVector<Water*> w );
   void removeFromRecipe( Recipe* rec, QVector<Instruction*> ins );
   
   // Mark an item as deleted.
   void removeEquipment(Equipment* equip); // DONE
   void removeFermentable(Fermentable* ferm); // DONE
   void removeHop(Hop* hop); // DONE
   void removeMash(Mash* mash); // DONE
   void removeMashStep(MashStep* mashStep); // DONE
   void removeMisc(Misc* misc); // DONE
   void removeRecipe(Recipe* rec); // DONE
   void removeStyle(Style* style); // DONE
   void removeWater(Water* water); // DONE
   void removeYeast(Yeast* yeast); // DONE

   // Or you can mark whole lists as deleted.
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

   // QUESTION: Necessary?
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
   
   //! Get the file where this database was loaded from.
   static QString getDbFileName(); // DONE
   
private:
   static QFile dbFile;
   static QString dbFileName;
   static QHash<DBTable,QString> tableNames;
   static QHash<DBTable,QString> tableNamesHash(); // DONE
   
   // The connection to the SQLite database.
   QSqlDatabase sqldb;
   // Model for all the tables in the db.
   QSqlRelationalTableModel tableModel;
   // Models set to specific tables in the db.
   QSqlRelationalTableModel equipments;
   QSqlRelationalTableModel fermentables;
   QSqlRelationalTableModel hops;
   QSqlRelationalTableModel mashs;
   QSqlRelationalTableModel miscs;
   QSqlRelationalTableModel recipes;
   QSqlRelationalTableModel styles;
   QSqlRelationalTableModel waters;
   QSqlRelationalTableModel yeasts;
   QHash<DBTable,QSqlRelationalTable*> tables;
   
   QUndoStack commandStack;
   
   //! Hidden constructor.
   Database(); // DONE
   //! Copy constructor hidden.
   Database(Database const&){} // DONE
   //! Assignment operator hidden.
   Database& operator=(Database const&){} // DONE
   //! Destructor hidden.
   ~Database(){} // DONE
   
   //! Load database from file.
   void load(); // DONE
   
   //! Make a new row in the \b table. Return key of new row.
   int insertNewRecord( DBTable table ); // DONE
   
   //! Mark the \b object in \b table as deleted.
   void deleteRecord( DBTable table, BeerXMLElement* object ); // DONE
};

#endif   /* _DATABASE_H */

