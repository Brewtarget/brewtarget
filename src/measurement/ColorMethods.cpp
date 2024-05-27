/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/ColorMethods.cpp is part of Brewtarget, and is copyright the following authors 2009-2014:
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

ColorMethods::ColorType ColorMethods::colorFormula = ColorMethods::MOREY;

QString ColorMethods::colorFormulaName() {

   switch (ColorMethods::colorFormula) {
      case ColorMethods::MOREY:
         return "Morey";
      case ColorMethods::DANIEL:
         return "Daniels";
      case ColorMethods::MOSHER:
         return "Mosher";
      default:
         // It's a coding error if we did not cover all possible options above
         Q_ASSERT(false);
         break;
   }
   return "Error!";
}

void ColorMethods::loadColorFormulaSettings() {
   QString text = PersistentSettings::value(PersistentSettings::Names::color_formula, "morey").toString();
   if (text == "morey") {
      ColorMethods::colorFormula = MOREY;
   } else if (text == "daniel") {
      ColorMethods::colorFormula = DANIEL;
   } else if (text == "mosher") {
      ColorMethods::colorFormula = MOSHER;
   } else {
      qCritical() << QString("Bad color_formula type: %1").arg(text);
   }
   return;
}

void ColorMethods::saveColorFormulaSettings() {
   switch (ColorMethods::colorFormula) {
      case MOREY:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "morey");
         break;
      case DANIEL:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "daniel");
         break;
      case MOSHER:
         PersistentSettings::insert(PersistentSettings::Names::color_formula, "mosher");
         break;
      default:
         // It's a coding error if we did not cover all possible options above
         Q_ASSERT(false);
         break;
   }
   return;
}


double ColorMethods::mcuToSrm(double mcu) {
   switch (ColorMethods::colorFormula) {
      case ColorMethods::MOREY:
         return morey(mcu);
      case ColorMethods::DANIEL:
         return daniel(mcu);
      case ColorMethods::MOSHER:
         return mosher(mcu);
      default:
         qCritical() << QObject::tr("Invalid color formula type: %1").arg(ColorMethods::colorFormula);
         return morey(mcu);
   }
}
