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
#include <QInputDialog>
#include <QString>
#include <string>
#include <list>
#include "MiscDialog.h"
#include "observable.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "misc.h"
#include "MiscEditor.h"

MiscDialog::MiscDialog(MainWindow* parent)
        : QDialog(parent)
{
   setupUi(this);
   mainWindow = parent;
   dbObs = 0;
   numMiscs = 0;
   miscEdit = new MiscEditor(this);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addMisc() ) );
   connect( pushButton_new, SIGNAL(clicked()), this, SLOT( newMisc() ) );
   connect( pushButton_edit, SIGNAL(clicked()), this, SLOT(editSelected()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeMisc()) );
}

void MiscDialog::removeMisc()
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

   Misc* m = miscTableWidget->getModel()->getMisc(row);
   dbObs->removeMisc(m);
}

void MiscDialog::notify(Observable *notifier, QVariant info)
{
   if( notifier != dbObs || (info.toInt() != DBMISC && info.toInt() != DBALL) )
      return;

   miscTableWidget->getModel()->removeAll();
   populateTable();
}

void MiscDialog::startObservingDB()
{
   dbObs = Database::getDatabase();
   setObserved(dbObs);
   populateTable();
}

void MiscDialog::populateTable()
{
   std::list<Misc*>::iterator it, end;

   if( ! Database::isInitialized() )
      return;

   numMiscs = dbObs->getNumMiscs();
   end = dbObs->getMiscEnd();
   for( it = dbObs->getMiscBegin(); it != end; ++it )
      miscTableWidget->getModel()->addMisc(*it);
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

void MiscDialog::editSelected()
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

   Misc* m = miscTableWidget->getModel()->getMisc(row);
   miscEdit->setMisc(m);
   miscEdit->show();
}

void MiscDialog::newMisc()
{
   QString name = QInputDialog::getText(this, tr("Misc name"),
                                              tr("Misc name:"));
   if(name.isEmpty())
      return;

   Misc *m = new Misc();
   QString stdname = name;
   m->setName(stdname);

   dbObs->addMisc(m);
   miscEdit->setMisc(m);
   miscEdit->show();
}
