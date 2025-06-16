/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * OptionDialog.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H
#pragma once

#include <memory> // For PImpl

#include <QDialog>
#include <QWidget>

#include "ui_optionsDialog.h"

/*!
 * \class OptionDialog
 *
 * \brief View/controller dialog to manage options.
 */
class OptionDialog : public QDialog, public Ui::optionsDialog {
   Q_OBJECT

public:
   //! \brief Default constructor.
   OptionDialog(QWidget * parent = 0);
   ~OptionDialog();

signals:
   void showAllAncestors(bool showem);

public slots:
   //! \brief Show the dialog.
   void show();
   //! \brief Save the options and close the dialog.
   void saveAndClose();
   //! \brief Close dialog without saving options.
   void cancel();
   //! \brief Pop up a dialog to choose the data directory.
   void setDataDir();
   void setBackupDir();
   //! \brief Pop up a dialog to choose the Log file directory.
   void setLogDir();
   //! \brief Reset data directory to default.
   void resetToDefault();

   //! \brief Enable or disable the configuration panel based on the engine choice
   void setEngine(int selected);
   //! \brief test connection to remote databases. This could get ugly
   void testConnection();
   //! \brief mark a change to the database config
   void testRequired();
   //! \brief handle the dialogs for saving passwords
   void savePassword(bool state);
   //! \brief does the version options
   void versioningChanged(bool state);

   void setFileLocationState(bool state);

protected:

   //! \brief Reimplemented from QWidget.
   virtual void changeEvent(QEvent * e);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   void connect_signals();

   bool saveDatabaseConfig();
   bool saveDefaultUnits();
   void saveLoggingSettings();
   void saveVersioningSettings();
   bool transferDatabase();
   void saveSqliteConfig();
   void saveFormulae();

   bool saveWeightUnits();
   bool saveTemperatureUnits();
   bool saveVolumeUnits();
   bool saveGravityUnits();
   bool saveDateFormat();
   bool saveColorUnits();
   bool saveDiastaticUnits();

   void signalAncestors();
};

#endif
