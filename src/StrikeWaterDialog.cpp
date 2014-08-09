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
   connect(pushButton_calculate, SIGNAL(clicked()), this, SLOT(calculate()));
}

StrikeWaterDialog::~StrikeWaterDialog() {}

void StrikeWaterDialog::calculate() {
  double initial = computeInitialInfusion();
  double mash = computeMashInfusion();
  initialResultTxt->setText(Algorithms::isNan(initial) 
    ? tr("N/A") : Brewtarget::displayAmount(initial, Units::celsius));
  mashResultTxt->setText(Algorithms::isNan(mash) 
    ? tr("N/A") : Brewtarget::displayAmount(mash, Units::liters));
}

double StrikeWaterDialog::computeInitialInfusion() {
  double grainTemp = Brewtarget::tempQStringToSI(grainTempVal->text());
  double targetMash = Brewtarget::tempQStringToSI(targetMashVal->text());
  double waterVolume = Brewtarget::volQStringToSI(waterVolumeVal->text());
  double grainWeight = Brewtarget::weightQStringToSI(grainWeightInitVal->text());

  return initialInfusionSi(
    grainTemp, 
    targetMash, 
    waterVolume / grainWeight
  );
}

double StrikeWaterDialog::computeMashInfusion() {
  double mashVol = Brewtarget::volQStringToSI(mashVolVal->text());
  double grainWeight = Brewtarget::weightQStringToSI(grainWeightVal->text());
  double actualMash = Brewtarget::tempQStringToSI(actualMashVal->text());
  double targetMashInf = Brewtarget::tempQStringToSI(targetMashInfVal->text());
  double infusionWater = Brewtarget::tempQStringToSI(infusionWaterVal->text());
  return mashInfusionSi(actualMash, targetMashInf, grainWeight, infusionWater, mashVol);
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

