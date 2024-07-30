/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Yeast.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
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
#include "model/Yeast.h"

#include <QDebug>

#include "database/ObjectStoreWrapper.h"
#include "model/InventoryYeast.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "PhysicalConstants.h"

QString Yeast::localisedName() { return tr("Yeast"); }

EnumStringMapping const Yeast::typeStringMapping {
   {Yeast::Type::Ale         , "ale"          },
   {Yeast::Type::Lager       , "lager"        },
   {Yeast::Type::Other       , "other"        },  // Was Wheat / wheat
   {Yeast::Type::Wine        , "wine"         },
   {Yeast::Type::Champagne   , "champagne"    },
   {Yeast::Type::Bacteria    , "bacteria"     },
   {Yeast::Type::Brett       , "brett"        },
   {Yeast::Type::Kveik       , "kveik"        },
   {Yeast::Type::Lacto       , "lacto"        },
   {Yeast::Type::Malolactic  , "malolactic"   },
   {Yeast::Type::MixedCulture, "mixed-culture"},
   {Yeast::Type::Pedio       , "pedio"        },
   {Yeast::Type::Spontaneous , "spontaneous"  },
};

EnumStringMapping const Yeast::typeDisplayNames {
   {Yeast::Type::Ale         , tr("Ale"          )},
   {Yeast::Type::Lager       , tr("Lager"        )},
   {Yeast::Type::Other       , tr("Other"        )},
   {Yeast::Type::Wine        , tr("Wine"         )},
   {Yeast::Type::Champagne   , tr("Champagne"    )},
   {Yeast::Type::Bacteria    , tr("Bacteria"     )},
   {Yeast::Type::Brett       , tr("Brett"        )},
   {Yeast::Type::Kveik       , tr("Kveik"        )},
   {Yeast::Type::Lacto       , tr("Lacto"        )},
   {Yeast::Type::Malolactic  , tr("Malolactic"   )},
   {Yeast::Type::MixedCulture, tr("Mixed-culture")},
   {Yeast::Type::Pedio       , tr("Pedio"        )},
   {Yeast::Type::Spontaneous , tr("Spontaneous"  )},
};

EnumStringMapping const Yeast::formStringMapping {
   {Yeast::Form::Liquid , "liquid" },
   {Yeast::Form::Dry    , "dry"    },
   {Yeast::Form::Slant  , "slant"  },
   {Yeast::Form::Culture, "culture"},
   {Yeast::Form::Dregs  , "dregs"  },
};

EnumStringMapping const Yeast::formDisplayNames  {
   {Yeast::Form::Liquid , tr("Liquid" )},
   {Yeast::Form::Dry    , tr("Dry"    )},
   {Yeast::Form::Slant  , tr("Slant"  )},
   {Yeast::Form::Culture, tr("Culture")},
   {Yeast::Form::Dregs  , tr("Dregs"  )},
};

EnumStringMapping const Yeast::flocculationStringMapping {
   {Yeast::Flocculation::VeryLow   , "very low"   },
   {Yeast::Flocculation::Low       , "low"        },
   {Yeast::Flocculation::MediumLow , "medium low" },
   {Yeast::Flocculation::Medium    , "medium"     },
   {Yeast::Flocculation::MediumHigh, "medium high"},
   {Yeast::Flocculation::High      , "high"       },
   {Yeast::Flocculation::VeryHigh  , "very high"  },
};

EnumStringMapping const Yeast::flocculationDisplayNames {
   {Yeast::Flocculation::VeryLow   , tr("Very Low"   )},
   {Yeast::Flocculation::Low       , tr("Low"        )},
   {Yeast::Flocculation::MediumLow , tr("Medium Low" )},
   {Yeast::Flocculation::Medium    , tr("Medium"     )},
   {Yeast::Flocculation::MediumHigh, tr("Medium High")},
   {Yeast::Flocculation::High      , tr("High"       )},
   {Yeast::Flocculation::VeryHigh  , tr("Very High"  )},
};


bool Yeast::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Yeast const & rhs = static_cast<Yeast const &>(other);
   // Base class will already have ensured names are equal
    bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the FermentableBase type
      this->m_type       == rhs.m_type       &&
      this->m_form       == rhs.m_form       &&
      this->m_laboratory == rhs.m_laboratory && // = producer in BeerJSON
      this->m_productId  == rhs.m_productId
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&

      // Remaining BeerJSON fields -- excluding inventories
      this->m_minTemperature_c          == rhs.m_minTemperature_c          &&
      this->m_maxTemperature_c          == rhs.m_maxTemperature_c          &&
      this->m_alcoholTolerance_pct      == rhs.m_alcoholTolerance_pct      &&
      this->m_flocculation              == rhs.m_flocculation              &&
      this->m_attenuationMin_pct        == rhs.m_attenuationMin_pct        &&
      this->m_attenuationMax_pct        == rhs.m_attenuationMax_pct        &&
      this->m_notes                     == rhs.m_notes                     &&
      this->m_bestFor                   == rhs.m_bestFor                   &&
      this->m_maxReuse                  == rhs.m_maxReuse                  &&
      this->m_phenolicOffFlavorPositive == rhs.m_phenolicOffFlavorPositive &&
      this->m_glucoamylasePositive      == rhs.m_glucoamylasePositive      &&

      this->m_killerProducingK1Toxin    == rhs.m_killerProducingK1Toxin    &&
      this->m_killerProducingK2Toxin    == rhs.m_killerProducingK2Toxin    &&
      this->m_killerProducingK28Toxin   == rhs.m_killerProducingK28Toxin   &&
      this->m_killerProducingKlusToxin  == rhs.m_killerProducingKlusToxin  &&
      this->m_killerNeutral             == rhs.m_killerNeutral
   );
}

ObjectStore & Yeast::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Yeast>::getInstance();
}

TypeLookup const Yeast::typeLookup {
   "Yeast",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::type                     , Yeast::m_type                     ,           NonPhysicalQuantity::Enum          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::form                     , Yeast::m_form                     ,           NonPhysicalQuantity::Enum          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::laboratory               , Yeast::m_laboratory               ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::productId                , Yeast::m_productId                ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::minTemperature_c         , Yeast::m_minTemperature_c         , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::maxTemperature_c         , Yeast::m_maxTemperature_c         , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::flocculation             , Yeast::m_flocculation             ,           NonPhysicalQuantity::Enum          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::notes                    , Yeast::m_notes                    ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::bestFor                  , Yeast::m_bestFor                  ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::maxReuse                 , Yeast::m_maxReuse                 ,           NonPhysicalQuantity::OrdinalNumeral),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::alcoholTolerance_pct     , Yeast::m_alcoholTolerance_pct     ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::attenuationMin_pct       , Yeast::m_attenuationMin_pct       ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::attenuationMax_pct       , Yeast::m_attenuationMax_pct       ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::phenolicOffFlavorPositive, Yeast::m_phenolicOffFlavorPositive,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::glucoamylasePositive     , Yeast::m_glucoamylasePositive     ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::killerProducingK1Toxin   , Yeast::m_killerProducingK1Toxin   ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::killerProducingK2Toxin   , Yeast::m_killerProducingK2Toxin   ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::killerProducingK28Toxin  , Yeast::m_killerProducingK28Toxin  ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::killerProducingKlusToxin , Yeast::m_killerProducingKlusToxin ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Yeast::killerNeutral            , Yeast::m_killerNeutral            ,           NonPhysicalQuantity::Bool          ),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Yeast>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Yeast>::value);


//============================CONSTRUCTORS======================================

Yeast::Yeast(QString name) :
   Ingredient{name},
   m_type                     {Yeast::Type::Ale},
   m_form                     {Yeast::Form::Liquid},
   m_laboratory               {""},
   m_productId                {""},
   m_minTemperature_c         {std::nullopt},
   m_maxTemperature_c         {std::nullopt},
   m_flocculation             {std::nullopt},
   m_notes                    {""},
   m_bestFor                  {""},
   m_maxReuse                 {std::nullopt},
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_alcoholTolerance_pct     {std::nullopt},
   m_attenuationMin_pct       {std::nullopt},
   m_attenuationMax_pct       {std::nullopt},
   m_phenolicOffFlavorPositive{std::nullopt},
   m_glucoamylasePositive     {std::nullopt},
   m_killerProducingK1Toxin   {std::nullopt},
   m_killerProducingK2Toxin   {std::nullopt},
   m_killerProducingK28Toxin  {std::nullopt},
   m_killerProducingKlusToxin {std::nullopt},
   m_killerNeutral            {std::nullopt} {
   return;
}

Yeast::Yeast(NamedParameterBundle const & namedParameterBundle) :
   Ingredient{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_type                             , namedParameterBundle, PropertyNames::Yeast::type                     ),
   SET_REGULAR_FROM_NPB (m_form                             , namedParameterBundle, PropertyNames::Yeast::form                     ),
   SET_REGULAR_FROM_NPB (m_laboratory                       , namedParameterBundle, PropertyNames::Yeast::laboratory               ),
   SET_REGULAR_FROM_NPB (m_productId                        , namedParameterBundle, PropertyNames::Yeast::productId                ),
   SET_REGULAR_FROM_NPB (m_minTemperature_c                 , namedParameterBundle, PropertyNames::Yeast::minTemperature_c         ),
   SET_REGULAR_FROM_NPB (m_maxTemperature_c                 , namedParameterBundle, PropertyNames::Yeast::maxTemperature_c         ),
   SET_OPT_ENUM_FROM_NPB(m_flocculation, Yeast::Flocculation, namedParameterBundle, PropertyNames::Yeast::flocculation             ),
   SET_REGULAR_FROM_NPB (m_notes                            , namedParameterBundle, PropertyNames::Yeast::notes                    ),
   SET_REGULAR_FROM_NPB (m_bestFor                          , namedParameterBundle, PropertyNames::Yeast::bestFor                  ),
   SET_REGULAR_FROM_NPB (m_maxReuse                         , namedParameterBundle, PropertyNames::Yeast::maxReuse                 ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_alcoholTolerance_pct             , namedParameterBundle, PropertyNames::Yeast::alcoholTolerance_pct     ),
   SET_REGULAR_FROM_NPB (m_attenuationMin_pct               , namedParameterBundle, PropertyNames::Yeast::attenuationMin_pct       ),
   SET_REGULAR_FROM_NPB (m_attenuationMax_pct               , namedParameterBundle, PropertyNames::Yeast::attenuationMax_pct       ),
   SET_REGULAR_FROM_NPB (m_phenolicOffFlavorPositive        , namedParameterBundle, PropertyNames::Yeast::phenolicOffFlavorPositive),
   SET_REGULAR_FROM_NPB (m_glucoamylasePositive             , namedParameterBundle, PropertyNames::Yeast::glucoamylasePositive     ),
   SET_REGULAR_FROM_NPB (m_killerProducingK1Toxin           , namedParameterBundle, PropertyNames::Yeast::killerProducingK1Toxin   ),
   SET_REGULAR_FROM_NPB (m_killerProducingK2Toxin           , namedParameterBundle, PropertyNames::Yeast::killerProducingK2Toxin   ),
   SET_REGULAR_FROM_NPB (m_killerProducingK28Toxin          , namedParameterBundle, PropertyNames::Yeast::killerProducingK28Toxin  ),
   SET_REGULAR_FROM_NPB (m_killerProducingKlusToxin         , namedParameterBundle, PropertyNames::Yeast::killerProducingKlusToxin ),
   SET_REGULAR_FROM_NPB (m_killerNeutral                    , namedParameterBundle, PropertyNames::Yeast::killerNeutral            ) {
   return;
}

Yeast::Yeast(Yeast const & other) :
   Ingredient{other                        },
   m_type                     {other.m_type                     },
   m_form                     {other.m_form                     },
   m_laboratory               {other.m_laboratory               },
   m_productId                {other.m_productId                },
   m_minTemperature_c         {other.m_minTemperature_c         },
   m_maxTemperature_c         {other.m_maxTemperature_c         },
   m_flocculation             {other.m_flocculation             },
   m_notes                    {other.m_notes                    },
   m_bestFor                  {other.m_bestFor                  },
   m_maxReuse                 {other.m_maxReuse                 },
   m_alcoholTolerance_pct     {other.m_alcoholTolerance_pct     },
   m_attenuationMin_pct       {other.m_attenuationMin_pct       },
   m_attenuationMax_pct       {other.m_attenuationMax_pct       },
   m_phenolicOffFlavorPositive{other.m_phenolicOffFlavorPositive},
   m_glucoamylasePositive     {other.m_glucoamylasePositive     },
   m_killerProducingK1Toxin   {other.m_killerProducingK1Toxin   },
   m_killerProducingK2Toxin   {other.m_killerProducingK2Toxin   },
   m_killerProducingK28Toxin  {other.m_killerProducingK28Toxin  },
   m_killerProducingKlusToxin {other.m_killerProducingKlusToxin },
   m_killerNeutral            {other.m_killerNeutral            } {
   return;
}

Yeast::~Yeast() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Yeast::Type                        Yeast::type                     () const { return                    m_type                     ; }
Yeast::Form                        Yeast::form                     () const { return                    m_form                     ; }
QString                            Yeast::laboratory               () const { return                    m_laboratory               ; }
QString                            Yeast::productId                () const { return                    m_productId                ; }
std::optional<double>              Yeast::minTemperature_c         () const { return                    m_minTemperature_c         ; } // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
std::optional<double>              Yeast::maxTemperature_c         () const { return                    m_maxTemperature_c         ; } // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
std::optional<Yeast::Flocculation> Yeast::flocculation             () const { return                    m_flocculation             ; } // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
std::optional<int>                 Yeast::flocculationAsInt        () const { return Optional::toOptInt(m_flocculation)            ; } // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
QString                            Yeast::notes                    () const { return                    m_notes                    ; }
QString                            Yeast::bestFor                  () const { return                    m_bestFor                  ; }
std::optional<int>                 Yeast::maxReuse                 () const { return                    m_maxReuse                 ; } // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
std::optional<double>              Yeast::alcoholTolerance_pct     () const { return                    m_alcoholTolerance_pct     ; }
std::optional<double>              Yeast::attenuationMin_pct       () const { return                    m_attenuationMin_pct       ; }
std::optional<double>              Yeast::attenuationMax_pct       () const { return                    m_attenuationMax_pct       ; }
std::optional<bool>                Yeast::phenolicOffFlavorPositive() const { return                    m_phenolicOffFlavorPositive; }
std::optional<bool>                Yeast::glucoamylasePositive     () const { return                    m_glucoamylasePositive     ; }
std::optional<bool>                Yeast::killerProducingK1Toxin   () const { return                    m_killerProducingK1Toxin   ; }
std::optional<bool>                Yeast::killerProducingK2Toxin   () const { return                    m_killerProducingK2Toxin   ; }
std::optional<bool>                Yeast::killerProducingK28Toxin  () const { return                    m_killerProducingK28Toxin  ; }
std::optional<bool>                Yeast::killerProducingKlusToxin () const { return                    m_killerProducingKlusToxin ; }
std::optional<bool>                Yeast::killerNeutral            () const { return                    m_killerNeutral            ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
// It seems a bit of overkill to enforce absolute zero as the lowest allowable temperature, but we do
void Yeast::setType                     (Yeast::Type                 const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::type                     , m_type            , val); return; }
void Yeast::setForm                     (Yeast::Form                 const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::form                     , m_form            , val); return; }
void Yeast::setLaboratory               (QString                     const & val) { SET_AND_NOTIFY(PropertyNames::Yeast::laboratory               , m_laboratory      , val); return; }
void Yeast::setProductId                (QString                     const & val) { SET_AND_NOTIFY(PropertyNames::Yeast::productId                , m_productId       , val); return; }
void Yeast::setMinTemperature_c         (std::optional<double>       const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::minTemperature_c         , m_minTemperature_c, this->enforceMin      (val, "max temp"       , PhysicalConstants::absoluteZero, 0.0  )); return; }
void Yeast::setMaxTemperature_c         (std::optional<double>       const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::maxTemperature_c         , m_maxTemperature_c, this->enforceMin      (val, "max temp"       , PhysicalConstants::absoluteZero, 0.0  )); return; }
void Yeast::setFlocculation             (std::optional<Flocculation> const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::flocculation             , m_flocculation    , val); return; }
void Yeast::setFlocculationAsInt        (std::optional<int>          const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::flocculation             , m_flocculation    , Optional::fromOptInt<Flocculation>(val)); return; }
void Yeast::setNotes                    (QString                     const & val) { SET_AND_NOTIFY(PropertyNames::Yeast::notes                    , m_notes           , val); return; }
void Yeast::setBestFor                  (QString                     const & val) { SET_AND_NOTIFY(PropertyNames::Yeast::bestFor                  , m_bestFor         , val); return; }
void Yeast::setMaxReuse                 (std::optional<int>          const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::maxReuse                 , m_maxReuse        , this->enforceMin      (val, "max reuse"      )); return; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Yeast::setAlcoholTolerance_pct     (std::optional<double>       const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::alcoholTolerance_pct     , m_alcoholTolerance_pct     , val); return; }
void Yeast::setAttenuationMin_pct       (std::optional<double>       const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::attenuationMin_pct       , m_attenuationMin_pct       , val); return; }
void Yeast::setAttenuationMax_pct       (std::optional<double>       const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::attenuationMax_pct       , m_attenuationMax_pct       , val); return; }
void Yeast::setPhenolicOffFlavorPositive(std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::phenolicOffFlavorPositive, m_phenolicOffFlavorPositive, val); return; }
void Yeast::setGlucoamylasePositive     (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::glucoamylasePositive     , m_glucoamylasePositive     , val); return; }
void Yeast::setKillerProducingK1Toxin   (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::killerProducingK1Toxin   , m_killerProducingK1Toxin   , val); return; }
void Yeast::setKillerProducingK2Toxin   (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::killerProducingK2Toxin   , m_killerProducingK2Toxin   , val); return; }
void Yeast::setKillerProducingK28Toxin  (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::killerProducingK28Toxin  , m_killerProducingK28Toxin  , val); return; }
void Yeast::setKillerProducingKlusToxin (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::killerProducingKlusToxin , m_killerProducingKlusToxin , val); return; }
void Yeast::setKillerNeutral            (std::optional<bool>         const   val) { SET_AND_NOTIFY(PropertyNames::Yeast::killerNeutral            , m_killerNeutral            , val); return; }

double Yeast::getTypicalAttenuation_pct() const {
   if (m_attenuationMin_pct && m_attenuationMax_pct) {
      return (*m_attenuationMin_pct + *m_attenuationMax_pct) / 2.0;
   }
   return Yeast::DefaultAttenuation_pct;
}

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Yeast)
