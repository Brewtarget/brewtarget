/*
 * BtTreeFilterProxyModel.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "BtTreeFilterProxyModel.h"

#include <QDebug>

#include "BtFolder.h"
#include "BtTreeModel.h"
#include "BtTreeItem.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

namespace {

   template<class T> bool lessThan(BtTreeModel * model,
                                   QModelIndex const & left,
                                   QModelIndex const & right,
                                   T * lhs,
                                   T * rhs);

   template<> bool lessThan<Recipe>(BtTreeModel * model,
                                    QModelIndex const & left,
                                    QModelIndex const & right,
                                    Recipe * lhs,
                                    Recipe * rhs) {
      // Yog-Sothoth knows the gate
      // This reads soo much better
      if (model->showChild(left) && model->showChild(right)) {
         return lhs->key() > rhs->key();
      }

      switch (left.column()) {
         case BtTreeItem::RECIPENAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::RECIPEBREWDATECOL:
            return lhs->date() < rhs->date();
         case BtTreeItem::RECIPESTYLECOL:
            if (! lhs->style()) {
               return true;
            } else if (! rhs->style()) {
               return false;
            } else {
               return lhs->style()->name() < rhs->style()->name();
            }
      }
      // Default will be to just do a name sort. This doesn't likely make sense,
      // but it will prevent a lot of warnings.
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Equipment>(BtTreeModel * model,
                                       QModelIndex const & left,
                                       QModelIndex const & right,
                                       Equipment * lhs,
                                       Equipment * rhs) {
      switch (left.column()) {
         case BtTreeItem::EQUIPMENTNAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::EQUIPMENTBOILTIMECOL:
            return lhs->boilTime_min() < rhs->boilTime_min();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Fermentable>(BtTreeModel * model,
                                         QModelIndex const & left,
                                         QModelIndex const & right,
                                         Fermentable * lhs,
                                         Fermentable * rhs) {
      switch (left.column()) {
         case BtTreeItem::FERMENTABLENAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::FERMENTABLETYPECOL:
            return lhs->type() < rhs->type();
         case BtTreeItem::FERMENTABLECOLORCOL:
            return lhs->color_srm() < rhs->color_srm();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Hop>(BtTreeModel * model,
                                 QModelIndex const & left,
                                 QModelIndex const & right,
                                 Hop * lhs,
                                 Hop * rhs) {
      switch (left.column()) {
         case BtTreeItem::HOPNAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::HOPFORMCOL:
            return lhs->form() < rhs->form();
         case BtTreeItem::HOPUSECOL:
            return lhs->use() < rhs->use();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Misc>(BtTreeModel * model,
                                  QModelIndex const & left,
                                  QModelIndex const & right,
                                  Misc * lhs,
                                  Misc * rhs) {
      switch (left.column()) {
         case BtTreeItem::MISCNAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::MISCTYPECOL:
            return lhs->type() < rhs->type();
         case BtTreeItem::MISCUSECOL:
            return lhs->use() < rhs->use();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Style>(BtTreeModel * model,
                                   QModelIndex const & left,
                                   QModelIndex const & right,
                                   Style * lhs,
                                   Style * rhs) {
      switch (left.column()) {
         case BtTreeItem::STYLENAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::STYLECATEGORYCOL:
            return lhs->category() < rhs->category();
         case BtTreeItem::STYLENUMBERCOL:
            return lhs->categoryNumber() < rhs->categoryNumber();
         case BtTreeItem::STYLELETTERCOL:
            return lhs->styleLetter() < rhs->styleLetter();
         case BtTreeItem::STYLEGUIDECOL:
            return lhs->styleGuide() < rhs->styleGuide();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Yeast>(BtTreeModel * model,
                                   QModelIndex const & left,
                                   QModelIndex const & right,
                                   Yeast * lhs,
                                   Yeast * rhs) {
      switch (left.column()) {
         case BtTreeItem::YEASTNAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::YEASTTYPECOL:
            return lhs->type() < rhs->type();
         case BtTreeItem::YEASTFORMCOL:
            return lhs->form() < rhs->form();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Water>(BtTreeModel * model,
                                   QModelIndex const & left,
                                   QModelIndex const & right,
                                   Water * lhs,
                                   Water * rhs) {
      switch (left.column()) {
         case BtTreeItem::WATERNAMECOL:
            return lhs->name() < rhs->name();
         case BtTreeItem::WATERpHCOL:
            return lhs->ph() < rhs->ph();
         case BtTreeItem::WATERHCO3COL:
            return lhs->bicarbonate_ppm() < rhs->bicarbonate_ppm();
         case BtTreeItem::WATERSO4COL:
            return lhs->sulfate_ppm() < rhs->sulfate_ppm();
         case BtTreeItem::WATERCLCOL:
            return lhs->chloride_ppm() < rhs->chloride_ppm();
         case BtTreeItem::WATERNACOL:
            return lhs->sodium_ppm() < rhs->sodium_ppm();
         case BtTreeItem::WATERMGCOL:
            return lhs->magnesium_ppm() < rhs->magnesium_ppm();
         case BtTreeItem::WATERCACOL:
            return lhs->calcium_ppm() < rhs->calcium_ppm();
      }
      return lhs->name() < rhs->name();
   }

   template<class T>
   bool isLessThan(BtTreeModel * model,
                 QModelIndex const & left,
                 QModelIndex const & right) {
      // As the models get more complex, so does the sort algorithm
      // Try to sort folders first.
      if (model->type(left) == BtTreeItem::Type::FOLDER && model->type(right) == BtTreeItem::typeOf<T>()) {
         auto leftFolder = model->getItem<BtFolder>(left);
         auto rightTee = model->getItem<T>(right);

         return leftFolder->fullPath() < rightTee->name();
      }

      if (model->type(right) == BtTreeItem::Type::FOLDER && model->type(left) == BtTreeItem::typeOf<T>()) {
         auto rightFolder = model->getItem<BtFolder>(right);
         auto leftTee = model->getItem<T>(left);
         return leftTee->name() < rightFolder->fullPath();
      }

      if (model->type(right) == BtTreeItem::Type::FOLDER && model->type(left) == BtTreeItem::Type::FOLDER) {
         auto rightFolder = model->getItem<BtFolder>(right);
         auto leftFolder = model->getItem<BtFolder>(left);
         return leftFolder->fullPath() < rightFolder->fullPath();
      }

      return lessThan(model, left, right, model->getItem<T>(left), model->getItem<T>(right));
   }
}

BtTreeFilterProxyModel::BtTreeFilterProxyModel(QObject * parent,
                                               BtTreeModel::TypeMasks mask) :
   QSortFilterProxyModel{parent},
   treeMask{mask} {
   return;
}

bool BtTreeFilterProxyModel::lessThan(const QModelIndex & left,
                                      const QModelIndex & right) const {

   BtTreeModel * model = qobject_cast<BtTreeModel *>(sourceModel());
   switch (treeMask) {
      case BtTreeModel::RECIPEMASK:
         // We don't want to sort brewnotes with the recipes, so only do this if
         // both sides are brewnotes
         if (model->type(left) == BtTreeItem::Type::BREWNOTE || model->type(right) == BtTreeItem::Type::BREWNOTE) {
            BrewNote * leftBn = model->getItem<BrewNote>(left);
            BrewNote * rightBn = model->getItem<BrewNote>(right);
            if (leftBn && rightBn) {
               return leftBn->brewDate() < rightBn->brewDate();
            }
            return false;
         }
         return isLessThan<Recipe>(model, left, right);
      case BtTreeModel::EQUIPMASK:
         return isLessThan<Equipment>(model, left, right);
      case BtTreeModel::FERMENTMASK:
         return isLessThan<Fermentable>(model, left, right);
      case BtTreeModel::HOPMASK:
         return isLessThan<Hop>(model, left, right);
      case BtTreeModel::MISCMASK:
         return isLessThan<Misc>(model, left, right);
      case BtTreeModel::YEASTMASK:
         return isLessThan<Yeast>(model, left, right);
      case BtTreeModel::STYLEMASK:
         return isLessThan<Style>(model, left, right);
      case BtTreeModel::WATERMASK:
         return isLessThan<Water>(model, left, right);
      default:
         return isLessThan<Recipe>(model, left, right);

   }
}

bool BtTreeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const {
   if (!source_parent.isValid()) {
      return true;
   }

   const BtTreeModel * model = qobject_cast<const BtTreeModel *>(source_parent.model());

   QModelIndex child = model->index(source_row, 0, source_parent);

   // We shouldn't get here, but if we cannot find the row in the parent,
   // don't display the item.
   if (! child.isValid()) {
      return false;
   }

   if (model->itemIs<BtFolder>(child)) {
      return true;
   }

   NamedEntity * thing = model->thing(child);

   if (treeMask == BtTreeModel::RECIPEMASK && thing) {

      // we are showing the child (context menu -> show snapshots ) OR
      // we are meant to display this thing.
      return model->showChild(child) || thing->display();
   }

   if (thing) {
      return thing->display();
   } else {
      return true;
   }

}
