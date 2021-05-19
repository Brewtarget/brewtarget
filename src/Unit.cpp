/*
 * Unit.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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
#include "Unit.h"

#include <string>
#include <iostream>

#include <QStringList>
#include <QRegExp>
#include <QDebug>

#include "brewtarget.h"
#include "Algorithms.h"

namespace {

   QString unitFromString(QString qstr) {
      QRegExp amtUnit;

      // Make sure we get the right decimal point (. or ,) and the right grouping
      // separator (, or .). Some locales write 1.000,10 and other write
      // 1,000.10. We need to catch both
      QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
      QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

      amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
      amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

      // if the regex dies, return 0.0
      if (amtUnit.indexIn(qstr) == -1) {
         return QString("?");
      }

      // Get the unit from the second capture
      return amtUnit.cap(2);
   }

   double valueFromString(QString qstr) {
      QRegExp amtUnit;

      // Make sure we get the right decimal point (. or ,) and the right grouping
      // separator (, or .). Some locales write 1.000,10 and other write
      // 1,000.10. We need to catch both
      QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
      QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

      amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
      amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

      // if the regex dies, return 0.0
      if (amtUnit.indexIn(qstr) == -1) {
         return 0.0;
      }

      return Brewtarget::toDouble(amtUnit.cap(1), "Unit::valueFromString()");
   }

}

Unit::Unit(UnitType const unitType,
           SystemOfMeasurement const systemOfMeasurement,
           QString const unitName,
           QString const siUnitName,
           std::function<double(double)> convertToCanonical,
           std::function<double(double)> convertFromCanonical,
           double boundaryValue) : unitType{unitType},
                                   systemOfMeasurement{systemOfMeasurement},
                                   unitName{unitName},
                                   siUnitName{siUnitName},
                                   convertToCanonical{convertToCanonical},
                                   convertFromCanonical{convertFromCanonical},
                                   boundaryValue{boundaryValue} {
   return;
}

Unit::~Unit() = default;

double Unit::toSI(double amt) const {
   return this->convertToCanonical(amt);
}

double Unit::fromSI(double amt) const {
   return this->convertFromCanonical(amt);
}

QString const & Unit::getUnitName() const {
   return this->unitName;
}

QString const & Unit::getSIUnitName() const {
   return this->siUnitName;
}

Unit::UnitType Unit::getUnitType() const {
   return this->unitType;
}

SystemOfMeasurement Unit::getUnitOrTempSystem() const {
   return this->systemOfMeasurement;
}

double Unit::boundary() const {
   return this->boundaryValue;
}


// Return a
QString Unit::convert(QString qstr, QString toUnit) {
   double si;

   QString fName = unitFromString(qstr);
   double amt = valueFromString(qstr);
   Unit const * f = getUnit(fName);

   if ( f ) {
      si = f->toSI(amt);
   } else {
      si = 0.0;
   }

   Unit const * u = getUnit(toUnit, false);

   // If we couldn't find either unit, or the two units don't match (eg, you
   // cannot convert L to lb)
   if( u == 0 || f == 0 || u->getUnitType() != f->getUnitType() ) {
      return QString("%1 ?").arg(Brewtarget::displayAmount(si));
   }

   return QString("%1 %2").arg(Brewtarget::displayAmount(u->fromSI(si))).arg(toUnit);
}

// This mostly gets called when the unit entered in the field does not match
// what the field has been set to. For example, if you displaying in Liters,
// but enter "20 qt". Since the SIVolumeUnitSystem doesn't know what "qt" is,
// we go searching for it.
Unit const * Unit::getUnit(QString& name, bool matchCurrentSystem) {
   Unit const * defUnit = nullptr;

   // Under most circumstances, there is a one-to-one relationship between
   // unit string and Unit. C will only map to Unit::Celsius, for example. If
   // there's only one match, just return it.
   if ( nameToUnit.count(name) == 1 ) {
      return nameToUnit.value(name);
   }

   // That solved something like 99% of the use cases. Now we have to handle those pesky volumes.
   // Loop through the found Units, like Unit::us_quart and
   // Unit::imperial_quart, and try to find one that matches the global default.
   for(auto i = nameToUnit.find(name); i != nameToUnit.end() && i.key() == name; ++i ) {
      Unit const * u = i.value();
      if( u == 0 ) {
         continue;
      }

      int system = u->getUnitOrTempSystem();

      // Save this for later if we need it
      if( system == USCustomary ) {
         defUnit = u;
      }

      if( Brewtarget::thingToUnitSystem.value(Volume|system) == Brewtarget::thingToUnitSystem.value(Volume) ) {
         return u;
      }
   }

   // If we got here, we couldn't find a match. Unless something weird has
   // happened, that means you entered "qt" into a field and the system
   // default is SI. At that point, just use the USCustomary
   return defUnit;
}


// This is where we actually define all the different units and how to convert them to/from their canonical equivalents
// Previously this was done with a huge number of subclasses, but lambdas mean that's no longer necessary
// === Mass ===
Unit const Units::kilograms            = Unit{Unit::Mass,           SI,          "kg",   "kg",  [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::grams                = Unit{Unit::Mass,           SI,          "g",    "kg",  [](double x){return x/1000.0;},        [](double y){return y*1000.0;},         1.0};
Unit const Units::milligrams           = Unit{Unit::Mass,           SI,          "mg",   "kg",  [](double x){return x/1000000.0;},     [](double y){return y*1000000.0;},      1.0};
Unit const Units::pounds               = Unit{Unit::Mass,           USCustomary, "lb",   "kg",  [](double x){return x*0.45359237;},    [](double y){return y/0.45359237;},     1.0};
Unit const Units::ounces               = Unit{Unit::Mass,           USCustomary, "oz",   "kg",  [](double x){return x*0.0283495231;},  [](double y){return y/0.0283495231;},   1.0};
// === Volume ===
Unit const Units::liters               = Unit{Unit::Volume,         SI,          "L",    "L",   [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::milliliters          = Unit{Unit::Volume,         SI,          "mL",   "L",   [](double x){return x/1000.0;},        [](double y){return y*1000.0;},         1.0};
Unit const Units::us_barrels           = Unit{Unit::Volume,         USCustomary, "bbl",  "L",   [](double x){return x*117.34777;},     [](double y){return y/117.34777;},      1.0};
Unit const Units::us_gallons           = Unit{Unit::Volume,         USCustomary, "gal",  "L",   [](double x){return x*3.78541178;},    [](double y){return y/3.78541178;},     1.0};
Unit const Units::us_quarts            = Unit{Unit::Volume,         USCustomary, "qt",   "L",   [](double x){return x*0.946352946;},   [](double y){return y/0.946352946;},    1.0};
Unit const Units::us_cups              = Unit{Unit::Volume,         USCustomary, "cup",  "L",   [](double x){return x*0.236588236;},   [](double y){return y/0.236588236;},    0.25};
Unit const Units::us_tablespoons       = Unit{Unit::Volume,         USCustomary, "tbsp", "L",   [](double x){return x*0.0147867648;},  [](double y){return y/0.0147867648;},   1.0};
Unit const Units::us_teaspoons         = Unit{Unit::Volume,         USCustomary, "tsp",  "L",   [](double x){return x*0.00492892159;}, [](double y){return y/0.00492892159;},  1.0};
Unit const Units::imperial_barrels     = Unit{Unit::Volume,         Imperial,    "bbl",  "L",   [](double x){return x*163.659;},       [](double y){return y/163.659;},        1.0};
Unit const Units::imperial_gallons     = Unit{Unit::Volume,         Imperial,    "gal",  "L",   [](double x){return x*4.54609;},       [](double y){return y/4.54609;},        1.0};
Unit const Units::imperial_quarts      = Unit{Unit::Volume,         Imperial,    "qt",   "L",   [](double x){return x*1.1365225;},     [](double y){return y/1.1365225;},      1.0};
Unit const Units::imperial_cups        = Unit{Unit::Volume,         Imperial,    "cup",  "L",   [](double x){return x*0.284130625;},   [](double y){return y/0.284130625;},    0.25};
Unit const Units::imperial_tablespoons = Unit{Unit::Volume,         Imperial,    "tbsp", "L",   [](double x){return x*0.0177581714;},  [](double y){return y/0.0177581714;},   1.0};
Unit const Units::imperial_teaspoons   = Unit{Unit::Volume,         Imperial,    "tsp",  "L",   [](double x){return x*0.00591939047;}, [](double y){return y/0.00591939047;},  1.0};
// === Time ===
Unit const Units::seconds              = Unit{Unit::Time,           Any,         "s",    "min", [](double x){return x/60.0;},          [](double y){return y*60.0;},           90.0};
Unit const Units::minutes              = Unit{Unit::Time,           Any,         "min",  "min", [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::hours                = Unit{Unit::Time,           Any,         "hr",   "min", [](double x){return x*60.0;},          [](double y){return y/60.0;},           2.0};
Unit const Units::days                 = Unit{Unit::Time,           Any,         "day",  "min", [](double x){return x*1440.0;},        [](double y){return y/1440.0;},         1.0};
// === Temperature ===
Unit const Units::celsius              = Unit{Unit::Temp,           SI,          "C",    "C",   [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::fahrenheit           = Unit{Unit::Temp,           USCustomary, "F",    "C",   [](double x){return (x-32)*5.0/9.0;},  [](double y){return y * 9.0/5.0 + 32;}, 1.0};
Unit const Units::kelvin               = Unit{Unit::Temp,           SI,          "K",    "C",   [](double x){return x - 273.15;},      [](double y){return y + 273.15;},       1.0};
// === Color ===
// I will consider the standard unit of color  to be SRM.
Unit const Units::srm                  = Unit{Unit::Color,          SI,          "srm",  "srm", [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::ebc                  = Unit{Unit::Color,          SI,          "ebc",  "srm", [](double x){return x * 12.7/25.0;},   [](double y){return y * 25.0/12.7;},    1.0};
// == Density ===
// Specific gravity (aka, Sg) will be the standard unit, since that is how we store things in the database.
Unit const Units::sp_grav              = Unit{Unit::Density,        Any,         "sg",   "sg",  [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::plato                = Unit{Unit::Density,        Any,         "P",    "sg",  [](double x){return x == 0.0 ? 0.0 : Algorithms::PlatoToSG_20C20C(x);},
                                                                                                                                       [](double y){return y == 0.0 ? 0.0 : Algorithms::SG_20C20C_toPlato(y);},
                                                                                                                                                                               1.0};
// == Diastatic power ==
// Lintner will be the standard unit, since that is how we store things in the database.
Unit const Units::lintner              = Unit{Unit::DiastaticPower, Any,         "L",    "L",   [](double x){return x;},               [](double y){return y;},                1.0};
Unit const Units::wk                   = Unit{Unit::DiastaticPower, Any,         "WK",   "L",   [](double x){return (x + 16) / 3.5;},  [](double y){return 3.5 * y - 16;},     1.0};

QMultiMap<QString, Unit const *> const Unit::nameToUnit{
   {Units::kilograms.unitName,              &Units::kilograms},
   {Units::grams.unitName,                  &Units::grams},
   {Units::milligrams.unitName,             &Units::milligrams},
   {Units::pounds.unitName,                 &Units::pounds},
   {Units::ounces.unitName,                 &Units::ounces},
   {Units::liters.unitName,                 &Units::liters},
   {Units::milliliters.unitName,            &Units::milliliters},
   {Units::us_barrels.unitName,             &Units::us_barrels},
   {Units::us_gallons.unitName,             &Units::us_gallons},
   {Units::us_quarts.unitName,              &Units::us_quarts},
   {Units::us_cups.unitName,                &Units::us_cups},
   {Units::us_tablespoons.unitName,         &Units::us_tablespoons},
   {Units::us_teaspoons.unitName,           &Units::us_teaspoons},
   {Units::imperial_barrels.unitName,       &Units::imperial_barrels},
   {Units::imperial_gallons.unitName,       &Units::imperial_gallons},
   {Units::imperial_quarts.unitName,        &Units::imperial_quarts},
   {Units::imperial_cups.unitName,          &Units::imperial_cups},
   {Units::imperial_tablespoons.unitName,   &Units::imperial_tablespoons},
   {Units::imperial_teaspoons.unitName,     &Units::imperial_teaspoons},
   {Units::seconds.unitName,                &Units::seconds},
   {Units::minutes.unitName,                &Units::minutes},
   {Units::hours.unitName,                  &Units::hours},
   {Units::days.unitName,                   &Units::days},
   {Units::celsius.unitName,                &Units::celsius},
   {Units::fahrenheit.unitName,             &Units::fahrenheit},
   {Units::kelvin.unitName,                 &Units::kelvin},
   {Units::srm.unitName,                    &Units::srm},
   {Units::ebc.unitName,                    &Units::ebc},
   {Units::sp_grav.unitName,                &Units::sp_grav},
   {Units::plato.unitName,                  &Units::plato},
   {Units::lintner.unitName,                &Units::lintner},
   {Units::wk.unitName,                     &Units::wk}
};
