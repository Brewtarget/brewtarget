/*======================================================================================================================
 * utils/VeriTable.h is part of Brewtarget, and is copyright the following authors 2025-2026:
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
#ifndef UTILS_DISPLAYTABLEMANAGER_H
#define UTILS_DISPLAYTABLEMANAGER_H
#pragma once

#include <memory>

#include <QAbstractButton>
#include <QItemSelectionModel>
#include <QMenu>
#include <QModelIndex>
#include <QTableView>

#include "MainWindow.h"
#include "widgets/TableViewKeys.h"

/**
 * \brief This class gathers together the common bits of data and functionality to handle a table that can be sorted by
 *        column.
 *
 *        Yes, it's a corny name, but it's relatively short and is a bit distinctive from TableView / TableModel /
 *        TableWidget etc.
 *
 *        Rather than extend \c QTableView or \c QTableWidget, we simply use composition to group together related data
 *        structures and functionality.  We are not actually extending the functionality of \c QTableView itself, so
 *        avoiding trying to subclass it keeps things simple: this can be a templated class without jumping through any
 *        hoops.
 */
template<class NE> struct VeriTable {

   QTableView * m_tableView = nullptr;
   typename NE::IngredientClass::EditorClass * m_editor = nullptr;
   std::unique_ptr<typename NE::TableModelClass> m_tableModel = nullptr;
   std::unique_ptr<typename NE::SortFilterProxyModelClass> m_sortFilterProxyModel = nullptr;
   std::unique_ptr<typename NE::ItemDelegateClass> m_itemDelegate = nullptr;
   //! Optional buttons that act on the selected row(s).  We enable/disable them according to what is selected.
   QAbstractButton * m_removeButton = nullptr;
   QAbstractButton * m_editButton   = nullptr;
   //! Whether the table can be edited at all (eg \c false if the recipe is locked)
   bool m_editable = true;

   /**
    * \brief Can't do setup in constructor as owner (eg MainWindow) won't have the info to give us in its own
    *        constructor.  Need to wait until owner has called its own setupUi() function before it has a QTableView to
    *        give us.
    */
   void setup(QTableView * tableView,
              typename NE::IngredientClass::EditorClass * editor,
              QAbstractButton * removeButton = nullptr,
              QAbstractButton * editButton   = nullptr) {
      this->m_tableView = tableView;
      this->m_editor = editor;
      this->m_removeButton = removeButton;
      this->m_editButton   = editButton;
      this->m_tableModel = std::make_unique<typename NE::TableModelClass>(this->m_tableView);
      this->m_sortFilterProxyModel = std::make_unique<typename NE::SortFilterProxyModelClass>(this->m_tableView, false);
      this->m_sortFilterProxyModel->setSourceModel(this->m_tableModel.get());
      this->m_tableView->setItemDelegate(new typename NE::ItemDelegateClass(this->m_tableView, *this->m_tableModel));
      this->m_tableView->setModel(this->m_sortFilterProxyModel.get());

      // Double clicking the first (ie name) column pops up an edit dialog for the selected item
      MainWindow & mainWindow{MainWindow::instance()};
      mainWindow.connect(this->m_tableView,
                         &QTableView::doubleClicked,
                         &mainWindow,
                         [&](const QModelIndex &idx) {
                            if (idx.column() == 0) {
                               this->editSelected();
                            }
                            return;
                         });

      //
      // Keep the Remove and Edit buttons in step with what is selected.  As well as the selection changing, rows can
      // come and go (eg when a different recipe is shown) without a selectionChanged signal, so we listen for those
      // too.
      //
      //
      // Right-click offers the same things as the buttons, and the Delete key removes the selected row(s)
      //
      this->m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
      mainWindow.connect(this->m_tableView,
                         &QWidget::customContextMenuRequested,
                         &mainWindow,
                         [this](QPoint const & pos) { this->showContextMenu(pos); return; });
      TableViewKeys::handleDeleteKey(*this->m_tableView, [this]() {
         if (this->m_editable) {
            this->removeSelected();
         }
         return;
      });

      auto updateButtons = [this]() { this->updateButtonStates(); return; };
      mainWindow.connect(this->m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, &mainWindow, updateButtons);
      mainWindow.connect(this->m_sortFilterProxyModel.get(), &QAbstractItemModel::modelReset  , &mainWindow, updateButtons);
      mainWindow.connect(this->m_sortFilterProxyModel.get(), &QAbstractItemModel::rowsRemoved , &mainWindow, updateButtons);
      mainWindow.connect(this->m_sortFilterProxyModel.get(), &QAbstractItemModel::rowsInserted, &mainWindow, updateButtons);
      this->updateButtonStates();
      return;
   }

   /**
    * \brief Enable or disable the whole table (eg because the recipe is locked).  Remove and Edit buttons additionally
    *        depend on what is selected.
    */
   void setEditable(bool const editable) {
      this->m_editable = editable;
      this->m_tableView->setEnabled(editable);
      this->updateButtonStates();
      return;
   }

   /**
    * \brief Context menu for the table rows (as opposed to the column headers, which have their own -- see
    *        \c BtTableModel::contextMenu).
    */
   void showContextMenu(QPoint const & pos) {
      QMenu menu{this->m_tableView};
      QAction * editAction = menu.addAction(
         MainWindow::tr("Edit %1…").arg(NE::IngredientClass::localisedName()),
         [this]() { this->editSelected(); return; }
      );
      editAction->setEnabled(this->m_editable && this->selected() != nullptr);
      QAction * removeAction = menu.addAction(
         MainWindow::tr("Remove from Recipe"),
         [this]() { this->removeSelected(); return; }
      );
      removeAction->setEnabled(this->m_editable && this->m_tableView->selectionModel()->hasSelection());
      removeAction->setShortcut(QKeySequence::Delete);
      // The shortcut is handled by TableViewKeys::handleDeleteKey; here it's just to tell the user it exists
      removeAction->setShortcutVisibleInContextMenu(true);
      menu.exec(this->m_tableView->viewport()->mapToGlobal(pos));
      return;
   }

   /**
    * \brief Remove needs at least one row selected; Edit needs exactly one.
    */
   void updateButtonStates() {
      if (this->m_removeButton) {
         this->m_removeButton->setEnabled(this->m_editable && this->m_tableView->selectionModel()->hasSelection());
      }
      if (this->m_editButton) {
         this->m_editButton->setEnabled(this->m_editable && this->selected() != nullptr);
      }
      return;
   }

   void setSortColumn(typename NE::TableModelClass::ColumnIndex const columnNumber) {
      this->m_tableView->horizontalHeader()->setSortIndicator(static_cast<int>(columnNumber), Qt::DescendingOrder);
      this->m_tableView->setSortingEnabled(true);
      this->m_sortFilterProxyModel->setDynamicSortFilter(true);
      return;
   }

   /**
    * \brief Common code for getting the currently highlighted entry in the table
    */
   NE * selected() {
      QModelIndexList selected = this->m_tableView->selectionModel()->selectedIndexes();

      auto const size = selected.size();
      if (size == 0) {
         return nullptr;
      }

      // Make sure only one row is selected.
      QModelIndex viewIndex = selected[0];
      int const row = viewIndex.row();
      for (auto ii = 1; ii < size; ++ii ) {
         if (selected[ii].row() != row) {
            return nullptr;
         }
      }

      QModelIndex const modelIndex = this->m_sortFilterProxyModel->mapToSource(viewIndex);
      return this->m_tableModel->getRow(modelIndex.row()).get();
   }

   /**
    * \brief
    */
   void editSelected() {
      NE * selectedAddition = this->selected();
      if (!selectedAddition) {
         return;
      }

      auto ingredient = selectedAddition->ingredient();
      if (!ingredient) {
         return;
      }

      this->m_editor->setEditItem(ingredient);
      this->m_editor->show();
      return;
   }

   /**
    * \brief
    */
   void removeSelected() {
      this->m_tableModel->removeSelectedIngredients(*this->m_tableView, *this->m_sortFilterProxyModel);
      return;
   }

   /**
    * \brief
    */
   void remove(std::shared_ptr<NE> itemToRemove) {
      this->m_tableModel->remove(itemToRemove);
      return;
   }

};

#endif