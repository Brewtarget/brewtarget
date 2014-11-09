/*
 * UnitSystem.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _UNITSYSTEM_H
#define _UNITSYSTEM_H

class UnitSystem;
class UnitSystems;

#include <QString>
#include <QRegExp>
#include "unit.h"

/*!
 * \class UnitSystem
 * \author Philip G. Lee
 *
 * \brief A unit system handles the display and format of physical quantities.
 */
class UnitSystem
{
public:
   UnitSystem();
   virtual ~UnitSystem() {}

   /*!
    * displayAmount() should return a string appropriately displaying
    * 'amount' of type 'units' in this UnitSystem. This string should also
    * be recognized by qstringToSI()
    */
   QString displayAmount( double amount, Unit* units, unitScale scale = noScale );

   /*!
    * qstringToSI() should convert 'qstr' (consisting of a decimal amount,
    * followed by a unit string) to the appropriate SI amount under this
    * UnitSystem.
    */
   double qstringToSI(QString qstr, Unit* defUnit = 0, bool force = false);

   /*!
    * Returns the unit associated with thickness. If this unit system is
    * US weight, it would return lb. If it were US volume, it would return
    * quarts.
    */
   virtual Unit* thicknessUnit() = 0;
   virtual Unit* unit() = 0;
   virtual void  loadMap() = 0;

   // \brief Returns the name of the unit
   virtual QString unitType() = 0;


protected:
   static const int fieldWidth;
   static const char format;
   static const int precision;

   UnitType _type;
   QRegExp amtUnit;

   QMap<unitScale, Unit*> scaleToUnit;

};

#endif /*_UNITSYSTEM_H*/
