/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/HopTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2023:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Luke Vincent <luke.r.vincent@gmail.com>
 *   • Markus Mårtensson <mackan.90@gmail.com>
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
#include "tableModels/HopTableModel.h"

#include <QAbstractItemModel>
#include <QComboBox>
#include <QHeaderView>
#include <QItemDelegate>
#include <QLineEdit>
#include <QModelIndex>
#include <QString>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Inventory.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"
#include "utils/BtStringConst.h"

HopTableModel::HopTableModel(QTableView * parent, bool editable) :
   BtTableModel{
      parent,
      editable,
      {
         TABLE_MODEL_HEADER(Hop, Name              , tr("Name"       ), PropertyNames::NamedEntity::name         ),
         TABLE_MODEL_HEADER(Hop, Form              , tr("Form"       ), PropertyNames::Hop::form                 , EnumInfo{Hop::formStringMapping, Hop::formDisplayNames}),
         TABLE_MODEL_HEADER(Hop, Year              , tr("Year"       ), PropertyNames::Hop::year                 ),
         TABLE_MODEL_HEADER(Hop, Alpha             , tr("Alpha %"    ), PropertyNames::Hop::alpha_pct            , PrecisionInfo{1}),
         TABLE_MODEL_HEADER(Hop, TotalInventory    , tr("Inventory"  ), PropertyNames::Ingredient::totalInventory, PrecisionInfo{1}),
         TABLE_MODEL_HEADER(Hop, TotalInventoryType, tr("Amount Type"), PropertyNames::Ingredient::totalInventory, Hop::validMeasures),
      }
   },
   TableModelBase<HopTableModel, Hop>{},
   showIBUs(false) {
   this->rows.clear();
   this->setObjectName("hopTable");

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &HopTableModel::contextMenu);
   connect(&ObjectStoreTyped<InventoryHop>::getInstance(), &ObjectStoreTyped<InventoryHop>::signalPropertyChanged, this,
           &HopTableModel::changedInventory);
   return;
}

HopTableModel::~HopTableModel() = default;

void HopTableModel::added  ([[maybe_unused]] std::shared_ptr<Hop> item) { return; }
void HopTableModel::removed([[maybe_unused]] std::shared_ptr<Hop> item) { return; }
void HopTableModel::updateTotals()                                      { return; }

void HopTableModel::setShowIBUs(bool var) {
   showIBUs = var;
}

QVariant HopTableModel::data(const QModelIndex & index, int role) const {
   if (!this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No special handling required for any of our columns
   return this->readDataFromModel(index, role);
}

Qt::ItemFlags HopTableModel::flags(const QModelIndex & index) const {
   auto const columnIndex = static_cast<HopTableModel::ColumnIndex>(index.column());
   if (columnIndex == HopTableModel::ColumnIndex::Name) {
      return Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
   }
   if (columnIndex == HopTableModel::ColumnIndex::TotalInventory) {
      return Qt::ItemIsEnabled | Qt::ItemIsEditable;
   }
   return Qt::ItemIsSelectable |
          (this->m_editable ? Qt::ItemIsEditable : Qt::NoItemFlags) | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled;
}

bool HopTableModel::setData(const QModelIndex & index, const QVariant & value, int role) {
   if (!this->indexAndRoleOk(index, role)) {
      return false;
   }

   // No special handling required for any of our columns...
   bool const retVal = this->writeDataToModel(index, value, role);

   // ...but we might need to re-show header IBUs
   if (retVal) {
      headerDataChanged(Qt::Vertical, index.row(), index.row());
   }

   return retVal;
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Hop, hop, PropertyNames::None::none)
//=============================================== CLASS HopItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Hop)
