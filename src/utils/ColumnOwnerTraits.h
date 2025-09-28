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
 * \class ColumnOwnerTraits
 *
 * \brief Traits base class for subclasses of \c BtTableModel and \c TreeNode to hold other info about their table
 *        columns.
 */
template<class ColumnOwner>
struct ColumnOwnerTraits {

   /**
    * \brief We're using a \c std::vector here because it's easier for constant lists.  (With \c QVector, at least in
    *        Qt 5, the items stored even in a const instance still need to be default constructable and copyable.)
    */
   static std::vector<ColumnInfo> const columnInfos;

   //
   // This is a traits class, so it only has static members and does not need any constructor or destructor
   //
   [[nodiscard]] static ColumnInfo const & getColumnInfo(size_t const columnIndex) {
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
      return getColumnInfo(columnIndex).label;
   }

   [[nodiscard]] static int numColumns() {
      return columnInfos.size();
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


#endif
