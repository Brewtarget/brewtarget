/*
 * UnitSystem.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2010.
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

#ifndef _UNITSYSTEM_H
#define _UNITSYSTEM_H

class UnitSystem;
class UnitSystems;

#include <QString>
#include "unit.h"

class UnitSystem
{
public:
   /*!
    * displayAmount() should return a string appropriately displaying
    * 'amount' of type 'units' in this UnitSystem. This string should also
    * be recognized by qstringToSI()
    */
   virtual QString displayAmount( double amount, Unit* units = 0 ) = 0;

   /*!
    * qstringToSI() should convert 'qstr' (consisting of a decimal amount,
    * followed by a unit string) to the appropriate SI amount under this
    * UnitSystem.
    */
   virtual double qstringToSI( QString qstr ) = 0;

   /*!
    * Returns the unit associated with thickness. If this unit system is
    * US weight, it would return lb. If it were US volume, it would return
    * quarts.
    */
   virtual Unit* thicknessUnit() = 0;

protected:
   static const int fieldWidth = 0;
   static const char format = 'f';
   static const int precision = 3;
};

#include "USWeightUnitSystem.h"
#include "SIWeightUnitSystem.h"
#include "ImperialVolumeUnitSystem.h"
#include "USVolumeUnitSystem.h"
#include "SIVolumeUnitSystem.h"
#include "CelsiusTempUnitSystem.h"
#include "FahrenheitTempUnitSystem.h"
#include "TimeUnitSystem.h"

class UnitSystems
{
public:
   static USWeightUnitSystem* usWeightUnitSystem();
   static SIWeightUnitSystem* siWeightUnitSystem();

   static ImperialVolumeUnitSystem* imperialVolumeUnitSystem();
   static USVolumeUnitSystem* usVolumeUnitSystem();
   static SIVolumeUnitSystem* siVolumeUnitSystem();

   static CelsiusTempUnitSystem* celsiusTempUnitSystem();
   static FahrenheitTempUnitSystem* fahrenheitTempUnitSystem();

   static TimeUnitSystem* timeUnitSystem();
};

#endif /*_UNITSYSTEM_H*/
