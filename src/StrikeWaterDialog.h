/*
 * StrikeWaterDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Philip G. Lee <rocketman768@gmail.com>
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
#ifndef STRIKEWATERDIALOG_H
#define STRIKEWATERDIALOG_H
#pragma once

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>

#include "ui_strikeWaterDialog.h"

#include "measurement/Unit.h"

/*!
 * \class StrikeWaterDialog
 *
 * \brief Dialog to calculate the amount and temperature of the strike water.
 */
class StrikeWaterDialog : public QDialog, public Ui::strikeWaterDialog {
   Q_OBJECT
public:
   StrikeWaterDialog(QWidget* parent = 0);
   ~StrikeWaterDialog();

public slots:
   void calculate();

private:
   /**
    * \brief
    */
   double computeInitialInfusion();

   /**
    * \brief
    */
   double computeMashInfusion();
};

#endif
