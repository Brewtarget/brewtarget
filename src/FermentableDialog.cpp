/*
 * FermentableDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <QString>
#include <QInputDialog>
#include <QList>
#include <string>
#include "FermentableDialog.h"
#include "observable.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "fermentable.h"

FermentableDialog::FermentableDialog(MainWindow* parent)
        : QDialog(parent)
{
   setupUi(this);
   mainWindow = parent;
   dbObs = 0;
   numFerms = 0;
   fermEdit = new FermentableEditor(this);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addFermentable() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeFermentable() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newFermentable() ) );
}

void FermentableDialog::removeFermentable()
{
   QModelIndexList selected = fermentableTableWidget->selectedIndexes();
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

   translated = fermentableTableWidget->getProxy()->mapToSource(selected[0]);
   Fermentable* ferm = fermentableTableWidget->getModel()->getFermentable(translated.row());
   dbObs->removeFermentable(ferm);
}

void FermentableDialog::editSelected()
{
   QModelIndexList selected = fermentableTableWidget->selectedIndexes();
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

   translated = fermentableTableWidget->getProxy()->mapToSource(selected[0]);
   Fermentable* ferm = fermentableTableWidget->getModel()->getFermentable(translated.row());
   fermEdit->setFermentable(ferm);
   fermEdit->show();
}

void FermentableDialog::notify(Observable *notifier, QVariant info)
{
   if( notifier != dbObs || (info.toInt() != DBFERM && info.toInt() != DBALL) )
      return;

   fermentableTableWidget->getModel()->removeAll();
   populateTable();
}

void FermentableDialog::startObservingDB()
{
   dbObs = Database::getDatabase();
   setObserved(dbObs);
   populateTable();
}

void FermentableDialog::populateTable()
{
   QList<Fermentable*>::iterator it, end;

   if( ! Database::isInitialized() )
      return;

   numFerms = dbObs->getNumFermentables();
   end = dbObs->getFermentableEnd();
   for( it = dbObs->getFermentableBegin(); it != end; ++it )
      fermentableTableWidget->getModel()->addFermentable(*it);
}

void FermentableDialog::addFermentable()
{
   QModelIndexList selected = fermentableTableWidget->selectedIndexes();
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

   translated = fermentableTableWidget->getProxy()->mapToSource(selected[0]);
   Fermentable *ferm = fermentableTableWidget->getModel()->getFermentable(translated.row());
   mainWindow->addFermentableToRecipe(new Fermentable(*ferm) ); // Need to add a copy so we don't change the database.
}

void FermentableDialog::newFermentable()
{
   QString name = QInputDialog::getText(this, tr("Fermentable name"),
                                          tr("Fermentable name:"));
   if( name.isEmpty() )
      return;
   
   Fermentable *ferm = new Fermentable();
   QString stdname = name;
   ferm->setName(stdname);

   dbObs->addFermentable(ferm);
   fermEdit->setFermentable(ferm);
   fermEdit->show();
}
