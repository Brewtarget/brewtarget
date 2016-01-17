/*
 * SIWeightUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "SIWeightUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

SIWeightUnitSystem::SIWeightUnitSystem()
   : UnitSystem()
{
   _type = Unit::Mass;
}

QMap<Unit::unitScale, Unit*> const& SIWeightUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleExtraSmall,Units::milligrams);
      _scaleToUnit.insert(Unit::scaleSmall, Units::grams);
      _scaleToUnit.insert(Unit::scaleMedium, Units::kilograms);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& SIWeightUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("mg", Units::milligrams);
      _qstringToUnit.insert( "g", Units::grams);
      _qstringToUnit.insert("kg", Units::kilograms);
   }

   return _qstringToUnit;
}

Unit* SIWeightUnitSystem::thicknessUnit()
{
   return Units::kilograms;
}

Unit* SIWeightUnitSystem::unit() { return Units::kilograms; }
QString SIWeightUnitSystem::unitType() { return "SI"; }
