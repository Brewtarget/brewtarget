/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionYeastTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TABLEMODELS_RECIPEADDITIONYEASTTABLEMODEL_H
#define TABLEMODELS_RECIPEADDITIONYEASTTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/RecipeAdditionYeast.h"
#include "tableModels/BtTableModel.h"
#include "tableModels/ItemDelegate.h"
#include "tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// RecipeAdditionYeastTableModel::ColumnIndex::Alpha etc.
class RecipeAdditionYeastTableModel;
template <> struct TableModelTraits<RecipeAdditionYeastTableModel> {
   enum class ColumnIndex {
      Name          ,
      Laboratory    ,
      ProductId     ,
      Type          ,
      Form          ,
      Amount        ,
      AmountType    ,
      TotalInventory,
      Stage         ,
      Step          ,
      Attenuation   ,
   };
};

/*!
 * \class RecipeAdditionYeastTableModel
 *
 * \brief Model class for a list of yeast additions.
 *
 *        TBD: Maybe there is a way for this class and \c YeastTableModel to share more code.
 */
class RecipeAdditionYeastTableModel : public BtTableModelRecipeObserver,
                                      public TableModelBase<RecipeAdditionYeastTableModel, RecipeAdditionYeast> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(RecipeAdditionYeast)
};

//=============================================== CLASS RecipeAdditionYeastItemDelegate ================================================

/**
 * \class RecipeAdditionYeastItemDelegate
 *
 * \brief An item delegate for \c RecipeAdditionYeastTableModel
 */
class RecipeAdditionYeastItemDelegate : public QItemDelegate,
                                        public ItemDelegate<RecipeAdditionYeastItemDelegate, RecipeAdditionYeastTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(RecipeAdditionYeast)
};

#endif
