/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/EnumStringMapping.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef UTILS_ENUMSTRINGMAPPING_H
#define UTILS_ENUMSTRINGMAPPING_H
#pragma once

#include <optional>
#include <stdexcept>

#include <QDebug>
#include <QString>
#include <QVector>

#include "Logging.h"
#include "utils/EnumMapping.h"
#include "utils/TypeLookup.h"

using EnumAndItsString = EnumKeyValue<QString>;

/**
 * \class EnumStringMapping
 *
 *        We don't actually bother creating hashmaps or similar between enum values and string representations
 *        because it's usually going to be a short list that we can search through pretty quickly (probably faster
 *        than calculating the hash of a key!)
 *
 *        In theory, it would suffice to use an array here rather than a vector, because the data for the mapping is
 *        always known at compile-time.  This would also allow us to make more things const or constexpr.  In practice,
 *        it's a bit clunky using arrays (eg length of std::array is a template parameter that cannot always be deduced
 *        by the compiler when it's an array of structs), and the extra "cost" of a vector here is negligible.
 *
 *        TBD: Could look at Boost Bimap
 *
 *        NOTE: We can get the number of values in the mapping by calling \c size (inherited from \c QVector).  By
 *              virtue of the fact that we always start our enums at 0 and don't skip any values, this also tells us
 *              the number of values in the enum (subject to the assumption that every value in the enum was given a
 *              mapping entry).  Use \c FlagEnumStringMapping for enums such as \c Ingredient::Measure where this is not
 *              the case.
 */
class EnumStringMapping : public EnumMapping<QString> {
public:
   using EnumMapping::EnumMapping;

   /**
    * \brief Convert data that \b might not be a valid string representation of an enum to the \c int value of that
    *        enum.  This is useful when we are deserialising a file for instance
    * \param stringValue
    * \param caseInsensitiveFallback If \c true (the default), this means we'll do a case-insensitive search if we didn't
    *                               find \c stringValue as a case-sensitive match.
    */
   std::optional<int> stringToEnumAsInt(QString const & stringValue, bool const caseInsensitiveFallback = true) const;

   /**
    * \brief Convenience function for using \c stringToEnum with strongly typed enums (ie those declared as
    *        "enum class")
    *
    * \throw std::out_of_range if no enum is found for the supplied string (because this is a coding error)
    */
   template<typename E>
   E stringToEnum(QString const & stringValue) const {
      std::optional<int> result = this->stringToEnumAsInt(stringValue);
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no enum mapping found for" << stringValue << Logging::getStackTrace();
         throw std::out_of_range("Missing enum value in EnumStringMapping");
      }
      return static_cast<E>(*result);
   }

   /**
    * \brief Convenience function for using \c stringToEnum with strongly typed enums (ie those declared as
    *        "enum class")
    */
   template<typename E>
   std::optional<E> stringToEnumOrNull(QString const & stringValue) const {
      std::optional<int> result = this->stringToEnumAsInt(stringValue);
      return result ? std::optional<E>{static_cast<E>(*result)} : std::optional<E>{std::nullopt};
   }
};

class FlagEnumStringMapping : public EnumStringMapping {
public:
   FlagEnumStringMapping(std::initializer_list<EnumAndItsString> args);
   std::optional<QString> enumAsIntToValue(int const enumValue) const;

};

#endif
