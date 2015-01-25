/*
 * ImperialVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "ImperialVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

ImperialVolumeUnitSystem::ImperialVolumeUnitSystem()
   : UnitSystem()
{
   _type = Volume;
}

void ImperialVolumeUnitSystem::loadMap()
{
   scaleToUnit.insert(scaleExtraSmall,Units::imperial_teaspoons);
   scaleToUnit.insert(scaleSmall,Units::imperial_tablespoons);
   scaleToUnit.insert(scaleMedium,Units::imperial_cups);
   scaleToUnit.insert(scaleLarge,Units::imperial_quarts);
   scaleToUnit.insert(scaleExtraLarge,Units::imperial_gallons);
   scaleToUnit.insert(scaleHuge,Units::imperial_barrels);
}

Unit* ImperialVolumeUnitSystem::thicknessUnit()
{
   return Units::imperial_quarts;
}

Unit* ImperialVolumeUnitSystem::unit() { return Units::imperial_gallons; };
QString ImperialVolumeUnitSystem::unitType() { return "Imperial"; }
