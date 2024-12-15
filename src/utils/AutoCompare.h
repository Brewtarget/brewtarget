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

#include <compare>
#include <source_location>

#include "measurement/Amount.h"
#include "measurement/ConstrainedAmount.h"
#include "utils/FuzzyCompare.h"
#include "utils/TypeTraits.h"


template<class S>
S & operator<<(S & stream, std::strong_ordering const & ordering) {
   if (ordering == std::strong_ordering::less      ) { stream << "less      "; }
   if (ordering == std::strong_ordering::equivalent) { stream << "equivalent"; }
   if (ordering == std::strong_ordering::equal     ) { stream << "equal     "; }
   if (ordering == std::strong_ordering::greater   ) { stream << "greater   "; }
   return stream;
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
   inline bool AutoCompare(Measurement::Amount const & lhs, Measurement::Amount const & rhs) { return lhs == rhs; }
   template<Measurement::PhysicalQuantityConstTypes PQT, PQT pqt>
   inline bool AutoCompare(Measurement::ConstrainedAmount<PQT, pqt> const & lhs,
                           Measurement::ConstrainedAmount<PQT, pqt> const & rhs) { return lhs == rhs; }
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
      // If the lists are different lengths they can't possibly be the same!
      if (lhs.size() != rhs.size()) {
         return false;
      }

      //
      // We first need to put the lists in order.  This is why we take the parameters by value and not by reference, and
      // why they aren't const!
      //
      std::sort(lhs.begin(), lhs.end(), [](auto const & a, auto const & b){ return *a < *b; } );
      std::sort(rhs.begin(), rhs.end(), [](auto const & a, auto const & b){ return *a < *b; } );

      //
      // If we just use == on two QList items, they will be "considered equal if they contain the same values in the
      // same order".  This would work for a list of Hop, but not for a list of RecipeAdditionHop items.  So we need to
      // do the comparisons on the objects pointed to.
      //
      // It would be cute to do the loop below with std::views::zip, but I'm not sure we yet have sufficient compiler
      // support on all platforms.
      //
      for (auto ii = 0; ii < lhs.size(); ++ii) {
         if (*lhs.at(ii) != *rhs.at(ii)) {
            // Normally leave the next line commented out as it generates a lot of logging
//            qDebug() << Q_FUNC_INFO << "LHS:" << *lhs.at(ii) << " NOT EQUAL TO RHS:" << *rhs.at(ii);
            return false;
         }
      }
      return true;
   }

   /**
    * \brief Similar to \c AutoCompare, but does 3-way comparison for "strong ordering" (see
    *        https://en.cppreference.com/w/cpp/utility/compare/strong_ordering).
    */
   template<typename T>
   std::strong_ordering Auto3WayCompare(T const & lhs, T const & rhs) {
      //
      // We want a multi-level comparison for the spaceship operator.  Often the recommendation for this sort of thing
      // is to use a tuple (via std::tie), since operator<=> is already well-defined for tuples.  However, there are
      // limitations, eg you can't put an rvalue in a tuple, that make it a bit fiddly here, so we do old-school
      // if/else statements.
      //
      if (AutoCompare(lhs, rhs)) {
         return std::strong_ordering::equal;
      }
      if (lhs < rhs) {
         return std::strong_ordering::less;
      }
      return std::strong_ordering::greater;
   }

   /**
    * \brief It's typically the case that you want to consult a number of fields for a 3-way comparison.  If the first
    *        pair of fields is not equal, then you know the result, otherwise you look at the next pair etc.
    *
    *        This template allows you to just pass all the parameter pairs in one call, and then it does the recursive
    *        logic to compare as many of them as necessary.
    */
   template<typename First, typename Second, typename... Others>
   std::strong_ordering Auto3WayCompare(First  const & lhsFirst , First  const & rhsFirst ,
                                        Second const & lhsSecond, Second const & rhsSecond,
                                        Others... others) {
      std::strong_ordering const result {Utils::Auto3WayCompare(lhsFirst, rhsFirst)};
      if (result != std::strong_ordering::equal) {
         return result;
      }
      return Auto3WayCompare(lhsSecond, rhsSecond, others...);
   }

   /**
    * \brief When comparing two objects with a lot of fields, it can be useful to log the first field that differs.
    *        This wrapper helps do that.  Eg usage is
    *           return (
    *              Utils::LogIfFalse(*this, rhs, "m_foo", Utils::AutoCompare(this->m_foo, rhs.m_foo)) &&
    *              Utils::LogIfFalse(*this, rhs, "m_bar", Utils::AutoCompare(this->m_bar, rhs.m_bar)) &&
    *              ...
    *           )
    *
    * \param lhs
    * \param rhs
    * \param fieldName
    * \param comparisonResult
    * \param sourceLocation Always leave at default value.  This allows us to log where in the source code the function
    *                       was called from.
    *
    * \return \c comparisonResult
    */
   template<class NE>
   bool LogIfFalse(NE const & lhsObj,
                   NE const & rhsObj,
                   char const * const fieldName,
                   bool const comparisonResult,
                   std::source_location const sourceLocation = std::source_location::current()) {
      if (!comparisonResult) {
         //
         // Logging when things differ doesn't usually add a lot of content to the log files.  Most things that are
         // different have different names.  We only compare other fields when the names are identical.
         //
         // std::source_location saves us from having to use macros
         //
         // It would be neat to log the actual field values when they differ, but it's non-trivial as not all of the
         // types can be written to an output stream -- eg various enum class types.
         //
         qDebug() <<
            Q_FUNC_INFO << "file: " << sourceLocation.file_name() << '(' << sourceLocation.line() << ':' <<
            sourceLocation.column() << ") `" << sourceLocation.function_name() << "`: " << fieldName << "of" <<
            lhsObj << "differs from that of" << rhsObj;
      } else {
         // Normally leave this commented out as we typically only want to know the first field that differed, not every
         // single field that matched.
//         qDebug() << Q_FUNC_INFO << "file: " << sourceLocation.file_name() << '(' << sourceLocation.line() << ':' <<
//            sourceLocation.column() << ") `" << sourceLocation.function_name() << "`: " << fieldName << "of" <<
//            lhsObj << "matches that of" << rhsObj;
      }
      return comparisonResult;
   }

}

/**
 * \brief This macro combines \c Utils::LogIfFalse and \c Utils::AutoCompare
 *
 *        Example usage:
 *           return (
 *              AUTO_LOG_COMPARE(this, rhs, m_foo) &&
 *              AUTO_LOG_COMPARE(this, rhs, m_bar) &&
 *              ...
 *           )
 */
#define AUTO_LOG_COMPARE(lhs, rhs, field) \
   Utils::LogIfFalse(*lhs, rhs, #field, Utils::AutoCompare(lhs->field, rhs.field))

/**
 * \brief As \c AUTO_LOG_COMPARE but when we need to compare the return values of getter functions rather than compare
 *        member fields directly.
 */
#define AUTO_LOG_COMPARE_FN(lhs, rhs, getter) \
   Utils::LogIfFalse(*lhs, rhs, #getter, Utils::AutoCompare(lhs->getter(), rhs.getter()))

/**
 * \brief As \c AUTO_LOG_COMPARE but for fields where we hold the ID of something (eg styleId, equipmentId) and we need
 *        to compare the objects to which the IDs relate.
 */
#define AUTO_LOG_COMPARE_ID(lhs, rhs, idFieldClass, idField) \
   Utils::LogIfFalse(*lhs, rhs, #idField, ObjectStoreWrapper::compareById<idFieldClass>(lhs->idField, rhs.idField))

#endif
