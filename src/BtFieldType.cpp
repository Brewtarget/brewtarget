/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtFieldType.cpp is part of Brewtarget, and is copyright the following authors 2022-2025:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "BtFieldType.h"

#include <QDebug>

#include "utils/EnumStringMapping.h"

QString GetLoggableName(NonPhysicalQuantity nonPhysicalQuantity) {
   // See comment in measurement/PhysicalQuantity.cpp for why we use a switch and not an EnumStringMapping here
   switch (nonPhysicalQuantity) {
      case NonPhysicalQuantity::Date           : return "Date"          ;
      case NonPhysicalQuantity::String         : return "String"        ;
      case NonPhysicalQuantity::Percentage     : return "Percentage"    ;
      case NonPhysicalQuantity::Bool           : return "Bool"          ;
      case NonPhysicalQuantity::Enum           : return "Enum"          ;
      case NonPhysicalQuantity::OrdinalNumeral : return "OrdinalNumeral";
      case NonPhysicalQuantity::Dimensionless  : return "Dimensionless" ;
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here!
   Q_ASSERT(false);
}

bool IsValid(BtFieldType const & fieldType, Measurement::PhysicalQuantity const physicalQuantity) {
   // It's a coding error to call this function if fieldType holds NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(fieldType));

   if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType)) {
      auto const choiceOfPhysicalQuantity = std::get<Measurement::ChoiceOfPhysicalQuantity>(fieldType);
      return Measurement::isValid(choiceOfPhysicalQuantity, physicalQuantity);
   }

   // Only one possibility left
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(fieldType));
   return physicalQuantity == std::get<Measurement::PhysicalQuantity>(fieldType);
}

Measurement::PhysicalQuantities ConvertToPhysicalQuantities(BtFieldType const & fieldType) {
   // It's a coding error to call this function if fieldType holds NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(fieldType));

   if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      return std::get<Measurement::PhysicalQuantity>(fieldType);
   }

   Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType));
   return std::get<Measurement::ChoiceOfPhysicalQuantity>(fieldType);
}

BtFieldType ConvertToBtFieldType(Measurement::PhysicalQuantities const & physicalQuantities) {
   if (std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities)) {
      return std::get<Measurement::PhysicalQuantity>(physicalQuantities);
   }

   return std::get<Measurement::ChoiceOfPhysicalQuantity>(physicalQuantities);
}
