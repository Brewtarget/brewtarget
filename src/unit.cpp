/*
 * unit.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QStringList>
#include <string>
#include <iostream>
#include "unit.h"
#include "stringparsing.h"
#include "brewtarget.h"

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
USGallonUnit* Units::us_gallons = new USGallonUnit();
USQuartUnit* Units::us_quarts = new USQuartUnit();
USCupUnit* Units::us_cups = new USCupUnit();
ImperialGallonUnit* Units::imperial_gallons = new ImperialGallonUnit();
ImperialQuartUnit* Units::imperial_quarts = new ImperialQuartUnit();
ImperialCupUnit* Units::imperial_cups = new ImperialCupUnit();
USTablespoonUnit* Units::us_tablespoons = new USTablespoonUnit();
USTeaspoonUnit* Units::us_teaspoons = new USTeaspoonUnit();
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

// Return the equivalent of 'amount' 'fromUnit's in 'toUnit's.
double Unit::convert( double amount, QString& fromUnit, QString& toUnit )
{
   double SI;

   if( ! Unit::isMapSetup )
      Unit::setupMap();

   // TODO: warn somebody if the units aren't in the map.
   Unit* f;
   Unit* t;

   f = getUnit(fromUnit);
   t = getUnit(toUnit, false);

   // Freak out if we can't find the units or if they're not the same type.
   if( f == 0 || t == 0  || f->getUnitType() != t->getUnitType() )
      return 0.0;

   SI = f->toSI(amount);
   return t->fromSI(SI);
}

// Gets the unit with the appropriate name. Select the one consistent
// with the current system (like Brewtarget::getWeightUnitSystem())
// if possible. Otherwise, get a unit of type USCustomary or Any.
Unit* Unit::getUnit(QString& name, bool matchCurrentSystem)
{
   Unit* u;
   QMultiMap<QString, Unit*>::iterator i = nameToUnit.find(name);

   // First, try to find a unit consistent with the measurement system.
   for( ; i != nameToUnit.end() && i.key() == name; ++i )
   {
      u = i.value();
      if( u == 0 )
         continue;

      int type = u->getUnitType();
      int system = u->getUnitOrTempSystem();

      if( type == Temp && system == Brewtarget::getTemperatureScale() )
         return u;
      else if( type == Mass )
      {
         if( system == Any || system == Brewtarget::getWeightUnitSystem() )
            return u;

         if( (Brewtarget::getWeightUnitSystem() == USCustomary || Brewtarget::getWeightUnitSystem() == Imperial)
            && system == ImperialAndUS)
            return u;
      }
      else if( type == Volume )
      {
         if( system == Any || system == Brewtarget::getVolumeUnitSystem() )
            return u;

         if( (Brewtarget::getVolumeUnitSystem() == USCustomary || Brewtarget::getVolumeUnitSystem() == Imperial)
            && system == ImperialAndUS )
            return u;
      }
      else if( type == Time )
         return u;
   }

   i = nameToUnit.find(name);

   // Now, just try to find a unit with the USCustomary or Any system.
   for( ; i != nameToUnit.end() && i.key() == name; ++i )
   {
      u = i.value();
      if( u == 0 )
         continue;

      int system = u->getUnitOrTempSystem();

      if( system == Any || system == USCustomary || system == ImperialAndUS || matchCurrentSystem == false )
         return u;
   }

   return 0;
}

// Translates something like "5.0 gal" into the appropriate SI units.
double Unit::qstringToSI( QString qstr, Unit** unit )
{
   if( ! Unit::isMapSetup )
      Unit::setupMap();

   QStringList list1 = qstr.split(" ");

   if( list1.size() < 1 ) // Didn't even provide a number.
      return 0.0;
   else if( list1.size() < 2  ) // Only provided a number.
   {
      // If we don't have units, just assume we're dealing with mass.
      switch(Brewtarget::weightUnitSystem)
      {
         case USCustomary:
         case Imperial:
            return Units::pounds->toSI(list1[0].toDouble());

         case SI:
         default:
            return list1[0].toDouble();
      }
   }
   else // Provided a number and unit.
   {
      Unit* u = getUnit(list1[1]);

      if( u == 0 ) // Invalid unit since it's not in the map.
         return list1[0].toDouble(); // Assume units are already SI.
      else
      {
         if( unit != 0 )
            *unit = u;
         return u->toSI(list1[0].toDouble());
      }
   }
}

// Return a
QString Unit::convert(QString qstr, QString toUnit)
{
   if( ! Unit::isMapSetup )
      Unit::setupMap();

   Unit* f;
   double si = qstringToSI( qstr, &f );
   Unit* u = getUnit(toUnit, false);
   
   if( u == 0 || f == 0 || u->getUnitType() != f->getUnitType() )
      return QString("%1 ?").arg(si, 0, 'f', 3);
   else
      return QString("%1 %2").arg(u->fromSI(si), 0, 'f', 3).arg(toUnit);
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

   Unit::isMapSetup = true;
}

// === Kilograms ===
KilogramUnit::KilogramUnit()
{
   unitName = "kg";
   SIUnitName = "kg";
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
}

double MilliliterUnit::toSI( double amt ) const
{
   return amt / (double)1000.0;
}

double MilliliterUnit::fromSI( double amt ) const
{
   return amt * (double)1000.0;
}

// === USGallons ===
USGallonUnit::USGallonUnit()
{
   unitName = "gal";
   SIUnitName = "L";
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
}

double USCupUnit::toSI( double amt ) const
{
   return amt * 0.236588236;
}

double USCupUnit::fromSI( double amt ) const
{
   return amt / 0.236588236;
}

// === ImperialGallons ===
ImperialGallonUnit::ImperialGallonUnit()
{
   unitName = "gal";
   SIUnitName = "L";
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
}

double ImperialCupUnit::toSI( double amt ) const
{
   return amt * 0.284130625;
}

double ImperialCupUnit::fromSI( double amt ) const
{
   return amt / 0.284130625;
}


// === US Tablepoons ===
USTablespoonUnit::USTablespoonUnit()
{
   unitName = "tbsp";
   SIUnitName = "L";
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
}

double USTeaspoonUnit::toSI( double amt ) const
{
   return amt * 0.00492892159;
}

double USTeaspoonUnit::fromSI( double amt ) const
{
   return amt / 0.00492892159;
}

// === Imperial Tablepoons ===
ImperialTablespoonUnit::ImperialTablespoonUnit()
{
   unitName = "tbsp";
   SIUnitName = "L";
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
   SIUnitName = "min"; // Pretend the SI unit is minutes for the sake of BeerXML.
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
   SIUnitName = "min"; // Pretend the SI unit is minutes for the sake of BeerXML.
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
   SIUnitName = "min"; // Pretend the SI unit is minutes for the sake of BeerXML.
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
   SIUnitName = "min"; // Pretend the SI unit is minutes for the sake of BeerXML.
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
}

double CelsiusUnit::toSI( double amt ) const
{
   return amt;
}

double CelsiusUnit::fromSI( double amt ) const
{
   return amt;
}

// === Celsius ===
FahrenheitUnit::FahrenheitUnit()
{
   unitName = "F";
   SIUnitName = "C";
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
   SIUnitName = "C";
}

double KelvinUnit::toSI( double amt ) const
{
   return amt - 273.15;
}

double KelvinUnit::fromSI( double amt ) const
{
   return amt + 273.15;
}
