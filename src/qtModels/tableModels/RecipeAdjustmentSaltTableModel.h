/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/tableModels/RecipeAdjustmentSaltTableModel.h is part of Brewtarget, and is copyright the following authors
 * 2009-2025:
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
#ifndef TABLEMODELS_RECIPEADJUSTMENTSALTTABLEMODEL_H
#define TABLEMODELS_RECIPEADJUSTMENTSALTTABLEMODEL_H
#pragma once

#include <QItemDelegate>
#include <QList>
#include <QMetaProperty>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QVariant>
#include <QWidget>

#include "measurement/Unit.h"
#include "model/RecipeAdjustmentSalt.h"
#include "model/Water.h"
#include "qtModels/tableModels/BtTableModel.h"
#include "qtModels/tableModels/ItemDelegate.h"
#include "qtModels/tableModels/TableModelBase.h"

// Forward declarations.
class Mash;
class Recipe;

// Define the columns on this table
TABLE_MODEL_TRAITS(RecipeAdjustmentSalt, Name          ,
                                         Type          ,
                                         Amount        ,
                                         AmountType    ,
                                         TotalInventory,
                                         AddTo         ,
                                         PctAcid       ,)

/*!
 * \class RecipeAdjustmentSaltTableModel
 *
 * \brief Table model for salts.
 */
class RecipeAdjustmentSaltTableModel :
   public BtTableModelRecipeObserver,
   public TableModelBase<RecipeAdjustmentSaltTableModel, RecipeAdjustmentSalt> {
   Q_OBJECT

   TABLE_MODEL_COMMON_DECL(RecipeAdjustmentSalt)

public:
   double total_Ca()   const;
   double total_Cl()   const;
   double total_CO3()  const;
   double total_HCO3() const;
   double total_Mg()   const;
   double total_Na()   const;
   double total_SO4()  const;

   double total(Water::Ion const ion) const;
   Measurement::Amount total(Salt::Type const type) const;
   double totalAcidWeight(Salt::Type type) const;

   void saveAndClose();

public slots:
///   void catchSalt();

signals:
   void newTotals();

private:
///   double spargePct;
   double multiplier(RecipeAdjustmentSalt const & salt, bool const convertKilogramsToGrams = true) const;
};

//======================================= CLASS RecipeAdjustmentSaltItemDelegate =======================================

/*!
 * \brief An item delegate for RecipeAdjustmentSalt tables.
 * \sa RecipeAdjustmentSaltTableModel.
 */
class RecipeAdjustmentSaltItemDelegate :
   public QItemDelegate,
   public ItemDelegate<RecipeAdjustmentSaltItemDelegate, RecipeAdjustmentSaltTableModel> {
   Q_OBJECT

   ITEM_DELEGATE_COMMON_DECL(RecipeAdjustmentSalt)
};

#endif
