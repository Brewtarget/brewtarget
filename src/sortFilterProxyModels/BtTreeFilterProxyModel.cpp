/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * sortFilterProxyModels/BtTreeFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2024:
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
#include "sortFilterProxyModels/BtTreeFilterProxyModel.h"

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

   template<> bool lessThan<Recipe>([[maybe_unused]] BtTreeModel * model,
                                    QModelIndex const & left,
                                    [[maybe_unused]] QModelIndex const & right,
                                    Recipe * lhs,
                                    Recipe * rhs) {
      // Yog-Sothoth knows the gate
      // This reads soo much better
      if (model->showChild(left) && model->showChild(right)) {
         return lhs->key() > rhs->key();
      }

      switch (static_cast<BtTreeItem::RecipeColumn>(left.column())) {
         case BtTreeItem::RecipeColumn::Name:
            return lhs->name() < rhs->name();
         case BtTreeItem::RecipeColumn::BrewDate:
            return lhs->date() < rhs->date();
         case BtTreeItem::RecipeColumn::Style:
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

   template<> bool lessThan<Equipment>([[maybe_unused]] BtTreeModel * model,
                                       QModelIndex const & left,
                                       [[maybe_unused]] QModelIndex const & right,
                                       Equipment * lhs,
                                       Equipment * rhs) {
      switch (static_cast<BtTreeItem::EquipmentColumn>(left.column())) {
         case BtTreeItem::EquipmentColumn::Name:
            return lhs->name() < rhs->name();
         case BtTreeItem::EquipmentColumn::BoilTime:
            return lhs->boilTime_min().value_or(Equipment::default_boilTime_mins) < rhs->boilTime_min().value_or(Equipment::default_boilTime_mins);
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Fermentable>([[maybe_unused]] BtTreeModel * model,
                                         QModelIndex const & left,
                                         [[maybe_unused]] QModelIndex const & right,
                                         Fermentable * lhs,
                                         Fermentable * rhs) {
      switch (static_cast<BtTreeItem::FermentableColumn>(left.column())) {
         case BtTreeItem::FermentableColumn::Name : return lhs->name()      < rhs->name();
         case BtTreeItem::FermentableColumn::Type : return lhs->type()      < rhs->type();
         case BtTreeItem::FermentableColumn::Color: return lhs->color_srm() < rhs->color_srm();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Hop>([[maybe_unused]] BtTreeModel * model,
                                 QModelIndex const & left,
                                 [[maybe_unused]] QModelIndex const & right,
                                 Hop * lhs,
                                 Hop * rhs) {
      switch (static_cast<BtTreeItem::HopColumn>(left.column())) {
         case BtTreeItem::HopColumn::Name    : return lhs->name()      < rhs->name();
         case BtTreeItem::HopColumn::Form    : return lhs->form()      < rhs->form();
         case BtTreeItem::HopColumn::AlphaPct: return lhs->alpha_pct() < rhs->alpha_pct();
         case BtTreeItem::HopColumn::Origin  : return lhs->origin()    < rhs->origin();
         default:
            return lhs->name() < rhs->name();
      }
      // Unreachable
   }

   template<> bool lessThan<Misc>([[maybe_unused]] BtTreeModel * model,
                                  QModelIndex const & left,
                                  [[maybe_unused]] QModelIndex const & right,
                                  Misc * lhs,
                                  Misc * rhs) {
      switch (static_cast<BtTreeItem::MiscColumn>(left.column())) {
         case BtTreeItem::MiscColumn::Name: return lhs->name() < rhs->name();
         case BtTreeItem::MiscColumn::Type: return lhs->type() < rhs->type();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Style>([[maybe_unused]] BtTreeModel * model,
                                   QModelIndex const & left,
                                   [[maybe_unused]] QModelIndex const & right,
                                   Style * lhs,
                                   Style * rhs) {
      switch (static_cast<BtTreeItem::StyleColumn>(left.column())) {
         case BtTreeItem::StyleColumn::Name          : return lhs->name()           < rhs->name();
         case BtTreeItem::StyleColumn::Category      : return lhs->category()       < rhs->category();
         case BtTreeItem::StyleColumn::CategoryNumber: return lhs->categoryNumber() < rhs->categoryNumber();
         case BtTreeItem::StyleColumn::CategoryLetter: return lhs->styleLetter()    < rhs->styleLetter();
         case BtTreeItem::StyleColumn::StyleGuide    : return lhs->styleGuide()     < rhs->styleGuide();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Yeast>([[maybe_unused]] BtTreeModel * model,
                                   QModelIndex const & left,
                                   [[maybe_unused]] QModelIndex const & right,
                                   Yeast * lhs,
                                   Yeast * rhs) {
      switch (static_cast<BtTreeItem::YeastColumn>(left.column())) {
         case BtTreeItem::YeastColumn::Name: return lhs->name() < rhs->name();
         case BtTreeItem::YeastColumn::Type: return lhs->type() < rhs->type();
         case BtTreeItem::YeastColumn::Form: return lhs->form() < rhs->form();
      }
      return lhs->name() < rhs->name();
   }

   template<> bool lessThan<Water>([[maybe_unused]] BtTreeModel * model,
                                   QModelIndex const & left,
                                   [[maybe_unused]] QModelIndex const & right,
                                   Water * lhs,
                                   Water * rhs) {
      switch (static_cast<BtTreeItem::WaterColumn>(left.column())) {
         case BtTreeItem::WaterColumn::Name       : return lhs->name()            < rhs->name();
         case BtTreeItem::WaterColumn::pH         : return lhs->ph()              < rhs->ph();
         case BtTreeItem::WaterColumn::Bicarbonate: return lhs->bicarbonate_ppm() < rhs->bicarbonate_ppm();
         case BtTreeItem::WaterColumn::Sulfate    : return lhs->sulfate_ppm()     < rhs->sulfate_ppm();
         case BtTreeItem::WaterColumn::Chloride   : return lhs->chloride_ppm()    < rhs->chloride_ppm();
         case BtTreeItem::WaterColumn::Sodium     : return lhs->sodium_ppm()      < rhs->sodium_ppm();
         case BtTreeItem::WaterColumn::Magnesium  : return lhs->magnesium_ppm()   < rhs->magnesium_ppm();
         case BtTreeItem::WaterColumn::Calcium    : return lhs->calcium_ppm()     < rhs->calcium_ppm();
      }
      return lhs->name() < rhs->name();
   }

   template<class T>
   bool isLessThan(BtTreeModel * model,
                 QModelIndex const & left,
                 QModelIndex const & right) {
      // As the models get more complex, so does the sort algorithm
      // Try to sort folders first.
      if (model->type(left) == BtTreeItem::Type::Folder && model->type(right) == BtTreeItem::typeOf<T>()) {
         auto leftFolder = model->getItem<BtFolder>(left);
         auto rightTee = model->getItem<T>(right);

         return leftFolder->fullPath() < rightTee->name();
      }

      if (model->type(right) == BtTreeItem::Type::Folder && model->type(left) == BtTreeItem::typeOf<T>()) {
         auto rightFolder = model->getItem<BtFolder>(right);
         auto leftTee = model->getItem<T>(left);
         return leftTee->name() < rightFolder->fullPath();
      }

      if (model->type(right) == BtTreeItem::Type::Folder && model->type(left) == BtTreeItem::Type::Folder) {
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
   m_treeMask{mask} {
   return;
}

bool BtTreeFilterProxyModel::lessThan(const QModelIndex & left,
                                      const QModelIndex & right) const {

   BtTreeModel * model = qobject_cast<BtTreeModel *>(sourceModel());

   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Recipe)) {
      // We don't want to sort brewnotes with the recipes, so only do this if
      // both sides are brewnotes
      if (model->type(left) == BtTreeItem::Type::BrewNote || model->type(right) == BtTreeItem::Type::BrewNote) {
         BrewNote * leftBn = model->getItem<BrewNote>(left);
         BrewNote * rightBn = model->getItem<BrewNote>(right);
         if (leftBn && rightBn) {
            return leftBn->brewDate() < rightBn->brewDate();
         }
         return false;
      }
      return isLessThan<Recipe>(model, left, right);
   }

   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Equipment  )) { return isLessThan<Equipment  >(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Fermentable)) { return isLessThan<Fermentable>(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Hop        )) { return isLessThan<Hop        >(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Misc       )) { return isLessThan<Misc       >(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Yeast      )) { return isLessThan<Yeast      >(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Style      )) { return isLessThan<Style      >(model, left, right); }
   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Water      )) { return isLessThan<Water      >(model, left, right); }
   return isLessThan<Recipe>(model, left, right);
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

   if (this->m_treeMask.testFlag(BtTreeModel::TypeMask::Recipe) && thing) {

      // we are showing the child (context menu -> show snapshots ) OR
      // we are meant to display this thing.
      return model->showChild(child) || thing->display();
   }

   if (thing) {
      return thing->display();
   }

   return true;

}
