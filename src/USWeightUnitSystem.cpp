/*
 * USWeightUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "USWeightUnitSystem.h"
#include <QStringList>
#include <cmath>

bool USWeightUnitSystem::isMapSetup = false;

USWeightUnitSystem::USWeightUnitSystem()
{
}

QString USWeightUnitSystem::displayAmount( double amount, Unit* units, int scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with mass.
   if( units == 0 || SIUnitName.compare("kg") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   switch(scale)
   {
      case extrasmall:
         ret = QString("%1 %2").arg(Units::ounces->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::ounces->getUnitName());
         break;
      case small:
         ret = QString("%1 %2").arg(Units::pounds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::pounds->getUnitName());
         break;
      default:
         if( qAbs(SIAmount) < Units::pounds->toSI(1.0) ) // If less than 1 pound, display ounces.
            ret = QString("%1 %2").arg(Units::ounces->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::ounces->getUnitName());
         else // Otherwise, display pounds.
            ret = QString("%1 %2").arg(Units::pounds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::pounds->getUnitName());
   }

   return ret;
}

double USWeightUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   return UnitSystem::qstringToSI(qstr,Units::pounds);
}

void USWeightUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.
   nameToUnit.insert(Units::kilograms->getUnitName(), Units::kilograms);
   nameToUnit.insert(Units::grams->getUnitName(), Units::grams);
   nameToUnit.insert(Units::milligrams->getUnitName(), Units::milligrams);

   nameToUnit.insert(Units::pounds->getUnitName(), Units::pounds);
   nameToUnit.insert(Units::ounces->getUnitName(), Units::ounces);

   isMapSetup = true;
}

Unit* USWeightUnitSystem::thicknessUnit()
{
   return Units::pounds;
}
