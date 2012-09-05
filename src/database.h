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
#define _DATABASE_H

class Database;

#include <QDomDocument>
#include <QDomNode>
#include <QList>
#include <QHash>
#include <QFile>
#include <QString>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QVariant>
#include <QMetaProperty>
#include <QUndoStack>
#include <QObject>
#include <QPair>
#include <QTableView>
#include <QSqlError>
#include <QDebug>
#include <QMap>
#include "BeerXMLElement.h"
#include "brewtarget.h"
#include "recipe.h"
// Forward declarations
class BrewNote;
//class BeerXMLElement;
class Equipment;
class Fermentable;
class Hop;
class Instruction;
class Mash;
class MashStep;
class Misc;
//class Recipe;
class Style;
class Water;
class Yeast;
class QThread;
class SetterCommandStack;

typedef struct{
   QString tableName; // Name of the table.
   QStringList propName; // List of BeerXML column names.
   BeerXMLElement* (Database::*newElement)(void); // Function to make a new ingredient in this table.
} TableParams;

/*!
 * \class Database
 * \author Philip G. Lee
 *
 * \brief Model for lists of all the BeerXMLElement items in the database.
 *
 * This class is a singleton, meaning that there should only ever be one
 * instance of this floating around, and its purpose is to manage all of
 * the BeerXMLElements in the app. The Database should be the only way
 * we ever get pointers to BeerXML ingredients and the like. This is our
 * big model class.
 */
class Database : public QObject
{
   Q_OBJECT

   friend class BtSqlQuery; // This class needs the _thread instance.
   friend class SetterCommand; // Needs sqlDatabase().
public:

   //! This should be the ONLY way you get an instance.
   static Database& instance();
   //! Call this to delete the internal instance.
   static void dropInstance();

   static bool backupToDir(QString dir);
   static bool restoreFromDir(QString dirStr);

   bool loadSuccessful();

   /*! Schedule an update of the entry, and call the notification when complete.
    */
   void updateEntry( Brewtarget::Brewtarget::DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify = true );
   
   //! \brief Get the contents of the cell specified by table/key/col_name.
   QVariant get( Brewtarget::DBTable table, int key, const char* col_name )
   {
      QSqlQuery& q = selectAll[table];
      q.bindValue( ":id", key );
      q.exec();
      if( !q.next() )
      {
         Brewtarget::logE( QString("Database::get(): %1").arg(q.lastError().text()) );
         q.finish();
         return QVariant();
      }
      
      QVariant ret( q.record().value(col_name) );
      q.finish();
      return ret;
   }
   
   //! Get a table view.
   QTableView* createView( Brewtarget::DBTable table );
   
   // Named constructors ======================================================
   //! Create new brew note attached to \b parent.
   BrewNote* newBrewNote(Recipe* parent, bool signal = true);
   Equipment* newEquipment();
   Fermentable* newFermentable();
   Hop* newHop();
   //! Create new instruction attached to \b parent.
   Instruction* newInstruction(Recipe* parent);
   Mash* newMash();
   //! Create new mash attached to \b parent.
   Mash* newMash(Recipe* parent);
   //! Create new mash step attached to \b parent.
   MashStep* newMashStep(Mash* parent);
   Misc* newMisc();
   Recipe* newRecipe();
   Style* newStyle();
   Water* newWater();
   Yeast* newYeast();
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   
   // Named copy constructors==================================================
   //! \returns a copy of the given note.
   BrewNote* newBrewNote(BrewNote* other, bool signal = true);
   Equipment* newEquipment(Equipment* other);
   //! \returns a copy of the given recipe.
   Recipe* newRecipe(Recipe* other);
   /*! \returns a copy of the given mash. Displaces the mash currently in the
    * parent recipe unless \b displace is false.
    */
   Mash* newMash(Mash* other, bool displace = true);
   Fermentable* newFermentable(Fermentable* other);
   Hop* newHop(Hop* other);
   Misc* newMisc(Misc* other);
   Yeast* newYeast(Yeast* other);
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   
   //! Import ingredients from BeerXML documents.
   void importFromXML(const QString& filename);
   
   //! Get recipe by key value.
   Recipe* recipe(int key);
   
   // Add a COPY of these ingredients to a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void addToRecipe( Recipe* rec, Hop* hop, bool noCopy = false );
   void addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy = false );
   void addToRecipe( Recipe* rec, Misc* m, bool noCopy = false );
   void addToRecipe( Recipe* rec, Yeast* y, bool noCopy = false );
   void addToRecipe( Recipe* rec, Water* w, bool noCopy = false );
   //! Add a mash, displacing any current mash.
   void addToRecipe( Recipe* rec, Mash* m, bool noCopy = false );
   //! Add an equipment, displacing any current equipment.
   void addToRecipe( Recipe* rec, Equipment* e, bool noCopy = false );
   //! Add a style, displacing any current style.
   void addToRecipe( Recipe* rec, Style* s, bool noCopy = false );
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
   void removeFromRecipe( Recipe* rec, BrewNote* b );
   
   //! Remove \b step from \b mash.
   void removeFrom( Mash* mash, MashStep* step );
   
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

   //! Get the recipe that this \b note is part of.
   Recipe* getParentRecipe( BrewNote const* note );
   
   //! Interchange the step orders of the two steps. Must be in same mash.
   void swapMashStepOrder(MashStep* m1, MashStep* m2);
   //! Interchange the instruction orders. Must be in same recipe.
   void swapInstructionOrder(Instruction* in1, Instruction* in2);
   //! Insert an instruction (already in a recipe) into position \b pos.
   void insertInstruction(Instruction* in, int pos);
   
   Q_PROPERTY( QList<BrewNote*> brewNotes READ brewNotes /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Equipment*> equipments READ equipments /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Fermentable*> fermentables READ fermentables /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Hop*> hops READ hops /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Mash*> mashs READ mashs /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<MashStep*> mashSteps READ mashSteps /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Misc*> miscs READ miscs /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Recipe*> recipes READ recipes /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Style*> styles READ styles /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Water*> waters READ waters /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Yeast*> yeasts READ yeasts /*WRITE*/ NOTIFY changed STORED false )
   
   // Returns non-deleted BeerXMLElements.
   QList<BrewNote*> brewNotes();
   QList<Equipment*> equipments();
   QList<Fermentable*> fermentables();
   QList<Hop*> hops();
   QList<Mash*> mashs();
   QList<MashStep*> mashSteps();
   QList<Misc*> miscs();
   QList<Recipe*> recipes();
   QList<Style*> styles();
   QList<Water*> waters();
   QList<Yeast*> yeasts();
   
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
   //! Get recipe's equipment.
   Equipment* equipment(Recipe const* parent);
   //! Get the recipe's mash.
   Mash* mash( Recipe const* parent );
   //! Get recipe's style.
   Style* style(Recipe const* parent);
   //! Return a list of all the steps in a mash.
   QList<MashStep*> mashSteps(Mash const* parent);
   
   // Export to BeerXML =======================================================
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
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   
   //! Get the file where this database was loaded from.
   static QString getDbFileName();
   
   /*!
    * Updates the brewtarget-provided ingredients from the given sqlite
    * database file.
    */
   void updateDatabase(QString const& filename);
   
signals:
   void changed(QMetaProperty prop, QVariant value);
   void newEquipmentSignal(Equipment*);
   void deletedEquipmentSignal(Equipment*);
   void newFermentableSignal(Fermentable*);
   void deletedFermentableSignal(Fermentable*);
   void newHopSignal(Hop*);
   void deletedHopSignal(Hop*);
   void newMashSignal(Mash*);
   void deletedMashSignal(Mash*);
   void newMiscSignal(Misc*);
   void deletedMiscSignal(Misc*);
   void newRecipeSignal(Recipe*);
   void deletedRecipeSignal(Recipe*);
   void newStyleSignal(Style*);
   void deletedStyleSignal(Style*);
   void newWaterSignal(Water*);
   void deletedWaterSignal(Water*);
   void newYeastSignal(Yeast*);
   void deletedYeastSignal(Yeast*);
   
private slots:
   //! Load database from file.
   bool load();
   
private:
   static Database* dbInstance; // The singleton object
   //QThread* _thread;
   //SetterCommandStack* _setterCommandStack;
   static QFile dbFile;
   static QString dbFileName;
   static QFile dataDbFile;
   static QString dataDbFileName;
   static QFile dbTempBackupFile;
   static QString dbTempBackupFileName;
   static QString dbConName;
   static QHash<Brewtarget::DBTable,QSqlQuery> selectAllHash();
   static QHash<Brewtarget::DBTable,QString> tableNames;
   static QHash<Brewtarget::DBTable,QString> tableNamesHash();
   static QHash<QString,Brewtarget::DBTable> classNameToTable;
   static QHash<QString,Brewtarget::DBTable> classNameToTableHash();
   static QHash<QThread*,QString> threadToDbCon; // Each thread should use a distinct database connection.
   
   static const QList<TableParams> tableParams;
   
   // Each thread should have its own connection to QSqlDatabase.
   static QHash< QThread*, QString > _threadToConnection;
   static QMutex _threadToConnectionMutex;

   // Instance variables.
   bool loadWasSuccessful;
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
   QHash<Brewtarget::DBTable,QSqlQuery> selectAll;
   
   //! Get the right database connection for the calling thread.
   static QSqlDatabase sqlDatabase();
   
   //! Helper to populate all* hashes. T should be a BeerXMLElement subclass.
   template <class T> void populateElements( QHash<int,T*>& hash, Brewtarget::DBTable table )
   {
      int key;
      BeerXMLElement* e;
      T* et;
      
      QSqlQuery q(sqlDatabase());
      q.setForwardOnly(true);
      QString queryString = QString("SELECT id FROM `%1`").arg(tableNames[table]);
      q.prepare( queryString );
      q.exec();
      
      while( q.next() )
      {
         key = q.record().value("id").toInt();
         
         e = new T();
         et = qobject_cast<T*>(e); // Do this casting from BeerXMLElement* to T* to avoid including BeerXMLElement.h, causing circular inclusion.
         et->_key = key;
         et->_table = table;
         
         if( ! hash.contains(key) )
            hash.insert(key,et);
      }
      
      q.finish();
   }
   
   //! Helper to populate the list using the given filter.
   template <class T> void getElements( QList<T*>& list, QString filter, Brewtarget::DBTable table, QHash<int,T*> allElements )
   {
      int key;
      QSqlQuery q(sqlDatabase());
      q.setForwardOnly(true);
      QString queryString;
      if( !filter.isEmpty() )
         queryString = QString("SELECT id FROM `%1` WHERE %2").arg(tableNames[table]).arg(filter);
      else
         queryString = QString("SELECT id FROM `%1`").arg(tableNames[table]);
      q.prepare( queryString );
      q.exec();
      
      while( q.next() )
      {
         key = q.record().value("id").toInt();
         if( allElements.contains(key) )
            list.append( allElements[key] );
      }
      
      q.finish();
   }
   
   /*! Populates the \b element with properties. This must be a class that
    *  simple properties only (no subelements).
    * \param element is the element you want to populate.
    * \param xmlTagsToProperties is a hash from xml tags to meta property names.
    * \param elementNode is the root node of the element we are reading from.
    */
   void fromXml( BeerXMLElement* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode );
   
   // Import from BeerXML =====================================================
   BrewNote* brewNoteFromXml( QDomNode const& node, Recipe* parent );
   Equipment* equipmentFromXml( QDomNode const& node, Recipe* parent = 0 );
   Fermentable* fermentableFromXml( QDomNode const& node, Recipe* parent = 0 );
   Hop* hopFromXml( QDomNode const& node, Recipe* parent = 0 );
   Instruction* instructionFromXml( QDomNode const& node, Recipe* parent );
   Mash* mashFromXml( QDomNode const& node, Recipe* parent = 0 );
   MashStep* mashStepFromXml( QDomNode const& node, Mash* parent );
   Misc* miscFromXml( QDomNode const& node, Recipe* parent = 0 );
   Recipe* recipeFromXml( QDomNode const& node );
   Style* styleFromXml( QDomNode const& node, Recipe* parent = 0 );
   Water* waterFromXml( QDomNode const& node, Recipe* parent = 0 );
   Yeast* yeastFromXml( QDomNode const& node, Recipe* parent = 0 );
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   
   //! Hidden constructor.
   Database();
   //! Copy constructor hidden.
   Database(Database const&){}
   //! Assignment operator hidden.
   Database& operator=(Database const&){ return *this; }
   //! Destructor hidden.
   ~Database();
   
   //! Helper to more easily get QMetaProperties.
   QMetaProperty metaProperty(const char* name)
   {
      return metaObject()->property(metaObject()->indexOfProperty(name));
   }
   
   /*! Make a new row in the \b table.
    *  \returns key of new row.
    *  Only works if all the fields in the table have default values.
    */
   int insertNewDefaultRecord( Brewtarget::DBTable table );
   
   /*! Insert a new row in \b mashstep, where \b parent is the parent mash.
    */
   int insertNewMashStepRecord( Mash* parent );
   
   //! Mark the \b object in \b table as deleted.
   void deleteRecord( Brewtarget::DBTable table, BeerXMLElement* object );
   
   // TODO: encapsulate this in a QUndoCommand.
   /*!
    * Create a \e copy of \b ing and add the copy to \b recipe where \b ing's
    * key is \b ingKeyName and the relational table is \b relTableName.
    *
    * \param noCopy By default, we create a copy of the ingredient. If true,
    *               add the ingredient directly.
    * \returns the new ingredient.
    */
   template<class T> T* addIngredientToRecipe(
      Recipe* rec,
      BeerXMLElement* ing,
      QString propName,
      QString relTableName,
      QString ingKeyName,
      bool noCopy = false,
      QHash<int,T*>* keyHash = 0
   )
   {
      T* newIng = 0;
      
      if( rec == 0 || ing == 0 )
         return 0;
      
      // Ensure this ingredient is not already in the recipe.
      QSqlQuery q(
                   QString("SELECT recipe_id from `%1` WHERE `%2`='%3' AND recipe_id='%4'")
                   .arg(relTableName).arg(ingKeyName).arg(ing->_key).arg(reinterpret_cast<BeerXMLElement*>(rec)->_key),
                   sqlDatabase()
                 );
      if( q.next() )
      {
         q.finish();
         Brewtarget::logW( "Database::addIngredientToRecipe: Ingredient already exists in recipe." );
         return 0;
      }
      else
         q.finish();
      
      if ( noCopy ) 
      {
         newIng = qobject_cast<T*>(ing);
         // Any ingredient part of a recipe shouldn't be visible. 
         ing->setDisplay(false);
      }
      else
      {
         newIng = copy<T>(ing, false, keyHash);
      }
      
      // Put this (ing,rec) pair in the <ing_type>_in_recipe table.
      q = QSqlQuery( sqlDatabase() );//sqldb );
      q.setForwardOnly(true);
      
      q.prepare( QString("INSERT INTO `%1` (`%2`, `recipe_id`) VALUES (:ingredient, :recipe)")
                 .arg(relTableName)
                 .arg(ingKeyName)
               );
      q.bindValue(":ingredient", newIng->key());
      q.bindValue(":recipe", rec->_key);
      if( q.exec() )
      {
         q.finish();
         emit rec->changed( rec->metaProperty(propName), QVariant() );
      }
      else
      {
         q.finish();
         Brewtarget::logW( QString("Database::addIngredientToRecipe: %1.").arg(q.lastError().text()) );
      }
      
      return newIng;
   }
   
   //! Remove ingredient from a recipe.
   void removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName );
   
   /*!
    * \brief Create a deep copy of the \b object.
    * \em T must be a subclass of \em BeerXMLElement.
    * \returns a pointer to the new copy. You must manually emit the changed()
    * signal after a copy() call. Also, does not insert things magically into
    * allHop or allInstructions etc. hashes. This just simply duplicates a
    * row in a table, unless you provide \em keyHash.
    * \param object is the thing you want to copy.
    * \param displayed is true if you want the \em displayed column set to true.
    * \param keyHash if nonzero, inserts the new (key,T*) pair into the hash.
    */
   template<class T> T* copy( BeerXMLElement const* object, bool displayed = true, QHash<int,T*>* keyHash=0 )
   {
      int newKey;
      int i;
      T* newOne = 0;
      
      Brewtarget::DBTable t = classNameToTable[object->metaObject()->className()];
      QString tName = tableNames[t];
      
      QSqlQuery q(QString("SELECT * FROM %1 WHERE id = %2")
                  .arg(tName).arg(object->_key),
                  sqlDatabase()
                 );
      
      if( !q.next() )
      {
         Brewtarget::logE( QString("Database::copy: %1").arg(q.lastError().text()) );
         q.finish();
         return 0;
      }
      
      QSqlRecord oldRecord = q.record();
      q.finish();
      
      // Create a new row.
      newKey = insertNewDefaultRecord(t);
      q = QSqlQuery( QString("SELECT * FROM %1 WHERE id = %2")
                     .arg(tName).arg(newKey),
                     sqlDatabase()
                   );
      q.next();
      QSqlRecord newRecord = q.record();
      q.finish();
      
      // Set the new row's columns equal to the old one's, except for any "parent"
      // field, which should be set to the oldRecord's key.
      QString newValString;
      for( i = 0; i < oldRecord.count(); ++i )
      {
         if( oldRecord.fieldName(i) == "parent" )
            newValString += QString("`parent` = '%2',").arg(object->_key);      
         else if( oldRecord.fieldName(i) == "display" )
            newValString += QString("`display` = %2,").arg( displayed ? 1 : 0 );
         else if ( oldRecord.fieldName(i) != "id" )
            newValString += QString("`%1` = '%2',").arg(oldRecord.fieldName(i)).arg(oldRecord.value(i).toString());
      }
      
      // Remove last comma.
      newValString.chop(1);
      
      QString updateString = QString("UPDATE `%1` SET %2 WHERE id = '%3'")
                                    .arg(tName)
                                    .arg(newValString)
                                    .arg(newKey);
      q = QSqlQuery( sqlDatabase() );
      q.prepare(updateString);
      q.exec();
      q.finish();
      
      // Update the hash if need be.
      if( keyHash )
      {
         newOne = new T();
         BeerXMLElement* newOneCast = qobject_cast<BeerXMLElement*>(newOne);
         newOneCast->_key = newKey;
         newOneCast->_table = t;
         keyHash->insert( newKey, newOne );
      }
      
      return newOne;
   }
   
   // Do an sql update.
   void sqlUpdate( Brewtarget::DBTable table, QString const& setClause, QString const& whereClause );
   
   // Do an sql delete.
   void sqlDelete( Brewtarget::DBTable table, QString const& whereClause );
   
   int getQualifiedHopTypeIndex(QString type, Hop* hop);
   int getQualifiedMiscTypeIndex(QString type, Misc* misc);
   int getQualifiedMiscUseIndex(QString use, Misc* misc);
   int getQualifiedHopUseIndex(QString use, Hop* hop);

   // Should be called when we are about to close down.
   void unload();

   // Cleans up the backup database if it was leftover from an error.
   bool cleanupBackupDatabase();
   
   static QList<TableParams> makeTableParams();
};

#endif   /* _DATABASE_H */

