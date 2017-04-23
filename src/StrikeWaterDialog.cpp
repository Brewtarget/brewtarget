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
#include <limits>
#include <Algorithms.h>
#include "StrikeWaterDialog.h"
#include "brewtarget.h"

// From Northern Brewer ~0.38 but Jon Palmer suggest 0.41
// to compensate for the lost to the tun even if the tun is pre-heaten
const double StrikeWaterDialog::specificHeatBarley = 0.41;

StrikeWaterDialog::StrikeWaterDialog(QWidget* parent) : QDialog(parent)
{
   setupUi(this);
   connect(pushButton_calculate, &QAbstractButton::clicked, this, &StrikeWaterDialog::calculate);
}

StrikeWaterDialog::~StrikeWaterDialog() {}

void StrikeWaterDialog::calculate()
{
  double initial = computeInitialInfusion();
  double mash = computeMashInfusion();

  initialResultTxt->setText(initial);
  mashResultTxt->setText(mash);
}

double StrikeWaterDialog::computeInitialInfusion()
{
  double grainTemp   = grainTempVal->toSI();
  double targetMash  = targetMashVal->toSI();
  double waterVolume = waterVolumeVal->toSI();
  double grainWeight = grainWeightInitVal->toSI();

  if ( grainWeight == 0.0 )
     return 0.0;

  return initialInfusionSi( grainTemp, targetMash, waterVolume / grainWeight);
}

double StrikeWaterDialog::computeMashInfusion()
{
  double mashVol       = mashVolVal->toSI();
  double grainWeight   = grainWeightVal->toSI();
  double actualMash    = actualMashVal->toSI();
  double targetMashInf = targetMashInfVal->toSI();
  double infusionWater = infusionWaterVal->toSI();

  return mashInfusionSi(actualMash, targetMashInf, grainWeight, infusionWater, mashVol);
}

double StrikeWaterDialog::initialInfusionSi(double grainTemp, double targetTemp, double waterToGrain)
{
   if ( waterToGrain == 0.0 )
      return 0.0;
   return (specificHeatBarley / waterToGrain) * (targetTemp - grainTemp) + targetTemp;
}
double StrikeWaterDialog::mashInfusionSi(double initialTemp, double targetTemp, double grainWeight, double infusionWater, double mashVolume)
{
   if ( infusionWater - targetTemp == 0.0 )
      return 0.0;

  return ((targetTemp - initialTemp) * (specificHeatBarley * grainWeight + mashVolume)) / (infusionWater - targetTemp);
}

