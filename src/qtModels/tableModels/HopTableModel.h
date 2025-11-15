/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/HopTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Markus Mårtensson <mackan.90@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#ifndef TABLEMODELS_HOPTABLEMODEL_H
#define TABLEMODELS_HOPTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/Hop.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// Define the columns on this table
COLUMN_NAMES(HopTableModel, Name              ,
                            Form              ,
                            Year              ,
                            Alpha             ,
                            TotalInventory    ,
///                            TotalInventoryType,
                            NumRecipesUsedIn  ,)

/*!
 * \class HopTableModel
 *
 * \brief Model class for a list of hops.
 */
class HopTableModel : public BtTableModel, public TableModelBase<HopTableModel, Hop> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Hop)

public:
   //! \brief Show IBUs in the vertical header.
   void setShowIBUs(bool var);

private:
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

//=============================================== CLASS HopItemDelegate ================================================

/**
 * \class HopItemDelegate
 *
 * \brief An item delegate for hop tables.
 * \sa HopTableModel
 */
class HopItemDelegate : public QStyledItemDelegate,
                        public ItemDelegate<HopItemDelegate, HopTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Hop)
};

#endif
