/*
 * database/ObjectStoreTyped.h is part of Brewtarget, and is copyright the
 * following authors 2021:
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
#ifndef DATABASE_OBJECTSTORETYPED_H
#define DATABASE_OBJECTSTORETYPED_H
#pragma once
#include <memory>

#include <QDebug>

#include "database/ObjectStore.h"
#include "model/NamedEntity.h"

/**
 * \brief Read, write and cache any subclass of \c NamedEntity in the database
 *
 *        Callers can mostly access via \c ObjectStoreWrapper which is slightly simpler
 */
template<class NE>
class ObjectStoreTyped : public ObjectStore {
public:
   /**
    * \brief Constructor sets up mappings but does not read in data from DB.  Private because singleton.
    *
    * \param primaryTable First in the list of fields in this table defn should be the primary key
    */
   ObjectStoreTyped(TableDefinition const & primaryTable,
                    JunctionTableDefinitions const & junctionTables = JunctionTableDefinitions{}) :
      ObjectStore(primaryTable, junctionTables) {
      return;
   }

   ~ObjectStoreTyped() = default;

public:

   /**
    * \brief Get the singleton instance of this class
    */
   static ObjectStoreTyped<NE> & getInstance();

   /**
    * \brief Insert a new object in the DB (and in our cache list)
    */
   virtual int insert(std::shared_ptr<NE> ne) {
      // The base class does all the work, we just need to cast the pointer
      // (We don't want to force callers to use std::shared_ptr<QObject>, as they would anyway have to recast it.  Eg
      // if you created a new Hop, you're going to need a std::shared_ptr<Hop> etc to be able to access Hop-specific
      // member functions)
      return this->ObjectStore::insert(std::static_pointer_cast<QObject>(ne));
   }

   /**
    * \brief Insert a copy of an existing object in the DB (and in our cache list)
    *
    * \param id  The ID of the object we want to copy
    */
   std::shared_ptr<NE> insertCopyOf(int id) {
      // We could do this all on one line, but we break it down a bit here to make clear what's going on

      // From the supplied ID, get a shared pointer to the object we want to copy
      auto otherNe = this->getById(id);
      if (!otherNe) {
         qWarning() << Q_FUNC_INFO << "Unable to find object #" << id;
      }

      // If we found an object with the supplied ID, make a copy, using the copy constructor (which should do the right
      // thing about parentage etc).  If not, which shouldn't really happen, make a default object.
      auto copyNe = otherNe ? std::make_shared<NE>(*otherNe) : std::make_shared<NE>();

      // Add the copied object to the database and our object cache, and return it to the caller
      this->insert(copyNe);
      return copyNe;
   }

   /**
    * \brief Raw pointer version of \c insert()
    *        Ideally the caller would already have a shared pointer to the object they want to insert, but, until we
    *        have refactored all the calling code, this wrapper function will be useful.
    *
    * \return ID of the newly-inserted object in the database
    */
   int insert(NE & ne) {
      std::shared_ptr<NE> nePointer{&ne};
      return this->insert(nePointer);
   }

   /**
    * \brief Convenience function that calls either \c insert or \c update, depending on whether the object is already
    *        stored.
    *
    * \return What was inserted or updated
    */
   virtual std::shared_ptr<NE> insertOrUpdate(std::shared_ptr<NE> ne) {
      this->ObjectStore::insertOrUpdate(std::static_pointer_cast<QObject>(ne));
      return ne;
   }

   /**
    * \brief Raw pointer version of \c insertOrUpdate
    *
    * \return ID of what was inserted or updated
    */
   int insertOrUpdate(NE & ne) {
      int id = ne.key();
      if (id > 0) {
         std::shared_ptr<NE> nep = this->getById(id);
         this->ObjectStore::update(std::static_pointer_cast<QObject>(nep));
         return id;
      }
      return this->insert(ne);
   }

   /**
    * \brief Return an object for the specified key
    *
    *        This overrides the base-class function of the same name, enabling us (by virtue of the fact that these
    *        particular functions do NOT need to be virtual) to template the return type.
    */
   std::shared_ptr<NE> getById(int id) const {
      if (!this->contains(id)) {
         return nullptr;
      }
      return std::static_pointer_cast<NE>(this->ObjectStore::getById(id));
   }

   /**
    * \brief Similar to \c getById but returns a list of cached objects matching a supplied list of IDs
    */
   QList<std::shared_ptr<NE> > getByIds(QVector<int> const & listOfIds) const {
      // Base class will give us QList<std::shared_ptr<QObject> >, which we convert to QList<std::shared_ptr<NE> >
      return this->convertShared(this->ObjectStore::getByIds(listOfIds));
   }

   /**
    * \brief Raw pointer version of \c getByIds
    */
   QList<NE *> getByIdsRaw(QVector<int> const & listOfIds) const {
      return this->convertRaw(this->ObjectStore::getByIds(listOfIds));
   }

   /**
    * \brief Mark an object as deleted (including in the database) and but leave it in existence (both in the database
    *        and in our local in-memory cache.
    *
    *        NB: We do not call down to \c ObjectStore::softDelete() from this member function (as that would remove the
    *            object from our local in-memory cache.
    *
    * \param id ID of the object to delete
    */
   virtual void softDelete(int id) {
      this->hardOrSoftDelete(id, false);
      return;
   }

   /**
    * \brief Remove the object from our local in-memory cache, mark it as deleted, and remove its record from the DB.
    *
    * \param id ID of the object to delete
    */
   virtual void hardDelete(int id) {
      this->hardOrSoftDelete(id, true);
      return;
   }

   /**
    * \brief Search the set of all cached objects with a lambda.
    *
    * \param matchFunction Takes a pointer to an object and returns \c true if the object is a match or \c false otherwise.
    *
    * \return Shared pointer to the first object that gives a \c true result to \c matchFunction, or \c std::nullopt if
    *         none does
    */
   std::optional< std::shared_ptr<NE> > findFirstMatching(std::function<bool(std::shared_ptr<NE>)> const & matchFunction) const {
      //
      // Caller has provided us with a lambda function that takes a shared pointer to NE (ie Water, Hop, Yeast, Recipe,
      // etc) and returns true or false depending on whether it's a match for whatever condition the caller requires.
      //
      // The base class findMatching() expects a lambda function that takes a std::shared_ptr<QObject> parameter.
      //
      // So, to call the base class findMatching(), we need to create our own "wrapper" lambda that receives a
      // std::shared_ptr<QObject> parameter and casts it to std::shared_ptr<NE>.
      //
      auto result = this->ObjectStore::findFirstMatching(
         [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(std::static_pointer_cast<NE>(obj));}
      );
      if (!result.has_value()) {
         return std::nullopt;
      }
      return std::optional< std::shared_ptr<NE> >{std::static_pointer_cast<NE>(result.value())};
   }

   /**
    * \brief Alternate version of \c findFirstMatching that uses raw pointers
    *
    * \return Pointer to the first object that gives a \c true result to \c matchFunction, or \c nullptr if
    *         none does
    */
   NE * findFirstMatching(std::function<bool(NE *)> const & matchFunction) const {
      //
      // Caller has provided us with a lambda function that takes a pointer to NE (ie Water, Hop, Yeast, Recipe, etc)
      // and returns true or false depending on whether it's a match for whatever condition the caller requires.
      //
      // The base class findMatching() expects a lambda function that takes a std::shared_ptr<QObject> parameter.
      //
      // So, to call the base class findMatching(), we need to create our own "wrapper" lambda that receives a
      // std::shared_ptr<QObject> parameter, extracts the raw pointer from it (which we know will always be valid) and
      // downcasts it from "QObject *" to "NE *".
      //
      auto result = this->ObjectStore::findFirstMatching(
         [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(static_cast<NE *>(obj.get()));}
      );
      if (!result.has_value()) {
         return nullptr;
      }
      return static_cast<NE *>(result.value().get());
   }

   /**
    * \brief Search the set of all cached objects with a lambda.
    *
    * \param matchFunction Takes a pointer to an object and returns \c true if the object is a match or \c false otherwise.
    *
    * \return List of shared pointers to all the objects that give a \c true result to \c matchFunction (and thus an
    *         empty list if none does).
    */
   QList<std::shared_ptr<NE> > findAllMatching(
      std::function<bool(std::shared_ptr<NE>)> const & matchFunction
   ) const {
      // Base class will give us QList<std::shared_ptr<QObject> >, which we convert to QList<std::shared_ptr<NE> >
      return this->convertShared(
         // As per above, we make a wrapper around the supplied lambda to do the necessary casting
         this->ObjectStore::findAllMatching(
            [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(std::static_pointer_cast<NE>(obj));}
         )
      );
   }

   /**
    * \brief Alternate version of \c findAllMatching that uses raw pointers
    *
    * \return List of pointers to all the objects that give a \c true result to \c matchFunction (and thus an
    *         empty list if none does).
    */
   QList<NE *> findAllMatching(std::function<bool(NE *)> const & matchFunction) const {
      return this->convertRaw(
         this->ObjectStore::findAllMatching(
            [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(static_cast<NE *>(obj.get()));}
         )
      );
   }

   /**
    * \brief Special case of \c findAllMatching that returns a list of all cached objects of a given type
    */
   QList<std::shared_ptr<NE> > getAll() {
      return this->convertShared(this->ObjectStore::getAll());
   }
   /**
    * \brief Raw pointer version of \c getAll
    */
   QList<NE *> getAllRaw() {
      return this->convertRaw(this->ObjectStore::getAll());
   }

protected:
   /**
    * \brief Create a new object of the type we are handling, using the parameters read from the DB
    */
   virtual std::shared_ptr<QObject> createNewObject(NamedParameterBundle & namedParameterBundle) {
      //
      // NB: std::static_pointer_cast actually creates a new instance of std::shared_ptr (whose stored pointer is
      // obtained from its parameter's stored pointer using a cast expression).  So there is no point creating a
      // shared_ptr of one type if we're straight away going to cast it to another type; just create the type we need
      // in the first place.
      //
      return std::shared_ptr<QObject>(new NE{namedParameterBundle});
   }

private:
   /**
    * \brief Do a hard or soft delete
    *
    * \param id ID of the object to delete
    * \param hard \c true for hard delete, \c false for soft delete
    */
   void hardOrSoftDelete(int id, bool hard) {
      qDebug() <<
         Q_FUNC_INFO << (hard ? "Hard" : "Soft") << "delete " << NE::staticMetaObject.className() << " #" << id;
      if (id <= 0 || !this->contains(id)) {
         // This is probably a coding error, but might be recoverable
         qWarning() <<
            Q_FUNC_INFO << "Trying to delete non-existent " << NE::staticMetaObject.className() << " with ID" << id;
         return;
      }

      auto object = this->ObjectStore::getById(id);
      std::shared_ptr<NE> ne = std::static_pointer_cast<NE>(object);
      if (hard) {
         // If the NamedEntity we are deleting owns any other NamedEntity objects (eg Mash owns its MashSteps) then tell
         // it to delete those first.
         ne->hardDeleteOwnedEntities();
         // Base class does the heavy lifting on removing the NamedEntity from the DB
         this->ObjectStore::hardDelete(id);
      }

      // This marks the in-memory object as deleted (and will get pushed down to the DB if this is a soft delete)
      ne->setDeleted(true);
      ne->setDisplay(false);

      if (!hard) {
         // Base class softDelete() actually does too much for the soft delete case; we just need to tell any bits of
         // the UI that need to know that an object was deleted.  (In the hard delete case, this signal will already
         // have been emitted.)
         emit this->signalObjectDeleted(id, object);
      }
      return;
   }

   /**
    * \brief Convert QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >
    */
   QList<std::shared_ptr<NE> > convertShared(QList<std::shared_ptr<QObject> > const results) const {
      // We can't just cast the resulting QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >, so we need
      // to create a new QList of the type we want and copy the elements across.
      QList<std::shared_ptr<NE> > convertedResults;
      convertedResults.reserve(results.size());
      std::transform(results.cbegin(),
                     results.cend(),
                     std::back_inserter(convertedResults),
                     [](auto & sharedPointer) { return std::static_pointer_cast<NE>(sharedPointer); });
      return convertedResults;
   }

   /**
    * \brief Convert QList<std::shared_ptr<QObject> > to QList<NE *>
    */
   QList<NE *> convertRaw(QList<std::shared_ptr<QObject> > const results) const {
      // We can't just cast the resulting QList<std::shared_ptr<QObject> > to QList<std::shared_ptr<NE> >, so we need
      // to create a new QList of the type we want and copy the elements across.
      QList<NE *> convertedResults;
      convertedResults.reserve(results.size());
      std::transform(results.cbegin(),
                     results.cend(),
                     std::back_inserter(convertedResults),
                     [](auto & sharedPointer) { return static_cast<NE *>(sharedPointer.get()); });
      return convertedResults;
   }

   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   ObjectStoreTyped(ObjectStoreTyped const &) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   ObjectStoreTyped & operator=(ObjectStoreTyped const &) = delete;

};

/**
 * \brief Does what it says on the tin.  Note that it is the caller's responsibility to handle transactions.
 *
 * \param database
 * \param connection  Need this as might be creating tables in a new database rather than the default one
 *
 * \return false if something went wrong, true otherwise
 */
bool CreateAllDatabaseTables(Database & database, QSqlDatabase & connection);

/**
 * \brief Returns a list of all the database tables managed by all the object stores.  Really only needed for copying
 *        one database to another.
 */
QVector<ObjectStore const *> GetAllObjectStores();

#endif
