/*
 * OgAdjuster.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
 * - Eric Tamme <etamme@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "OgAdjuster.h"

#include "Algorithms.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Equipment.h"
#include "model/Recipe.h"

OgAdjuster::OgAdjuster( QWidget* parent ) :
   QDialog{parent},
   recObs {nullptr} {
   setupUi(this);

   SMART_FIELD_INIT_FIXED(OgAdjuster, label_sg       , lineEdit_sg       , double, Measurement::Units::specificGravity       , 3); // Input: SG
   SMART_FIELD_INIT_FS   (OgAdjuster, label_temp     , lineEdit_temp     , double, Measurement::PhysicalQuantity::Temperature, 1); // Input: Temp
   SMART_FIELD_INIT_FS   (OgAdjuster, label_calTemp  , lineEdit_calTemp  , double, Measurement::PhysicalQuantity::Temperature, 1); // Input: Calibration Temp
   SMART_FIELD_INIT_FIXED(OgAdjuster, label_plato    , lineEdit_plato    , double, Measurement::Units::plato                 , 1); // Input: Plato
   SMART_FIELD_INIT_FS   (OgAdjuster, label_volume   , lineEdit_volume   , double, Measurement::PhysicalQuantity::Volume        ); // Input: Pre-Boil Volume
   SMART_FIELD_INIT_FIXED(OgAdjuster, label_og       , lineEdit_og       , double, Measurement::Units::specificGravity       , 3); // Output: OG w/o Correction
   SMART_FIELD_INIT_FS   (OgAdjuster, label_add      , lineEdit_add      , double, Measurement::PhysicalQuantity::Volume        ); // Output: Add to Boil
   SMART_FIELD_INIT_FS   (OgAdjuster, label_batchSize, lineEdit_batchSize, double, Measurement::PhysicalQuantity::Volume        ); // Output: Final Batch Size

   connect(this->pushButton_calculate, &QAbstractButton::clicked, this, &OgAdjuster::calculate );
   return;
}


void OgAdjuster::setRecipe(Recipe* rec) {
   if (rec && rec != recObs) {
      recObs = rec;
   }
   return;
}

// TODO: There are a LOT of assumptions and simplifications here. Need to change that.
void OgAdjuster::calculate() {

   // Get inputs.
   double sg          = lineEdit_sg->toCanonical().quantity();
   bool   okPlato     = true;
   double plato       = Measurement::extractRawFromString<double>(lineEdit_plato->text(), &okPlato);
   double temp_c      = lineEdit_temp->toCanonical().quantity();
   double hydroTemp_c = lineEdit_calTemp->toCanonical().quantity();
   double wort_l      = lineEdit_volume->toCanonical().quantity();

   // Make sure we got enough info.
   bool gotSG = sg != 0 && temp_c != 0 && hydroTemp_c != 0;

   if (wort_l == 0) {
      return;
   }

   if (!gotSG && !okPlato) {
      return;
   }

   if (recObs == 0 || recObs->equipment() == 0) {
      return;
   }

   Equipment* equip = recObs->equipment();
   double evapRate_lHr = equip->evapRate_lHr();

   // Calculate missing input parameters.
   double sg_20C = 0.0;
   if (gotSG) {
      double sg_15C = sg * Algorithms::getWaterDensity_kgL(hydroTemp_c)/Algorithms::getWaterDensity_kgL(15) + Algorithms::hydrometer15CCorrection( temp_c );
      sg_20C = sg_15C * Algorithms::getWaterDensity_kgL(15)/Algorithms::getWaterDensity_kgL(20);

      plato = Algorithms::SG_20C20C_toPlato(sg_20C);
      lineEdit_plato->setAmount(sg_20C); // Event if the display is in Plato, we must send it in default unit
   } else {
      sg_20C = Algorithms::PlatoToSG_20C20C(plato);
   }

   // Calculate intermediate parameters.
   double sugar_kg = sg_20C * Algorithms::getWaterDensity_kgL(20) * wort_l * plato/(double)100;
   //std::cerr << "sugar_kg = " << sugar_kg << std::endl;
   double water_kg = sg_20C * Algorithms::getWaterDensity_kgL(20) * wort_l * ((double)1 - plato/(double)100);
   //std::cerr << "water_kg = " << water_kg << std::endl;

   // Calculate OG w/o correction.
   double finalVolume_l = equip->wortEndOfBoil_l(wort_l);
   double finalWater_kg = water_kg - equip->boilTime_min()/(double)60 * evapRate_lHr * Algorithms::getWaterDensity_kgL(20);
   //std::cerr << "finalWater_kg = " << finalWater_kg << std::endl;
   //std::cerr << "boilTime = " << equip->getBoilTime_min() << std::endl;
   //std::cerr << "evapRate_lHr = " << evapRate_lHr << std::endl;
   //std::cerr << "waterDensity = " << Algorithms::getWaterDensity_kgL(20) << std::endl;
   double finalPlato = (double)100 * sugar_kg / (sugar_kg + finalWater_kg);
   //std::cerr << "finalPlato = " << finalPlato << std::endl;
   double finalUncorrectedSg_20C = Algorithms::PlatoToSG_20C20C( finalPlato );

   // Calculate volume to add to boil
   finalPlato = Algorithms::SG_20C20C_toPlato( recObs->og() ); // This is bad. This assumes the post-boil gravity = og. Need account for post-boil water additions.
   // postBoilWater_kg = batchSize - topUpWater;
   // postBoilSugar_kg = Algorithms::SG_20C20C_toPlato( recObs->getOG() ) / 100.0 * batchSize * recObs->getOG() * Algorithms::getWaterDensity_kgL(20);
   // finalPlato = 100 * postBoilSugar_kg / ( postBoilSugar_kg + postBoilWater_kg );
   double waterToAdd_kg = (double)100 * sugar_kg / finalPlato - sugar_kg - finalWater_kg;
   double waterToAdd_l = waterToAdd_kg / Algorithms::getWaterDensity_kgL(20);

   // Calculate final batch size.
   finalVolume_l += waterToAdd_l;

   // Display output.
   this->lineEdit_og       ->setAmount(finalUncorrectedSg_20C);
   this->lineEdit_add      ->setAmount(waterToAdd_l);
   this->lineEdit_batchSize->setAmount(finalVolume_l);
   return;
}
