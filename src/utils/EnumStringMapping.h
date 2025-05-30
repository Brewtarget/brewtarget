/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/EnumStringMapping.h is part of Brewtarget, and is copyright the following authors 2021-2023:
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

/**
 * \brief Associates an enum value with a string representation (eg in the DB or BeerXML or BeerJSON).  Storing a
 *        string is in some cases required by the external serialisation but, even for "internal" storage in the DB,
 *        is more robust than just storing the raw numerical value of the enum.
 */
struct EnumAndItsString {
   int     native;
   QString string;

   /**
    * \brief Need a default constructor for storing in a vector
    */
   EnumAndItsString();

   /**
    * \brief Standard constructor, which we need to declare explicitly in order to have the templated version below
    */
   EnumAndItsString(int native, QString string);

   /**
    * \brief Convenience constructor for creating \c EnumAndItsString using strongly typed enums (ie those declared as
    *        "enum class")
    */
   template<typename E>
   EnumAndItsString(E native, QString string) : EnumAndItsString(static_cast<int>(native), string) {
      return;
   }
};

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
class EnumStringMapping : public QVector<EnumAndItsString> {
public:
   /**
    * \brief We could just use the \c QVector constructor, but this way allows us to do some extra optimisation and
    *        checking.
    *
    * \param args \b must be in enum order.  This is a small burden but it rather simplifies the code.
    * \param isRegularEnum Shouldn't normally need to be specified, other than by subclass constructor
    */
   EnumStringMapping(std::initializer_list<EnumAndItsString> args, bool const isRegularEnum = true);

   /**
    * \brief Convert data that \b might not be a valid string representation of an enum to the \c int value of that
    *        enum.  This is useful when we are deserialising a file for instance
    * \param stringValue
    * \param caseInensitiveFallback If \c true (the default), this means we'll do a case-insensitive search if we didn't
    *                               find \c stringValue as a case-sensitive match.
    */
   std::optional<int> stringToEnumAsInt(QString const & stringValue, bool const caseInensitiveFallback = true) const;

   /**
    * \brief Convert data that \b might not be a valid \c int value of an enum to the string representation of that
    *        enum.  This is possibly useful in some circumstances when we are reading a Qt property of an object and we
    *        can't be 100% certain that the object is of the class we expect
    */
   std::optional<QString> enumAsIntToString(int const enumValue) const;

   /**
    * \brief Convenience function for using \c enumToString with strongly typed enums (ie those declared as
    *        "enum class")
    *
    * \throw std::out_of_range if no string is found for the supplied enum (because this is a coding error)
    *        NOTE however that, because \c EnumAndItsString is not a templated class, the compiler cannot detect if you
    *        are using the wrong object (eg if you use \c Hop::formStringMapping instead of \c Hop::useStringMapping)
    */
   template<typename E>
   QString enumToString(E const enumValue) const {
      std::optional<QString> result = this->enumAsIntToString(static_cast<int>(enumValue));
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no string mapping found for" << static_cast<int>(enumValue) <<
            Logging::getStackTrace();
         throw std::out_of_range("Missing string value in EnumStringMapping");
      }
      return *result;
   }

   /**
    * \brief A more concise version of \c enumToString
    *
    *        (We could do similar for stringToEnum, but it would be a bit more clunky as the compiler wouldn't be able
    *        to deduce the enum type automatically.)
    *
    *        See comments in widgets/SmartField.h for why we have the enable_if_t here
    */
   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   QString operator[](E const enumValue) const {
      return this->enumToString(enumValue);
   }

   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   QString operator[](std::optional<E> const optionalEnumValue) const {
      return optionalEnumValue ? this->enumToString(*optionalEnumValue) : "";
   }

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
   std::optional<QString> enumAsIntToString(int const enumValue) const;

};

#endif
