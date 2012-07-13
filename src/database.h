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
   
   // Hey, kate highlights all these words...
   // FIXME
   // HACK
   // NOTE
   // NOTICE
   // TASK
   // TODO
   // ###
   
   //! Get the contents of the cell specified by table/key/col_name. Mostly for BeerXMLElement.
   inline QVariant get( Brewtarget::DBTable table, int key, const char* col_name ) __attribute__((always_inline))
   {
      QSqlQuery q( QString("SELECT `%1` FROM `%2` WHERE id='%3'")
                   .arg(col_name).arg(tableNames[table]).arg(key),
                   sqlDatabase()
                 );
                   
      if( q.next() )
      {
         QVariant ret(q.value(0));
         q.finish();
         return ret;
      }
      else
      {
         q.finish();
         return QVariant();
      }
      //======================
      /*
      BtSqlQuery q( QString("SELECT `%1` FROM `%2` WHERE `%3`='%4'")
                    .arg(col_name).arg(tableNames[table]).arg(keyNames[table]).arg(key)
                  );
                  
      QList<QSqlRecord> records;
      q.exec(records); // Wait here for the results.
      
      if( records.size() > 0 )
         return records[0].value(0);
      else
         return QVariant();
      */
   }
   
   //! Get a table view.
   QTableView* createView( Brewtarget::DBTable table );
   
   // Named constructors ======================================================
   //! Create new brew note attached to \b parent.
   BrewNote* newBrewNote(Recipe* parent);
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
   BrewNote* newBrewNote(BrewNote* other);
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
   
   //! Retrieve a list of elements with given \b filter.
   QList<BeerXMLElement*> listByFilter( Brewtarget::DBTable table, QString filter = "" );
   
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
   void addToRecipe( Recipe* rec, Hop* hop, bool initialLoad = false );
   void addToRecipe( Recipe* rec, Fermentable* ferm, bool initialLoad = false );
   void addToRecipe( Recipe* rec, Misc* m, bool initialLoad = false );
   void addToRecipe( Recipe* rec, Yeast* y, bool initialLoad = false );
   void addToRecipe( Recipe* rec, Water* w, bool initialLoad = false );
   //! Add a mash, displacing any current mash.
   void addToRecipe( Recipe* rec, Mash* m, bool initialLoad = false );
   //! Add an equipment, displacing any current equipment.
   void addToRecipe( Recipe* rec, Equipment* e, bool initialLoad = false );
   //! Add a style, displacing any current style. Does not add a copy, adds the actual style \b s.
   void addToRecipe( Recipe* rec, Style* s);
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
   //! Return a list of all the steps in a mash.
   QList<MashStep*> mashSteps(Mash const* parent);
   
   // Import from BeerXML =====================================================
   // TODO: make all these private.
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
   
signals:
   void changed(QMetaProperty prop, QVariant value);
   
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
   static QHash<Brewtarget::DBTable,QString> tableNames;
   static QHash<Brewtarget::DBTable,QString> tableNamesHash();
   static QHash<QString,Brewtarget::DBTable> classNameToTable;
   static QHash<QString,Brewtarget::DBTable> classNameToTableHash();
   static QHash<QThread*,QString> threadToDbCon; // Each thread should use a distinct database connection.
   
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

   bool loadWasSuccessful;
   bool loadedFromXml;
   
   // Each thread should have its own connection to QSqlDatabase.
   static QHash< QThread*, QString > _threadToConnection;
   static QMutex _threadToConnectionMutex;
   
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
   
   //QUndoStack commandStack;
   
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
    * \returns the key of the new ingredient.
    */
   template<class T> int addIngredientToRecipe( Recipe* rec,
                                                BeerXMLElement* ing,
                                                QString propName,
                                                QString relTableName,
                                                QString ingKeyName,
                                                bool initialLoad = false,
                                                QHash<int,T*>* keyHash = 0 )
   {
      // TODO: encapsulate this in a QUndoCommand.
      int newKey;
      QSqlRecord r;
      
      if( rec == 0 || ing == 0 )
         return -1;
      
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
         return -1;
      }
      else
         q.finish();
      
      // Create a copy of the ingredient. We don't want to do this on the initial
      // load of the recipe database, because the stuff is already a copy.
      if ( ! initialLoad ) 
      {
         r = copy<T>(ing, false, keyHash);
         newKey = r.value("id").toInt();
      }
      else 
      {
         newKey = ing->_key;
      }
      
      // Any ingredient added to a recipe should not be visible for the trees?
      ing->setDisplay(false);
      // Put this (ing,rec) pair in the <ing_type>_in_recipe table.
      q = QSqlQuery( sqlDatabase() );//sqldb );
      q.setForwardOnly(true);
      
      q.prepare( QString("INSERT INTO `%1` (`%2`, `recipe_id`) VALUES (:ingredient, :recipe)")
                 .arg(relTableName)
                 .arg(ingKeyName)
               );
      q.bindValue(":ingredient", newKey);
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
      
      return newKey;
   }
   
   // TODO: encapsulate in QUndoCommand
   //! Remove ingredient from a recipe.
   void removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing, QString propName, QString relTableName, QString ingKeyName );
   
   /*!
    * \brief Create a deep copy of the \b object.
    * \em T must be a subclass of \em BeerXMLElement.
    * \returns a record to the new copy. You must manually emit the changed()
    * signal after a copy() call. Also, does not insert things magically into
    * allHop or allInstructions etc. hashes. This just simply duplicates a
    * row in a table, unless you provide \em keyHash.
    * \param object is the thing you want to copy.
    * \param displayed is true if you want the \em displayed column set to true.
    * \param keyHash if nonzero, inserts the new (key,T*) pair into the hash.
    */
   template<class T> QSqlRecord copy( BeerXMLElement const* object, bool displayed = true, QHash<int,T*>* keyHash=0 )
   {
      int newKey;
      int i;
      
      Brewtarget::DBTable t = classNameToTable[object->metaObject()->className()];
      QString tName = tableNames[t];
      
      QSqlQuery q(QString("SELECT * FROM %1 WHERE id = %2")
                  .arg(tName).arg(object->_key),
                  sqlDatabase()
                 );
      
      if( !q.next() )
      {
         q.finish();
         return QSqlRecord();
      }
      
      QSqlRecord oldRecord = q.record();
      q.finish(); // NOTE: Is this safe, since we will later use oldRecord?
      
      // Create a new row.
      newKey = insertNewDefaultRecord(t);
      q = QSqlQuery( QString("SELECT * FROM %1 WHERE id = %2")
                     .arg(tName).arg(newKey),
                     sqlDatabase()
                   );
      q.next();
      QSqlRecord newRecord = q.record();
      q.finish(); // NOTE: Is this safe, since we will later use newRecord?
      
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
         T* newOne = new T();
         BeerXMLElement* newOneCast = qobject_cast<BeerXMLElement*>(newOne);
         newOneCast->_key = newKey;
         newOneCast->_table = t;
         keyHash->insert( newKey, newOne );
      }
      
      return newRecord;
   }
   
   //! Do an sql update.
   void sqlUpdate( Brewtarget::DBTable table, QString const& setClause, QString const& whereClause );
   
   //! Do an sql delete.
   void sqlDelete( Brewtarget::DBTable table, QString const& whereClause );
   
   int getQualifiedHopTypeIndex(QString type, Hop* hop);
   int getQualifiedMiscTypeIndex(QString type, Misc* misc);
   int getQualifiedMiscUseIndex(QString use, Misc* misc);
   int getQualifiedHopUseIndex(QString use, Hop* hop);

   //! Should be called when we are about to close down.
   void unload();

   // Cleans up the backup database if it was leftover from an error.
   bool cleanupBackupDatabase();
};

#endif   /* _DATABASE_H */

