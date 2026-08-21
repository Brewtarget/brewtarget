/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Misc.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Samuel Östling <MrOstling@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "model/Misc.h"

#include <iostream>
#include <string>

#include <QDebug>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "measurement/PhysicalConstants.h"
#include "model/Folder.h"
#include "model/StockPurchaseMisc.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Misc.cpp"
#endif

QString Misc::localisedName() { return tr("Miscellaneous"); }
QString Misc::localisedName_notes    () { return tr("Notes"     ); }
QString Misc::localisedName_producer () { return tr("Producer"  ); }
QString Misc::localisedName_productId() { return tr("Product ID"); }
QString Misc::localisedName_type     () { return tr("Type"      ); }
QString Misc::localisedName_useFor   () { return tr("Use For"   ); }
QString Misc::localisedName_waterAgentIsAcid     () { return tr("Water Agent Is Acid"); }
QString Misc::localisedName_waterAgentPercentAcid() { return tr("Water Agent %% Acid"); }
QString Misc::localisedName_waterAgentType       () { return tr("Water Agent Type"   ); }

EnumStringMapping const Misc::typeStringMapping {
   {Misc::Type::Spice      , "spice"      },
   {Misc::Type::Fining     , "fining"     },
   {Misc::Type::WaterAgent , "water agent"},
   {Misc::Type::Herb       , "herb"       },
   {Misc::Type::Flavor     , "flavor"     },
   {Misc::Type::Other      , "other"      },
   {Misc::Type::Wood       , "wood"       },
};

EnumStringMapping const Misc::typeDisplayNames {
   {Misc::Type::Spice      , tr("Spice"      )},
   {Misc::Type::Fining     , tr("Fining"     )},
   {Misc::Type::WaterAgent , tr("Water Agent")},
   {Misc::Type::Herb       , tr("Herb"       )},
   {Misc::Type::Flavor     , tr("Flavor"     )},
   {Misc::Type::Other      , tr("Other"      )},
   {Misc::Type::Wood       , tr("Wood"       )},
};

EnumStringMapping const Misc::waterAgentTypeStringMapping {
   {Misc::WaterAgentType::CalciumChloride  , "calcium chloride"  },
   {Misc::WaterAgentType::CalciumCarbonate , "calcium carbonate" },
   {Misc::WaterAgentType::CalciumSulfate   , "calcium sulfate"   },
   {Misc::WaterAgentType::MagnesiumSulfate , "magnesium sulfate" },
   {Misc::WaterAgentType::SodiumChloride   , "sodium chloride"   },
   {Misc::WaterAgentType::SodiumBicarbonate, "sodium bicarbonate"},
   {Misc::WaterAgentType::LacticAcid       , "lactic acid"       },
   {Misc::WaterAgentType::PhosphoricAcid   , "phosphoric acid"   },
   {Misc::WaterAgentType::Other            , "other"             }
};

EnumStringMapping const Misc::waterAgentTypeDisplayNames {
   //
   // I guess it's a matter of preference whether you want to refer to something by its molecular formula or its name.
   // There isn't a strong reason for lactic acid to be different from the others.  In older versions of the code, we
   // did not show its molecular formula at all, so that's the only current reason it's the other way around from the
   // rest.
   //
   {Misc::WaterAgentType::CalciumChloride  , tr("Calcium Chloride"  " (CaCl₂)" )},
   {Misc::WaterAgentType::CalciumCarbonate , tr("Calcium Carbonate" " (CaCO₃)" )},
   {Misc::WaterAgentType::CalciumSulfate   , tr("Calcium Sulfate"   " (CaSO₄)" )},
   {Misc::WaterAgentType::MagnesiumSulfate , tr("Magnesium Sulfate" " (MgSO₄)" )},
   {Misc::WaterAgentType::SodiumChloride   , tr("Sodium Chloride"   " (NaCl)"  )},
   {Misc::WaterAgentType::SodiumBicarbonate, tr("Sodium Bicarbonate"" (NaHCO₃)")},
   {Misc::WaterAgentType::LacticAcid       , tr("Lactic Acid"       " (C₃H₆O₃)")},
   {Misc::WaterAgentType::PhosphoricAcid   , tr("Phosphoric acid"   " (H₃PO₄)" )},
   {Misc::WaterAgentType::Other            , tr("Other")},
};

bool Misc::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Misc const & rhs = static_cast<Misc const &>(other);
   // Base class will already have ensured names are equal
   bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the MiscellaneousBase type
      AUTO_PROPERTY_COMPARE(this, rhs, m_producer , PropertyNames::Misc::producer , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_productId, PropertyNames::Misc::productId, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_type     , PropertyNames::Misc::type     , propertiesThatDiffer) &&
      // These are not part of BeerJSON, but logically belong here too
      AUTO_PROPERTY_COMPARE(this, rhs, m_waterAgentType       , PropertyNames::Misc::waterAgentType       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_waterAgentPercentAcid, PropertyNames::Misc::waterAgentPercentAcid, propertiesThatDiffer)
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&
      // Remaining BeerJSON fields -- excluding inventories
      AUTO_PROPERTY_COMPARE(this, rhs, m_useFor, PropertyNames::Misc::useFor, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_notes , PropertyNames::Misc::notes , propertiesThatDiffer)
   );
}

ObjectStore & Misc::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Misc>::getInstance();
}

TypeLookup const Misc::typeLookup {
   "Misc",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, notes    , m_notes    , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, type     , m_type     , ENUM_INFO(Misc::type)),
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, useFor   , m_useFor   , NonPhysicalQuantity::String),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, producer , m_producer , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, productId, m_productId, NonPhysicalQuantity::String),
      // All below migrated from Salt
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, waterAgentType       , m_waterAgentType       , ENUM_INFO(Misc::waterAgentType)),
      PROPERTY_TYPE_LOOKUP_NO_MV(Misc, waterAgentIsAcid     , waterAgentIsAcid       , BOOL_INFO(tr("No"), tr("Yes")) ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Misc, waterAgentPercentAcid, m_waterAgentPercentAcid, NonPhysicalQuantity::Percentage),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Misc>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Misc>::value);

//============================CONSTRUCTORS======================================

Misc::Misc(QString const & name) :
   Ingredient        {name},
   FolderPropertyBase{} {
   // Default values for all our properties are set in the header file, so nothing to do here.
   CONSTRUCTOR_END
   return;
}

Misc::Misc(NamedParameterBundle const & namedParameterBundle) :
   Ingredient        {namedParameterBundle},
   FolderPropertyBase{namedParameterBundle},
   SET_REGULAR_FROM_NPB(m_type     , namedParameterBundle, PropertyNames::Misc::type     ),
   SET_REGULAR_FROM_NPB(m_useFor   , namedParameterBundle, PropertyNames::Misc::useFor   ),
   SET_REGULAR_FROM_NPB(m_notes    , namedParameterBundle, PropertyNames::Misc::notes    ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB(m_producer , namedParameterBundle, PropertyNames::Misc::producer ),
   SET_REGULAR_FROM_NPB(m_productId, namedParameterBundle, PropertyNames::Misc::productId),
   // All below migrated from Salt
   SET_OPT_ENUM_FROM_NPB(m_waterAgentType, Misc::WaterAgentType, namedParameterBundle, PropertyNames::Misc::waterAgentType),
   SET_REGULAR_FROM_NPB(m_waterAgentPercentAcid, namedParameterBundle, PropertyNames::Misc::waterAgentPercentAcid) {

   CONSTRUCTOR_END
   return;
}

Misc::Misc(Misc const & other) :
   Ingredient        {other},
   FolderPropertyBase{other},
   m_type     {other.m_type     },
   m_useFor   {other.m_useFor   },
   m_notes    {other.m_notes    },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_producer {other.m_producer },
   m_productId{other.m_productId},
   // All below migrated from Salt
   m_waterAgentType       {other.m_waterAgentType       },
   m_waterAgentPercentAcid{other.m_waterAgentPercentAcid} {

   CONSTRUCTOR_END
   return;
}

Misc::~Misc() = default;

bool Misc::isAcid(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumChloride :
      case Misc::WaterAgentType::CalciumCarbonate :
      case Misc::WaterAgentType::CalciumSulfate :
      case Misc::WaterAgentType::MagnesiumSulfate :
      case Misc::WaterAgentType::SodiumChloride  :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::Other :
         return false;
      case Misc::WaterAgentType::LacticAcid:
      case Misc::WaterAgentType::PhosphoricAcid     :
         return true;
      // No default case as we want the compiler to warn us if we missed one
   }
   Q_UNREACHABLE();
}

Measurement::PhysicalQuantity Misc::suggestedMeasureFor(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumChloride :
      case Misc::WaterAgentType::CalciumCarbonate :
      case Misc::WaterAgentType::CalciumSulfate :
      case Misc::WaterAgentType::MagnesiumSulfate :
      case Misc::WaterAgentType::SodiumChloride  :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::Other :
         return Measurement::PhysicalQuantity::Mass;
      case Misc::WaterAgentType::LacticAcid:
      case Misc::WaterAgentType::PhosphoricAcid     :
         return Measurement::PhysicalQuantity::Volume;
      // No default case as we want the compiler to warn us if we missed one
   }
   Q_UNREACHABLE();
}

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Misc::Type Misc::type          () const { return m_type     ; }
QString    Misc::useFor        () const { return m_useFor   ; }
QString    Misc::notes         () const { return m_notes    ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
QString    Misc::producer      () const { return m_producer ; }
QString    Misc::productId     () const { return m_productId; }
// All below migrated from Salt
std::optional<Misc::WaterAgentType> Misc::waterAgentType() const {
   if (this->m_type != Misc::Type::WaterAgent) {
      qWarning() << Q_FUNC_INFO << Misc::typeStringMapping[this->m_type] << "is not water agent";
      return std::nullopt;
   }
   return this->m_waterAgentType;
}
std::optional<int> Misc::waterAgentTypeAsInt() const {
   return Optional::toOptInt(this->waterAgentType());
}

bool Misc::waterAgentIsAcid() const {
   if (this->m_type != Misc::Type::WaterAgent) {
      qWarning() << Q_FUNC_INFO << Misc::typeStringMapping[this->m_type] << "is not water agent";
      return false;
   }
   if (!this->m_waterAgentType) {
      qWarning() << Q_FUNC_INFO << this << "has no water agent type set, so can't determine whether is acid";
      return false;
   }
   return Misc::isAcid(*this->m_waterAgentType);
}

std::optional<double> Misc::waterAgentPercentAcid() const {
   if (!this->waterAgentIsAcid()) {
      return std::nullopt;
   }
   return this->m_waterAgentPercentAcid;
}


//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Misc::setType     (Type    const   val) {
   SET_AND_NOTIFY( PropertyNames::Misc::type    , this->m_type     , val);
   // We could check here whether val is Misc::Type::WaterAgent and, if not, unset m_waterAgentType and
   // m_waterAgentPercentAcid.  However, this would make undo/redo more complicated to implement.  So, instead, we have
   // logic in the getters to ensure that null values are returned for "water agent" properties when the Misc is set to
   // be a type other than water agent.
   return;
}
void Misc::setUseFor   (QString const & val) { SET_AND_NOTIFY( PropertyNames::Misc::useFor  , this->m_useFor   , val); return; }
void Misc::setNotes    (QString const & val) { SET_AND_NOTIFY( PropertyNames::Misc::notes   , this->m_notes    , val); return; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Misc::setProducer (QString const & val) { SET_AND_NOTIFY(PropertyNames::Misc::producer , this->m_producer , val); return; }
void Misc::setProductId(QString const & val) { SET_AND_NOTIFY(PropertyNames::Misc::productId, this->m_productId, val); return; }
// All below migrated from Salt
void Misc::setWaterAgentType(std::optional<WaterAgentType> const val) {
   SET_AND_NOTIFY(PropertyNames::Misc::waterAgentType, this->m_waterAgentType, val);
   return;
}
void Misc::setWaterAgentTypeAsInt(std::optional<int> const val) {
   SET_AND_NOTIFY(PropertyNames::Misc::waterAgentType, this->m_waterAgentType, Optional::fromOptInt<WaterAgentType>(val));
   return;
}
void Misc::setWaterAgentPercentAcid(std::optional<double> const val) {
   SET_AND_NOTIFY(PropertyNames::Misc::waterAgentPercentAcid, this->m_waterAgentPercentAcid, val);
   return;
}

//=============================================== OTHER MEMBER FUNCTIONS ===============================================

//====== Water Agent Maths ===========
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
//    Let 'S' be the salt, and 'I' be the constituent ion we're interested in.  Call the molecular masses of the salt
//    and the ion 'Ms' and 'Mi' respectively.  We have:
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
// See also Bru'n Water's excellent water knowledge page (previously at
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
// that's two multiplications by 1000.  Inside the functions here we do it to go from parts per thousand to parts per
// million.  The caller typically needs to do it again to go from kilograms to grams.)
//
template<Water::MineralIon ion> double Misc::concentrationPerGramPerLiter_massConcPpm(Misc::WaterAgentType const waterAgentType) {
   // TODO: This is a placeholder.  Ultimately, we should have a specialisation for each ion
   qDebug() << Q_FUNC_INFO << ion << waterAgentType;
   return 0.0;
}
// Instantiate the above template function for the types for which we don't have specialisations below
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Copper     >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Iron       >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Manganese  >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Nitrate    >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Nitrite    >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Phosphate  >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Potassium  >(Misc::WaterAgentType const waterAgentType);
template double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Zinc       >(Misc::WaterAgentType const waterAgentType);

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Calcium>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumChloride  : return (MolarMass::Calcium / MolarMass::CalciumChloride ) * 1000.0;
      case Misc::WaterAgentType::CalciumCarbonate : return (MolarMass::Calcium / MolarMass::CalciumCarbonate) * 1000.0;
      case Misc::WaterAgentType::CalciumSulfate   : return (MolarMass::Calcium / MolarMass::CalciumSulfate  ) * 1000.0;

      case Misc::WaterAgentType::MagnesiumSulfate :
      case Misc::WaterAgentType::SodiumChloride   :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::LacticAcid       :
      case Misc::WaterAgentType::PhosphoricAcid   :
      case Misc::WaterAgentType::Other            :
         return 0.0;
         // No default case as we want the compiler to warn us if we missed one
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Chloride>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumChloride: return (MolarMass::Chloride * 2.0 / MolarMass::CalciumChloride) * 1000.0;
      case Misc::WaterAgentType::SodiumChloride : return (MolarMass::Chloride       / MolarMass::SodiumChloride ) * 1000.0;

      case Misc::WaterAgentType::CalciumCarbonate :
      case Misc::WaterAgentType::CalciumSulfate   :
      case Misc::WaterAgentType::MagnesiumSulfate :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::LacticAcid       :
      case Misc::WaterAgentType::PhosphoricAcid   :
      case Misc::WaterAgentType::Other            :
         return 0.0;
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Carbonate>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumCarbonate: return (MolarMass::Carbonate / MolarMass::CalciumCarbonate) * 1000.0;

      case Misc::WaterAgentType::CalciumChloride  :
      case Misc::WaterAgentType::CalciumSulfate   :
      case Misc::WaterAgentType::MagnesiumSulfate :
      case Misc::WaterAgentType::SodiumChloride   :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::LacticAcid       :
      case Misc::WaterAgentType::PhosphoricAcid   :
      case Misc::WaterAgentType::Other            :
         return 0.0;
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Bicarbonate>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::SodiumBicarbonate: return (MolarMass::Bicarbonate / MolarMass::SodiumBicarbonate) * 1000.0;

      case Misc::WaterAgentType::CalciumChloride :
      case Misc::WaterAgentType::CalciumCarbonate:
      case Misc::WaterAgentType::CalciumSulfate  :
      case Misc::WaterAgentType::MagnesiumSulfate:
      case Misc::WaterAgentType::SodiumChloride  :
      case Misc::WaterAgentType::LacticAcid      :
      case Misc::WaterAgentType::PhosphoricAcid  :
      case Misc::WaterAgentType::Other           :
         return 0.0;
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Magnesium>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::MagnesiumSulfate: return (MolarMass::Magnesium / MolarMass::MagnesiumSulfate) * 1000.0;

      case Misc::WaterAgentType::CalciumChloride  :
      case Misc::WaterAgentType::CalciumCarbonate :
      case Misc::WaterAgentType::CalciumSulfate   :
      case Misc::WaterAgentType::SodiumChloride   :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::LacticAcid       :
      case Misc::WaterAgentType::PhosphoricAcid   :
      case Misc::WaterAgentType::Other            :
         return 0.0;
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Sodium>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::SodiumChloride   : return (MolarMass::Sodium / MolarMass::SodiumChloride   ) * 1000.0;
      case Misc::WaterAgentType::SodiumBicarbonate: return (MolarMass::Sodium / MolarMass::SodiumBicarbonate) * 1000.0;

      case Misc::WaterAgentType::CalciumChloride :
      case Misc::WaterAgentType::CalciumCarbonate:
      case Misc::WaterAgentType::CalciumSulfate  :
      case Misc::WaterAgentType::MagnesiumSulfate:
      case Misc::WaterAgentType::LacticAcid      :
      case Misc::WaterAgentType::PhosphoricAcid  :
      case Misc::WaterAgentType::Other           :
         return 0.0;
   }
   Q_UNREACHABLE();
}

template<> double Misc::concentrationPerGramPerLiter_massConcPpm<Water::MineralIon::Sulfate>(Misc::WaterAgentType const waterAgentType) {
   switch (waterAgentType) {
      case Misc::WaterAgentType::CalciumSulfate  : return (MolarMass::Sulfate / MolarMass::CalciumSulfate  ) * 1000.0;
      case Misc::WaterAgentType::MagnesiumSulfate: return (MolarMass::Sulfate / MolarMass::MagnesiumSulfate) * 1000.0;

      case Misc::WaterAgentType::CalciumChloride  :
      case Misc::WaterAgentType::CalciumCarbonate :
      case Misc::WaterAgentType::SodiumChloride   :
      case Misc::WaterAgentType::SodiumBicarbonate:
      case Misc::WaterAgentType::LacticAcid       :
      case Misc::WaterAgentType::PhosphoricAcid   :
      case Misc::WaterAgentType::Other            :
         return 0.0;
   }
   Q_UNREACHABLE();
}

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Misc)

// Insert the boilerplate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Misc)

// Boilerplate code for FolderPropertyBase
FOLDER_BASE_COMMON_CODE(Misc)