/*
 * BtFieldType.h is part of Brewtarget, and is copyright the following
 * authors 2022:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef BTFIELDTYPE_H
#define BTFIELDTYPE_H
#pragma once

#include <variant>

#include "measurement/PhysicalQuantity.h"

/**
 * \brief The types of value other than \c Measurement::PhysicalQuantity that can be shown in a UI field
 *
 *        Note that there is intentionally \b no value here for \c none or similar.
 */
enum class NonPhysicalQuantity {
   Date,
   String,
   Count,
   Percentage
};

/**
 * \brief Return the name of a \c NonPhysicalQuantity suitable either for display to the user or logging
 */
QString GetDisplayName(NonPhysicalQuantity nonPhysicalQuantity);

/**
 * \brief All types of value that can be shown in a UI field
 */
typedef std::variant<Measurement::PhysicalQuantity, NonPhysicalQuantity> BtFieldType;

/**
 * \brief Convenience function to allow output of \c BtFieldType to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class.)
 */
template<class S>
S & operator<<(S & stream, BtFieldType fieldType) {
   if (std::holds_alternative<NonPhysicalQuantity>(fieldType)) {
      stream << "NonPhysicalQuantity:" << GetDisplayName(std::get<NonPhysicalQuantity>(fieldType));
   } else {
      stream << "PhysicalQuantity:" << Measurement::getDisplayName(std::get<Measurement::PhysicalQuantity>(fieldType));
   }
   return stream;
}

#endif
