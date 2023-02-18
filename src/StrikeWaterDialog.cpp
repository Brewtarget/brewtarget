/*
 * StrikeWaterDialog.cpp is part of Brewtarget, and is Copyright the following
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
#include "StrikeWaterDialog.h"

#include <limits>

#include <Algorithms.h>

namespace {
   // From Northern Brewer ~0.38 but Jon Palmer suggest 0.41
   // to compensate for the lost to the tun even if the tun is pre-heated
   double const specificHeatBarley = 0.41;

   /**
    * \brief
    */
   double initialInfusionSi(double grainTemp, double targetTemp, double waterToGrain) {
      if (waterToGrain == 0.0) {
         return 0.0;
      }
      return (specificHeatBarley / waterToGrain) * (targetTemp - grainTemp) + targetTemp;
   }

   /**
    * \brief
    */
   double mashInfusionSi(double initialTemp,
                         double targetTemp,
                         double grainWeight,
                         double infusionWater,
                         double mashVolume) {
      if (infusionWater - targetTemp == 0.0) {
         return 0.0;
      }

      return ((targetTemp - initialTemp) * (specificHeatBarley * grainWeight + mashVolume)) / (infusionWater - targetTemp);
   }

}

StrikeWaterDialog::StrikeWaterDialog(QWidget* parent) : QDialog(parent) {
   setupUi(this);
   connect(pushButton_calculate, &QAbstractButton::clicked, this, &StrikeWaterDialog::calculate);
   return;
}

StrikeWaterDialog::~StrikeWaterDialog() = default;

void StrikeWaterDialog::calculate() {
  double initial = computeInitialInfusion();
  double mash = computeMashInfusion();

  this->initialResultTxt->setText(initial);
  this->mashResultTxt->setText(mash);
  return;
}

double StrikeWaterDialog::computeInitialInfusion() {
   double grainTemp   = this->grainTempVal->toCanonical().quantity();
   double targetMash  = this->targetMashVal->toCanonical().quantity();
   double waterVolume = this->waterVolumeVal->toCanonical().quantity();
   double grainWeight = this->grainWeightInitVal->toCanonical().quantity();

   if (grainWeight == 0.0) {
      return 0.0;
   }

   return initialInfusionSi(grainTemp, targetMash, waterVolume / grainWeight);
}

double StrikeWaterDialog::computeMashInfusion() {
   double mashVol       = this->mashVolVal->toCanonical().quantity();
   double grainWeight   = this->grainWeightVal->toCanonical().quantity();
   double actualMash    = this->actualMashVal->toCanonical().quantity();
   double targetMashInf = this->targetMashInfVal->toCanonical().quantity();
   double infusionWater = this->infusionWaterVal->toCanonical().quantity();

   return mashInfusionSi(actualMash, targetMashInf, grainWeight, infusionWater, mashVol);
}
