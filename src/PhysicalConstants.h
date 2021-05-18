/*
 * PhysicalConstants.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef _PHYSICALCONSTANTS_H
#define _PHYSICALCONSTANTS_H

/*!
 * \brief Collection of physical constants like density of materials.
 */
namespace PhysicalConstants{
   //! \brief Sucrose density in kg per L.
   const double sucroseDensity_kgL = 1.587;
   //! \brief This estimate for grain density is from my own (Philip G. Lee) experiments.
   const double grainDensity_kgL = 0.963;
   //! \brief Liquid extract density in kg per L.
   const double liquidExtractDensity_kgL = 1.412;
   //! \brief Dry extract density in kg per L.
   const double dryExtractDensity_kgL = sucroseDensity_kgL;
   
   //! \brief How many liters of water get absorbed by 1 kg of grain.
   const double grainAbsorption_Lkg = 1.085;

   const double absoluteZero = -273.15;
}

#endif
