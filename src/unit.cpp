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

std::map<std::string, Unit*> Unit::nameToUnit;
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
GallonUnit* Units::gallons = new GallonUnit();
QuartUnit* Units::quarts = new QuartUnit();
CupUnit* Units::cups = new CupUnit();
TablespoonUnit* Units::tablespoons = new TablespoonUnit();
TeaspoonUnit* Units::teaspoons = new TeaspoonUnit();
// === Time ===
SecondUnit* Units::seconds = new SecondUnit();
MinuteUnit* Units::minutes = new MinuteUnit();
HourUnit* Units::hours = new HourUnit();
// === Temperature ===
CelsiusUnit* Units::celsius = new CelsiusUnit();
FahrenheitUnit* Units::fahrenheit = new FahrenheitUnit();
KelvinUnit* Units::kelvin = new KelvinUnit();

double Unit::convert( double amount, const std::string& fromUnit, const std::string& toUnit )
{
   double SI;

   if( ! Unit::isMapSetup )
      Unit::setupMap();

   // TODO: warn somebody if the units aren't in the map.
   SI = Unit::nameToUnit[fromUnit]->toSI(amount);
   return Unit::nameToUnit[toUnit]->fromSI(SI);
}

/*
double Unit::stringToSI( std::string input )
{
   size_t pos;
   std::string num, units;

   if( ! Unit::isMapSetup )
      Unit::setupMap();
   
   pos = input.find_first_of(' ');
   num = input.substr( 0, pos );
   units = input.substr(pos);
   trim(num);
   trim(units);

   std::cerr << "Units: " + units << std::endl;
   std::cerr << "Num: " + num <<  std::endl;
   std::cerr << "Converted: " << Unit::nameToUnit[units]->toSI(parseDouble(num)) << " " << Unit::nameToUnit[units]->getSIUnitName() << std::endl;

   // If we are not provided units, assume "num" is already in
   // SI units.
   if( units.length() == 0 )
      return parseDouble(num);
   else
      return Unit::nameToUnit[units]->toSI(parseDouble(num));
}
 */

// Translates something like "5.0 gal" into the appropriate SI units.
double Unit::qstringToSI( QString qstr )
{
   if( ! Unit::isMapSetup )
      Unit::setupMap();

   QStringList list1 = qstr.split(" ");

   if( list1.size() < 1 ) // Didn't even provide a number.
      return 0.0;
   else if( list1.size() < 2  ) // Only provided a number.
      return list1[0].toDouble(); // Assume units are already SI.
   else // Provided a number and unit.
   {
      std::string units;
      units = list1[1].toStdString();

      if( nameToUnit[units] == 0 ) // Invalid unit since it's not in the map.
         return list1[0].toDouble(); // Assume units are already SI.
      else
         return Unit::nameToUnit[units]->toSI(list1[0].toDouble());
   }
}

void Unit::setupMap()
{
   Unit::nameToUnit[Units::kilograms->getUnitName()] = Units::kilograms;
   Unit::nameToUnit[Units::grams->getUnitName()] = Units::grams;
   Unit::nameToUnit[Units::milligrams->getUnitName()] = Units::milligrams;
   Unit::nameToUnit[Units::pounds->getUnitName()] = Units::pounds;
   Unit::nameToUnit[Units::ounces->getUnitName()] = Units::ounces;
   Unit::nameToUnit[Units::liters->getUnitName()] = Units::liters;
   Unit::nameToUnit[Units::milliliters->getUnitName()] = Units::milliliters;
   Unit::nameToUnit[Units::gallons->getUnitName()] = Units::gallons;
   Unit::nameToUnit[Units::quarts->getUnitName()] = Units::quarts;
   Unit::nameToUnit[Units::cups->getUnitName()] = Units::cups;
   Unit::nameToUnit[Units::tablespoons->getUnitName()] = Units::tablespoons;
   Unit::nameToUnit[Units::teaspoons->getUnitName()] = Units::teaspoons;
   Unit::nameToUnit[Units::seconds->getUnitName()] = Units::seconds;
   Unit::nameToUnit[Units::minutes->getUnitName()] = Units::minutes;
   Unit::nameToUnit[Units::hours->getUnitName()] = Units::hours;
   Unit::nameToUnit[Units::celsius->getUnitName()] = Units::celsius;
   Unit::nameToUnit[Units::kelvin->getUnitName()] = Units::kelvin;
   Unit::nameToUnit[Units::fahrenheit->getUnitName()] = Units::fahrenheit;

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

// === Gallons ===
GallonUnit::GallonUnit()
{
   unitName = "gal";
   SIUnitName = "L";
}

double GallonUnit::toSI( double amt ) const
{
   return amt * 3.78541178;
}

double GallonUnit::fromSI( double amt ) const
{
   return amt / 3.78541178;
}

// === Quarts ===
QuartUnit::QuartUnit()
{
   unitName = "qt";
   SIUnitName = "L";
}

double QuartUnit::toSI( double amt ) const
{
   return amt * 0.946352946;
}

double QuartUnit::fromSI( double amt ) const
{
   return amt / 0.946352946;
}

// === Cups ===
CupUnit::CupUnit()
{
   unitName = "cup";
   SIUnitName = "L";
}

double CupUnit::toSI( double amt ) const
{
   return amt * 0.236588236;
}

double CupUnit::fromSI( double amt ) const
{
   return amt / 0.236588236;
}

// === Tablepoons ===
TablespoonUnit::TablespoonUnit()
{
   unitName = "tbsp";
   SIUnitName = "L";
}

double TablespoonUnit::toSI( double amt ) const
{
   return amt * 0.0147867648;
}

double TablespoonUnit::fromSI( double amt ) const
{
   return amt / 0.0147867648;
}

// === Teaspoons ===
TeaspoonUnit::TeaspoonUnit()
{
   unitName = "tsp";
   SIUnitName = "L";
}

double TeaspoonUnit::toSI( double amt ) const
{
   return amt * 0.00492892159;
}

double TeaspoonUnit::fromSI( double amt ) const
{
   return amt / 0.00492892159;
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
