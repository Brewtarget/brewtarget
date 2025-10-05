/*======================================================================================================================
 * model/InventoryFermentable.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/InventoryFermentable.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_InventoryFermentable.cpp"
#endif

bool InventoryFermentable::compareWith(NamedEntity const & other,
                                       QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   InventoryFermentable const & rhs = static_cast<InventoryFermentable const &>(other);
   return (
      // Parent classes have to be equal
      this->Inventory       ::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount::doCompareWith(rhs, propertiesThatDiffer)
   );
}


// Boilerplate code for Inventory and IngredientAmount
INVENTORY_COMMON_CODE(Fermentable, fermentable)
INGREDIENT_AMOUNT_COMMON_CODE(InventoryFermentable, Fermentable)
