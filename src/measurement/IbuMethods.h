/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/IbuMethods.h is part of Brewtarget, and is copyright the following authors 2009-2021:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef MEASUREMENT_IBUMETHODS_H
#define MEASUREMENT_IBUMETHODS_H
#pragma once

class QString;

/*!
 * \namespace IbuMethods
 *
 * \brief Make IBU calculations.
 */
namespace IbuMethods {
   //! \brief The formula used to get IBUs.
   enum IbuType {TINSETH, RAGER, NOONAN};

   extern IbuType ibuFormula;

   /**
    * \brief Read in from persistent settings
    */
   void loadIbuFormula();

   /**
    * \brief Write out to persistent settings
    */
   void saveIbuFormula();

   //! \brief return the bitterness formula's name
   QString ibuFormulaName();

   /*!
    * \return IBUs according to selected algorithm.
    * \param AArating in [0,1] (0.04 means 4% AA for example)
    * \param hops_grams - mass of hops in grams
    * \param finalVolume_liters - self explanatory
    * \param wort_grav in specific gravity at around 60F I guess.
    * \param minutes - minutes that the hops are in the boil
    */
   double getIbus(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes);
}

#endif
