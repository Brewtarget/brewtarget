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

#include <limits>
#include <Algorithms.h>
#include "StrikeWaterDialog.h"
#include "brewtarget.h"

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

Unit* StrikeWaterDialog::volumeUnit() { return volume; }
Unit* StrikeWaterDialog::weightUnit() { return weight; }
TempScale StrikeWaterDialog::tempUnit() { return temp; }

void StrikeWaterDialog::calculate() {
	double initial = computeInitialInfusion();
	double mash = computeMashInfusion();
    
	QString initialVal = Algorithms::Instance().isnan(initial) ? "N/A" : "12 gal";
	QString mashVal = Algorithms::Instance().isnan(mash) ? "N/A" : "12 gal";

	InitialResultTxt->setText(initialVal);
	InitialResultTxt->setText(mashVal);
	//MashResultTxt->setText(Brewtarget::displayAmount(99, Units::grams));
}

void StrikeWaterDialog::setImperial() {
	setVolumeUnit(Units::us_quarts);
	setWeightUnit(Units::pounds);
	setTempUnit(Fahrenheit);
}

void StrikeWaterDialog::setSi() {
	setVolumeUnit(Units::liters);
	setWeightUnit(Units::kilograms);
	setTempUnit(Celsius);
}

void StrikeWaterDialog::setVolumeUnit(Unit* unit) {
  volume = unit;
  int idx = VolumeSelect->findData(unit->getUnitName());
  if (idx == -1) return;
  VolumeSelect->setCurrentIndex(idx);
}

void StrikeWaterDialog::setWeightUnit(Unit* unit) {
  weight = unit;
  int idx = WeightSelect->findData(unit->getUnitName());
  if (idx == -1) return;
  WeightSelect->setCurrentIndex(idx);
}

void StrikeWaterDialog::setTempUnit(TempScale unit) {
  temp = unit;
  int idx = TemperatureSelect->findData(unit);
  if (idx == -1) return;
  TemperatureSelect->setCurrentIndex(idx);
}

double StrikeWaterDialog::computeInitialInfusion() {
  return std::numeric_limits<double>::quiet_NaN();
}

double StrikeWaterDialog::computeMashInfusion() {
  return std::numeric_limits<double>::quiet_NaN();
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