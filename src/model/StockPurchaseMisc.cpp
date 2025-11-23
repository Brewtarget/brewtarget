/*======================================================================================================================
 * model/StockPurchaseMisc.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/StockPurchaseMisc.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseMisc.cpp"
#endif

bool StockPurchaseMisc::compareWith(NamedEntity const & other,
                                QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockPurchaseMisc const & rhs = static_cast<StockPurchaseMisc const &>(other);
   return (
      // Parent classes have to be equal
      this->StockPurchase       ::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const StockPurchaseMisc::typeLookup {
   "StockPurchaseMisc",
   {
      // Most properties are defined in base classes
      PROPERTY_TYPE_LOOKUP_NO_MV(StockPurchaseMisc, misc, misc),
   },
   // Parent classes lookup
   {&StockPurchase::typeLookup,
    std::addressof(IngredientAmount<StockPurchaseMisc, Misc>::typeLookup),
    std::addressof(StockPurchaseBase   <StockPurchaseMisc,
                                    Misc,
                                    StockUseMisc>::typeLookup)
   }
};
static_assert(std::is_base_of<StockPurchase, StockPurchaseMisc>::value);

StockPurchaseMisc::StockPurchaseMisc(QString const & name) :
   StockPurchase{name},
   IngredientAmount<StockPurchaseMisc, Misc>{},
   StockPurchaseBase{} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseMisc::StockPurchaseMisc(NamedParameterBundle const & npb) :
   StockPurchase{npb},
   IngredientAmount<StockPurchaseMisc, Misc>{npb},
   StockPurchaseBase{npb} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseMisc::StockPurchaseMisc(StockPurchaseMisc const & other) :
   StockPurchase{other},
   IngredientAmount<StockPurchaseMisc, Misc>{other},
   StockPurchaseBase{other} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseMisc::~StockPurchaseMisc() = default;

void StockPurchaseMisc::setIngredient(Misc const & val) {
   // No extra work to do for Miscs
   this->setIngredientId(val.key());
   return;
}

// Boilerplate code for StockPurchase and IngredientAmount
STOCK_PURCHASE_COMMON_CODE(Misc, misc)
INGREDIENT_AMOUNT_COMMON_CODE(StockPurchaseMisc, Misc)
