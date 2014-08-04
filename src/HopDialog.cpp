/*
 * HopDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Luke Vincent <luke.r.vincent@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QWidget>
#include <QDialog>
#include <QInputDialog>
#include <QString>
#include <QList>
#include "HopDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "hop.h"
#include "HopEditor.h"
#include "HopTableModel.h"
#include "HopTableModel.h"
#include "HopSortFilterProxyModel.h"

HopDialog::HopDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), hopEditor(new HopEditor(this)), numHops(0)
{
   setupUi(this);

   hopTableModel = new HopTableModel(hopTableWidget, false);
   hopTableModel->setInventoryEditable(true);
   hopTableProxy = new HopSortFilterProxyModel(hopTableWidget);
   hopTableProxy->setSourceModel(hopTableModel);
   hopTableWidget->setModel(hopTableProxy);
   hopTableWidget->setSortingEnabled(true);
   hopTableWidget->sortByColumn( HOPNAMECOL, Qt::AscendingOrder );
   hopTableProxy->setDynamicSortFilter(true);
   
   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addHop() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newHop() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeHop() ));
   connect( hopTableWidget, SIGNAL( doubleClicked(const QModelIndex&) ), this, SLOT( addHop(const QModelIndex&) ) );
   
   hopTableModel->observeDatabase(true);
}

void HopDialog::removeHop()
{
    QModelIndex modelIndex, viewIndex;
    QModelIndexList selected = hopTableWidget->selectionModel()->selectedIndexes();
    // Qlist requires this check before using member functions per doc
    if(!(selected.isEmpty())){
    // Make sure only one row is selected.
        if(selected.size() == 1){
            viewIndex = selected[0]; // [] or .at() prefered to .value()
        }
    }
    modelIndex = hopTableProxy->mapToSource(viewIndex);
    Hop *hop = hopTableModel->getHop(modelIndex.row());
    Database::instance().remove(hop);
}

void HopDialog::addHop(const QModelIndex& index)
{
   QModelIndex translated;
   if( !index.isValid() )
   {
      QModelIndexList selected = hopTableWidget->selectionModel()->selectedIndexes();
      int row, size, i;

      size = selected.size();
      if( size == 0 )
         return;

      // Make sure only one row is selected.
      row = selected.value(0).row();
      for( i = 1; i < size; ++i )
      {
         if( selected.value(i).row() != row )
            return;
      }

      translated = hopTableProxy->mapToSource(selected.value(0));
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == HOPNAMECOL )
         translated = hopTableProxy->mapToSource(index);
      else
         return;
   }
   
   Hop *hop = hopTableModel->getHop(translated.row());
   
   Database::instance().addToRecipe( mainWindow->currentRecipe(), hop );
}

void HopDialog::editSelected()
{
   QModelIndexList selected = hopTableWidget->selectionModel()->selectedIndexes();
   QModelIndex translated;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected.value(0).row();
   for( i = 1; i < size; ++i )
   {
      if( selected.value(i).row() != row )
         return;
   }

   translated = hopTableProxy->mapToSource(selected.value(0));
   Hop *hop = hopTableModel->getHop(translated.row());
   hopEditor->setHop(hop);
   hopEditor->show();
}

void HopDialog::newHop()
{
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                          tr("Hop name:"));
   if( name.isEmpty() )
      return;

   Hop* hop = Database::instance().newHop();
   hop->setName(name);
   hopEditor->setHop(hop);
   hopEditor->show();
}
