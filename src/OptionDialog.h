/*
 * OptionDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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

#ifndef _OPTIONDIALOG_H
#define   _OPTIONDIALOG_H

class OptionDialog;

#include <QDialog>
#include <QWidget>
#include <QAbstractButton>
#include <QMap>
#include <QString>
#include <QVector>
#include <QIcon>
#include "ui_optionsDialog.h"
#include "unit.h"

/*!
 * \class OptionDialog
 * \author Philip G. Lee
 *
 * \brief View/controller dialog to manage options.
 */
class OptionDialog : public QDialog, public Ui::optionsDialog
{
   Q_OBJECT
public:
   //! \brief Default constructor.
   OptionDialog(QWidget *parent=0);

public slots:
   //! \brief Show the dialog.
   void show();
   //! \brief Save the options and close the dialog.
   void saveAndClose();
   //! \brief Close dialog without saving options.
   void cancel();
   //! \brief Pop up a dialog to choose the data directory.
   void setDataDir();
   //! \brief Reset data directory to default.
   void defaultDataDir();
   //! \brief Enable or disable the configuration panel based on the engine choice
   void setEngine(int selected);
   //! \brief test connection to remote databases. This could get ugly
   void testConnection();
   //! \brief mark a change to the database config
   void testRequired();
   //! \brief handle the dialogs for saving passwords
   void savePassword(bool state);
   
protected:
   
   //! \brief Reimplemented from QWidget.
   virtual void changeEvent(QEvent* e);

private:
   enum testStates {
      NOCHANGE,
      NEEDSTEST,
      TESTFAILED,
      TESTPASSED
   };

   testStates status;
   // Update UI strings according to current language.
   void retranslate();
   // Update dialog with current options.
   void showChanges();
   //
   void changeColors();
   QButtonGroup *colorGroup, *ibuGroup;
   QStringList ndxToLangCode;
   QVector<QIcon> langIcons;
};

#endif   /* _OPTIONDIALOG_H */
