/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/ItemDelegate.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef TABLEMODELS_ITEMDELEGATE_H
#define TABLEMODELS_ITEMDELEGATE_H
#pragma once

#include <QLineEdit>

#include "measurement/Measurement.h"
#include "utils/ColumnInfo.h"
#include "utils/NoCopy.h"
#include "widgets/BtComboBoxBool.h"
#include "widgets/BtComboBoxEnum.h"
#include "qtModels/tableModels/BtTableModel.h"

namespace {
   /**
    * Sets a combo box value from model data
    */
   template<class CB, typename T>
   void setComboBoxValue(CB * comboBox, TypeInfo const & typeInfo, QVariant & modelData) {
      if (typeInfo.isOptional()) {
         bool hasValue = false;
         Optional::removeOptionalWrapper<T>(modelData, &hasValue);
         if (!hasValue) {
            comboBox->setNull();
            return;
         }
      }
      comboBox->setValue(modelData.value<T>());
      return;
   }

}

/**
 * \class ItemDelegate
 *
 * \brief Used by \c BtTableModel subclasses
 *
 *           QObject
 *                \
 *                ...
 *                  \
 *                  QItemDelegate       ItemDelegate<HopItemDelegate, HopTableModel>
 *                              \       /
 *                               \     /
 *                           HopItemDelegate
 *
 *        Derived classes (eg \c HopItemDelegate in this example) need to implement the following boilerplate member
 *        functions that override \c QItemDelegate:
 *           createEditor         -- calls ItemDelegate::getEditWidget
 *           setEditorData        -- calls ItemDelegate::readDataFromModel
 *           setModelData         -- calls ItemDelegate::writeDataToModel
 *           updateEditorGeometry -- calls setGeometry on its first parameter
 *
 *        The code for the definitions of all these functions is "the same" for all delegates and should be inserted in
 *        the implementation file using the ITEM_DELEGATE_COMMON_CODE macro.  Eg, in HopItemDelegate, we need:
 *
 *          ITEM_DELEGATE_COMMON_CODE(Hop)
 *
 *
 */
template<class Derived, class NeTableModel>
class ItemDelegate {

public:
   ItemDelegate(NeTableModel & tableModel) : m_derived{static_cast<Derived *>(this)}, m_tableModel{tableModel} {
      return;
   }

   ~ItemDelegate() = default;

private:
   ColumnInfo const & getColumnInfo(QModelIndex const & index) const {
      // In theory, in C++20, we don't need the `typename` here, but as of 2023-05-26, Apple C++ compiler is Clang
      // 14.0.0, and we need Clang 16 before support for
      // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0634r3.html is implemented.
      auto const columnIndex = static_cast<typename NeTableModel::ColumnIndex>(index.column());
      //
      // Although we can get `QAbstractItemModel const *` from `index.model()`, this is an NeSortFilterProxyModel
      // (HopSortFilterProxyModel, FermentableSortFilterProxyModel, etc) rather than NeTableModel (ie HopTableModel,
      // FermentableTableModel, etc).  (The NeTableModel keeps rows in an arbitrary
      // order, and NeSortFilterProxyModel / QSortFilterProxyModel sits on top of it to do the mapping between "base"
      // order and "sorted" order depending on which column is being used for sorting and whether its an ascending or
      // descending sort.)
      //
      // So, for the purposes of getting column info, we need to grab a pointer to the NeTableModel in the constructor.
      //
      ColumnInfo const & columnInfo = m_tableModel.get_ColumnInfo(columnIndex);
      Q_ASSERT(index.column() >= 0);
      Q_ASSERT(columnInfo.index == static_cast<size_t>(index.column()));
      return columnInfo;
   }

public:

   /**
    * \brief Subclass should call this from its override of \c QItemDelegate::createEditor.
    *        Returns the widget used to edit the item specified by index for editing.
    */
   QWidget * getEditWidget(QWidget * parent,
                           [[maybe_unused]] QStyleOptionViewItem const & option,
                           QModelIndex const & index) const {
      ColumnInfo const & columnInfo = this->getColumnInfo(index);
      TypeInfo const & typeInfo = columnInfo.typeInfo;

      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         auto const fieldType = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);

         if (fieldType == NonPhysicalQuantity::Enum) {
            Q_ASSERT(typeInfo.displayAs);
            Q_ASSERT(std::holds_alternative<DisplayInfo::Enum>(*typeInfo.displayAs));
            DisplayInfo::Enum const & enumInfo = std::get<DisplayInfo::Enum>(*typeInfo.displayAs);

            BtComboBoxEnum * comboBox = new BtComboBoxEnum(parent);
            comboBox->init(columnInfo.modelName,
                           columnInfo.columnName,
                           columnInfo.columnFqName,
                           enumInfo.stringMapping,
                           enumInfo.displayNames,
                           typeInfo);
            comboBox->setMinimumWidth(comboBox->minimumSizeHint().width());
            comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            comboBox->setFocusPolicy(Qt::StrongFocus);

            return comboBox;
         }

         if (fieldType == NonPhysicalQuantity::Bool) {
            // It's a coding error if there isn't a BoolInfo structure for a bool field
            Q_ASSERT(typeInfo.displayAs);
            Q_ASSERT(std::holds_alternative<DisplayInfo::Bool>(*typeInfo.displayAs));
            DisplayInfo::Bool const & boolInfo = std::get<DisplayInfo::Bool>(*typeInfo.displayAs);

            BtComboBoxBool * boolComboBox = new BtComboBoxBool(parent);
            boolComboBox->init(columnInfo.modelName,
                               columnInfo.columnName,
                               columnInfo.columnFqName,
                               boolInfo.unsetDisplay,
                               boolInfo.setDisplay,
                               typeInfo);
            boolComboBox->setMinimumWidth(boolComboBox->minimumSizeHint().width());
            boolComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
            boolComboBox->setFocusPolicy(Qt::StrongFocus);

            return boolComboBox;
         }
      } else if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) &&
                 columnInfo.extras) {
         //
         // Where we have an editable amount that can be more than one physical quantity -- eg mass or volume -- we want
         // a combo box to allow the user to select the physical quantity.  This is a bit tricky as such a selector does
         // not have its own property.  Rather than over-generalise the ColumnInfo structure to
         // accommodate a new type of column, we adopt a convention that the selector column shares the same property as
         // the amount column, but has the Measurement::ChoiceOfPhysicalQuantity value (instead of, typically, a
         // PrecisionInfo) in the extras field.
         //
         auto const validMeasures = *columnInfo.extras;
         BtComboBoxEnum * comboBox = new BtComboBoxEnum(parent);
         comboBox->init(columnInfo.modelName,
                        columnInfo.columnName,
                        columnInfo.columnFqName,
                        Measurement::physicalQuantityStringMapping,
                        Measurement::physicalQuantityDisplayNames,
                        typeInfo,
                        &Measurement::allPossibilitiesAsInt(validMeasures));
         comboBox->setMinimumWidth(comboBox->minimumSizeHint().width());
         comboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
         comboBox->setFocusPolicy(Qt::StrongFocus);

         return comboBox;
      }

      return new QLineEdit(parent);
   }

   /**
    * \brief Subclass should call this from its override of \c QItemDelegate::setEditorData.
    *        Sets the data to be displayed and edited by the editor from the data model item specified by the model
    *        index.
    */
   void readDataFromModel(QWidget * editor,
                          QModelIndex const & index) const {
      ColumnInfo const & columnInfo = this->getColumnInfo(index);
      TypeInfo const & typeInfo = columnInfo.typeInfo;

      // Note that we need index.model(), not m_tableModel, as the former (eg a an HopSortFilterProxyModel) adds sorting
      // to the latter (eg HopTableModel).
      QAbstractItemModel const * model = index.model();
      // Because index is a run-time value, we need to pull the model data out in a QVariant.  We can use the Qt
      // Property system as we do elsewhere.
      QVariant modelData = model->data(index, Qt::EditRole);

      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         auto const fieldType = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);

         if (fieldType == NonPhysicalQuantity::Enum) {
            BtComboBoxEnum * comboBox = qobject_cast<BtComboBoxEnum *>(editor);
            setComboBoxValue<BtComboBoxEnum, int>(comboBox, typeInfo, modelData);
            return;
         }

         if (fieldType == NonPhysicalQuantity::Bool) {
            BtComboBoxBool * boolComboBox = qobject_cast<BtComboBoxBool *>(editor);
            setComboBoxValue<BtComboBoxBool, bool>(boolComboBox, typeInfo, modelData);
            return;
         }
      } else if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) &&
                 columnInfo.extras) {
         // Selector for editable amount that can be more than one physical quantity.  (See comment above in
         // getEditWidget() for more details.)
         BtComboBoxEnum * comboBox = qobject_cast<BtComboBoxEnum *>(editor);
         setComboBoxValue<BtComboBoxEnum, int>(comboBox, typeInfo, modelData);
         return;
      }

      // For everything else, TableModelBase::readDataFromModel (called from HopTableModel::data,
      // FermentableTableModel::data, etc) will have done all the work for us (including handling optional and forced
      // units etc) and provided a suitable QString in the QVariant.
      QLineEdit * line = qobject_cast<QLineEdit *>(editor);
      line->setText(modelData.toString());
      return;

   }

   /**
    * \brief Subclass should call this from its override of \c QItemDelegate::setModelData.
    *        Gets data from the editor widget and stores it in the specified model at the item index.
    *
    * \param editor
    * \param model This is needed on this function (and \c QItemDelegate::setModelData etc) because the function needs
    *              to be able to modify the model, so the \b \c const pointer to \c QAbstractItemModel returned from
    *              \c index.model() (which is what is used in \c readDataFromModel / \c QItemDelegate::setEditorData) is
    *              not sufficient.
    * \param index
    */
   void writeDataToModel(QWidget * editor,
                         QAbstractItemModel * model,
                         QModelIndex const & index) const {
      ColumnInfo const & columnInfo = this->getColumnInfo(index);
      TypeInfo const & typeInfo = columnInfo.typeInfo;

      // .:TBD:. For the moment, for enums and bools we don't check whether the combo box was changed before calling
      //         model->setData.  We could grab model->data(index, Qt::UserRole) and check it first, if it turns out
      //         this is a problem.
      if (std::holds_alternative<NonPhysicalQuantity>(*typeInfo.fieldType)) {
         auto const fieldType = std::get<NonPhysicalQuantity>(*typeInfo.fieldType);

         if (fieldType == NonPhysicalQuantity::Enum) {
            BtComboBoxEnum * comboBox = qobject_cast<BtComboBoxEnum *>(editor);
            model->setData(index, comboBox->getAsVariant(), Qt::EditRole);
            return;
         }

         if (fieldType == NonPhysicalQuantity::Bool) {
            BtComboBoxBool * boolComboBox = qobject_cast<BtComboBoxBool *>(editor);
            model->setData(index, boolComboBox->getAsVariant(), Qt::EditRole);
            return;
         }

         //
         // For strings and pure numbers there is no additional processing to do.  It is just percentages where we need
         // to strip the '%' off the end.  But, in reality, it's better to just strip any non-numeric stuff off the end
         // of anything that's a number without units.
         //
         // We do parsing in double here, even if it's going to end up in an int as (a) it's reasonably safe and (b)
         // that's what's happening under the hood in Measurement::extractRawFromString anyway.
         //
         QLineEdit* line = qobject_cast<QLineEdit*>(editor);
         QString rawValue = line->text();
         if (fieldType != NonPhysicalQuantity::String) {
            model->setData(index, Measurement::extractRawFromString(rawValue, typeInfo), Qt::EditRole);
         } else {
            model->setData(index, rawValue, Qt::EditRole);
         }
         return;
      } else if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*typeInfo.fieldType) &&
                 columnInfo.extras) {
         // Selector for editable amount that can be more than one physical quantity.  (See comment above in
         // getEditWidget() for more details.)
         BtComboBoxEnum * comboBox = qobject_cast<BtComboBoxEnum *>(editor);
         model->setData(index, comboBox->getAsVariant(), Qt::EditRole);
         return;
      }

      // Note that we handle any conversions to and from canonical amounts in the table model class, as we sometimes
      // need to look at multiple columns (eg "amount" and "is weight").  This means we also handle optional values
      // there too.  All that we do here is pass in the raw text.
      QLineEdit* line = qobject_cast<QLineEdit*>(editor);
      if (line->isModified()) {
         model->setData(index, line->text(), Qt::EditRole);
      }

      return;
   }

   //================================================ Member Variables =================================================

   /**
    * \brief This is the 'this' pointer downcast to the derived class, which allows us to call non-virtual member
    *        functions in the derived class from this templated base class.
    */
   Derived * m_derived;

   NeTableModel & m_tableModel;

   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(ItemDelegate)
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note:
 *          - As elsewhere, we have to be careful about comment formats in macro definitions
 *          - We cannot put the whole class declaration in the macro as (I think) it confuses the Qt MOC
 */
#define ITEM_DELEGATE_COMMON_DECL(NeName) \
   public:                                                                                                                    \
      NeName##ItemDelegate(QTableView * parent, NeName##TableModel & tableModel);                                             \
      virtual ~NeName##ItemDelegate();                                                                                        \
                                                                                                                              \
      /** \brief Reimplemented from QItemDelegate. */                                                                         \
      virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;     \
      /** \brief Reimplemented from QItemDelegate. */                                                                         \
      virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;                                            \
      /** \brief Reimplemented from QItemDelegate. */                                                                         \
      virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;                  \
      /** \brief Reimplemented from QItemDelegate. */                                                                         \
      virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const; \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define ITEM_DELEGATE_COMMON_CODE(NeName) \
   NeName##ItemDelegate::NeName##ItemDelegate(QTableView * parent,               \
                                              NeName##TableModel & tableModel) : \
      QItemDelegate(parent),                                               \
      ItemDelegate<NeName##ItemDelegate, NeName##TableModel>(tableModel) { \
      return; \
   }          \
   NeName##ItemDelegate::~NeName##ItemDelegate() = default; \
   QWidget * NeName##ItemDelegate::createEditor(QWidget * parent,                    \
                                                QStyleOptionViewItem const & option, \
                                                QModelIndex const & index) const {   \
      return this->getEditWidget(parent, option, index);                       \
   }                                                                           \
   void NeName##ItemDelegate::setEditorData(QWidget * editor,                  \
                                            QModelIndex const & index) const { \
      this->readDataFromModel(editor, index); \
      return;                                 \
   }                                          \
   void NeName##ItemDelegate::setModelData(QWidget * editor,                  \
                                           QAbstractItemModel * model,        \
                                           QModelIndex const & index) const { \
      this->writeDataToModel(editor, model, index); \
      return;                                       \
   }                                                \
   void NeName##ItemDelegate::updateEditorGeometry(QWidget * editor,                                   \
                                                   QStyleOptionViewItem const & option,                \
                                                   [[maybe_unused]] QModelIndex const & index) const { \
      editor->setGeometry(option.rect);                                                                \
      return;                                                                                          \
   }                                                                                                   \


#endif
