/*
 * DiastaticPowerUnitSystem.cpp is part of Brewtarget, and was written by
 * Mark de Wever (koraq@xs4all.nl), copyright 2016
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

#include "DiastaticPowerUnitSystem.h"
#include <QStringList>
#include "unit.h"

LintnerDiastaticPowerUnitSystem::LintnerDiastaticPowerUnitSystem()
   : UnitSystem()
{
   _type = Unit::DiastaticPower;
}

QMap<Unit::unitScale, Unit*> const& LintnerDiastaticPowerUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleWithout, Units::lintner);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& LintnerDiastaticPowerUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("lintner", Units::lintner);
   }

   return _qstringToUnit;
}

QString LintnerDiastaticPowerUnitSystem::unitType() { return "DiastaticPower"; }
Unit* LintnerDiastaticPowerUnitSystem::unit() { return Units::lintner; }


WkDiastaticPowerUnitSystem::WkDiastaticPowerUnitSystem()
   : UnitSystem()
{
   _type = Unit::DiastaticPower;
}

QMap<Unit::unitScale, Unit*> const& WkDiastaticPowerUnitSystem::scaleToUnit()
{
   static QMap<Unit::unitScale, Unit*> _scaleToUnit;
   if( _scaleToUnit.empty() )
   {
      _scaleToUnit.insert(Unit::scaleWithout, Units::wk);
   }

   return _scaleToUnit;
}

QMap<QString, Unit*> const& WkDiastaticPowerUnitSystem::qstringToUnit()
{
   static QMap<QString, Unit*> _qstringToUnit;
   if( _qstringToUnit.empty() )
   {
      _qstringToUnit.insert("wk", Units::wk);
   }

   return _qstringToUnit;
}

QString WkDiastaticPowerUnitSystem::unitType() { return "DiastaticPower"; }
Unit* WkDiastaticPowerUnitSystem::unit() { return Units::wk; }

