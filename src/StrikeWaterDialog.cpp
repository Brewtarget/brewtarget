/*
* StrikeWaterDialog.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "StrikeWaterDialog.h"
#include "brewtarget.h"

StrikeWaterDialog::StrikeWaterDialog(QWidget* parent) : QDialog(parent) {
	setupUi(this);
	connect(pushButton_calculate, SIGNAL(clicked()), this, SLOT(calculate()));
}

StrikeWaterDialog::~StrikeWaterDialog() {}

void StrikeWaterDialog::calculate() {
	InitialResultTxt->setText(Brewtarget::displayAmount(100, Units::grams));
	MashResultTxt->setText(Brewtarget::displayAmount(99, Units::grams));
}

double StrikeWaterDialog::initialInfusionSi(double grainTemp, double targetTemp,
        double waterToGrain) {
  // Jon Palmer's equation
  return (.41 / waterToGrain) * (targetTemp - grainTemp) + targetTemp;
}
double StrikeWaterDialog::mashInfusionSi(double initialTemp, double targetTemp,
        double grainWeight, double infusionWater, double mashVolume) {
  // Jon Palmer's equation
  return ((targetTemp - initialTemp) * (0.41 * grainWeight + mashVolume)) 
      / (infusionWater - targetTemp);
}