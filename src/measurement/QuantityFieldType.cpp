/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/QuantityFieldType.cpp is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#include "measurement/QuantityFieldType.h"

bool IsValid(QuantityFieldType const & fieldType, Measurement::PhysicalQuantity const physicalQuantity) {
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

Measurement::PhysicalQuantities ConvertToPhysicalQuantities(QuantityFieldType const & fieldType) {
   // It's a coding error to call this function if fieldType holds NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(fieldType));

   if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      return std::get<Measurement::PhysicalQuantity>(fieldType);
   }

   Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType));
   return std::get<Measurement::ChoiceOfPhysicalQuantity>(fieldType);
}

QuantityFieldType ConvertToBtFieldType(Measurement::PhysicalQuantities const & physicalQuantities) {
   if (std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities)) {
      return std::get<Measurement::PhysicalQuantity>(physicalQuantities);
   }

   return std::get<Measurement::ChoiceOfPhysicalQuantity>(physicalQuantities);
}
