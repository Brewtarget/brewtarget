/*
 * SrmColorUnitSystem.cpp is part of Brewtarget, and was written by Mik
 * Firestone (mikfire@gmail.com), copyright 2014-2015
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

#include "SrmColorUnitSystem.h"
#include <QStringList>

SrmColorUnitSystem::SrmColorUnitSystem()
   : UnitSystem()
{
   _type = Unit::Color;
}

QMap<Unit::unitScale, Unit*> const& SrmColorUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleWithout,Units::srm);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& SrmColorUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("srm",Units::srm);
   }

   return _qstringToUnit;
}

QString SrmColorUnitSystem::unitType() { return "Color"; }
Unit* SrmColorUnitSystem::unit() { return Units::srm; }
