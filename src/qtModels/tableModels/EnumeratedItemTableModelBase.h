/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/StepTableModelBase.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
 =====================================================================================================================*/
#ifndef QTMODELS_TABLEMODELS_ENUMERATEDITEMTABLEMODELBASE_H
#define QTMODELS_TABLEMODELS_ENUMERATEDITEMTABLEMODELBASE_H
#pragma once

#include <memory>

#include <QDebug>

#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief Extra functions used by \c MashEnumeratedItemTableModel, \c BoilEnumeratedItemTableModel, \c FermentationEnumeratedItemTableModel
 *
 *        Classes inheriting from this one need to include the ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL macro in their header file
 *        and the ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE macro in their .cpp file.
 */
template<class Derived> class EnumeratedItemTableModelPhantom;
template<class Derived, class ItemClass, class OwnerClass>
class EnumeratedItemTableModelBase : public CuriouslyRecurringTemplateBase<EnumeratedItemTableModelPhantom, Derived> {

protected:
   EnumeratedItemTableModelBase() :
      m_itemOwnerObs{nullptr} {
      return;
   }
   ~EnumeratedItemTableModelBase() = default;

public:
   /**
    * \brief Set the item owner (eg Mash) whose items (eg MashEnumeratedItem objects) we want to model, or reload items
    *        from an existing item owner after they were changed.
    */
   void setEnumeratedItemOwner(std::shared_ptr<OwnerClass> itemOwner) {
      if (this->m_itemOwnerObs && this->derived().m_rows.size() > 0) {
         qDebug() <<
            Q_FUNC_INFO << "Removing" << this->derived().m_rows.size() << ItemClass::staticMetaObject.className() <<
            "rows for old" << OwnerClass::staticMetaObject.className() << "#" << this->m_itemOwnerObs->key();
         this->derived().beginRemoveRows(QModelIndex(), 0, this->derived().m_rows.size() - 1);

         for (auto item : this->derived().m_rows) {
            this->derived().disconnect(item.get(), nullptr, &this->derived(), nullptr);
         }
         this->derived().m_rows.clear();
         this->derived().endRemoveRows();
      }

      // Disconnect old signals if any were connected and we're changing item owner (eg Mash)
      if (this->m_itemOwnerObs && this->m_itemOwnerObs != itemOwner) {
         // Remove m_itemOwnerObs and all items.
         this->derived().disconnect(this->m_itemOwnerObs.get(), nullptr, &this->derived(), nullptr);
      }

      // Connect new signals, unless there is no new Mash/Boil/etc or we're not changing item owner (eg Mash)
      if (itemOwner && this->m_itemOwnerObs != itemOwner) {
         qDebug() <<
            Q_FUNC_INFO << "Connecting ownedItemsChanged signal from" << OwnerClass::staticMetaObject.className() << "#" <<
            itemOwner->key() << "to" << Derived::staticMetaObject.className();
         this->derived().connect(itemOwner.get(), &OwnerClass::ownedItemsChanged, &this->derived(), &Derived::itemOwnerChanged);
      }

      this->m_itemOwnerObs = itemOwner;
      if (this->m_itemOwnerObs) {
         qDebug() <<
            Q_FUNC_INFO << "Now watching" << OwnerClass::staticMetaObject.className() << "#" <<
            this->m_itemOwnerObs->key();

         auto tmpEnumeratedItems = this->m_itemOwnerObs->ownedItems();
         if (tmpEnumeratedItems.size() > 0) {
            qDebug() <<
               Q_FUNC_INFO << "Inserting" << tmpEnumeratedItems.size() << " " << ItemClass::staticMetaObject.className() <<
               "rows";
            this->derived().beginInsertRows(QModelIndex(), 0, tmpEnumeratedItems.size() - 1);
            this->derived().m_rows = tmpEnumeratedItems;
            for (auto item : this->derived().m_rows) {
               this->derived().connect(item.get(), &NamedEntity::changed, &this->derived(), &Derived::itemChanged);
            }
            this->derived().endInsertRows();
         }
      }

      if (this->derived().m_parentTableWidget) {
         this->derived().m_parentTableWidget->resizeColumnsToContents();
         this->derived().m_parentTableWidget->resizeRowsToContents();
      }
      return;

   }

   std::shared_ptr<OwnerClass> getEnumeratedItemOwner() const {
      return this->m_itemOwnerObs;
   }

protected:
   //! \returns true if \c item is successfully found and removed.
   bool doRemoveItem(std::shared_ptr<ItemClass> item) {
      int ii {static_cast<int>(this->derived().m_rows.indexOf(item))};
      if (ii >= 0) {
         qDebug() <<
            Q_FUNC_INFO << "Removing" << ItemClass::staticMetaObject.className() << item->name() << "(#" <<
            item->key() << ")";
         this->derived().beginRemoveRows(QModelIndex(), ii, ii);
         this->derived().disconnect(item.get(), nullptr, &this->derived(), nullptr);
         this->derived().m_rows.removeAt(ii);
         //reset(); // Tell everybody the table has changed.
         this->derived().endRemoveRows();

         return true;
      }

      return false;
   }

private:
   void reorderEnumeratedItem(std::shared_ptr<ItemClass> item, int current) {
      // doSomething will be -1 if we are moving up and 1 if we are moving down
      // and 0 if nothing is to be done (see next comment)
      int destChild   = item->sequenceNumber();
      int doSomething = destChild - current - 1;

      qDebug() << Q_FUNC_INFO << "Swapping" << destChild << "with" << current << ", so doSomething=" << doSomething;

      //
      // Moving a item up or down generates two signals, one for each row impacted. If we move row B above row A:
      //    1. The first signal is to move B above A, which will result in A being below B
      //    2. The second signal is to move A below B, which we just did.
      // Therefore, the second signal mostly needs to be ignored. In those circumstances, A->sequenceNumber() will be the
      // same as its position in the items list, modulo some indexing.
      //
      if (doSomething == 0) {
         return;
      }

      // beginMoveRows is a little odd. When moving rows within the same parent,
      // destChild points one beyond where you want to insert the row. Think of
      // it as saying "insert before destChild". If we are moving something up,
      // we need to be one less than sequenceNumber. If we are moving down, it just
      // works.
      if (doSomething < 0) {
         destChild--;
      }

      // We assert that we are swapping valid locations on the list as, to do otherwise implies a coding error
      qDebug() <<
         Q_FUNC_INFO << "Swap" << current + doSomething << "with" << current << ", in list of " <<
         this->derived().m_rows.size();
      Q_ASSERT(current >= 0);
      Q_ASSERT(current + doSomething >= 0);
      Q_ASSERT(current < this->derived().m_rows.size());
      Q_ASSERT(current + doSomething < this->derived().m_rows.size());

      this->derived().beginMoveRows(QModelIndex(), current, current, QModelIndex(), destChild);

      // doSomething is -1 if moving up and 1 if moving down. swap current with
      // current -1 when moving up, and swap current with current+1 when moving
      // down
      this->derived().m_rows.swapItemsAt(current, current + doSomething);
      this->derived().endMoveRows();
      return;
   }

protected:
   void doMoveItemUp(int itemNum) {
      if (!this->m_itemOwnerObs || itemNum == 0 || itemNum >= this->derived().m_rows.size()) {
         return;
      }

      this->m_itemOwnerObs->ownedSet().swap(*this->derived().m_rows[itemNum], *this->derived().m_rows[itemNum - 1]);
      return;
   }

   void doMoveItemDown(int itemNum) {
      if (!this->m_itemOwnerObs || itemNum + 1 >= this->derived().m_rows.size()) {
         return;
      }

      this->m_itemOwnerObs->ownedSet().swap(*this->derived().m_rows[itemNum], *this->derived().m_rows[itemNum + 1]);
      return;
   }

   void doEnumeratedItemOwnerChanged() {
      // An item was added, removed or changed order.  Remove and re-add all items.
      qDebug() << Q_FUNC_INFO << "Re-reading" << ItemClass::staticMetaObject.className() << "items for" << this->m_itemOwnerObs;
      this->setEnumeratedItemOwner(this->m_itemOwnerObs);
      return;
   }

   void doEnumeratedItemChanged(QMetaProperty prop, [[maybe_unused]] QVariant val) {
      qDebug() << Q_FUNC_INFO << prop.name();

      ItemClass * itemSender = qobject_cast<ItemClass *>(this->derived().sender());
      if (itemSender) {
         if (itemSender->ownerId() != this->m_itemOwnerObs->key()) {
            // It really shouldn't happen that we get a notification for a item (eg MashEnumeratedItem) that's not in the item
            // owner (eg Mash) we're watching, but, if we do, then stop trying to process the update.
            qCritical() <<
               Q_FUNC_INFO << "Instance @" << static_cast<void *>(this) << "received update for" <<
               ItemClass::staticMetaObject.className() << "#" << itemSender->key() << "of" <<
               OwnerClass::staticMetaObject.className() << "#" << itemSender->ownerId() << "but we are watching" <<
               OwnerClass::staticMetaObject.className() << "#" << this->m_itemOwnerObs->key();
            return;
         }

         int ii = this->derived().findIndexOf(itemSender);
         if (ii >= 0) {
            if (prop.name() == PropertyNames::EnumeratedBase::sequenceNumber) {
//               qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
               this->reorderEnumeratedItem(this->derived().m_rows.at(ii), ii);
            }

            emit this->derived().dataChanged(
               this->derived().QAbstractItemModel::createIndex(ii, 0),
               this->derived().QAbstractItemModel::createIndex(ii, this->derived().columnCount() - 1)
            );
         }

      }

      if (this->derived().m_parentTableWidget) {
         this->derived().m_parentTableWidget->resizeColumnsToContents();
         this->derived().m_parentTableWidget->resizeRowsToContents();
      }
      return;
   }

   //================================================ Member Variables =================================================
   std::shared_ptr<OwnerClass> m_itemOwnerObs;
};

/**
 * \brief Derived classes should include this in their header file, right after TABLE_MODEL_COMMON_DECL
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(ItemName, OwnerName)                               \
   /* This allows EnumeratedItemTableModelBase to call protected and private members of Derived */ \
   friend EnumeratedItemTableModelBase<ItemName##TableModel, ItemName, OwnerName>;                 \
                                                                                                   \
   public:                                                                                         \
      void set##OwnerName(std::shared_ptr<OwnerName> itemOwner);                                   \
      std::shared_ptr<OwnerName> get##OwnerName() const;                                           \
      bool removeItem(std::shared_ptr<ItemName> item);                                             \
                                                                                                   \
   public slots:                                                                                   \
      void moveItemUp(int itemNum);                                                                \
      void moveItemDown(int itemNum);                                                              \
      void itemOwnerChanged();                                                                     \
      void itemChanged(QMetaProperty prop, QVariant val);                                          \


/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(ItemName, OwnerName)                  \
   void ItemName##TableModel::set##OwnerName(std::shared_ptr<OwnerName> itemOwner) {  \
      this->setEnumeratedItemOwner(itemOwner);                                        \
      return;                                                                         \
   }                                                                                  \
   std::shared_ptr<OwnerName> ItemName##TableModel::get##OwnerName() const {          \
      return this->getEnumeratedItemOwner();                                          \
   }                                                                                  \
   bool ItemName##TableModel::removeItem(std::shared_ptr<ItemName> item) {            \
      return this->doRemoveItem(item);                                                \
   }                                                                                  \
   void ItemName##TableModel::moveItemUp(int itemNum) {                               \
      this->doMoveItemUp(itemNum);                                                    \
      return;                                                                         \
   }                                                                                  \
   void ItemName##TableModel::moveItemDown(int itemNum) {                             \
      this->doMoveItemDown(itemNum);                                                  \
      return;                                                                         \
   }                                                                                  \
   void ItemName##TableModel::itemOwnerChanged() {                                    \
      this->doEnumeratedItemOwnerChanged();                                           \
      return;                                                                         \
   }                                                                                  \
   void ItemName##TableModel::itemChanged(QMetaProperty prop, QVariant val) {         \
      this->doEnumeratedItemChanged(prop, val);                                       \
      return;                                                                         \
   }                                                                                  \

#endif
