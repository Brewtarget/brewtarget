/*
 * OgAdjuster.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Eric Tamme <etamme@gmail.com>
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
#include "equipment.h"
#include "brewtarget.h"
#include "unit.h"
#include "Algorithms.h"
#include "recipe.h"

OgAdjuster::OgAdjuster( QWidget* parent ) : QDialog(parent)
{
   setupUi(this);

   recObs = 0;

   connect( pushButton_calculate, &QAbstractButton::clicked, this, &OgAdjuster::calculate );
}


void OgAdjuster::setRecipe( Recipe* rec )
{
   if( rec && rec != recObs )
   {
      recObs = rec;
   }
}

// TODO: There are a LOT of assumptions and simplifications here. Need to change that.
void OgAdjuster::calculate()
{
   Equipment* equip;
   double sg = 0.0;
   double temp_c = 0.0;
   double plato = 0.0;
   double wort_l = 0.0;
   double hydroTemp_c = 0.0;

   double sugar_kg = 0.0;
   double water_kg = 0.0;
   double sg_15C = 0.0;
   double sg_20C = 0.0;
   double evapRate_lHr = 0.0;

   double finalPlato = 0.0;
   double finalVolume_l = 0.0;
   double finalWater_kg = 0.0;
   double finalUncorrectedSg_20C = 0.0;
   double waterToAdd_kg = 0.0;
   double waterToAdd_l = 0.0;

   bool gotSG = false;
   bool okPlato = true;

   // Get inputs.
   sg          = lineEdit_sg->toSI();
   plato       = lineEdit_plato->toDouble(&okPlato);
   temp_c      = lineEdit_temp->toSI();
   hydroTemp_c = lineEdit_calTemp->toSI();
   wort_l      = lineEdit_volume->toSI();

   // Make sure we got enough info.
   gotSG = sg != 0 && temp_c != 0 && hydroTemp_c != 0;

   if( wort_l == 0 )
      return;
   if( ! gotSG && ! okPlato )
      return;

   if( recObs == 0 || recObs->equipment() == 0 )
      return;

   equip = recObs->equipment();
   evapRate_lHr = equip->evapRate_lHr();

   // Calculate missing input parameters.
   if( gotSG )
   {
      sg_15C = sg * Algorithms::getWaterDensity_kgL(hydroTemp_c)/Algorithms::getWaterDensity_kgL(15) + Algorithms::hydrometer15CCorrection( temp_c );
      sg_20C = sg_15C * Algorithms::getWaterDensity_kgL(15)/Algorithms::getWaterDensity_kgL(20);

      plato = Algorithms::SG_20C20C_toPlato( sg_20C );
      lineEdit_plato->setText( sg_20C ); //Event if the display is in Plato, we must send it in default unit
   }
   else
   {
      sg_20C = Algorithms::PlatoToSG_20C20C( plato );
   }

   // Calculate intermediate parameters.
   sugar_kg = sg_20C * Algorithms::getWaterDensity_kgL(20) * wort_l * plato/(double)100;
   //std::cerr << "sugar_kg = " << sugar_kg << std::endl;
   water_kg = sg_20C * Algorithms::getWaterDensity_kgL(20) * wort_l * ((double)1 - plato/(double)100);
   //std::cerr << "water_kg = " << water_kg << std::endl;

   // Calculate OG w/o correction.
   finalVolume_l = equip->wortEndOfBoil_l(wort_l);
   finalWater_kg = water_kg - equip->boilTime_min()/(double)60 * evapRate_lHr * Algorithms::getWaterDensity_kgL(20);
   //std::cerr << "finalWater_kg = " << finalWater_kg << std::endl;
   //std::cerr << "boilTime = " << equip->getBoilTime_min() << std::endl;
   //std::cerr << "evapRate_lHr = " << evapRate_lHr << std::endl;
   //std::cerr << "waterDensity = " << Algorithms::getWaterDensity_kgL(20) << std::endl;
   finalPlato = (double)100 * sugar_kg / (sugar_kg + finalWater_kg);
   //std::cerr << "finalPlato = " << finalPlato << std::endl;
   finalUncorrectedSg_20C = Algorithms::PlatoToSG_20C20C( finalPlato );

   // Calculate volume to add to boil
   finalPlato = Algorithms::SG_20C20C_toPlato( recObs->og() ); // This is bad. This assumes the post-boil gravity = og. Need account for post-boil water additions.
   // postBoilWater_kg = batchSize - topUpWater;
   // postBoilSugar_kg = Algorithms::SG_20C20C_toPlato( recObs->getOG() ) / 100.0 * batchSize * recObs->getOG() * Algorithms::getWaterDensity_kgL(20);
   // finalPlato = 100 * postBoilSugar_kg / ( postBoilSugar_kg + postBoilWater_kg );
   waterToAdd_kg = (double)100 * sugar_kg / finalPlato - sugar_kg - finalWater_kg;
   waterToAdd_l = waterToAdd_kg / Algorithms::getWaterDensity_kgL(20);

   // Calculate final batch size.
   finalVolume_l += waterToAdd_l;

   // Display output.
   lineEdit_og->setText(finalUncorrectedSg_20C);
   lineEdit_add->setText(waterToAdd_l);
   lineEdit_batchSize->setText(finalVolume_l);
}
