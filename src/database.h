/*
 * database.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/NamedEntity.h"
#include "brewtarget.h"
#include "model/Recipe.h"
#include "DatabaseSchema.h"
#include "TableSchema.h"
#include "TableSchemaConst.h"

// Forward declarations
class BeerXML;
class BrewNote;
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

/*!
 * \class Database
 * \author Philip G. Lee
 *
 * \brief Model for lists of all the NamedEntity items in the database.
 *
 * This class is a singleton, meaning that there should only ever be one
 * instance of this floating around, and its purpose is to manage all of
 * the NamedEntitys in the app. The Database should be the only way
 * we ever get pointers to BeerXML ingredients and the like. This is our
 * big model class.
 *
 * .:TBD:. (MY 2020-01-03) The trouble with having such a broad purpose to this class is that it ends up being enormous
 * and very complicated.  It would be better IMHO to separate things out to:
 *  - one or more registries of NamedEntity derivatives (Hops, Fermentables, Recipes etc)
 *  - a set of mappings and classes that know how to store and retrieve each of these things in the DB
 * Because we load everything into memory, searching for a Yeast etc doesn't require us to access the DB.  We just ask
 * the relevant registry "give me Yeast X".  If we then create a new Yeast (either via the UI or by reading it in from
 * a BeerXML file) we can then ask for it to be saved in the database.
 */
class Database : public QObject
{
   Q_OBJECT

   friend class BeerXML;

public:

   //! This should be the ONLY way you get an instance.
   static Database& instance();
   //! Call this to delete the internal instance.
   static void dropInstance();
   //! \brief Should be called when we are about to close down.
   void unload();

   //! \brief Create a blank database in the given file
   static bool createBlank(QString const& filename);

   static char const * getDefaultBackupFileName();

   //! backs up database to chosen file
   static bool backupToFile(QString newDbFileName);

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

   int numberOfRecipes() const;

   // boolean return, because upstream needs to make some choices
   bool modifyEntry( NamedEntity* object, QString propName, QVariant value, bool notify = true );
   void updateEntry( NamedEntity* object, QString propName, QVariant value, bool notify = true, bool transact = false );

   //! \brief Get the contents of the cell specified by table/key/col_name
   QVariant get( Brewtarget::DBTable table, int key, QString col_name );

   QVariant get( TableSchema* tbl, int key, QString col_name );

   //! Get a table view.
   QTableView* createView( Brewtarget::DBTable table );

   // Named constructors ======================================================
   // maybe I should have never learned templates?
   // We only really need one of these, and the ones like (name, &all)
   // duplicated work. So I whacked those and made one of these.
   template<class T> T* newNamedEntity(QHash<int,T*>* all) {
      int key;
      // To quote the talking heads, my god what have I done?
      Brewtarget::DBTable table = dbDefn->classNameToTable( T::classNameStr() );
      TableSchema* tbl = dbDefn->table(table);
      QSqlRecord rec;

      QString insert = QString("INSERT INTO %1 DEFAULT VALUES").arg(tbl->tableName());

      QSqlQuery q(sqlDatabase());

      // q.setForwardOnly(true);

      try {
         if ( ! q.exec(insert) )
            throw QString("could not insert a record into");

         key = q.lastInsertId().toInt();

         // this allows me to simplify things later.
         QString select = QString("SELECT * FROM %1 WHERE %2 = %3")
                              .arg(tbl->tableName())
                              .arg(tbl->keyName())
                              .arg(key);
         qDebug() << Q_FUNC_INFO << "SELECT SQL: " << select;
         if ( ! q.exec(select) ) {
            throw QString("%1 %2").arg(q.lastQuery()).arg(q.lastError().text());
         }
         q.next();
         rec = q.record();
         q.finish();
      }
      catch (QString e) {
         qCritical() << Q_FUNC_INFO << e << q.lastError().text();
         throw; // rethrow the error until somebody cares
      }

      // this is weird, but I want the sqlrecord
      T* tmp = new T(tbl, rec);
      all->insert(tmp->key(),tmp);

      return tmp;
   }

   BrewNote* newBrewNote(Recipe* parent, bool signal = true);
   //! Create new instruction attached to \b parent.
   Instruction* newInstruction(Recipe* parent);

   MashStep* newMashStep(Mash* parent, bool connected = true);

   Mash* newMash(Mash* other = nullptr, bool displace = true);
   Mash* newMash(Recipe* parent, bool transaction = true);

   Recipe* newRecipe(QString name);
   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   // Named copy constructors==================================================
   //! \returns a copy of the given note.
   BrewNote* newBrewNote(BrewNote* other, bool signal = true);
   Equipment* newEquipment(Equipment* other = nullptr);
   Fermentable* newFermentable(Fermentable* other = nullptr, bool add_inventory = false);
   Hop* newHop(Hop* other = nullptr, bool add_inventory = false);
   //! \returns a copy of the given recipe.
   Recipe* newRecipe(Recipe* other, bool ancestor = false);
   /*! \returns a copy of the given mash. Displaces the mash currently in the
    * parent recipe unless \b displace is false.
    */
   Misc* newMisc(Misc* other = nullptr, bool add_inventory = false);

   Style* newStyle(Style* other);
   Style* newStyle(QString name);
   Water* newWater(Water* other = nullptr);
   Salt* newSalt(Salt* other = nullptr);
   Yeast* newYeast(Yeast* other = nullptr, bool add_inventory = false);

   int    insertElement(NamedEntity* ins);
   int    insertEquipment(Equipment* ins);
   int    insertFermentable(Fermentable* ins);
   int    insertHop(Hop* ins);
   int    insertMash(Mash* ins);
   int    insertMisc(Misc* ins);
   int    insertRecipe(Recipe* ins);
   int    insertStyle(Style* ins);
   int    insertYeast(Yeast* ins);
   int    insertWater(Water* ins);
   int    insertSalt(Salt* ins);

   // Brewnotes, instructions and mashsteps are impossible without their parent objects
   int    insertBrewNote(BrewNote* ins, Recipe *parent);
   int    insertInstruction(Instruction* ins, Recipe *parent);
   int    insertMashStep(MashStep* ins, Mash *parent);

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   /* This links ingredients with the same name.
   * The first displayed ingredient in the database is assumed to be the parent.
   */
   void populateChildTablesByName(Brewtarget::DBTable table);

   // Runs populateChildTablesByName for each
   void populateChildTablesByName();

   //! \returns the key of the parent ingredient
   int getParentID(TableSchema* table, int childKey);

   //! \returns true if this ingredient is stored in the DB, false otherwise
   bool isStored(NamedEntity const & ingredient);

   //! Inserts an new inventory row in the appropriate table
   int newInventory(TableSchema* schema);

   int getInventoryId(TableSchema* tbl, int key );
   void setInventory(NamedEntity* ins, QVariant value, int invKey = 0, bool notify=true );

   //! \returns The entire inventory for a table.
   QMap<int, double> getInventory(const Brewtarget::DBTable table) const;

   QVariant getInventoryAmt(Brewtarget::DBTable table, int key);

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   //! \brief Copies all of the mashsteps from \c oldMash to \c newMash
   void duplicateMashSteps(Mash *oldMash, Mash *newMash);

   //! Get anything by key value.
   Recipe* recipe(int key);
   Equipment* equipment(int key);
   Fermentable* fermentable(int key);
   Hop* hop(int key);
   Misc* misc(int key);
   Style* style(int key);
   Yeast* yeast(int key);
   Salt* salt(int key);
   Water* water(int key);

   // Add a COPY of these ingredients to a recipe, then call the changed()
   // signal corresponding to the appropriate QList
   // of ingredients in rec. If noCopy is true, then don't copy, and set
   // the ingredient's display parameter to 0 (don't display in lists).
   Equipment* addToRecipe( Recipe* rec, Equipment* e, bool noCopy = false, bool transact = true );
   Hop * addToRecipe( Recipe* rec, Hop* hop, bool noCopy = false, bool transact = true);
   Fermentable * addToRecipe( Recipe* rec, Fermentable* ferm, bool noCopy = false, bool transact = true);
   //! Add a mash, displacing any current mash.
   Mash * addToRecipe( Recipe* rec, Mash* m, bool noCopy = false, bool transact = true );
   //! a no-op to make later code prettier
   MashStep* addToRecipe( Recipe* rec, MashStep* m) { return m; }
   Misc * addToRecipe( Recipe* rec, Misc* m, bool noCopy = false, bool transact = true);
   //! Add a style, displacing any current style.
   Style * addToRecipe( Recipe* rec, Style* s, bool noCopy = false, bool transact = true );
   Water * addToRecipe( Recipe* rec, Water* w, bool noCopy = false, bool transact = true);
   Salt * addToRecipe( Recipe* rec, Salt* s,  bool noCopy = false, bool transact = true);
   Yeast * addToRecipe( Recipe* rec, Yeast* y, bool noCopy = false, bool transact = true);
   // NOTE: not possible in this format.
   //void addToRecipe( Recipe* rec, Instruction* ins );
   //
   //! \brief bulk add to a recipe.
   QList<Fermentable*> addToRecipe(Recipe* rec, QList<Fermentable*> ferms, bool transact = true);
   QList<Hop*> addToRecipe(Recipe* rec, QList<Hop*> hops, bool transact = true);
   QList<Misc*> addToRecipe(Recipe* rec, QList<Misc*> miscs, bool transact = true);
   QList<Yeast*> addToRecipe(Recipe* rec, QList<Yeast*> yeasts, bool transact = true);

   //! \brief bulk add to a recipe, with exclusions
   QList<Fermentable*> addToRecipe(Recipe *rec, QList<Fermentable*> ferms, Fermentable* exclude, bool transact = true );
   QList<Hop*> addToRecipe(Recipe *rec, QList<Hop*> hops, Hop* exclude, bool transact = true );
   QList<Misc*> addToRecipe(Recipe *rec, QList<Misc*> miscs, Misc* exclude, bool transact = true );
   QList<Yeast*> addToRecipe(Recipe *rec, QList<Yeast*> yeasts, Yeast* exclude, bool transact = true );

   /**
   * \brief  This function is intended to be called by an ingredient that has not already cached its parent's key
   * \return Key of parent ingredient if there is one, 0 otherwise
   */
   int getParentNamedEntityKey(NamedEntity const & ingredient);

   /*! \brief Removes the specified ingredient from the recipe, then calls the changed()
    *         signal corresponding to the appropriate QList
    *         of ingredients in rec.
    *  \param rec
    *  \param ing
    *  \returns the parent of the ingredient deleted (which is needed to be able to undo the removal)
    */
   NamedEntity * removeNamedEntityFromRecipe( Recipe* rec, NamedEntity* ing );

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

      int ndx;
      bool emitSignal;

      foreach(T* toBeDeleted, list) {
         const QMetaObject* meta = toBeDeleted->metaObject();
         ndx = meta->indexOfClassInfo("signal");
         emitSignal = ndx != -1 ? true : false;

         remove(toBeDeleted, emitSignal);
      }
   }

   template <class T>void remove(T* ing, bool emitSignal = true)
   {
      if (!ing) return;

      const QMetaObject *meta = ing->metaObject();
      char const * propName = "";
      Brewtarget::DBTable ingTable = dbDefn->classNameToTable(meta->className());

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
         deleteRecord(ing);
      }
      catch (QString e) {
         throw;
      }

      // Brewnotes are weird and don't emit a metapropery change
      if ( emitSignal )
         emit changed( metaProperty(propName), QVariant() );
      // This was screaming until I needed to emit a freaking signal
      if ( ingTable != Brewtarget::MASHSTEPTABLE )
         emit deletedSignal(ing);
   }

   //! Get the recipe that this \b ing is part of.
   Recipe* getParentRecipe(NamedEntity const * ing);

   //! Get the recipe that this \b note is part of.  (BrewNotes are stored differently so we need a different function
   //  for them.)
   Recipe* getParentRecipe( BrewNote const* note );

   //! And lets not even get started on mash steps, am I right?
   Recipe* getParentRecipe( MashStep const* step );

   //! For things without in_recipe tables
   QString findRecipeFromForeignKey(TableSchema* tbl, NamedEntity const *obj);
   QString findRecipeFromInRec(TableSchema* tbl, TableSchema* inrec, NamedEntity const *obj);

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
   Q_PROPERTY( QList<Salt*> salts READ salts /*WRITE*/ NOTIFY changed STORED false )
   Q_PROPERTY( QList<Yeast*> yeasts READ yeasts /*WRITE*/ NOTIFY changed STORED false )

   // Returns non-deleted NamedEntitys.
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
   QList<Salt*> salts();
   QList<Yeast*> yeasts();

   /**
    * Templated static versions of the above functions, so other parts of the code can call Database::getAll<Hop>,
    * Database::getAll<Yeast>, etc.
    *
    * This is a template where we _only_ use the specialisations - ie there isn't a general definition.  The
    * specialisations are trivial functions and, in theory, since C++17, we should be able to define them here, eg
    * immediately after the template declaration:
    *   template<class S> static QList<S *> getAll();
    *   template<> QList<BrewNote*>  getAll<BrewNote>()   { return Database::instance().brewNotes(); }
    *   template<> QList<Equipment*> getAll<Equipment>() { return Database::instance().equipments(); }
    *   etc
    * However, due to bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282, this won't compile in gcc.  So we
    * we therefore put them in the cpp file.  (This is fine because callers to getAll<T>() just need the generic bit
    * here in the header file to compile, and the specific implementations of getAll<BrewNote>(), getAll<Equipment>()
    * are only required by the linker.
    */
   template<class S> QList<S *> getAll();

   //! \b returns a list of all ancestors of a recipe
   QList<int> ancestoralIds(Recipe const* descendant);
   //! \b returns a list of the brew notes in a recipe.
   QList<BrewNote*> brewNotes(Recipe const* parent,bool recurse = true);
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
   //! Return a list of all the salts in a recipe.
   QList<Salt*> salts( Recipe const* parent );
   //! Return a list of all the yeasts in a recipe.
   QList<Yeast*> yeasts( Recipe const* parent );
   //! Get recipe's equipment.
   Equipment* equipment(Recipe const* parent);
   //! Get the recipe's mash.
   Mash* mash( Recipe const* parent );
   //! Get recipe's style.
   Style* style(Recipe const* parent);
   Style* styleById(int styleId );
   //! Return a list of all the steps in a mash.
   QList<MashStep*> mashSteps(Mash const* parent);

   QString textFromValue(QVariant value, QString type);

   //! Get the file where this database was loaded from.
   static QString getDbFileName();

   /*!
    * Updates the brewtarget-provided ingredients from the given sqlite
    * database file.
    */
   void updateDatabase(QString const& filename);
   //!brief convenience method for use by updateDatabase
   void bindForUpdateDatabase(TableSchema* tbl, QSqlQuery qry, QSqlRecord rec);
   void convertFromXml();

   bool isConverted();
   bool wantsVersion(Recipe* rec);
   void setAncestor(Recipe* descendant, Recipe* ancestor, bool transact = true);

   Recipe* breed(Recipe* parent);
   Recipe* copyRecipeExcept(Recipe *other, NamedEntity* except);

   NamedEntity* clone( NamedEntity* donor, Recipe* rec );
   NamedEntity* clone( Recipe* rec, NamedEntity *donor, QString propName, QVariant value);

   //! \brief Figures out what databases we are copying to and from, opens what
   //   needs opens and then calls the appropriate workhorse to get it done.
   void convertDatabase(QString const& Hostname, QString const& DbName,
                        QString const& Username, QString const& Password,
                        int Portnum, Brewtarget::DBTypes newType);

   BeerXML* getBeerXml() { return m_beerxml; }

signals:
   void changed(QMetaProperty prop, QVariant value);

   void createdSignal(BrewNote*);
   void createdSignal(Equipment*);
   void createdSignal(Fermentable*);
   void createdSignal(Hop*);
   void createdSignal(Mash*);
   void createdSignal(Misc*);
   void createdSignal(Recipe*);
   void createdSignal(Style*);
   void createdSignal(Water*);
   void createdSignal(Salt*);
   void createdSignal(Yeast*);

   void deletedSignal(Equipment*);
   void deletedSignal(Fermentable*);
   void deletedSignal(Hop*);
   void deletedSignal(Instruction*);
   void deletedSignal(Mash*);
   void deletedSignal(Misc*);
   void deletedSignal(Recipe*);
   void deletedSignal(Style*);
   void deletedSignal(Water*);
   void deletedSignal(Salt*);
   void deletedSignal(Yeast*);
   void deletedSignal(BrewNote*);
   void deletedSignal(MashStep*);

   // MashSteps need signals too
   void newMashStepSignal(MashStep*);

   // Sigh
   void changedInventory(Brewtarget::DBTable,int,QVariant);

   // emits a signal when we create a version
   void spawned(Recipe* ancestor, Recipe* descendant);

private slots:
   //! Load database from file.
   bool load();

private:
   static Database* dbInstance; // The singleton object
   static DatabaseSchema* dbDefn;

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

   // Each thread should have its own connection to QSqlDatabase.
   static QHash< QThread*, QString > _threadToConnection;
   static QMutex _threadToConnectionMutex;

   // Instance variables.
   bool loadWasSuccessful;
   bool converted;
   bool createFromScratch;
   bool schemaUpdated;
   BeerXML* m_beerxml;

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
   QHash< int, Salt* > allSalts;
   QHash< int, Yeast* > allYeasts;
   QHash<QString,QSqlQuery> selectSome;

   //! Get the right database connection for the calling thread.
   static QSqlDatabase sqlDatabase();

   //! Helper to populate all* hashes. T should be a NamedEntity subclass.
   template <class T> void populateElements( QHash<int,T*>& hash, Brewtarget::DBTable table );

   //! we search by name enough that this is actually not a bad idea
   // Although this is private, it needs to be defined in the header as it's called from BeerXML
   template <class T> bool getElementsByName( QList<T*>& list, Brewtarget::DBTable table, QString name, QHash<int,T*> allElements, QString id=QString("") )
   {
      QSqlQuery q(sqlDatabase());
      TableSchema* tbl = dbDefn->table( table );
      q.setForwardOnly(true);
      QString queryString;

      if ( id.isEmpty() )
         id = tbl->keyName(Brewtarget::dbType());
      else
         id = tbl->propertyToColumn(id);

      queryString = QString("SELECT %1 as id FROM %2 WHERE %3=:name")
            .arg(id)
            .arg(tbl->tableName())
            .arg(tbl->propertyToColumn(PropertyNames::NamedEntity::name));

      try {
         q.prepare(queryString);
         q.bindValue(":name", name);
         if ( ! q.exec() )
            throw QString("could not execute query: %2 : %3").arg(queryString).arg(q.lastError().text());
      }
      catch (QString e) {
         qCritical() << Q_FUNC_INFO << e;
         q.finish();
         throw;
      }

      while( q.next() )
      {
         int key = q.record().value("id").toInt();
         if( allElements.contains(key) )
            list.append( allElements[key] );
      }

      q.finish();
      return true;
   }

   //! Helper to populate the list using the given filter.
   template <class T> bool getElements( QList<T*>& list, QString filter, Brewtarget::DBTable table,
                                        QHash<int,T*> allElements, QString id=QString() );


   //! Hidden constructor.
   Database();
   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   Database(Database const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   Database& operator=(Database const&) = delete;
   //! Destructor hidden.
   ~Database();

   //! Helper to more easily get QMetaProperties.
   QMetaProperty metaProperty(const char* name);

   //! Mark the \b object in \b table as deleted.
   void deleteRecord( NamedEntity* object );

   // Note -- this has to happen on a transactional boundary. We are touching
   // something like four tables, and just sort of hoping it all works.
   /*!
    * Create a \e copy (by default) of \b ing and add the copy to \b recipe where \b ing's
    * key is \b ingKeyName and the relational table is \b relTableName.
    *
    * \tparam T the type of ingredient. Must inherit NamedEntity.
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
   template<class T> T* addNamedEntityToRecipe(
      Recipe* rec,
      NamedEntity* ing,
      bool noCopy = false,
      QHash<int,T*>* keyHash = 0,
      bool doNotDisplay = true,
      bool transact = true
   );

   //! fetches one row from the given table. We do this a lot, so do it once.
   QSqlRecord fetchOne(TableSchema* tbl, int key);
   /*!
    * \brief Create a deep copy of the \b object.
    * \em T must be a subclass of \em NamedEntity.
    * \returns a pointer to the new copy. You must manually emit the changed()
    * signal after a copy() call. Also, does not insert things magically into
    * allHop or allInstructions etc. hashes. This just simply duplicates a
    * row in a table, unless you provide \em keyHash.
    * \param object is the thing you want to copy.
    * \param displayed is true if you want the \em displayed column set to true.
    * \param keyHash if nonzero, inserts the new (key,T*) pair into the hash.
    */
   template<class T> T* copy( NamedEntity const* object, QHash<int,T*>* keyHash, bool displayed = true );

   /*!
    * \brief Create a deep copy of the \b object, except for \b propName which
    * gets \b value.
    * \em T must be a subclass of \em NamedEntity.
    * \returns a pointer to the new copy. You must manually emit the changed()
    * signal after a copy() call. Also, does not insert things magically into
    * allHop or allInstructions etc. hashes. This just simply duplicates a
    * row in a table, unless you provide \em keyHash.
    * \param object is the thing you want to copy.
    * \param keyHash if nonzero, inserts the new (key,T*) pair into the hash.
    * \param propName the property that will be replaced
    * \param value that new value for \b propName
    */
   template<class T> T* replicant(NamedEntity const* object, QHash<int,T*>* keyHash, QString propName, QVariant value);

   // Do an sql update.
   void sqlUpdate( Brewtarget::DBTable table, QString const& setClause, QString const& whereClause );

   // Do an sql delete.
   void sqlDelete( Brewtarget::DBTable table, QString const& whereClause );

   int getQualifiedHopTypeIndex(QString type, Hop* hop);
   int getQualifiedMiscTypeIndex(QString type, Misc* misc);
   int getQualifiedMiscUseIndex(QString use, Misc* misc);
   int getQualifiedHopUseIndex(QString use, Hop* hop);

   // Returns true if the schema gets updated, false otherwise.
   // If err != 0, set it to true if an error occurs, false otherwise.
   bool updateSchema(bool* err = nullptr);

   // May St. Stevens intercede on my behalf.
   //
   //! \brief opens an SQLite db for transfer
   QSqlDatabase openSQLite();

   //! \brief opens a PostgreSQL db for transfer. I need
   QSqlDatabase openPostgres(QString const& Hostname, QString const& DbName,
                             QString const& Username, QString const& Password,
                             int Portnum);

   //! \brief converts sqlite values (mostly booleans) into something postgres wants
   QVariant convertValue(Brewtarget::DBTypes newType, QSqlField field);

   //! \brief does the heavy lifting to copy the contents from one db to the next
   void copyDatabase( Brewtarget::DBTypes oldType, Brewtarget::DBTypes newType, QSqlDatabase oldDb);
   void automaticBackup();

};

#endif   /* _DATABASE_H */

