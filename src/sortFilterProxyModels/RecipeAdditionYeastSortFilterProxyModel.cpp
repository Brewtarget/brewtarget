/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2024:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
#include "sortFilterProxyModels/RecipeAdditionYeastSortFilterProxyModel.h"

#include "measurement/Measurement.h"
#include "measurement/PhysicalQuantity.h"

bool RecipeAdditionYeastSortFilterProxyModel::isLessThan(RecipeAdditionYeastTableModel::ColumnIndex const columnIndex,
                                                         QVariant const & leftItem,
                                                         QVariant const & rightItem) const {
   switch (columnIndex) {
      case RecipeAdditionYeastTableModel::ColumnIndex::Name:
      case RecipeAdditionYeastTableModel::ColumnIndex::Laboratory:
      case RecipeAdditionYeastTableModel::ColumnIndex::ProductId:
      case RecipeAdditionYeastTableModel::ColumnIndex::Type:
      case RecipeAdditionYeastTableModel::ColumnIndex::Form:
      case RecipeAdditionYeastTableModel::ColumnIndex::Stage:
      case RecipeAdditionYeastTableModel::ColumnIndex::Step:
      case RecipeAdditionYeastTableModel::ColumnIndex::AmountType:
         return leftItem.toString() < rightItem.toString();

      case RecipeAdditionYeastTableModel::ColumnIndex::Attenuation:
         // Attenuation on a RecipeAdditionYeast is std::optional<double> in the underlying model.  But here, the
         // leftItem and rightItem QVariants will contain QString eg "75.0 %" or "" (for std::nullopt).
         //
         // Measurement::extractRawFromString does pretty much what we want though TODO we should explicitly tell it
         // blanks are OK.
         //
         return Measurement::extractRawFromString<double>( leftItem.toString()) <
                Measurement::extractRawFromString<double>(rightItem.toString());

      case RecipeAdditionYeastTableModel::ColumnIndex::TotalInventory:
      case RecipeAdditionYeastTableModel::ColumnIndex::Amount:
         return Measurement::qStringToSI( leftItem.toString(), Measurement::PhysicalQuantity::Mass) <
                Measurement::qStringToSI(rightItem.toString(), Measurement::PhysicalQuantity::Mass);

      // No default case as we want the compiler to warn us if we missed one
   }

   // Should be unreachable
   Q_ASSERT(false);
   return true;
}

// Insert the boiler-plate stuff that we cannot do in SortFilterProxyModelBase
SORT_FILTER_PROXY_MODEL_COMMON_CODE(RecipeAdditionYeast)
