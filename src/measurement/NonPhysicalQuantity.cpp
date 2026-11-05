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
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

std::optional<int> GetUltimateDefaultDecimalPlaces(NonPhysicalQuantity const nonPhysicalQuantity) {
   switch (nonPhysicalQuantity) {
      case NonPhysicalQuantity::Date           : return std::nullopt;
      case NonPhysicalQuantity::String         : return std::nullopt;
      case NonPhysicalQuantity::Percentage     : return 1;
      case NonPhysicalQuantity::Bool           : return std::nullopt;
      case NonPhysicalQuantity::Enum           : return std::nullopt;
      case NonPhysicalQuantity::OrdinalNumeral : return std::nullopt;
      case NonPhysicalQuantity::CardinalNumber : return std::nullopt;
      case NonPhysicalQuantity::Dimensionless  : return std::nullopt;
      case NonPhysicalQuantity::Currency       : return std::nullopt;
      case NonPhysicalQuantity::Objects        : return std::nullopt;
         // As usual, we don't want a default case here as we want the compiler to warn us if we missed a physical quantity
      default: std::unreachable();
   }
   // It's a coding error if we get here!
   Q_UNREACHABLE();
}

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
      case NonPhysicalQuantity::Currency       : return "Currency"      ;
      case NonPhysicalQuantity::Objects        : return "Objects"       ;
      // As usual, we don't want a default case here as we want the compiler to warn us if we missed a physical quantity
      default: std::unreachable();
   }
   // It's a coding error if we get here!
   Q_UNREACHABLE();
}