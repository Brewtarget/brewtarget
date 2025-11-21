/*======================================================================================================================
 * trees/TreeSortFilterProxyModelBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef TREES_TREESORTFILTERPROXYMODELBASE_H
#define TREES_TREESORTFILTERPROXYMODELBASE_H
#pragma once

#include <QModelIndex>
#include <QSortFilterProxyModel>

#include "trees/TreeModel.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/*!
 * \class TreeSortFilterProxyModelBase
 *
 * \brief CRTP base for \c HopTreeSortFilterProxyModel, \c RecipeTreeSortFilterProxyModel, etc.  See comment on
 *        \c TreeModel class for more info.
 *
 * \param SNE should be \c void if there is no secondary item
 */
template<class Derived> class TreeSortFilterProxyModelPhantom;
template<class Derived, class NeTreeModel, class NE, typename SNE = void>
class TreeSortFilterProxyModelBase : public CuriouslyRecurringTemplateBase<TreeSortFilterProxyModelPhantom, Derived> {
   friend Derived;

protected:

   /**
    * \brief Implements override of \c QSortFilterProxyModel::lessThan.
    *
    *        Returns \c true if the value of the item referred to by the given index \c left is less than the value of
    *        the item referred to by the given index \c right, otherwise returns \c false.
    *
    *        Note: The indices passed in correspond to the source model
    */
   bool doLessThan(QModelIndex const & left, QModelIndex const & right) const {
      NeTreeModel * treeModel = qobject_cast<NeTreeModel *>(this->derived().sourceModel());
      if (!treeModel) {
         // This shouldn't happen
         qWarning() << Q_FUNC_INFO << "No model";
         return false;
      }

      TreeNode const * lhs = treeModel->doTreeNode(left );
      TreeNode const * rhs = treeModel->doTreeNode(right);

      //
      // NOTE that we have to be careful about column indexes here.  A tree can have up to three different types of node
      // (Folder, PrimaryItem and SecondaryItem), each of which potentially has a different number of columns.  So we
      // may be sorting by a column for which some of the node types do not have data.
      //
      int const rawColumnNum = left.column();

      // If the two node types are the same then we can leave the comparison to the overloaded functions in TreeNode.h /
      // TreeNode.cpp
      if (lhs->classifier() == TreeNodeClassifier::Folder &&
          rhs->classifier() == TreeNodeClassifier::Folder) {
         if (rawColumnNum >= ColumnOwnerTraits<TreeFolderNode<NE>>::numColumns()) {
            return false;
         }
         auto const & lhsFolder = static_cast<TreeFolderNode<NE> const &>(*lhs);
         auto const & rhsFolder = static_cast<TreeFolderNode<NE> const &>(*rhs);
         auto const      column = static_cast<TreeFolderNode<NE>::ColumnIndex>(rawColumnNum);
         return lhsFolder.columnIsLessThan(rhsFolder, column);
      }

      if (lhs->classifier() == TreeNodeClassifier::PrimaryItem &&
          rhs->classifier() == TreeNodeClassifier::PrimaryItem) {
         if (rawColumnNum >= ColumnOwnerTraits<TreeItemNode<NE>>::numColumns()) {
            return false;
         }
         auto const & lhsNode = static_cast<TreeItemNode<NE> const &>(*lhs);
         auto const & rhsNode = static_cast<TreeItemNode<NE> const &>(*rhs);
         auto const    column = static_cast<TreeItemNode<NE>::ColumnIndex>(rawColumnNum);
         return lhsNode.columnIsLessThan(rhsNode, column);
      }

      if constexpr (!IsVoid<SNE>) {
         if (lhs->classifier() == TreeNodeClassifier::SecondaryItem &&
             rhs->classifier() == TreeNodeClassifier::SecondaryItem) {

            if (rawColumnNum >= ColumnOwnerTraits<TreeItemNode<SNE>>::numColumns()) {
               return false;
            }
            auto const & lhsNode = static_cast<TreeItemNode<SNE> const &>(*lhs);
            auto const & rhsNode = static_cast<TreeItemNode<SNE> const &>(*rhs);
            //
            // We could write:
            //    auto const    column = static_cast<TreeItemNode<SNE>::ColumnIndex>(rawColumnNum);
            //    return lhsNode.columnIsLessThan(rhsNode, column);
            // However, we typically _don't_ want the secondary items to be sorted by column as (a) their columns mostly
            // won't match those of the primary items and (b) it often won't make sense to sort them.  Eg, if you have
            // a load of StockPurchases sorted by name or by date, you still want their StockUses in date order.
            //
            // The code below will still have secondaries "backwards" when a primary column is sorted in reverse order,
            // but I think we can live with that.
            //
            SNE & lhsItem = *lhsNode.underlyingItem();
            SNE & rhsItem = *rhsNode.underlyingItem();
            return lhsItem < rhsItem;
         }
      }

      // If the node types are different, then we want folders before items, and primary items before secondary items.
      // Beyond that, the only meaningful comparison we can do is on the name.
      if (lhs->classifier() == TreeNodeClassifier::Folder &&
          rhs->classifier() != TreeNodeClassifier::Folder) {
         return true;
      }
      if (rhs->classifier() == TreeNodeClassifier::Folder) {
         return false;
      }
      if (lhs->classifier() != TreeNodeClassifier::Folder) {
         if (lhs->classifier() == TreeNodeClassifier::PrimaryItem &&
             rhs->classifier() != TreeNodeClassifier::PrimaryItem) {
            return true;
         }
         if (rhs->classifier() == TreeNodeClassifier::PrimaryItem) {
            return false;
         }
      }

      return lhs->name() < rhs->name();
   }

   /**
    * \brief Implements partial override of \c QSortFilterProxyModel::filterAcceptsRow.
    *
    *        Returns \c true if the item in the row indicated by the given \c row and \c parent should be included in
    *        the model; otherwise returns \c false.
    *
    *        NOTE: The row and index passed in correspond to the \b source model
    */
   bool doFilterAcceptsRow(int row, QModelIndex const & parentIndex) const {
      NeTreeModel * treeModel = qobject_cast<NeTreeModel *>(this->derived().sourceModel());
      if (!treeModel) {
         // This shouldn't happen
         qWarning() << Q_FUNC_INFO << "No model";
         return false;
      }

      //
      // Get the index of the child node to be filtered
      //
      QModelIndex childIndex = treeModel->index(row, 0, parentIndex);
      if (!childIndex.isValid()) {
         // This shouldn't happen
         qWarning() << Q_FUNC_INFO << "Invalid childIndex";
         return false;
      }

      TreeNode * childNode = treeModel->doTreeNode(childIndex);

      // Always show folders
      if (childNode->classifier() == TreeNodeClassifier::Folder) {
         return true;
      }

      // In the Recipe tree, we can override the normal rule.  This is used to show Recipe snapshots
      // (context menu -> show snapshots )
      if constexpr (std::same_as<NE, Recipe>) {
         if (childNode->showMe()) {
            return true;
         }
      }

      // Hide deleted items
      NamedEntity * underlyingItem = childNode->rawUnderlyingItem();
      if (underlyingItem && underlyingItem->deleted()) {
         return false;
      }

      // Base class does remaining filtering -- eg for Derived::setFilterFixedString (see TreeViewBase::filter).
      return this->derived().QSortFilterProxyModel::filterAcceptsRow(row, parentIndex);
   }
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define TREE_SORT_FILTER_PROXY_MODEL_COMMON_DECL(NeName, ...) \
   /* This allows TreeSortFilterProxyModelBase to call protected and private members of Derived */  \
   friend class TreeSortFilterProxyModelBase<NeName##TreeSortFilterProxyModel,                      \
                                             NeName##TreeModel,                                     \
                                             NeName __VA_OPT__(, __VA_ARGS__)>;                     \
                                                                                                    \
   public:                                                                                          \
      NeName##TreeSortFilterProxyModel(QWidget * parent);                                           \
      virtual ~NeName##TreeSortFilterProxyModel();                                                  \
      virtual bool lessThan(QModelIndex const & left, QModelIndex const & right) const override;    \
      virtual bool filterAcceptsRow(int row, QModelIndex const & parent) const override;            \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions
 *
 *        NB: Mostly I have tried to make these macro-included function bodies trivial.  Macros are a bit clunky, so we
 *            only really want to use them for the things that are hard to do other ways.
 */
#define TREE_SORT_FILTER_PROXY_MODEL_COMMON_CODE(NeName, ...) \
   NeName##TreeSortFilterProxyModel::NeName##TreeSortFilterProxyModel(QWidget * parent) :      \
      QSortFilterProxyModel{parent} {                                                          \
      return;                                                                                  \
   }                                                                                           \
   NeName##TreeSortFilterProxyModel::~NeName##TreeSortFilterProxyModel() = default;            \
                                                                                               \
   bool NeName##TreeSortFilterProxyModel::lessThan(QModelIndex const & left,                   \
                                                   QModelIndex const & right) const {          \
      return this->doLessThan(left, right);                                                    \
   }                                                                                           \
   bool NeName##TreeSortFilterProxyModel::filterAcceptsRow(int row,                            \
                                                           QModelIndex const & parent) const { \
      return this->doFilterAcceptsRow(row, parent);                                            \
   }                                                                                           \

#endif
