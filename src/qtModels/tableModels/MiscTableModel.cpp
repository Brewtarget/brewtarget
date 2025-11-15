/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MiscTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "qtModels/tableModels/MiscTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/StockPurchaseMisc.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_MiscTableModel.cpp"
#endif

COLUMN_INFOS(
   MiscTableModel,
   TABLE_MODEL_HEADER(Misc, Name              , PropertyNames::NamedEntity::name            ), // "Name"
   TABLE_MODEL_HEADER(Misc, Type              , PropertyNames::Misc::type                   ), // "Type"
   TABLE_MODEL_HEADER(Misc, TotalInventory    , PropertyNames::Ingredient::totalInventory   ), // "Inventory"
///   TABLE_MODEL_HEADER(Misc, TotalInventoryType, PropertyNames::Ingredient::totalInventory   , Misc::validMeasures), // "Amount Type"
   TABLE_MODEL_HEADER(Misc, NumRecipesUsedIn  , PropertyNames::NamedEntity::numRecipesUsedIn), // "N° Recipes"
)

MiscTableModel::MiscTableModel(QTableView* parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<MiscTableModel, Misc>{} {
   this->m_rows.clear();

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &MiscTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseMisc>::getInstance(),
           &ObjectStoreTyped<StockPurchaseMisc>::signalPropertyChanged,
           this,
           &MiscTableModel::changedInventory);
   return;
}

MiscTableModel::~MiscTableModel() = default;

void MiscTableModel::added  ([[maybe_unused]] std::shared_ptr<Misc> item) { return; }
void MiscTableModel::removed([[maybe_unused]] std::shared_ptr<Misc> item) { return; }
void MiscTableModel::updateTotals()                                       { return; }

QVariant MiscTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

bool MiscTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Misc, misc, PropertyNames::None::none)
//=============================================== CLASS MiscItemDelegate ===============================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Misc)
