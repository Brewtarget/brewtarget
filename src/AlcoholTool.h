/*
 * AlcoholTool.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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

#include <QDialog>
#include <QEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

#include "BtLineEdit.h"

/*!
 * \brief Dialog to convert units.
 */
class AlcoholTool : public QDialog {
   Q_OBJECT

public:
   AlcoholTool(QWidget* parent = nullptr);
   virtual ~AlcoholTool();

public slots:
   void convert();

protected:
   virtual void changeEvent(QEvent* event);

private:
   QPushButton   * pushButton_convert;
   QLabel        * label_og;
   BtDensityEdit * input_og;
   QLabel        * label_fg;
   BtDensityEdit * input_fg;
   QLabel        * label_result;
   QLabel        * output_result;
   QHBoxLayout   * hLayout;
   QFormLayout   * formLayout;
   QVBoxLayout   * vLayout;
   QSpacerItem   * verticalSpacer;
   QSpacerItem   * verticalSpacer2;
   QSpacerItem   * verticalSpacer3;

   void doLayout();
   void retranslateUi();
};

#endif
