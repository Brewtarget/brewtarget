/*======================================================================================================================
 * qtModels/tableModels/StockUseFermentableTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_INVENTORYCHANGEFERMENTABLETABLEMODEL_H
#define TABLEMODELS_INVENTORYCHANGEFERMENTABLETABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/StockUseIngredient.h"
#include "model/StockPurchaseFermentable.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StockUseFermentableTableModel, Date           ,
                                            Reason         ,
                                            AmountUsed     ,
                                            Comment        ,
                                            AmountRemaining)

/*!
 * \class StockUseFermentableTableModel
 *
 * \brief Model for the list of \c StockUseFermentable items in an \c StockPurchaseFermentable.
 */
class StockUseFermentableTableModel : public BtTableModel,
                                      public TableModelBase<StockUseFermentableTableModel,
                                                            StockUseFermentable>,
                                      public EnumeratedItemTableModelBase<StockUseFermentableTableModel,
                                                                          StockUseFermentable,
                                                                          StockPurchaseFermentable> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(StockUseFermentable)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(StockUseFermentable, StockPurchaseFermentable)

};

//==================================== CLASS StockUseFermentableItemDelegate ====================================

/**
 * \class StockUseFermentableItemDelegate
 *
 * \brief An item delegate for \c StockUseFermentableTableModel
 */
class StockUseFermentableItemDelegate : public QStyledItemDelegate,
                                        public ItemDelegate<StockUseFermentableItemDelegate,
                                                            StockUseFermentableTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(StockUseFermentable)
};

#endif
