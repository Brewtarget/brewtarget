/*======================================================================================================================
 * qtModels/tableModels/StockUseFermentableTableModel.cpp is part of Brewtarget, and is copyright the following
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
#include "qtModels/tableModels/StockUseFermentableTableModel.h"

#include "model/StockUseIngredient.h"
#include "qtModels/tableModels/BtTableModel.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockUseFermentableTableModel.cpp"
#endif

COLUMN_INFOS(
   StockUseFermentableTableModel,
   TABLE_MODEL_HEADER(StockUseFermentable, Date           , PropertyNames::StockUse::date               ), // "Date"
   TABLE_MODEL_HEADER(StockUseFermentable, Reason         , PropertyNames::StockUse::reason             ), // "Reason"
   TABLE_MODEL_HEADER(StockUseFermentable, AmountUsed     , PropertyNames::StockUseBase::amountUsed     ), // "Used"
   TABLE_MODEL_HEADER(StockUseFermentable, Comment        , PropertyNames::StockUse::comment            ), // "Comment"
   TABLE_MODEL_HEADER(StockUseFermentable, AmountRemaining, PropertyNames::StockUseBase::amountRemaining), // "Remaining"
)

StockUseFermentableTableModel::StockUseFermentableTableModel(QTableView * parent, bool editable) :
   BtTableModel{parent, editable},
   TableModelBase<StockUseFermentableTableModel, StockUseFermentable>{},
   EnumeratedItemTableModelBase<StockUseFermentableTableModel, StockUseFermentable, StockPurchaseFermentable>{} {
   this->setObjectName("stockUseFermentableTableModel");

   QHeaderView* headerView = m_parentTableWidget->horizontalHeader();
   connect(headerView, &QWidget::customContextMenuRequested, this, &StockUseFermentableTableModel::contextMenu);
   //
   // See comment in qtModels/tableModels/BoilStepTableModel.cpp for why we don't listen directly to signals from
   // ObjectStore.
   //
   return;
}

StockUseFermentableTableModel::~StockUseFermentableTableModel() = default;

void StockUseFermentableTableModel::added  ([[maybe_unused]] std::shared_ptr<StockUseFermentable> item) { return; }
void StockUseFermentableTableModel::removed([[maybe_unused]] std::shared_ptr<StockUseFermentable> item) { return; }
void StockUseFermentableTableModel::updateTotals()                                      { return; }

QVariant StockUseFermentableTableModel::data(QModelIndex const & index, int role) const {
   if (!this->m_itemOwnerObs || !this->indexAndRoleOk(index, role)) {
      return QVariant();
   }

   // No other special handling required for any of our other columns
   return this->readDataFromModel(index, role);
}

bool StockUseFermentableTableModel::setData(QModelIndex const & index, QVariant const & value, int role) {
   if (!this->m_itemOwnerObs) {
      return false;
   }
   return this->doSetDataDefault(index, value, role);
}

// Insert the boiler-plate stuff that we cannot do in TableModelBase
TABLE_MODEL_COMMON_CODE(StockUseFermentable, stockUseFermentable, PropertyNames::None::none)
// Insert the boiler-plate stuff that we cannot do in StepTableModelBase
ENUMERATED_ITEM_TABLE_MODEL_COMMON_CODE(StockUseFermentable, StockPurchaseFermentable)
//=============================================== CLASS StockUseFermentableItemDelegate ================================================

// Insert the boiler-plate stuff that we cannot do in ItemDelegate
ITEM_DELEGATE_COMMON_CODE(StockUseFermentable)
