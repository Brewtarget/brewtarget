/*
 * MiscDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#ifndef _MISCDIALOG_H
#define	_MISCDIALOG_H

class MiscDialog;

#include <QWidget>
#include <QDialog>
#include "ui_miscDialog.h"
#include "observable.h"
#include "database.h"
#include "MainWindow.h"

class MiscDialog : public QDialog, public Ui::miscDialog, public Observer
{
   Q_OBJECT

public:
   MiscDialog(MainWindow* parent);
   void startObservingDB();
   virtual void notify(Observable *notifier); // From Observer

public slots:

   void addMisc();

private:
   Database* dbObs;
   MainWindow* mainWindow;
   unsigned int numMiscs;

   void populateTable();
};

#endif	/* _MISCDIALOG_H */

