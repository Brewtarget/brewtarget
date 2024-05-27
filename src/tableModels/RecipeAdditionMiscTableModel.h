/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * tableModels/RecipeAdditionMiscTableModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TABLEMODELS_RECIPEADDITIONMISCTABLEMODEL_H
#define TABLEMODELS_RECIPEADDITIONMISCTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/RecipeAdditionMisc.h"
#include "tableModels/BtTableModel.h"
#include "tableModels/ItemDelegate.h"
#include "tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// You have to get the order of everything right with traits classes, but the end result is that we can refer to
// RecipeAdditionMiscTableModel::ColumnIndex::Alpha etc.
class RecipeAdditionMiscTableModel;
template <> struct TableModelTraits<RecipeAdditionMiscTableModel> {
   enum class ColumnIndex {
      Name          ,
      Type          ,
      Amount        ,
      AmountType    ,
      TotalInventory,
      Stage         ,
      Time          ,
   };
};

/*!
 * \class RecipeAdditionMiscTableModel
 *
 * \brief Model class for a list of hop additions.
 *
 *        TBD: Maybe there is a way for this class and \c MiscTableModel to share more code.
 */
class RecipeAdditionMiscTableModel : public BtTableModelRecipeObserver,
                                     public TableModelBase<RecipeAdditionMiscTableModel, RecipeAdditionMisc> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(RecipeAdditionMisc)

public:
   //! \brief Show ibus in the vertical header.
   void setShowIBUs(bool var);

private:
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

//=============================================== CLASS RecipeAdditionMiscItemDelegate ================================================

/**
 * \class RecipeAdditionMiscItemDelegate
 *
 * \brief An item delegate for \c RecipeAdditionMiscTableModel
 */
class RecipeAdditionMiscItemDelegate : public QItemDelegate,
                                       public ItemDelegate<RecipeAdditionMiscItemDelegate, RecipeAdditionMiscTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(RecipeAdditionMisc)
};

#endif
