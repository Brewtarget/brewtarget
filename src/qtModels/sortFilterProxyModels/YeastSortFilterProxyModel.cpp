/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * qtModels/sortFilterProxyModels/YeastSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors
 * 2009-2024:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
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
#include "qtModels/sortFilterProxyModels/YeastSortFilterProxyModel.h"

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_YeastSortFilterProxyModel.cpp"
#endif

bool YeastSortFilterProxyModel::isLessThan(YeastTableModel::ColumnIndex const columnIndex,
                                           QVariant const & leftItem,
                                           QVariant const & rightItem) const {
   switch (columnIndex) {
      case YeastTableModel::ColumnIndex::Name      :
      case YeastTableModel::ColumnIndex::Laboratory:
      case YeastTableModel::ColumnIndex::Type      :
      case YeastTableModel::ColumnIndex::Form      :
      case YeastTableModel::ColumnIndex::ProductId :
      case YeastTableModel::ColumnIndex::TotalInventoryType :
         return leftItem.toString() < rightItem.toString();

      case YeastTableModel::ColumnIndex::TotalInventory:
         // This is a lie. I need to figure out if they are weights or volumes.
         // and then figure some reasonable way to compare weights to volumes.
         // Maybe lying isn't such a bad idea
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Volume) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Volume);

      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   Q_ASSERT(false);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Yeast)
