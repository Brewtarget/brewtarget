/*======================================================================================================================
 * qtModels/tableModels/StockUseHopTableModel.cpp is part of Brewtarget, and is copyright the following
 * authors 2025:
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
#include "qtModels/tableModels/StockUseHopTableModel.h"

#include "model/StockUseIngredient.h"
#include "qtModels/tableModels/BtTableModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockUseHopTableModel.cpp"
#endif

COLUMN_INFOS(
   StockUseHopTableModel,
   TABLE_MODEL_HEADER(StockUseHop, Date           , PropertyNames::StockUse::date               ), // "Date"
   TABLE_MODEL_HEADER(StockUseHop, Reason         , PropertyNames::StockUse::reason             ), // "Reason"
   TABLE_MODEL_HEADER(StockUseHop, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ), // "Used"
   TABLE_MODEL_HEADER(StockUseHop, Comment        , PropertyNames::StockUse::comment            ), // "Comment"
   TABLE_MODEL_HEADER(StockUseHop, AmountRemaining, PropertyNames::StockUseBase::amountRemaining), // "Remaining"
)

StockUseHopTableModel::StockUseHopTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<StockUseHopTableModel, StockUseHop>{},
   EnumeratedItemTableModelBase<StockUseHopTableModel, StockUseHop, StockPurchaseHop>{} {
   this->setObjectName("stockUseHopTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &StockUseHopTableModel::contextMenu);
   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

StockUseHopTableModel::~StockUseHopTableModel() = default;

void StockUseHopTableModel::added  ([[maybe_unused]] std::shared_ptr<StockUseHop> item) { return; }
void StockUseHopTableModel::removed([[maybe_unused]] std::shared_ptr<StockUseHop> item) { return; }
void StockUseHopTableModel::updateTotals()                                      { return; }

QVariant StockUseHopTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_itemOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

bool StockUseHopTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(StockUseHop, stockUseHop, PropertyNames::None::none)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(StockUseHop, StockPurchaseHop)
//=============================================== CLASS StockUseHopItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(StockUseHop)
