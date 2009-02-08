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

#include <string>
#include "unit.h"

std::map<std::string, Unit> Unit::nameToUnit;
bool Unit::isMapSetup;

double Unit::convert( double amount, const std::string& fromUnit, const std::string& toUnit )
{
   double SI;

   if( ! Unit::isMapSetup )
      Unit::setupMap();

   // TODO: warn somebody if the units aren't in the map.
   SI = Unit::nameToUnit[fromUnit].toSI(amount);
   return Unit::nameToUnit[toUnit].fromSI(SI);
}

void Unit::setupMap()
{
   Unit::nameToUnit[Kilograms.getUnitName()] = Kilograms;
   Unit::nameToUnit[Grams.getUnitName()] = Grams;
   Unit::nameToUnit[Milligrams.getUnitName()] = Milligrams;
   Unit::nameToUnit[Pounds.getUnitName()] = Pounds;
   Unit::nameToUnit[Ounces.getUnitName()] = Ounces;
   Unit::nameToUnit[Liters.getUnitName()] = Liters;
   Unit::nameToUnit[Milliliters.getUnitName()] = Milliliters;
   Unit::nameToUnit[Gallons.getUnitName()] = Gallons;
   Unit::nameToUnit[Quarts.getUnitName()] = Quarts;
   Unit::nameToUnit[Cups.getUnitName()] = Cups;
   Unit::nameToUnit[Tablespoons.getUnitName()] = Tablespoons;
   Unit::nameToUnit[Teaspoons.getUnitName()] = Teaspoons;
   Unit::nameToUnit[Seconds.getUnitName()] = Seconds;
   Unit::nameToUnit[Minutes.getUnitName()] = Minutes;

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
   SIUnitName = "s";
}

double SecondUnit::toSI( double amt ) const
{
   return amt;
}

double SecondUnit::fromSI( double amt ) const
{
   return amt;
}

// === Minutes ===
MinuteUnit::MinuteUnit()
{
   unitName = "min";
   SIUnitName = "s";
}

double MinuteUnit::toSI( double amt ) const
{
   return amt * 60.0;
}

double MinuteUnit::fromSI( double amt ) const
{
   return amt / 60.0;
}
