/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OwnedSet.h is part of Brewtarget, and is copyright the following authors 2024-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_OWNEDSET_H
#define MODEL_OWNEDSET_H
#pragma once

#include <memory>
#include <ranges>

#include <QDebug>
#include <QVector>

#include "utils/AutoCompare.h"
#include "utils/BtStringConst.h"
#include "utils/TypeTraits.h"


//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::OwnedSet { inline BtStringConst const property{#property}; }
AddPropertyName(items)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

struct OwnedSetOptions {
   /**
    * \brief In an enumerated set, items are in a strict order and are accordingly numbered (starting from 1) with a
    *        step number, which is accessed via \c seqNum / \c setSeqNum.  \c OwnedSet::items returns items in this
    *        order.
    *
    *        In a non-enumerated set, the \c OwnedSet class does not manage the order of the items and returns them in
    *        arbitrary order.  (Typically there may be a weak ordering -- eg addition time for a \c RecipeAddition
    *        subclass or brew date for a \c BrewnNote -- but it is not managed inside of \c OwnedSet.
    *
    *        Note that \c setSeqNum has to take an optional second parmeter \c bool \c const \c notify (which should
    *        default to \c true).  This is because, in an enumerated set, we need to be very careful and controlled
    *        about when we (either directly or indirectly) emit signals from our member functions.
    *           If we are part way through modifying a sequence of steps (eg to swap two steps or insert a new one) then
    *        we do not want other parts of the code to read the steps when they are in an "in between" state.  Reading
    *        in such a state might either give incorrect data (eg two steps with the same step number) or, worse, make
    *        unwanted changes (eg normalising step numbers whilst we're in the middle of re-ordering steps).
    *           You might think that we can solve that with locks, but this is complicated because, usually, when we
    *        emit a signal, its slots are straight away run on the same thread before the signal function returns.  This
    *        means the thread that you want to prevent from reading the step sequence is the same one that is already
    *        modifying it.  So, instead, we ensure that whenever sequence numbers are being modified, no signal is
    *        emitted for the change of sequence number on each step.  Instead, a single \c changed signal for the whole
    *        set is emitted at the end.
    */
   bool enumerated = false;

   /**
    * \brief By default, when an object owning a set is copied, we want to do a deep copy of that set - ie create a new
    *        set containing copies of each item in the original set.  This is what \c copyable being \c true means.
    *
    *        The only other option is that, when the owning object is copied, the set is not copied at all, and the set
    *        on the newly-created copy is empty.  This is, for instance, what we do with any \c BrewNote objects on a
    *        \c Recipe.  This is what \c copyable being \c false means.
    *
    *        Note that it can never be possible to do a shallow copy of an \c OwnedSet, since an item in such a set
    *        cannot meaningfully have two different owners.
    */
   bool copyable = true;
};
template <OwnedSetOptions os> struct is_Enumerated : public std::integral_constant<bool, os.enumerated>{};
template <OwnedSetOptions os> struct is_Copyable   : public std::integral_constant<bool, os.copyable  >{};
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <OwnedSetOptions os> concept CONCEPT_FIX_UP IsEnumerated = is_Enumerated<os>::value;
template <OwnedSetOptions os> concept CONCEPT_FIX_UP IsCopyable   = is_Copyable  <os>::value;

/**
 * \brief Template class that handles ownership of either:
 *          - an "unordered" set of things - eg \c Recipe ownership of \c RecipeAdditionFermentable,
 *            \c RecipeAdditionHop etc; or
 *          - an "ordered", aka "enumerated" list of "steps" - eg \c Mash ownership of \c MashSteps
 *
 *        For an "unordered" set, we do not have a strict ordering of the owned items (eg two hop additions could happen
 *        at the same time).
 *
 *        When the set changes, we emit the \c NamedEntity::changed signal on the \c owner object.
 *
 * \param Owner class needs to inherit from \c NamedEntity
 * \param Item class also needs to inherit from \c NamedEntity, plus implement: \c ownerId, \c setOwnerId
 *        For an enumerated set, \c Item also needs to implement \c seqNum, \c setSeqNum
 * \param propertyName is the name of the property that we use to signal changes to the size of the owned set (eg item
 *        added or removed).  It is typically the name of the property holding this \c OwnedSet object, but the value
 *        that we send with the \c changed signal is simply the new size of the set.
 * \param itemChangedSlot if not \c nullptr, is a member function slot on \c Owner that can receive
 *        \c NamedEntity::changed signals from \c Item objects in this set.  (Otherwise, if it is \c nullptr,
 *        \c Owner::acceptStepChange will be used.)  Typically that member function just needs to call our
 *        \c acceptItemChange function.  It is simpler to go via \c Owner because the owner object is able to call
 *        \c QObject::sender to get the sender and pass it to us.  (From looking at the Qt source code eg at
 *        https://github.com/qt/qtbase/blob/dev/src/corelib/kernel/qobject.cpp, it seems \c QObject::sender will return
 *        null if a slot on the object in question is not being invoked, so there is no use us trying to find ways to
 *        call \c this->m_owner.sender(), as we'll always just get back \c nullptr.)
 * \param ownedSetOptions See \c OwnedSetOptions
 */
template<class Owner,
         class Item,
         BtStringConst const & propertyName,
         void (Owner::*itemChangedSlot)(QMetaProperty, QVariant),
         OwnedSetOptions ownedSetOptions = OwnedSetOptions{} >
class OwnedSet {
public:

   //! Non-virtual equivalent of compareWith
   bool doCompareWith(OwnedSet const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
      return AUTO_PROPERTY_COMPARE_FN(this, other, items, PropertyNames::OwnedSet::items, propertiesThatDiffer);
   }

   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   static QString localisedName_items() { return Owner::tr("Items"); }

   /**
    * \brief Minimal constructor.  Note that the reason we do not just initialise everything here is that the object
    *        stores on which we depend might not yet themselves be initialised when this constructor is called.
    *
    * \param owner should be a subclass of \c NamedEntity and we should be being called from its constructor
    */
   OwnedSet(Owner & owner) :
      m_owner{owner} {
      //
      // Note that it is always correct in this constructor for m_itemIds to start out as an empty set.  It is only used
      // when adding items to an owner that has not yet been stored in the DB.  Either a brand new owner is just being
      // created, so it doesn't yet have any items in this OwnedSet, or we just read it out of the DB (in which case
      // we'll be able, in due course, to get the owned items from the relevant object store (once that has been
      // populated from the DB) so we don't need a list of item IDs.
      //
      return;
   }

   /**
    * \brief "Copy" constructor for when we want a no-op copy.
    */
   OwnedSet(Owner & owner, [[maybe_unused]] OwnedSet const & other) requires (!IsCopyable<ownedSetOptions>) :
      m_owner{owner} {
      qDebug() << Q_FUNC_INFO << "Copy is no-op";
      return;
   }

   /**
    * \brief "Copy" constructor for when we want a deep copy.
    */
   OwnedSet(Owner & owner, OwnedSet const & other) requires (IsCopyable<ownedSetOptions>) :
      m_owner{owner} {
      // Deep copy of Steps
      auto otherItems = other.items();
      for (auto item : otherItems) {
         // Make a copy of the current Item object we're looking at in the other OwnedSet
         auto itemToAdd = std::make_shared<Item>(*item);

         // The owner won't yet have an ID, so we can't give it to the new Item
         itemToAdd->setOwnerId(-1);

         // However, if we insert the new Item in the object store, that will give it its own ID
         ObjectStoreWrapper::insert(itemToAdd);
         qDebug() << Q_FUNC_INFO << "Copied" << *item << "to" << *itemToAdd;

         // Store the ID of the copy Item
         // If and when we get our ID then we can give it to our Items
         this->m_itemIds.append(itemToAdd->key());

         // Connect signals so that we are notified when there are changes to the Item we just added to our Owner.
         this->connectItemChangedSignal(itemToAdd);
      }

      return;
   }

   /**
    * \brief We have to delete the default copy constructor that the compiler would generate because that would copy
    *        m_owner, which we don't want.  Instead we need to force callers to provide the new owner as a parameter
    *        (via the constructor above).
    */
   OwnedSet(OwnedSet const & other) = delete;

   /**
    * \brief Similarly, we don't want copy assignment happening.
    */
   OwnedSet & operator=(OwnedSet const & other) = delete;

   ~OwnedSet() = default;

   /**
    * \brief If one of the items in our set changes, we receive its "changed" signal and emit a "changed" signal for the
    *        the set.
    *
    *        TBD: At the moment we emit the same signal for "number of items in the set changed" and "a property of one
    *             of the items in the set changed".  In future, we could do something more sophisticated here if need
    *             be.
    */
   void acceptItemChange(Item const & item, [[maybe_unused]] QMetaProperty prop, [[maybe_unused]] QVariant val) {
      // If one of our items changed, our pseudo properties may also change, so we need to emit some signals
      if (item.ownerId() == this->m_owner.key()) {
         emit this->m_owner.changed(this->m_owner.metaProperty(*propertyName), QVariant());
      }
      return;
   }

   /**
    * \brief Connect all our item's "changed" signals to us
    *
    *        Needs to be called by our Owner \b after all the calls to ObjectStoreTyped<FooBar>::getInstance().loadAll()
    */
   void connectAllItemChangedSignals() {
      for (auto item : this->items()) {
         this->connectItemChangedSignal(item);
      }
      return;
   }

private:
   /**
    * \brief Connect an item's "changed" signal to us.
    *
    *        Unusually, we don't worry about disconnecting this later.  The \c Item object (\c item) will never belong
    *        to any other \c OwnedSet, and Qt will do the disconnection itself when \c item is destroyed.
    */
   void connectItemChangedSignal(std::shared_ptr<Item> item) {
      if constexpr (itemChangedSlot) {
         this->m_owner.connect(item.get(), &NamedEntity::changed, &this->m_owner, itemChangedSlot);
      } else {
         this->m_owner.connect(item.get(), &NamedEntity::changed, &this->m_owner, &Owner::acceptStepChange);
      }
      return;
   }

   void putInOrder(QList<std::shared_ptr<Item>> & items) const requires (IsEnumerated<ownedSetOptions>) {
      std::sort(items.begin(),
                items.end(),
                [](std::shared_ptr<Item> const lhs, std::shared_ptr<Item> const rhs) {
                   // Per https://en.cppreference.com/w/cpp/algorithm/sort, this function needs to return "returns ​true
                   // if the first argument is less than (i.e. is ordered before) the second".
                   return lhs->seqNum() < rhs->seqNum();
                });
      return;
   }

   void normaliseSeqNums(QList<std::shared_ptr<Item>> & items) const requires (IsEnumerated<ownedSetOptions>) {
      int const ownerId = this->m_owner.key();
      for (int canonicalSeqNum = 1, prevSeqNum = 1; auto item : items) {
         int const existingSeqNum = item->seqNum();
         // Normally leave this debug statement commented out as it generates too much logging, but can be useful for
         // troubleshooting.
//         qDebug() <<
//            Q_FUNC_INFO << Owner::staticMetaObject.className() << "#" << ownerId << ":" <<
//            Item::staticMetaObject.className() << "#" << item->key() << "sequence number is" <<
//            existingSeqNum << "; should be" << canonicalSeqNum;

         // It's a coding error if any item in the list does not belong to the owner
         Q_ASSERT(item->ownerId() == ownerId);

         // Unfortunately, for historical reasons, we cannot assume that sequence (aka step) numbers read from the DB
         // will always be unique for items belonging to a given owner (eg MashSteps in a Mash).  So, although we can
         // assert that step numbers never go down (because we ran them through sort), and that they are never less than
         // 1, we cannot assert that they always go up!
         Q_ASSERT(existingSeqNum >= prevSeqNum);

         // At this point, we correct things as best we can here so that the rest of the code can work on the basis of
         // sequence numbers being in sequence starting from 1.
         if (existingSeqNum != canonicalSeqNum) {
            // We don't want a "changed" signal just because we normalised a sequence number.  Firstly, the items are
            // already in the correct order.  We are just ensuring that all the sequence numbers are sequential and
            // start from 1.  Secondly, if we are calling this as part of a modification to the set, we only want one
            // notification, at the end of whatever modification it is we are doing.
            item->setSeqNum(canonicalSeqNum, false);
         }
         prevSeqNum = existingSeqNum;
         ++canonicalSeqNum;
      }
      return;
   }

public:
   QList<std::shared_ptr<Item>> items() const {
      //
      // The Owner object (eg Recipe) owns its Items (eg RecipeAdditionFermentables, RecipeAdditionHops, etc).  But,
      // it's the Item that knows which Owner it's in rather than the Owner which knows which Items it has, so we have
      // to ask.  The only exception to this is if the Owner is not yet stored in the DB, in which case there is not yet
      // any Owner ID to give to the Items, so we store an internal list of them.
      //
      int const ownerId = this->m_owner.key();

      QList<std::shared_ptr<Item>> items;
      if (ownerId < 0) {
         for (int ii : this->m_itemIds) {
            items.append(ObjectStoreWrapper::getById<Item>(ii));
         }
         // We don't need to sort here as we assume m_itemIds is already in the correct order (if there is one)

      } else {
         items = ObjectStoreWrapper::findAllMatching<Item>(
            [ownerId](std::shared_ptr<Item> const item) {return item->ownerId() == ownerId && !item->deleted();}
         );
      }

      //
      // Couple of extra things we need to do for enumerated sets.  Obviously the joy of templates is that we know at
      // compile-time whether the set is enumerated or not, hence the constexpr here.
      //
      if constexpr (IsEnumerated<ownedSetOptions>) {
         //
         // The object store does not guarantee what order it returned the items in, so, if they are enumerated, we need
         // to put them in the right order.  The same comment applies to our m_itemIds list.  For enumerated sets, we
         // _could_ enforce that the order in m_itemIds is the same as the ordering implied by seqNum() on the set
         // members, but this would make other parts of the code a bit more complicated (where we share logic between
         // enumerated and non-enumerated sets) for little if no gain.
         //
         this->putInOrder(items);

         //
         // It can be that, although they are in the right order, the items are not canonically numbered.  If this happens,
         // it looks a bit odd in the UI -- eg because you have Instructions in a Recipe starting with Instruction #2 as
         // the first one.  We _could_ fix this in the UI layer, but it's easier to do it here -- and, since we're never
         // talking about more than a handful of items (often less than 10, usually less than 20, pretty much always less
         // than 30), the absolute overhead of doing so should be pretty small.
         //
         this->normaliseSeqNums(items);
      }

      return items;
   }

   //! An alternate way of calling \c items.  Used in \c trees/TreeModelBase.h
   static QList<std::shared_ptr<Item>> ownedBy(Owner const & owner) {
      return owner.items();
   }

   /**
    * \return Number of items in the set
    */
   size_t size() const {
      // We could optimise this a bit, but it's not noticeably hurting performance
      return this->items().size();
   }

   /**
    * \brief For the moment we only do the unenumerated version of this.  For the enumerated version, we would implement
    *        in terms of \c items().
    */
   QVector<int> itemIds() const {
      int const ownerId = this->m_owner.key();
      if (ownerId < 0) {
         return this->m_itemIds;
      }

      return ObjectStoreWrapper::idsOfAllMatching<Item>(
         [ownerId](Item const * item) { return item->ownerId() == ownerId; }
      );
   }

   /**
    * \brief Returns the number of items matching \c matchFunction
    */
   int numMatching(std::function<bool(Item const &)> const & matchFunction) const {
      //
      // We could use std::accumulate here, but it doesn't buy us anything.  Firstly, because this->items() returns a
      // _copy_ of the list of items, we can't , eg, write:
      //    int const numItems = std::accumulate(
      //       this->items().cbegin(),
      //       this->items().cend(),
      //       ...
      // because the begin and end iterators would be in different lists!
      //
      // Secondly, we'd have to have the lambda for std::accumulate invoke the lambda passed into this function, which
      // all starts to get a but cumbersome.
      //
      int count = 0;
      for (std::shared_ptr<Item> item : this->items()) {
         if (matchFunction(*item)) {
            ++count;
         }
      }
      return count;
   }


private:
   /**
    * \brief If we changed the set in any way, we call this function to have the owner emit a signal
    *
    * \param newSize If the caller already knows the set size, they pass it in to save us working it out.
    */
   void emitSetChanged(std::optional<int> const newSize = std::nullopt) {
      auto const sizeToEmit {newSize.value_or(this->items().size())};
      qDebug() <<
         Q_FUNC_INFO << "Emitting set changed signal (size =" << sizeToEmit << ") for" <<
         Owner::staticMetaObject.className() << "#" << this->m_owner.key();
      emit this->m_owner.changed(this->m_owner.metaProperty(*propertyName), sizeToEmit);

      //
      // For the moment at least, various things dealing with steps are expecting a specific signal stepsChanged rather
      // than the generic NamedEntity::changed one, so send that too.  (Non-step owners do not have the stepsChanged
      // signal though.)
      //
      if constexpr (IsEnumerated<ownedSetOptions>) {
         emit this->m_owner.stepsChanged();
      }

      return;
   }

   /**
    * \brief Adds a new item to the set.  This is private because, for enumerated sets, we need to handle step number in
    *        the public functions that calls this one -- so we don't want it to be possible for this to be called from
    *        outside the class for an enumerated set.
    */
   std::shared_ptr<Item> extend(std::shared_ptr<Item> item) {
      //
      // It's a coding error to add an item to a set if it is already in one
      //
      if (item->ownerId() > 0) {
         qWarning() <<
            Q_FUNC_INFO << "Trying to add" << Item::staticMetaObject.className() << "#" << item->key() << "to" <<
            Owner::staticMetaObject.className() << "#" << this->m_owner.key() << "when already owned by #" <<
            item->ownerId();
      }

      if (this->m_owner.key() > 0) {
         qDebug() <<
            Q_FUNC_INFO << "Add" << Item::staticMetaObject.className() << "#" << item->key() << "to" <<
            Owner::staticMetaObject.className() << "#" << this->m_owner.key();
         item->setOwnerId(this->m_owner.key());
      }

      // Item needs to be in the DB for us to add it to the Owner
      if (item->key() < 0) {
         qDebug() <<
            Q_FUNC_INFO << "Inserting" << Item::staticMetaObject.className() << "in DB for" <<
            Owner::staticMetaObject.className() << "#" << this->m_owner.key() << "(" << item->ownerId() << ")";
         ObjectStoreWrapper::insert(item);
      }

      Q_ASSERT(item->key() > 0);

      //
      // If the Owner itself is not yet stored in the DB then it needs to hang on to its list of Items so that, when the
      // Owner does get stored, it can tell all the Items what their Owner ID is (see doSetKey()).
      //
      // (Conversely, if the Owner is in the DB, then we don't need to do anything further.  We can get all our Items
      // any time by just asking the relevant ObjectStore for all Items with Owner ID the same as ours.)
      //
      if (this->m_owner.key() < 0) {
         qDebug() <<
            Q_FUNC_INFO << "Adding" << Item::staticMetaObject.className() << "#" << item->key() << "to" <<
            Owner::staticMetaObject.className() << "#" << this->m_owner.key();
         //
         // See comment above in items() for why, even in an enumerated set, we just append to this list rather than
         // inserting the ID at its "correct" position.
         //
         this->m_itemIds.append(item->key());
      }

      // Now we added an item to the set, we need to listen for changes to it
      this->connectItemChangedSignal(item);

      // And, now we changed the size of the set, we have the owner tell people about it
      this->emitSetChanged();

      return item;
   }

public:
   /**
    * \brief For enumerated owned sets, inserts the supplied item at the specified position in the list.  If there is
    *        already an item in that position, it (and all subsequent ones) will be bumped one place down the list.
    *
    * \param item
    * \param seqNum counted from 1 (or 0 to append to the end of the list)
    */
   std::shared_ptr<Item> insert(std::shared_ptr<Item> item,
                                int seqNum) requires (IsEnumerated<ownedSetOptions>) {
      auto existingItems = this->items();

      // We'll treat any out of range sequence number as meaning "append to the end"
      if (seqNum < 1 || seqNum > existingItems.size() + 1) {
         seqNum = existingItems.size() + 1;
      }

      // Note per https://en.cppreference.com/w/cpp/ranges/drop_view that dropping more than the number of elements is
      // OK (and just gives an empty range.
      for (auto existingItem : std::ranges::drop_view(existingItems, seqNum - 1)) {
         // Don't want to emit a "changed" signal here, as we're still part-way through modifying the sequence
         existingItem->setSeqNum(existingItem->seqNum() + 1, false);
      }

      // Even here is a bit to early to emit a "changed" signal, as the item may need to be inserted in the DB.  The
      // extend member function call below will emit a "changed" signal for the whole set, which should be all we need.
      item->setSeqNum(seqNum, false);

      return this->extend(item);
   }

   /**
    * \brief Adds a new item (at the end of the current list if it's an enumerated owned set)
    */
   std::shared_ptr<Item> add(std::shared_ptr<Item> item) requires (!IsEnumerated<ownedSetOptions>) {
      return this->extend(item);
   }
   std::shared_ptr<Item> add(std::shared_ptr<Item> item) requires (IsEnumerated<ownedSetOptions>) {
      return this->insert(item, 0);
   }

   /**
    * \brief Remove the specified item from the set and delete it from the DB
    *
    * \return Pointer to the removed item (which caller now owns)
    */
   std::shared_ptr<Item> remove(std::shared_ptr<Item> item) {
      // It's a coding error if we try to remove an item that didn't belong to the owner
      Q_ASSERT(item->ownerId() == this->m_owner.key());

      // Disassociate the Item from its Owner
      item->setOwnerId(-1);

      // As per add(), if we're not yet stored in the database, then we also need to update our list of Items.
      if (this->m_owner.key() < 0) {
         int indexOfItem = this->m_itemIds.indexOf(item->key());
         if (indexOfItem < 0 ) {
            // This shouldn't happen, but it doesn't inherently break anything, so just log a warning and carry on
            qWarning() <<
               Q_FUNC_INFO << "Tried to remove" << Item::staticMetaObject.className() << "#" << item->key() <<
               " (from unsaved" << Owner::staticMetaObject.className() << "#" << this->m_owner.key() <<
               ") but couldn't find it";
         } else {
            this->m_itemIds.removeAt(indexOfItem);
         }
      }

      //
      // Since a Owner owns its Items, we need to remove the Item from the DB when we remove it from the Owner.  It then
      // makes sense (in the context of undo/redo) to put the Item object back into "new" state, which ObjectStoreTyped
      // will do for us.
      //
      ObjectStoreWrapper::hardDelete(item);

      //
      // Note that, in an enumerated set, this call to items() will also call this->normaliseSeqNums(), so item sequence
      // numbers will be adjusted/corrected for the fact that an item has been removed.
      //
      auto currentItems = this->items();

      // Now we changed the size of the set, have the owner tell people about it
      this->emitSetChanged(currentItems.size());

      return item;
   }

   /**
    * \brief Remove all items from the set and delete them from the DB
    */
   void removeAll() {
      auto items = this->items();
      qDebug() <<
         Q_FUNC_INFO << "Removing" << items.size() << Item::staticMetaObject.className() << "objects from" <<
         Owner::staticMetaObject.className() << "#" << this->m_owner.key();

      if (items.size() > 0) {
         for (auto item : items) {
            ObjectStoreWrapper::hardDelete(*item);
         }
         this->m_itemIds.clear();

         // Now we changed the size of the set, have the owner tell people about it
         this->emitSetChanged(0);
      }
      return;
   }

   /**
    * \brief Sets all the items (in the supplied order if this is an enumerated set)
    */
   void setAll(QList<std::shared_ptr<Item>> const & val) {
      this->removeAll();
      for (auto item : val) {
         this->add(item);
      }
      return;
   }

   /*!
    * \brief Swap the positions of Items \c lhs and \c rhs in an enumerated set
    */
   void swap(Item & lhs, Item & rhs) requires (IsEnumerated<ownedSetOptions>) {
      // It's a coding error if either of the items does not belong to this set
      Q_ASSERT(lhs.ownerId() == this->m_owner.key());
      Q_ASSERT(rhs.ownerId() == this->m_owner.key());

      // It's also a coding error if we're trying to swap a item with itself
      Q_ASSERT(lhs.key() != rhs.key());

      qDebug() <<
         Q_FUNC_INFO << "Swapping items" << lhs.seqNum() << "(#" << lhs.key() << ") and " << rhs.seqNum() << " (#" <<
         rhs.key() << ")";

      // Make sure we don't send notifications until the end (hence the false parameter on setSeqNum).
      int temp = lhs.seqNum();
      lhs.setSeqNum(rhs.seqNum(), false);
      rhs.setSeqNum(temp, false);

      //
      // If the owner hasn't yet been put in the DB then we also need to swap things in our local list of item IDs
      //
      if (this->m_owner.key() < 0) {
         int lhsIndex = this->m_itemIds.indexOf(lhs.key());
         int rhsIndex = this->m_itemIds.indexOf(rhs.key());
         // It's a coding error if we couldn't find either of the items
         Q_ASSERT(lhsIndex >= 0);
         Q_ASSERT(rhsIndex >= 0);

         this->m_itemIds.swapItemsAt(lhsIndex, rhsIndex);
      }

      this->emitSetChanged();
      return;
   }

   /**
    * \brief Needs to be called from \c Owner::setKey
    */
   void doSetKey(int key) {
      qDebug() <<
         Q_FUNC_INFO << "Setting" << Owner::staticMetaObject.className() << "#" << this->m_owner.key() << "key on" <<
         this->m_itemIds.size() << Item::staticMetaObject.className() << "objects";
      // Give our ID (key) to our Items
      for (auto itemId : this->m_itemIds) {
         if (!ObjectStoreWrapper::contains<Item>(itemId)) {
            // This is almost certainly a coding error, as each Item is owned by one Owner, but we can
            // (probably) recover by ignoring the missing Item.
            qCritical() <<
               Q_FUNC_INFO << "Unable to retrieve" << Item::staticMetaObject.className() << "#" << itemId <<
               "for" << Owner::staticMetaObject.className() << "#" << this->m_owner.key();
         } else {
            ObjectStoreWrapper::getById<Item>(itemId)->setOwnerId(key);
         }
      }
      return;
   }

   /**
    * \brief Needs to be called from Owner::hardDeleteOwnedEntities (which is virtual)
    */
   void doHardDeleteOwnedEntities() {
      // It's the Item that stores its Owner ID, so all we need to do is delete our Items then the subsequent database
      // delete of this Owner won't hit any foreign key problems.
      for (auto item : this->items()) {
         ObjectStoreWrapper::hardDelete<Item>(*item);
      }
      return;
   }

   /**
    * \brief For an enumerated set, returns the item at the specified position, if it exists, or \c nullptr if not
    *
    * \param seqNum counted from 1
    */
   std::shared_ptr<Item> itemAt(int const seqNum) const requires (IsEnumerated<ownedSetOptions>) {
      Q_ASSERT(seqNum > 0);
      auto items = this->items();
      if (items.size() >= seqNum) {
         return items[seqNum - 1];
      }
      return nullptr;
   }

   /**
    * \brief For an enumerated set, sets (or unsets) the item at the specified position.
    *
    *        Note this is different from \c insert(), as:
    *          - If there is a item in the specified position it will be overwritten rather than bumped down the list
    *          - Calling this with non-null value (ie not std::nullopt) for second and later items will ensure prior
    *            item(s) exist by creating default ones if necessary.
    *          - Calling this with a null value will delete any subsequent items.  (Doesn't make sense for third item to
    *            become second in the context of this function.)
    *
    * \param item The item to set, or \c nullptr to unset it
    * \param seqNum
    */
   void setAt(std::shared_ptr<Item> item, int const seqNum) {
      Q_ASSERT(seqNum > 0);
      auto items = this->items();
      if (items.size() >= seqNum) {
         // We already have a item of the number supplied, and possibly some subsequent ones

         if (item) {
            // This is an easy case: we're replacing an existing item.  This isn't the most efficient way to do things,
            // but it has the merit of being less code, and the absolute overhead of doing things this way is small,
            // because we are never dealing with huge sets (in fact not even more than a few dozen items).
            this->remove(items[seqNum - 1]);
            this->insert(item, seqNum);
            return;
         }

         // Caller supplied nullptr, so we're deleting this item and all the ones after it
         for (int seqNumToDelete = items.size(); seqNumToDelete >= seqNum; --seqNumToDelete) {
            this->remove(items[seqNumToDelete]);
         }
         return;
      }

      // There isn't a item of the number supplied
      if (!item) {
         // Nothing to do if caller supplied std::nullopt
         return;
      }

      // We have to ensure any prior items exist
      for (int seqNumToCreate = items.size(); seqNumToCreate < seqNum; ++seqNumToCreate) {
         this->insert(std::make_shared<Item>(), seqNumToCreate);
      }
      this->insert(item, seqNum);

      return;
   }

   /**
    * \brief Search the set with a lambda.
    *
    * \param matchFunction Takes a pointer to an object and returns \c true if the object is a match or \c false otherwise.
    *
    * \return Shared pointer to the first object that gives a \c true result to \c matchFunction, or \c nullptr if none
    *         does.
    */
   std::shared_ptr<Item> findFirstMatching(
      std::function<bool(std::shared_ptr<Item>)> const & matchFunction
   ) {
      auto items = this->items();

      auto result = std::find_if(items.cbegin(), items.cend(), matchFunction);
      if (result == items.cend()) {
         return nullptr;
      }
      return *result;
   }

   //! \brief Convenience function for logging
   template<class S>
   friend S & operator<<(S & stream, OwnedSet const & ownedSet) {
      QString output{};
      QTextStream outputAsStream{&output};
      auto const items = ownedSet.items();
      outputAsStream << "OwnedSet of " << ownedSet.m_owner << " with " << items.size() << " members: ";
      for (auto const & item : items) {
         outputAsStream << "|| " << *item << " || ";
      }
      stream << output;
      return stream;
   }

   //! \brief Convenience function for logging
   template<class S>
   friend S & operator<<(S & stream, OwnedSet const * ownedSet) {
      if (ownedSet) {
         stream << *ownedSet;
      } else {
         stream << "Null";
      }
      return stream;
   }

private:
   //================================================ MEMBER VARIABLES =================================================
   Owner & m_owner;
   //! Note that this list is not in any particular order (see comments in \c items member function)
   QVector<int> m_itemIds = {};
};

template<class Owner,
         class Item,
         BtStringConst const & propertyName,
         void (Owner::*itemChangedSlot)(QMetaProperty, QVariant),
         OwnedSetOptions ownedSetOptions>
TypeLookup const OwnedSet<Owner, Item, propertyName, itemChangedSlot, ownedSetOptions>::typeLookup {
   "OwnedSet",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::OwnedSet::items,
       TypeInfo::construct<MemberFunctionReturnType_t<&OwnedSet::items>>(
          PropertyNames::OwnedSet::items,
          OwnedSet::localisedName_items,
          TypeLookupOf<MemberFunctionReturnType_t<&OwnedSet::items>>::value
       )},
   },
   // Parent class lookup: none as we are at the top of this arm of the inheritance tree
   {}
};



#endif
