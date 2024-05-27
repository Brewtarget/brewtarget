/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * HydrometerTool.h is part of Brewtarget, and is copyright the following authors 2016-2023:
 *   • Jamie Daws <jdelectronics1@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Ryan Hoobler <rhoob@yahoo.com>
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
#ifndef HYDROMETERTOOL_H
#define HYDROMETERTOOL_H
#pragma once

#include <QDialog>

#include "widgets/SmartLabel.h"
#include "widgets/SmartLineEdit.h"

class QEvent;
class QGroupBox;
class QPushButton;
class QWidget;

class HydrometerTool : public QDialog {
   Q_OBJECT
public:
   HydrometerTool(QWidget* parent = nullptr);

   //! \name Public UI Variables
   //! @{
   QPushButton   * pushButton_convert;
   SmartLabel    * label_inputSg;
   SmartLineEdit * lineEdit_inputSg;
   SmartLabel    * label_outputSg;
   SmartLineEdit * lineEdit_outputSg;

   SmartLabel    * label_inputTemp;
   SmartLineEdit * lineEdit_inputTemp;
   SmartLabel    * label_calibratedTemp;
   SmartLineEdit * lineEdit_calibratedTemp;
   QGroupBox     * groupBox_inputSg;
   //! @}

public slots:
   void convert();

protected:
   virtual void changeEvent(QEvent* event);

private:
   void doLayout();
   void retranslateUi();
};

#endif
