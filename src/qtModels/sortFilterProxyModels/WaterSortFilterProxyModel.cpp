/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2024:
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "qtModels/sortFilterProxyModels/WaterSortFilterProxyModel.h"

#include <iostream>

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_WaterSortFilterProxyModel.cpp"
#endif

bool WaterSortFilterProxyModel::isLessThan(WaterTableModel::ColumnIndex const columnIndex,
                                           QVariant const & leftItem,
                                           QVariant const & rightItem) const {
   switch (columnIndex) {
      case WaterTableModel::ColumnIndex::Name:
         return leftItem.toString() < rightItem.toString();

///      case WaterTableModel::ColumnIndex::Amount:
///         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Volume) <
///                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Volume);

      case WaterTableModel::ColumnIndex::Calcium    :
      case WaterTableModel::ColumnIndex::Bicarbonate:
      case WaterTableModel::ColumnIndex::Sulfate    :
      case WaterTableModel::ColumnIndex::Chloride   :
      case WaterTableModel::ColumnIndex::Sodium     :
      case WaterTableModel::ColumnIndex::Magnesium  :
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::MassFractionOrConc) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::MassFractionOrConc);

      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   Q_ASSERT(false);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Water)
