/*
 * YeastDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include "YeastDialog.h"
#include "observable.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "yeast.h"
#include "YeastEditor.h"

YeastDialog::YeastDialog(MainWindow* parent)
        : QDialog(parent)
{
   setupUi(this);
   mainWindow = parent;
   yeastEditor = new YeastEditor(this);
   dbObs = 0;
   numYeasts = 0;

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addYeast() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newYeast() ) );
}

void YeastDialog::notify(Observable *notifier)
{
   if( notifier != dbObs )
      return;

   if( numYeasts != dbObs->getNumYeasts() )
   {
      yeastTableWidget->getModel()->removeAll();
      populateTable();
   }
}

void YeastDialog::startObservingDB()
{
   dbObs = Database::getDatabase();
   populateTable();
}

void YeastDialog::populateTable()
{
   unsigned int i;

   if( ! Database::isInitialized() )
      return;

   numYeasts = dbObs->getNumYeasts();
   for( i = 0; i < numYeasts; ++i )
      yeastTableWidget->getModel()->addYeast(dbObs->getYeast(i));
}

void YeastDialog::addYeast()
{
   QModelIndexList selected = yeastTableWidget->selectedIndexes();
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

   Yeast *yeast = yeastTableWidget->getModel()->getYeast(row);
   mainWindow->addYeastToRecipe(new Yeast(*yeast) ); // Need to add a copy so we don't change the database.
}

void YeastDialog::editSelected()
{
   QModelIndexList selected = yeastTableWidget->selectedIndexes();
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

   Yeast *yeast = yeastTableWidget->getModel()->getYeast(row);
   yeastEditor->setYeast(yeast);
   yeastEditor->show();
}

void YeastDialog::newYeast()
{
   Yeast* y = new Yeast();

   Database::getDatabase()->addYeast(y);
   yeastEditor->setYeast(y);
   yeastEditor->show();
}
