/*
 * YeastDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _YEASTDIALOG_H
#define   _YEASTDIALOG_H

class YeastDialog;

#include <QWidget>
#include <QDialog>
#include <QVariant>
#include "ui_yeastDialog.h"
#include "observable.h"
#include "database.h"
class MainWindow;
#include "YeastEditor.h"

class YeastDialog : public QDialog, public Ui::yeastDialog, public Observer
{
   Q_OBJECT

public:
   YeastDialog(MainWindow* parent);
   virtual ~YeastDialog() {}
   void startObservingDB();
   virtual void notify(Observable *notifier, QVariant info = QVariant()); // From Observer

public slots:
   void addYeast();
   void removeYeast();
   void editSelected();
   void newYeast();

private:
   Database* dbObs;
   MainWindow* mainWindow;
   YeastEditor* yeastEditor;
   unsigned int numYeasts;

   void populateTable();
};

#endif   /* _YEASTDIALOG_H */

