/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/WaterTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef TABLEMODELS_WATERTABLEMODEL_H
#define TABLEMODELS_WATERTABLEMODEL_H
#pragma once

#include <memory>

#include <QStyledItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/Water.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Forward declarations.
class Water;
class Recipe;
class RecipeUseOfWater;

class WaterItemDelegate;

// Define the columns on this table
COLUMN_NAMES(WaterTableModel, Name            ,
                              Calcium         ,
                              Bicarbonate     ,
                              Sulfate         ,
                              Chloride        ,
                              Sodium          ,
                              Magnesium       ,
                              NumRecipesUsedIn,)

/*!
 * \class WaterTableModel
 *
 * \brief Table model for waters.
 */
class WaterTableModel : public BtTableModel, public TableModelBase<WaterTableModel, Water> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Water)
};

//=============================================== CLASS WaterItemDelegate ===============================================

/*!
 * \brief An item delegate for Water tables.
 * \sa WaterTableModel.
 */
class WaterItemDelegate : public QStyledItemDelegate,
                          public ItemDelegate<WaterItemDelegate, WaterTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Water)
};

#endif
