/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BtFieldType.h is part of Brewtarget, and is copyright the following authors 2022-2023:
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
#ifndef BTFIELDTYPE_H
#define BTFIELDTYPE_H
#pragma once

#include <variant>

#include <QString>

#include "measurement/PhysicalQuantity.h"

/**
 * \brief The types of value other than \c Measurement::PhysicalQuantity that can be shown in a UI field.
 *
 *        Note that there is intentionally \b no value here for \c none or similar.
 *
 *        See comment in \c measurement/PhysicalQuantity.h for why \c Count is a \c PhysicalQuantity rather than a
 *        \c NonPhysicalQuantity.
 */
enum class NonPhysicalQuantity {
   Date,
   String,
   Percentage,
   Bool,
   //! This usually means we need a \c BtComboBox
   Enum,
   /**
    * This is for things like step number and other orderings where we have 1st, 2nd, 3rd etc (though its usually
    * presented as step #1, step #2, step #3, etc) and should usually be stored in an int (or unsigned int).
    *
    * This is also used for "number of times" something has been done or is to be done.  Eg number of times a yeast
    * sample has been cultured or reused.  Similarly, it's used for the number of fermentation stages in a Recipe.
    */
   OrdinalNumeral,
   /**
    * \brief This is for a number that has no units, not even pseudo ones.  It is currently a bit over-used -- ie there
    *        are places we are using this where we probably should be using a \c PhysicalQuantity.  We should fix these
    *        over time.
    */
   Dimensionless,
};

/**
 * \brief Return the (untranslated) name of a \c NonPhysicalQuantity suitable for logging
 */
QString GetLoggableName(NonPhysicalQuantity nonPhysicalQuantity);

/**
 * \brief All types of value that can be shown in a UI field
 */
typedef std::variant<Measurement::PhysicalQuantity,
                     Measurement::ChoiceOfPhysicalQuantity,
                     NonPhysicalQuantity> BtFieldType;

/**
 * \brief If you know a \c BtFieldType does \b not contain a \c NonPhysicalQuantity, then this will convert it into
 *        a Measurement::PhysicalQuantities
 */
Measurement::PhysicalQuantities ConvertToPhysicalQuantities(BtFieldType const & btFieldType);

/**
 * \brief If you have a \c Measurement::PhysicalQuantities variant and need to convert it to a \c BtFieldType variant,
 *        this is the function to call.
 */
BtFieldType ConvertToBtFieldType(Measurement::PhysicalQuantities const & physicalQuantities);

/**
 * \brief Convenience function to allow output of \c BtFieldType to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class.)
 */
template<class S>
S & operator<<(S & stream, BtFieldType fieldType) {
   if (std::holds_alternative<NonPhysicalQuantity>(fieldType)) {
      stream << "BtFieldType: NonPhysicalQuantity:" << GetLoggableName(std::get<NonPhysicalQuantity>(fieldType));
   } else if (std::holds_alternative<Measurement::PhysicalQuantity>(fieldType)) {
      stream << "BtFieldType: PhysicalQuantity:" << std::get<Measurement::PhysicalQuantity>(fieldType);
   } else {
      Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(fieldType));
      stream << "BtFieldType: ChoiceOfPhysicalQuantity:" << std::get<Measurement::ChoiceOfPhysicalQuantity>(fieldType);
   }
   return stream;
}

#endif
