/*
 * FermentableDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _FERMENTABLEDIALOG_H
#define   _FERMENTABLEDIALOG_H

class FermentableDialog;

#include <QWidget>
#include <QDialog>
#include <QVariant>
#include "ui_fermentableDialog.h"
#include "observable.h"
#include "database.h"
//#include "MainWindow.h"
class MainWindow;
#include "FermentableEditor.h"

class FermentableDialog : public QDialog, public Ui::fermentableDialog, public Observer
{
   Q_OBJECT

public:
   FermentableDialog(MainWindow* parent);
   virtual ~FermentableDialog() {}
   void startObservingDB();
   virtual void notify(Observable *notifier, QVariant info = QVariant()); // From Observer

public slots:
   void addFermentable();
   void removeFermentable();
   void editSelected();
   void newFermentable();

private:
   Database* dbObs;
   MainWindow* mainWindow;
   FermentableEditor *fermEdit;
   unsigned int numFerms;

   void populateTable();
};

#endif   /* _FERMENTABLEDIALOG_H */

