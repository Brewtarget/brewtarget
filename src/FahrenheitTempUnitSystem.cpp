/*
 * FahrenheitTempUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "FahrenheitTempUnitSystem.h"
#include <QStringList>

bool FahrenheitTempUnitSystem::isMapSetup = false;

FahrenheitTempUnitSystem::FahrenheitTempUnitSystem()
   : UnitSystem()
{
}

//! scale is ignored here, but must be included for the virtual method
QString FahrenheitTempUnitSystem::displayAmount( double amount, Unit* units, int scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with temperature.
   if( units == 0 || SIUnitName.compare("C") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   ret = QString("%1 %2").arg(Units::fahrenheit->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::fahrenheit->getUnitName());

   return ret;
}

double FahrenheitTempUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();
   return UnitSystem::qstringToSI(qstr,Units::fahrenheit);
}

void FahrenheitTempUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.
   nameToUnit.insert(Units::celsius->getUnitName(), Units::celsius);
   nameToUnit.insert(Units::kelvin->getUnitName(), Units::kelvin);
   nameToUnit.insert(Units::fahrenheit->getUnitName(), Units::fahrenheit);

   isMapSetup = true;
}

