/*
 * SIVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "SIVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>

bool SIVolumeUnitSystem::isMapSetup = false;

SIVolumeUnitSystem::SIVolumeUnitSystem()
   : UnitSystem()
{
}

QString SIVolumeUnitSystem::displayAmount( double amount, Unit* units, int scale )
{
   QString SIUnitName = units->getSIUnitName();
   double SIAmount = units->toSI( amount );
   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || SIUnitName.compare("L") != 0 )
      return QString("%1").arg(amount, fieldWidth, format, precision);

   if( qAbs(SIAmount) < Units::liters->toSI(1.0) ) // If less than 1 L, display mL.
      ret = QString("%1 %2").arg(Units::milliliters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::milliliters->getUnitName());
   else // Otherwise, display liters.
      ret = QString("%1 %2").arg(Units::liters->fromSI(SIAmount), fieldWidth, format, precision).arg(Units::liters->getUnitName());

   return ret;
}

double SIVolumeUnitSystem::qstringToSI( QString qstr )
{
   ensureMapIsSetup();

   return UnitSystem::qstringToSI(qstr, Units::liters);
}

void SIVolumeUnitSystem::ensureMapIsSetup()
{
   // If it is setup, return now.
   if( isMapSetup )
      return;

   // Ok, map was not setup, so set it up.
   nameToUnit.insert(Units::liters->getUnitName(), Units::liters);
   nameToUnit.insert(Units::milliliters->getUnitName(), Units::milliliters);

   // Assume that "gal" "qt" etc. refer to the US versions.
   nameToUnit.insert(Units::us_gallons->getUnitName(), Units::us_gallons);
   nameToUnit.insert(Units::us_quarts->getUnitName(), Units::us_quarts);
   nameToUnit.insert(Units::us_cups->getUnitName(), Units::us_cups);
   nameToUnit.insert(Units::us_tablespoons->getUnitName(), Units::us_tablespoons);
   nameToUnit.insert(Units::us_teaspoons->getUnitName(), Units::us_teaspoons);

   isMapSetup = true;
}

Unit* SIVolumeUnitSystem::thicknessUnit()
{
   return Units::liters;
}
