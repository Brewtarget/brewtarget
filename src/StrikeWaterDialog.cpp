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
#include "unit.h"

StrikeWaterDialog::StrikeWaterDialog(QWidget* parent) : QDialog(parent) {
	setupUi(this);

	TemperatureSelect->addItem(tempScaleToString(Celsius), Celsius);
	TemperatureSelect->addItem(tempScaleToString(Fahrenheit), Fahrenheit);

	VolumeSelect->addItem(Units::liters->getUnitName(), Units::liters->getUnitName());
	VolumeSelect->addItem(Units::us_quarts->getUnitName(), Units::us_quarts->getUnitName());

    WeightSelect->addItem(Units::kilograms->getUnitName(), Units::kilograms->getUnitName());
    WeightSelect->addItem(Units::pounds->getUnitName(), Units::pounds->getUnitName());

    setSi();

	connect(pushButton_calculate, SIGNAL(clicked()), this, SLOT(calculate()));
	connect(SetImperialBtn, SIGNAL(clicked()), this, SLOT(setImperial()));
	connect(SetSiBtn, SIGNAL(clicked()), this, SLOT(setSi()));
}

StrikeWaterDialog::~StrikeWaterDialog() {}

void StrikeWaterDialog::calculate() {
	InitialResultTxt->setText(Brewtarget::displayAmount(100, Units::grams));
	MashResultTxt->setText(Brewtarget::displayAmount(99, Units::grams));
}

void StrikeWaterDialog::setImperial() {
	int idx = TemperatureSelect->findData(Fahrenheit);
	if (idx != -1)  TemperatureSelect->setCurrentIndex(idx);
	idx = VolumeSelect->findData(Units::us_quarts->getUnitName());
	if (idx != -1)  VolumeSelect->setCurrentIndex(idx);
	idx = WeightSelect->findData(Units::pounds->getUnitName());
	if (idx != -1)  WeightSelect->setCurrentIndex(idx);
}

void StrikeWaterDialog::setSi() {
	int idx = TemperatureSelect->findData(Celsius);
	if (idx != -1)  TemperatureSelect->setCurrentIndex(idx);
	idx = VolumeSelect->findData(Units::liters->getUnitName());
	if (idx != -1)  VolumeSelect->setCurrentIndex(idx);
	idx = WeightSelect->findData(Units::kilograms->getUnitName());
	if (idx != -1)  WeightSelect->setCurrentIndex(idx);
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