/*======================================================================================================================
 * qtModels/tableModels/StockUseSaltTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_INVENTORYCHANGESALTTABLEMODEL_H
#define TABLEMODELS_INVENTORYCHANGESALTTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/StockUseIngredient.h"
#include "model/StockPurchaseSalt.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StockUseSaltTableModel, Date           ,
                                     Reason         ,
                                     AmountUsed     ,
                                     Comment        ,
                                     AmountRemaining)

/*!
 * \class StockUseSaltTableModel
 *
 * \brief Model for the list of \c StockUseSalt items in an \c StockPurchaseSalt.
 */
class StockUseSaltTableModel : public BtTableModel,
                               public TableModelBase<StockUseSaltTableModel,
                                                     StockUseSalt>,
                               public EnumeratedItemTableModelBase<StockUseSaltTableModel,
                                                                   StockUseSalt,
                                                                   StockPurchaseSalt> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(StockUseSalt)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(StockUseSalt, StockPurchaseSalt)

};

//==================================== CLASS StockUseSaltItemDelegate ====================================

/**
 * \class StockUseSaltItemDelegate
 *
 * \brief An item delegate for \c StockUseSaltTableModel
 */
class StockUseSaltItemDelegate : public QStyledItemDelegate,
                                 public ItemDelegate<StockUseSaltItemDelegate,
                                                     StockUseSaltTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(StockUseSalt)
};

#endif
