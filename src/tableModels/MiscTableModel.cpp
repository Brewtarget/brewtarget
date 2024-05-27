/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/MiscTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "tableModels/MiscTableModel.h"

#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>

#include "database/ObjectStoreWrapper.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"
#include "widgets/BtComboBox.h"

MiscTableModel::MiscTableModel(QTableView* parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         // NOTE: Need PropertyNames::Fermentable::amountWithUnits not PropertyNames::Fermentable::amount below so we
         //       can handle mass-or-volume generically in TableModelBase.  Same for inventoryWithUnits.
         TABLE_MODEL_HEADER(Misc, Name              , tr("Name"       ), PropertyNames::NamedEntity::name                           ),
         TABLE_MODEL_HEADER(Misc, Type              , tr("Type"       ), PropertyNames::Misc::type                                  , EnumInfo{Misc::typeStringMapping, Misc::typeDisplayNames}),
///         TABLE_MODEL_HEADER(Misc, Use               , tr("Use"        ), PropertyNames::Misc::use                                   , EnumInfo{Misc:: useStringMapping, Misc:: useDisplayNames}),
///         TABLE_MODEL_HEADER(Misc, Time              , tr("Time"       ), PropertyNames::Misc::time_min                              ),
         TABLE_MODEL_HEADER(Misc, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(Misc, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory, Misc::validMeasures),
      }
   },
   TableModelBase<MiscTableModel, Misc>{} {
   this->rows.clear();
   setObjectName("miscTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryMisc>::getInstance(),
           &ObjectStoreTyped<InventoryMisc>::signalPropertyChanged,
           this,
           &MiscTableModel::changedInventory);
   return;
}

MiscTableModel::~MiscTableModel() = default;

void MiscTableModel::added  ([[maybe_unused]] std::shared_ptr<Misc> item) { return; }
void MiscTableModel::removed([[maybe_unused]] std::shared_ptr<Misc> item) { return; }
void MiscTableModel::updateTotals()                                       { return; }

QVariant MiscTableModel::data(QModelIndex const & index, int role) const {
   if (!this->isIndexOk(index)) {
      return QVariant();
   }

   auto row = this->rows[index.row()];

   // Deal with the column and return the right data.
   auto const columnIndex = static_cast<MiscTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case MiscTableModel::ColumnIndex::Name:
      case MiscTableModel::ColumnIndex::Type:
///      case MiscTableModel::ColumnIndex::Use:
///      case MiscTableModel::ColumnIndex::Time:
      case MiscTableModel::ColumnIndex::TotalInventory    :
      case MiscTableModel::ColumnIndex::TotalInventoryType:
         return this->readDataFromModel(index, role);

      // No default case as we want the compiler to warn us if we missed one
   }
   return QVariant();
}

QVariant MiscTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      return this->getColumnLabel(section);
   }
   return QVariant();
}

Qt::ItemFlags MiscTableModel::flags(QModelIndex const & index) const {
   Qt::ItemFlags const defaults = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
   auto const columnIndex = static_cast<MiscTableModel::ColumnIndex>(index.column());
   if (columnIndex == MiscTableModel::ColumnIndex::Name) {
      return defaults;
   }
   if (columnIndex == MiscTableModel::ColumnIndex::TotalInventory) {
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
   }
   return defaults | (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags);
}

bool MiscTableModel::setData(QModelIndex const & index,
                             QVariant const & value,
                             [[maybe_unused]] int role) {
   if (!this->isIndexOk(index)) {
      return false;
   }

///   auto row = this->rows[index.row()];

   auto const columnIndex = static_cast<MiscTableModel::ColumnIndex>(index.column());
   switch (columnIndex) {
      case MiscTableModel::ColumnIndex::Name:
      case MiscTableModel::ColumnIndex::Type:
///      case MiscTableModel::ColumnIndex::Use:
///      case MiscTableModel::ColumnIndex::Time:
      case MiscTableModel::ColumnIndex::TotalInventory    :
      case MiscTableModel::ColumnIndex::TotalInventoryType:
         return this->writeDataToModel(index, value, role);

      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   emit dataChanged(index, index);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Misc, misc, PropertyNames::None::none)
//=============================================== CLASS MiscItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Misc)
