/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdditionHopTableModel.h is part of Brewtarget, and is copyright the following authors
 * 2009-2025:
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

#include <QStyledItemDelegate>
#include <QMetaProperty>
#include <QModelIndex>
#include <QVariant>
#include <QWidget>

#include "model/RecipeAdditionHop.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

class BtStringConst;
class Recipe;

// Define the columns on this table
COLUMN_NAMES(RecipeAdditionHopTableModel, Name          ,
                                          Form          ,
                                          Alpha         ,
                                          Year          ,
                                          Amount        ,
                                          AmountType    ,
                                          TotalInventory,
                                          Stage         ,
                                          Time          ,)

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

   //! \brief Reimplemented from QAbstractTableModel.
   virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
   bool showIBUs; // True if you want to show the IBU contributions in the table rows.
};

//=============================================== CLASS RecipeAdditionHopItemDelegate ================================================

/**
 * \class RecipeAdditionHopItemDelegate
 *
 * \brief An item delegate for \c RecipeAdditionHopTableModel
 */
class RecipeAdditionHopItemDelegate : public QStyledItemDelegate,
                                      public ItemDelegate<RecipeAdditionHopItemDelegate, RecipeAdditionHopTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(RecipeAdditionHop)
};

#endif
