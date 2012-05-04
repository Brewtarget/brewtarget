/*
 * TimeUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "TimeUnitSystem.h"
#include <QStringList>
#include <cmath>

bool TimeUnitSystem::isMapSetup = false;

TimeUnitSystem::TimeUnitSystem()
{
}

QString TimeUnitSystem::displayAmount( double amount, Unit* units, int scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with time.
   if( units == 0 || SIUnitName.compare("min") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   if( absSIAmount < Units::minutes->toSI(1.0) ) // Less than a minute, show seconds.
      ret = QString("%1 %2").arg(Units::seconds->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::seconds->getUnitName());
   else if( absSIAmount < Units::hours->toSI(2.0) ) // Less than two hours, show minutes.
      ret = QString("%1 %2").arg(Units::minutes->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::minutes->getUnitName());
   else if( absSIAmount < Units::days->toSI(1.0) )// Show hours.
      ret = QString("%1 %2").arg(Units::hours->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::hours->getUnitName());
   else
      ret = QString("%1 %2").arg(Units::days->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::days->getUnitName());

   return ret;
}

double TimeUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   return UnitSystem::qstringToSI(qstr,Units::minutes);
}

void TimeUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.

   nameToUnit.insert(Units::seconds->getUnitName(), Units::seconds);
   nameToUnit.insert(Units::minutes->getUnitName(), Units::minutes);
   nameToUnit.insert(Units::hours->getUnitName(), Units::hours);
   nameToUnit.insert(Units::days->getUnitName(), Units::days);

   isMapSetup = true;
}

