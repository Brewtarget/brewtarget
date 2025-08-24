/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/OptionalHelpers.h is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#ifndef UTILS_OPTIONALHELPERS_H
#define UTILS_OPTIONALHELPERS_H
#pragma once

#include <optional>

#include <QDebug>
#include <QVariant>

#include "Logging.h"
#include "measurement/PhysicalQuantity.h"
#include "utils/TypeTraits.h"

// Note that we cannot include utils/TypeLookup.h because it includes this header
struct TypeInfo;

/**
 * \brief A set of utilities that help us deal with std::optional values
 */
namespace Optional {
   /**
    * \brief Helper function called from \c ObjectStore::impl::unwrapAndMapAsNeeded and other places to convert a
    *        \c QVariant containing \c std::optional<T> to either a \c QVariant containing \c T or a null \c QVariant
    *
    * \param propertyValue the QVariant to be modified
    * \param hasValue If set, used to return \c false if the supplied \c std::optional<T> is \c std::nullopt, \c true
    *                 otherwise
    */
   template<typename T>
   void removeOptionalWrapper(QVariant & propertyValue, bool * hasValue = nullptr) {
      auto val = propertyValue.value<std::optional<T> >();
      if (hasValue) {
         *hasValue = val.has_value();
      }
      if (val.has_value()) {
         propertyValue = QVariant::fromValue<T>(val.value());
      } else {
         propertyValue = QVariant();
      }
      return;
   }

   /**
    * \brief This version of \c removeOptionalWrapper handles all the different types internally.  Valid types are
    *        \c double, \c int, \c unsigned \c int, \c bool.
    *
    * \param propertyValue the QVariant to be modified
    * \param typeInfo
    * \param hasValue If set, used to return \c false if the supplied \c std::optional is \c std::nullopt, \c true
    *                 otherwise
    */
   void removeOptionalWrapper(QVariant & propertyValue, TypeInfo const & typeInfo, bool * hasValue = nullptr);

   /**
    * \brief Creates a QVariant containing null \c std::optional<int>, \c std::optional<double>, etc according to
    *        \c typeInfo
    */
   QVariant makeNullOpt(TypeInfo const & typeInfo);

   /**
    * \brief Helper function called from \c ObjectStore::impl::wrapAndUnmapAsNeeded and other places to convert a
    *        \c QVariant that is either null or contains \c T to a \c QVariant containing \c std::optional<T>
    */
   template<typename T>
   void insertOptionalWrapper(QVariant & propertyValue) {
      if (propertyValue.isNull()) {
         propertyValue = QVariant::fromValue(std::optional<T>());
      } else {
         propertyValue = QVariant::fromValue(std::optional<T>(propertyValue.value<T>()));
      }
      return;
   }

   /**
    * \brief Create a \c QVariant from a raw value, including the \c std::optional wrapper if needed
    */
   template<typename T>
   QVariant variantFromRaw(T const & rawValue, bool const propertyIsOptional) {
      if (propertyIsOptional) {
         return QVariant::fromValue< std::optional<T> >(rawValue);
      }
      return QVariant::fromValue<T>(rawValue);
   }

   /**
    * \brief Create a possily null \c QVariant from a raw value, including the \c std::optional wrapper if needed
    *
    *        This overload also takes the string input from which the raw value was obtained.
    *        How we deal with empty string input depends on the field type.  If the field is optional then we need to
    *        unset it in the model.  If it is not optional then we'll set \c rawValue, which will typically be 0.0 or
    *        similar.
    */
   template<typename T>
   QVariant variantFromRaw(QString const & inputString, T const & rawValue, bool const propertyIsOptional) {
      if (propertyIsOptional) {
         if (inputString.trimmed().isEmpty()) {
            return QVariant::fromValue<std::optional<T>>(std::nullopt);
         }
         return QVariant::fromValue< std::optional<T> >(rawValue);
      }
      return QVariant::fromValue<T>(rawValue);
   }

   /**
    * \brief Convenience wrapper function that calls \c QVariant::canConvert() for either \c T or \c std::optional<T>
    *        as appropriate.
    */
   template<typename T>
   bool canConvert(QVariant const & propertyValue, bool const propertyIsOptional) {
      if (propertyIsOptional) {
         return propertyValue.canConvert< std::optional<T> >();
      }
      return propertyValue.canConvert<T>();
   }

   /**
    * \brief Remove the \c std::optional wrapper, if it is present, from inside a \c QVariant
    *
    * \return \c false if the contained value is optional and not present, \c true otherwise
    */
   template<typename T>
   bool removeOptionalWrapperIfPresent(QVariant & propertyValue, bool const propertyIsOptional) {
      // It is a coding error to pass a QVariant that can't be converted to (optional) T
      Q_ASSERT(canConvert<T>(propertyValue, propertyIsOptional));
      if (propertyIsOptional) {
         removeOptionalWrapper<T>(propertyValue);
         return !propertyValue.isNull();
      }
      return true;
   }

   /**
    * \brief Create an \c std::optional wrapped type \c T (eg \c MassOrVolumeAmt or \c MassOrVolumeConcentrationAmt)
    *        from an optional \c double and a flag that chooses between two possibilities for the second parameter (of
    *        type \c U) to construct a \c T.  \c U is typically \c Measurement::Unit.
    *
    *        In a lot of model objects, where we allow an optional amount to be measured two ways -- eg by Mass or by
    *        Volume -- the underlying storage has two fields: an optional double (for the quantity if it's set) and a
    *        boolean flag (to say which way is being measured -- eg whether the quantity is a Mass or a Volume).  We
    *        sometimes need a single getter to be able to return an optional \c Measurement::ConstrainedAmount derived
    *        from the two underlying fields.
    */
   template<typename T, typename U>
   std::optional<T> eitherOr(std::optional<double> const & quantity,
                             bool const isFirstUnit,
                             U const & firstUnit,
                             U const & secondUnit) {
      if (!quantity) {
         return std::nullopt;
      }
      return std::make_optional<T>(*quantity, isFirstUnit ? firstUnit : secondUnit);
   }

   /**
    * \brief This is the inverse of the other \c eitherOr!
    *
    *        Note that the template here does not need to know about the \c Measurement::Unit type.  It suffices that
    *        type \c T (typically \c MassOrVolumeAmt or \c MassOrVolumeConcentrationAmt) implements member functions
    *        \c quantity() and \c isFirst().
    *
    * \param constrainedAmount Input
    * \param quantity Output (along with return value)
    *
    * \return \true if the unit of the quantity is of type physicalQuantity (or if there was no quantity); false otherwise
    */
   template<typename T>
   bool eitherOr(std::optional<T> const & constrainedAmount,
                 std::optional<double> & quantity,
                 Measurement::PhysicalQuantity const physicalQuantity) {
      if (!constrainedAmount) {
         quantity = std::nullopt;
         // The return value is not strictly meaningful here, but, for the moment, it's simpler to keep the return type
         // bool
         return true;
      }
      quantity = constrainedAmount->quantity;
      return (constrainedAmount->unit->getPhysicalQuantity() == physicalQuantity);
   }

   /**
    * \brief Convenience function for, in effect, casting std::optional<int> to std::optional<E> where E is an enum class
    */
   template <class E, typename = std::enable_if_t<is_non_optional_enum<E>::value> >
   std::optional<E> fromOptInt(std::optional<int> const & val) {
      if (val.has_value()) {
         return static_cast<E>(val.value());
      }
      return std::nullopt;
   }

   /**
    * \brief Convenience function for, in effect, casting std::optional<E> to std::optional<int> where E is an enum class
    */
   template <class E, typename = std::enable_if_t<is_non_optional_enum<E>::value> >
   std::optional<int> toOptInt(std::optional<E> const & val) {
      if (val.has_value()) {
         return static_cast<int>(val.value());
      }
      return std::nullopt;
   }

   /**
    * \brief Not strictly about whether something is optional, but often somewhat related
    *
    * \return \c true if \c input is empty or blank (ie contains only whitespace), \c false otherwise
    */
   [[nodiscard]] bool isEmptyOrBlank(QString const & input);

   //! \brief Useful for logging
   template <typename T>
   QString toString(std::optional<T> const & val) {
      if (val) {
         return QString{"%1"}.arg(*val);
      }
      return "NULL";
   }
}

/**
 * \brief Convenience function to allow output of \c std::optional to \c QDebug or \c QTextStream stream
 *
 *        (For some reason, \c QDebug does not inherit from \c QTextStream so we template the stream class as well as
 *        what we're outputting.)
 */
template<class S, class T>
S & operator<<(S & stream, std::optional<T> optionalItem) {
   if (optionalItem) {
      stream << *optionalItem;
   } else {
      stream << "NULL";
   }
   return stream;
}

#endif
