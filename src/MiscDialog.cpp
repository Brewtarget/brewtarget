/*
 * MiscDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
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
#include "MiscDialog.h"

#include <QDialog>
#include <QInputDialog>
#include <QList>
#include <QString>
#include <QWidget>

//#include "database/Database.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"
#include "MainWindow.h"
#include "model/Misc.h"
#include "MiscEditor.h"
#include "MiscSortFilterProxyModel.h"
#include "tableModels/MiscTableModel.h"

MiscDialog::MiscDialog(MainWindow* parent) :
   QDialog(parent),
   mainWindow(parent),
   numMiscs(0),
   miscEdit(new MiscEditor(this)) {
   doLayout();

   miscTableModel = new MiscTableModel(tableWidget, false);
   miscTableModel->setInventoryEditable(true);
   miscTableProxy = new MiscSortFilterProxyModel(tableWidget);
   miscTableProxy->setSourceModel(miscTableModel);
   tableWidget->setModel(miscTableProxy);
   tableWidget->setSortingEnabled(true);
   tableWidget->sortByColumn(static_cast<int>(MiscTableModel::ColumnIndex::Name), Qt::AscendingOrder);
   miscTableProxy->setDynamicSortFilter(true);
   miscTableProxy->setFilterKeyColumn(1);

   // Note, per https://wiki.qt.io/New_Signal_Slot_Syntax#Default_arguments_in_slot, the use of a trivial lambda
   // function to allow use of default argument on newHop() slot
///   connect(this->pushButton_addToRecipe, &QAbstractButton::clicked,         this, &MiscDialog::addMisc     ); .:TODO:. Work out what this is supposed to do!
   connect(this->pushButton_new,         &QAbstractButton::clicked,         this, [this]() { this->newMisc(); return; } );
   connect(this->pushButton_edit,        &QAbstractButton::clicked,         this, &MiscDialog::editSelected);
   connect(this->pushButton_remove,      &QAbstractButton::clicked,         this, &MiscDialog::removeMisc  );
   connect(this->tableWidget,            &QAbstractItemView::doubleClicked, this, &MiscDialog::addMisc     );
   connect(this->qLineEdit_searchBox,    &QLineEdit::textEdited,            this, &MiscDialog::filterMisc  );

   miscTableModel->observeDatabase(true);
   return;
}

void MiscDialog::doLayout() {
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
   return;
}

void MiscDialog::retranslateUi() {
   setWindowTitle(tr("Misc Database"));
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
   return;
}

void MiscDialog::removeMisc() {
   QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();

   int size = selected.size();
   if( size == 0 ) {
      return;
   }

   // Make sure only one row is selected.
   int row = selected[0].row();
   for (int i = 1; i < size; ++i ) {
      if( selected[i].row() != row ) {
         return;
      }
   }

   auto m = miscTableModel->getRow(miscTableProxy->mapToSource(selected[0]).row());
   ObjectStoreWrapper::softDelete(*m);
   return;
}

void MiscDialog::addMisc(const QModelIndex& index) {
   QModelIndex translated;

   if ( !index.isValid() ) {
      QModelIndexList selected = tableWidget->selectionModel()->selectedIndexes();
      int row, size, i;

      size = selected.size();
      if( size == 0 ) {
         return;
      }

      // Make sure only one row is selected.
      row = selected[0].row();
      for( i = 1; i < size; ++i ) {
         if( selected[i].row() != row ) {
            return;
         }
      }

      // Always need to translate indices through the proxy
      translated = miscTableProxy->mapToSource(selected[0]);
   } else {
      // Only respond if the name is selected. Since we connect to double-click signal,
      // this keeps us from adding something to the recipe when we just want to edit
      // one of the other columns.
      if (static_cast<MiscTableModel::ColumnIndex>(index.column()) == MiscTableModel::ColumnIndex::Name) {
         translated = miscTableProxy->mapToSource(index);
      } else {
         return;
      }
   }

   MainWindow::instance().addMiscToRecipe(miscTableModel->getRow(translated.row()));

   return;
}

void MiscDialog::editSelected()
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

   auto m = miscTableModel->getRow(miscTableProxy->mapToSource(selected[0]).row());
   miscEdit->setMisc(m.get());
   miscEdit->show();
   return;
}

void MiscDialog::newMisc(QString folder) {
   QString name = QInputDialog::getText(this, tr("Misc name"),
                                              tr("Misc name:"));
   if(name.isEmpty())
      return;

   Misc* m = new Misc(name);
   if ( ! folder.isEmpty() )
      m->setFolder(folder);

   miscEdit->setMisc(m);
   miscEdit->show();
   return;
}

void MiscDialog::filterMisc(QString searchExpression) {
    miscTableProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    miscTableProxy->setFilterFixedString(searchExpression);
   return;
}

void MiscDialog::changeEvent(QEvent* event) {
   if(event->type() == QEvent::LanguageChange) {
      retranslateUi();
   }
   QDialog::changeEvent(event);
   return;
}
