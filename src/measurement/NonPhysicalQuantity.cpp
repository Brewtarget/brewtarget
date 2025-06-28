/*======================================================================================================================
 * measurement/NonPhysicalQuantity.cpp is part of Brewtarget, and is copyright the following authors 2022-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 =====================================================================================================================*/
#include "measurement/NonPhysicalQuantity.h"

#include <QDebug>

QString GetLoggableName(NonPhysicalQuantity nonPhysicalQuantity) {
   // See comment in measurement/PhysicalQuantity.cpp for why we use a switch and not an EnumStringMapping here
   switch (nonPhysicalQuantity) {
      case NonPhysicalQuantity::Date           : return "Date"          ;
      case NonPhysicalQuantity::String         : return "String"        ;
      case NonPhysicalQuantity::Percentage     : return "Percentage"    ;
      case NonPhysicalQuantity::Bool           : return "Bool"          ;
      case NonPhysicalQuantity::Enum           : return "Enum"          ;
      case NonPhysicalQuantity::OrdinalNumeral : return "OrdinalNumeral";
      case NonPhysicalQuantity::CardinalNumber : return "CardinalNumber";
      case NonPhysicalQuantity::Dimensionless  : return "Dimensionless" ;
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here!
   Q_ASSERT(false);
}
