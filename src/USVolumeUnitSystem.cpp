/*
 * USVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2010.
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

#include "USVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>

bool USVolumeUnitSystem::isMapSetup = false;
QMap<QString, Unit*> USVolumeUnitSystem::nameToUnit;

USVolumeUnitSystem::USVolumeUnitSystem()
{
}

QString USVolumeUnitSystem::displayAmount( double amount, Unit* units )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || SIUnitName.compare("L") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   if( absSIAmount < Units::us_tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
      ret = QString("%1 %2").arg(Units::us_teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_teaspoons->getUnitName());
   else if( absSIAmount < Units::us_cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
      ret = QString("%1 %2").arg(Units::us_tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_tablespoons->getUnitName());
   else if( absSIAmount < Units::us_quarts->toSI(1.0) ) // If less than 1 qt, show us_cups
      ret = QString("%1 %2").arg(Units::us_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_cups->getUnitName());
   else if( absSIAmount < Units::us_gallons->toSI(1.0) ) // If less than 1 gallon, show us_quarts
      ret = QString("%1 %2").arg(Units::us_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_quarts->getUnitName());
   else
      ret = QString("%1 %2").arg(Units::us_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_gallons->getUnitName());

   return ret;
}

double USVolumeUnitSystem::qstringToSI( QString qstr )
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
   else if( listSize < 2  ) // Only provided a number. Assume it's gallons.
      return Units::us_gallons->toSI(amt);
   else // Provided a number and unit.
   {
      Unit* u = getUnit(list1[1]);

      if( u == 0 ) // Invalid unit since it's not in the map.
         return Units::us_gallons->toSI(amt); // Assume units are gallons.
      else
         return u->toSI(amt);
   }
}

void USVolumeUnitSystem::ensureMapIsSetup()
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

   nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);

   // Assume that "gal" "qt" etc. refer to the US versions.
   nameToUnit.insert(Units::us_gallons->getUnitName(), Units::us_gallons);
   nameToUnit.insert(Units::us_quarts->getUnitName(), Units::us_quarts);
   nameToUnit.insert(Units::us_cups->getUnitName(), Units::us_cups);
   nameToUnit.insert(Units::us_tablespoons->getUnitName(), Units::us_tablespoons);
   nameToUnit.insert(Units::us_teaspoons->getUnitName(), Units::us_teaspoons);

   //nameToUnit.insert(Units::imperial_gallons->getUnitName(), Units::imperial_gallons);
   //nameToUnit.insert(Units::imperial_quarts->getUnitName(), Units::imperial_quarts);
   //nameToUnit.insert(Units::imperial_cups->getUnitName(), Units::imperial_cups);
   //nameToUnit.insert(Units::imperial_tablespoons->getUnitName(), Units::imperial_tablespoons);
   //nameToUnit.insert(Units::imperial_teaspoons->getUnitName(), Units::imperial_teaspoons);

   //nameToUnit.insert(Units::seconds->getUnitName(), Units::seconds);
   //nameToUnit.insert(Units::minutes->getUnitName(), Units::minutes);
   //nameToUnit.insert(Units::hours->getUnitName(), Units::hours);
   //nameToUnit.insert(Units::celsius->getUnitName(), Units::celsius);
   //nameToUnit.insert(Units::kelvin->getUnitName(), Units::kelvin);
   //nameToUnit.insert(Units::fahrenheit->getUnitName(), Units::fahrenheit);

   isMapSetup = true;
}

Unit* USVolumeUnitSystem::getUnit(const QString& name)
{
   if( nameToUnit.count(name) < 1 )
      return 0;
   else
      return nameToUnit[name];
}

Unit* USVolumeUnitSystem::thicknessUnit()
{
   return Units::us_quarts;
}
