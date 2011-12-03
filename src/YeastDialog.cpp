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

YeastDialog::YeastDialog(MainWindow* parent)
        : QDialog(parent), mainWindow(parent), yeastEditor(new YeastEditor(this)), numYeast(0)
{
   setupUi(this);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addYeast() ) );
   connect( pushButton_edit, SIGNAL( clicked() ), this, SLOT( editSelected() ) );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newYeast() ) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT( removeYeast() ) );
   connect( yeastTableWidget, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT( addYeast(const QModelIndex&) ) );
   
   connect( &(Database::instance()), SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   populateTable();
}

void YeastDialog::removeYeast()
{
   QModelIndexList selected = yeastTableWidget->selectedIndexes();
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
   translated = yeastTableWidget->getProxy()->mapToSource(selected[0]);
   Yeast *yeast = yeastTableWidget->getModel()->getYeast(translated.row());
   Database::instance().removeYeast(yeast);
}

void YeastDialog::changed(QMetaProperty prop, QVariant val)
{
   // Notifier should only be the database.
   if( sender() == &(Database::instance()) &&
       prop.propertyIndex() == Database::instance().metaObject().indexOfProperty("yeasts") )
   {
      yeastTableWidget->getModel()->removeAll();
      populateTable();
   }

}

void YeastDialog::populateTable()
{
   QList<Yeast*> yeasts;
   Database::instance().getYeasts(yeasts);
   
   numYeasts = yeasts.size();
   int i;
   for( i = 0; i < numYeasts; ++i )
      yeastTableWidget->getModel()->addYeast(yeasts[i]);
}

void YeastDialog::addYeast(const QModelIndex& index)
{
   QModelIndex translated;
   
   if( !index.isValid() )
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

      translated = yeastTableWidget->getProxy()->mapToSource(selected[0]);
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == YEASTNAMECOL )
         translated = yeastTableWidget->getProxy()->mapToSource(index);
      else
         return;
   }
   
   Yeast *yeast = yeastTableWidget->getModel()->getYeast(translated.row());
   
   // TODO: how should we restructure this call?
   mainWindow->addYeastToRecipe(new Yeast(*yeast) ); // Need to add a copy so we don't change the database.
}

void YeastDialog::editSelected()
{
   QModelIndexList selected   = yeastTableWidget->selectedIndexes();
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
   translated = yeastTableWidget->getProxy()->mapToSource(selected[0]);
   Yeast *yeast = yeastTableWidget->getModel()->getYeast(translated.row());
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
}
