/*
 * HydrometerTool.h is part of Brewtarget and was written by Ryan Hoobler
 * (rhoob@yahoo.com).  Copyright is granted to Philip G. Lee
 * (rocketman768@gmail.com), 2016.
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

#ifndef _HYDROMETERTOOL_H
#define  _HYDROMETERTOOL_H

class HydrometerTool;

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
class HydrometerTool : public QDialog
{
   Q_OBJECT
public:

   HydrometerTool(QWidget* parent=0);

   //! \name Public UI Variables
   //! @{
   QPushButton* pushButton_convert;
   QLabel* inputLabel;
   BtLineEdit* lineEdit_inputSg;
   QLabel* outputLabel;
   BtLineEdit* lineEdit_outputSg;
 //  QLabel* inputTempLabel;
 //  BtTemperatureEdit* lineEdit_inputTemp;

   BtTemperatureLabel *label_inputTemp;
   BtTemperatureEdit *lineEdit_inputTemp;
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
