/*
 * HopDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Luke Vincent <luke.r.vincent@gmail.com>
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
#include "HopDialog.h"
#include "database.h"
#include "recipe.h"
#include "MainWindow.h"
#include "hop.h"
#include "HopEditor.h"
#include "HopTableModel.h"
#include "HopTableModel.h"
#include "HopSortFilterProxyModel.h"

HopDialog::HopDialog(MainWindow* parent) :
   QDialog(parent),
   mainWindow(parent),
   hopEditor(new HopEditor(this)),
   numHops(0)
{
   doLayout();

   hopTableModel = new HopTableModel(tableWidget, false);
   hopTableModel->setInventoryEditable(true);
   hopTableProxy = new HopSortFilterProxyModel(tableWidget);
   hopTableProxy->setSourceModel(hopTableModel);
   tableWidget->setModel(hopTableProxy);
   tableWidget->setSortingEnabled(true);
   tableWidget->sortByColumn( HOPNAMECOL, Qt::AscendingOrder );
   hopTableProxy->setDynamicSortFilter(true);
   hopTableProxy->setFilterKeyColumn(1);

   connect( pushButton_addToRecipe, SIGNAL( clicked() ), this, SLOT( addHop() ) );
   connect( pushButton_edit, &QAbstractButton::clicked, this, &HopDialog::editSelected );
   connect( pushButton_new, SIGNAL( clicked() ), this, SLOT( newHop() ) );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &HopDialog::removeHop);
   connect( tableWidget, &QAbstractItemView::doubleClicked, this, &HopDialog::addHop );
   connect( qLineEdit_searchBox, &QLineEdit::textEdited, this, &HopDialog::filterHops);

   hopTableModel->observeDatabase(true);
}

void HopDialog::doLayout()
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

void HopDialog::retranslateUi()
{
   setWindowTitle(tr("Hop Database"));
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

void HopDialog::removeHop()
{
   QModelIndex modelIndex, viewIndex;
   QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if (size == 0)
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for (i = 1; i < size; ++i)
   {
      if (selected[i].row() != row)
         return;
   }
   modelIndex = hopTableProxy->mapToSource(selected[0]);
   Hop *hop = hopTableModel->getHop(modelIndex.row());
   if (hop)
      Database::instance().remove(hop);
}

void HopDialog::addHop(const QModelIndex& index)
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
      row = selected.value(0).row();
      for( i = 1; i < size; ++i )
      {
         if( selected.value(i).row() != row )
            return;
      }

      translated = hopTableProxy->mapToSource(selected.value(0));
   }
   else
   {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if( index.column() == HOPNAMECOL )
         translated = hopTableProxy->mapToSource(index);
      else
         return;
   }
   
   Hop *hop = hopTableModel->getHop(translated.row());
   
   Database::instance().addToRecipe( mainWindow->currentRecipe(), hop );
}

void HopDialog::editSelected()
{
   QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
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

   translated = hopTableProxy->mapToSource(selected.value(0));
   Hop *hop = hopTableModel->getHop(translated.row());
   hopEditor->setHop(hop);
   hopEditor->show();
}

void HopDialog::newHop()
{
   newHop(QString());
}

void HopDialog::newHop(QString folder) 
{
   QString name = QInputDialog::getText(this, tr("Hop name"),
                                          tr("Hop name:"));
   if( name.isEmpty() )
      return;

   Hop* hop = new Hop(name,true);
   if ( ! folder.isEmpty() )
      hop->setFolder(folder);

   hopEditor->setHop(hop);
   hopEditor->show();
}

void HopDialog::filterHops(QString searchExpression)
{
    hopTableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    hopTableProxy->setFilterFixedString(searchExpression);
}
