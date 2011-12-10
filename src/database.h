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

#include <QDomDocument>
#include <QDomNode>
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
class BrewNote;
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
   static Database& instance();

   static bool backupToDir(QString dir);
   static bool restoreFromDir(QString dirStr);

   /*! Schedule an update of the entry, and call the notification when complete.
    */
   void updateEntry( DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify = true );
   
   // Hey, kate highlights all these words...
   // FIXME
   // HACK
   // NOTE
   // NOTICE
   // TASK
   // TODO
   // ###
   
   //! Get the contents of the cell specified by table/key/col_name. Mostly for BeerXMLElement.
   QVariant get( DBTable table, int key, const char* col_name );
      
   //! Get a table view.
   QTableView* createView( DBTable table );
   
   // Named constructors for new BeerXML stuff.
   //! Create new brew note attached to \b parent.
   BrewNote* newBrewNote(Recipe* parent); // TODO: implement.
   Equipment* newEquipment();
   Fermentable* newFermentable();
   Hop* newHop();
   //! Create new instruction attached to \b parent.
   Instruction* newInstruction(Recipe* parent);
   Mash* newMash();
   //! Create new mash attached to \b parent.
   Mash* newMash(Recipe* parent); // TODO: implement.
   //! Create new mash step attached to \b parent.
   MashStep* newMashStep(Mash* parent);
   Misc* newMisc();
   Recipe* newRecipe();
   Style* newStyle();
   Water* newWater();
   Yeast* newYeast();
   
   // Named copy constructors.
   //! \returns a copy of the given note.
   BrewNote* newBrewNote(BrewNote* other);
   Equipment* newEquipment(Equipment* other);
   //! \returns a copy of the given recipe.
   Recipe* newRecipe(Recipe* other); // TODO: implement.
   /*! \returns a copy of the given mash. Displaces the mash currently in the
    * parent recipe unless \b displace is false.
    */
   Mash* newMash(Mash* other, bool displace = true); // TODO: implement.
   Fermentable* newFermentable(Fermentable* other); // TODO: implement.
   Hop* newHop(Hop* other); // TODO: implement.
   Misc* newMisc(Misc* other); // TODO: implement.
   Yeast* newYeast(Yeast* other); // TODO: implement.
   
   //! Import ingredients from BeerXML documents.
   void importFromXML(const QString& filename);
   
   //! Retrieve a list of elements with given \b filter.
   QList<BeerXMLElement*> listByFilter( DBTable table, QString filter = "" );
   
   // NOTICE: Necessary?
   //! Get recipe by key value.
   Recipe* recipe(int key);
   Equipment* equipment(int key);
   Mash* mash(int key);
   Style* style(int key);
   /*
   Fermentable* fermentable(int key);
   Hop* hop(int key);
   MashStep* mashStep(int key);
   Misc* misc(int key);
   Water* water(int key);
   Yeast* yeast(int key);
   */
   
   // Add a COPY of these ingredients to a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void addToRecipe( Recipe* rec, Hop* hop );
   void addToRecipe( Recipe* rec, Fermentable* ferm );
   void addToRecipe( Recipe* rec, Misc* m );
   void addToRecipe( Recipe* rec, Yeast* y );
   void addToRecipe( Recipe* rec, Water* w );
   void addToRecipe( Recipe* rec, Mash* m ); // TODO: implement
   void addToRecipe( Recipe* rec, Equipment* e ); // TODO: implement
   void addToRecipe( Recipe* rec, Style* s ); // TODO: implement
   // NOTE: not possible in this format.
   //void addToRecipe( Recipe* rec, Instruction* ins );
   
   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void removeFromRecipe( Recipe* rec, Hop* hop );
   void removeFromRecipe( Recipe* rec, Fermentable* ferm );
   void removeFromRecipe( Recipe* rec, Misc* m );
   void removeFromRecipe( Recipe* rec, Yeast* y );
   void removeFromRecipe( Recipe* rec, Water* w );
   void removeFromRecipe( Recipe* rec, Instruction* ins );
   void removeFromRecipe( Recipe* rec, BrewNote* b ); // TODO: implement
   
   //! Remove \b step from \b mash.
   void removeFrom( Mash* mash, MashStep* step );
   
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

   // Or you can mark whole lists as deleted.
   // NOTE: should these also remove all references to the ingredients?
   // TODO: convert from a sequence of single removes to one single operation?
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

   // Return a list of elements according to the given filter.
   void getBrewNotes( QList<BrewNote*>& list, QString filter="" );
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
   
   //! Get the recipe that this \b note is part of.
   Recipe* getParentRecipe( BrewNote const* note );
   
   //! Interchange the step orders of the two steps. Must be in same mash.
   void swapMashStepOrder(MashStep* m1, MashStep* m2);
   //! Interchange the instruction orders. Must be in same recipe.
   void swapInstructionOrder(Instruction* in1, Instruction* in2);
   //! Insert an instruction (already in a recipe) into position \b pos.
   void insertInstruction(Instruction* in, int pos); // TODO: implement.
   
   Q_PROPERTY( QList<BrewNote*> brewNotes READ brewNotes /*WRITE*/ NOTIFY changed STORED false );
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
   
   QList<BrewNote*>& brewNotes();
   QList<Equipment*>& equipments();
   QList<Fermentable*>& fermentables();
   QList<Hop*>& hops();
   QList<Mash*>& mashs();
   QList<MashStep*>& mashSteps();
   QList<Misc*>& miscs();
   QList<Recipe*>& recipes();
   QList<Style*>& styles();
   QList<Water*>& waters();
   QList<Yeast*>& yeasts();
   
   //! \b returns a list of the brew notes in a recipe.
   QList<BrewNote*> brewNotes(Recipe const* parent);
   //! Return a list of all the fermentables in a recipe.
   QList<Fermentable*> fermentables(Recipe const* parent);
   //! Return a list of all the hops in a recipe.
   QList<Hop*> hops( Recipe const* parent );
   //! Return a list of all the instructions in a recipe.
   QList<Instruction*> instructions( Recipe const* parent );
   //! Return a list of all the miscs in a recipe.
   QList<Misc*> miscs( Recipe const* parent );
   //! Return a list of all the waters in a recipe.
   QList<Water*> waters( Recipe const* parent );
   //! Return a list of all the yeasts in a recipe.
   QList<Yeast*> yeasts( Recipe const* parent );
   //! Return a list of all the steps in a mash.
   QList<MashStep*> mashSteps(Mash const* parent);
   
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
   static QString getDbFileName();
   
   //! Get a const copy of a particular table model. Const so that no editing can take place outside the db.
   const QSqlRelationalTableModel* getModel( DBTable table );
   
signals:
   void changed(QMetaProperty prop, QVariant value);
   
private:
   static QFile dbFile;
   static QString dbFileName;
   static QFile dataDbFile;
   static QString dataDbFileName;
   static QHash<DBTable,QString> tableNames;
   static QHash<DBTable,QString> tableNamesHash();
   static QHash<QString,DBTable> classNameToTable;
   static QHash<QString,DBTable> classNameToTableHash();
   
   // Keeps all pointers to the elements referenced by key.
   QHash< int, BrewNote* > allBrewNotes;
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
   template <class T> void populateElements( QHash<int,T*> hash, QSqlRelationalTableModel* tm, DBTable table )
   {
      int i, size, key;
      BeerXMLElement* e;
      T* et;
      QString filter = tm->filter();
      
      tm->setFilter("");
      tm->select();
      
      size = tm->rowCount();
      for( i = 0; i < size; ++i )
      {
         key = tm->record(i).value(keyName(table)).toInt();
         
         e = new T();
         et = qobject_cast<T*>(e); // Do this casting from BeerXMLElement* to T* to avoid including BeerXMLElement.h, causing circular inclusion.
         et->_key = key;
         et->_table = table;
         
         if( ! hash.contains(key) )
            hash.insert(key,et);
      }
      
      tm->setFilter(filter);
      tm->select();
   }
   
   // Helper to populate the list using the given filter.
   template <class T> void getElements( QList<T*>& list, QString filter, QSqlRelationalTableModel* tm, DBTable table, QHash<int,T*> allElements )
   {
      int i, size, key;
      QString oldFilter = tm->filter();
      
      tm->setFilter(filter);
      tm->select();
      
      list.clear();
      size = tm->rowCount();
      for( i = 0; i < size; ++i )
      {
         key = tm->record(i).value(keyName(table)).toInt();
         list.append( allElements[key] );
      }
      
      tm->setFilter(oldFilter);
      tm->select();
   }
   
   // The connection to the SQLite database.
   QSqlDatabase sqldb;
   
   // Model for all the tables in the db.
   QSqlRelationalTableModel* tableModel;
   // Models set to specific tables in the db.
   QSqlRelationalTableModel* brewnotes_tm;
   QSqlRelationalTableModel* equipments_tm;
   QSqlRelationalTableModel* fermentables_tm;
   QSqlRelationalTableModel* hops_tm;
   QSqlRelationalTableModel* instructions_tm;
   QSqlRelationalTableModel* mashs_tm;
   QSqlRelationalTableModel* mashSteps_tm;
   QSqlRelationalTableModel* miscs_tm;
   QSqlRelationalTableModel* recipes_tm;
   QSqlRelationalTableModel* styles_tm;
   QSqlRelationalTableModel* waters_tm;
   QSqlRelationalTableModel* yeasts_tm;
   QHash<DBTable,QSqlRelationalTableModel*> tables;
   
   QUndoStack commandStack;
   
   //! Hidden constructor.
   Database();
   //! Copy constructor hidden.
   Database(Database const&){}
   //! Assignment operator hidden.
   Database& operator=(Database const&){ return *this; }
   //! Destructor hidden.
   ~Database(){}
   
   //! Helper to more easily get QMetaProperties.
   QMetaProperty metaProperty(const char* name)
   {
      return metaObject()->property(metaObject()->indexOfProperty(name));
   }
   
   //! Load database from file.
   void load();
   
   //! Return primary key name of \b table.
   QString keyName( DBTable table );
   
   //! Make a new row in the \b table. Return key of new row.
   int insertNewRecord( DBTable table );
   
   //! Mark the \b object in \b table as deleted.
   void deleteRecord( DBTable table, BeerXMLElement* object );
   
   // TODO: encapsulate this in a QUndoCommand.
   /*!
    * Create a \e copy of \b ing and add the copy to \b recipe where \b ing's
    * key is \b ingKeyName and the relational table is \b relTableName.
    * \returns the key of the new ingredient.
    */
   int addIngredientToRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName );
   
   // TODO: encapsulate in QUndoCommand
   //! Remove ingredient from a recipe.
   void removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName );
   
   /*!
    * Create a deep copy of the \b object.
    * \returns a record to the new copy.
    */
   QSqlRecord copy( BeerXMLElement* object );
   
   // Export to BeerXML.
   void toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Equipment* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Hop* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Instruction* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Mash* a, QDomDocument& doc, QDomNode& parent );
   void toXml( MashStep* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Misc* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Recipe* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Style* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Water* a, QDomDocument& doc, QDomNode& parent );
   void toXml( Yeast* a, QDomDocument& doc, QDomNode& parent );
};

#endif   /* _DATABASE_H */

