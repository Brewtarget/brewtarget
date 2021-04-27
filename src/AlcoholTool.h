/*
 * AlcoholTool.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2018
 * - Ryan Hoobler <rhoob@yahoo.com>
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
#ifndef ALCOHOLTOOL_H
#define ALCOHOLTOOL_H

class AlcoholTool;

#include <QDialog>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QEvent>
#include <brewtarget.h>
#include "BtLineEdit.h"
#include "BtLabel.h"

/*!
 * \brief Dialog to convert units.
 * \author Philip G. Lee
 */
class AlcoholTool : public QDialog
{
   Q_OBJECT
public:

   AlcoholTool(QWidget* parent=0);

   //! \name Public UI Variables
   //! @{
   QPushButton* pushButton_convert;
   QLabel* label_og;
   BtLineEdit* lineEdit_og;
   QLabel* label_fg;
   BtLineEdit* lineEdit_fg;
   QLabel *label_result;
   QLineEdit *lineEdit_result;
   //! @}

public slots:
   void convert();

protected:

   virtual void changeEvent(QEvent* event)
   {
      if(event->type() == QEvent::LanguageChange)
         retranslateUi();
      QDialog::changeEvent(event);
   }

private:

   void doLayout();
   void retranslateUi();
};

#endif
