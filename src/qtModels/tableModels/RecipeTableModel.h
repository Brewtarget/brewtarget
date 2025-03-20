/*======================================================================================================================
 * qtModels/tableModels/RecipeTableModel.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TABLEMODELS_RECIPETABLEMODEL_H
#define TABLEMODELS_RECIPETABLEMODEL_H
#pragma once

#include <QItemDelegate>

#include "model/Recipe.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// RecipeTableModel::ColumnIndex::Name etc.
class RecipeTableModel;
template <> struct TableModelTraits<RecipeTableModel> {
   enum class ColumnIndex {
      Name,
   };
};

/*!
 * \class RecipeTableModel
 *
 * \brief Table model for a list of styles.
 */
class RecipeTableModel : public BtTableModel, public TableModelBase<RecipeTableModel, Recipe> {
   Q_OBJECT
   TABLE_MODEL_COMMON_DECL(Recipe)
};

//============================================== CLASS RecipeItemDelegate ===============================================
/*!
 * \class RecipeItemDelegate
 *
 * \brief Item delegate for style tables.
 * \sa RecipeTableModel
 */
class RecipeItemDelegate : public QItemDelegate,
                           public ItemDelegate<RecipeItemDelegate, RecipeTableModel> {
   Q_OBJECT
   ITEM_DELEGATE_COMMON_DECL(Recipe)
};

#endif
