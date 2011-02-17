/*
 * UnitSystems.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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

class USWeightUnitSystem;
class SIWeightUnitSystem;
class ImperialVolumeUnitSystem;
class USVolumeUnitSystem;
class SIVolumeUnitSystem;
class CelsiusTempUnitSystem;
class FahrenheitTempUnitSystem;
class TimeUnitSystem;

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

