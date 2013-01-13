/*
 * HopDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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
#include <QMetaProperty>
#include "ui_hopDialog.h"
#include "database.h"

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
class HopDialog : public QDialog, public Ui::hopDialog
{
   Q_OBJECT

public:
   HopDialog(MainWindow* parent);
   virtual ~HopDialog() {}

public slots:
   //! Add selected hop to current recipe.
   void addHop(const QModelIndex& = QModelIndex());
   //! Delete the selected hop from the database.
   void removeHop();
   //! Bring up the editor for the selected hop.
   void editSelected();
   //! Create a new hop.
   void newHop();

private:
   MainWindow* mainWindow;
   HopEditor* hopEditor;
   HopTableModel* hopTableModel;
   HopSortFilterProxyModel* hopTableProxy;
   int numHops;
};

#endif   /* _HOPDIALOG_H */
