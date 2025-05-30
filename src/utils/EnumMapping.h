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

#endif
