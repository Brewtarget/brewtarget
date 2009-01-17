/*
 * MiscDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "MiscDialog.h"
#include "observable.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "misc.h"

MiscDialog::MiscDialog(MainWindow* parent)
        : QDialog(parent)
{
   setupUi(this);
   mainWindow = parent;
   dbObs = 0;
   numMiscs = 0;

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addMisc() ) );
}

void MiscDialog::notify(Observable *notifier)
{
   if( notifier != dbObs )
      return;

   if( numMiscs != dbObs->getNumMiscs() )
   {
      miscTableWidget->getModel()->removeAll();
      populateTable();
   }
}

void MiscDialog::startObservingDB()
{
   dbObs = Database::getDatabase();
   populateTable();
}

void MiscDialog::populateTable()
{
   unsigned int i;

   if( ! Database::isInitialized() )
      return;

   numMiscs = dbObs->getNumMiscs();
   for( i = 0; i < numMiscs; ++i )
      miscTableWidget->getModel()->addMisc(dbObs->getMisc(i));
}

void MiscDialog::addMisc()
{
   QModelIndexList selected = miscTableWidget->selectedIndexes();
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

   Misc *misc = miscTableWidget->getModel()->getMisc(row);
   mainWindow->addMiscToRecipe(new Misc(*misc) ); // Need to add a copy so we don't change the database.
}
