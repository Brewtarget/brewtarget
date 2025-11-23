/*======================================================================================================================
 * qtModels/tableModels/StockUseYeastTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_INVENTORYCHANGEYEASTTABLEMODEL_H
#define TABLEMODELS_INVENTORYCHANGEYEASTTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/StockUseIngredient.h"
#include "model/StockPurchaseYeast.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StockUseYeastTableModel, Date           ,
                                      Reason         ,
                                      AmountUsed     ,
                                      Comment        ,
                                      AmountRemaining)

/*!
 * \class StockUseYeastTableModel
 *
 * \brief Model for the list of \c StockUseYeast items in an \c StockPurchaseYeast.
 */
class StockUseYeastTableModel : public BtTableModel,
                                public TableModelBase<StockUseYeastTableModel,
                                                      StockUseYeast>,
                                public EnumeratedItemTableModelBase<StockUseYeastTableModel,
                                                                    StockUseYeast,
                                                                    StockPurchaseYeast> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(StockUseYeast)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(StockUseYeast, StockPurchaseYeast)

};

//==================================== CLASS StockUseYeastItemDelegate ====================================

/**
 * \class StockUseYeastItemDelegate
 *
 * \brief An item delegate for \c StockUseYeastTableModel
 */
class StockUseYeastItemDelegate : public QStyledItemDelegate,
                                  public ItemDelegate<StockUseYeastItemDelegate,
                                                      StockUseYeastTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(StockUseYeast)
};

#endif
