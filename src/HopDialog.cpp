/*
 * HopDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "HopDialog.h"
#include "observable.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "hop.h"
#include "HopEditor.h"

HopDialog::HopDialog(MainWindow* parent)
        : QDialog(parent)
{
   setupUi(this);
   mainWindow = parent;
   dbObs = 0;
   numHops = 0;
   hopEditor = new HopEditor(this);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addHop() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newHop() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeHop() ));
}

void HopDialog::removeHop()
{
   QModelIndexList selected = hopTableWidget->selectedIndexes();
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

   translated = hopTableWidget->getProxy()->mapFromSource(selected[0]);
   Hop *hop = hopTableWidget->getModel()->getHop(translated.row());
   dbObs->removeHop(hop);
}

void HopDialog::notify(Observable *notifier, QVariant info)
{
   if( notifier != dbObs || (info.toInt() != DBHOP && info.toInt() != DBALL) )
      return;

   hopTableWidget->getModel()->removeAll();
   populateTable();
}

void HopDialog::startObservingDB()
{
   dbObs = Database::getDatabase();
   setObserved(dbObs);
   populateTable();
}

void HopDialog::populateTable()
{
   std::list<Hop*>::iterator it, end;


   if( ! Database::isInitialized() )
      return;

   numHops = dbObs->getNumHops();
   end = dbObs->getHopEnd();
   for( it = dbObs->getHopBegin(); it != end; ++it )
      hopTableWidget->getModel()->addHop(*it);
}

void HopDialog::addHop()
{
   QModelIndexList selected = hopTableWidget->selectedIndexes();
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

   translated = hopTableWidget->getProxy()->mapToSource(selected[0]);
   Hop *hop = hopTableWidget->getModel()->getHop(translated.row());
   mainWindow->addHopToRecipe(new Hop(*hop) ); // Need to add a copy so we don't change the database.
}

void HopDialog::editSelected()
{
   QModelIndexList selected = hopTableWidget->selectedIndexes();
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

   translated = hopTableWidget->getProxy()->mapToSource(selected[0]);
   Hop *hop = hopTableWidget->getModel()->getHop(translated.row());
   hopEditor->setHop(hop);
   hopEditor->show();
}

void HopDialog::newHop()
{
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                          tr("Hop name:"));
   if( name.isEmpty() )
      return;

   Hop* hop = new Hop();
   QString stdname = name;
   hop->setName(stdname);

   dbObs->addHop(hop);
   hopEditor->setHop(hop);
   hopEditor->show();
}
