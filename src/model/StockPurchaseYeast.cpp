/*======================================================================================================================
 * model/StockPurchaseYeast.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/StockPurchaseYeast.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseYeast.cpp"
#endif

bool StockPurchaseYeast::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockPurchaseYeast const & rhs = static_cast<StockPurchaseYeast const &>(other);
   return (
      // Parent classes have to be equal
      this->StockPurchase       ::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const StockPurchaseYeast::typeLookup {
   "StockPurchaseYeast",
   {
      // Most properties are defined in base classes
      PROPERTY_TYPE_LOOKUP_NO_MV(StockPurchaseYeast, yeast, yeast),
   },
   // Parent classes lookup
   {&StockPurchase::typeLookup,
    std::addressof(IngredientAmount<StockPurchaseYeast, Yeast>::typeLookup),
    std::addressof(StockPurchaseBase   <StockPurchaseYeast,
                                    Yeast,
                                    StockUseYeast>::typeLookup)
   }
};
static_assert(std::is_base_of<StockPurchase, StockPurchaseYeast>::value);

StockPurchaseYeast::StockPurchaseYeast(QString const & name) :
   StockPurchase{name},
   IngredientAmount<StockPurchaseYeast, Yeast>{},
   StockPurchaseBase{} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseYeast::StockPurchaseYeast(NamedParameterBundle const & npb) :
   StockPurchase{npb},
   IngredientAmount<StockPurchaseYeast, Yeast>{npb},
   StockPurchaseBase{npb} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseYeast::StockPurchaseYeast(StockPurchaseYeast const & other) :
   StockPurchase{other},
   IngredientAmount<StockPurchaseYeast, Yeast>{other},
   StockPurchaseBase{other} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseYeast::~StockPurchaseYeast() = default;

void StockPurchaseYeast::setIngredient(Yeast const & val) {
   // No extra work to do for Yeasts
   this->setIngredientId(val.key());
   return;
}

// Boilerplate code for StockPurchase and IngredientAmount
STOCK_PURCHASE_COMMON_CODE(Yeast, yeast)
INGREDIENT_AMOUNT_COMMON_CODE(StockPurchaseYeast, Yeast)
