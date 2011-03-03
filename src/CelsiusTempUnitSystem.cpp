/*
 * CelsiusTempUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

#include "CelsiusTempUnitSystem.h"
#include <QStringList>

bool CelsiusTempUnitSystem::isMapSetup = false;
QMap<QString, Unit*> CelsiusTempUnitSystem::nameToUnit;

CelsiusTempUnitSystem::CelsiusTempUnitSystem()
{
}

QString CelsiusTempUnitSystem::displayAmount( double amount, Unit* units )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with temperature.
   if( units == 0 || SIUnitName.compare("C") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   ret = QString("%1 %2").arg(Units::celsius->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::celsius->getUnitName());

   return ret;
}

double CelsiusTempUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   QStringList list1 = qstr.split(" ");
   int listSize = list1.size();
   double amt = 0.0;

   // If we have more than one item, assume the first is the amount.
   if( listSize >= 1 )
      amt = list1[0].toDouble();

   if( listSize < 1 ) // Didn't even provide a number.
      return 0.0;
   else if( listSize < 2  ) // Only provided a number. Assume it's in C.
      return amt;
   else // Provided a number and unit.
   {
      Unit* u = getUnit(list1[1]);

      if( u == 0 ) // Invalid unit since it's not in the map.
         return amt; // Assume units are C.
      else
         return u->toSI(amt);
   }
}

void CelsiusTempUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.
   //nameToUnit.insert(Units::kilograms->getUnitName(), Units::kilograms);
   //nameToUnit.insert(Units::grams->getUnitName(), Units::grams);
   //nameToUnit.insert(Units::milligrams->getUnitName(), Units::milligrams);

   //nameToUnit.insert(Units::pounds->getUnitName(), Units::pounds);
   //nameToUnit.insert(Units::ounces->getUnitName(), Units::ounces);

   //nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   //nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);

   //nameToUnit.insert(Units::us_gallons->getUnitName(), Units::us_gallons);
   //nameToUnit.insert(Units::us_quarts->getUnitName(), Units::us_quarts);
   //nameToUnit.insert(Units::us_cups->getUnitName(), Units::us_cups);
   //nameToUnit.insert(Units::us_tablespoons->getUnitName(), Units::us_tablespoons);
   //nameToUnit.insert(Units::us_teaspoons->getUnitName(), Units::us_teaspoons);

   // Assume that "gal" "qt" etc. do NOT refer to the imperial versions.
   /*
   nameToUnit.insert(Units::imperial_gallons->getUnitName(), Units::imperial_gallons);
   nameToUnit.insert(Units::imperial_quarts->getUnitName(), Units::imperial_quarts);
   nameToUnit.insert(Units::imperial_cups->getUnitName(), Units::imperial_cups);
   nameToUnit.insert(Units::imperial_tablespoons->getUnitName(), Units::imperial_tablespoons);
   nameToUnit.insert(Units::imperial_teaspoons->getUnitName(), Units::imperial_teaspoons);
   */

   //nameToUnit.insert(Units::seconds->getUnitName(), Units::seconds);
   //nameToUnit.insert(Units::minutes->getUnitName(), Units::minutes);
   //nameToUnit.insert(Units::hours->getUnitName(), Units::hours);

   nameToUnit.insert(Units::celsius->getUnitName(), Units::celsius);
   nameToUnit.insert(Units::kelvin->getUnitName(), Units::kelvin);
   nameToUnit.insert(Units::fahrenheit->getUnitName(), Units::fahrenheit);

   isMapSetup = true;
}

Unit* CelsiusTempUnitSystem::getUnit(const QString& name)
{
   if( nameToUnit.count(name) < 1 )
      return 0;
   else
      return nameToUnit[name];
}
