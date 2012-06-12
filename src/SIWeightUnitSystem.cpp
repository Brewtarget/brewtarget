/*
 * SIWeightUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "SIWeightUnitSystem.h"
#include <QStringList>
#include <cmath>

bool SIWeightUnitSystem::isMapSetup = false;

SIWeightUnitSystem::SIWeightUnitSystem()
   : UnitSystem()
{
}

QString SIWeightUnitSystem::displayAmount( double amount, Unit* units, unitScale scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with mass.
   if( units == 0 || SIUnitName.compare("kg") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   switch(scale)
   {
      case extrasmall:
         ret = QString("%1 %2").arg(Units::milligrams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milligrams->getUnitName());
         break;
      case small:
         ret = QString("%1 %2").arg(Units::grams->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::grams->getUnitName());
         break;
      case medium:
         ret = QString("%1 %2").arg(Units::kilograms->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::kilograms->getUnitName());
         break;
      default:
         if( absSIAmount < Units::grams->toSI(1.0) ) // If less than a gram, show mg.
            ret = QString("%1 %2")
                  .arg(Units::milligrams->fromSI(SIAmount), fieldWidth, format, precision)
                  .arg(Units::milligrams->getUnitName());
         else if( absSIAmount < Units::kilograms->toSI(1.0) ) // If less than a kg, show g.
            ret = QString("%1 %2")
                  .arg(Units::grams->fromSI(SIAmount), fieldWidth, format, precision)
                  .arg(Units::grams->getUnitName());
         else // Otherwise, show kg.
            ret = QString("%1 %2")
               .arg(Units::kilograms->fromSI(SIAmount), fieldWidth, format, precision)
               .arg(Units::kilograms->getUnitName());
   }

   return ret;
}

double SIWeightUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   return UnitSystem::qstringToSI(qstr, Units::kilograms);
}

void SIWeightUnitSystem::ensureMapIsSetup()
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

Unit* SIWeightUnitSystem::thicknessUnit()
{
   return Units::kilograms;
}
