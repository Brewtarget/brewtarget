/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/ColorMethods.h is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#ifndef MEASUREMENT_COLORMETHODS_H
#define MEASUREMENT_COLORMETHODS_H
#pragma once

#include "utils/BtStringConst.h"
#include "utils/EnumStringMapping.h"

class QString;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in measurement/IbuMethods.h
//
#define AddPropertyName(property) namespace PropertyNames::ColorMethods { inline BtStringConst const property{#property}; }
AddPropertyName(formula)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \namespace ColorMethods
 *
 * \brief Convert malt color units to SRM.
 */
namespace ColorMethods {
   //! \brief The formula used to get beer color.
   enum class ColorFormula {
      Mosher,
      Daniel,
      Morey ,
   };

   /*!
    * \brief Mapping between \c ColorMethods::ColorType and string values suitable for serialisation in
    *        \c PersistentSettings, etc.
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   extern EnumStringMapping const formulaStringMapping;

   /*!
    * \brief Localised names of \c ColorMethods::ColorType values suitable for displaying to the end user
    */
   extern EnumStringMapping const formulaDisplayNames;

   extern ColorFormula formula;

   extern TypeLookup const typeLookup;

   //! \brief return the color formula name
   QString formulaName();

   void loadFormula();
   void saveFormula();

   //! Depending on selected algorithm, convert malt color units to SRM.
   double mcuToSrm(double mcu);
}

#endif
