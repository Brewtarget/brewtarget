/*
 * HopDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _HOPDIALOG_H
#define   _HOPDIALOG_H

class HopDialog;

#include <QWidget>
#include <QDialog>
#include <QVariant>
#include "ui_hopDialog.h"
#include "observable.h"
#include "database.h"
class MainWindow;
#include "HopEditor.h"

class HopDialog : public QDialog, public Ui::hopDialog, public Observer
{
   Q_OBJECT

public:
   HopDialog(MainWindow* parent);
   void startObservingDB();
   virtual void notify(Observable *notifier, QVariant info = QVariant()); // From Observer

public slots:
   void addHop();
   void removeHop();
   void editSelected();
   void newHop();

private:
   Database* dbObs;
   MainWindow* mainWindow;
   HopEditor* hopEditor;
   unsigned int numHops;

   void populateTable();
};


#endif   /* _HOPDIALOG_H */

