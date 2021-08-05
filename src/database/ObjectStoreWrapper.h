/*
 * database/ObjectStoreWrapper.h is part of Brewtarget, and is copyright the
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
#ifndef DATABASE_OBJECTSTOREWRAPPER_H
#define DATABASE_OBJECTSTOREWRAPPER_H
#pragma once
#include "database/ObjectStoreTyped.h"

/**
 * \brief Namespace containing convenience functions for accessing member functions of appropriate ObjectStoreTyped
 *        instances via template argument deduction
 *
 *        Where functions below are not commented, the documentation is in the corresponding function declaration in
 *        ObjectStoreTyped.h
 */
namespace ObjectStoreWrapper {
   template<class NE> std::shared_ptr<NE> getById(int id) {
      return ObjectStoreTyped<NE>::getInstance().getById(id);
   }

   /**
    * \brief Raw pointer version of \c getById
    */
   template<class NE> NE * getByIdRaw(int id) {
      return ObjectStoreTyped<NE>::getInstance().getById(id).get();
   }

   template<class NE> QList<std::shared_ptr<NE> > getAll() {
      return ObjectStoreTyped<NE>::getInstance().getAll();
   }

   template<class NE> QList<NE *> getAllRaw() {
      return ObjectStoreTyped<NE>::getInstance().getAllRaw();
   }

   /**
    * \brief Gets only those objects which are:
    *          - marked displayable
    *          - not marked deleted
    *          - do not have a parent (ie are not "an instance of use of"
    */
   template<class NE> QList<NE *> getAllDisplayableRaw() {
      return ObjectStoreTyped<NE>::getInstance().findAllMatching(
         [](NE const * ne) { return (ne->display() && !ne->deleted() && ne->getParentKey() <= 0); }
      );
   }

   template<class NE> std::shared_ptr<NE> copy(NE const & ne) {
      return std::make_shared<NE>(ne);
   }

   /**
    * \brief Preferred way of inserting a new object in a store.
    *
    *        Caller creates a new object in a shared_ptr and, if and when it caller decides we want to keep (rather than
    *        discard) this new object, calls this function, which will store data in DB and make a copy of the
    *        shared_ptr inside the the ObjectStore.   This means the object will be properly destroyed, without
    *        additional coding, in all cases: (i) object intentionally discarded, (ii) object stored, (iii) ghastly
    *        error happens causing exceptions etc.
    */
   template<class NE> int insert(std::shared_ptr<NE> ne) {
      return ObjectStoreTyped<NE>::getInstance().insert(ne);
   }

   /**
    * \brief Deprecated way of inserting a new object in a store
    *
    *        Caller doesn't have a shared_ptr so gives us a reference to the object directly.  The main problem with
    *        this is that, if the caller doesn't reach the point of storing the object in the object store, then, extra
    *        hand-rolled code, the object's destructor doesn't get called.  A secondary problem is that, if a caller
    *        _does_ have a shared_ptr to the object but inadvertently passes us the reference to the object itself then
    *        the new shared_ptr we create will not know about the caller's shared_ptr, so the object's destructor will
    *        get called twice and, sooner or later, we'll get a segfault.
    */
   template<class NE> int insert(NE & ne) {
      return ObjectStoreTyped<NE>::getInstance().insert(ne);
   }

   template<class NE> std::shared_ptr<NE> insertCopyOf(NE const & ne) {
      return ObjectStoreTyped<NE>::getInstance().insertCopyOf(ne.key());
   }

   template<class NE> int insertOrUpdate(NE & ne) {
      return ObjectStoreTyped<NE>::getInstance().insertOrUpdate(ne);
   }

   template<class NE> void updateProperty(NE const & ne, BtStringConst const & propertyName) {
      ObjectStoreTyped<NE>::getInstance().updateProperty(ne, propertyName);
      return;
   }

   template<class NE> void softDelete(NE const & ne) {
      ObjectStoreTyped<NE>::getInstance().softDelete(ne.key());
      return;
   }

   template<class NE> void hardDelete(int id) {
      ObjectStoreTyped<NE>::getInstance().hardDelete(id);
      return;
   }

   template<class NE> void hardDelete(NE const & ne) {
      ObjectStoreTyped<NE>::getInstance().hardDelete(ne.key());
      return;
   }

   template<class NE> void hardDelete(std::shared_ptr<NE> ne) {
      if (ne.get()) {
        ObjectStoreTyped<NE>::getInstance().hardDelete(ne->key());
      }
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
   template<class NE> std::optional< std::shared_ptr<NE> > findFirstMatching(
      std::function<bool(std::shared_ptr<NE>)> const & matchFunction
   ) {
      return ObjectStoreTyped<NE>::getInstance().findFirstMatching(matchFunction);
   }

   /**
    * \brief Alternate version of \c findFirstMatching that uses raw pointers
    *
    * \return Pointer to the first object that gives a \c true result to \c matchFunction, or \c nullptr if
    *         none does
    */
   template<class NE> NE * findFirstMatching(
      std::function<bool(NE *)> const & matchFunction
   ) {
      return ObjectStoreTyped<NE>::getInstance().findFirstMatching(matchFunction);
   }

   template<class NE> QList<std::shared_ptr<NE> > findAllMatching(
      std::function<bool(std::shared_ptr<NE>)> const & matchFunction
   ) {
      return ObjectStoreTyped<NE>::getInstance().findAllMatching(matchFunction);
   }

   template<class NE> QList<NE *> findAllMatching(
      std::function<bool(NE *)> const & matchFunction
   ) {
      return ObjectStoreTyped<NE>::getInstance().findAllMatching(matchFunction);
   }

   /**
    * \brief Given two IDs of some subclass of \c NamedEntity, return \c true if the corresponding objects are equal (or
    *        if both IDs are invalid), and \c false otherwise
    */
   template<class NE>
   bool compareById(int lhsId, int rhsId) {
      if (lhsId <= 0 && rhsId <= 0) {
         // Both are invalid IDs
         return true;
      }
      NE const * lhs = ObjectStoreWrapper::getByIdRaw<NE>(lhsId);
      NE const * rhs = ObjectStoreWrapper::getByIdRaw<NE>(rhsId);
      if (nullptr == lhs && nullptr == rhs) {
         // Neither ID was found in the ObjectStore
         return true;
      }
      if (nullptr == lhs || nullptr == rhs) {
         // Only one of the IDs was found in the ObjectStore
         return false;
      }
      // Both IDs found in the ObjectStore, so we can compare the corresponding object directly
      return *lhs == *rhs;
   }

   /**
    * \brief Given two lists of IDs of some subclass of \c NamedEntity, return \c true if all the corresponding objects
    *        are equal, and \c false otherwise
    */
   template<class NE>
   bool compareListByIds(QVector<int> & lhsIds, QVector<int> & rhsIds) {
      if (lhsIds.size() != rhsIds.size()) {
         return false;
      }

      // .:TBD:. For the moment we're assuming everything is always in the same order and order matters
      for (int ii = 0; ii < lhsIds.size(); ++ii) {
         if (!compareById<NE>(lhsIds.at(ii), rhsIds.at(ii))) {
            return false;
         }
      }

      return true;
   }
}

#endif
