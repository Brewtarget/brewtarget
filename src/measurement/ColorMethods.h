/*
 * ColorMethods.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#ifndef MEASUREMENT_COLORMETHODS_H
#define MEASUREMENT_COLORMETHODS_H
#pragma once

class QString;

/*!
 * \namespace ColorMethods
 *
 * \brief Convert malt color units to SRM.
 */
namespace ColorMethods {
   //! \brief The formula used to get beer color.
   enum ColorType {MOSHER, DANIEL, MOREY};

   extern ColorType colorFormula;

   //! \brief return the color formula name
   QString colorFormulaName();

   void loadColorFormulaSettings();
   void saveColorFormulaSettings();

   //! Depending on selected algorithm, convert malt color units to SRM.
   double mcuToSrm(double mcu);
}

#endif
