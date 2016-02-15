/*
 * HopDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _HOPDIALOG_H
#define _HOPDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QSpacerItem>
#include <QPushButton>

// Forward declarations.
class MainWindow;
class HopEditor;
class HopTableModel;
class HopSortFilterProxyModel;

/*!
 * \class HopDialog
 * \author Philip G. Lee
 *
 * \brief View/controller class for showing/editing the list of hops in the database.
 */
class HopDialog : public QDialog
{
   Q_OBJECT

public:
   HopDialog(MainWindow* parent);
   virtual ~HopDialog() {}

   //! \name Public UI Variables
   //! @{
   QVBoxLayout *verticalLayout;
   QTableView *tableWidget;
   QHBoxLayout *horizontalLayout;
   QSpacerItem *horizontalSpacer;
   QPushButton *pushButton_addToRecipe;
   QPushButton *pushButton_new;
   QPushButton *pushButton_edit;
   QPushButton *pushButton_remove;
   QLineEdit *qLineEdit_searchBox;
   //! @}

   void newHop(QString folder);

public slots:
   //! Add selected hop to current recipe.
   void addHop(const QModelIndex& = QModelIndex());
   //! Delete the selected hop from the database.
   void removeHop();
   //! Bring up the editor for the selected hop.
   void editSelected();
   //! Create a new hop.
   void newHop();
   //! FIlters the shown hops
   void filterHops(QString searchExpression);

protected:

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QDialog::changeEvent(event);
   }

private:
   MainWindow* mainWindow;
   HopEditor* hopEditor;
   HopTableModel* hopTableModel;
   HopSortFilterProxyModel* hopTableProxy;
   int numHops;

   void doLayout();
   void retranslateUi();
};

#endif   /* _HOPDIALOG_H */
