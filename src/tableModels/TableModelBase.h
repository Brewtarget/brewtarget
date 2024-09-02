/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/TableModelBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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

#include  <type_traits>

#include <QList>
#include <QModelIndex>

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "model/IngredientAmount.h"
#include "model/Inventory.h"
#include "tableModels/BtTableModel.h"
///#include "tableModels/BtTableModelInventory.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/MetaTypes.h"

// TODO: We would like to change "Add to Recipe" to "Set for Recipe" for things where the recipe only has one of them, eg Style or Equipment

class Style;

//
// Using concepts allows us to tailor the templated TableModelTraits class without having the same inheritance
// structure as BtTableModel / BtTableModelRecipeObserver.
//
// Note that concepts have some limitations:
//    - Concepts cannot recursively refer to themselves and cannot be constrained, is you cannot define one concept in
//      terms of another
//    - You cannot have explicit instantiations, explicit specializations, or partial specializations of concepts
// An alternative would be to use a traits struct, which has fewer limitations, but is somewhat more clunky.
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename T> concept CONCEPT_FIX_UP IsTableModel   = std::is_base_of_v<BtTableModel, T>;
///template <typename T> concept CONCEPT_FIX_UP HasInventory   = std::is_base_of_v<BtTableModelInventory, T>;
///template <typename T> concept CONCEPT_FIX_UP HasNoInventory = std::negation_v<std::is_base_of<BtTableModelInventory, T>>;
template <typename T> concept CONCEPT_FIX_UP ObservesRecipe       = std::is_base_of_v<BtTableModelRecipeObserver, T>;
template <typename T> concept CONCEPT_FIX_UP DoesNotObserveRecipe = std::negation_v<std::is_base_of<BtTableModelRecipeObserver, T>>;

//
// NOTE: At several places below in TableModelBase, we would like to have two versions of a member function depending on
//       some property of Derived.  Eg for updateInventory() we would like a substantive one for when Derived inherits
//       from BtTableModelInventory and a no-op one for when it doesn't.  On GCC, we can do the following:
//          void updateInventory(...) requires HasInventory<Derived> { ... } // Substantive version
//          void updateInventory(...) requires HasNoInventory<Derived> { ... } // No-op version
//
//       HOWEVER, other compilers (eg Clang) do not permit this and give an error along the lines of "incomplete type
//       ... used in type trait expression ... definition of ...  is not complete until the closing '}'".  This is
//       annoying but correct.  The C++ standard rules say we are not allowed to use `HasInventory<Derived>` or
//       `HasNoInventory<Derived>` until `Derived` is fully defined, which won't be until the closing brace of its class
//       definition (and the `TableModelBase` template is being instantiated before that because `Derived` inherits from
//       it).
//
//       The way round this is to template the member function so that the evaluation of the constraint is deferred
//       until after the class declaration of `Derived` is complete.  Now, in, eg, HopTableModel we can call the right
//       version via:
//          this->updateInventory<HopTableModel>(...);
//
//       (I did also try `this->updateInventory<decltype(*this)>(...)`, but I couldn't get it to work.
//
//       But what if we want to call one of these templated functions from inside TableModelBase?  Well, then we have to
//       call down to a wrapper function in Derived that knows which version of the TableModelBase function to call.  Eg
//       if we want a function that returns "pointer to the observed recipe or null if this class does not observe a
//       recipe" then, in TableModelBase we have two "doer" member functions:
//          template<class Caller> Recipe * doGetObservedRecipe() requires ObservesRecipe<Caller> {...} Substantive
//          template<class Caller> Recipe * doGetObservedRecipe() requires DoesNotObserveRecipe<Caller> {...} No-op
//       And in each of the Derived classes, such as HopTableModel, we have a (macro-inserted) wrapper function such as:
//          void HopTableModel::getObservedRecipe() { return this->doGetObservedRecipe<NeName##TableModel>(); }
//       Then, from TableModel, we call this->derived().getObservedRecipe() and the correct version of doGetObservedRecipe()
//       ends up being called.
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
 * \brief We want, eg, \c HopTableModel to inherit from \c TableModelBase<HopTableModel, Hop> and to have its own enum
 *        \c HopTableModel::ColumnIndex.  But we'd also like \c HopTableModel::ColumnIndex to be accessible from within
 *        \c TableModelBase, which normally isn't possible, eg as explained at
 *        https://stackoverflow.com/questions/5534759/c-with-crtp-class-defined-in-the-derived-class-is-not-accessible-in-the-base
 *        However, per the same link, the way around this is to use a traits class.  This is another "trick" where we
 *        declare a template for the "traits" class before the base class of the curiously recurring template pattern
 *        (CRTP), but then specialise that "traits" class in the derived class.
 */
template<class Derived>
struct TableModelTraits;

/**
 * \brief See comment in tableModels/BtTableModel.h for more info on inheritance structure
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
 *        Note that we use TableModelTraits as the phantom class for CuriouslyRecurringTemplateBase since it fits the
 *        bill.
 */
template<class Derived, class NE>
class TableModelBase : public CuriouslyRecurringTemplateBase<TableModelTraits, Derived> {
public:
   // This gets round the fact that we would not be able to access Derived::ColumnIndex directly
   //
   // In theory, in C++20, we don't need the `typename` here, but, per comment in BtTableModel::ColumnInfo, we need to
   // retain it until our Mac build environment is using a more recent version of Clang.
   using ColumnIndex = typename TableModelTraits<Derived>::ColumnIndex;

protected:
   TableModelBase() : rows{} {
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
   BtTableModel::ColumnInfo const & get_ColumnInfo(ColumnIndex const columnIndex) const {
      return this->derived().BtTableModel::getColumnInfo(static_cast<size_t>(columnIndex));
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
         // this->addItems(this->derived().recObs->getAll<NE>());
         this->addItems(rec->getAll<NE>());
      }
      qDebug() << Q_FUNC_INFO << "Now have" << this->rows.size() << "rows";
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
      if (!(this->rows.isEmpty())) {
         if (ii >= 0 && ii < this->rows.size()) {
            return this->rows[ii];
         }
         qWarning() << Q_FUNC_INFO << "index out of range (" << ii << "/" << this->rows.size() << ")";
      } else {
         qWarning() << Q_FUNC_INFO << "this->rows is empty (" << ii << "/" << this->rows.size() << ")";
      }
      return nullptr;
   }

   /**
    * \brief Remove duplicates and non-displayable items from the supplied list
    */
   QList< std::shared_ptr<NE> > removeDuplicates(QList< std::shared_ptr<NE> > items,
                                                 Recipe const * recipe = nullptr) {
      decltype(items) tmp;

      for (auto ii : items) {
         if (!recipe && (ii->deleted() || !ii->display())) {
            continue;
         }
         if (!this->rows.contains(ii) ) {
            tmp.append(ii);
         }
      }
      return tmp;
   }

   /**
    * \brief Remove duplicates, ignoring if the item is displayed
    */
   QList< std::shared_ptr<NE> > removeDuplicatesIgnoreDisplay(QList< std::shared_ptr<NE> > items,
                                                              Recipe const * recipe = nullptr) {
      decltype(items) tmp;

      for (auto ii : items) {
         if (!recipe && ii->deleted() ) {
            continue;
         }
         if (!this->rows.contains(ii) ) {
            tmp.append(ii);
         }
      }
      return tmp;
   }

   /**
    * \brief Given a raw pointer, find the index of the corresponding shared pointer in \c this->rows
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
    * \return index of object in this->rows or -1 if it's not found
    */
   int findIndexOf(NE const * object) const {
      for (int index = 0; index < this->rows.size(); ++index) {
         if (this->rows.at(index).get() == object) {
            return index;
         }
      }
      return -1;
   }

   void add(std::shared_ptr<NE> item) {
      qDebug() << Q_FUNC_INFO << item->name();

      // Check to see if it's already in the list
      if (this->rows.contains(item)) {
         return;
      }

      // If we are observing the database, ensure that the item is undeleted and fit to display.
      Recipe * observedRecipe = this->derived().getObservedRecipe();
      if (!observedRecipe && (item->deleted() || !item->display())) {
         return;
      }

///      // If we are watching a Recipe and the new item does not belong to it then there is nothing for us to do
///      if (observedRecipe && !observedRecipe->uses(*item)) {
///         qDebug() <<
///            Q_FUNC_INFO << "Ignoring signal about new" << NE::staticMetaObject.className() << "#" << item->key() <<
///            "as it does not belong to the Recipe we are watching: #" << observedRecipe->key();
///         return;
///      }

      int size = this->rows.size();
      this->derived().beginInsertRows(QModelIndex(), size, size);
      this->rows.append(item);
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
      int rowNum = this->rows.indexOf(item);
      if (rowNum >= 0)  {
         this->derived().beginRemoveRows(QModelIndex(), rowNum, rowNum);
         this->derived().disconnect(item.get(), nullptr, &this->derived(), nullptr);
         this->rows.removeAt(rowNum);

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
    * \brief Currently only used on SaltTableModel I think
    */
   bool remove(QModelIndex const & index) {
      if (!this->isIndexOk(index)) {
         return false;
      }

      return this->remove(this->rows[index.row()]);
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
         "to existing list of" << this->rows.size();

      auto tmp = this->removeDuplicates(items, this->derived().getObservedRecipe());

      qDebug() << Q_FUNC_INFO << "After de-duping, adding " << tmp.size() << "of" << NE::staticMetaObject.className();

      int size = this->rows.size();
      if (size + tmp.size()) {
         this->derived().beginInsertRows(QModelIndex(), size, size + tmp.size() - 1);

         this->rows.append(tmp);

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
      int const size = this->rows.size();
      if (size > 0) {
         this->derived().beginRemoveRows(QModelIndex(), 0, size - 1);
         while (!this->rows.empty()) {
            auto item = this->rows.takeLast();
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
    * \brief Check supplied index is within bounds
    */
   bool isIndexOk(QModelIndex const & index) const {
      if (index.row() >= static_cast<int>(this->rows.size())) {
         qCritical() << Q_FUNC_INFO << "Bad model index. row = " << index.row() << "; max row = " << this->rows.size();
         return false;
      }

      auto row = this->rows[index.row()];
      if (!row) {
         // This is almost certainly a coding error
         qCritical() << Q_FUNC_INFO << "Null pointer at row" << index.row() << "of" << this->rows.size();
         return false;
      }
      return true;
   }

   /**
    * \brief Child classes should call this from their \c data() member function (overriding
    *        \c QAbstractTableModel::data()) to read data for any column that does not require special handling
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
      // Per https://doc.qt.io/qt-6/qt.html#ItemDataRole-enum, there are a dozen or so different "roles" that we can
      // get called for, mostly from the Qt framework itself.  If we don't have anything special to say for a particular
      // role, eg if we don't want to return a custom QFont when requested with Qt::FontRole, then
      // https://doc.qt.io/qt-6/qabstractitemmodel.html#data says we just need to return "an invalid (default-
      // constructed) QVariant".
      //
      if (role != Qt::DisplayRole && role != Qt::EditRole) {
         return QVariant{};
      }

      auto row = this->rows[index.row()];
      auto const columnIndex = static_cast<ColumnIndex>(index.column());
      auto const & columnInfo = this->get_ColumnInfo(columnIndex);

      QVariant modelData = columnInfo.propertyPath.getValue(*row);
      if (!modelData.isValid()) {
         // It's a programming error if we couldn't read a property modelData
         qCritical() <<
            Q_FUNC_INFO << "Unable to read" << row->metaObject()->className() << "#" << row->key() << "property" <<
            columnInfo.propertyPath << "(Got" << modelData << ")";
         Q_ASSERT(false); // Stop here on debug builds
      }

      //
      // Unlike in an editor, in the table model, the edit control is only shown when you are actually editing a field.
      // Normally there's a separate control flow for just displaying the modelData otherwise.  We'll get called in both
      // cases, but the modelData of `role` will be different.
      //
      // For Qt::EditRole, we're being called from ItemDelegate::readDataFromModel (see tableModels/ItemDelegate.h),
      // which will handle any special display requirements for enums and bools (where, in both cases, we show combo
      // boxes), because it is feeding directly into the appropriate editor widget.  For other types, we want to hand
      // back something that can be converted to QString.
      //
      // For Qt::DisplayRole, we're typically being called from QSortFilterProxyModel::data which is, in turn, called by
      // QItemDelegate::paint.  We don't want to override QItemDelegate::paint in ItemDelegate, because it would be
      // overkill.  So, here, we just need to make sure we're returning something that can sensibly be converted to
      // QString.
      //
      TypeInfo const & typeInfo = columnInfo.typeInfo;

      // First handle the cases where ItemDelegate::readDataFromModel wants "raw" data
      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         auto const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);
         if (nonPhysicalQuantity == NonPhysicalQuantity::Enum ||
             nonPhysicalQuantity == NonPhysicalQuantity::Bool) {
            if (role != Qt::DisplayRole) {
               return modelData;
            }
         }
      }

      // Next handle unset optional values
      bool hasValue = false;
      if (typeInfo.isOptional()) {
         // This does the right thing even for enums - see comment in utils/OptionalHelpers.cpp
         Optional::removeOptionalWrapper(modelData, typeInfo, &hasValue);
         if (!hasValue) {
            return QString{""};
         }
      }

      // Now we know:
      //    - the value is either not optional or is optional and set
      //    - we need to return a string
      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         auto const nonPhysicalQuantity = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);
         if (nonPhysicalQuantity == NonPhysicalQuantity::Enum) {
            Q_ASSERT(role == Qt::DisplayRole);

            Q_ASSERT(columnInfo.extras);
            Q_ASSERT(std::holds_alternative<BtTableModel::EnumInfo>(*columnInfo.extras));
            BtTableModel::EnumInfo const & enumInfo = std::get<BtTableModel::EnumInfo>(*columnInfo.extras);
            Q_ASSERT(modelData.canConvert<int>());
            std::optional<QString> displayText = enumInfo.displayNames.enumAsIntToString(modelData.toInt());
            // It's a coding error if we couldn't find something to display!
            Q_ASSERT(displayText);
            return *displayText;
         }

         if (nonPhysicalQuantity == NonPhysicalQuantity::Bool) {
            Q_ASSERT(role == Qt::DisplayRole);

            Q_ASSERT(columnInfo.extras);
            Q_ASSERT(std::holds_alternative<BtTableModel::BoolInfo>(*columnInfo.extras));
            BtTableModel::BoolInfo const & info = std::get<BtTableModel::BoolInfo>(*columnInfo.extras);
            Q_ASSERT(modelData.canConvert<bool>());
            return modelData.toBool() ? info.setDisplay : info.unsetDisplay;
         }

         if (nonPhysicalQuantity == NonPhysicalQuantity::Percentage) {
            unsigned int precision = 3;
            if (columnInfo.extras) {
               Q_ASSERT(std::holds_alternative<BtTableModel::PrecisionInfo>(*columnInfo.extras));
               BtTableModel::PrecisionInfo const & precisionInfo =
                  std::get<BtTableModel::PrecisionInfo>(*columnInfo.extras);
               precision = precisionInfo.precision;
            }
            // We assert that percentages are numbers and therefore either are double or convertible to double
            Q_ASSERT(modelData.canConvert<double>());
            return QVariant(Measurement::displayQuantity(modelData.toDouble(), precision, nonPhysicalQuantity));
         }
      } else {
         // Most of the handling for Measurement::ChoiceOfPhysicalQuantity and Measurement::PhysicalQuantity is the same.
         Q_ASSERT(!columnInfo.extras ||
                  std::holds_alternative<BtTableModel::PrecisionInfo          >(*columnInfo.extras) ||
                  std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*columnInfo.extras) );

         unsigned int precision = 3;
         if (columnInfo.extras &&
             std::holds_alternative<BtTableModel::PrecisionInfo>(*columnInfo.extras)) {
            BtTableModel::PrecisionInfo const & info = std::get<BtTableModel::PrecisionInfo>(*columnInfo.extras);
            precision = info.precision;
         }

         // A field marked Measurement::ChoiceOfPhysicalQuantity can, for now, be a double or Measurement::Amount (or
         // a subclass thereof).  If it's a double, we can't return a Measurement::Amount without consulting another
         // field, so we don't.  If it's a Measurement::Amount, it's handled properly in the `else` below.
         if (typeInfo.typeIndex == typeid(double)) {
            Q_ASSERT(modelData.canConvert<double>());
            double rawValue = modelData.value<double>();
            if (std::holds_alternative<Measurement::PhysicalQuantity>(*typeInfo.fieldType)) {
               // This is one of the points where it's important that NamedEntity classes always store data in canonical
               // units.  For any properties where that's _not_ the case, we need to ensure we're passing
               // Measurement::Amount, ie the units are always included.
               auto const physicalQuantity = std::get<Measurement::PhysicalQuantity>(*typeInfo.fieldType);
               Measurement::Amount amount{rawValue, Measurement::Unit::getCanonicalUnit(physicalQuantity)};
               return QVariant(
                  Measurement::displayAmount(amount,
                                             precision,
                                             columnInfo.getForcedSystemOfMeasurement(),
                                             columnInfo.getForcedRelativeScale())
               );
            }
         } else if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) ||
                    typeInfo.typeIndex == typeid(Measurement::Amount)) {
            //
            // Per the comments in tableModels/ItemDelegate.h, depending on the value of extras, this is either the
            // amount itself or a drop-down for the PhysicalQuantity of the amount.
            //
            // In both cases, we start by getting the amount from the model.
            //
            // Note that, although we can downcast MassOrVolumeAmt to Measurement::Amount, QVariant doesn't know about
            // this.  So a QVariant holding MassOrVolumeAmt will return false from canConvert<Measurement::Amount>().
            //
            // Similarly, since QVariant doesn't understand type aliases, it will treat two aliases to the same type as
            // though they were different types.
            //
            // On the whole therefore, it's simpler to pass Measurement::Amount through the property system, even when
            // the underlying type is an instance of the Measurement::ConstrainedAmount template such as type alias
            // MassOrVolumeAmt or type alias MassVolumeOrCountAmt.
            //
            // TBD: In the long run, we might get rid of the MassOrVolumeAmt etc type aliases
            //
            Measurement::Amount amount;
            if (modelData.canConvert<Measurement::Amount>()) {
               amount = modelData.value<Measurement::Amount>();
            } else {
               // It's a coding error if we get here
               qCritical() <<
                  Q_FUNC_INFO << columnInfo.columnFqName << "Don't know how to parse" << columnInfo.propertyPath <<
                  "TypeInfo:" << typeInfo << ", modelData:" << modelData;
               Q_ASSERT(false);
            }
            if (!amount.isValid()) {
               qCritical() <<
                  Q_FUNC_INFO << columnInfo.columnFqName << "Invalid amount for" << columnInfo.propertyPath <<
                  "TypeInfo:" << typeInfo << ", modelData:" << modelData;
               Q_ASSERT(false);
            }

            if (columnInfo.extras &&
                std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*columnInfo.extras)) {
               // This is the drop-down for the PhysicalQuantity of the Amount
               Measurement::PhysicalQuantity const physicalQuantity = amount.unit->getPhysicalQuantity();
               if (role != Qt::DisplayRole) {
                  // For edit, we just want the actual PhysicalQuantity
                  return QVariant::fromValue(static_cast<int>(physicalQuantity));
               }

               // For display we want to map the PhysicalQuantity to its user-friendly name string
               std::optional<QString> displayText =
                  Measurement::physicalQuantityDisplayNames.enumToString(physicalQuantity);
               // It's a coding error if we couldn't find something to display!
               Q_ASSERT(displayText);
               return *displayText;
            }

            // This is the Amount itself
            return QVariant(
               Measurement::displayAmount(amount,
                                          precision,
                                          columnInfo.getForcedSystemOfMeasurement(),
                                          columnInfo.getForcedRelativeScale())
            );
         }
      }

      // If we got here, there's no special handling required - ie the data has no units or special formatting
      // requirements, so we can just return as-is.
      return modelData;
   }

   /**
    * \brief Child classes should call this from their \c setData() member function (overriding
    *        \c QAbstractTableModel::setData()) to write data for any column that does not require special handling
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
      auto row = this->rows[index.row()];
      auto const columnIndex = static_cast<ColumnIndex>(index.column());
      auto const & columnInfo = this->get_ColumnInfo(columnIndex);

      TypeInfo const & typeInfo = columnInfo.typeInfo;

      // Uncomment this if one of the physicalQuantity-related asserts below is firing
//      qDebug().noquote() << Q_FUNC_INFO << "physicalQuantity: " << physicalQuantity << Logging::getStackTrace();
      // For all non physical quantities, including enums and bools, ItemDelegate::writeDataToModel will already have
      // created the right type of QVariant for us, including handling whether or not it is optional.
      QVariant processedValue;
      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         // It's a coding error if physicalQuantity was supplied for a field that is not a PhysicalQuantity
         Q_ASSERT(!physicalQuantity);
         processedValue = value;
      } else  {
         // For physical quantities, we need to handle any conversions to and from canonical amounts, as well as deal
         // with optional values.
         //
         // ItemDelegate::writeDataToModel should have just given us a raw string
         Q_ASSERT(value.canConvert(QVariant::String));

         //
         // For cases where we have an Amount and a drop-down chooser to select PhysicalQuantity (eg between Mass and
         // Volume), we have two columns with the same type.  The one that actually holds the amount is relatively
         // straightforward because that's what we're already holding in `value`.  The one that holds the drop-down
         // chooser also needs access to the amount, so it needs to get it from the model (which happens below).
         //
         bool const isPhysicalQuantityChooser =
           std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) &&
           columnInfo.extras &&
           std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*columnInfo.extras);

         if (std::holds_alternative<Measurement::PhysicalQuantity>(*typeInfo.fieldType)) {
            // It's a coding error if physicalQuantity was supplied - because it's known in advance from the field type
            Q_ASSERT(!physicalQuantity);
            // Might seem a bit odd to overwrite the parameter here, but it allows us to share most of the code for
            // PhysicalQuantity and ChoiceOfPhysicalQuantity
            physicalQuantity = std::get<Measurement::PhysicalQuantity>(*typeInfo.fieldType);
         } else {
            // This should be the only possibility left
            Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType));
            // It's a coding error if physicalQuantity was not supplied for a non-Amount quantity column.  Equally it's
            // a coding error to supply it for the drop-down chooser column or for an Amount column.
            if (typeInfo.typeIndex == typeid(double)) {
               // For a double representing a ChoiceOfPhysicalQuantity, we need to be passed in the current
               // PhysicalQuantity because we don't know how to obtain it generically.
               Q_ASSERT(physicalQuantity);
            } else {
               Q_ASSERT(!physicalQuantity);
               if (!isPhysicalQuantityChooser) {
                  // If this is an Amount field, we just ask the model for the current amount and look at what
                  // PhysicalQuantity that is.
                  // As above, overwriting the physicalQuantity parameter here simplifies the code below
                  physicalQuantity =
                     QVariant(columnInfo.propertyPath.getValue(*row)).value<Measurement::Amount>().unit->getPhysicalQuantity();
               }
            }
         }

         // For the moment, I'm assuming any ChoiceOfPhysicalQuantity amount is never optional.  If we change our minds
         // about that in future then we'd need some additional logic here and in several other places.
         Q_ASSERT(isPhysicalQuantityChooser || physicalQuantity);
         Measurement::Amount amount{
            isPhysicalQuantityChooser ?
            // Drop-down
            QVariant(columnInfo.propertyPath.getValue(*row)).value<Measurement::Amount>() :
            // Amount itself
            Measurement::qStringToSI(value.toString(),
                                     *physicalQuantity,
                                     columnInfo.getForcedSystemOfMeasurement(),
                                     columnInfo.getForcedRelativeScale())
         };
         if (typeInfo.typeIndex == typeid(double)) {
            processedValue = Optional::variantFromRaw(amount.quantity, typeInfo.isOptional());
         } else {
            // Comments above in readDataFromModel apply equally here.  You can cast between MassOrVolumeAmt and
            // Measurement::Amount, but not between QVariant<MassOrVolumeAmt> and QVariant<Measurement::Amount>, so
            // we have to do the casting before we wrap.
            if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) ||
                typeInfo.typeIndex == typeid(Measurement::Amount)) {

               // If this is the drop-down for the PhysicalQuantity of the amount, then changing it is just another way
               // of changing the amount itself.
               if (isPhysicalQuantityChooser) {
                  //
                  // There isn't an ideal way to convert an Amount from one PhysicalQuantity to another.  There is no
                  // generic way, eg, to convert from mass to volume because it depends on the density of the thing
                  // being measured, which we don't know.  The best we can do is convert from one canonical unit to
                  // another, eg from kilograms to liters in the case of mass to volume.
                  //
                  // This is easy to do because whatever amount we get back from the model will already be in canonical
                  // units because, by convention, we only store things in canonical units in the model and the DB.
                  //
                  Q_ASSERT(value.canConvert<int>());
                  Measurement::Unit const & newUnit = Measurement::Unit::getCanonicalUnit(
                     static_cast<Measurement::PhysicalQuantity>(value.value<int>())
                  );
                  amount.unit = &newUnit;
               }

               processedValue = Optional::variantFromRaw(static_cast<Measurement::Amount>(amount),
                                                         typeInfo.isOptional());
            } else {
               // It's a coding error if we get here
               qCritical() <<
                  Q_FUNC_INFO << columnInfo.columnFqName << "Don't know how to parse" << columnInfo.propertyPath <<
                  "TypeInfo:" << typeInfo << ", value:" << value << ", amount:" << amount;
               Q_ASSERT(false);
            }
         }
      }

      MainWindow::instance().doOrRedoUpdate(
         *row,
         columnInfo.propertyPath,
         typeInfo,
         processedValue,
         NE::tr("Change %1 %2").arg(NE::staticMetaObject.className()).arg(columnInfo.columnName)
      );

      return true;
   }

   template<class TM>
   void updateInventory(int invKey, BtStringConst const & propertyName) requires IsTableModel<TM> &&
                                                                                 CanHaveInventory<NE> {
      // Substantive version
      if (propertyName == PropertyNames::IngredientAmount::amount) {
         for (int ii = 0; ii < this->rows.size(); ++ii) {
            std::shared_ptr<NE> ingredient = this->rows.at(ii);
            if (InventoryTools::hasInventory<NE>(*ingredient)) {
               std::shared_ptr<typename NE::InventoryClass> inventory = InventoryTools::getInventory(*ingredient);
               if (inventory->key() == invKey) {
                  emit this->derived().dataChanged(
                     this->derived().createIndex(ii, static_cast<int>(Derived::ColumnIndex::TotalInventory)),
                     this->derived().createIndex(ii, static_cast<int>(Derived::ColumnIndex::TotalInventory))
                  );
               }
            }
         }
      }
      return;
   }
   template<class TM>
   void updateInventory([[maybe_unused]] int invKey,
                        [[maybe_unused]] BtStringConst const & propertyName) requires IsTableModel<TM> &&
                                                                                      CannotHaveInventory<NE> {
      // No-op version
      return;
   }

   template<class Caller>
   void checkRecipeItems(Recipe * recipe) requires IsTableModel<Caller> && ObservesRecipe<Caller>{
      qDebug() << Q_FUNC_INFO;
      if (recipe == this->derived().recObs) {
         this->removeAll();
         // TBD: Commented out version doesn't compile on GCC
         // this->addItems(this->derived().recObs->getAll<NE>());
         this->addItems(recipe->getAll<NE>());
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
    * \param propNameOfOurAdditionsInRecipe  This needs to be something valid in all cases, but is only used if Derived is a
    *                                  recipe observer.
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

   //================================================ Member Variables =================================================

   QList< std::shared_ptr<NE> > rows;
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define TABLE_MODEL_COMMON_DECL(NeName) \
   /* This allows TableModelBase to call protected and private members of Derived */                            \
   friend class TableModelBase<NeName##TableModel, NeName>;                                                     \
                                                                                                                \
   public:                                                                                                      \
      NeName##TableModel(QTableView * parent = nullptr, bool editable = true);                                  \
      virtual ~NeName##TableModel();                                                                            \
                                                                                                                \
      /* This will be a no-op if we do not inherit from BtTableModelRecipeObserver */                           \
      void observeRecipe(Recipe * rec);                                                                         \
      /* This will always return nullptr if we do not inherit from BtTableModelRecipeObserver */                \
      Recipe * getObservedRecipe() const;                                                                       \
                                                                                                                \
   protected:                                                                                                   \
      /* This block of functions is called from the TableModelBase class */                                     \
      void added  (std::shared_ptr<NeName> item);                                                               \
      void removed(std::shared_ptr<NeName> item);                                                               \
      void updateTotals();                                                                                      \
                                                                                                                \
   public:                                                                                                      \
      /** \brief Reimplemented from QAbstractTableModel. */                                                     \
      virtual int rowCount(QModelIndex const & parent = QModelIndex()) const;                                   \
      /** \brief Reimplemented from QAbstractTableModel. */                                                     \
      virtual QVariant data(QModelIndex const & index, int role = Qt::DisplayRole) const;                       \
      /** \brief Reimplemented from QAbstractTableModel. */                                                     \
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;  \
      /** \brief Reimplemented from QAbstractTableModel. */                                                     \
      virtual Qt::ItemFlags flags(const QModelIndex& index) const;                                              \
      /** \brief Reimplemented from QAbstractTableModel. */                                                     \
      virtual bool setData(QModelIndex const & index, QVariant const & value, int role = Qt::EditRole);         \
                                                                                                                \
   private slots:                                                                                               \
      /** \brief Watch \b NeName for changes. */                                                                \
      void addItem(int itemId);                                                                                 \
                                                                                                                \
      void removeItem(int itemId, std::shared_ptr<QObject> object);                                             \
                                                                                                                \
      /** \brief Catch changes to Recipe, Database, and NeName. */                                              \
      void changed(QMetaProperty, QVariant);                                                                    \
                                                                                                                \
      /** \brief Catches changes to inventory.  (Can be no-op where not relevant (eg \c MashStepTableModel). */ \
      void changedInventory(int invKey, BtStringConst const & propertyName);                                    \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 *
 *        Note that the RecipePropertyName parameter is ultimately ignored in updateInventory for table models that
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
   int NeName##TableModel::rowCount([[maybe_unused]] QModelIndex const & parent) const {                \
      return this->rows.size();                                                                         \
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
      this->updateInventory<NeName##TableModel>(invKey, propertyName);                                  \
      return;                                                                                           \
   }

#endif
