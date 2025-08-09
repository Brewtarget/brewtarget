/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/PhysicalConstants.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MEASUREMENT_PHYSICALCONSTANTS_H
#define MEASUREMENT_PHYSICALCONSTANTS_H
#pragma once

/*!
 * \brief Collection of physical constants like density of materials.
 */
namespace PhysicalConstants {
   //! \brief Sucrose density in kg per L.
   double constexpr sucroseDensity_kgL = 1.587;
   //! \brief This estimate for grain density is from my own (Philip G. Lee) experiments.
   double constexpr grainDensity_kgL = 0.963;
   //! \brief Liquid extract density in kg per L.
   double constexpr liquidExtractDensity_kgL = 1.412;
   //! \brief Dry extract density in kg per L.
   double constexpr dryExtractDensity_kgL = sucroseDensity_kgL;

   //! \brief How many liters of water get absorbed by 1 kg of grain.
   double constexpr grainAbsorption_Lkg = 1.085;

   double constexpr absoluteZero = -273.15;

   /***Specific heats***/

   /**
    * \brief Specific heat capacity of water = 4184 J⋅kg⁻¹⋅K⁻¹ per https://en.wikipedia.org/wiki/Specific_heat_capacity
    *                                        = 1 c/g·C
    */
   double constexpr waterSpecificHeat_calGC = 1.0;

   /**
    * \brief Specific heat to use for grain.
    *
    *        Of course there is not one exact figure to use, but this is a reasonable approximation.
    *
    *        See International Agrophysics 1994, 8, 271-275: "Thermal characteristics of barley and oat" by
    *        Bogusława Łapczyńska-Kordon, A Zaremba, K. Kempkiewicz (available at
    *        http://www.international-agrophysics.org/Thermal-characteristics-of-barley-and-oat,139711,0,2.html) for
    *        more detailed analysis on how the specific heats of oats and barley vary by moisture content.
    */
   double constexpr grainSpecificHeat_calGC = 0.4;

}

#endif
