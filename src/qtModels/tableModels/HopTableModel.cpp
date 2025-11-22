/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/HopTableModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "qtModels/tableModels/HopTableModel.h"

#include <QHeaderView>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/StockPurchaseHop.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_HopTableModel.cpp"
#endif

COLUMN_INFOS(
   HopTableModel,
   TABLE_MODEL_HEADER(Hop, Name            , PropertyNames::NamedEntity::name            ), // "Name"
   TABLE_MODEL_HEADER(Hop, Form            , PropertyNames::Hop::form                    ), // "Form"
   TABLE_MODEL_HEADER(Hop, Year            , PropertyNames::Hop::year                    ), // "Year"
   TABLE_MODEL_HEADER(Hop, Alpha           , PropertyNames::Hop::alpha_pct               ), // "Alpha %"
   TABLE_MODEL_HEADER(Hop, TotalInventory  , PropertyNames::Ingredient::totalInventory   ), // "Inventory"
   TABLE_MODEL_HEADER(Hop, NumRecipesUsedIn, PropertyNames::NamedEntity::numRecipesUsedIn), // "N° Recipes"
)

HopTableModel::HopTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<HopTableModel, Hop>{},
   showIBUs(false) {
   this->m_rows.clear();

   QHeaderView * headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &HopTableModel::contextMenu);
   connect(&ObjectStoreTyped<StockPurchaseHop>::getInstance(), &ObjectStoreTyped<StockPurchaseHop>::signalPropertyChanged, this,
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

QVariant HopTableModel::data(QModelIndex const & index, int role) const {
   return this->doDataDefault(index, role);
}

bool HopTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   // Template parameter is true as we might need to re-show header IBUs
   return this->doSetDataDefault<true>(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(Hop, hop, PropertyNames::None::none)
//=============================================== CLASS HopItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(Hop)
