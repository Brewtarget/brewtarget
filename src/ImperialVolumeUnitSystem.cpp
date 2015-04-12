/*
 * ImperialVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ImperialVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

ImperialVolumeUnitSystem::ImperialVolumeUnitSystem()
   : UnitSystem()
{
   _type = Volume;
}

QMap<unitScale, Unit*> const& ImperialVolumeUnitSystem::scaleToUnit()
{
   static QMap<unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(scaleExtraSmall,Units::imperial_teaspoons);
      _scaleToUnit.insert(scaleSmall,Units::imperial_tablespoons);
      _scaleToUnit.insert(scaleMedium,Units::imperial_cups);
      _scaleToUnit.insert(scaleLarge,Units::imperial_quarts);
      _scaleToUnit.insert(scaleExtraLarge,Units::imperial_gallons);
      _scaleToUnit.insert(scaleHuge,Units::imperial_barrels);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& ImperialVolumeUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("tsp",Units::imperial_teaspoons);
      _qstringToUnit.insert("tbs",Units::imperial_tablespoons);
      _qstringToUnit.insert("cup",Units::imperial_cups);
      _qstringToUnit.insert("qt",Units::imperial_quarts);
      _qstringToUnit.insert("gal",Units::imperial_gallons);
      _qstringToUnit.insert("bbl",Units::imperial_barrels);
   }

   return _qstringToUnit;
}

Unit* ImperialVolumeUnitSystem::thicknessUnit()
{
   return Units::imperial_quarts;
}

Unit* ImperialVolumeUnitSystem::unit() { return Units::imperial_gallons; };
QString ImperialVolumeUnitSystem::unitType() { return "Imperial"; }
