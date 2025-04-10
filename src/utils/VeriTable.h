/*======================================================================================================================
 * utils/VeriTable.h is part of Brewtarget, and is copyright the following authors 2025:
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

#include <QModelIndex>
#include <QTableView>

#include "MainWindow.h"

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

   /**
    * \brief Can't do setup in constructor as owner (eg MainWindow) won't have the info to give us in its own
    *        constructor.  Need to wait until owner has called its own setupUi() function before it has a QTableView to
    *        give us.
    */
   void setup(QTableView * tableView,
              typename NE::IngredientClass::EditorClass * editor) {
      this->m_tableView = tableView;
      this->m_editor = editor;
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

      int size = selected.size();
      if (size == 0) {
         return nullptr;
      }

      // Make sure only one row is selected.
      QModelIndex viewIndex = selected[0];
      int row = viewIndex.row();
      for (int ii = 1; ii < size; ++ii ) {
         if (selected[ii].row() != row) {
            return nullptr;
         }
      }

      QModelIndex modelIndex = this->m_sortFilterProxyModel->mapToSource(viewIndex);
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
