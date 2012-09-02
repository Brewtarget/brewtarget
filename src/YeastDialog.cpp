/*
 * YeastDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QInputDialog>
#include <QString>
#include <QList>
#include "YeastDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "yeast.h"
#include "YeastEditor.h"
#include "YeastTableModel.h"
#include "YeastSortFilterProxyModel.h"

YeastDialog::YeastDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), yeastEditor(new YeastEditor(this)), numYeasts(0)
{
   setupUi(this);

   yeastTableModel = new YeastTableModel(yeastTableWidget);
   yeastTableProxy = new YeastSortFilterProxyModel(yeastTableWidget);
   yeastTableProxy->setSourceModel(yeastTableModel);
   yeastTableWidget->setModel(yeastTableProxy);
   yeastTableWidget->setSortingEnabled(true);
   yeastTableWidget->sortByColumn( YEASTNAMECOL, Qt::AscendingOrder );
   
   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addYeast() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newYeast() ) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT( removeYeast() ) );
   connect( yeastTableWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT( addYeast(const QModelIndex&) ) );
   

   yeastTableModel->observeDatabase(true);

}

void YeastDialog::removeYeast()
{
   QModelIndexList selected = yeastTableWidget->selectionModel()->selectedIndexes();
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

   // We need to translate from the view's index to the model's index.  The
   // proxy model does the heavy lifting, as long as we do the call.
   translated = yeastTableProxy->mapToSource(selected[0]);
   Yeast *yeast = yeastTableModel->getYeast(translated.row());
   Database::instance().removeYeast(yeast);
}

void YeastDialog::changed(QMetaProperty prop, QVariant val)
{
   QString propName(prop.name());
   
   // Notifier should only be the database.
   if( sender() == &(Database::instance()) &&
       propName == "yeasts" )
   {
      yeastTableModel->removeAll();
      populateTable();
   }
}

void YeastDialog::populateTable()
{
   QList<Yeast*> yeasts;
   yeasts = Database::instance().yeasts();
   
   numYeasts = yeasts.size();
   yeastTableModel->addYeasts(yeasts);
}

void YeastDialog::addYeast(const QModelIndex& index)
{
   QModelIndex translated;
   
   if( !index.isValid() )
   {
      QModelIndexList selected = yeastTableWidget->selectionModel()->selectedIndexes();
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

      translated = yeastTableProxy->mapToSource(selected[0]);
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == YEASTNAMECOL )
         translated = yeastTableProxy->mapToSource(index);
      else
         return;
   }
   
   Yeast* yeast = yeastTableModel->getYeast(translated.row());
   
   // Adds a copy of yeast.
   Database::instance().addToRecipe( mainWindow->currentRecipe(), yeast );
}

void YeastDialog::editSelected()
{
   QModelIndexList selected = yeastTableWidget->selectionModel()->selectedIndexes();
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
   translated = yeastTableProxy->mapToSource(selected[0]);
   Yeast *yeast = yeastTableModel->getYeast(translated.row());
   yeastEditor->setYeast(yeast);
   yeastEditor->show();
}

void YeastDialog::newYeast()
{
   QString name = QInputDialog::getText(this, tr("Yeast name"),
                                              tr("Yeast name:"));
   if( name.isEmpty() )
      return;

   Yeast* y = Database::instance().newYeast();
   y->setName(name);
   yeastEditor->setYeast(y);
   yeastEditor->show();
   y->setDisplay(true);
}
