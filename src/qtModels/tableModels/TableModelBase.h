/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/TableModelBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef TABLEMODELS_TABLEMODELBASE_H
#define TABLEMODELS_TABLEMODELBASE_H
#pragma once

#include <type_traits>
#include <utility> // For std::pair
#include <vector>

#include <QList>
#include <QModelIndex>

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "undoRedo/Undoable.h"
#include "measurement/Measurement.h"
#include "model/IngredientAmount.h"
#include "model/StockPurchase.h"
#include "model/Recipe.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "undoRedo/UndoableAddOrRemove.h"
#include "utils/ColumnOwnerTraits.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/MetaTypes.h"
#include "utils/PropertyHelper.h"
#include "utils/TypeTraits.h"

// TODO: We would like to change "Add to Recipe" to "Set for Recipe" for things where the recipe only has one of them, eg Style or Equipment

class Style;

//
// Using concepts allows us to tailor the templated TableModelBase class without having the same inheritance
// structure as BtTableModel / BtTableModelRecipeObserver.
//
// Note that concepts have some limitations:
//    - Concepts cannot recursively refer to themselves and cannot be constrained, is you cannot define one concept in
//      terms of another
//    - You cannot have explicit instantiations, explicit specializations, or partial specializations of concepts
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename T> concept CONCEPT_FIX_UP IsTableModel         = std::is_base_of_v<BtTableModel, T>;
template <typename T> concept CONCEPT_FIX_UP ObservesRecipe       = std::is_base_of_v<BtTableModelRecipeObserver, T>;
template <typename T> concept CONCEPT_FIX_UP DoesNotObserveRecipe = std::negation_v<std::is_base_of<BtTableModelRecipeObserver, T>>;

//
// NOTE: At several places below in TableModelBase, we would like to have two versions of a member function depending on
//       some property of Derived - eg using `requires HasSomeAttribute<Derived>`.  Although GCC lets you do this, other
//       compilers (eg Clang) do not and give an error along the lines of "incomplete type ... used in type trait
//       expression ... definition of ...  is not complete until the closing '}'".  This is annoying but correct.  The
//       C++ standard rules say we are not allowed to use `HasSomeAttribute<Derived>` until `Derived` is fully defined,
//       which won't be until the closing brace of its class definition (and the `TableModelBase` template is being
//       instantiated before that because `Derived` inherits from it).
//
//       The way round this is to template the member function so that the evaluation of the constraint is deferred
//       until after the class declaration of `Derived` is complete.  Now, in, eg, HopTableModel we can call the right
//       version via:
//          this->updateStockPurchase<HopTableModel>(...);
//
//       (I did also try `this->updateStockPurchase<decltype(*this)>(...)`, but I couldn't get it to work.)
//
//       But what if we want to call one of these templated functions from inside TableModelBase?  Well, then we have to
//       call down to a wrapper function in Derived that knows which version of the TableModelBase function to call.  Eg
//       if we want a function that returns "pointer to the observed recipe or null if this class does not observe a
//       recipe" then, in TableModelBase we have two "doer" member functions:
//          template<class Caller> Recipe * doGetObservedRecipe() requires ObservesRecipe<Caller> {...} Substantive
//          template<class Caller> Recipe * doGetObservedRecipe() requires DoesNotObserveRecipe<Caller> {...} No-op
//       And in each of the Derived classes, such as HopTableModel, we have a (macro-inserted) wrapper function such as:
//          void HopTableModel::getObservedRecipe() { return this->doGetObservedRecipe<NeName##TableModel>(); }
//       Then, from TableModel, we call this->derived().getObservedRecipe() and the correct version of
//       doGetObservedRecipe() ends up being called.
//
//       NOTE: The IsTableModel constraint is useful as a belt-and-braces to make sure you're passing in a useful
//             template parameter (eg `HopTableModel`, not `HopTableModel *` or `TableModelBase<HopTableModel, Hop>`).
//             Nonetheless, if you change the calls to these functions (or add new ones), logging statements are the
//             best way to be absolutely sure the right version of each is being called.
//
//       Essentially, every time you see `template<class Caller>` below, something along the lines described above is
//       what is going on.
//

/**
 * This is to allow us to detect at compile time (via \c HAS_MEMBER) whether a table model has a Name column
 */
CREATE_HAS_MEMBER(Name);
CREATE_HAS_MEMBER(TotalInventory);
CREATE_HAS_MEMBER(PctAcid);

/**
 * \brief See comment in qtModels/tableModels/BtTableModel.h for more info on inheritance structure
 *
 *        Classes inheriting from this one need to include the TABLE_MODEL_COMMON_DECL macro in their header file and
 *        the TABLE_MODEL_COMMON_CODE macro in their .cpp file.
 *
 *        Subclasses also need to declare and implement the following functions (with the obvious substitutions for NS):
 *           void added  (std::shared_ptr<NE> item);  // Updates any global info as a result of item being added
 *           void removed(std::shared_ptr<NE> item);  // Updates any global info as a result of item being removed
 *           void updateTotals();                     // Updates any global info, eg as a result of an item changed or
 *                                                    // all items being removed.  (Better in latter case than repeated
 *                                                    // calls to removed() because avoids rounding errors on running
 *                                                    // totals.)
 *
 *        Note that we use ColumnOwnerTraits as the phantom class for CuriouslyRecurringTemplateBase since it fits the
 *        bill.
 */
template<class Derived, class NE>
class TableModelBase : public CuriouslyRecurringTemplateBase<ColumnOwnerTraits, Derived> {
public:
   //
   // We want, eg, \c HopTableModel to inherit from \c TableModelBase<HopTableModel, Hop> and to have its own enum
   // \c HopTableModel::ColumnIndex.  But we'd also like \c HopTableModel::ColumnIndex to be accessible from within
   // \c TableModelBase, which normally isn't possible, eg as explained at
   // https://stackoverflow.com/questions/5534759/c-with-crtp-class-defined-in-the-derived-class-is-not-accessible-in-the-base
   // However, per the same link, the way around this is to use a traits class.  This is another "trick" where we
   // declare a template for the "traits" class before the base class of the curiously recurring template pattern
   // (CRTP), but then specialise that "traits" class in the derived class.
   //
   // This gets round the fact that we would not be able to access Derived::ColumnIndex directly
   //
   // In theory, in C++20, we don't need the `typename` here, but, per comment in ColumnInfo, we need to
   // retain it until our Mac build environment is using a more recent version of Clang.
   //
   using ColumnIndex = typename ColumnIndexHolder<Derived>::ColumnIndex;

protected:
   TableModelBase() : m_rows{} {
      return;
   }
   // Need a virtual destructor as we have a virtual member function
   virtual ~TableModelBase() = default;

public:
   /**
    * \brief Casting wrapper for \c BtTableModel::getColumnInfo
    *
    *        Note that, without additional `using` declarations in the derived class we cannot simply call this
    *        \c getColumnInfo as we'd then be trying to have two unconnected base classes participate in the name
    *        resolution (as explained at
    *        https://stackoverflow.com/questions/51690394/overloading-member-function-among-multiple-base-classes).
    */
   ColumnInfo const & get_ColumnInfo(ColumnIndex const columnIndex) const {
      return ColumnOwnerTraits<Derived>::getColumnInfo(static_cast<size_t>(columnIndex));
   }

   /**
    * \brief Overload for \c get_ColumnInfo
    */
   ColumnInfo const & get_ColumnInfo(QModelIndex const & index) const {
      auto const columnIndex = static_cast<ColumnIndex>(index.column());
      return this->get_ColumnInfo(columnIndex);
   }

   /**
    * \brief Observe a recipe's list of NE (hops, fermentables, etc).  Mostly called from Derived::observeRecipe.
    */
   template<class Caller>
   void doObserveRecipe(Recipe * rec) requires IsTableModel<Caller> && ObservesRecipe<Caller> {
      if (this->derived().recObs) {
         qDebug() <<
            Q_FUNC_INFO << "Unobserve Recipe #" << this->derived().recObs->key() << "(" <<
            this->derived().recObs->name() << ")";
         this->derived().disconnect(this->derived().recObs, nullptr, &this->derived(), nullptr);
         this->removeAll();
      }

      this->derived().recObs = rec;
      if (this->derived().recObs) {
         qDebug() <<
            Q_FUNC_INFO << "Observe Recipe #" << this->derived().recObs->key() << "(" <<
            this->derived().recObs->name() << ")";
         this->derived().connect(this->derived().recObs, &NamedEntity::changed, &this->derived(), &Derived::changed);

         // TBD: Commented out version doesn't compile on GCC
         // this->addItems(this->derived().recObs->allOwned<NE>());
         this->addItems(rec->allOwned<NE>());
      }
      qDebug() << Q_FUNC_INFO << "Now have" << this->m_rows.size() << "rows";
      return;
   }
   template<class Caller>
   void doObserveRecipe([[maybe_unused]] Recipe * rec) requires IsTableModel<Caller> && DoesNotObserveRecipe<Caller> {
      qDebug() << Q_FUNC_INFO << "No-op version";
      // No-op version
      return;
   }

   /**
    * \brief If true, we model the database's list of NE (hops, fermentables. etc).
    */
   void observeDatabase(bool val) {
      if (val) {
         // Observing a database and a recipe are mutually exclusive.
         this->derived().observeRecipe(nullptr);
         this->removeAll();
         this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectInserted, &this->derived(), &Derived::addItem);
         this->derived().connect(&ObjectStoreTyped<NE>::getInstance(), &ObjectStoreTyped<NE>::signalObjectDeleted , &this->derived(), &Derived::removeItem);
         this->addItems(ObjectStoreWrapper::getAll<NE>());
      } else {
         this->derived().disconnect(&ObjectStoreTyped<NE>::getInstance(), nullptr, &this->derived(), nullptr);
         this->removeAll();
      }
      return;

   }

   /**
    * \brief Return the \c i-th row in the model.
    *        Returns \c nullptr on failure.
    */
   std::shared_ptr<NE> getRow(int ii) const {
      if (!(this->m_rows.isEmpty())) {
         if (ii >= 0 && ii < this->m_rows.size()) {
            return this->m_rows[ii];
         }
         qWarning() << Q_FUNC_INFO << "index out of range (" << ii << "/" << this->m_rows.size() << ")";
      } else {
         qWarning() << Q_FUNC_INFO << "this->m_rows is empty (" << ii << "/" << this->m_rows.size() << ")";
      }
      return nullptr;
   }

   /**
    * \brief Remove duplicates and deleted items from the supplied list
    */
   QList< std::shared_ptr<NE> > removeDuplicates(QList< std::shared_ptr<NE> > items,
                                                 Recipe const * recipe = nullptr) {
      decltype(items) tmp;

      for (auto ii : items) {
         if (!recipe && ii->deleted()) {
            continue;
         }
         if (!this->m_rows.contains(ii) ) {
            tmp.append(ii);
         }
      }
      return tmp;
   }

   /**
    * \brief Given a raw pointer, find the index of the corresponding shared pointer in \c this->m_rows
    *
    *        This is useful because the Qt signals and slots framework allows the slot receiving a signal to get a raw
    *        pointer to the object that sent the signal, and we often want to find the corresponding shared pointer in
    *        our list.
    *
    *        Note that using this function is a lot safer than, say, calling ObjectStoreWrapper::getSharedFromRaw(), as
    *        that only works for objects that are already stored in the database, something which is not guaranteed to
    *        be the case with our rows.  (Eg in SaltTableModel, new Salts are only stored in the DB when the window is
    *        closed with OK.)
    *
    *        Function name is for consistency with \c QList::indexOf
    *
    * \param object  what to search for
    * \return index of object in this->m_rows or -1 if it's not found
    */
   int findIndexOf(NE const * object) const {
      for (int index = 0; index < this->m_rows.size(); ++index) {
         if (this->m_rows.at(index).get() == object) {
            return index;
         }
      }
      return -1;
   }

   void add(std::shared_ptr<NE> item) {
      qDebug() << Q_FUNC_INFO << item->name();

      // Check to see if it's already in the list
      if (this->m_rows.contains(item)) {
         return;
      }

      // If we are observing the database, ensure that the item is not deleted.
      Recipe * observedRecipe = this->derived().getObservedRecipe();
      if (!observedRecipe && item->deleted()) {
         return;
      }

      int size = this->m_rows.size();
      this->derived().beginInsertRows(QModelIndex(), size, size);
      this->m_rows.append(item);
      this->derived().connect(item.get(), &NamedEntity::changed, &this->derived(), &Derived::changed);
      this->derived().added(item);
      //reset(); // Tell everybody that the table has changed.
      this->derived().endInsertRows();
      return;
   }

   void addById(int itemId) {
      auto itemToAdd = ObjectStoreWrapper::getById<NE>(itemId);
      if (!itemToAdd) {
         // Not sure this should ever happen in practice, but, if there ever is no item with the
         // specified ID, there's not a lot we can do.
         qWarning() <<
            Q_FUNC_INFO << "Received signal that" << NE::staticMetaObject.className() <<  "ID" <<
            itemId << "added, but unable to retrieve the" << NE::staticMetaObject.className();
         return;
      }
      this->add(itemToAdd);
      return;
   }

   //! \returns true if \c item is successfully found and removed.
   bool remove(std::shared_ptr<NE> item) {
      int rowNum = this->m_rows.indexOf(item);
      if (rowNum >= 0)  {
         this->derived().beginRemoveRows(QModelIndex(), rowNum, rowNum);
         this->derived().disconnect(item.get(), nullptr, &this->derived(), nullptr);
         this->m_rows.removeAt(rowNum);

         this->derived().removed(item);

         //reset(); // Tell everybody the table has changed.
         this->derived().endRemoveRows();

         if (this->derived().m_parentTableWidget) {
            this->derived().m_parentTableWidget->resizeColumnsToContents();
            this->derived().m_parentTableWidget->resizeRowsToContents();
         }

         return true;
      }

      return false;
   }

   /**
    * \brief Currently only used in WaterDialog I think
    */
   bool remove(QModelIndex const & index) {
      if (!this->indexOk(index)) {
         return false;
      }

      return this->remove(this->m_rows[index.row()]);
   }

   /**
    * \brief Use this for removing \c RecipeAdditionHop etc
    */
   template<class Proxy>
   void removeSelectedIngredients(QTableView & tableView, Proxy & proxy) {
      QModelIndexList selected = tableView.selectionModel()->selectedIndexes();
      QList<std::shared_ptr<NE>> itemsToRemove;

      int size = selected.size();
      if (size == 0) {
         return;
      }

      for (int ii = 0; ii < size; ii++) {
         QModelIndex viewIndex = selected.at(ii);
         QModelIndex modelIndex = proxy.mapToSource(viewIndex);
         itemsToRemove.append(this->getRow(modelIndex.row()));
      }

      for (auto item : itemsToRemove) {
         Undoable::doOrRedoUpdate(
            newUndoableAddOrRemove(*this->derived().recObs,
                                    &Recipe::removeAddition<NE>,
                                    item,
                                    &Recipe::addAddition<NE>,
                                    Recipe::tr("Remove %1 from recipe").arg(NE::localisedName()))
         );
         this->remove(item);
      }
      return;
   }

   /**
    * \brief Called from SortFilterProxyModelBase::doLessThan
    */
   bool isLessThan(QModelIndex const & leftIndex, QModelIndex const & rightIndex) const {
      //
      // Strictly, we should call Derived::data() here, which mostly ends up calling readDataFromModel(), but with the
      // possibility of class-specific logic (eg MashStepTableModel shows "---" for the temperature field when the mash
      // step type is a decoction).  However, the extra class-specific logic is not needed for sorting, so it's simpler
      // not to worry about adding subclass handling for Qt::UserRole, and just call readDataFromModel() direct.
      //
      QVariant  leftItem = this->readDataFromModel( leftIndex, Qt::UserRole);
      QVariant rightItem = this->readDataFromModel(rightIndex, Qt::UserRole);

      ColumnInfo const & columnInfo = this->get_ColumnInfo(leftIndex);
      return PropertyHelper::isLessThan(leftItem, rightItem, columnInfo.typeInfo);
   }

protected:

   template<class Caller>
   Recipe * doGetObservedRecipe() const requires IsTableModel<Caller> && ObservesRecipe<Caller> {
      qDebug() << Q_FUNC_INFO << "Substantive version";
      return this->derived().recObs;
   }
   template<class Caller>
   Recipe * doGetObservedRecipe() const requires IsTableModel<Caller> && DoesNotObserveRecipe<Caller> {
      qDebug() << Q_FUNC_INFO << "No-op version";
      return nullptr;
   }

   /**
    * \brief Watch all the \c NE for changes.
    */
   void addItems(QList< std::shared_ptr<NE> > items) {
      qDebug() <<
         Q_FUNC_INFO << "Add up to " << items.size() << "of" << NE::staticMetaObject.className() <<
         "to existing list of" << this->m_rows.size();

      auto tmp = this->removeDuplicates(items, this->derived().getObservedRecipe());

      qDebug() << Q_FUNC_INFO << "After de-duping, adding " << tmp.size() << "of" << NE::staticMetaObject.className();

      int size = this->m_rows.size();
      if (size + tmp.size()) {
         this->derived().beginInsertRows(QModelIndex(), size, size + tmp.size() - 1);

         this->m_rows.append(tmp);

         for (auto item : tmp) {
            this->derived().connect(item.get(), &NamedEntity::changed, &this->derived(), &Derived::changed);
            this->derived().added(item);
         }

         this->derived().endInsertRows();
      }
      return;
   }

   /**
    * \brief Clear the model.
    */
   void removeAll() {
      int const size = this->m_rows.size();
      if (size > 0) {
         this->derived().beginRemoveRows(QModelIndex(), 0, size - 1);
         while (!this->m_rows.empty()) {
            auto item = this->m_rows.takeLast();
            this->derived().disconnect(item.get(), nullptr, &this->derived(), nullptr);
            //this->derived().removed(item); // Shouldn't be necessary as we call updateTotals() below
         }
         this->derived().endRemoveRows();
         this->derived().updateTotals();
      }
      return;
   }

   virtual std::shared_ptr<NamedEntity> getRowAsNamedEntity(int ii) {
      return std::static_pointer_cast<NamedEntity>(this->getRow(ii));
   }

   /**
    * \brief Check that supplied index is within bounds.
    */
   bool indexOk(QModelIndex const & index) const {
      if (index.row() >= static_cast<int>(this->m_rows.size())) {
         qCritical() << Q_FUNC_INFO << "Bad model index. row = " << index.row() << "; max row = " << this->m_rows.size();
         return false;
      }

      auto row = this->m_rows[index.row()];
      if (!row) {
         // This is almost certainly a coding error
         qCritical() << Q_FUNC_INFO << "Null pointer at row" << index.row() << "of" << this->m_rows.size();
         return false;
      }

      return true;
   }

   /**
    * \brief Check that supplied index is within bounds, and that the role is one for which we would normally want to
    *        return data.
    *
    *        Per https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum, there are a dozen or so different "roles" that we can
    *        get called for, mostly from the Qt framework itself.  If we don't have anything special to say for a
    *        particular role, eg if we don't want to return a custom QFont when requested with Qt::FontRole, then
    *        https://doc.qt.io/qt-6/qabstractitemmodel.html#data says we just need to return "an invalid (default-
    *        constructed) QVariant".
    *
    *        Note that if we do not send data back for the edit role, then double-clicking a cell to edit it will blank
    *        the contents, which can be rather annoying. See
    *        https://stackoverflow.com/questions/55855284/qabstracttablemodel-editing-without-clearing-previous-data-in-cell.)
    */
   bool indexAndRoleOk(QModelIndex const & index, int const role) const {
      if (!this->indexOk(index)) {
         // indexOk() will already have logged an error
         return false;
      }

      if (role != Qt::DisplayRole &&
          role != Qt::EditRole &&
          role != Qt::TextAlignmentRole) {
         // No need to log anything here, as it's perfectly normal to get called with other roles
         return false;
      }

      return true;
   }

   /**
    * \brief Child classes should call this from their \c data() member function (overriding
    *        \c QAbstractTableModel::data()) to read data for any column that does not require special handling.
    *
    *        Caller is expected to have called \c indexAndRoleOk before calling this function.
    *
    * \param index
    * \param role  This will be a \c Qt::ItemDataRole value.  See \c PropertyHelper::readDataFromPropertyValue.
    *
    * NOTE: The debug logging in this function is commented out because the function gets called A LOT.  I tend to
    *       uncomment these lines only when working on a problem in this area of the code, otherwise the log file fills
    *       up too quickly!
    */
   QVariant readDataFromModel(QModelIndex const & index, int const role) const {
      //
      // We assume we are always being called from the Derived::data() member function (eg HopTableModel::data(), etc).
      // Often the call stack is along the following lines (albeit with some of the functions optimised away in
      // reality):
      //    HopTableModel::data()
      //    QSortFilterProxyModel::data()
      //    ItemDelegate<HopItemDelegate, HopTableModel>::readDataFromModel()
      //    HopItemDelegate::setEditorData()
      //    QAbstractItemView::edit()
      //
      auto row = this->m_rows[index.row()];
      ColumnInfo const & columnInfo = this->get_ColumnInfo(index);
      TypeInfo const & typeInfo = columnInfo.typeInfo;
      if (role == Qt::TextAlignmentRole) {
         return PropertyHelper::getAlignment(typeInfo);
      }

      QVariant modelData = columnInfo.propertyPath.getValue(*row);
      if (!modelData.isValid()) {
         // It's a programming error if we couldn't read a property modelData
         qCritical() <<
            Q_FUNC_INFO << "Unable to read" << row->metaObject()->className() << "#" << row->key() << "property" <<
            columnInfo.propertyPath << "(Got" << modelData << ")";
         Q_ASSERT(false); // Stop here on debug builds
      }

      // Uncomment this log statement if asserts in PropertyHelper::readDataFromPropertyValue are firing
//      qDebug() <<
//         Q_FUNC_INFO << columnInfo.columnFqName << ", propertyPath:" << columnInfo.propertyPath << "TypeInfo:" <<
//         typeInfo << ", modelData:" << modelData;

      return PropertyHelper::readDataFromPropertyValue(modelData,
                                                       typeInfo,
                                                       role,
                                                       columnInfo.extras.has_value(),
                                                       columnInfo.getForcedSystemOfMeasurement(),
                                                       columnInfo.getForcedRelativeScale());

   }

   /**
    * \brief Child classes should call this from their \c setData() member function (overriding
    *        \c QAbstractTableModel::setData()) to write data for any column that does not require special handling
    *
    *        Caller is expected to have called \c indexAndRoleOk before calling this function.
    *
    * \param physicalQuantity Needs to be supplied if and only if the column type is
    *                         \c Measurement::MixedPhysicalQuantities
    *
    * \return \c true if successful, \c false otherwise
    */
   bool writeDataToModel(QModelIndex const & index,
                         QVariant const & value,
                         int const role,
                         std::optional<Measurement::PhysicalQuantity> physicalQuantity = std::nullopt) const {
      //
      // This gets called from a stack along the following lines:
      //
      //    RecipeAdditionHopTableModel::setData()
      //    QSortFilterProxyModel::setData()
      //    ItemDelegate<HopItemDelegate, HopTableModel>::writeDataToModel()
      //    HopItemDelegate::setModelData()
      //    QAbstractItemView::commitData()
      //
      if (role != Qt::EditRole) {
//         qCritical().noquote() << Q_FUNC_INFO << "Unexpected role: " << role << Logging::getStackTrace();
         return false;
      }
      auto row = this->m_rows[index.row()];
      auto const columnIndex = static_cast<ColumnIndex>(index.column());
      auto const & columnInfo = this->get_ColumnInfo(columnIndex);

      TypeInfo const & typeInfo = columnInfo.typeInfo;

      return PropertyHelper::writeDataToProperty(*row,
                                                 columnInfo.propertyPath,
                                                 typeInfo,
                                                 value,
                                                 columnInfo.extras,
                                                 columnInfo.getForcedSystemOfMeasurement(),
                                                 columnInfo.getForcedRelativeScale(),
                                                 physicalQuantity);
   }

   template<class TM>
   void updateStockPurchase(int invKey,
                            BtStringConst const & propertyName) requires IsTableModel<TM> && CanHaveStockPurchase<NE> {
      // Substantive version
      if (propertyName == PropertyNames::IngredientAmount::amount) {
         auto stockPurchase = ObjectStoreWrapper::getById<typename NE::StockPurchaseClass>(invKey);
         for (int ii = 0; ii < this->m_rows.size(); ++ii) {
            std::shared_ptr<NE> ingredient = this->m_rows.at(ii);
            if (ingredient->key() == stockPurchase->ingredientId()) {
               emit this->derived().dataChanged(
                  this->derived().createIndex(ii, static_cast<int>(Derived::ColumnIndex::TotalInventory)),
                  this->derived().createIndex(ii, static_cast<int>(Derived::ColumnIndex::TotalInventory))
               );
            }
         }
      }
      return;
   }
   template<class TM>
   void updateStockPurchase([[maybe_unused]] int invKey,
                        [[maybe_unused]] BtStringConst const & propertyName) requires IsTableModel<TM> &&
                                                                                      CannotHaveStockPurchase<NE> {
      // No-op version
      return;
   }

   template<class Caller>
   void checkRecipeItems(Recipe * recipe) requires IsTableModel<Caller> && ObservesRecipe<Caller>{
      qDebug() << Q_FUNC_INFO;
      if (recipe == this->derived().recObs) {
         this->removeAll();
         // TBD: Commented out version doesn't compile on GCC
         // this->addItems(this->derived().recObs->allOwned<NE>());
         this->addItems(recipe->allOwned<NE>());
         if (this->derived().rowCount() > 0) {
            emit this->derived().headerDataChanged(Qt::Vertical, 0, this->derived().rowCount() - 1);
         }
      }
      return;
   }
   template<class Caller>
   void checkRecipeItems([[maybe_unused]] Recipe * recipe) requires IsTableModel<Caller> && DoesNotObserveRecipe<Caller> {
      qDebug() << Q_FUNC_INFO;
      // No-op version
      return;
   }

   /**
    * \brief Called from \c Derived::changed slot
    *
    * \param propNameOfOurAdditionsInRecipe  This needs to be something valid in all cases, but is only used if Derived
    *                                        is a recipe observer.
    */
   template<class Caller>
   void propertyChanged(QMetaProperty prop,
                        [[maybe_unused]] QVariant val,
                        BtStringConst const & propNameOfOurAdditionsInRecipe) {
      QObject * rawSender = this->derived().sender();
      QString senderClassName{"Null"};
      if (rawSender) {
         senderClassName = rawSender->metaObject()->className();
      }
      // Normally leave this logging statement commented out as it generates a lot of logging (because this function is
      // called a lot!)
//      qDebug() <<
//         Q_FUNC_INFO << "Sender:" << senderClassName << "; property:" << prop.name() << "; val:" << val <<
//         "; propNameOfOurAdditionsInRecipe:" << propNameOfOurAdditionsInRecipe;
      // Is sender one of our items?
      NE * itemSender = qobject_cast<NE *>(rawSender);
      if (itemSender) {
         int ii = this->findIndexOf(itemSender);
         if (ii < 0) {
            return;
         }

         this->derived().updateTotals();
         emit this->derived().dataChanged(this->derived().createIndex(ii, 0),
                                          this->derived().createIndex(ii, this->derived().columnCount() - 1));
         emit this->derived().headerDataChanged(Qt::Vertical, ii, ii);
         return;
      }

      // See if our recipe gained or lost items.
      Recipe * recSender = qobject_cast<Recipe *>(rawSender);
      if (recSender && prop.name() == propNameOfOurAdditionsInRecipe) {
         this->checkRecipeItems<Caller>(recSender);
      }

      return;
   }

   //! \brief Default implementation for Derived::data
   QVariant doDataDefault(QModelIndex const & index, int role) const {
      if (!this->indexAndRoleOk(index, role)) {
         return QVariant();
      }

      // No special handling required for any of our columns
      return this->readDataFromModel(index, role);
   }

   Qt::ItemFlags doFlags(QModelIndex const & index) const {
      bool const tableIsEditable = this->derived().m_editable;
      static Qt::ItemFlags const defaults = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
      auto const columnIndex = static_cast<ColumnIndex>(index.column());

      // Not every table has a name column
      if constexpr (HAS_MEMBER(ColumnIndex, Name)) {
         if (columnIndex == ColumnIndex::Name) {
            return defaults;
         }
      }
      // If there's a TotalInventory column, it's read-only
      if constexpr (HAS_MEMBER(typename Derived::ColumnIndex, TotalInventory)) {
         if (columnIndex == Derived::ColumnIndex::TotalInventory) {
            return defaults /*| Qt::ItemIsEnabled*/;
         }
      }
      // This is only for SaltTableModel and RecipeAdjustmentSaltTableModel, but it's still less work to do a special
      // case here than have each subclass implement its own flags function.
      if constexpr (HAS_MEMBER(typename Derived::ColumnIndex, PctAcid)) {
         if (columnIndex == Derived::ColumnIndex::PctAcid &&
             !this->derived().m_rows[index.row()]->isAcid()) {
            return Qt::NoItemFlags;
         }
      }

      ColumnInfo const & columnInfo = this->get_ColumnInfo(index);
      if (columnInfo.typeInfo.isReadOnly()) {
         return defaults | Qt::ItemIsEnabled;
      }

      return defaults | (tableIsEditable ? Qt::ItemIsEditable : Qt::NoItemFlags);
   }

   //! \brief Default implementation for Derived::setData
   template<bool updateHeader = false>
   bool doSetDataDefault(QModelIndex const & index, QVariant const & value, int role) {
      if (!this->indexAndRoleOk(index, role)) {
         return false;
      }

      // No special handling required for any of our columns
      bool const retVal = this->writeDataToModel(index, value, role);

      // ...but we might need to re-show header
      if constexpr (updateHeader) {
         if (retVal) {
            emit this->derived().headerDataChanged(Qt::Vertical, index.row(), index.row());
         }
      }

      return retVal;
   }

   //================================================ Member Variables =================================================

   QList< std::shared_ptr<NE> > m_rows;
};


/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define TABLE_MODEL_COMMON_DECL(NeName) \
   /* This allows TableModelBase to call protected and private members of Derived */                             \
   friend class TableModelBase<NeName##TableModel, NeName>;                                                      \
                                                                                                                 \
   public:                                                                                                       \
      NeName##TableModel(QTableView * parent = nullptr, bool editable = true);                                   \
      virtual ~NeName##TableModel();                                                                             \
                                                                                                                 \
      /* This will be a no-op if we do not inherit from BtTableModelRecipeObserver */                            \
      void observeRecipe(Recipe * rec);                                                                          \
      /* This will always return nullptr if we do not inherit from BtTableModelRecipeObserver */                 \
      Recipe * getObservedRecipe() const;                                                                        \
                                                                                                                 \
   protected:                                                                                                    \
      /* This block of functions is called from the TableModelBase class */                                      \
      void added  (std::shared_ptr<NeName> item);                                                                \
      void removed(std::shared_ptr<NeName> item);                                                                \
      void updateTotals();                                                                                       \
                                                                                                                 \
   public:                                                                                                       \
      /** \brief Reimplemented from \c QAbstractTableModel */                                                    \
      virtual int columnCount(QModelIndex const & parent = QModelIndex()) const override;                        \
      /** \brief From \c BtTableModel */                                                                         \
      virtual ColumnInfo columnInfo(int section) const override;                                                 \
      /** \brief From \c BtTableModel */                                                                         \
      virtual QVariant columnLabel(int section) const override;                                                  \
      /** \brief Reimplemented from QAbstractTableModel. */                                                      \
      virtual int rowCount(QModelIndex const & parent = QModelIndex()) const override;                           \
      /** \brief Reimplemented from QAbstractTableModel. */                                                      \
      virtual QVariant data(QModelIndex const & index, int role = Qt::DisplayRole) const override;               \
      /** \brief Reimplemented from QAbstractTableModel. */                                                      \
      virtual Qt::ItemFlags flags(const QModelIndex& index) const override;                                      \
      /** \brief Reimplemented from QAbstractTableModel. */                                                      \
      virtual bool setData(QModelIndex const & index, QVariant const & value, int role = Qt::EditRole) override; \
                                                                                                                 \
   private slots:                                                                                                \
      /** \brief Watch \b NeName for changes. */                                                                 \
      void addItem(int itemId);                                                                                  \
                                                                                                                 \
      void removeItem(int itemId, std::shared_ptr<QObject> object);                                              \
                                                                                                                 \
      /** \brief Catch changes to Recipe, Database, and NeName. */                                               \
      void changed(QMetaProperty, QVariant);                                                                     \
                                                                                                                 \
      /** \brief Catches changes to inventory.  (Can be no-op where not relevant (eg \c MashStepTableModel). */  \
      void changedInventory(int invKey, BtStringConst const & propertyName);                                     \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 *
 *        Note that the RecipePropertyName parameter is ultimately ignored in updateStockPurchase for table models that
 *        do not observe a recipe, so can be set to PropertyNames::None::none in such cases.
 */
#define TABLE_MODEL_COMMON_CODE(NeName, LcNeName, RecipePropertyName) \
   void NeName##TableModel::observeRecipe(Recipe * rec) {                                               \
      this->doObserveRecipe<NeName##TableModel>(rec);                                                   \
      return;                                                                                           \
   }                                                                                                    \
   Recipe * NeName##TableModel::getObservedRecipe() const {                                             \
      return this->doGetObservedRecipe<NeName##TableModel>();                                           \
   }                                                                                                    \
   int NeName##TableModel::columnCount([[maybe_unused]] QModelIndex const & parent) const {             \
      return ColumnOwnerTraits<NeName##TableModel>::numColumns();                                       \
   }                                                                                                    \
   ColumnInfo NeName##TableModel::columnInfo(int section) const {                                       \
      return ColumnOwnerTraits<NeName##TableModel>::getColumnInfo(section);                             \
   }                                                                                                    \
   QVariant NeName##TableModel::columnLabel(int section) const {                                        \
      return ColumnOwnerTraits<NeName##TableModel>::getColumnLabel(section);                            \
   }                                                                                                    \
   int NeName##TableModel::rowCount([[maybe_unused]] QModelIndex const & parent) const {                \
      return this->m_rows.size();                                                                       \
   }                                                                                                    \
   Qt::ItemFlags NeName##TableModel::flags(QModelIndex const & index) const {                           \
      return this->doFlags(index);                                                                      \
   }                                                                                                    \
   void NeName##TableModel::addItem(int itemId) {                                                       \
      this->addById(itemId);                                                                            \
      return;                                                                                           \
   }                                                                                                    \
   void NeName##TableModel::removeItem([[maybe_unused]] int itemId, std::shared_ptr<QObject> object) {  \
      this->remove(std::static_pointer_cast<NeName>(object));                                           \
      return;                                                                                           \
   }                                                                                                    \
   void NeName##TableModel::changed(QMetaProperty prop, QVariant val) {                                 \
      this->propertyChanged<NeName##TableModel>(prop, val, RecipePropertyName);                         \
      return;                                                                                           \
   }                                                                                                    \
                                                                                                        \
   void NeName##TableModel::changedInventory(int invKey, BtStringConst const & propertyName) {          \
      this->updateStockPurchase<NeName##TableModel>(invKey, propertyName);                                  \
      return;                                                                                           \
   }

#endif
