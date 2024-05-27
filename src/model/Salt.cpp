/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Salt.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "model/Salt.h"

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/InventorySalt.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

namespace {
   // Constants used in our mass concentration calculations below
   double constexpr molarMass_Ca     =  40.078     ; // https://en.wikipedia.org/wiki/Calcium
   double constexpr molarMass_Cl     =  35.45      ; // https://en.wikipedia.org/wiki/Chloride
   double constexpr molarMass_CO3    =  60.008     ; // https://en.wikipedia.org/wiki/Carbonate
   double constexpr molarMass_HCO3   =  61.0168    ; // https://en.wikipedia.org/wiki/Bicarbonate
   double constexpr molarMass_Mg     =  24.305     ; // https://en.wikipedia.org/wiki/Magnesium
   double constexpr molarMass_Na     =  22.98976928; // https://en.wikipedia.org/wiki/Sodium
   double constexpr molarMass_SO4    =  96.06      ; // https://en.wikipedia.org/wiki/Sulfate
   double constexpr molarMass_CaCl2  = 110.98      ; // https://en.wikipedia.org/wiki/Calcium_chloride
   double constexpr molarMass_CaCO3  = 100.0869    ; // https://en.wikipedia.org/wiki/Calcium_carbonate
   double constexpr molarMass_CaSO4  = 136.14      ; // https://en.wikipedia.org/wiki/Calcium_sulfate (anhydrous form)
   double constexpr molarMass_MgSO4  = 120.366     ; // https://en.wikipedia.org/wiki/Magnesium_sulfate (anhydrous form)
   double constexpr molarMass_NaCl   =  58.443     ; // https://en.wikipedia.org/wiki/Sodium_chloride
   double constexpr molarMass_NaHCO3 =  84.0066    ; // https://en.wikipedia.org/wiki/Sodium_bicarbonate
   double constexpr molarMass_H3PO4  =  97.994     ; // https://en.wikipedia.org/wiki/Phosphoric_acid
}

QString Salt::localisedName() { return tr("Salt"); }

EnumStringMapping const Salt::typeStringMapping {
   {Salt::Type::CaCl2         , "CaCl2"         },
   {Salt::Type::CaCO3         , "CaCO3"         },
   {Salt::Type::CaSO4         , "CaSO4"         },
   {Salt::Type::MgSO4         , "MgSO4"         },
   {Salt::Type::NaCl          , "NaCl"          },
   {Salt::Type::NaHCO3        , "NaHCO3"        },
   {Salt::Type::LacticAcid    , "LacticAcid"    },
   {Salt::Type::H3PO4         , "H3PO4"         },
   {Salt::Type::AcidulatedMalt, "AcidulatedMalt"},
};

EnumStringMapping const Salt::typeDisplayNames {
   {Salt::Type::CaCl2                  , tr("CaCl2"           " (Calcium chloride)"  )},
   {Salt::Type::CaCO3                  , tr("CaCO3"           " (Calcium carbonate)" )},
   {Salt::Type::CaSO4                  , tr("CaSO4"           " (Calcium sulfate)"   )},
   {Salt::Type::MgSO4                  , tr("MgSO4"           " (Magnesium sulfate)" )},
   {Salt::Type::NaCl                   , tr("NaCl"            " (Sodium chloride)"   )},
   {Salt::Type::NaHCO3                 , tr("NaHCO3"          " (Sodium bicarbonate)")},
   {Salt::Type::LacticAcid             , tr("Lactic Acid"                            )},
   {Salt::Type::H3PO4                  , tr("H3PO4"           " (Phosphoric acid)"   )},
   {Salt::Type::AcidulatedMalt         , tr("Acidulated Malt"                        )},
};

bool Salt::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Salt const & rhs = static_cast<Salt const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type   == rhs.m_type
   );
}

ObjectStore & Salt::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Salt>::getInstance();
}

TypeLookup const Salt::typeLookup {
   "Salt",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::isAcid        , Salt::m_is_acid         ,         NonPhysicalQuantity::Bool      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::percentAcid   , Salt::m_percent_acid    ,         NonPhysicalQuantity::Percentage),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Salt::type          , Salt::m_type            ,         NonPhysicalQuantity::Enum      ),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Salt>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Salt>::value);

Salt::Salt(QString name) :
   Ingredient    {name},
   m_type        {Salt::Type::CaCl2},
   m_percent_acid{0.0},
   m_is_acid     {false} {
   return;
}

Salt::Salt(NamedParameterBundle const & namedParameterBundle) :
   Ingredient       {namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_type        , namedParameterBundle, PropertyNames::Salt::type       ),
   SET_REGULAR_FROM_NPB (m_percent_acid, namedParameterBundle, PropertyNames::Salt::percentAcid),
   SET_REGULAR_FROM_NPB (m_is_acid     , namedParameterBundle, PropertyNames::Salt::isAcid     ) {
   return;
}

Salt::Salt(Salt const & other) :
   Ingredient       {other                   },
   m_type            {other.m_type            },
   m_percent_acid    {other.m_percent_acid    },
   m_is_acid         {other.m_is_acid         } {
   return;
}

Salt::~Salt() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Salt::Type      Salt::type          () const { return this->m_type            ; }
bool            Salt::isAcid        () const { return this->m_is_acid         ; }
double          Salt::percentAcid   () const { return this->m_percent_acid    ; }

Measurement::PhysicalQuantity Salt::suggestedMeasure() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         :
      case Salt::Type::CaCO3         :
      case Salt::Type::CaSO4         :
      case Salt::Type::MgSO4         :
      case Salt::Type::NaCl          :
      case Salt::Type::NaHCO3        :
      case Salt::Type::AcidulatedMalt:
         return Measurement::PhysicalQuantity::Mass;
      case Salt::Type::LacticAcid    :
      case Salt::Type::H3PO4         :
         return Measurement::PhysicalQuantity::Volume;
      // No default case as we want the compiler to warn us if we missed one
   }
   return Measurement::PhysicalQuantity::Mass; // Should be unreachable, but GCC gives a warning if we don't have this
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================

// This may come to haunt me, but I am setting the isAcid flag and the
// amount_is_weight flags here.
//
// 2023-06-02: MY: In for a penny, in for a pound.  I've moved the logic that "automatically" works out the acidity from
// SaltTableModel to here too.  But TBD I think we want to take another look at this at some point.
void Salt::setType(Salt::Type type) {
   bool isAcid = false;
   double newPercentAcid = m_percent_acid;
   if (type == Salt::Type::LacticAcid ||
       type == Salt::Type::H3PO4      ||
       type == Salt::Type::AcidulatedMalt) {
      isAcid = true;
   } else {
      newPercentAcid = 0.0;
   }
   SET_AND_NOTIFY(PropertyNames::Salt::type,           this->m_type,    type);
   SET_AND_NOTIFY(PropertyNames::Salt::isAcid,         this->m_is_acid, isAcid);
   if (isAcid && newPercentAcid == 0.0) {
      switch (type) {
         case Salt::Type::LacticAcid    : newPercentAcid = 88; break;
         case Salt::Type::H3PO4         : newPercentAcid = 10; break;
         case Salt::Type::AcidulatedMalt: newPercentAcid =  2; break;
         // The next line should be unreachable!
         default                        : Q_ASSERT(false); break;
      }
   }
   this->setPercentAcid(newPercentAcid);
   return;
}

void Salt::setIsAcid(bool val) {
   SET_AND_NOTIFY(PropertyNames::Salt::isAcid, this->m_is_acid, val);
}

void Salt::setPercentAcid(double val) {
   // .:TBD:. Maybe we should check here that we are an acid...
   SET_AND_NOTIFY(PropertyNames::Salt::percentAcid, this->m_percent_acid, val);
}

//====== maths ===========
//
// It's common to see the use of "parts per million (ppm)" used as a measure of concentration, but, as explained in
// measurement/PhysicalQuantity.h and at https://en.wikipedia.org/wiki/Parts-per_notation, we need to clarify whether we
// mean mass fraction, mole fraction or volume fraction.
//
// Note below that '.' is decimal separator and ',' is thousands separator.
//
//  • The Avogadro constant (6.022×10²³ = 6.022×10^23) says how many entities in 1 mole of something.
//
//  • The weight in grams of one mole of a substance is the same as its mean atomic mass.
//    Eg the mean molecular weight of water is 18.015 atomic mass units (amu), aka g/mol, so one mole of water weighs
//    18.015 grams.
//
//  • We also know that a liter of water weighs 1000 grams (at standard temperature and pressure).  So 1 liter of water
//    contains 1000/18,015 mol water molecules.
//
//  • Thus if we know the atomic weights of a salt and its constituent ions, we can calculate how many ions per million
//    water molecules (aka parts per million) we get from adding 1 gram of the salt to 1 liter of water.
//
//  • Various things cancel out to make the calculation relatively simple.
//    Let S be the salt, and I be the constituent ion we're interested in.  Call the molecular masses of the salt and
//    the ion Ms and Mi respectively.  We'll write L for The Avogadro constant.  We have:
//      ◦ 1 mol of S weighs Ms grams
//      ◦ 1 gram of S contains 1/Ms mol of S molecules, and thus 1/Ms mol of I ions
//      ◦ Thus 1 gram of S in 1 liter of water is 1/Ms mol of I ions in 1000/18.015 mol water molecules
//          = 1/Ms I ions per 1,000/18.015 mol water molecules
//          = 18.015/Ms I ions per 1,000 water molecules
//          = 18,015/Ms I ions per 1,000,000 water molecules
//          = 18,015/Ms ppm mole fraction I ions
//
//  • If we want the mass concentration, it's simpler.
//      ◦ 1 gram of S contains Mi/Ms grams of I ions
//      ◦ 1 liter of water weighs 1000 grams
//      ◦ So 1 gram of S in 1 liter of water gives a mass concentration of Mi/Ms parts per thousand
//          = 1000 × Mi/Ms parts per million mass concentration
//
// See also Bru'n Water's execellent water knowledge page (previously at
// https://sites.google.com/site/brunwater/water-knowledge, currently at https://www.brunwater.com/) for more info on
// water adjustments.
//
// eg:
//    NaHCO3 84 g/mol
//       Na provides    23 g/mol
//       HCO3 provides  61 g/mol (ish)
//     So 1 g of NaHCO3 in 1L of water provides 1000*(61/84) = 726 ppm HCO3
//
// Remember, we store masses in kilograms, so the results of these functions need to be multiplied by 1000.  (Yes,
// that's two multiplications by 1000.  Inside the functions here we do it to to go from parts per thousand to parts per
// million.  The caller typically needs to do it again to go from kilograms to grams.)
//
double Salt::massConcPpm_Ca_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return (molarMass_Ca / molarMass_CaCl2) * 1000.0;
      case Salt::Type::CaCO3         : return (molarMass_Ca / molarMass_CaCO3) * 1000.0;
      case Salt::Type::CaSO4         : return (molarMass_Ca / molarMass_CaSO4) * 1000.0;
      case Salt::Type::MgSO4         : return 0.0;
      case Salt::Type::NaCl          : return 0.0;
      case Salt::Type::NaHCO3        : return 0.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_Cl_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return (molarMass_Cl * 2.0 / molarMass_CaCl2) * 1000.0;
      case Salt::Type::CaCO3         : return 0.0;
      case Salt::Type::CaSO4         : return 0.0;
      case Salt::Type::MgSO4         : return 0.0;
      case Salt::Type::NaCl          : return (molarMass_Cl       / molarMass_NaCl ) * 1000.0;
      case Salt::Type::NaHCO3        : return 0.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_CO3_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return 0.0;
      case Salt::Type::CaCO3         : return (molarMass_CO3 / molarMass_CaCO3) * 1000.0;
      case Salt::Type::CaSO4         : return 0.0;
      case Salt::Type::MgSO4         : return 0.0;
      case Salt::Type::NaCl          : return 0.0;
      case Salt::Type::NaHCO3        : return 0.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_HCO3_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return 0.0;
      case Salt::Type::CaCO3         : return 0.0;
      case Salt::Type::CaSO4         : return 0.0;
      case Salt::Type::MgSO4         : return 0.0;
      case Salt::Type::NaCl          : return 0.0;
      case Salt::Type::NaHCO3        : return (molarMass_HCO3 / molarMass_NaHCO3) * 1000.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_Mg_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return 0.0;
      case Salt::Type::CaCO3         : return 0.0;
      case Salt::Type::CaSO4         : return 0.0;
      case Salt::Type::MgSO4         : return (molarMass_Mg / molarMass_MgSO4) * 1000.0;
      case Salt::Type::NaCl          : return 0.0;
      case Salt::Type::NaHCO3        : return 0.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_Na_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return 0.0;
      case Salt::Type::CaCO3         : return 0.0;
      case Salt::Type::CaSO4         : return 0.0;
      case Salt::Type::MgSO4         : return 0.0;
      case Salt::Type::NaCl          : return (molarMass_Na / molarMass_NaCl  ) * 1000.0;
      case Salt::Type::NaHCO3        : return (molarMass_Na / molarMass_NaHCO3) * 1000.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

double Salt::massConcPpm_SO4_perGramPerLiter() const {
   switch (this->m_type) {
      case Salt::Type::CaCl2         : return 0.0;
      case Salt::Type::CaCO3         : return 0.0;
      case Salt::Type::CaSO4         : return (molarMass_SO4 / molarMass_CaSO4)  * 1000.0;
      case Salt::Type::MgSO4         : return (molarMass_SO4 / molarMass_MgSO4)  * 1000.0;
      case Salt::Type::NaCl          : return 0.0;
      case Salt::Type::NaHCO3        : return 0.0;
      case Salt::Type::LacticAcid    : return 0.0;
      case Salt::Type::H3PO4         : return 0.0;
      case Salt::Type::AcidulatedMalt: return 0.0;
      // No default case as we want the compiler to warn us if we missed one
   }
   return 0.0; // Should be unreachable, but GCC gives a warning if we don't have this
}

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Salt)
