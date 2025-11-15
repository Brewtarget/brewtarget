/*======================================================================================================================
 * qtModels/tableModels/StockUseMiscTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_INVENTORYCHANGEMISCTABLEMODEL_H
#define TABLEMODELS_INVENTORYCHANGEMISCTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>

#include "model/StockUseIngredient.h"
#include "model/StockPurchaseMisc.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/EnumeratedItemTableModelBase.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Define the columns on this table
COLUMN_NAMES(StockUseMiscTableModel, Date           ,
                                     Reason         ,
                                     AmountUsed     ,
                                     Comment        ,
                                     AmountRemaining)

/*!
 * \class StockUseMiscTableModel
 *
 * \brief Model for the list of \c StockUseMisc items in an \c StockPurchaseMisc.
 */
class StockUseMiscTableModel : public BtTableModel,
                               public TableModelBase<StockUseMiscTableModel,
                                                     StockUseMisc>,
                               public EnumeratedItemTableModelBase<StockUseMiscTableModel,
                                                                   StockUseMisc,
                                                                   StockPurchaseMisc> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(StockUseMisc)
   ENUMERATED_ITEM_TABLE_MODEL_COMMON_DECL(StockUseMisc, StockPurchaseMisc)

};

//==================================== CLASS StockUseMiscItemDelegate ====================================

/**
 * \class StockUseMiscItemDelegate
 *
 * \brief An item delegate for \c StockUseMiscTableModel
 */
class StockUseMiscItemDelegate : public QStyledItemDelegate,
                                 public ItemDelegate<StockUseMiscItemDelegate,
                                                     StockUseMiscTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(StockUseMisc)
};

#endif
