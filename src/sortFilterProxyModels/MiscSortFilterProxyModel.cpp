/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * sortFilterProxyModels/MiscSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following authors
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
#include "sortFilterProxyModels/MiscSortFilterProxyModel.h"

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"

bool MiscSortFilterProxyModel::isLessThan(MiscTableModel::ColumnIndex const columnIndex,
                                          QVariant const & leftItem,
                                          QVariant const & rightItem) const {
   switch (columnIndex) {
      case MiscTableModel::ColumnIndex::Name:
      case MiscTableModel::ColumnIndex::Type:
///      case MiscTableModel::ColumnIndex::Use:
      case MiscTableModel::ColumnIndex::TotalInventoryType:
         return leftItem.toString() < rightItem.toString();

      case MiscTableModel::ColumnIndex::TotalInventory:
         // It's probably wrong to assume everything is mass, but OTOH, it's not instantly obvious how to sort a mixture
         // of masses and volumes
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Mass) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Mass);

///      case MiscTableModel::ColumnIndex::Time:
///         return (Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Time) <
///                 Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Time));

      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   Q_ASSERT(false);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(Misc)
