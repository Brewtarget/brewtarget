/*
 * PlatoDensityUnitSystem.cpp is part of Brewtarget, and was written by Mik
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

#include "PlatoDensityUnitSystem.h"
#include <QStringList>
#include "unit.h"

PlatoDensityUnitSystem::PlatoDensityUnitSystem()
   : UnitSystem()
{
   _type = Unit::Density;
}

QMap<Unit::unitScale, Unit*> const& PlatoDensityUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleWithout, Units::plato);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& PlatoDensityUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("P", Units::plato);
   }

   return _qstringToUnit;
}


QString PlatoDensityUnitSystem::unitType() { return "Density"; }
Unit* PlatoDensityUnitSystem::unit() { return Units::plato; }
