/*
 * MiscDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <string>
#include <QList>
#include "MiscDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "misc.h"
#include "MiscEditor.h"
#include "MiscTableModel.h"

MiscDialog::MiscDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), numMiscs(0), miscEdit(newMiscEditor(this))
{
   setupUi(this);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addMisc() ) );
   connect( pushButton_new, SIGNAL(clicked()), this, SLOT( newMisc() ) );
   connect( pushButton_edit, SIGNAL(clicked()), this, SLOT(editSelected()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeMisc()) );
   connect( miscTableWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT( addMisc(const QModelIndex&) ) );
   
   connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   populateTable();
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
   Database::instance().removeMisc(m);
}

void MiscDialog::changed(QMetaProperty prop, QVariant /*value*/)
{
   if( sender() == &(Database::instance()) &&
       prop.propertyIndex() == Database::instance().metaObject().indexOfProperty("miscs") )
   {
      miscTableWidget->getModel()->removeAll();
      populateTable();
   }
}

void MiscDialog::populateTable()
{
   QList<Misc*> miscs;
   Database::instance().getFermentables(ferms);

   numMiscs = miscs.size();
   int i;
   for( i = 0; i < numMiscs; ++i )
      miscTableWidget->getModel()->addMisc(miscs[i]);
}

void MiscDialog::addMisc(const QModelIndex& index)
{
   QModelIndex translated;
   
   if( !index.isValid() )
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
      
      translated = selected[0];
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == MISCNAMECOL )
         translated = miscTableWidget->getProxy()->mapToSource(index);
      else
         return;
   }
   
   Misc *misc = miscTableWidget->getModel()->getMisc(translated.row());
   
   // TODO: how should we restructure this call?
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

   Misc* m = Database::instance().newMisc();
   m->setName(name);
   miscEdit->setMisc(m);
   miscEdit->show();
}
