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

#include <QColor>
#include <QDebug>
#include <QString>
#include <QObject>

#include "PersistentSettings.h"

namespace {
   //
   // https://babblehomebrewers.wordpress.com/wp-content/uploads/2015/06/estimating-beer-color.pdf and
   // https://byo.com/images/28-33_Beer_Color.pdf both give:
   //    SRM = 1.49 × (MCU ^ 0.69)
   //
   // However, https://beersmith.com/forum/index.php?threads/how-beersmith-calculates-recipe-srm.6244/ and
   // http://brewwiki.com/index.php/Estimating_Color give:
   //    SRM = 1.4922 × (MCU ^ 0.6859)
   //
   double morey(double const mcu) {
      return 1.4922 * pow( mcu, 0.6859 );
   }

   // From Palmer's "How to Brew"
   double daniel(double const mcu) {
      return 0.2 * mcu + 8.4;
   }

   // From Palmer's "How to Brew"
   double mosher(double const mcu) {
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
   PersistentSettings::readEnum(PersistentSettings::Names::color_formula,
                                ColorMethods::formulaStringMapping,
                                ColorMethods::formula,
                                ColorMethods::ColorFormula::Morey);
   return;
}

void ColorMethods::saveFormula() {
   PersistentSettings::insert(PersistentSettings::Names::color_formula,
                              ColorMethods::formulaStringMapping[ColorMethods::formula]);
   return;
}

QString ColorMethods::formulaName() {
   return ColorMethods::formulaDisplayNames[ColorMethods::formula];
}

double ColorMethods::mcuToSrm(double mcu) {
   switch (ColorMethods::formula) {
      case ColorMethods::ColorFormula::Morey : return morey (mcu);
      case ColorMethods::ColorFormula::Daniel: return daniel(mcu);
      case ColorMethods::ColorFormula::Mosher: return mosher(mcu);
   }
//      std::unreachable();
}

QColor ColorMethods::srmToDisplayColor(double srm) {
   QColor ret;

   //==========My approximation from a photo and spreadsheet===========
   //double red = 232.9 * pow( (double)0.93, srm );
   //double green = (double)-106.25 * log(srm) + 280.9;
   //
   //int r = (int)Algorithms::round(red);
   //int g = (int)Algorithms::round(green);
   //int b = 0;

   // Philip Lee's approximation from a color swatch and curve fitting.
   int r = 0.5 + (272.098 - 5.80255*srm); if( r > 253.0 ) r = 253.0;
   int g = (srm > 35)? 0 : 0.5 + (2.41975e2 - 1.3314e1*srm + 1.881895e-1*srm*srm);
   int b = 0.5 + (179.3 - 28.7*srm);

   r = (r < 0) ? 0 : ((r > 255)? 255 : r);
   g = (g < 0) ? 0 : ((g > 255)? 255 : g);
   b = (b < 0) ? 0 : ((b > 255)? 255 : b);
   ret.setRgb( r, g, b );

   return ret;
}
