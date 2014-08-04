/*
 * MiscDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QWidget>
#include <QDialog>
#include <QInputDialog>
#include <QString>
#include <QList>
#include "MiscDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "misc.h"
#include "MiscEditor.h"
#include "MiscTableModel.h"
#include "MiscSortFilterProxyModel.h"

MiscDialog::MiscDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), numMiscs(0), miscEdit(new MiscEditor(this))
{
   setupUi(this);

   miscTableModel = new MiscTableModel(miscTableWidget, false);
   miscTableModel->setInventoryEditable(true);
   miscTableProxy = new MiscSortFilterProxyModel(miscTableWidget);
   miscTableProxy->setSourceModel(miscTableModel);
   miscTableWidget->setModel(miscTableProxy);
   miscTableWidget->setSortingEnabled(true);
   miscTableWidget->sortByColumn( MISCNAMECOL, Qt::AscendingOrder );
   miscTableProxy->setDynamicSortFilter(true);
   
   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addMisc() ) );
   connect( pushButton_new, SIGNAL(clicked()), this, SLOT( newMisc() ) );
   connect( pushButton_edit, SIGNAL(clicked()), this, SLOT(editSelected()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeMisc()) );
   connect( miscTableWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT( addMisc(const QModelIndex&) ) );
   
   miscTableModel->observeDatabase(true);
}

void MiscDialog::removeMisc()
{
   QModelIndexList selected = miscTableWidget->selectionModel()->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Misc* m = miscTableModel->getMisc(miscTableProxy->mapToSource(selected[0]).row());
   Database::instance().remove(m);
}

void MiscDialog::addMisc(const QModelIndex& index)
{
   QModelIndex translated;
   
   if( !index.isValid() )
   {
      QModelIndexList selected = miscTableWidget->selectionModel()->selectedIndexes();
      int row, size, i;

      size = selected.size();
      if( size == 0 )
         return;

      // Make sure only one row is selected.
      row = selected[0].row();
      for( i = 1; i < size; ++i )
      {
         if( selected[i].row() != row )
            return;
      }
     
      // Always need to translate indices through the proxy 
      translated = miscTableProxy->mapToSource(selected[0]);
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == MISCNAMECOL )
         translated = miscTableProxy->mapToSource(index);
      else
         return;
   }
   
   Misc *misc = miscTableModel->getMisc(translated.row());
   
   Database::instance().addToRecipe( mainWindow->currentRecipe(), misc );
}

void MiscDialog::editSelected()
{
   QModelIndexList selected = miscTableWidget->selectionModel()->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Misc* m = miscTableModel->getMisc(miscTableProxy->mapToSource(selected[0]).row());
   miscEdit->setMisc(m);
   miscEdit->show();
}

void MiscDialog::newMisc()
{
   QString name = QInputDialog::getText(this, tr("Misc name"),
                                              tr("Misc name:"));
   if(name.isEmpty())
      return;

   Misc* m = Database::instance().newMisc();
   m->setName(name);
   miscEdit->setMisc(m);
   miscEdit->show();
}
