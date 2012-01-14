/*
 * YeastDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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

#ifndef _YEASTDIALOG_H
#define   _YEASTDIALOG_H

class YeastDialog;

#include <QWidget>
#include <QDialog>
#include <QVariant>
#include <QMetaProperty>
#include "ui_yeastDialog.h"

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
class YeastDialog : public QDialog, public Ui::yeastDialog
{
   Q_OBJECT

public:
   YeastDialog(MainWindow* parent);
   virtual ~YeastDialog() {}

public slots:
   void addYeast(const QModelIndex& = QModelIndex());
   void removeYeast();
   void editSelected();
   void newYeast();

   void changed(QMetaProperty, QVariant);
private:
   MainWindow* mainWindow;
   YeastTableModel* yeastTableModel;
   YeastSortFilterProxyModel* yeastTableProxy;
   YeastEditor* yeastEditor;
   int numYeasts;

   void populateTable();
};

#endif   /* _YEASTDIALOG_H */

