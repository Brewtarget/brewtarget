/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/ColorMethods.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
#include "measurement/ColorMethods.h"

#include <cmath>

#include <QDebug>
#include <QString>
#include <QObject>

#include "PersistentSettings.h"

namespace {
   // I don't know where this is from.
   double morey(double mcu) {
      return 1.4922 * pow( mcu, 0.6859 );
   }

   // From Palmer's "How to Brew"
   double daniel(double mcu) {
      return 0.2 * mcu + 8.4;
   }

   // From Palmer's "How to Brew"
   double mosher(double mcu) {
      return 0.3 * mcu + 4.7;
   }
}

EnumStringMapping const ColorMethods::formulaStringMapping {
   {ColorMethods::ColorFormula::Mosher, "mosher"},
   {ColorMethods::ColorFormula::Daniel, "daniel"},
   {ColorMethods::ColorFormula::Morey , "morey" },
};

EnumStringMapping const ColorMethods::formulaDisplayNames {
   {ColorMethods::ColorFormula::Mosher, QObject::tr("Mosher's approximation")},
   {ColorMethods::ColorFormula::Daniel, QObject::tr("Daniel's approximation")},
   {ColorMethods::ColorFormula::Morey , QObject::tr("Morey's approximation" )},
};

TypeLookup const ColorMethods::typeLookup {
   "ColorMethods",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::ColorMethods::formula, ColorMethods::formula, NonPhysicalQuantity::Enum),
   }
};

ColorMethods::ColorFormula ColorMethods::formula = ColorMethods::ColorFormula::Morey;

void ColorMethods::loadFormula() {
   ColorMethods::formula = ColorMethods::formulaStringMapping.stringToEnum<ColorMethods::ColorFormula>(
      PersistentSettings::value(PersistentSettings::Names::color_formula,
                                ColorMethods::formulaStringMapping[ColorMethods::ColorFormula::Morey]).toString()
   );
   return;
}

void ColorMethods::saveFormula() {
   PersistentSettings::insert(PersistentSettings::Names::ibu_formula,
                              ColorMethods::formulaStringMapping[ColorMethods::formula]);
   return;
}

QString ColorMethods::formulaName() {
   return ColorMethods::formulaDisplayNames[ColorMethods::formula];
}

double ColorMethods::mcuToSrm(double mcu) {
   switch (ColorMethods::formula) {
      case ColorMethods::ColorFormula::Morey:
         return morey(mcu);
      case ColorMethods::ColorFormula::Daniel:
         return daniel(mcu);
      case ColorMethods::ColorFormula::Mosher:
         return mosher(mcu);
   }
//      std::unreachable();
}
