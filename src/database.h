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

class Database;

#include <QList>
#include <QHash>
#include <iostream>
#include <QFile>
#include <QString>
#include <QSqlDatabase>
#include <QSqlRelationalTableModel>
#include <QSqlRecord>
#include <QMetaProperty>
#include <QUndoStack>
#include <QObject>
#include <QPair>
#include <QTableView>
/* Replace with forward declarations
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
*/
class BeerXMLElement;
class Equipment;
class Fermentable;
class Hop;
class Instruction;
class Mash;
class MashStep;
class Misc;
class Recipe;
class Style;
class Water;
class Yeast;

/*!
 * \class Database
 * This class is a singleton, meaning that there should only ever be one
 * instance of this floating around. The Database should be the only way
 * we ever get pointers to BeerXML ingredients and the like.
 */
class Database : public QObject
{
   Q_OBJECT
   
public:
   enum DBTable{ NOTABLE, BREWNOTETABLE, EQUIPTABLE, FERMTABLE, HOPTABLE, INSTRUCTIONTABLE,
                 MASHSTEPTABLE, MASHTABLE, MISCTABLE, RECTABLE, STYLETABLE, WATERTABLE, YEASTTABLE  };

   //! This should be the ONLY way you get an instance.
   static Database& instance(); // DONE

   static bool backupToDir(QString dir); // DONE
   static bool restoreFromDir(QString dirStr); // DONE

   /*! Schedule an update of the entry, and call the notification when complete.
    */
   void updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object ); // DONE
   
   // Hey, kate highlights all these words...
   // FIXME
   // HACK
   // NOTE
   // NOTICE
   // TASK
   // TODO
   // ###
   
   // NOTICE: Necessary?
   // //! Get the contents of the cell specified by table/key/col_name.
   // QVariant get( DBTable table, int key, const char* col_name );
      
   //! Get a table view.
   QTableView* createView( DBTable table );
   
   // Create a new ingredient.
   Equipment* newEquipment(); // DONE
   Fermentable* newFermentable(); // DONE
   Hop* newHop(); // DONE
   //! Create new instruction attached to \b rec.
   Instruction* newInstruction(Recipe* rec); // DONE
   Mash* newMash(); // DONE
   MashStep* newMashStep(); // DONE
   Misc* newMisc(); // DONE
   Recipe* newRecipe(); // DONE
   Style* newStyle(); // DONE
   Water* newWater(); // DONE
   Yeast* newYeast(); // DONE
   
   //! Retrieve a list of elements with given \b filter.
   QList<BeerXMLElement*> listByFilter( DBTable table, QString filter = "" );
   
   // NOTICE: Necessary?
   // Get ingredients by key value.
   Recipe* recipe(int key);
   /*
   Equipment* equipment(int key);
   Fermentable* fermentable(int key);
   Hop* hop(int key);
   Mash* mash(int key);
   MashStep* mashStep(int key);
   Misc* misc(int key);
   Style* style(int key);
   Water* water(int key);
   Yeast* yeast(int key);
   */
   
   // Add these to a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void addToRecipe( Recipe* rec, Hop* hop ); // DONE
   void addToRecipe( Recipe* rec, Fermentable* ferm ); // DONE
   void addToRecipe( Recipe* rec, Misc* m ); // DONE
   void addToRecipe( Recipe* rec, Yeast* y ); // DONE
   void addToRecipe( Recipe* rec, Water* w ); // DONE
   // NOTE: not possible in this format.
   //void addToRecipe( Recipe* rec, Instruction* ins );
   
   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void removeFromRecipe( Recipe* rec, Hop* hop ); // DONE
   void removeFromRecipe( Recipe* rec, Fermentable* ferm ); // DONE
   void removeFromRecipe( Recipe* rec, Misc* m ); // DONE
   void removeFromRecipe( Recipe* rec, Yeast* y ); // DONE
   void removeFromRecipe( Recipe* rec, Water* w ); // DONE
   void removeFromRecipe( Recipe* rec, Instruction* ins ); // DONE
   
   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QVector
   // of ingredients in rec.
   /*
   void removeFromRecipe( Recipe* rec, QList<Hop*> hop );
   void removeFromRecipe( Recipe* rec, QList<Fermentable*> ferm );
   void removeFromRecipe( Recipe* rec, QList<Misc*> m );
   void removeFromRecipe( Recipe* rec, QList<Yeast*> y );
   void removeFromRecipe( Recipe* rec, QList<Water*> w );
   void removeFromRecipe( Recipe* rec, QList<Instruction*> ins );
   */
   
   // Mark an item as deleted.
   // NOTE: should these also remove all references to the ingredients?
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
   // NOTE: should these also remove all references to the ingredients?
   // TODO: convert from a sequence of single removes to one single operation?
   void removeEquipment(QList<Equipment*> equip); // DONE
   void removeFermentable(QList<Fermentable*> ferm); // DONE
   void removeHop(QList<Hop*> hop); // DONE
   void removeMash(QList<Mash*> mash); // DONE
   void removeMashStep(QList<MashStep*> mashStep); // DONE
   void removeMisc(QList<Misc*> misc); // DONE
   void removeRecipe(QList<Recipe*> rec); // DONE
   void removeStyle(QList<Style*> style); // DONE
   void removeWater(QList<Water*> water); // DONE
   void removeYeast(QList<Yeast*> yeast); // DONE

   // Return a list of elements according to the given filter.
   void getEquipments( QList<Equipment*>&, QString filter="" );
   void getFermentables( QList<Fermentable*>&, QString filter="" );
   void getHops( QList<Hop*>&, QString filter="" );
   void getMashs( QList<Mash*>&, QString filter="" );
   void getMashSteps( QList<MashStep*>&, QString filter="" );
   void getMiscs( QList<Misc*>&, QString filter="" );
   void getRecipes( QList<Recipe*>&, QString filter="" );
   void getStyles( QList<Style*>&, QString filter="" );
   void getWaters( QList<Water*>&, QString filter="" );
   void getYeasts( QList<Yeast*>&, QString filter="" );
   
   Q_PROPERTY( QList<Equipment*> equipments READ equipments /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Fermentable*> fermentables READ fermentables /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Hop*> hops READ hops /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Mash*> mashs READ mashs /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<MashStep*> mashSteps READ mashSteps /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Misc*> miscs READ miscs /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Recipe*> recipes READ recipes /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Style*> styles READ styles /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Water*> waters READ waters /*WRITE*/ NOTIFY changed STORED false );
   Q_PROPERTY( QList<Yeast*> yeasts READ yeasts /*WRITE*/ NOTIFY changed STORED false );
   
   // NOTICE: Necessary?
   /*
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
   */

   // NOTICE: obsolete?.
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
   
   //! Get a const copy of a particular table model. Const so that no editing can take place outside the db.
   const QSqlRelationalTableModel getModel( DBTable table );
   
signals:
   void changed(QMetaProperty prop, QVariant value);
   
private:
   static QFile dbFile;
   static QString dbFileName;
   static QHash<DBTable,QString> tableNames;
   static QHash<DBTable,QString> tableNamesHash(); // DONE
   
   // Keeps all pointers to the elements referenced by key.
   QHash< int, Equipment* > allEquipments;
   QHash< int, Fermentable* > allFermentables;
   QHash< int, Hop* > allHops;
   QHash< int, Instruction* > allInstructions;
   QHash< int, Mash* > allMashs;
   QHash< int, MashStep* > allMashSteps;
   QHash< int, Misc* > allMiscs;
   QHash< int, Recipe* > allRecipes;
   QHash< int, Style* > allStyles;
   QHash< int, Water* > allWaters;
   QHash< int, Yeast* > allYeasts;
   
   // Helper to populate all* hashes. T should be a BeerXMLElement subclass.
   template <class T> void populateElements( QHash<int,T*> hash, QSqlRelationalTableModel& tm, DBTable table )
   {
      int i, size, key;
      BeerXMLElement* e;
      T* et;
      QString filter = tm.filter();
      
      tm.setFilter("");
      tm.select();
      
      size = tm.rowCount();
      for( i = 0; i < size; ++i )
      {
         key = tm.record(i).value(keyName(table)).toInt();
         
         e = new T();
         et = e; // Do this casting from BeerXMLElement* to T* to avoid including BeerXMLElement.h, causing circular inclusion.
         et->key = key;
         et->table = table;
         
         if( ! hash.contains(key) )
            hash.insert(key,e);
      }
      
      tm.setFilter(filter);
      tm.select();
   }
   
   // Helper to populate the list using the given filter.
   template <class T> void getElements( QList<T*>& list, QString filter, QSqlRelationalTableModel& tm, DBTable table, QHash<int,T*> allElements )
   {
      int i, size, key;
      QString oldFilter = tm.filter();
      
      tm.setFilter(filter);
      tm.select();
      
      list.clear();
      size = tm.rowCount();
      for( i = 0; i < size; ++i )
      {
         key = tm.record(i).value(keyName(table));
         list.append( allElements[key] );
      }
      
      tm.setFilter(oldFilter);
      tm.select();
   }
   
   // The connection to the SQLite database.
   QSqlDatabase sqldb;
   // Model for all the tables in the db.
   QSqlRelationalTableModel tableModel;
   // Models set to specific tables in the db.
   QSqlRelationalTableModel equipments_tm;
   QSqlRelationalTableModel fermentables_tm;
   QSqlRelationalTableModel hops_tm;
   QSqlRelationalTableModel instructions_tm;
   QSqlRelationalTableModel mashs_tm;
   QSqlRelationalTableModel mashSteps_tm;
   QSqlRelationalTableModel miscs_tm;
   QSqlRelationalTableModel recipes_tm;
   QSqlRelationalTableModel styles_tm;
   QSqlRelationalTableModel waters_tm;
   QSqlRelationalTableModel yeasts_tm;
   QHash<DBTable,QSqlRelationalTableModel*> tables;
   
   QUndoStack commandStack;
   
   //! Hidden constructor.
   Database(); // DONE
   //! Copy constructor hidden.
   Database(Database const&){} // DONE
   //! Assignment operator hidden.
   Database& operator=(Database const&){ return *this; } // DONE
   //! Destructor hidden.
   ~Database(){} // DONE
   
   //! Load database from file.
   void load(); // DONE
   
   //! Return primary key name of \b table.
   QString keyName( DBTable table );
   
   //! Make a new row in the \b table. Return key of new row.
   int insertNewRecord( DBTable table ); // DONE
   
   //! Mark the \b object in \b table as deleted.
   void deleteRecord( DBTable table, BeerXMLElement* object ); // DONE
   
   // TODO: encapsulate this in a QUndoCommand.
   //! Add \b ing to \b recipe where \b ing's key is \b ingKeyName and the relational table is \b relTableName.
   void addIngredientToRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName ); // DONE
   
   // TODO: encapsulate in QUndoCommand
   //! Remove ingredient from a recipe.
   void removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName ); // DONE
};

#endif   /* _DATABASE_H */

