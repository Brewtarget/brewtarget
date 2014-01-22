/*
* PrimingDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
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
#ifndef _STRIKEWATERDIALOG_H
#define _STRIKEWATERDIALOG_H

#include <QDialog>
#include <QWidget>
#include <QButtonGroup>
#include "ui_strikeWaterDialog.h"
#include "unit.h"

/*! 
 * \class StrikeWaterDialog
 * \author Maxime Lavigne <malavv>
 *
 * \brief Dialog to calculate the amount and temperature of the strike water.
 */
class StrikeWaterDialog : public QDialog, public Ui::strikeWaterDialog 
{
  Q_OBJECT
  Q_PROPERTY( Unit* volume READ volumeUnit WRITE setVolumeUnit )
  Q_PROPERTY( Unit* weight READ weightUnit WRITE setWeightUnit )
  Q_PROPERTY( Unit* temp READ tempUnit WRITE setTempUnit )
  public:
    StrikeWaterDialog(QWidget* parent = 0);
    ~StrikeWaterDialog();

    Unit* volumeUnit();
    Unit* weightUnit();
    Unit* tempUnit();

  public slots:
    void calculate();
    void setImperial();
    void setSi();
    void setVolumeUnit(Unit* unit);
    void setWeightUnit(Unit* unit);
    void setTempUnit(Unit* unit);

  private:
    // From Northern Brewer ~0.38 but Jon Palmer suggest 0.41
    // to compensate for the lost to the tun even if the tun is pre-heaten
    static const double specificHeatBarley = 0.41;
  	Unit* volume;
  	Unit* weight;
  	Unit* temp;

    double initialInfusionSi(double grainTemp, double targetTemp,
        double waterToGrain);
    double mashInfusionSi(double initialTemp, double targetTemp,
        double grainWeight, double infusionWater, double mashVolume);

    double computeInitialInfusion();
    double computeMashInfusion();
};

#endif