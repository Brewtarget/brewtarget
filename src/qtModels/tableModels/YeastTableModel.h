/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/YeastTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef TABLEMODELS_YEASTTABLEMODEL_H
#define TABLEMODELS_YEASTTABLEMODEL_H
#pragma once

#include <memory>

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/InventoryYeast.h"
#include "model/Yeast.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Forward declarations.
class BtStringConst;
class Recipe;

// Define the columns on this table
COLUMN_NAMES(YeastTableModel, Name              ,
                              Laboratory        ,
                              ProductId         ,
                              Type              ,
                              Form              ,
                              TotalInventory    ,
                              TotalInventoryType,
                              NumRecipesUsedIn  ,)

/*!
 * \class YeastTableModel
 *
 * \brief Table model for yeasts.
 */
class YeastTableModel : public BtTableModel, public TableModelBase<YeastTableModel, Yeast> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Yeast)
};

//============================================== CLASS YeastItemDelegate ===============================================

/*!
 * \class YeastItemDelegate
 *
 * \brief Item delegate for yeast tables.
 * \sa YeastTableModel
 */
class YeastItemDelegate : public QItemDelegate,
                          public ItemDelegate<YeastItemDelegate, YeastTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Yeast)
};

#endif
