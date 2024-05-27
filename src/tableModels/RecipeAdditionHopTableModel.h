/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionHopTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2023:
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
#ifndef TABLEMODELS_RECIPEADDITIONHOPTABLEMODEL_H
#define TABLEMODELS_RECIPEADDITIONHOPTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/RecipeAdditionHop.h"
#include "tableModels/BtTableModel.h"
#include "tableModels/ItemDelegate.h"
#include "tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// RecipeAdditionHopTableModel::ColumnIndex::Alpha etc.
class RecipeAdditionHopTableModel;
template <> struct TableModelTraits<RecipeAdditionHopTableModel> {
   enum class ColumnIndex {
      Name          ,
      Form          ,
      Alpha         ,
      Year          ,
      Amount        ,
      AmountType    ,
      TotalInventory,
      Stage         ,
      Time          ,
   };
};

/*!
 * \class RecipeAdditionHopTableModel
 *
 * \brief Model class for a list of hop additions.
 *
 *        TBD: Maybe there is a way for this class and \c HopTableModel to share more code.
 */
class RecipeAdditionHopTableModel : public BtTableModelRecipeObserver,
                                    public TableModelBase<RecipeAdditionHopTableModel, RecipeAdditionHop> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(RecipeAdditionHop)

public:
   //! \brief Show ibus in the vertical header.
   void setShowIBUs(bool var);

private:
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

//=============================================== CLASS RecipeAdditionHopItemDelegate ================================================

/**
 * \class RecipeAdditionHopItemDelegate
 *
 * \brief An item delegate for \c RecipeAdditionHopTableModel
 */
class RecipeAdditionHopItemDelegate : public QItemDelegate,
                                      public ItemDelegate<RecipeAdditionHopItemDelegate, RecipeAdditionHopTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(RecipeAdditionHop)
};

#endif
