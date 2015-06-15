/*
 * CelsiusTempUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "CelsiusTempUnitSystem.h"
#include "unit.h"
#include <QStringList>

CelsiusTempUnitSystem::CelsiusTempUnitSystem() :
   UnitSystem()
{
   _type = Unit::Temp;
}

QMap<Unit::unitScale, Unit*> const& CelsiusTempUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleWithout,Units::celsius);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& CelsiusTempUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("C",Units::celsius);
   }

   return _qstringToUnit;
}

Unit* CelsiusTempUnitSystem::unit() { return Units::celsius; }

QString CelsiusTempUnitSystem::unitType() { return "SI"; }
