/*
 * utils/OptionalHelpers.h is part of Brewtarget, and is copyright the following
 * authors 2022:
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef UTILS_OPTIONALTOSTREAM_H
#define UTILS_OPTIONALTOSTREAM_H
#pragma once

#include <optional>

#include <QVariant>

/**
 * \brief A set of utilities that help us deal with std::optional values
 */
namespace Optional {
   /**
    * \brief Helper function called from \c ObjectStore::impl::unwrapAndMapAsNeeded and other places to convert a
    *        \c QVariant containing \c std::optional<T> to either a \c QVariant containing \c T or a null \c QVariant
    */
   template<typename T>
   void removeOptionalWrapper(QVariant & propertyValue) {
      auto val = propertyValue.value<std::optional<T> >();
      if (val.has_value()) {
         propertyValue = QVariant::fromValue<T>(val.value());
      } else {
         propertyValue = QVariant();
      }
      return;
   }

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
