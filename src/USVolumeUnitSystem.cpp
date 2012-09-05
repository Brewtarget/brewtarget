/*
 * USVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "USVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>

bool USVolumeUnitSystem::isMapSetup = false;

USVolumeUnitSystem::USVolumeUnitSystem()
{
}

QString USVolumeUnitSystem::displayAmount( double amount, Unit* units, unitScale scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || SIUnitName.compare("L") != 0 )
      return QString("%L1").arg(amount, fieldWidth, format, precision);

   switch(scale)
   {
      case extrasmall:
         ret = QString("%L1 %2").arg(Units::us_teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_teaspoons->getUnitName());
         break;
      case small:
         ret = QString("%L1 %2").arg(Units::us_tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_tablespoons->getUnitName());
         break;
      case medium:
         ret = QString("%L1 %2").arg(Units::us_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_cups->getUnitName());
         break;
      case large:
         ret = QString("%L1 %2").arg(Units::us_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_quarts->getUnitName());
         break;
      case extralarge:
         ret = QString("%L1 %2").arg(Units::us_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_gallons->getUnitName());
         break;
      default:
         if( absSIAmount < Units::us_tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
            ret = QString("%L1 %2").arg(Units::us_teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_teaspoons->getUnitName());
         else if( absSIAmount < Units::us_cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
            ret = QString("%L1 %2").arg(Units::us_tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_tablespoons->getUnitName());
         else if( absSIAmount < Units::us_quarts->toSI(1.0) ) // If less than 1 qt, show us_cups
            ret = QString("%L1 %2").arg(Units::us_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_cups->getUnitName());
         else if( absSIAmount < Units::us_gallons->toSI(1.0) ) // If less than 1 gallon, show us_quarts
            ret = QString("%L1 %2").arg(Units::us_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_quarts->getUnitName());
         else if( absSIAmount < Units::us_barrels->toSI(1.0) ) // If less than 1 barrel, show gallons.
            ret = QString("%L1 %2").arg(Units::us_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_gallons->getUnitName());
         else
            ret = QString("%L1 %2").arg(Units::us_barrels->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::us_barrels->getUnitName());
   }

   return ret;
}

double USVolumeUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   return UnitSystem::qstringToSI(qstr,Units::us_gallons);

}

void USVolumeUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);

   // Assume that "gal" "qt" etc. refer to the US versions.
   nameToUnit.insert(Units::us_barrels->getUnitName(), Units::us_barrels);
   nameToUnit.insert(Units::us_gallons->getUnitName(), Units::us_gallons);
   nameToUnit.insert(Units::us_quarts->getUnitName(), Units::us_quarts);
   nameToUnit.insert(Units::us_cups->getUnitName(), Units::us_cups);
   nameToUnit.insert(Units::us_tablespoons->getUnitName(), Units::us_tablespoons);
   nameToUnit.insert(Units::us_teaspoons->getUnitName(), Units::us_teaspoons);

   isMapSetup = true;
}


Unit* USVolumeUnitSystem::thicknessUnit()
{
   return Units::us_quarts;
}
