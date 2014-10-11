/*
 * CelsiusTempUnitSystem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

CelsiusTempUnitSystem::CelsiusTempUnitSystem()
   : UnitSystem()
{
   _type = Temp;
}

void CelsiusTempUnitSystem::loadMap()
{
   scaleToUnit.insert(without,Units::celsius);
}

QString CelsiusTempUnitSystem::unitType() { return "SI"; }
