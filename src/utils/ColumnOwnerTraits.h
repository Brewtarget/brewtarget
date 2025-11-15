/*======================================================================================================================
 * utils/ColumnOwnerTraits.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef UTILS_COLUMNOWNERTRAITS_H
#define UTILS_COLUMNOWNERTRAITS_H
#pragma once

#include <vector>

#include "model/Ingredient.h"
#include "utils/ColumnInfo.h"

/**
 * \class ColumnIndexHolder
 *
 * \brief Templated class that exists solely to hold the class-specific enum class \c ColumnIndex.  There is therefore
 *        no default definition.  (You can't declare an enum class without defining it.)
 */
template<class Derived>
struct ColumnIndexHolder;

/**
 * \brief We want the data about the columns to be in a Meyers singleton to avoid static initialisation order fiasco.
 *        The function to access that singleton might naturally be a static member function on \c ColumnOwnerTraits.
 *        However, we need to be able to partially specialise the function (for \c TreeFolderNode), and C++ does not
 *        permit partial specialisation of an individual member function of a templated class (nor of a global or
 *        namespace function).  So we make a separate class instead.
 *
 *        We're using a \c std::vector here because it's easier for constant lists.  (With \c QVector, at least in
 *        Qt 5, the items stored even in a const instance still need to be default constructable and copyable.)
 */
template<class ColumnOwner>
struct ColumnOwnerTraitsData {
   static std::vector<ColumnInfo> const & getColumnInfos();
};

/**
 * \class ColumnOwnerTraits
 *
 * \brief Helper class for \c ColumnOwnerTraits.  We want to be able to do partial specialisations of
 *        \c ColumnOwnerTraits::getColumnInfos, but C++ does not permit partial specialisations of an individual member
 *        function of a templated class.  You have to do a partial specialisation of the whole templated class.  As a
 *        workaround, we split out the parts we don't need to specialise into this separate class.
 */
template<class ColumnOwner>
struct ColumnOwnerTraits {

   //
   // This is a traits class, so it only has static members and does not need any constructor or destructor
   //
   [[nodiscard]] static ColumnInfo const & getColumnInfo(size_t const columnIndex) {
      std::vector<ColumnInfo> const & columnInfos = ColumnOwnerTraitsData<ColumnOwner>::getColumnInfos();

      // It's a coding error to call this for a non-existent column
      Q_ASSERT(columnIndex < columnInfos.size());

      ColumnInfo const & columnInfo = columnInfos[columnIndex];

      // Normally the following log statement should be left commented, as it generates a _lot_ of logging.  Uncomment it
      // temporarily if the assert below is firing.
//      qDebug().noquote() <<
//         Q_FUNC_INFO << "columnInfo.index:" << columnInfo.index << ", columnIndex:" << columnIndex <<
//         Logging::getStackTrace();

      // It's a coding error if the info for column N isn't at position N in the vector (in both cases counting from 0)
      Q_ASSERT(columnInfo.index == columnIndex);

      return columnInfo;
   }

   //! \brief Called from, eg, \c BtTableModel::headerData()
   [[nodiscard]] static QVariant getColumnLabel(size_t const columnIndex) {
      //
      // Mostly we can ask the property path to tell us what label to use for a column.  However, when we have
      // Measurement::ChoiceOfPhysicalQuantity "extra" info, we need to show "Amount Type" as column heading instead of
      // "Amount" which, eg, PropertyNames::IngredientAmount::amount would give us.
      //
      ColumnInfo const & columnInfo = getColumnInfo(columnIndex);
      if (columnInfo.extras) {
         // Can't call IngredientAmount::tr as IngredientAmount is a CRTP class.  Ingredient is close enough to give
         // translators context though I think.
         return Ingredient::tr("Amount Type");
      }
      return getColumnInfo(columnIndex).propertyPath.getLocalisedName();
///      return getColumnInfo(columnIndex).label;
   }

   // We _could_ use size_t for numColumns, since it's obviously never negative.  However, various Qt functions for
   // column number use int (and -1 means "invalid"), so we can spare ourselves compiler warnings about comparing signed
   // and unsigned types by sticking to int ourselves.
   [[nodiscard]] static int numColumns() {
      return ColumnOwnerTraitsData<ColumnOwner>::getColumnInfos().size();
   }

};

/**
 * \brief ColumnOwner classes (eg subclasses of \c BtTableModel) should include this in their header file, right before
 *        their class declaration.
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define COLUMN_NAMES(Derived, ...) \
   /* You have to get the order of everything right with traits classes, but the */ \
   /* end result is that we can refer to BoilTableModel::ColumnIndex::Name etc.  */ \
   class Derived;                                                                   \
   template <> struct ColumnIndexHolder<Derived> {                                  \
      enum class ColumnIndex {                                                      \
         __VA_ARGS__                                                                \
      };                                                                            \
   };                                                                               \

/**
 * \brief ColumnOwner classes (eg subclasses of \c BtTableModel) should use this to define the \c getColumnInfos member
 *        function.
 */
#define COLUMN_INFOS(Derived, ...)                                                               \
   template<> std::vector<ColumnInfo> const & ColumnOwnerTraitsData<Derived>::getColumnInfos() { \
      /* Meyers singleton */                                                                     \
      static std::vector<ColumnInfo> const columnInfos {                                         \
         __VA_ARGS__                                                                             \
      };                                                                                         \
      return columnInfos;                                                                        \
   }                                                                                             \


#endif
