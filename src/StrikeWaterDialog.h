/*
 * StrikeWaterDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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
#ifndef _STRIKEWATERDIALOG_H
#define _STRIKEWATERDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>
#include "ui_strikeWaterDialog.h"
#include "Unit.h"

/*!
 * \class StrikeWaterDialog
 * \author Maxime Lavigne
 *
 * \brief Dialog to calculate the amount and temperature of the strike water.
 */
class StrikeWaterDialog : public QDialog, public Ui::strikeWaterDialog
{
  Q_OBJECT
  public:
    StrikeWaterDialog(QWidget* parent = 0);
    ~StrikeWaterDialog();

  public slots:
    void calculate();

  private:
    static const double specificHeatBarley;

    double initialInfusionSi(double grainTemp, double targetTemp, double waterToGrain);
    double mashInfusionSi(double initialTemp, double targetTemp,
        double grainWeight, double infusionWater, double mashVolume);

    double computeInitialInfusion();
    double computeMashInfusion();
};

#endif
