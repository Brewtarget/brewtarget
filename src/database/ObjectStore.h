/*
 * database/ObjectStore.h is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef DATABASE_OBJECTSTORE_H
#define DATABASE_OBJECTSTORE_H
#pragma once
#include <functional>

#include <memory> // For PImpl
#include <optional>

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

#include "utils/BtStringConst.h"
class Database;
class NamedParameterBundle;


/**
 * \brief Base class for storing objects (of a given class) in (a) the database and (b) a local in-memory cache.
 *
 *        This class does all the generic work and, by virtue of being a non-template class, can have most of its
 *        implementation private.  The template class \c ObjectStoreTyped then does the class-specific work (eg
 *        call constructor for the right type of object) and provides a class- specific interface (so that callers
 *        don't have to downcast return values etc).
 *
 *        A further namespace \c ObjectStoreWrapper slightly simplifies the syntax of calls into \c ObjectStoreTyped
 *        member functions.
 *
 *        Note that we do not try to implement every single feature of SQL.  This is not a generic object-to-relational
 *        mapper.  It's just as much as we need.
 *
 *        For the moment at least, we do not support triggers as, AFAICT, they are not needed.  (The DB might be using
 *        its own triggers to handle primary key columns, but that happens without our intervention once we've specified
 *        the column type.)
 *
 *        Inheritance from QObject is to allow this class to send signals (and therefore that inheritance needs to be
 *        public).
 */
class ObjectStore : public QObject {
   // We also need the Q_OBJECT macro to use signals and/or slots
   Q_OBJECT

public:
   /**
    * \brief The different field types that can be stored directly in an object's DB table.
    *
    *        Note that older versions of the code do a lot of special handling for boolean because SQLite has no native
    *        boolean type and therefore stores bools as integers (0 or 1).  However, since bugs in this area of Qt were
    *        fixed back in 2012 -- see https://bugreports.qt.io/browse/QTBUG-23895 (and
    *        https://bugreports.qt.io/browse/QTBUG-15640) -- I believe we are now safe to rely on QVariant to do all
    *        the right conversions for us.
    */
   enum FieldType {
      Bool,
      Int,
      UInt,
      Double,
      String,
      Date,
      Enum   // Stored as a string in the DB
   };

   /**
    * \brief Associates an enum value with a string representation in the DB.  This is more robust than just storing
    *        the raw numerical value of the enum.
    */
   struct EnumAndItsDbString {
      QString string;
      int     native;
   };

   /**
    * \brief We don't actually bother creating hashmaps or similar between enum values and string representations
    *        because it's usually going to be a short list that we can search through pretty quickly (probably faster
    *        than calculating the hash of a key!)
    */
   typedef QVector<EnumAndItsDbString> EnumStringMapping;

   //
   // It's a bit tedious having to create constructors for structs but we need them to allow BtStringConst members to be
   // constructed from a string literal without having to put wrappers (BtStringConst const {}) around each string
   // literal.
   //
   // The reason we don't have existing constants for table name and column name string literals (in contrast with
   // property names) is that these values are not needed anywhere else in the code.
   //

   struct TableDefinition;
   struct TableField {
      FieldType const                 fieldType;
      BtStringConst const             columnName;   // Shouldn't ever be empty in practice
      BtStringConst const             propertyName; // Can be empty in a junction table (see below)
      EnumStringMapping const * const enumMapping;  // Only needed if fieldType is Enum
      TableDefinition const * const   foreignKeyTo;
      //! Constructor
      TableField(FieldType const                 fieldType,
                 char const * const              columnName = nullptr,
                 BtStringConst const &           propertyName = BtString::NULL_STR,
                 EnumStringMapping const * const enumMapping = nullptr,
                 TableDefinition const * const   foreignKeyTo = nullptr) :
         fieldType{fieldType},
         columnName{columnName},
         propertyName{propertyName},
         enumMapping{enumMapping},
         foreignKeyTo{foreignKeyTo} {
         return;
      }
   };

   /**
    * \brief The main table in which objects of the type handled by this \c ObjectStore live, and how to map between
    *        object properties and table fields.
    */
   struct TableDefinition {
      BtStringConst tableName;
      QVector<TableField> const tableFields;
      //! Constructor
      TableDefinition(char const * const tableName = nullptr,
                      std::initializer_list<TableField> const tableFields = {}) :
         tableName{tableName},
         tableFields{tableFields} {
         return;
      }
   };

   /**
    * \brief See \c JunctionTableDefinition
    */
   enum AssumedNumEntries {
      MAX_ONE_ENTRY,
      MULTIPLE_ENTRIES_OK
   };

   /**
    * \brief Cross-references to other objects that are stored in a junction table.  (See
    *        https://en.wikipedia.org/wiki/Associative_entity)  Eg, for a Recipe, there are several junction tables
    *        (fermentable_in_recipe, hop_in_recipe, etc) to store info where potentially many other objects
    *        (Fermentable, Hop, etc) are associated with a single recipe.
    *
    *        We could store junction table information in quite a concise structure, but we prefer to extend the same
    *        structure used for primary tables.  This is more consistent, and simplifies some of the code, at the
    *        expense of a little more boilerplate text in the JUNCTION_TABLE definitions in ObjectStoreTyped.cpp.
    *
    *        NB: What we are storing here is the junction table from the point of view of one class.  Eg
    *        fermentable_in_recipe could be seen from the point of view of the Recipe or of the Fermentable.  In this
    *        particular example, it will be configured from the point of view of the Recipe because the Recipe class
    *        knows about which Hops it uses (but the Hop class does not know about which Recipes it is used in).
    *
    *        .:TBD:. For reasons that are not entirely clear, the parent-child relationship between various objects is
    *        also stored in junction tables.  Although we could change this, it's more likely in the long run that we
    *        change how the parent-child stuff works as it currently involves lots of duplicated data.
    *
    * \param tableName
    * \param tableFields  • First entry is the primary key of this table, which we don't explicitly use (other
    *                               than for table creation) and therefore has no property name.
    *                             • Second entry is the foreign key to primary table of this store.  Its property name
    *                               should be the same as that of the primary key of this store's primary table.  Its
    *                               \c foreignKeyTo (which we use only for table creation) should point to the
    *                               \c TableDefinition for this store.
    *                             • Third entry is a field to be stored in the objects managed by this store against
    *                               the property name specified.  If this is a true junction table then its
    *                               \c foreignKeyTo will show what table it is a foreign key to -- information that we
    *                               use only for table creation
    *                             • Fourth entry is optional.  If present, it is used for ordering.  We assume the
    *                               actual values do not matter, just the ordering they imply, so it has no property
    *                               name.
    * \param assumedNumEntries  When passing data between the table and the object, we'll normally pass a list of
    *                           values (typically integers).  However, if \c assumedNumEntries is set to
    *                           \c MAX_ONE_ENTRY, then we'll pull at most one matching row and pass an integer (wrapped
    *                           in QVariant and thus 0 for an integer if no row returned).
    */
   struct JunctionTableDefinition : public TableDefinition {
      AssumedNumEntries assumedNumEntries = MULTIPLE_ENTRIES_OK;
      JunctionTableDefinition(char const * const tableName = nullptr,
                              std::initializer_list<TableField> tableFields = {},
                              AssumedNumEntries assumedNumEntries = MULTIPLE_ENTRIES_OK) :
         TableDefinition{tableName, tableFields},
         assumedNumEntries{assumedNumEntries} {
         return;
      }

   };


   // This isn't strictly necessary, but it makes various declarations more concise
   typedef QVector<JunctionTableDefinition> JunctionTableDefinitions;

   /**
    * \brief Constructor sets up mappings but does not read in data from DB
    *
    * \param primaryTable  First in the list should be the primary key
    * \param junctionTables  Optional
    */
   ObjectStore(TableDefinition const &          primaryTable,
               JunctionTableDefinitions const & junctionTables = JunctionTableDefinitions{});

   ~ObjectStore();

   /**
    * \brief Create the table(s) for the objects handled by this store.  It is the caller's responsibility to handle
    *        transactions (on the assumption that callers will typically want to call \c createTables() on all
    *        \c ObjectStore objects, then call \c addTableConstraints() on the same, then, potentially, import data.
    */
   bool createTables(Database & database, QSqlDatabase & connection) const;

   /**
    * \brief Add (eg foreign key) constraints to the table(s) for the objects handled by this store
    */
   bool addTableConstraints(Database & database, QSqlDatabase & connection) const;

   /**
    * \brief Load from database all objects handled by this store
    *
    * \param database Sets and stores the Database this store is going to work with.  If not supplied (or set to
    *                 nullptr) then the store will use \c Database::getInstance()
    */
   void loadAll(Database * database = nullptr);

   /**
    * \brief Create a new object of the type we are handling, using the parameters read from the DB.  Subclass needs to
    *        implement.
    */
   virtual std::shared_ptr<QObject> createNewObject(NamedParameterBundle & namedParameterBundle) = 0;

   /**
    * \brief Insert a new object in the DB (and in our cache list)
    *
    * \return The ID of what was inserted
    */
   virtual int insert(std::shared_ptr<QObject> object);

   /**
    * \brief Update an existing object in the DB
    */
   virtual void update(std::shared_ptr<QObject> object);

   /**
    * \brief Convenience function that calls either \c insert or \c update, depending on whether the object is already
    *        stored.
    *
    * \return What was inserted or updated
    */
   virtual std::shared_ptr<QObject> insertOrUpdate(std::shared_ptr<QObject> object);

   /**
    * \brief Raw pointer version of \c insertOrUpdate
    *
    * \param object
    *
    * \return ID of what was inserted or updated
    */
   int insertOrUpdate(QObject & object);

   /**
    * \brief Update a single property of an existing object in the DB
    */
   void updateProperty(QObject const & object, BtStringConst const & propertyName);

   /**
    * \brief Remove the object from our local in-memory cache
    *
    *        Subclasses can do additional or different work, eg \c ObjectStoreTyped will mark the object as deleted
    *        both in memory and in the database (via the \c "deleted" property of \c NamedEntity which is also stored
    *        in the DB) but will leave the object in the local cache (ie will not call down to this base class member
    *        function).
    *
    * \param id ID of the object to delete
    *
    *        (We take the ID of the object to delete rather than, say, std::shared_ptr<QObject> because it's almost
    *        certainly simpler for the caller to extract the ID than for us.)
    */
   virtual void softDelete(int id);

   /**
    * \brief Remove the object from our local in-memory cache and remove its record from the DB.
    *
    *        Subclasses can do additional work, eg \c ObjectStoreTyped will also mark the in-memory object as
    *        deleted (via the \c "deleted" property of \c NamedEntity).
    *
    *        .:TODO:. Need to work out where to do "is this object used elsewhere" checks - eg should a Hop be deletable if it's used in a Recipe
    *
    * \param id ID of the object to delete
    */
   virtual void hardDelete(int id);

   /**
    * \brief Return \c true if an object with the supplied ID is stored in the cache or \c false otherwise
    */
   bool contains(int id) const;

   /**
    * \brief Return pointer to the object with the specified key (or pointer to null if no object exists for the key,
    *        though callers should ideally check this first via \c contains()  Subclasses are expected to provide a
    *        public override of this function to downcast the QObject shared pointer to a more specific one.
    *
    *        NB: We do NOT simply have a virtual \c getById because we want subclasses to be able to have a different
    *            return type to a \c getById call.  You can't change the return type on a virtual function because, by
    *            definition, callers need to know the return type of the function without knowing which version of it
    *            is called - hence "invalid covariant return type" compiler errors if you try.
    *
    * \return \c nullptr if the object with the specified ID cannot be found
    */
   std::shared_ptr<QObject> getById(int id) const;

   /**
    * \brief Similar to \c getById but returns a list of cached objects matching a supplied list of IDs
    *
    *        NB: This is non-virtual for the same reason as \c getById
    */
   QList<std::shared_ptr<QObject> > getByIds(QVector<int> const & listOfIds) const;

   /**
    * \brief Search for a single object (in the set of all cached objects of a given type) with a lambda.  Subclasses
    *        are expected to provide a public override of this function that implements a class-specific interface.
    *
    *        NB: This is non-virtual for the same reason as \c getById
    *
    * \param matchFunction Takes a shared pointer to object and returns \c true if it's a match or \c false otherwise.
    *
    * \return Shared pointer to the first object that gives a \c true result to \c matchFunction, or \c std::nullopt if
    *         none does
    */
   std::optional< std::shared_ptr<QObject> > findFirstMatching(
      std::function<bool(std::shared_ptr<QObject>)> const & matchFunction
   ) const;
   /**
    * \brief Alternate version of \c findFirstMatching that uses raw pointers
    */
   std::optional< QObject * > findFirstMatching(std::function<bool(QObject *)> const & matchFunction) const;

   /**
    * \brief Search for multiple objects (in the set of all cached objects of a given type) with a lambda.  Subclasses
    *        are expected to provide a public override of this function that implements a class-specific interface.
    *
    *        NB: This is non-virtual for the same reason as \c getById
    *
    * \param matchFunction Takes a pointer to object and returns \c true if it's a match or \c false otherwise.
    *
    * \return List of pointers to all objects that give a \c true result to \c matchFunction.  (The list will be empty
    *         if no objects match.)
    */
   QList<std::shared_ptr<QObject> > findAllMatching(
      std::function<bool(std::shared_ptr<QObject>)> const & matchFunction
   ) const;
   /**
    * \brief Alternate version of \c findAllMatching that uses raw pointers
    */
   QList<QObject *> findAllMatching(std::function<bool(QObject *)> const & matchFunction) const;

   /**
    * \brief Special case of \c findAllMatching that returns a list of all cached objects of a given type
    */
   QList<std::shared_ptr<QObject> > getAll() const;
   /**
    * \brief Raw pointer version of \c getAll
    */
   QList<QObject *> getAllRaw() const;

   /**
    * \brief Write everything in this object store to a new database.  Caller's responsibility to wrap everything in a
    *        transaction and turn off foreign key constraints.
    *
    * \param databaseNew
    * \param connectionNew
    *
    * \return \c true if succeeded \c false otherwise
    */
   bool writeAllToNewDb(Database & databaseNew, QSqlDatabase & connectionNew) const;

signals:
   /**
    * \brief Signal emitted when a new object is inserted in the database.  Parts of the UI that need to display all
    *        objects of this type should connect this signal to a slot.  NB: Replaces the following signals:
    *
    *            void Database::newBrewNoteSignal(BrewNote*);        /  void Database::createdSignal(BrewNote*);
    *            void Database::newEquipmentSignal(Equipment*);      /  void Database::createdSignal(Equipment*);
    *            void Database::newFermentableSignal(Fermentable*);  /  void Database::createdSignal(Fermentable*);
    *            void Database::newHopSignal(Hop*);                  /  void Database::createdSignal(Hop*);
    *            void Database::newMashSignal(Mash*);                /  void Database::createdSignal(Mash*);
    *            void Database::newMashStepSignal(MashStep*);
    *            void Database::newMiscSignal(Misc*);                /  void Database::createdSignal(Misc*);
    *            void Database::newRecipeSignal(Recipe*);            /  void Database::createdSignal(Recipe*);
    *            void Database::newSaltSignal(Salt*);                /  void Database::createdSignal(Style*);
    *            void Database::newStyleSignal(Style*);              /  void Database::createdSignal(Water*);
    *            void Database::newWaterSignal(Water*);              /  void Database::createdSignal(Salt*);
    *            void Database::newYeastSignal(Yeast*);              /  void Database::createdSignal(Yeast*);
    *
    *        Note that Qt Signals and Slots can't be in templated classes because part of the code to implement them is
    *        generated by the Meta-Object Compiler (MOC), which runs before main compilation, and therefore before
    *        template instantiation.
    *
    *        Also, NB that when calling QObject::connect() to connect a signal to a slot, you need the \b object that
    *        emits the signal (not the class).
    *
    *        So, we emit the signal here in the virtual base class, and it will be received in slot(s) that have
    *        connected to the relevant singleton instance of the subclass.  Eg, if you connect a slot to
    *        \c ObjectStoreTyped<Water>::getInstance() then its going to receive a signal whenever a new Water
    *        object is inserted in the database.
    *
    *        This also means that the signal parameter can't be type-specific (eg Hop * or shared_ptr<Hop>).  We could
    *        send shared_ptr<QObject> (or even Object *) but then recipients are going to have to downcast it, which
    *        seems a bit clunky.  So, we send the ID of the object, which the recipient can either easily check against
    *        the ID of anything they are holding or use to request an instance of the object.
    *
    *        (In contrast, see below, for \c signalObjectDeleted we do currently have to send the object pointer.)
    *
    * \param id The primary key of the newly inserted object.  (For the moment we assume all primary keys are integers.
    *           If we want to extend this in future then we'd change this param to a QVariant.)
    */
   void signalObjectInserted(int id);

   /**
    * \brief Signal emitted when an object is deleted.  Replaces
    *
    *            void Database::deletedSignal(Equipment*);
    *            void Database::deletedSignal(Fermentable*);
    *            void Database::deletedSignal(Hop*);
    *            void Database::deletedSignal(Instruction*);
    *            void Database::deletedSignal(Mash*);
    *            void Database::deletedSignal(Misc*);
    *            void Database::deletedSignal(Recipe*);
    *            void Database::deletedSignal(Style*);
    *            void Database::deletedSignal(Water*);
    *            void Database::deletedSignal(Salt*);
    *            void Database::deletedSignal(Yeast*);
    *            void Database::deletedSignal(BrewNote*);
    *            void Database::deletedSignal(MashStep*);
    *
    * \param id The primary key of the deleted object.  (For the moment we assume all primary keys are integers.
    *           If we want to extend this in future then we'd change this param to a QVariant.)
    * \param object Shared pointer to the deleted object.  (Some recipients currently need the deleted object, and they
    *               won't be able to get it from the ID because ... it's deleted.
    */
   void signalObjectDeleted(int id, std::shared_ptr<QObject> object);

   /**
    * \brief Signal emitted when an object parameter is changed.  This is for listeners that want to know about any
    *        change to _any_ object of a particular type.  (See \c NamedEntity::changed() for listening to changes to
    *        one specific object.)  NB: Replaces the following signals:
    *            void Database::changedInventory(DatabaseConstants::DbTableId, int, QVariant);
    *
    *        Note that this signal is only emitted when \c updateProperty() is called, NOT when \c update() is called
    *        (as in the latter case we won't know which, if any, properties were changed).
    *
    * \param id The primary key of the object that changed.  (Recipient will already know which class, as, eg, will
    *           connect slot to \c ObjectStoreTyped<InventoryFermentable>::getInstance(),
    *           \c ObjectStoreTyped<InventoryHop>::getInstance(), etc
    * \param propertyName The name of the property that changed
    */
   void signalPropertyChanged(int id, BtStringConst const & propertyName);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   ObjectStore(ObjectStore const &) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   ObjectStore & operator=(ObjectStore const &) = delete;
   //! No move constructor
   ObjectStore(ObjectStore &&) = delete;
   //! No move assignment
   ObjectStore & operator=(ObjectStore &&) = delete;

};


#endif
