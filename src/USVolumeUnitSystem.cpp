/*
 * USVolumeUnitSystem.cpp is part of Brewtarget, and is Copyright the following
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

#include "USVolumeUnitSystem.h"
#include <QStringList>
#include <cmath>
#include "unit.h"

USVolumeUnitSystem::USVolumeUnitSystem()
{
   _type = Volume;
}

void USVolumeUnitSystem::loadMap()
{
   scaleToUnit.insert(scaleExtraSmall,Units::us_teaspoons);
   scaleToUnit.insert(scaleSmall,Units::us_tablespoons);
   scaleToUnit.insert(scaleMedium,Units::us_cups);
   scaleToUnit.insert(scaleLarge,Units::us_quarts);
   scaleToUnit.insert(scaleExtraLarge,Units::us_gallons);
   scaleToUnit.insert(scaleHuge,Units::us_barrels);
}

Unit* USVolumeUnitSystem::thicknessUnit()
{
   return Units::us_quarts;
}

Unit* USVolumeUnitSystem::unit() { return Units::us_gallons; };
QString USVolumeUnitSystem::unitType() { return "USCustomary"; }
