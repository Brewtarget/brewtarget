/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "trees/TreeFilterProxyModel.h"

#include <QDebug>

#include "model/BrewNote.h"
#include "model/Folder.h"
#include "trees/TreeModel.h"
#include "trees/TreeNode.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"

namespace {

   template<class T> bool lessThan(TreeModel * model,
                                   QModelIndex const & left,
                                   QModelIndex const & right,
                                   T * lhs,
                                   T * rhs) {
      return TreeItemNode<T>::lessThan(*model, left, right, *lhs, *rhs);
   }

   template<class T>
   bool isLessThan(TreeModel * model,
                 QModelIndex const & left,
                 QModelIndex const & right) {
      // As the models get more complex, so does the sort algorithm
      // Try to sort folders first.
      if (model->type(left) == TreeNode::Type::Folder && model->type(right) == TreeNode::typeOf<T>()) {
         auto leftFolder = model->getItem<Folder>(left);
         auto rightTee = model->getItem<T>(right);

         return leftFolder->fullPath() < rightTee->name();
      }

      if (model->type(right) == TreeNode::Type::Folder && model->type(left) == TreeNode::typeOf<T>()) {
         auto rightFolder = model->getItem<Folder>(right);
         auto leftTee = model->getItem<T>(left);
         return leftTee->name() < rightFolder->fullPath();
      }

      if (model->type(right) == TreeNode::Type::Folder && model->type(left) == TreeNode::Type::Folder) {
         auto rightFolder = model->getItem<Folder>(right);
         auto leftFolder = model->getItem<Folder>(left);
         return leftFolder->fullPath() < rightFolder->fullPath();
      }

      return lessThan(model, left, right, model->getItem<T>(left), model->getItem<T>(right));
   }
}

TreeFilterProxyModel::TreeFilterProxyModel(QObject * parent,
                                           TreeModel::TypeMasks mask) :
   QSortFilterProxyModel{parent},
   m_treeMask{mask} {
   return;
}

bool TreeFilterProxyModel::lessThan(const QModelIndex & left,
                                      const QModelIndex & right) const {

   TreeModel * model = qobject_cast<TreeModel *>(sourceModel());

   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Recipe)) {
      // We don't want to sort brewnotes with the recipes, so only do this if
      // both sides are brewnotes
      if (model->type(left) == TreeNode::Type::BrewNote || model->type(right) == TreeNode::Type::BrewNote) {
         BrewNote * leftBn = model->getItem<BrewNote>(left);
         BrewNote * rightBn = model->getItem<BrewNote>(right);
         if (leftBn && rightBn) {
            return leftBn->brewDate() < rightBn->brewDate();
         }
         return false;
      }
      return isLessThan<Recipe>(model, left, right);
   }

   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Equipment  )) { return isLessThan<Equipment  >(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Fermentable)) { return isLessThan<Fermentable>(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Hop        )) { return isLessThan<Hop        >(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Misc       )) { return isLessThan<Misc       >(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Yeast      )) { return isLessThan<Yeast      >(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Style      )) { return isLessThan<Style      >(model, left, right); }
   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Water      )) { return isLessThan<Water      >(model, left, right); }
   return isLessThan<Recipe>(model, left, right);
}

bool TreeFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent) const {
   if (!source_parent.isValid()) {
      return true;
   }

   const TreeModel * model = qobject_cast<const TreeModel *>(source_parent.model());

   QModelIndex child = model->index(source_row, 0, source_parent);

   // We shouldn't get here, but if we cannot find the row in the parent,
   // don't display the item.
   if (! child.isValid()) {
      return false;
   }

   if (model->itemIs<Folder>(child)) {
      return true;
   }

   NamedEntity * thing = model->thing(child);

   if (this->m_treeMask.testFlag(TreeModel::TypeMask::Recipe) && thing) {

      // we are showing the child (context menu -> show snapshots ) OR
      // we are meant to display this thing.
      return model->showChild(child) || thing->display();
   }

   if (thing) {
      return thing->display();
   }

   return true;

}
