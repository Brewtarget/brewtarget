/*
 * TimeUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "TimeUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

TimeUnitSystem::TimeUnitSystem()
{
   _type = Unit::Time;
}

QMap<Unit::unitScale, Unit*> const& TimeUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleExtraSmall,Units::seconds);
      _scaleToUnit.insert(Unit::scaleSmall,Units::minutes);
      _scaleToUnit.insert(Unit::scaleMedium,Units::hours);
      _scaleToUnit.insert(Unit::scaleLarge,Units::days);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& TimeUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("s", Units::seconds);
      _qstringToUnit.insert("m", Units::minutes);
      _qstringToUnit.insert("h", Units::hours);
      _qstringToUnit.insert("d", Units::days);
   }

   return _qstringToUnit;
}

Unit* TimeUnitSystem::unit() { return Units::minutes; }
QString TimeUnitSystem::unitType() { return "entropy"; }
