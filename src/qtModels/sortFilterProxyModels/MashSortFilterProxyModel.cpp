/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/MashSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "qtModels/sortFilterProxyModels/MashSortFilterProxyModel.h"

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_MashSortFilterProxyModel.cpp"

bool MashSortFilterProxyModel::isLessThan(MashTableModel::ColumnIndex const columnIndex,
                                          QVariant const & leftItem,
                                          QVariant const & rightItem) const {
   switch (columnIndex) {
      case MashTableModel::ColumnIndex::Name        :
         return leftItem.toString() < rightItem.toString();

      case MashTableModel::ColumnIndex::InitialGrainTemp:
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Temperature) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Temperature);

      case MashTableModel::ColumnIndex::TotalMashWater:
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Volume) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Volume);

      case MashTableModel::ColumnIndex::TotalTime:
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Time) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Time);
      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   Q_ASSERT(false);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Mash)
