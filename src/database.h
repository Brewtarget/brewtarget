/*
 * database.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DATABASE_H
#define _DATABASE_H

class Database;

#include <functional>
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
#include <QRegExp>
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

typedef struct
{
   QString tableName; // Name of the table.
   QStringList propName; // List of BeerXML column names.
   std::function<BeerXMLElement*()> newElement;

   // BeerXMLElement* (Database::*newElement)(int); // Function to make a new ingredient in this table.
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
public:

   //! This should be the ONLY way you get an instance.
   static Database& instance();
   //! Call this to delete the internal instance.
   static void dropInstance();
   //! \brief Should be called when we are about to close down.
   void unload();

   //! \brief Create a blank database in the given file
   static bool createBlank(QString const& filename);

   //! backs up database to 'dir' in chosen directory
   static bool backupToDir(QString dir, QString filename="");

   //! \brief Reverts database to that of chosen file.
   static bool restoreFromFile(QString newDbFileStr);

   static bool verifyDbConnection( Brewtarget::DBTypes testDb, QString const& hostname,
                                   int portnum=5432,
                                   QString const& schema="public",
                                   QString const& database="brewtarget",
                                   QString const& username="brewtarget",
                                   QString const& password="brewtarget");
   bool loadSuccessful();

   /*! update an entry, and call the notification when complete.
    * NOTE: This cannot be simplified without a bit more work. The inventory
    * needs to specify a table other than the one named by the beerXML element
    */
   void updateEntry( Brewtarget::DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify = true, bool transact = false );

   //! \brief Get the contents of the cell specified by table/key/col_name.
   QVariant get( Brewtarget::DBTable table, int key, const char* col_name )
   {
      QSqlQuery q;
      QString index = QString("%1_%2").arg(tableNames[table]).arg(col_name);

      if ( ! selectSome.contains(index) ) {
         QString query = QString("SELECT %1 from %2 WHERE id=:id")
                           .arg(col_name)
                           .arg(tableNames[table]);
         q = QSqlQuery( sqlDatabase() );
         q.prepare(query);
         selectSome.insert(index,q);
      }

      q = selectSome.value(index);
      q.bindValue(":id", key);

      q.exec();
      if( !q.next() )
      {
         Brewtarget::logE( QString("Database::get(): %1 (%2) %3").arg(q.lastQuery()).arg(col_name).arg(q.lastError().text()));
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
   // maybe I should have never learned templates?
   template<class T> T* newIngredient(QHash<int,T*>* all) {
      int key;
      T* tmp = new T();
      // To quote the talking heads, my god what have I done?
      Brewtarget::DBTable table = classNameToTable[ tmp->metaObject()->className() ];
      QString insert = QString("INSERT INTO %1 DEFAULT VALUES").arg(tableNames[table]);

      QSqlQuery q(sqlDatabase());

      q.setForwardOnly(true);

      try {
         if ( ! q.exec(insert) )
            throw QString("could not insert a record into");

         key = q.lastInsertId().toInt();
         q.finish();
      }
      catch (QString e) {
         Brewtarget::logE(QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         throw; // rethrow the error until somebody cares
      }

      tmp->_key = key;
      tmp->_table = table;

      all->insert(tmp->_key,tmp);

      return tmp;
   }

   BrewNote* newBrewNote(Recipe* parent, bool signal = true);
   //! Create new instruction attached to \b parent.
   Instruction* newInstruction(Recipe* parent);

   MashStep* newMashStep(Mash* parent, bool connected = true);

   Mash* newMash(Mash* other = 0, bool displace = true);
   Mash* newMash(Recipe* parent, bool transaction = true);

   Recipe* newRecipe();
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   // Named copy constructors==================================================
   //! \returns a copy of the given note.
   BrewNote* newBrewNote(BrewNote* other, bool signal = true);
   Equipment* newEquipment(Equipment* other = 0);
   Fermentable* newFermentable(Fermentable* other = 0);
   Hop* newHop(Hop* other = 0);
   //! \returns a copy of the given recipe.
   Recipe* newRecipe(Recipe* other);
   /*! \returns a copy of the given mash. Displaces the mash currently in the
    * parent recipe unless \b displace is false.
    */
   Misc* newMisc(Misc* other = 0);
   Style* newStyle(Style* other = 0);
   Water* newWater(Water* other = 0);
   Yeast* newYeast(Yeast* other = 0);

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   /* This links ingredients with the same name.
   * The first displayed ingredient in the database is assumed to be the parent.
   */
   void populateChildTablesByName(Brewtarget::DBTable table);
   // Runs populateChildTablesByName for each
   void populateChildTablesByName();
   //! \returns the key of the parent ingredient
   int getParentID(Brewtarget::DBTable table, int childKey);
   //! \returns the key to the inventory table for a given ingredient
   int getInventoryID(Brewtarget::DBTable table, int key);
   //! \returns the parent table number from the hash
   Brewtarget::DBTable getChildTable(Brewtarget::DBTable table);
   //! \returns the inventory table number from the hash
   Brewtarget::DBTable getInventoryTable(Brewtarget::DBTable table);
   //! Inserts an new inventory row in the appropriate table
   void newInventory(Brewtarget::DBTable invForTable, int invForID);
   //! \returns The entire inventory for a table.
   QMap<int, double> getInventory(const Brewtarget::DBTable table) const;

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   //! \brief Copies all of the mashsteps from \c oldMash to \c newMash
   void duplicateMashSteps(Mash *oldMash, Mash *newMash);
   //! Import ingredients from BeerXML documents.
   bool importFromXML(const QString& filename);

   //! Get anything by key value.
   Recipe* recipe(int key);
   Equipment* equipment(int key);
   Fermentable* fermentable(int key);
   Hop* hop(int key);
   Misc* misc(int key);
   Style* style(int key);
   Yeast* yeast(int key);

   // Add a COPY of these ingredients to a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec. If noCopy is true, then don't copy, and set
   // the ingredient's display parameter to 0 (don't display in lists).
   void addToRecipe( Recipe* rec, Equipment* e, bool noCopy = false, bool transact = true );
   void addToRecipe( Recipe* rec, Hop* hop, bool noCopy = false, bool transact = true);
   void addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy = false, bool transact = true);
   //! Add a mash, displacing any current mash.
   void addToRecipe( Recipe* rec, Mash* m, bool noCopy = false, bool transact = true );
   void addToRecipe( Recipe* rec, Misc* m, bool noCopy = false, bool transact = true);
   //! Add a style, displacing any current style.
   void addToRecipe( Recipe* rec, Style* s, bool noCopy = false, bool transact = true );
   void addToRecipe( Recipe* rec, Water* w, bool noCopy = false, bool transact = true);
   void addToRecipe( Recipe* rec, Yeast* y, bool noCopy = false, bool transact = true);
   // NOTE: not possible in this format.
   //void addToRecipe( Recipe* rec, Instruction* ins );
   //
   //! \brief bulk add to a recipe.
   void addToRecipe(Recipe* rec, QList<Fermentable*> ferms, bool transact = true);
   void addToRecipe(Recipe* rec, QList<Hop*> hops, bool transact = true);
   void addToRecipe(Recipe* rec, QList<Misc*> miscs, bool transact = true);
   void addToRecipe(Recipe* rec, QList<Yeast*> yeasts, bool transact = true);

   // Remove these from a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec.
   void removeIngredientFromRecipe( Recipe* rec, BeerXMLElement* ing );

   // An odd ball I can't resolve quite yet. But I will.
   // This one isn't even needed. remove does it
   void removeFromRecipe( Recipe* rec, Instruction* ins );

   //! Remove \b step from \b mash.
   void removeFrom( Mash* mash, MashStep* step );

   // Or you can mark whole lists as deleted.
   // ONE METHOD TO CALL THEM ALL AND IN DARKNESS BIND THEM!
   template<class T> void remove(QList<T*> list)
   {
      if ( list.empty() )
         return;

      const QMetaObject *meta = list[0]->metaObject();
      Brewtarget::DBTable ingTable = classNameToTable[ meta->className() ];
      QString propName;

      int ndx = meta->indexOfClassInfo("signal");
      if ( ndx != -1 ) {
         propName = meta->classInfo(ndx).value();
      }
      else
         throw QString("%1 cannot find signal property on %2").arg(Q_FUNC_INFO).arg(meta->className());

      foreach( T* dead, list ) {
         deleteRecord(ingTable,dead);
      }

      emit changed( metaProperty(propName.toLatin1().data()), QVariant() );
   }

   template <class T>void remove(T* ing, bool emitSignal = true)
   {
      const QMetaObject *meta = ing->metaObject();
      Brewtarget::DBTable ingTable = classNameToTable[ meta->className() ];
      QString propName;

      if ( ingTable == Brewtarget::BREWNOTETABLE ) {
         emitSignal = false;
      }

      if ( emitSignal ) {
         int ndx = meta->indexOfClassInfo("signal");
         if ( ndx != -1 ) {
            propName = meta->classInfo(ndx).value();
         }
         else {
            throw QString("%1 cannot find signal property on %2").arg(Q_FUNC_INFO).arg(meta->className());
         }
      }

      try {
         deleteRecord(ingTable, ing);
      }
      catch (QString e) {
         throw;
      }

      // Brewnotes are weird and don't emit a metapropery change
      if ( emitSignal )
         emit changed( metaProperty(propName.toLatin1().data()), QVariant() );
      // This was screaming until I needed to emit a freaking signal
      if ( ingTable != Brewtarget::MASHSTEPTABLE )
         emit deletedSignal(ing);
   }

   //! Get the recipe that this \b note is part of.
   Recipe* getParentRecipe( BrewNote const* note );

   //! Interchange the step orders of the two steps. Must be in same mash.
   void swapMashStepOrder(MashStep* m1, MashStep* m2);
   //! Interchange the instruction orders. Must be in same recipe.
   void swapInstructionOrder(Instruction* in1, Instruction* in2);
   //! Insert an instruction (already in a recipe) into position \b pos.
   void insertInstruction(Instruction* in, int pos);
   //! \brief The instruction number of an instruction.
   int instructionNumber(Instruction const* in);

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
   void convertFromXml();

   bool isConverted();

   //! \brief Figures out what databases we are copying to and from, opens what
   //   needs opens and then calls the appropriate workhorse to get it done. 
   void convertDatabase(QString const& Hostname, QString const& DbName,
                        QString const& Username, QString const& Password,
                        int Portnum, Brewtarget::DBTypes newType);
signals:
   void changed(QMetaProperty prop, QVariant value);
   void newEquipmentSignal(Equipment*);
   void newFermentableSignal(Fermentable*);
   void newHopSignal(Hop*);
   void newMashSignal(Mash*);
   void newMiscSignal(Misc*);
   void newRecipeSignal(Recipe*);
   void newStyleSignal(Style*);
   void newWaterSignal(Water*);
   void newYeastSignal(Yeast*);
   // This is still experimental. Or at least mental
   void newBrewNoteSignal(BrewNote*);

   void deletedSignal(Equipment*);
   void deletedSignal(Fermentable*);
   void deletedSignal(Hop*);
   void deletedSignal(Mash*);
   void deletedSignal(Misc*);
   void deletedSignal(Recipe*);
   void deletedSignal(Style*);
   void deletedSignal(Water*);
   void deletedSignal(Yeast*);
   void deletedSignal(BrewNote*);
   void deletedSignal(MashStep*);

   // MashSteps need signals too
   void newMashStepSignal(MashStep*);

private slots:
   //! Load database from file.
   bool load();

private:
   static Database* dbInstance; // The singleton object

   //QThread* _thread;
   // These are for SQLite databases
   static QFile dbFile;
   static QString dbFileName;
   static QFile dataDbFile;
   static QString dataDbFileName;
   static QString dbConName;

   // And these are for Postgres databases -- are these really required? Are
   // the sqlite ones really required?
   static QString dbHostname;
   static int dbPortnum;
   static QString dbName;
   static QString dbSchema;
   static QString dbUsername;
   static QString dbPassword;

//   static QHash<Brewtarget::DBTable,QSqlQuery> selectAllHash();
   static QHash<Brewtarget::DBTable,QString> tableNames;
   static QHash<Brewtarget::DBTable,QString> tableNamesHash();
   static QHash<QString,Brewtarget::DBTable> classNameToTable;
   static QHash<QString,Brewtarget::DBTable> classNameToTableHash();
   static QHash<Brewtarget::DBTable,Brewtarget::DBTable> tableToChildTable;
   static QHash<Brewtarget::DBTable,Brewtarget::DBTable> tableToChildTableHash();
   static QHash<Brewtarget::DBTable,Brewtarget::DBTable> tableToInventoryTable;
   static QHash<Brewtarget::DBTable,Brewtarget::DBTable> tableToInventoryTableHash();
   static QHash<QThread*,QString> threadToDbCon; // Each thread should use a distinct database connection.

   // Each thread should have its own connection to QSqlDatabase.
   static QHash< QThread*, QString > _threadToConnection;
   static QMutex _threadToConnectionMutex;

   // Instance variables.
   bool loadWasSuccessful;
   bool converted;
   bool createFromScratch;
   bool schemaUpdated;

   // Don't know where to put this, so it goes here for right now
   bool loadSQLite();
   bool loadPgSQL();

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
//   QHash<Brewtarget::DBTable,QSqlQuery> selectAll;
   QHash<QString,QSqlQuery> selectSome;

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
      QString queryString = QString("SELECT id FROM %1").arg(tableNames[table]);
      q.prepare( queryString );

      try {
         if ( ! q.exec() )
            throw QString("%1 %2").arg(q.lastQuery()).arg(q.lastError().text());
      }
      catch (QString e) {
         Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         q.finish();
         throw;
      }

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
   template <class T> bool getElements( QList<T*>& list, QString filter, Brewtarget::DBTable table, QHash<int,T*> allElements, QString id=QString("") )
   {
      int key;
      QSqlQuery q(sqlDatabase());
      q.setForwardOnly(true);
      QString queryString;

      if ( id.isEmpty() )
         id = "id";

      if( !filter.isEmpty() )
         queryString = QString("SELECT %1 as id FROM %2 WHERE %3").arg(id).arg(tableNames[table]).arg(filter);
      else
         queryString = QString("SELECT %1 as id FROM %2").arg(id).arg(tableNames[table]);

      try {
         if ( ! q.exec(queryString) )
            throw QString("could not execute query: %2 : %3").arg(queryString).arg(q.lastError().text());
      }
      catch (QString e) {
         Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         q.finish();
         throw;
      }

      while( q.next() )
      {
         key = q.record().value("id").toInt();
         if( allElements.contains(key) )
            list.append( allElements[key] );
      }

      q.finish();
      return true;
   }

   /*! Populates the \b element with properties. This must be a class that
    *  simple properties only (no subelements).
    * \param element is the element you want to populate.
    * \param xmlTagsToProperties is a hash from xml tags to meta property names.
    * \param elementNode is the root node of the element we are reading from.
    */
   void fromXml(BeerXMLElement* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode);

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

   //! Mark the \b object in \b table as deleted.
   void deleteRecord( Brewtarget::DBTable table, BeerXMLElement* object );

   // TODO: encapsulate this in a QUndoCommand.
   // Note -- this has to happen on a transactional boundary. We are touching
   // something like four tables, and just sort of hoping it all works.
   /*!
    * Create a \e copy (by default) of \b ing and add the copy to \b recipe where \b ing's
    * key is \b ingKeyName and the relational table is \b relTableName.
    *
    * \tparam T the type of ingredient. Must inherit BeerXMLElement.
    * \param rec the recipe to add the ingredient to
    * \param ing the ingredient to add to the recipe
    * \param propName the Recipe property that will change when we add \c ing to it
    * \param relTableName the name of the relational table, perhaps "ingredient_in_recipe"
    * \param ingKeyName the name of the key in the ingredient table corresponding to \c ing
    * \param noCopy By default, we create a copy of the ingredient. If true,
    *               add the ingredient directly.
    * \param keyHash if not null, add the new (key, \c ing) pair to it
    * \param doNotDisplay if true (default), calls \c setDisplay(\c false) on the new ingredient
    * \returns the new ingredient.
    */
   template<class T> T* addIngredientToRecipe(
      Recipe* rec,
      BeerXMLElement* ing,
      bool noCopy = false,
      QHash<int,T*>* keyHash = 0,
      bool doNotDisplay = true,
      bool transact = true
   )
   {
      T* newIng = 0;
      QString propName, relTableName, ingKeyName, childTableName;
      const QMetaObject* meta = ing->metaObject();
      int ndx = meta->indexOfClassInfo("signal");

      if( rec == 0 || ing == 0 )
         return 0;

      // TRANSACTION BEGIN, but only if requested. Yeah. Had to go there.
      if ( transact ) {
         sqlDatabase().transaction();
      }
      // Queries have to be created inside transactional boundaries

      QSqlQuery q(sqlDatabase());
      try {
         if ( ndx != -1 ) {
            QString prefix = meta->classInfo( meta->indexOfClassInfo("prefix")).value();
            propName  = meta->classInfo(ndx).value();
            relTableName = QString("%1_in_recipe").arg(prefix);
            ingKeyName = QString("%1_id").arg(prefix);
            childTableName = QString("%1_children").arg(prefix);
         }
         else
            throw QString("could not locate classInfo for signal on %2").arg(meta->className());
         // Ensure this ingredient is not already in the recipe.
         QString select = QString("SELECT recipe_id from %1 WHERE %2=%3 AND recipe_id=%4")
                              .arg(relTableName)
                              .arg(ingKeyName)
                              .arg(ing->_key)
                              .arg(reinterpret_cast<BeerXMLElement*>(rec)->_key);
         if (! q.exec(select) )
            throw QString("Couldn't execute search");

         if( q.next() )
            throw QString("Ingredient already exists in recipe." );

         q.finish();

         if ( noCopy )
         {
            newIng = qobject_cast<T*>(ing);
            // Any ingredient part of a recipe shouldn't be visible, unless otherwise requested.
            // Not sure I like this. It's a long call stack just to end up back
            // here
            ing->setDisplay(! doNotDisplay );
         }
         else
         {
            newIng = copy<T>(ing, false, keyHash);
            if ( newIng == 0 )
               throw QString("error copying ingredient");
         }

         // Put this (ing,rec) pair in the <ing_type>_in_recipe table.
         q.setForwardOnly(true);

         QString insert = QString("INSERT INTO %1 (%2, recipe_id) VALUES (:ingredient, :recipe)")
                  .arg(relTableName)
                  .arg(ingKeyName);

         q.prepare(insert);
         q.bindValue(":ingredient", newIng->key());
         q.bindValue(":recipe", rec->_key);

         if ( ! q.exec() )
            throw QString("%2 : %1.").arg(q.lastQuery()).arg(q.lastError().text());

         emit rec->changed( rec->metaProperty(propName), QVariant() );

         q.finish();

         //Put this in the <ing_type>_children table.
         if( childTableName != "instruction_children" ) {

            /*
             * The parent to link to depends on where the ingredient is copied from:
             * - A fermentable from the fermentable tabel -> the ID of the fermentable.
             * - An ingredient from another recipe -> the ID of the ingredient's parent.
             *
             * This is required for:
             * - Getting the proper link to the inventory table, else the new record
             *   will have no inventory.
             * - When deleting the ingredient from the original recipe no longer fails.
             *   Else if fails due to a foreign key constrain.
             */
            int key = ing->key();
            q.prepare(QString("SELECT parent_id FROM %1 WHERE child_id=%2")
                  .arg(childTableName)
                  .arg(key));
            if (q.exec() && q.next())
            {
               key = q.record().value("parent_id").toInt();
            }
            q.finish();

            insert = QString("INSERT INTO %1 (parent_id, child_id) VALUES (:parent, :child)")
                  .arg(childTableName);

            q.prepare(insert);
            q.bindValue(":parent", key);
            q.bindValue(":child", newIng->key());

            if ( ! q.exec() )
               throw QString("%1 %2.").arg(q.lastQuery()).arg(q.lastError().text());

            emit rec->changed( rec->metaProperty(propName), QVariant() );
         }
      }
      catch (QString e) {
         Brewtarget::logE( QString("%1 %2").arg(QString("Q_FUNC_INFO")).arg(e));
         q.finish();
         if ( transact )
            sqlDatabase().rollback();
         throw;
      }
      q.finish();
      if ( transact )
         sqlDatabase().commit();

      return newIng;
   }

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
      T* newOne = new T();
      QString holder,fields;

      Brewtarget::DBTable t = classNameToTable[object->metaObject()->className()];

      QString tName = tableNames[t];

      QSqlQuery q(sqlDatabase());

      try {
         QString select = QString("SELECT * FROM %1 WHERE id = %2").arg(tName).arg(object->_key);

         if( !q.exec(select) )
            throw QString("%1 %2").arg(q.lastQuery()).arg(q.lastError().text());
         else 
            q.next();

         QSqlRecord oldRecord = q.record();
         q.finish();

         // Get the field names from the oldRecord. But skip ID, because it
         // won't work to copy it

         for (i=0; i< oldRecord.count(); ++i)
         {
            QString name = oldRecord.fieldName(i);
            if ( name != "id" )
            {
               fields += fields.isEmpty() ? name : QString(",%1").arg(name);
               holder += holder.isEmpty() ? QString(":%1").arg(name) : QString(",:%1").arg(name);
            }
         }

         // Create a new row.
         QString prepString = QString("INSERT INTO %1 (%2) VALUES(%3)")
                              .arg(tName)
                              .arg(fields)
                              .arg(holder);

         QSqlQuery insert = QSqlQuery( sqlDatabase() );
         insert.prepare(prepString);

         // Bind, bind like the wind! Or at least like mueslix
         for (i=0; i< oldRecord.count(); ++i)
         {
            QString name = oldRecord.fieldName(i);
            QVariant val = oldRecord.value(i);

            // We need to set the parent correctly.
            if ( name == "parent" )
               insert.bindValue(QString(":%1").arg(name), object->_key);
            // Display is being set by the call, not by what we are copying
            else if ( name == "display" )
               insert.bindValue(":display", displayed ? Brewtarget::dbTrue() : Brewtarget::dbFalse() );
            // Ignore ID again, for the same reasons as before.
            else if ( name != "id" )
               insert.bindValue(QString(":%1").arg(name), val);
         }

         if (! insert.exec() )
            throw QString("could not execute %1 : %2").arg(insert.lastQuery()).arg(insert.lastError().text());

         newKey = insert.lastInsertId().toInt();
      }
      catch (QString e) {
         Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
         q.finish();
         throw;
      }

      q.finish();

      // Update the hash if need be.
      if( keyHash )
      {
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

   QList<TableParams> makeTableParams();

   // Returns true if the schema gets updated, false otherwise.
   // If err != 0, set it to true if an error occurs, false otherwise.
   bool updateSchema(bool* err = 0);

   // May St. Stevens intercede on my behalf.
   //
   //! \brief opens an SQLite db for transfer
   QSqlDatabase openSQLite();

   //! \brief opens a PostgreSQL db for transfer. I need
   QSqlDatabase openPostgres(QString const& Hostname, QString const& DbName,
                             QString const& Username, QString const& Password,
                             int Portnum);
   //! \brief makes a query string we can prepare. Needed this in two places,
   // so it got a method
   QString makeInsertString( QSqlRecord here, QString realName );
   QString makeUpdateString( QSqlRecord here, QString realName, int key );

   //! \brief converts sqlite values (mostly booleans) into something postgres
   // wants
   QVariant convertValue(Brewtarget::DBTypes newType, QSqlField field);

   QStringList allTablesInOrder(QSqlQuery q);

   //! \brief does the heavy lifting to copy the contents from one db to the
   //next
   void copyDatabase( Brewtarget::DBTypes oldType, Brewtarget::DBTypes newType, QSqlDatabase oldDb);
   void automaticBackup();

};

#endif   /* _DATABASE_H */

