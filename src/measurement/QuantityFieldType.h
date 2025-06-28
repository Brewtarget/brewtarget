/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/QuantityFieldType.h is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#ifndef MEASUREMENT_QUANTITYFIELDTYPE_H
#define MEASUREMENT_QUANTITYFIELDTYPE_H
#pragma once

#include <variant>

#include "measurement/PhysicalQuantity.h"
#include "measurement/NonPhysicalQuantity.h"

/**
 * \brief All types of value that can be shown in a UI field
 */
typedef std::variant<Measurement::PhysicalQuantity,
                     Measurement::ChoiceOfPhysicalQuantity,
                     NonPhysicalQuantity> QuantityFieldType;

/**
 * \brief Returns \c true if \c physicalQuantity is compatible with the supplied \c fieldType, or \c false otherwise
 */
bool IsValid(QuantityFieldType const & fieldType, Measurement::PhysicalQuantity const physicalQuantity);

/**
 * \brief If you know a \c QuantityFieldType does \b not contain a \c NonPhysicalQuantity, then this will convert it into
 *        a Measurement::PhysicalQuantities
 */
Measurement::PhysicalQuantities ConvertToPhysicalQuantities(QuantityFieldType const & fieldType);

/**
 * \brief If you have a \c Measurement::PhysicalQuantities variant and need to convert it to a \c QuantityFieldType variant,
 *        this is the function to call.
 */
QuantityFieldType ConvertToBtFieldType(Measurement::PhysicalQuantities const & physicalQuantities);

/**
 * \brief Convenience function to allow output of \c QuantityFieldType to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class.)
 */
template<class S>
S & operator<<(S & stream, QuantityFieldType fieldType) {
   if (std::holds_alternative<NonPhysicalQuantity>(fieldType)) {
      stream << "QuantityFieldType: NonPhysicalQuantity:" << GetLoggableName(std::get<NonPhysicalQuantity>(fieldType));
   } else if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      stream << "QuantityFieldType: PhysicalQuantity:" << std::get<Measurement::PhysicalQuantity>(fieldType);
   } else {
      Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType));
      stream << "QuantityFieldType: ChoiceOfPhysicalQuantity:" << std::get<Measurement::ChoiceOfPhysicalQuantity>(fieldType);
   }
   return stream;
}

#endif
