/*
 * MiscDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef MISCDIALOG_H
#define MISCDIALOG_H
#pragma once

#include <QDialog>
#include <QEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QTableView>
#include <QVBoxLayout>
#include <QWidget>

// Forward declarations.
class MainWindow;
class MiscEditor;
class MiscTableModel;
class MiscSortFilterProxyModel;

/*!
 * \class MiscDialog
 *
 * \brief View/controller dialog for the miscs in the database.
 */
class MiscDialog : public QDialog {
   Q_OBJECT

public:
   MiscDialog(MainWindow* parent);
   virtual ~MiscDialog() {}

   //! \name Public UI Variables
   //! @{
   QVBoxLayout *verticalLayout;
   QTableView *tableWidget;
   QHBoxLayout *horizontalLayout;
   QLineEdit *qLineEdit_searchBox;
   QSpacerItem *horizontalSpacer;
   QPushButton *pushButton_addToRecipe;
   QPushButton *pushButton_new;
   QPushButton *pushButton_edit;
   QPushButton *pushButton_remove;
   //! @}

public slots:
   //! Add the selected misc to the current recipe.
   void addMisc(const QModelIndex& = QModelIndex());
   //! Delete selected misc from the database.
   void removeMisc();
   //! Bring up the editor for the selected misc.
   void editSelected();
   //! Add a new misc to the database.
   void newMisc(QString folder = "");
   //! Filter out the matching miscs.
   void filterMisc(QString searchExpression);

protected:

   virtual void changeEvent(QEvent* event);

private:
   MainWindow* mainWindow;
   MiscTableModel* miscTableModel;
   MiscSortFilterProxyModel* miscTableProxy;
   int numMiscs;
   MiscEditor* miscEdit;

   void doLayout();
   void retranslateUi();
};

#endif
