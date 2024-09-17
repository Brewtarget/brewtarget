/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * utils/AutoCompare.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef UTILS_AUTOCOMPARE_H
#define UTILS_AUTOCOMPARE_H
#pragma once

#include "utils/FuzzyCompare.h"
#include "utils/TypeTraits.h"

namespace Measurement {
   struct Amount;
}

namespace Utils {
   /**
    * \brief This overloaded function decides automatically whether to use regular comparison (==) or \c FuzzyCompare,
    *        as well as preventing comparison of different types.
    *
    *        Note that, for std::optional, regular comparison is well-defined, so we don't have to do anything special.
    */
   inline bool AutoCompare(double  const   lhs, double  const   rhs) { return FuzzyCompare(lhs, rhs); }
   inline bool AutoCompare(int     const   lhs, int     const   rhs) { return lhs == rhs; }
   inline bool AutoCompare(bool    const   lhs, bool    const   rhs) { return lhs == rhs; }
   // Amount already implements operator== using fuzzy comparison
   inline bool AutoCompare(Measurement::Amount const &  lhs, Measurement::Amount const & rhs) { return lhs == rhs; }
   // QStrings should ignore trailling spaces etc when comparing
   inline bool AutoCompare(QString const & lhs, QString const & rhs) { return lhs.trimmed() == rhs.trimmed(); }
   inline bool AutoCompare(std::optional<double> const & lhs, std::optional<double> const & rhs) { return FuzzyCompare(lhs, rhs); }
   inline bool AutoCompare(std::optional<int   > const & lhs, std::optional<int   > const & rhs) { return lhs == rhs; }
   inline bool AutoCompare(std::optional<bool  > const & lhs, std::optional<bool  > const & rhs) { return lhs == rhs; }
   inline bool AutoCompare(std::optional<Measurement::Amount> const & lhs,
                           std::optional<Measurement::Amount> const & rhs) { return lhs == rhs; }

   /**
    * \brief We want to be careful about implicit argument conversion for AutoCompare, so we use the same trick here as
    *        in EditorBase::setEditItem to prevent it.  But we do want generic handling of strongly-typed enums, and
    *        optional strongly-typed enums.  But we can't partially specialise function templates.  So we have to have
    *        non-intersecting versions of enable_if.
    */
   template<typename D, typename E, std::enable_if_t<!std::is_same<D, E>::value>* = nullptr>
   bool AutoCompare(D, E) = delete;

   // This case has to cover only what's not covered below
   template<typename D, typename E, std::enable_if_t<std::is_same<D, E>::value &&
                                                     !IsRequiredEnum<D> &&
                                                     !IsOptionalEnum<D> &&
                                                     !IsQListOfPointer<D>>* = nullptr>
   bool AutoCompare(D, E) = delete;

   template<typename D, typename E, std::enable_if_t<std::is_same<D, E>::value &&
                                                     (IsRequiredEnum<D> ||
                                                      IsOptionalEnum<D>)>* = nullptr>
   bool AutoCompare(D const lhs, E const rhs) { return lhs == rhs; }

   template<typename D, typename E, std::enable_if_t<std::is_same<D, E>::value && IsQListOfPointer<D>>* = nullptr>
   bool AutoCompare(D lhs, E rhs) {
      //
      // We first need to put the lists in order.  This is why we take the parameters by value and not by reference, and
      // why they aren't const!
      //
      std::sort(lhs.begin(), lhs.end(), [](auto a, auto b){ return *a < *b; } );
      std::sort(rhs.begin(), rhs.end(), [](auto a, auto b){ return *a < *b; } );

      //
      // If we just use == on two QList items, they will be "considered equal if they contain the same values in the
      // same order".  This would work for a list of Hop, but not for a list of RecipeAdditionHop items.  So we need to
      // do the comparisons on the objects pointed to.
      //
      // It would be cute to do the loop below with std::views::zip, but I'm not sure we yet have sufficient compiler
      // support on all platforms.
      //
      if (lhs.size() != rhs.size()) {
         return false;
      }
      for (auto ii = 0; ii < lhs.size(); ++ii) {
         if (*lhs.at(ii) != *rhs.at(ii)) {
            return false;
         }
      }
      return true;
   }

}

#endif
