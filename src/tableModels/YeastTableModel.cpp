/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/YeastTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Tim Payne <swstim@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "tableModels/YeastTableModel.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QHeaderView>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "model/RecipeUseOfWater.h"
#include "tableModels/ItemDelegate.h"
#include "utils/BtStringConst.h"
#include "widgets/BtComboBox.h"

YeastTableModel::YeastTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         // NOTE: Need PropertyNames::Yeast::amountWithUnits not PropertyNames::Yeast::amount below so we
         //       can handle mass-or-volume generically in TableModelBase.  Same for inventoryWithUnits.
         TABLE_MODEL_HEADER(Yeast, Name              , tr("Name"       ), PropertyNames::NamedEntity::name         ),
         TABLE_MODEL_HEADER(Yeast, Laboratory        , tr("Laboratory" ), PropertyNames::Yeast::laboratory         ),
         TABLE_MODEL_HEADER(Yeast, ProductId         , tr("Product ID" ), PropertyNames::Yeast::productId          ),
         TABLE_MODEL_HEADER(Yeast, Type              , tr("Type"       ), PropertyNames::Yeast::type               , EnumInfo{Yeast::typeStringMapping, Yeast::typeDisplayNames}),
         TABLE_MODEL_HEADER(Yeast, Form              , tr("Form"       ), PropertyNames::Yeast::form               , EnumInfo{Yeast::formStringMapping, Yeast::formDisplayNames}),
         TABLE_MODEL_HEADER(Yeast, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(Yeast, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory, Yeast::validMeasures),
      }
   },
   TableModelBase<YeastTableModel, Yeast>{} {

   setObjectName("yeastTableModel");

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &YeastTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryYeast>::getInstance(), &ObjectStoreTyped<InventoryYeast>::signalPropertyChanged,
           this, &YeastTableModel::changedInventory);
   return;
}

YeastTableModel::~YeastTableModel() = default;

void YeastTableModel::added  ([[maybe_unused]] std::shared_ptr<Yeast> item) { return; }
void YeastTableModel::removed([[maybe_unused]] std::shared_ptr<Yeast> item) { return; }
void YeastTableModel::updateTotals()                                        { return; }

QVariant YeastTableModel::data(QModelIndex const & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto const columnIndex = static_cast<YeastTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case YeastTableModel::ColumnIndex::Name:
      case YeastTableModel::ColumnIndex::Laboratory:
      case YeastTableModel::ColumnIndex::ProductId:
      case YeastTableModel::ColumnIndex::Type:
      case YeastTableModel::ColumnIndex::Form:
      case YeastTableModel::ColumnIndex::TotalInventory:
      case YeastTableModel::ColumnIndex::TotalInventoryType:
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   // Should never reach here
   return QVariant();
}

QVariant YeastTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   return QVariant();
}

Qt::ItemFlags YeastTableModel::flags(const QModelIndex & index) const {
   auto const columnIndex = static_cast<YeastTableModel::ColumnIndex>(index.column());
   if (columnIndex == YeastTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   if (columnIndex == YeastTableModel::ColumnIndex::TotalInventory) {
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
   }
   return Qt::ItemIsSelectable |
          (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool YeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

///   auto row = this->rows[index.row()];
   auto const columnIndex = static_cast<YeastTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case YeastTableModel::ColumnIndex::Name:
      case YeastTableModel::ColumnIndex::Laboratory:
      case YeastTableModel::ColumnIndex::ProductId:
      case YeastTableModel::ColumnIndex::Type:
      case YeastTableModel::ColumnIndex::Form:
      case YeastTableModel::ColumnIndex::TotalInventory:
      case YeastTableModel::ColumnIndex::TotalInventoryType:
         return this->writeDataToModel(index, value, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   // Should never reach here
   return false;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Yeast, yeast, PropertyNames::None::none)
//============================================== CLASS YeastItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Yeast)
