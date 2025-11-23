/*======================================================================================================================
 * qtModels/tableModels/StockUseHopTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_INVENTORYCHANGEHOPTABLEMODEL_H
#define TABLEMODELS_INVENTORYCHANGEHOPTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/StockUseIngredient.h"
#include "model/StockPurchaseHop.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StockUseHopTableModel, Date           ,
                                    Reason         ,
                                    AmountUsed     ,
                                    Comment        ,
                                    AmountRemaining)

/*!
 * \class StockUseHopTableModel
 *
 * \brief Model for the list of \c StockUseHop items in an \c StockPurchaseHop.
 */
class StockUseHopTableModel : public BtTableModel,
                              public TableModelBase<StockUseHopTableModel,
                                                    StockUseHop>,
                              public EnumeratedItemTableModelBase<StockUseHopTableModel,
                                                                  StockUseHop,
                                                                  StockPurchaseHop> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(StockUseHop)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(StockUseHop, StockPurchaseHop)

};

//==================================== CLASS StockUseHopItemDelegate ====================================

/**
 * \class StockUseHopItemDelegate
 *
 * \brief An item delegate for \c StockUseHopTableModel
 */
class StockUseHopItemDelegate : public QStyledItemDelegate,
                                public ItemDelegate<StockUseHopItemDelegate,
                                                    StockUseHopTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(StockUseHop)
};

#endif
