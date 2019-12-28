/*
 * FermentableDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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



FermentableDialog::FermentableDialog(MainWindow* parent) :
   QDialog(parent),
   mainWindow(parent),
   fermEdit(new FermentableEditor(this)),
   numFerms(0)
{
   doLayout();

   fermTableModel = new FermentableTableModel(tableWidget, false);
   fermTableModel->setInventoryEditable(true);
   fermTableProxy = new FermentableSortFilterProxyModel(tableWidget);
   fermTableProxy->setSourceModel(fermTableModel);
   tableWidget->setModel(fermTableProxy);
   tableWidget->setSortingEnabled(true);
   tableWidget->sortByColumn( FERMNAMECOL, Qt::AscendingOrder );
   fermTableProxy->setDynamicSortFilter(true);
   fermTableProxy->setFilterKeyColumn(1);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addFermentable() ) );
   connect( pushButton_edit, &QAbstractButton::clicked, this, &FermentableDialog::editSelected );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &FermentableDialog::removeFermentable );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newFermentable() ) );
   connect( tableWidget, &QAbstractItemView::doubleClicked, this, &FermentableDialog::addFermentable );
   connect( qLineEdit_searchBox, &QLineEdit::textEdited, this, &FermentableDialog::filterFermentables);
   // Let me see if this works
   fermTableModel->observeDatabase(true);
}

void FermentableDialog::doLayout()
{
   resize(800, 300);
   verticalLayout = new QVBoxLayout(this);
      tableWidget = new QTableView(this);
      horizontalLayout = new QHBoxLayout();
         qLineEdit_searchBox = new QLineEdit();
         qLineEdit_searchBox->setMaxLength(30);
         qLineEdit_searchBox->setPlaceholderText("Enter filter");
         horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
         pushButton_addToRecipe = new QPushButton(this);
            pushButton_addToRecipe->setObjectName(QStringLiteral("pushButton_addToRecipe"));
            pushButton_addToRecipe->setAutoDefault(false);
            pushButton_addToRecipe->setDefault(true);
         pushButton_new = new QPushButton(this);
            pushButton_new->setObjectName(QStringLiteral("pushButton_new"));
            pushButton_new->setAutoDefault(false);
         pushButton_edit = new QPushButton(this);
            pushButton_edit->setObjectName(QStringLiteral("pushButton_edit"));
            QIcon icon;
            icon.addFile(QStringLiteral(":/images/edit.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_edit->setIcon(icon);
            pushButton_edit->setAutoDefault(false);
         pushButton_remove = new QPushButton(this);
            pushButton_remove->setObjectName(QStringLiteral("pushButton_remove"));
            QIcon icon1;
            icon1.addFile(QStringLiteral(":/images/smallMinus.svg"), QSize(), QIcon::Normal, QIcon::Off);
            pushButton_remove->setIcon(icon1);
            pushButton_remove->setAutoDefault(false);
         horizontalLayout->addWidget(qLineEdit_searchBox);
         horizontalLayout->addItem(horizontalSpacer);
         horizontalLayout->addWidget(pushButton_addToRecipe);
         horizontalLayout->addWidget(pushButton_new);
         horizontalLayout->addWidget(pushButton_edit);
         horizontalLayout->addWidget(pushButton_remove);
      verticalLayout->addWidget(tableWidget);
      verticalLayout->addLayout(horizontalLayout);

   retranslateUi();
   QMetaObject::connectSlotsByName(this);
}

void FermentableDialog::retranslateUi()
{
   setWindowTitle(tr("Fermentable Database"));
   pushButton_addToRecipe->setText(tr("Add to Recipe"));
   pushButton_new->setText(tr("New"));
   pushButton_edit->setText(QString());
   pushButton_remove->setText(QString());
#ifndef QT_NO_TOOLTIP
   pushButton_addToRecipe->setToolTip(tr("Add selected ingredient to recipe"));
   pushButton_new->setToolTip(tr("Create new ingredient"));
   pushButton_edit->setToolTip(tr("Edit selected ingredient"));
   pushButton_remove->setToolTip(tr("Remove selected ingredient"));
#endif // QT_NO_TOOLTIP
}

void FermentableDialog::removeFermentable()
{
   QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
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
   Database::instance().remove(ferm);
}

void FermentableDialog::editSelected()
{
   QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
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
      QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
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

void FermentableDialog::newFermentable(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Fermentable name"),
                                          tr("Fermentable name:"));
   if( name.isEmpty() )
      return;
   
   Fermentable* ferm = new Fermentable(name);
   if ( ! folder.isEmpty() )
      ferm->setFolder(folder);

   fermEdit->setFermentable(ferm);
   fermEdit->show();
}

void FermentableDialog::newFermentable() {
   newFermentable(QString());
}

void FermentableDialog::filterFermentables(QString searchExpression)
{
    fermTableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    fermTableProxy->setFilterFixedString(searchExpression);
}
