/*======================================================================================================================
 * model/StockPurchaseHop.cpp is part of Brewtarget, and is copyright the following authors 2025:
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
#include "model/StockPurchaseHop.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchaseHop.cpp"
#endif

QString StockPurchaseHop::localisedName_alpha_pct() { return tr("Alpha Acid"); }
QString StockPurchaseHop::localisedName_form     () { return tr("Form"      ); }
QString StockPurchaseHop::localisedName_year     () { return tr("Year"      ); }

bool StockPurchaseHop::compareWith(NamedEntity const & other,
                               QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockPurchaseHop const & rhs = static_cast<StockPurchaseHop const &>(other);
   return (
      // Parent classes have to be equal
      this->StockPurchase       ::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const StockPurchaseHop::typeLookup {
   "StockPurchaseHop",
   {
      // Most properties are defined in base classes
      PROPERTY_TYPE_LOOKUP_NO_MV(StockPurchaseHop, hop      , hop        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchaseHop, alpha_pct, m_alpha_pct, NonPhysicalQuantity::Percentage, DisplayInfo::Precision{1}),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchaseHop, form     , m_form     , ENUM_INFO(Hop::form)           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchaseHop, year     , m_year     , NonPhysicalQuantity::String    ),
   },
   // Parent classes lookup
   {&StockPurchase::typeLookup,
    std::addressof(IngredientAmount<StockPurchaseHop, Hop>::typeLookup),
    std::addressof(StockPurchaseBase   <StockPurchaseHop,
                                    Hop,
                                    StockUseHop>::typeLookup)
   }
};
static_assert(std::is_base_of<StockPurchase, StockPurchaseHop>::value);

StockPurchaseHop::StockPurchaseHop(QString const & name) :
   StockPurchase{name},
   IngredientAmount<StockPurchaseHop, Hop>{},
   StockPurchaseBase{},
   m_alpha_pct{std::nullopt},
   m_form     {std::nullopt},
   m_year     {""          } {
   CONSTRUCTOR_END
   return;
}

StockPurchaseHop::StockPurchaseHop(NamedParameterBundle const & npb) :
   StockPurchase{npb},
   IngredientAmount<StockPurchaseHop, Hop>{npb},
   StockPurchaseBase{npb},
   SET_REGULAR_FROM_NPB (m_alpha_pct         , npb, PropertyNames::StockPurchaseHop::alpha_pct),
   // NB: Hop::Form is the correct enum, and StockPurchaseHop::form the correct property
   SET_OPT_ENUM_FROM_NPB(m_form   , Hop::Form, npb, PropertyNames::StockPurchaseHop::form     ),
   SET_REGULAR_FROM_NPB (m_year              , npb, PropertyNames::StockPurchaseHop::year     , "") {
   CONSTRUCTOR_END
   return;
}

StockPurchaseHop::StockPurchaseHop(StockPurchaseHop const & other) :
   StockPurchase{other},
   IngredientAmount<StockPurchaseHop, Hop>{other},
   StockPurchaseBase{other},
   m_alpha_pct{other.m_alpha_pct},
   m_form     {other.m_form     },
   m_year     {other.m_year     } {
   CONSTRUCTOR_END
   return;
}

StockPurchaseHop::~StockPurchaseHop() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<double>    StockPurchaseHop::alpha_pct() const { return this->m_alpha_pct         ; }
std::optional<Hop::Form> StockPurchaseHop::form     () const { return this->m_form              ; }
std::optional<int>       StockPurchaseHop::formAsInt() const { return Optional::toOptInt(m_form); }
QString                  StockPurchaseHop::year     () const { return this->m_year              ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void StockPurchaseHop::setAlpha_pct(std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::StockPurchaseHop::alpha_pct, this->m_alpha_pct, this->enforceMinAndMax(val, "alpha", 0.0, 100.0)); return; }
void StockPurchaseHop::setForm     (std::optional<Hop::Form> const   val) { SET_AND_NOTIFY(PropertyNames::StockPurchaseHop::form     , this->m_form     , val); return; }
void StockPurchaseHop::setFormAsInt(std::optional<int>       const   val) { SET_AND_NOTIFY(PropertyNames::StockPurchaseHop::form     , this->m_form     , Optional::fromOptInt<Hop::Form>(val)); return; }
void StockPurchaseHop::setYear     (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchaseHop::year     , this->m_year     , val); return; }

// Boilerplate code for StockPurchase and IngredientAmount
STOCK_PURCHASE_COMMON_CODE(Hop, hop)
INGREDIENT_AMOUNT_COMMON_CODE(StockPurchaseHop, Hop)
