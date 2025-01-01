/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/MiscTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TABLEMODELS_MISCTABLEMODEL_H
#define TABLEMODELS_MISCTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/InventoryMisc.h"
#include "model/Misc.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// MiscTableModel::ColumnIndex::Type etc.
class MiscTableModel;
template <> struct TableModelTraits<MiscTableModel> {
   enum class ColumnIndex {
      Name              ,
      Type              ,
///      Use               ,
///      Time              ,
      TotalInventory    ,
      TotalInventoryType,
   };
};

/*!
 * \class MiscTableModel
 *
 * \brief Table model for a list of miscs.
 */
class MiscTableModel : public BtTableModel, public TableModelBase<MiscTableModel, Misc> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(Misc)
};

//=============================================== CLASS MiscItemDelegate ===============================================
/*!
 * \class MiscItemDelegate
 *
 * \brief Item delegate for misc tables.
 * \sa MiscTableModel
 */
class MiscItemDelegate : public QItemDelegate,
                         public ItemDelegate<MiscItemDelegate, MiscTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(Misc)
};

#endif
