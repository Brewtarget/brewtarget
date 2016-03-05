/*
 * YeastDialog.h is part of Brewtarget, and is Copyright the following
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

#ifndef _YEASTDIALOG_H
#define _YEASTDIALOG_H

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
class YeastEditor;
class YeastEditor;
class YeastTableModel;
class YeastSortFilterProxyModel;

/*!
 * \class YeastDialog
 * \author Philip G. Lee
 *
 * \brief View/controller dialog for displaying all the yeasts in the database.
 */
class YeastDialog : public QDialog
{
   Q_OBJECT

public:
   YeastDialog(MainWindow* parent);
   virtual ~YeastDialog() {}

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

   void newYeast(QString folder);
public slots:
   void addYeast(const QModelIndex& = QModelIndex());
   void removeYeast();
   void editSelected();
   void newYeast();
   void filterYeasts(QString searchExpression);

protected:

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QDialog::changeEvent(event);
   }

private:
   MainWindow* mainWindow;
   YeastTableModel* yeastTableModel;
   YeastSortFilterProxyModel* yeastTableProxy;
   YeastEditor* yeastEditor;
   int numYeasts;

   void doLayout();
   void retranslateUi();
};

#endif   /* _YEASTDIALOG_H */

