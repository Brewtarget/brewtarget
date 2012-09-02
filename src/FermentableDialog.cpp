/*
 * FermentableDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include <QString>
#include <QInputDialog>
#include <QList>
#include "FermentableEditor.h"
#include "FermentableDialog.h"
#include "FermentableTableModel.h"
#include "FermentableSortFilterProxyModel.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "fermentable.h"

FermentableDialog::FermentableDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent),
          fermEdit(new FermentableEditor(this)), numFerms(0)
{
   setupUi(this);

   fermTableModel = new FermentableTableModel(fermentableTableWidget);
   fermTableProxy = new FermentableSortFilterProxyModel(fermentableTableWidget);
   fermTableProxy->setSourceModel(fermTableModel);
   fermentableTableWidget->setModel(fermTableProxy);
   fermentableTableWidget->setSortingEnabled(true);
   fermentableTableWidget->sortByColumn( FERMNAMECOL, Qt::AscendingOrder );
   
   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addFermentable() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeFermentable() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newFermentable() ) );
   connect( fermentableTableWidget, SIGNAL( doubleClicked(const QModelIndex&) ), this, SLOT(addFermentable(const QModelIndex&)) );
   
   fermTableModel->observeDatabase(true);
}

void FermentableDialog::removeFermentable()
{
   QModelIndexList selected = fermentableTableWidget->selectionModel()->selectedIndexes();
   QModelIndex translated;
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

   translated = fermTableProxy->mapToSource(selected[0]);
   Fermentable* ferm = fermTableModel->getFermentable(translated.row());
   Database::instance().removeFermentable(ferm);
}

void FermentableDialog::editSelected()
{
   QModelIndexList selected = fermentableTableWidget->selectionModel()->selectedIndexes();
   QModelIndex translated;
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

   translated = fermTableProxy->mapToSource(selected[0]);
   Fermentable* ferm = fermTableModel->getFermentable(translated.row());
   fermEdit->setFermentable(ferm);
   fermEdit->show();
}

void FermentableDialog::addFermentable(const QModelIndex& index)
{
   QModelIndex translated;
   
   // If there is no provided index, get the selected index.
   if( !index.isValid() )
   {
      QModelIndexList selected = fermentableTableWidget->selectionModel()->selectedIndexes();
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
      
      translated = fermTableProxy->mapToSource(selected[0]);
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other fermentable fields.
      if( index.column() == FERMNAMECOL )
         translated = fermTableProxy->mapToSource(index);
      else
         return;
   }
   
   Fermentable *ferm = fermTableModel->getFermentable(translated.row());
   
   Database::instance().addToRecipe( mainWindow->currentRecipe(), ferm );
}

void FermentableDialog::newFermentable()
{
   QString name = QInputDialog::getText(this, tr("Fermentable name"),
                                          tr("Fermentable name:"));
   if( name.isEmpty() )
      return;
   
   Fermentable* ferm = Database::instance().newFermentable();
   ferm->setName(name);
   fermEdit->setFermentable(ferm);
   fermEdit->show();
}
