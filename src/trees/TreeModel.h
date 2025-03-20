/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeModel.h is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
#ifndef TREES_TREEMODEL_H
#define TREES_TREEMODEL_H
#pragma once

#include <memory>
#include <optional>

#include <QAbstractItemModel>
#include <QModelIndex>

#include "trees/TreeNodeBase.h"
#include "utils/NoCopy.h"

// Forward declarations
class TreeView;

class TreeModelRowInsertGuard;

/*!
 * \class TreeModel
 *
 * \brief Model for a tree of Recipes, Equipments, Fermentables, Hops, Miscs and Yeasts
 *
 *        Provides the necessary model so we can build the trees. It extends the \c QAbstractItemModel, so it has to
 *        implement some of the virtual member functions required.
 *
 *        Although this may initially look like too many layers of abstraction, we do things this way to have Qt do as
 *        much of the heavy lifting as possible on the UI, eg via \c QTreeView.  The cost of this is that we need to
 *        present a data model that fits in with Qt's existing abstractions.  There is a "standard" approach to doing
 *        this for trees, described at https://doc.qt.io/qt-6/qtwidgets-itemviews-simpletreemodel-example.html.  We take
 *        things a bit further and try to set a number of the underlying constraints at compile time (eg that it is not
 *        possible to add \c Hop items to a tree of \c Misc items).
 *
 *        A \c TreeModel object "owns" the logical structure of the tree, and provides the standard Qt interface for
 *        interacting with a data model.  The items in the model are subclasses of \c TreeNode and \c TreeNodeBase (as
 *        explained in more detail in the comments in \c trees/TreeNode.h).  Although the parent-child links between
 *        these items effectively define the structure of the tree, there is a more generic location mechanism whereby
 *        any item in the model can be located by a \c QModelIndex object (which only remains valid until the model is
 *        altered - like an array index etc).  See more on this below.
 *
 *        Because \c TreeModel inherits from \c QAbstractItemModel, and supports \c QModelIndex locators, we can use it
 *        with \c QTreeView -- or rather our custom subclass thereof, \c TreeView -- which means we get the "standard"
 *        UI for trees pretty much for free.  Of course, we want a strongly-typed sub-class of \c TreeModel for each
 *        type of tree, using \c TreeModelBase to help:
 *
 *         QAbstractItemModel
 *              |
 *           TreeModel    TreeModelBase<Recipe>
 *                  \       /
 *                RecipeTreeModel
 *
 *        Additionally, \c TreeView can use a subclass of \c QSortFilterProxyModel to sort and filter what is shown in
 *        the tree.
 *
 *        Thus, in general, \c TreeView owns and uses \c TreeModel which owns and uses \c TreeNode.
 *
 *        The way to use \c QModelIndex isn't as brilliantly explained in the Qt doco as you might hope.  Because
 *        \c QModelIndex can be used with all sorts of different models, it has rather generic fields.  In order to work
 *        properly with \c QTreeView etc, we need to use them as follows:
 *           - \c parent: The coordinates on each \c QModelIndex object are relative to its "parent" \c QModelIndex
 *             object (ie the coordinates of the parent node in the tree) so there are no "absolute" coordinates in the
 *             tree
 *           - \c row means "child number" of our parent node (starting from 0), and is 0 for the root node
 *           - \c column isn't really a location in the tree, but relates to showing multiple columns of data for each
 *             (primary) item in the tree.  For each primary element in the tree (eg \c Fermentable in
 *             \c FermentableTreeModel) we show a number of columns of data, usually with \b Name as the first column.
 *             Eg for \c Fermentable, the other columns are \b Type and \b Color.
 *           - \c internalPointer is an untyped (ie \c void) raw pointer to the underlying tree node.  We always cast it
 *             back to \c TreeNode, and then call \c classifier() to find out the actual node type.  HOWEVER, see notes
 *             below for constraints on when we can use this field.
 *        Moreover, we can't just create our own \c QModelIndex objects directly.  Instead, we have to call
 *        \c QAbstractItemModel::createIndex.
 *
 *        But wait, it gets more complicated.  In order to allow the user to sort the tree by different columns, we use
 *        a subclass of \c QSortFilterProxyModel.  Just as we do for list models (eg \c HopListModel) and table models
 *        (eg \c HopTableModel), we just override \c QSortFilterProxyModel::lessThan to handle comparison for each
 *        column we want to display, and Qt can handle the rest, including, for hierarchical models, applying sorting
 *        recursively to all child items.  The \c QSortFilterProxyModel subclass sits between the view (eg
 *        \c HopTreeView) and the model (eg \c HopTreeModel) and translates each \c QModelIndex between those used in
 *        the source model and those used in the proxy model.  Mostly, as long as we go via
 *        \c QSortFilterProxyModel::mapToSource, \c QSortFilterProxyModel::mapFromSource, etc, everything should "just
 *        work".  HOWEVER, we need to keep in mind that we cannot use the \c QModelIndex::internalPointer field in proxy
 *        model indexes (because \c QSortFilterProxyModel puts its own data in that field).  So we must be certain we
 *        have a source model index before we try to read or write this field.
 *
 *        Initially, it is tempting to reuse the same subclasses of \c QSortFilterProxyModel that we created for list
 *        models and tree models -- eg to reuse \c HopSortFilterProxyModel for \c HopTree.  However, this would tie us
 *        in to using the same columns in, eg, \c HopTree as we use in \c HopTableModel and \c HopListModel, which is
 *        too much of a constraint.  So, we have \c HopTreeSortFilterProxyModel etc:
 *
 *           QSortFilterProxyModel        TreeSortFilterProxyModelBase
 *                             \             /
 *                       HopTreeSortFilterProxyModel
 */
class TreeModel : public QAbstractItemModel {
   Q_OBJECT

   // See comment below in TreeModelRowInsertGuard declaration for why it needs to be a friend
   friend class TreeModelRowInsertGuard;

public:

   TreeModel(TreeView * parent = nullptr);
   virtual ~TreeModel();

   virtual TreeNode * treeNode(QModelIndex const & index) const = 0;

   /**
    * \brief Returns the name of the class of the main object stored in the tree, eg "Recipe" for RecipeTreeModel.
    */
   virtual QString treeForClassName() const = 0;
   virtual QString treeForLocalisedClassName() const = 0;

   //! \brief Reimplemented from QAbstractItemModel
   virtual Qt::ItemFlags flags(QModelIndex const & index) const override;

   /**
    * \brief Reimplemented from \c QAbstractItemModel to ensure we don't call the default implementation by mistake.
    *
    *        Use \c TreeModelBase::insertChild instead of this member function to add nodes to the tree.
    */
   [[deprecated]] virtual bool insertRows(int row, int count, QModelIndex const & parent = QModelIndex{}) override;

   /**
    * \brief Reimplemented from \c QAbstractItemModel to ensure we don't call the default implementation by mistake.
    *
    *        Use \c TreeModelBase::removeChildren instead of this member function to remove nodes from the tree.
    */
   [[deprecated]] virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex()) override;

   //! \brief Reimplemented from QAbstractItemModel: what our supported drop actions are. Don't know if I need the drag option or not?
   virtual Qt::DropActions supportedDropActions() const override;
   //! \brief Reimplemented from QAbstractItemModel
   virtual QStringList mimeTypes() const override = 0;

   //! \return the classifier of the item at \c index, or \c nullopt if \c index is invalid
   std::optional<TreeNodeClassifier> classifier(QModelIndex const & index) const;

signals:
   void expandFolder(/*TreeModel::TypeMasks kindofThing, */QModelIndex fIdx);

private:
   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(TreeModel)
};


/**
 * \brief Any time we change the tree structure, we need to call beginInsertRows() and endInsertRows() to notify
 *        other components that the model has changed.  This RAII class handles that for us.
 *
 *        NOTE that because \c QAbstractItemModel::beginInsertRows and \c QAbstractItemModel::endInsertRows are
 *        protected, this class needs to be a friend of \c TreeModel.
 */
class TreeModelRowInsertGuard {
public:
   TreeModelRowInsertGuard(TreeModel & model, QModelIndex const & parent, int const first, int const last);
   ~TreeModelRowInsertGuard();
private:
   TreeModel & m_model;
};

#endif
