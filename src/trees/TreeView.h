/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * trees/TreeView.h is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#ifndef TREES_TREEVIEW_H
#define TREES_TREEVIEW_H
#pragma once

#include <QTreeView>
#include <QWidget>
#include <QPoint>
#include <QMouseEvent>

#include "trees/TreeNode.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/NoCopy.h"

// Forward declarations.
class TreeModel;

/*!
 * \class TreeView
 *
 * \brief View class for \c TreeModel.
 */
class TreeView : public QTreeView {
   Q_OBJECT
public:
   //! \brief The standard constructor
   TreeView(QWidget * parent = nullptr);
   /**
    * \brief returns the \c TreeModel associated with this tree.  Note that this bypasses the \c QSortFilterProxyModel
    *        that maps between view and model (and which would be returned by a call to \c QTreeView::model).
    */
   virtual TreeModel & treeModel() = 0;

   virtual TreeNode * treeNode(QModelIndex const & index) const = 0;

   //! Called from \c MainWindow::treeActivated
   virtual void activated(QModelIndex const & index) = 0;

   //! \brief returns the context menu associated with the \c selected item
   virtual QMenu * getContextMenu(QModelIndex const & selectedViewIndex) = 0;

   //! \brief Copy the specified items
   virtual void copy(QModelIndexList const & selectedViewIndexes) = 0;

   /**
    * \brief Delete the specified items
    * \return Index of what, if anything,  should now be selected (assuming the deleted items were what were previously
    *         selected).
    */
   virtual std::optional<QModelIndex> deleteItems(QModelIndexList const & selectedViewIndexes) = 0;

   virtual void setSelected(QModelIndex const & index) = 0;

   //! \brief Copy the selected items in this tree
   virtual void copySelected() = 0;
   //! \brief Delete the selected items in this tree
   virtual void deleteSelected() = 0;
   //! \brief Export the selected items in this tree to BeerXML or BeerJSON
   void exportSelected() const;
   //! \brief Import items from BeerXML or BeerJSON
   void importFiles();

   virtual void renameSelected() = 0;

   //! \brief adds a folder to the tree
   virtual void addFolder(QString const & folder) = 0;

   virtual QString folderName(QModelIndex const & viewIndex) const = 0;

public:
   //! \return the classifier of the item at \c index, or \c nullopt if \c index is invalid
   std::optional<TreeNodeClassifier> classifier(QModelIndex const & index) const;

   // Another try at drag and drop
   //! \brief Overrides \c QTreeView::mousePressEvent.  Starts a drag and drop event
   virtual void mousePressEvent(QMouseEvent * event) override;
   //! \brief Overrides \c QTreeView::mouseMoveEvent.  Distinguishes between a move event and a double click
   virtual void mouseMoveEvent(QMouseEvent * event) override;
   //! \brief Overrides \c QTreeView::mouseDoubleClickEvent.  Recognizes a double click event
   virtual void mouseDoubleClickEvent(QMouseEvent * event) override;

   //! \brief Overrides \c QTreeView::keyPressEvent.  Catches a key stroke in a tree
   virtual void keyPressEvent(QKeyEvent * event) override;

protected:
   QPoint dragStart;

   bool m_doubleClick;

private:
   // Insert all the usual boilerplate to prevent copy/assignment/move
   NO_COPY_DECLARATIONS(TreeView)
};

#endif
