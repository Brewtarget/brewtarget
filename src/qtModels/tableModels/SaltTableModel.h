/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/SaltTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#ifndef TABLEMODELS_SALTTABLEMODEL_H
#define TABLEMODELS_SALTTABLEMODEL_H
#pragma once

#include <QStyledItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/Salt.h"
#include "model/Water.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Forward declarations.
class Mash;
class Recipe;

// Define the columns on this table
COLUMN_NAMES(SaltTableModel, Name            ,
                             Type            ,
                             PctAcid         ,
                             TotalInventory  ,
                             NumRecipesUsedIn,)

/*!
 * \class SaltTableModel
 *
 * \brief Table model for salts.
 */
class SaltTableModel : public BtTableModel, public TableModelBase<SaltTableModel, Salt> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Salt)

};

//=============================================== CLASS SaltItemDelegate ===============================================

/*!
 * \brief An item delegate for Salt tables.
 * \sa SaltTableModel.
 */
class SaltItemDelegate : public QStyledItemDelegate,
                         public ItemDelegate<SaltItemDelegate, SaltTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Salt)
};

#endif
