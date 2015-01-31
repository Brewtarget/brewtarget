/*
 * EbcColorUnitSystem.cpp is part of Brewtarget, and was written by Mik
 * Firestone (mikfire@gmail.com), copyright 2014-2019
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

#include "EbcColorUnitSystem.h"
#include <QStringList>
#include "unit.h"

EbcColorUnitSystem::EbcColorUnitSystem()
   : UnitSystem()
{
   _type = Color;
}

void EbcColorUnitSystem::loadMap() { scaleToUnit.insert(scaleWithout, Units::ebc); }
void EbcColorUnitSystem::loadUnitmap() { qstringToUnit.insert("ebc", Units::ebc); }
QString EbcColorUnitSystem::unitType() { return "Color"; }
Unit* EbcColorUnitSystem::unit() { return Units::ebc; }
