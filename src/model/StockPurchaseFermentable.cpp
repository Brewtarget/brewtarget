/*======================================================================================================================
 * model/StockPurchaseFermentable.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/StockPurchaseFermentable.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseFermentable.cpp"
#endif

QString StockPurchaseFermentable::localisedName_color_lovibond() { return tr("Color"); } // See header file comment
QString StockPurchaseFermentable::localisedName_color_srm     () { return tr("Color"); }

bool StockPurchaseFermentable::compareWith(NamedEntity const & other,
                                       QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockPurchaseFermentable const & rhs = static_cast<StockPurchaseFermentable const &>(other);
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_color_lovibond, PropertyNames::StockPurchaseFermentable::color_lovibond, propertiesThatDiffer) &&
      // Parent classes have to be equal
      this->StockPurchase   ::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const StockPurchaseFermentable::typeLookup {
   "StockPurchaseFermentable",
   {
      // Most properties are defined in base classes
      PROPERTY_TYPE_LOOKUP_NO_MV(StockPurchaseFermentable, fermentable   , fermentable     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchaseFermentable, color_lovibond, m_color_lovibond, Measurement::PhysicalQuantity::Color ),
      PROPERTY_TYPE_LOOKUP_NO_MV(StockPurchaseFermentable, color_srm     , color_srm       , Measurement::PhysicalQuantity::Color , DisplayInfo::Precision{1}),
   },
   // Parent classes lookup
   {&StockPurchase::typeLookup,
    std::addressof(IngredientAmount<StockPurchaseFermentable, Fermentable>::typeLookup),
    std::addressof(StockPurchaseBase<StockPurchaseFermentable,
                                     Fermentable,
                                     StockUseFermentable>::typeLookup)
   }
};
static_assert(std::is_base_of<StockPurchase, StockPurchaseFermentable>::value);

StockPurchaseFermentable::StockPurchaseFermentable(QString const & name) :
   StockPurchase{name},
   IngredientAmount<StockPurchaseFermentable, Fermentable>{},
   StockPurchaseBase{},
   m_color_lovibond{std::nullopt} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseFermentable::StockPurchaseFermentable(NamedParameterBundle const & npb) :
   StockPurchase{npb},
   IngredientAmount<StockPurchaseFermentable, Fermentable>{npb},
   StockPurchaseBase{npb} {
   // The bundle could have either color_lovibond (BeerXML, DB) or color_srm (BeerJSON) set, or neither (but not both)
   if (!SET_IF_PRESENT_FROM_NPB_NO_MV(StockPurchaseFermentable::setColor_lovibond, npb, PropertyNames::StockPurchaseFermentable::color_lovibond) &&
       !SET_IF_PRESENT_FROM_NPB_NO_MV(StockPurchaseFermentable::setColor_srm     , npb, PropertyNames::StockPurchaseFermentable::color_srm     )) {
      this->m_color_lovibond = std::nullopt;
   }

   CONSTRUCTOR_END
   return;
}

StockPurchaseFermentable::StockPurchaseFermentable(StockPurchaseFermentable const & other) :
   StockPurchase{other},
   IngredientAmount<StockPurchaseFermentable, Fermentable>{other},
   StockPurchaseBase{other},
   m_color_lovibond{other.m_color_lovibond} {
   CONSTRUCTOR_END
   return;
}

StockPurchaseFermentable::~StockPurchaseFermentable() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<double> StockPurchaseFermentable::color_lovibond() const { return this->m_color_lovibond; }
std::optional<double> StockPurchaseFermentable::color_srm     () const {
   // SRM is canonical color unit
   if (!this->m_color_lovibond) {
      return std::nullopt;
   }
   return Measurement::Units::lovibond.toCanonical(*this->m_color_lovibond).quantity;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void StockPurchaseFermentable::setColor_lovibond(std::optional<double> const   val) {
   SET_AND_NOTIFY(PropertyNames::StockPurchaseFermentable::color_lovibond,
                  this->m_color_lovibond,
                  this->enforceMin(val, "color"));
   return;
}
void StockPurchaseFermentable::setColor_srm     (std::optional<double> const   val) {
   if (!val) {
      this->setColor_lovibond(std::nullopt);
   }
   this->setColor_lovibond(Measurement::Units::lovibond.fromCanonical(*val));
   return;
}

void StockPurchaseFermentable::setIngredient(Fermentable const & val) {
   // Take default values for our optional fields that can "override" those of the base Fermentable
   if (!this->m_color_lovibond) { this->setColor_lovibond(val.color_lovibond()); }

   this->setIngredientId(val.key());
   return;
}

// Boilerplate code for StockPurchase and IngredientAmount
STOCK_PURCHASE_COMMON_CODE(Fermentable, fermentable)
INGREDIENT_AMOUNT_COMMON_CODE(StockPurchaseFermentable, Fermentable)
