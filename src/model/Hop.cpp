/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Hop.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
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
#include "model/Hop.h"

#include <QDebug>
#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "model/InventoryHop.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

QString Hop::localisedName() { return tr("Hop"); }

// Note that Hop::typeStringMapping and Hop::formStringMapping are as defined by BeerJSON, but we also use them for the
// DB and for the UI.  We can't use them for BeerXML as it only supports subsets of these types.
EnumStringMapping const Hop::formStringMapping {
   {Hop::Form::Leaf   , "leaf"      },
   {Hop::Form::Pellet , "pellet"    },
   {Hop::Form::Plug   , "plug"      },
   {Hop::Form::Extract, "extract"   },
   {Hop::Form::WetLeaf, "leaf (wet)"},
   {Hop::Form::Powder , "powder"    },
};

EnumStringMapping const Hop::formDisplayNames {
   {Hop::Form::Leaf   , tr("Leaf"   )},
   {Hop::Form::Pellet , tr("Pellet" )},
   {Hop::Form::Plug   , tr("Plug"   )},
   {Hop::Form::Extract, tr("Extract")},
   {Hop::Form::WetLeaf, tr("WetLeaf")},
   {Hop::Form::Powder , tr("Powder" )},
};

EnumStringMapping const Hop::typeStringMapping {
   {Hop::Type::Bittering              , "bittering"              },
   {Hop::Type::Aroma                  , "aroma"                  },
   {Hop::Type::AromaAndBittering      , "aroma/bittering"        }, // Previous seralisation (still used for BeerXML) was "Both"
   {Hop::Type::Flavor                 , "flavor"                 },
   {Hop::Type::BitteringAndFlavor     , "bittering/flavor"       },
   {Hop::Type::AromaAndFlavor         , "aroma/flavor"           },
   {Hop::Type::AromaBitteringAndFlavor, "aroma/bittering/flavor" },
};

EnumStringMapping const Hop::typeDisplayNames {
   {Hop::Type::Bittering              , tr("Bittering"                )},
   {Hop::Type::Aroma                  , tr("Aroma"                    )},
   {Hop::Type::AromaAndBittering      , tr("Aroma & Bittering"        )},
   {Hop::Type::Flavor                 , tr("Flavor"                   )},
   {Hop::Type::BitteringAndFlavor     , tr("Bittering & Flavor"       )},
   {Hop::Type::AromaAndFlavor         , tr("Aroma & Flavor"           )},
   {Hop::Type::AromaBitteringAndFlavor, tr("Aroma, Bittering & Flavor")},
};

bool Hop::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Hop const & rhs = static_cast<Hop const &>(other);
   // Base class will already have ensured names are equal
   bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the FermentableBase type
      Utils::AutoCompare(this->m_producer , rhs.m_producer ) &&
      Utils::AutoCompare(this->m_productId, rhs.m_productId) &&
      Utils::AutoCompare(this->m_origin   , rhs.m_origin   ) &&
      Utils::AutoCompare(this->m_year     , rhs.m_year     ) &&
      Utils::AutoCompare(this->m_form     , rhs.m_form     ) &&
      Utils::AutoCompare(this->m_alpha_pct, rhs.m_alpha_pct) &&
      Utils::AutoCompare(this->m_beta_pct , rhs.m_beta_pct )
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&

      // Remaining BeerJSON fields -- excluding inventories

      Utils::AutoCompare(this->m_type              , rhs.m_type              ) &&
      Utils::AutoCompare(this->m_notes             , rhs.m_notes             ) &&
      Utils::AutoCompare(this->m_hsi_pct           , rhs.m_hsi_pct           ) &&
      Utils::AutoCompare(this->m_substitutes       , rhs.m_substitutes       ) &&

      // Oil content
      Utils::AutoCompare(this->m_totalOil_mlPer100g, rhs.m_totalOil_mlPer100g) &&
      Utils::AutoCompare(this->m_humulene_pct      , rhs.m_humulene_pct      ) &&
      Utils::AutoCompare(this->m_caryophyllene_pct , rhs.m_caryophyllene_pct ) &&
      Utils::AutoCompare(this->m_cohumulone_pct    , rhs.m_cohumulone_pct    ) &&
      Utils::AutoCompare(this->m_myrcene_pct       , rhs.m_myrcene_pct       ) &&
      Utils::AutoCompare(this->m_farnesene_pct     , rhs.m_farnesene_pct     ) &&
      Utils::AutoCompare(this->m_geraniol_pct      , rhs.m_geraniol_pct      ) &&
      Utils::AutoCompare(this->m_bPinene_pct       , rhs.m_bPinene_pct       ) &&
      Utils::AutoCompare(this->m_linalool_pct      , rhs.m_linalool_pct      ) &&
      Utils::AutoCompare(this->m_limonene_pct      , rhs.m_limonene_pct      ) &&
      Utils::AutoCompare(this->m_nerol_pct         , rhs.m_nerol_pct         ) &&
      Utils::AutoCompare(this->m_pinene_pct        , rhs.m_pinene_pct        ) &&
      Utils::AutoCompare(this->m_polyphenols_pct   , rhs.m_polyphenols_pct   ) &&
      Utils::AutoCompare(this->m_xanthohumol_pct   , rhs.m_xanthohumol_pct   )
   );
}

ObjectStore & Hop::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Hop>::getInstance();
}

TypeLookup const Hop::typeLookup {
   "Hop",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::alpha_pct         , Hop::m_alpha_pct         , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::form              , Hop::m_form              , NonPhysicalQuantity::Enum         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::beta_pct          , Hop::m_beta_pct          , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::origin            , Hop::m_origin            , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::type              , Hop::m_type              , NonPhysicalQuantity::Enum         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::notes             , Hop::m_notes             , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::hsi_pct           , Hop::m_hsi_pct           , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::substitutes       , Hop::m_substitutes       , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::humulene_pct      , Hop::m_humulene_pct      , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::caryophyllene_pct , Hop::m_caryophyllene_pct , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::cohumulone_pct    , Hop::m_cohumulone_pct    , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::myrcene_pct       , Hop::m_myrcene_pct       , NonPhysicalQuantity::Percentage   ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::totalOil_mlPer100g, Hop::m_totalOil_mlPer100g, NonPhysicalQuantity::Dimensionless), // Not really dimensionless...
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::farnesene_pct     , Hop::m_farnesene_pct     , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::geraniol_pct      , Hop::m_geraniol_pct      , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::bPinene_pct       , Hop::m_bPinene_pct       , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::linalool_pct      , Hop::m_linalool_pct      , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::limonene_pct      , Hop::m_limonene_pct      , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::nerol_pct         , Hop::m_nerol_pct         , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::pinene_pct        , Hop::m_pinene_pct        , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::polyphenols_pct   , Hop::m_polyphenols_pct   , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::xanthohumol_pct   , Hop::m_xanthohumol_pct   , NonPhysicalQuantity::Percentage   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::producer          , Hop::m_producer          , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::productId         , Hop::m_productId         , NonPhysicalQuantity::String       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Hop::year              , Hop::m_year              , NonPhysicalQuantity::String       ),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Hop>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Hop>::value);

Hop::Hop(QString name) :
   Ingredient{name},
   m_alpha_pct         {0.0         },
   m_form              {std::nullopt},
   m_beta_pct          {std::nullopt},
   m_origin            {""          },
   m_type              {std::nullopt},
   m_notes             {""          },
   m_hsi_pct           {0.0         },
   m_substitutes       {""          },
   m_humulene_pct      {0.0         },
   m_caryophyllene_pct {0.0         },
   m_cohumulone_pct    {0.0         },
   m_myrcene_pct       {0.0         },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_totalOil_mlPer100g{std::nullopt},
   m_farnesene_pct     {std::nullopt},
   m_geraniol_pct      {std::nullopt},
   m_bPinene_pct       {std::nullopt},
   m_linalool_pct      {std::nullopt},
   m_limonene_pct      {std::nullopt},
   m_nerol_pct         {std::nullopt},
   m_pinene_pct        {std::nullopt},
   m_polyphenols_pct   {std::nullopt},
   m_xanthohumol_pct   {std::nullopt},
   m_producer          {""          },
   m_productId         {""          },
   m_year              {""          } {
   return;
}

Hop::Hop(NamedParameterBundle const & namedParameterBundle) :
   Ingredient{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_alpha_pct         , namedParameterBundle, PropertyNames::Hop::alpha_pct         ),
   SET_OPT_ENUM_FROM_NPB(m_form   , Hop::Form, namedParameterBundle, PropertyNames::Hop::form              ),
   SET_REGULAR_FROM_NPB (m_beta_pct          , namedParameterBundle, PropertyNames::Hop::beta_pct          , std::nullopt),
   SET_REGULAR_FROM_NPB (m_origin            , namedParameterBundle, PropertyNames::Hop::origin            ),
   SET_OPT_ENUM_FROM_NPB(m_type   , Hop::Type, namedParameterBundle, PropertyNames::Hop::type              ),
   SET_REGULAR_FROM_NPB (m_notes             , namedParameterBundle, PropertyNames::Hop::notes             ),
   SET_REGULAR_FROM_NPB (m_hsi_pct           , namedParameterBundle, PropertyNames::Hop::hsi_pct           ),
   SET_REGULAR_FROM_NPB (m_substitutes       , namedParameterBundle, PropertyNames::Hop::substitutes       ),
   SET_REGULAR_FROM_NPB (m_humulene_pct      , namedParameterBundle, PropertyNames::Hop::humulene_pct      ),
   SET_REGULAR_FROM_NPB (m_caryophyllene_pct , namedParameterBundle, PropertyNames::Hop::caryophyllene_pct ),
   SET_REGULAR_FROM_NPB (m_cohumulone_pct    , namedParameterBundle, PropertyNames::Hop::cohumulone_pct    ),
   SET_REGULAR_FROM_NPB (m_myrcene_pct       , namedParameterBundle, PropertyNames::Hop::myrcene_pct       ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_totalOil_mlPer100g, namedParameterBundle, PropertyNames::Hop::totalOil_mlPer100g, std::nullopt),
   SET_REGULAR_FROM_NPB (m_farnesene_pct     , namedParameterBundle, PropertyNames::Hop::farnesene_pct     , std::nullopt),
   SET_REGULAR_FROM_NPB (m_geraniol_pct      , namedParameterBundle, PropertyNames::Hop::geraniol_pct      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_bPinene_pct       , namedParameterBundle, PropertyNames::Hop::bPinene_pct       , std::nullopt),
   SET_REGULAR_FROM_NPB (m_linalool_pct      , namedParameterBundle, PropertyNames::Hop::linalool_pct      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_limonene_pct      , namedParameterBundle, PropertyNames::Hop::limonene_pct      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_nerol_pct         , namedParameterBundle, PropertyNames::Hop::nerol_pct         , std::nullopt),
   SET_REGULAR_FROM_NPB (m_pinene_pct        , namedParameterBundle, PropertyNames::Hop::pinene_pct        , std::nullopt),
   SET_REGULAR_FROM_NPB (m_polyphenols_pct   , namedParameterBundle, PropertyNames::Hop::polyphenols_pct   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_xanthohumol_pct   , namedParameterBundle, PropertyNames::Hop::xanthohumol_pct   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_producer          , namedParameterBundle, PropertyNames::Hop::producer          , ""          ),
   SET_REGULAR_FROM_NPB (m_productId         , namedParameterBundle, PropertyNames::Hop::productId         , ""          ),
   SET_REGULAR_FROM_NPB (m_year              , namedParameterBundle, PropertyNames::Hop::year              , ""          ) {
   return;
}

Hop::Hop(Hop const & other) :
   Ingredient          {other                     },
   m_alpha_pct         {other.m_alpha_pct         },
   m_form              {other.m_form              },
   m_beta_pct          {other.m_beta_pct          },
   m_origin            {other.m_origin            },
   m_type              {other.m_type              },
   m_notes             {other.m_notes             },
   m_hsi_pct           {other.m_hsi_pct           },
   m_substitutes       {other.m_substitutes       },
   m_humulene_pct      {other.m_humulene_pct      },
   m_caryophyllene_pct {other.m_caryophyllene_pct },
   m_cohumulone_pct    {other.m_cohumulone_pct    },
   m_myrcene_pct       {other.m_myrcene_pct       },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_totalOil_mlPer100g{other.m_totalOil_mlPer100g},
   m_farnesene_pct     {other.m_farnesene_pct     },
   m_geraniol_pct      {other.m_geraniol_pct      },
   m_bPinene_pct       {other.m_bPinene_pct       },
   m_linalool_pct      {other.m_linalool_pct      },
   m_limonene_pct      {other.m_limonene_pct      },
   m_nerol_pct         {other.m_nerol_pct         },
   m_pinene_pct        {other.m_pinene_pct        },
   m_polyphenols_pct   {other.m_polyphenols_pct   },
   m_xanthohumol_pct   {other.m_xanthohumol_pct   },
   m_producer          {other.m_producer          },
   m_productId         {other.m_productId         },
   m_year              {other.m_year              } {
   return;
}

Hop::~Hop() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
double                   Hop::alpha_pct         () const { return this->m_alpha_pct         ; }
std::optional<Hop::Form> Hop::form              () const { return this->m_form              ; }
std::optional<int>       Hop::formAsInt         () const { return Optional::toOptInt(m_form); }
std::optional<double>    Hop::beta_pct          () const { return this->m_beta_pct          ; }
QString                  Hop::origin            () const { return this->m_origin            ; }
QString                  Hop::notes             () const { return this->m_notes             ; }
std::optional<Hop::Type> Hop::type              () const { return this->m_type              ; }
std::optional<int>       Hop::typeAsInt         () const { return Optional::toOptInt(m_type); }
std::optional<double>    Hop::hsi_pct           () const { return this->m_hsi_pct           ; }
QString                  Hop::substitutes       () const { return this->m_substitutes       ; }
std::optional<double>    Hop::humulene_pct      () const { return this->m_humulene_pct      ; }
std::optional<double>    Hop::caryophyllene_pct () const { return this->m_caryophyllene_pct ; }
std::optional<double>    Hop::cohumulone_pct    () const { return this->m_cohumulone_pct    ; }
std::optional<double>    Hop::myrcene_pct       () const { return this->m_myrcene_pct       ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
std::optional<double>    Hop::totalOil_mlPer100g() const { return this->m_totalOil_mlPer100g; }
std::optional<double>    Hop::farnesene_pct     () const { return this->m_farnesene_pct     ; }
std::optional<double>    Hop::geraniol_pct      () const { return this->m_geraniol_pct      ; }
std::optional<double>    Hop::bPinene_pct       () const { return this->m_bPinene_pct       ; }
std::optional<double>    Hop::linalool_pct      () const { return this->m_linalool_pct      ; }
std::optional<double>    Hop::limonene_pct      () const { return this->m_limonene_pct      ; }
std::optional<double>    Hop::nerol_pct         () const { return this->m_nerol_pct         ; }
std::optional<double>    Hop::pinene_pct        () const { return this->m_pinene_pct        ; }
std::optional<double>    Hop::polyphenols_pct   () const { return this->m_polyphenols_pct   ; }
std::optional<double>    Hop::xanthohumol_pct   () const { return this->m_xanthohumol_pct   ; }
QString                  Hop::producer          () const { return this->m_producer          ; }
QString                  Hop::productId         () const { return this->m_productId         ; }
QString                  Hop::year              () const { return this->m_year              ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Hop::setAlpha_pct            (double                   const   val) { SET_AND_NOTIFY(PropertyNames::Hop::alpha_pct            , this->m_alpha_pct            , this->enforceMinAndMax(val, "alpha", 0.0, 100.0)); return; }
void Hop::setForm                 (std::optional<Hop::Form> const   val) { SET_AND_NOTIFY(PropertyNames::Hop::form                 , this->m_form                 , val                                             ); return; }
void Hop::setFormAsInt            (std::optional<int>       const   val) { SET_AND_NOTIFY(PropertyNames::Hop::form                 , this->m_form                 , Optional::fromOptInt<Form>(val)                 ); return; }
void Hop::setBeta_pct             (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::beta_pct             , this->m_beta_pct             , this->enforceMinAndMax(val, "beta",  0.0, 100.0)); return; }
void Hop::setOrigin               (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::Hop::origin               , this->m_origin               , val                                             ); return; }
///void Hop::setAmount               (double                   const   val) { SET_AND_NOTIFY(PropertyNames::Hop::amount               , this->m_amount               , this->enforceMin(val, "amount")                 ); return; } // Deprecated - moved to RecipeAdditionHop  TODO: Remove this, once we have RecipeAdditionHop working
///void Hop::setAmountIsWeight       (bool                     const   val) { SET_AND_NOTIFY(PropertyNames::Hop::amountIsWeight       , this->m_amountIsWeight       , val); return; } // Deprecated - moved to RecipeAdditionHop  TODO: Remove this, once we have RecipeAdditionHop working

///void Hop::setTime_min             (double                   const   val) { SET_AND_NOTIFY(PropertyNames::Hop::time_min             , this->m_time_min             , this->enforceMin      (val, "time")                             ); return; }
void Hop::setNotes                (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::Hop::notes                , this->m_notes                , val                                                             ); return; }
void Hop::setType                 (std::optional<Hop::Type> const   val) { SET_AND_NOTIFY(PropertyNames::Hop::type                 , this->m_type                 , val                                                             ); return; }
void Hop::setTypeAsInt            (std::optional<int>       const   val) { SET_AND_NOTIFY(PropertyNames::Hop::type                 , this->m_type                 , Optional::fromOptInt<Type>(val));                                  return; }
void Hop::setHsi_pct              (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::hsi_pct              , this->m_hsi_pct              , this->enforceMinAndMax(val, "hsi",                   0.0, 100.0)); return; }
void Hop::setSubstitutes          (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::Hop::substitutes          , this->m_substitutes          , val                                                             ); return; }
void Hop::setHumulene_pct         (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::humulene_pct         , this->m_humulene_pct         , this->enforceMinAndMax(val, "humulene",              0.0, 100.0)); return; }
void Hop::setCaryophyllene_pct    (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::caryophyllene_pct    , this->m_caryophyllene_pct    , this->enforceMinAndMax(val, "caryophyllene",         0.0, 100.0)); return; }
void Hop::setCohumulone_pct       (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::cohumulone_pct       , this->m_cohumulone_pct       , this->enforceMinAndMax(val, "cohumulone",            0.0, 100.0)); return; }
void Hop::setMyrcene_pct          (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::myrcene_pct          , this->m_myrcene_pct          , this->enforceMinAndMax(val, "myrcene",               0.0, 100.0)); return; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Hop::setTotalOil_mlPer100g(std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::totalOil_mlPer100g, this->m_totalOil_mlPer100g, this->enforceMinAndMax(val, "totalOil_mlPer100g", 0.0, 100.0)); return; }
void Hop::setFarnesene_pct        (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::farnesene_pct        , this->m_farnesene_pct        , this->enforceMinAndMax(val, "farnesene_pct",         0.0, 100.0)); return; }
void Hop::setGeraniol_pct         (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::geraniol_pct         , this->m_geraniol_pct         , this->enforceMinAndMax(val, "geraniol_pct",          0.0, 100.0)); return; }
void Hop::setBPinene_pct         (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::bPinene_pct         , this->m_bPinene_pct         , this->enforceMinAndMax(val, "bPinene_pct",          0.0, 100.0)); return; }
void Hop::setLinalool_pct         (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::linalool_pct         , this->m_linalool_pct         , this->enforceMinAndMax(val, "linalool_pct",          0.0, 100.0)); return; }
void Hop::setLimonene_pct         (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::limonene_pct         , this->m_limonene_pct         , this->enforceMinAndMax(val, "limonene_pct",          0.0, 100.0)); return; }
void Hop::setNerol_pct            (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::nerol_pct            , this->m_nerol_pct            , this->enforceMinAndMax(val, "nerol_pct",             0.0, 100.0)); return; }
void Hop::setPinene_pct           (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::pinene_pct           , this->m_pinene_pct           , this->enforceMinAndMax(val, "pinene_pct",            0.0, 100.0)); return; }
void Hop::setPolyphenols_pct      (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::polyphenols_pct      , this->m_polyphenols_pct      , this->enforceMinAndMax(val, "polyphenols_pct",       0.0, 100.0)); return; }
void Hop::setXanthohumol_pct      (std::optional<double>    const   val) { SET_AND_NOTIFY(PropertyNames::Hop::xanthohumol_pct      , this->m_xanthohumol_pct      , this->enforceMinAndMax(val, "xanthohumol_pct",       0.0, 100.0)); return; }
void Hop::setProducer             (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::Hop::producer             , this->m_producer             , val                                                             ); return; }
void Hop::setProductId           (QString                  const & val) { SET_AND_NOTIFY(PropertyNames::Hop::productId           , this->m_productId           , val                                                             ); return; }
void Hop::setYear                 (QString                  const   val) { SET_AND_NOTIFY(PropertyNames::Hop::year                 , this->m_year                 , val                                                             ); return; }

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Hop)
