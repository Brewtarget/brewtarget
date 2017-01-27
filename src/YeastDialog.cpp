/*
 * YeastDialog.cpp is part of Brewtarget, and is Copyright the following
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
   doLayout();

   yeastTableModel = new YeastTableModel(tableWidget, false);
   yeastTableModel->setInventoryEditable(true);
   yeastTableProxy = new YeastSortFilterProxyModel(tableWidget);
   yeastTableProxy->setSourceModel(yeastTableModel);
   tableWidget->setModel(yeastTableProxy);
   tableWidget->setSortingEnabled(true);
   tableWidget->sortByColumn( YEASTNAMECOL, Qt::AscendingOrder );
   yeastTableProxy->setDynamicSortFilter(true);
   yeastTableProxy->setFilterKeyColumn(1);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addYeast() ) );
   connect( pushButton_edit, &QAbstractButton::clicked, this, &YeastDialog::editSelected );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newYeast() ) );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &YeastDialog::removeYeast );
   connect( tableWidget, &QAbstractItemView::doubleClicked, this, &YeastDialog::addYeast );
   connect( qLineEdit_searchBox, &QLineEdit::textEdited, this, &YeastDialog::filterYeasts);

   yeastTableModel->observeDatabase(true);

}

void YeastDialog::doLayout()
{
   resize(800, 300);
   verticalLayout = new QVBoxLayout(this);
      tableWidget = new QTableView(this);
      horizontalLayout = new QHBoxLayout();
         horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
         qLineEdit_searchBox = new QLineEdit();
         qLineEdit_searchBox->setMaxLength(30);
         qLineEdit_searchBox->setPlaceholderText("Enter filter");
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

void YeastDialog::retranslateUi()
{
   setWindowTitle(tr("Yeast Database"));
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

void YeastDialog::removeYeast()
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

   // We need to translate from the view's index to the model's index.  The
   // proxy model does the heavy lifting, as long as we do the call.
   translated = yeastTableProxy->mapToSource(selected[0]);
   Yeast *yeast = yeastTableModel->getYeast(translated.row());
   Database::instance().remove(yeast);
}

void YeastDialog::addYeast(const QModelIndex& index)
{
   QModelIndex translated;
   
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
   translated = yeastTableProxy->mapToSource(selected[0]);
   Yeast *yeast = yeastTableModel->getYeast(translated.row());
   yeastEditor->setYeast(yeast);
   yeastEditor->show();
}

void YeastDialog::newYeast()
{
   newYeast(QString());
}

void YeastDialog::newYeast(QString folder)
{
   QString name = QInputDialog::getText(this, tr("Yeast name"),
                                              tr("Yeast name:"));
   if( name.isEmpty() )
      return;

   Yeast* y = Database::instance().newYeast();
   y->setName(name);
   if ( ! folder.isEmpty() )
      y->setFolder(folder);

   yeastEditor->setYeast(y);
   yeastEditor->show();
   y->setDisplay(true);
}

void YeastDialog::filterYeasts(QString searchExpression)
{
    yeastTableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    yeastTableProxy->setFilterFixedString(searchExpression);
}
