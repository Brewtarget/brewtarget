/*
 * UnitSystem.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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

   double qstringToSI(QString qstr, Unit* defUnit);

protected:
   const static int fieldWidth = 0;
   const static char format = 'f';
   const static int precision = 3;
   static QMap<QString, Unit*> nameToUnit;
   QRegExp amtUnit;

   Unit* getUnit(const QString& name);

};

#endif /*_UNITSYSTEM_H*/
