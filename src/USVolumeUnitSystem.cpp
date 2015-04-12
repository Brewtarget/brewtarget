/*
 * USVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "USVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

USVolumeUnitSystem::USVolumeUnitSystem()
{
   _type = Volume;
}

QMap<unitScale, Unit*> const& USVolumeUnitSystem::scaleToUnit()
{
   static QMap<unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(scaleExtraSmall,Units::us_teaspoons);
      _scaleToUnit.insert(scaleSmall,Units::us_tablespoons);
      _scaleToUnit.insert(scaleMedium,Units::us_cups);
      _scaleToUnit.insert(scaleLarge,Units::us_quarts);
      _scaleToUnit.insert(scaleExtraLarge,Units::us_gallons);
      _scaleToUnit.insert(scaleHuge,Units::us_barrels);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& USVolumeUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("tsp",Units::us_teaspoons);
      _qstringToUnit.insert("tbs",Units::us_tablespoons);
      _qstringToUnit.insert("cup",Units::us_cups);
      _qstringToUnit.insert("qt",Units::us_quarts);
      _qstringToUnit.insert("gal",Units::us_gallons);
      _qstringToUnit.insert("bbl",Units::us_barrels);
   }

   return _qstringToUnit;
}

Unit* USVolumeUnitSystem::thicknessUnit()
{
   return Units::us_quarts;
}

Unit* USVolumeUnitSystem::unit() { return Units::us_gallons; };

QString USVolumeUnitSystem::unitType() { return "USCustomary"; }
