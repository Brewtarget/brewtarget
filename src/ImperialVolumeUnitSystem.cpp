/*
 * ImperialVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "ImperialVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>

bool ImperialVolumeUnitSystem::isMapSetup = false;

ImperialVolumeUnitSystem::ImperialVolumeUnitSystem()
   : UnitSystem()
{
}

QString ImperialVolumeUnitSystem::displayAmount( double amount, Unit* units, unitScale scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || SIUnitName.compare("L") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   // The scale needs to override everything. If I select "imperial
   // teaspoons", I don't care how big or small the value it. Display it in
   // that scale
   switch( scale ) 
   {
      case extrasmall:
         ret = QString("%1 %2").arg(Units::imperial_teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_teaspoons->getUnitName());
         break;
      case small:
         ret = QString("%1 %2").arg(Units::imperial_tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_tablespoons->getUnitName());
         break;
      case medium:
         ret = QString("%1 %2").arg(Units::imperial_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_cups->getUnitName());
         break;
      case large:
         ret = QString("%1 %2").arg(Units::imperial_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_quarts->getUnitName());
         break;
      case extralarge:
         ret = QString("%1 %2").arg(Units::imperial_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_gallons->getUnitName());
         break;
      default:
         if( absSIAmount < Units::imperial_tablespoons->toSI(1.0) ) // If less than 1 tbsp, show tsp
            ret = QString("%1 %2").arg(Units::imperial_teaspoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_teaspoons->getUnitName());
         else if( absSIAmount < Units::imperial_cups->toSI(0.25) ) // If less than 1/4 cup, show tbsp
            ret = QString("%1 %2").arg(Units::imperial_tablespoons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_tablespoons->getUnitName());
         else if( absSIAmount < Units::imperial_quarts->toSI(1.0) ) // If less than 1 qt, show imperial_cups
            ret = QString("%1 %2").arg(Units::imperial_cups->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_cups->getUnitName());
         else if( absSIAmount < Units::imperial_gallons->toSI(1.0) ) // If less than 1 gallon, show imperial_quarts
            ret = QString("%1 %2").arg(Units::imperial_quarts->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_quarts->getUnitName());
         else
            ret = QString("%1 %2").arg(Units::imperial_gallons->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::imperial_gallons->getUnitName());
   }

   return ret;
}

double ImperialVolumeUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();
   return UnitSystem::qstringToSI(qstr,Units::imperial_gallons);
}

void ImperialVolumeUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.

   nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);


   nameToUnit.insert(Units::imperial_gallons->getUnitName(), Units::imperial_gallons);
   nameToUnit.insert(Units::imperial_quarts->getUnitName(), Units::imperial_quarts);
   nameToUnit.insert(Units::imperial_cups->getUnitName(), Units::imperial_cups);
   nameToUnit.insert(Units::imperial_tablespoons->getUnitName(), Units::imperial_tablespoons);
   nameToUnit.insert(Units::imperial_teaspoons->getUnitName(), Units::imperial_teaspoons);

   isMapSetup = true;
}

Unit* ImperialVolumeUnitSystem::thicknessUnit()
{
   return Units::imperial_quarts;
}
