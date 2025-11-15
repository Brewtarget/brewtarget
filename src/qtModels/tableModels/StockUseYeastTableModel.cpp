/*======================================================================================================================
 * qtModels/tableModels/StockUseYeastTableModel.cpp is part of Brewtarget, and is copyright the following
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
#include "qtModels/tableModels/StockUseYeastTableModel.h"

#include "model/StockUseIngredient.h"
#include "qtModels/tableModels/BtTableModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockUseYeastTableModel.cpp"
#endif

COLUMN_INFOS(
   StockUseYeastTableModel,
   TABLE_MODEL_HEADER(StockUseYeast, Date           , PropertyNames::StockUse::date               ), // "Date"
   TABLE_MODEL_HEADER(StockUseYeast, Reason         , PropertyNames::StockUse::reason             ), // "Reason"
   TABLE_MODEL_HEADER(StockUseYeast, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ), // "Used"
   TABLE_MODEL_HEADER(StockUseYeast, Comment        , PropertyNames::StockUse::comment            ), // "Comment"
   TABLE_MODEL_HEADER(StockUseYeast, AmountRemaining, PropertyNames::StockUseBase::amountRemaining), // "Remaining"
)

StockUseYeastTableModel::StockUseYeastTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<StockUseYeastTableModel, StockUseYeast>{},
   EnumeratedItemTableModelBase<StockUseYeastTableModel, StockUseYeast, StockPurchaseYeast>{} {
   this->setObjectName("stockUseYeastTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &StockUseYeastTableModel::contextMenu);
   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

StockUseYeastTableModel::~StockUseYeastTableModel() = default;

void StockUseYeastTableModel::added  ([[maybe_unused]] std::shared_ptr<StockUseYeast> item) { return; }
void StockUseYeastTableModel::removed([[maybe_unused]] std::shared_ptr<StockUseYeast> item) { return; }
void StockUseYeastTableModel::updateTotals()                                      { return; }

QVariant StockUseYeastTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_itemOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

bool StockUseYeastTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(StockUseYeast, stockUseYeast, PropertyNames::None::none)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(StockUseYeast, StockPurchaseYeast)
//=============================================== CLASS StockUseYeastItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(StockUseYeast)
