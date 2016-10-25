/*
 * unit.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include <QStringList>
#include <string>
#include <iostream>
#include <QRegExp>
#include <QDebug>
#include "unit.h"
#include "brewtarget.h"
#include "Algorithms.h"

QMultiMap<QString, Unit*> Unit::nameToUnit;
bool Unit::isMapSetup = false;

// === Mass ===
KilogramUnit* Units::kilograms = new KilogramUnit();
GramUnit* Units::grams = new GramUnit();
MilligramUnit* Units::milligrams = new MilligramUnit();

PoundUnit* Units::pounds = new PoundUnit();
OunceUnit* Units::ounces = new OunceUnit();
// === Volume ===
LiterUnit* Units::liters = new LiterUnit();
MilliliterUnit* Units::milliliters = new MilliliterUnit();

USBarrelUnit* Units::us_barrels = new USBarrelUnit();
USGallonUnit* Units::us_gallons = new USGallonUnit();
USQuartUnit* Units::us_quarts = new USQuartUnit();
USCupUnit* Units::us_cups = new USCupUnit();
USTablespoonUnit* Units::us_tablespoons = new USTablespoonUnit();
USTeaspoonUnit* Units::us_teaspoons = new USTeaspoonUnit();

ImperialBarrelUnit* Units::imperial_barrels = new ImperialBarrelUnit();
ImperialGallonUnit* Units::imperial_gallons = new ImperialGallonUnit();
ImperialQuartUnit* Units::imperial_quarts = new ImperialQuartUnit();
ImperialCupUnit* Units::imperial_cups = new ImperialCupUnit();
ImperialTablespoonUnit* Units::imperial_tablespoons = new ImperialTablespoonUnit();
ImperialTeaspoonUnit* Units::imperial_teaspoons = new ImperialTeaspoonUnit();
// === Time ===
SecondUnit* Units::seconds = new SecondUnit();
MinuteUnit* Units::minutes = new MinuteUnit();
HourUnit* Units::hours = new HourUnit();
DayUnit* Units::days = new DayUnit();
// === Temperature ===
CelsiusUnit* Units::celsius = new CelsiusUnit();
FahrenheitUnit* Units::fahrenheit = new FahrenheitUnit();
KelvinUnit* Units::kelvin = new KelvinUnit();
// === Color ===
SRMUnit* Units::srm = new SRMUnit();
EBCUnit* Units::ebc = new EBCUnit();
// == density ===
SgUnit* Units::sp_grav = new SgUnit();
PlatoUnit* Units::plato = new PlatoUnit();
// == diastatic power ==
LintnerUnit* Units::lintner = new LintnerUnit();;
WKUnit* Units::wk = new WKUnit();

QString Unit::unitFromString(QString qstr)
{
   QRegExp amtUnit;

   // Make sure we get the right decimal point (. or ,) and the right grouping
   // separator (, or .). Some locales write 1.000,10 and other write
   // 1,000.10. We need to catch both
   QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

   // if the regex dies, return 0.0
   if (amtUnit.indexIn(qstr) == -1)
      return QString("?");

   // Get the unit from the second capture
   return amtUnit.cap(2);

}

double Unit::valueFromString(QString qstr)
{
   QRegExp amtUnit;

   // Make sure we get the right decimal point (. or ,) and the right grouping
   // separator (, or .). Some locales write 1.000,10 and other write
   // 1,000.10. We need to catch both
   QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);

   // if the regex dies, return 0.0
   if (amtUnit.indexIn(qstr) == -1)
      return 0.0;

   return Brewtarget::toDouble(amtUnit.cap(1), "Unit::valueFromString()");
}

// Return a
QString Unit::convert(QString qstr, QString toUnit)
{
   QString fName;
   double amt,si;
   Unit *f, *u;

   if( ! Unit::isMapSetup )
      Unit::setupMap();

   fName = unitFromString(qstr);
   amt = valueFromString(qstr);
   f = getUnit(fName);

   if ( f )
      si = f->toSI(amt);
   else
      si = 0.0;

   u = getUnit(toUnit, false);

   // If we couldn't find either unit, or the two units don't match (eg, you
   // cannot convert L to lb)
   if( u == 0 || f == 0 || u->getUnitType() != f->getUnitType() )
      return QString("%1 ?").arg(Brewtarget::displayAmount(si));
   else
      return QString("%1 %2").arg(Brewtarget::displayAmount(u->fromSI(si))).arg(toUnit);
}

// This mostly gets called when the unit entered in the field does not match
// what the field has been set to. For example, if you displaying in Liters,
// but enter "20 qt". Since the SIVolumeUnitSystem doesn't know what "qt" is,
// we go searching for it.
Unit* Unit::getUnit(QString& name, bool matchCurrentSystem)
{
   Unit* u;
   Unit* defUnit = 0;


   if( ! Unit::isMapSetup )
      Unit::setupMap();

   // Under most circumstances, there is a one-to-one relationship between
   // unit string and Unit. C will only map to Unit::Celsius, for example. If
   // there's only one match, just return it.
   if ( nameToUnit.count(name) == 1 )
      return nameToUnit.value(name);

   // That solved something like 99% of the use cases. Now we have to handle
   // those pesky volumes.
   QMap<QString, Unit*>::iterator i = nameToUnit.find(name);

   // Loop through the found Units, like Unit::us_quart and
   // Unit::imperial_quart, and try to find one that matches the global
   // default.
   for( ; i != nameToUnit.end() && i.key() == name; ++i )
   {
      u = i.value();
      if( u == 0 )
         continue;

      int system = u->getUnitOrTempSystem();

      // Save this for later if we need it
      if( system == USCustomary )
         defUnit = u;

      if( Brewtarget::thingToUnitSystem.value(Volume|system) == Brewtarget::thingToUnitSystem.value(Volume) )
         return u;
   }

   // If we got here, we couldn't find a match. Unless something weird has
   // happened, that means you entered "qt" into a field and the system
   // default is SI. At that point, just use the USCustomary
   return defUnit;
}

void Unit::setupMap()
{
   Unit::nameToUnit.insert(Units::kilograms->getUnitName(), Units::kilograms);
   Unit::nameToUnit.insert(Units::grams->getUnitName(), Units::grams);
   Unit::nameToUnit.insert(Units::milligrams->getUnitName(), Units::milligrams);

   Unit::nameToUnit.insert(Units::pounds->getUnitName(), Units::pounds);
   Unit::nameToUnit.insert(Units::ounces->getUnitName(), Units::ounces);

   Unit::nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   Unit::nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);

   Unit::nameToUnit.insert(Units::us_gallons->getUnitName(), Units::us_gallons);
   Unit::nameToUnit.insert(Units::us_quarts->getUnitName(), Units::us_quarts);
   Unit::nameToUnit.insert(Units::us_cups->getUnitName(), Units::us_cups);
   Unit::nameToUnit.insert(Units::us_tablespoons->getUnitName(), Units::us_tablespoons);
   Unit::nameToUnit.insert(Units::us_teaspoons->getUnitName(), Units::us_teaspoons);

   Unit::nameToUnit.insert(Units::imperial_gallons->getUnitName(), Units::imperial_gallons);
   Unit::nameToUnit.insert(Units::imperial_quarts->getUnitName(), Units::imperial_quarts);
   Unit::nameToUnit.insert(Units::imperial_cups->getUnitName(), Units::imperial_cups);
   Unit::nameToUnit.insert(Units::imperial_tablespoons->getUnitName(), Units::imperial_tablespoons);
   Unit::nameToUnit.insert(Units::imperial_teaspoons->getUnitName(), Units::imperial_teaspoons);

   Unit::nameToUnit.insert(Units::seconds->getUnitName(), Units::seconds);
   Unit::nameToUnit.insert(Units::minutes->getUnitName(), Units::minutes);
   Unit::nameToUnit.insert(Units::hours->getUnitName(), Units::hours);
   Unit::nameToUnit.insert(Units::days->getUnitName(), Units::days);

   Unit::nameToUnit.insert(Units::celsius->getUnitName(), Units::celsius);
   Unit::nameToUnit.insert(Units::kelvin->getUnitName(), Units::kelvin);
   Unit::nameToUnit.insert(Units::fahrenheit->getUnitName(), Units::fahrenheit);

   Unit::nameToUnit.insert(Units::srm->getUnitName(), Units::srm);
   Unit::nameToUnit.insert(Units::ebc->getUnitName(), Units::ebc);

   Unit::nameToUnit.insert(Units::sp_grav->getUnitName(), Units::sp_grav);
   Unit::nameToUnit.insert(Units::plato->getUnitName(), Units::plato);

   Unit::nameToUnit.insert(Units::lintner->getUnitName(), Units::lintner);
   Unit::nameToUnit.insert(Units::wk->getUnitName(), Units::wk);

   Unit::isMapSetup = true;
}

// === Kilograms ===
KilogramUnit::KilogramUnit()
{
   unitName = "kg";
   SIUnitName = "kg";
   _type = Mass;
   _unitSystem = SI;
}

double KilogramUnit::toSI( double amt ) const
{
   return amt;
}

double KilogramUnit::fromSI( double amt ) const
{
   return amt;
}

// === Grams ===
GramUnit::GramUnit()
{
   unitName = "g";
   SIUnitName = "kg";
   _type = Mass;
   _unitSystem = SI;
}

double GramUnit::toSI( double amt ) const
{
   return amt / (double)1000.0;
}

double GramUnit::fromSI( double amt ) const
{
   return amt * (double)1000.0;
}

// === Milligrams ===
MilligramUnit::MilligramUnit()
{
   unitName = "mg";
   SIUnitName = "kg";
   _type = Mass;
   _unitSystem = SI;
}

double MilligramUnit::toSI( double amt ) const
{
   return amt / (double)1000000.0;
}

double MilligramUnit::fromSI( double amt ) const
{
   return amt * (double)1000000.0;
}

// === Pounds ===
PoundUnit::PoundUnit()
{
   unitName = "lb";
   SIUnitName = "kg";
   _type = Mass;
   _unitSystem = USCustomary;
}

double PoundUnit::toSI( double amt ) const
{
   return amt * 0.45359237;
}
      
double PoundUnit::fromSI( double amt ) const
{
   return amt * 2.2046226;
}

// === Ounces (weight) ===
OunceUnit::OunceUnit()
{
   unitName = "oz";
   SIUnitName = "kg";
   _type = Mass;
   _unitSystem = USCustomary;
}

double OunceUnit::toSI( double amt ) const
{
   return amt * 0.0283495231;
}

double OunceUnit::fromSI( double amt ) const
{
   return amt * 35.2739619;
}

// === Liters ===
LiterUnit::LiterUnit()
{
   unitName = "L";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = SI;
}

double LiterUnit::toSI( double amt ) const
{
   return amt;
}

double LiterUnit::fromSI( double amt ) const
{
   return amt;
}

// === Milliliters ===
MilliliterUnit::MilliliterUnit()
{
   unitName = "mL";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = SI;
}

double MilliliterUnit::toSI( double amt ) const
{
   return amt / (double)1000.0;
}

double MilliliterUnit::fromSI( double amt ) const
{
   return amt * (double)1000.0;
}

// === US Beer Barrel ===

USBarrelUnit::USBarrelUnit()
{
   unitName = "bbl";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USBarrelUnit::toSI( double amt ) const
{
   return amt * 117.34777;
}

double USBarrelUnit::fromSI( double amt ) const
{
   return amt / 117.34777;
}

// === USGallons ===
USGallonUnit::USGallonUnit()
{
   unitName = "gal";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USGallonUnit::toSI( double amt ) const
{
   return amt * 3.78541178;
}

double USGallonUnit::fromSI( double amt ) const
{
   return amt / 3.78541178;
}

// === USQuarts ===
USQuartUnit::USQuartUnit()
{
   unitName = "qt";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USQuartUnit::toSI( double amt ) const
{
   return amt * 0.946352946;
}

double USQuartUnit::fromSI( double amt ) const
{
   return amt / 0.946352946;
}

// === USCups ===
USCupUnit::USCupUnit()
{
   unitName = "cup";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USCupUnit::toSI( double amt ) const
{
   return amt * 0.236588236;
}

double USCupUnit::fromSI( double amt ) const
{
   return amt / 0.236588236;
}

// === US Tablepoons ===
USTablespoonUnit::USTablespoonUnit()
{
   unitName = "tbsp";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USTablespoonUnit::toSI( double amt ) const
{
   return amt * 0.0147867648;
}

double USTablespoonUnit::fromSI( double amt ) const
{
   return amt / 0.0147867648;
}

// === US Teaspoons ===
USTeaspoonUnit::USTeaspoonUnit()
{
   unitName = "tsp";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = USCustomary;
}

double USTeaspoonUnit::toSI( double amt ) const
{
   return amt * 0.00492892159;
}

double USTeaspoonUnit::fromSI( double amt ) const
{
   return amt / 0.00492892159;
}

// === Imperial Beer Barrel ===

ImperialBarrelUnit::ImperialBarrelUnit()
{
   unitName = "bbl";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialBarrelUnit::toSI( double amt ) const
{
   return amt * 163.659;
}

double ImperialBarrelUnit::fromSI( double amt ) const
{
   return amt / 163.659;
}

// === ImperialGallons ===
ImperialGallonUnit::ImperialGallonUnit()
{
   unitName = "gal";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialGallonUnit::toSI( double amt ) const
{
   return amt * 4.54609;
}

double ImperialGallonUnit::fromSI( double amt ) const
{
   return amt / 4.54609;
}

// === ImperialQuarts ===
ImperialQuartUnit::ImperialQuartUnit()
{
   unitName = "qt";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialQuartUnit::toSI( double amt ) const
{
   return amt * 1.1365225;
}

double ImperialQuartUnit::fromSI( double amt ) const
{
   return amt / 1.1365225;
}

// === ImperialCups ===
ImperialCupUnit::ImperialCupUnit()
{
   unitName = "cup";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialCupUnit::toSI( double amt ) const
{
   return amt * 0.284130625;
}

double ImperialCupUnit::fromSI( double amt ) const
{
   return amt / 0.284130625;
}


// === Imperial Tablepoons ===
ImperialTablespoonUnit::ImperialTablespoonUnit()
{
   unitName = "tbsp";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialTablespoonUnit::toSI( double amt ) const
{
   return amt * 0.0177581714;
}

double ImperialTablespoonUnit::fromSI( double amt ) const
{
   return amt / 0.0177581714;
}

// === Imperial Teaspoons ===
ImperialTeaspoonUnit::ImperialTeaspoonUnit()
{
   unitName = "tsp";
   SIUnitName = "L";
   _type = Volume;
   _unitSystem = Imperial;
}

double ImperialTeaspoonUnit::toSI( double amt ) const
{
   return amt * 0.00591939047;
}

double ImperialTeaspoonUnit::fromSI( double amt ) const
{
   return amt / 0.00591939047;
}

// === Seconds ===
SecondUnit::SecondUnit()
{
   unitName = "s";
   SIUnitName = "min";
   _type = Time;
   _unitSystem = Any;
}

double SecondUnit::toSI( double amt ) const
{
   return amt/(double)60.0;
}

double SecondUnit::fromSI( double amt ) const
{
   return amt*(double)60.0;
}

// === Minutes ===
MinuteUnit::MinuteUnit()
{
   unitName = "min";
   SIUnitName = "min";
   _type = Time;
   _unitSystem = Any;
}

double MinuteUnit::toSI( double amt ) const
{
   return amt;
}

double MinuteUnit::fromSI( double amt ) const
{
   return amt;
}

// === Hours ===
HourUnit::HourUnit()
{
   unitName = "hr";
   SIUnitName = "min";
   _type = Time;
   _unitSystem = Any;
}

double HourUnit::toSI( double amt ) const
{
   return amt * (double)60.0;
}

double HourUnit::fromSI( double amt ) const
{
   return amt / (double)60.0;
}

// === Days ===
DayUnit::DayUnit()
{
   unitName = "day";
   SIUnitName = "min";
   _type = Time;
   _unitSystem = Any;
}

double DayUnit::toSI( double amt ) const
{
   return amt * (double)1440.0;
}

double DayUnit::fromSI( double amt ) const
{
   return amt / (double)1440.0;
}

// === Celsius ===
CelsiusUnit::CelsiusUnit()
{
   unitName = "C";
   SIUnitName = "C";
   _type = Temp;
   _unitSystem = SI;
}

double CelsiusUnit::toSI( double amt ) const
{
   return amt;
}

double CelsiusUnit::fromSI( double amt ) const
{
   return amt;
}

// === Fahrenheit ===
FahrenheitUnit::FahrenheitUnit()
{
   unitName = "F";
   SIUnitName = "C";
   _type = Temp;
   _unitSystem = USCustomary;
}

double FahrenheitUnit::toSI( double amt ) const
{
   return (amt-32)*5/9;
}

double FahrenheitUnit::fromSI( double amt ) const
{
   return amt*9/5 + 32;
}

// === Kelvin ===
KelvinUnit::KelvinUnit()
{
   unitName = "K";
   SIUnitName = "K";
   _type = Temp;
   _unitSystem = SI;
}

double KelvinUnit::toSI( double amt ) const
{
   return amt - 273.15;
}

double KelvinUnit::fromSI( double amt ) const
{
   return amt + 273.15;
}

// === SRM ===
SRMUnit::SRMUnit()
{
   unitName = "srm";
   SIUnitName = "srm";
   _type = Color;
   _unitSystem = Any;
}

double SRMUnit::toSI( double amt ) const
{
   return amt;
}

double SRMUnit::fromSI( double amt ) const
{
   return amt;
}

// === EBC ===
EBCUnit::EBCUnit()
{
   unitName = "ebc";
   SIUnitName = "srm";
   _type = Color;
   _unitSystem = Any;
}

double EBCUnit::toSI( double amt ) const
{
   return amt * 12.7/25.0;
}

double EBCUnit::fromSI( double amt ) const
{
   return amt * 25.0/12.7;
}

// === Density ===
SgUnit::SgUnit()
{
   unitName   = "sg";
   SIUnitName = "sg";
   _type = Density;
   _unitSystem = Any;
}

double SgUnit::toSI( double amt ) const { return amt; }

double SgUnit::fromSI( double amt ) const { return amt; }

PlatoUnit::PlatoUnit()
{
   unitName = "P";
   SIUnitName = "sg";
   _type = Density;
   _unitSystem = Any;
}

double PlatoUnit::toSI( double amt ) const
{
   return Algorithms::PlatoToSG_20C20C( amt );
}

double PlatoUnit::fromSI(double amt) const
{
   return Algorithms::SG_20C20C_toPlato(amt);
}

// == diastatic power ==
LintnerUnit::LintnerUnit()
{
   unitName   = "L";
   SIUnitName = "L";
   _type = DiastaticPower;
   _unitSystem = Any;
}

double LintnerUnit::toSI( double amt ) const
{
   return amt;
}

double LintnerUnit::fromSI( double amt ) const
{
   return amt;
}

WKUnit::WKUnit()
{
   unitName = "WK";
   SIUnitName = "L";
   _type = DiastaticPower;
   _unitSystem = Any;
}

double WKUnit::toSI( double amt ) const
{
   return (amt + 16) / 3.5;
}

double WKUnit::fromSI(double amt) const
{
   return 3.5 * amt - 16;
}
