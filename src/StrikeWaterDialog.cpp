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

// From Northern Brewer ~0.38 but Jon Palmer suggest 0.41
// to compensate for the lost to the tun even if the tun is pre-heaten
const double StrikeWaterDialog::specificHeatBarley = 0.41;

StrikeWaterDialog::StrikeWaterDialog(QWidget* parent) : QDialog(parent) {
	setupUi(this);

	temperatureSelect->addItem(Units::celsius->getUnitName(), Units::celsius->getUnitName());
	temperatureSelect->addItem(Units::fahrenheit->getUnitName(), Units::fahrenheit->getUnitName());

	volumeSelect->addItem(Units::liters->getUnitName(), Units::liters->getUnitName());
	volumeSelect->addItem(Units::us_quarts->getUnitName(), Units::us_quarts->getUnitName());

  weightSelect->addItem(Units::kilograms->getUnitName(), Units::kilograms->getUnitName());
  weightSelect->addItem(Units::pounds->getUnitName(), Units::pounds->getUnitName());

  initialResultTxt->setText(tr("N/A"));
  mashResultTxt->setText(tr("N/A"));
  grainWeightVal->setText("0");
  mashVolVal->setText("0");

  setSi();

	connect(pushButton_calculate, SIGNAL(clicked()), this, SLOT(calculate()));
	connect(setImperialBtn, SIGNAL(clicked()), this, SLOT(setImperial()));
	connect(setSiBtn, SIGNAL(clicked()), this, SLOT(setSi()));
  connect(temperatureSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(recheckUnits(int)));
  connect(volumeSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(recheckUnits(int)));
  connect(weightSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(recheckUnits(int)));
}

StrikeWaterDialog::~StrikeWaterDialog() {}

Unit* StrikeWaterDialog::volumeUnit() { return volume; }
Unit* StrikeWaterDialog::weightUnit() { return weight; }
Unit* StrikeWaterDialog::tempUnit() { return temp; }

void StrikeWaterDialog::calculate() {
	double initial = computeInitialInfusion();
	double mash = computeMashInfusion();
    
	QString initialVal = Algorithms::Instance().isnan(initial) 
    ? tr("N/A") 
    : Brewtarget::displayAmount(temp->fromSI(initial), NULL) + " " + temp->getUnitName();
	QString mashVal = Algorithms::Instance().isnan(mash) 
    ? tr("N/A") 
    : Brewtarget::displayAmount(volume->fromSI(mash), NULL) + " " + volume->getUnitName();

	initialResultTxt->setText(initialVal);
	mashResultTxt->setText(mashVal);
}

void StrikeWaterDialog::setImperial() {
	setVolumeUnit(Units::us_quarts);
	setWeightUnit(Units::pounds);
	setTempUnit(Units::fahrenheit);
  waterToGrainUnit->setText(
    QString("%1/%2").arg(volume->getUnitName(), weight->getUnitName())
  );
}

void StrikeWaterDialog::setSi() {
	setVolumeUnit(Units::liters);
	setWeightUnit(Units::kilograms);
	setTempUnit(Units::celsius);
  waterToGrainUnit->setText(
    QString("%1/%2").arg(volume->getUnitName(), weight->getUnitName())
  );
}

void StrikeWaterDialog::recheckUnits(int i) {
  setVolumeUnit(volumeSelect->currentText() == Units::liters->getUnitName() 
    ? (Unit*)Units::liters : (Unit*)Units::us_quarts);
  setWeightUnit(weightSelect->currentText() == Units::kilograms->getUnitName() 
    ? (Unit*)Units::kilograms : (Unit*)Units::pounds);
  setTempUnit(temperatureSelect->currentText() == Units::celsius->getUnitName() 
    ? (Unit*)Units::celsius : (Unit*)Units::fahrenheit);
  waterToGrainUnit->setText(
    QString("%1/%2").arg(volume->getUnitName(), weight->getUnitName())
  );
}

void StrikeWaterDialog::setVolumeUnit(Unit* unit) {
  volume = unit;

  mashResultUnit->setText(unit->getUnitName());
  mashVolUnit->setText(unit->getUnitName());
  
  int idx = volumeSelect->findData(unit->getUnitName());
  if (idx == -1) return;
  volumeSelect->setCurrentIndex(idx);
}

void StrikeWaterDialog::setWeightUnit(Unit* unit) {
  weight = unit;

  grainWeightUnit->setText(unit->getUnitName());
  
  int idx = weightSelect->findData(unit->getUnitName());
  if (idx == -1) return;
  weightSelect->setCurrentIndex(idx);
}

void StrikeWaterDialog::setTempUnit(Unit* unit) {
  temp = unit;
  QString tempUnitStr = QString(QString::fromUtf8("\u00B0%1")).arg(unit->getUnitName());
  actualMashUnit->setText(tempUnitStr);
  grainTempUnit->setText(tempUnitStr);
  infusionWaterUnit->setText(tempUnitStr);
  initialResultUnit->setText(tempUnitStr);
  targetMashInfUnit->setText(tempUnitStr);
  targetMashUnit->setText(tempUnitStr);
  
  int idx = temperatureSelect->findData(unit->getUnitName());
  if (idx == -1) return;
  temperatureSelect->setCurrentIndex(idx);
}

double StrikeWaterDialog::computeInitialInfusion() {
  bool ok;
  double grainTemp = grainTempVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double targetMash = targetMashVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double waterToGrain = waterToGrainVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();

  return initialInfusionSi(
    temp->toSI(grainTemp), 
    temp->toSI(targetMash), 
    waterToGrain * (volume->toSI(1.0) / weight->toSI(1.0))
  );
}

double StrikeWaterDialog::computeMashInfusion() {
  bool ok;
  double mashVol = mashVolVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double grainWeight = grainWeightVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double actualMash = actualMashVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double targetMashInf = targetMashInfVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();
  double infusionWater = infusionWaterVal->text().toDouble(&ok);
  if (!ok)  return std::numeric_limits<double>::quiet_NaN();  
  
  return mashInfusionSi(
    temp->toSI(actualMash), 
    temp->toSI(targetMashInf),
    weight->toSI(grainWeight),
    temp->toSI(infusionWater),
    volume->toSI(mashVol)
  );
}

double StrikeWaterDialog::initialInfusionSi(double grainTemp, double targetTemp,
        double waterToGrain) {
  return (specificHeatBarley / waterToGrain) * (targetTemp - grainTemp) + targetTemp;
}
double StrikeWaterDialog::mashInfusionSi(double initialTemp, double targetTemp,
        double grainWeight, double infusionWater, double mashVolume) {
  return ((targetTemp - initialTemp) * (specificHeatBarley * grainWeight + mashVolume)) 
      / (infusionWater - targetTemp);
}