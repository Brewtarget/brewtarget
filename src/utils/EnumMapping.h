/*======================================================================================================================
 * utils/EnumMapping.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef UTILS_ENUMMAPPING_H
#define UTILS_ENUMMAPPING_H
#pragma once

#include <initializer_list>

#include <QVector>

#include "utils/TypeTraits.h"

/**
 * \brief Pairs an enum value \c key with \c value of type \c T (which must be default constructable).
 */
template<typename T> struct EnumKeyValue {
   int     key;
   T       value;

   /**
    * \brief Need a default constructor for storing in a vector
    */
   EnumKeyValue() :
      key{-1},
      value{} {
      return;
   };

   /**
    * \brief Standard constructor, which we need to declare explicitly in order to have the templated version below
    */
   EnumKeyValue(int const key, T const value) :
      key{key},
      value{value} {
      return;
   };

   /**
    * \brief Convenience constructor for creating \c EnumAndItsString using strongly typed enums (ie those declared as
    *        "enum class")
    */
   template<typename E>
   EnumKeyValue(E const key, T const value) : EnumKeyValue{static_cast<int>(key), value} {
      return;
   }

};

template<typename T> class EnumMapping : public QVector<EnumKeyValue<T>> {
public:
   /**
    * \brief We could just use the \c QVector constructor, but this way allows us to do some extra optimisation and
    *        checking.
    *
    * \param args \b must be in enum order.  This is a small burden but it rather simplifies the code.
    * \param isRegularEnum Shouldn't normally need to be specified, other than by subclass constructor
    */
   EnumMapping(std::initializer_list<EnumKeyValue<T>> args, bool const isRegularEnum = true) {
      this->reserve(args.size());
      for (auto arg : args) {
         // Uncomment this block for debugging -- eg if you are hitting the assert below at start-up!
//         qDebug().noquote() <<
//            Q_FUNC_INFO << "Inserting at" << arg.native << ". Size=" << this->size() <<
//            Logging::getStackTrace();
         //
         // Essentially, if isRegularEnum is true (which should be the case unless we are called from
         // FlagEnumStringMapping) we are asserting here that args are passed in enum order and that our enum values
         // always start from 0 and never skip any numbers.  If we ever pass things in in the wrong order, we'll get an
         // assert at start-up, so it's pretty immediate feedback of the coding error.
         //
         Q_ASSERT((arg.key == this->size()) || !isRegularEnum);
         this->append(arg);
      }
      return;
   }

   /**
    * \brief Convert data that \b might not be a valid \c int value of an enum to the value for that enum.  This is
    *        possibly useful in some circumstances when we are reading a Qt property of an object and we can't be 100%
    *        certain that the object is of the class we expect
    */
   std::optional<T> enumAsIntToValue(int const enumValue) const  {
      // So here's the advantage of forcing construction to be in enum order
      if (enumValue < 0 || enumValue >= this->size()) {
         return std::nullopt;
      }
      EnumKeyValue<T> const & match = this->at(enumValue);
      Q_ASSERT(match.key == enumValue);

      return std::optional<T>{match.value};
   }

   /**
    * \brief Convenience function for use with strongly typed enums (ie those declared as "enum class")};
    *
    * \throw std::out_of_range if no string is found for the supplied enum (because this is a coding error)
    *        NOTE however that, because we do not template on the enum class (which would cause other problems), the
    *        compiler cannot detect if you are using the wrong object (eg if you use \c Hop::formStringMapping instead
    *        of \c Hop::useStringMapping)
    */
   template<typename E>
   QString enumToValue(E const enumValue) const {
      std::optional<T> result = this->enumAsIntToValue(static_cast<int>(enumValue));
      if (!result) {
         qCritical().noquote() <<
            Q_FUNC_INFO << "Coding error: no mapping found for" << static_cast<int>(enumValue) <<
            Logging::getStackTrace();
         throw std::out_of_range("Missing value in EnumMapping");
      }
      return *result;
   }

   /**
    * \brief A more concise version of \c enumToValue
    *
    *        (We could do similar for looking up key from value, but it would be a bit more clunky as the compiler
    *        wouldn't be able to deduce the enum type automatically.)
    *
    *        See comments in widgets/SmartField.h for why we have the enable_if_t here
    */
   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   QString operator[](E const enumValue) const {
      return this->enumToValue(enumValue);
   }

   template<typename E, typename = std::enable_if_t<is_non_optional<E>::value> >
   QString operator[](std::optional<E> const optionalEnumValue) const {
      return optionalEnumValue ? this->enumToValue(*optionalEnumValue) : T{};
   }
};

#endif
