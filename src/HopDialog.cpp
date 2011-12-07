/*
 * HopDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "HopDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "hop.h"
#include "HopEditor.h"
#include "HopTableModel.h"

HopDialog::HopDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), hopEditor(new HopEditor(this)), numHops(0)
{
   setupUi(this);
   
   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addHop() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newHop() ) );
   connect( pushButton_remove, SIGNAL( clicked() ), this, SLOT( removeHop() ));
   connect( hopTableWidget, SIGNAL( doubleClicked(const QModelIndex&) ), this, SLOT( addHop(const QModelIndex&) ) );
   
   connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   populateTable();
}

void HopDialog::removeHop()
{
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   // ---------------Artificial block-------------------
   {
      QModelIndexList selected = hopTableWidget->selectedIndexes();
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

      viewIndex = selected.value(0);
   } // If we blow up here, it's because something is wrong with selected's destructor.
   //----------------END Artificial block---------------

   modelIndex = hopTableWidget->getProxy()->mapToSource(viewIndex);

   //std::cerr << "Model: " << modelIndex.row() << " View: " << viewIndex.row() << std::endl;

   Hop *hop = hopTableWidget->getModel()->getHop(modelIndex.row());
   Database::instance().removeHop(hop);
}

void HopDialog::changed(QMetaProperty prop, QVariant val)
{
   if( sender() == &(Database::instance()) &&
       QString(prop.name()) == "hops" )
   {
      hopTableWidget->getModel()->removeAll();
      populateTable();
   }
}

void HopDialog::populateTable()
{
   QList<Hop*> hops;
   Database::instance().getHops(hops);

   numHops = hops.length();
   int i;
   for( i = 0; i < numHops; ++i )
      hopTableWidget->getModel()->addHop(hops[i]);
}

void HopDialog::addHop(const QModelIndex& index)
{
   QModelIndex translated;
   if( !index.isValid() )
   {
      QModelIndexList selected = hopTableWidget->selectedIndexes();
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

      translated = hopTableWidget->getProxy()->mapToSource(selected.value(0));
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == HOPNAMECOL )
         translated = hopTableWidget->getProxy()->mapToSource(index);
      else
         return;
   }
   
   Hop *hop = hopTableWidget->getModel()->getHop(translated.row());
   
   Database::instance().addToRecipe( mainWindow->currentRecipe(), hop );
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
   row = selected.value(0).row();
   for( i = 1; i < size; ++i )
   {
      if( selected.value(i).row() != row )
         return;
   }

   translated = hopTableWidget->getProxy()->mapToSource(selected.value(0));
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

   Hop* hop = Database::instance().newHop();
   hop->setName(name);
   hopEditor->setHop(hop);
   hopEditor->show();
}
