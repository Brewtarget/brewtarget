/*
 * OptionDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _OPTIONDIALOG_H
#define   _OPTIONDIALOG_H

class OptionDialog;

#include <QDialog>
#include <QWidget>
#include <QAbstractButton>
#include <QMap>
#include <QString>
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
   
protected:
   
   //! \brief Reimplemented from QWidget.
   virtual void changeEvent(QEvent* e);
   
private:
   // Update UI strings according to current language.
   void retranslate();
   // Update dialog with current options.
   void showChanges();
   QButtonGroup *colorGroup, *ibuGroup;
   QButtonGroup *weightGroup, *volumeGroup, *tempGroup, *gravGroup, *colorUnitGroup;
   QStringList ndxToLangCode;
   
};

#endif   /* _OPTIONDIALOG_H */
