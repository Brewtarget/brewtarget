/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentable.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Blair Bonnett <blair.bonnett@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Kregg Kemper <gigatropolis@yahoo.com>
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
#include "model/Fermentable.h"

#include <QDebug>
#include <QObject>
#include <QVariant>

#include "database/ObjectStoreWrapper.h"
#include "model/Inventory.h"
#include "model/InventoryFermentable.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"
#include "utils/OptionalHelpers.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_Fermentable.cpp"

QString Fermentable::localisedName() { return tr("Fermentable"); }

// Note that Fermentable::typeStringMapping and Fermentable::grainGroupStringMapping are as defined by BeerJSON, but we
// also use them for the DB and for the UI.  We can't use them for BeerXML as it only supports subsets of these types.
EnumStringMapping const Fermentable::typeStringMapping {
   {Fermentable::Type::Grain        , "grain"      },
   {Fermentable::Type::Sugar        , "sugar"      },
   {Fermentable::Type::Extract      , "extract"    },
   {Fermentable::Type::Dry_Extract  , "dry extract"},
   {Fermentable::Type::Other_Adjunct, "other"      },
   {Fermentable::Type::Fruit        , "fruit"      },
   {Fermentable::Type::Juice        , "juice"      },
   {Fermentable::Type::Honey        , "honey"      },
};

EnumStringMapping const Fermentable::typeDisplayNames {
   {Fermentable::Type::Grain        , tr("Grain"        )},
   {Fermentable::Type::Sugar        , tr("Sugar"        )},
   {Fermentable::Type::Extract      , tr("Extract"      )},
   {Fermentable::Type::Dry_Extract  , tr("Dry Extract"  )},
   {Fermentable::Type::Other_Adjunct, tr("Other Adjunct")},
   {Fermentable::Type::Fruit        , tr("Fruit"        )},
   {Fermentable::Type::Juice        , tr("Juice"        )},
   {Fermentable::Type::Honey        , tr("Honey"        )},
};

// This is based on the BeerJSON encoding
EnumStringMapping const Fermentable::grainGroupStringMapping {
   {Fermentable::GrainGroup::Base     , "base"     },
   {Fermentable::GrainGroup::Caramel  , "caramel"  },
   {Fermentable::GrainGroup::Flaked   , "flaked"   },
   {Fermentable::GrainGroup::Roasted  , "roasted"  },
   {Fermentable::GrainGroup::Specialty, "specialty"},
   {Fermentable::GrainGroup::Smoked   , "smoked"   },
   {Fermentable::GrainGroup::Adjunct  , "adjunct"  },
};

EnumStringMapping const Fermentable::grainGroupDisplayNames {
   {Fermentable::GrainGroup::Base     , tr("Base"     )},
   {Fermentable::GrainGroup::Caramel  , tr("Caramel"  )},
   {Fermentable::GrainGroup::Flaked   , tr("Flaked"   )},
   {Fermentable::GrainGroup::Roasted  , tr("Roasted"  )},
   {Fermentable::GrainGroup::Specialty, tr("Specialty")},
   {Fermentable::GrainGroup::Smoked   , tr("Smoked"   )},
   {Fermentable::GrainGroup::Adjunct  , tr("Adjunct"  )},
};

bool Fermentable::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentable const & rhs = static_cast<Fermentable const &>(other);

   // Base class will already have ensured names are equal
   bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the FermentableBase type

      AUTO_LOG_COMPARE(this, rhs, m_type      ) &&
      AUTO_LOG_COMPARE(this, rhs, m_origin    ) &&
      AUTO_LOG_COMPARE(this, rhs, m_color_srm ) &&
      AUTO_LOG_COMPARE(this, rhs, m_producer  ) &&
      AUTO_LOG_COMPARE(this, rhs, m_productId ) &&
      AUTO_LOG_COMPARE(this, rhs, m_grainGroup) &&

      // Yield
      AUTO_LOG_COMPARE(this, rhs, m_fineGrindYield_pct  ) &&
      AUTO_LOG_COMPARE(this, rhs, m_coarseGrindYield_pct) &&
      AUTO_LOG_COMPARE(this, rhs, m_coarseFineDiff_pct  ) &&
      AUTO_LOG_COMPARE(this, rhs, m_potentialYield_sg   ) &&

      AUTO_LOG_COMPARE(this, rhs, m_color_srm)
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&

      // Remaining BeerJSON fields -- excluding inventories
      AUTO_LOG_COMPARE(this, rhs, m_notes                 ) &&
      AUTO_LOG_COMPARE(this, rhs, m_moisture_pct          ) &&
      AUTO_LOG_COMPARE(this, rhs, m_alphaAmylase_dextUnits) &&
      AUTO_LOG_COMPARE(this, rhs, m_diastaticPower_lintner) &&
      AUTO_LOG_COMPARE(this, rhs, m_protein_pct           ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kolbachIndex_pct      ) &&
      AUTO_LOG_COMPARE(this, rhs, m_maxInBatch_pct        ) &&
      AUTO_LOG_COMPARE(this, rhs, m_recommendMash         ) &&
      AUTO_LOG_COMPARE(this, rhs, m_hardnessPrpGlassy_pct ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kernelSizePrpPlump_pct) &&
      AUTO_LOG_COMPARE(this, rhs, m_hardnessPrpHalf_pct   ) &&
      AUTO_LOG_COMPARE(this, rhs, m_hardnessPrpMealy_pct  ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kernelSizePrpThin_pct ) &&
      AUTO_LOG_COMPARE(this, rhs, m_friability_pct        ) &&
      AUTO_LOG_COMPARE(this, rhs, m_di_ph                 ) &&
      AUTO_LOG_COMPARE(this, rhs, m_viscosity_cP          ) &&
      AUTO_LOG_COMPARE(this, rhs, m_dmsP_ppm              ) &&
      AUTO_LOG_COMPARE(this, rhs, m_fan_ppm               ) &&
      AUTO_LOG_COMPARE(this, rhs, m_fermentability_pct    ) &&
      AUTO_LOG_COMPARE(this, rhs, m_betaGlucan_ppm        ) &&

      // Non-BeerJSON fields
      AUTO_LOG_COMPARE(this, rhs, m_supplier)
   );
}

ObjectStore & Fermentable::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentable>::getInstance();
}

TypeLookup const Fermentable::typeLookup {
   "Fermentable",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::type                     , Fermentable::m_type                     ,           NonPhysicalQuantity::Enum           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::color_srm                , Fermentable::m_color_srm                , Measurement::PhysicalQuantity::Color          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::origin                   , Fermentable::m_origin                   ,           NonPhysicalQuantity::String         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::supplier                 , Fermentable::m_supplier                 ,           NonPhysicalQuantity::String         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::notes                    , Fermentable::m_notes                    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::coarseFineDiff_pct       , Fermentable::m_coarseFineDiff_pct       ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::moisture_pct             , Fermentable::m_moisture_pct             ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::diastaticPower_lintner   , Fermentable::m_diastaticPower_lintner   , Measurement::PhysicalQuantity::DiastaticPower ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::protein_pct              , Fermentable::m_protein_pct              ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::maxInBatch_pct           , Fermentable::m_maxInBatch_pct           ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::recommendMash            , Fermentable::m_recommendMash            ,           NonPhysicalQuantity::Bool           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::ibuGalPerLb              , Fermentable::m_ibuGalPerLb              ,           NonPhysicalQuantity::Dimensionless  ), // Not really dimensionless...
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::grainGroup               , Fermentable::m_grainGroup               ,           NonPhysicalQuantity::Enum           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::producer                 , Fermentable::m_producer                 ,           NonPhysicalQuantity::String         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::productId                , Fermentable::m_productId                ,           NonPhysicalQuantity::String         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::fineGrindYield_pct       , Fermentable::m_fineGrindYield_pct       ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::coarseGrindYield_pct     , Fermentable::m_coarseGrindYield_pct     ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::potentialYield_sg        , Fermentable::m_potentialYield_sg        , Measurement::PhysicalQuantity::Density        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::alphaAmylase_dextUnits   , Fermentable::m_alphaAmylase_dextUnits   ,           NonPhysicalQuantity::Dimensionless  ), // Not really dimensionless...
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::kolbachIndex_pct         , Fermentable::m_kolbachIndex_pct         ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::hardnessPrpGlassy_pct    , Fermentable::m_hardnessPrpGlassy_pct    ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::hardnessPrpHalf_pct      , Fermentable::m_hardnessPrpHalf_pct      ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::hardnessPrpMealy_pct     , Fermentable::m_hardnessPrpMealy_pct     ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::kernelSizePrpPlump_pct   , Fermentable::m_kernelSizePrpPlump_pct   ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::kernelSizePrpThin_pct    , Fermentable::m_kernelSizePrpThin_pct    ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::friability_pct           , Fermentable::m_friability_pct           ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::di_ph                    , Fermentable::m_di_ph                    , Measurement::PhysicalQuantity::Acidity        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::viscosity_cP             , Fermentable::m_viscosity_cP             , Measurement::PhysicalQuantity::Viscosity      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::dmsP_ppm                 , Fermentable::m_dmsP_ppm                 , Measurement::PhysicalQuantity::MassFractionOrConc),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::fan_ppm                  , Fermentable::m_fan_ppm                  , Measurement::PhysicalQuantity::MassFractionOrConc),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::fermentability_pct       , Fermentable::m_fermentability_pct       ,           NonPhysicalQuantity::Percentage     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Fermentable::betaGlucan_ppm           , Fermentable::m_betaGlucan_ppm           , Measurement::PhysicalQuantity::MassFractionOrConc),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Fermentable>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Fermentable>::value);

Fermentable::Fermentable(QString name) :
   Ingredient                 {name},
   m_type                     {Fermentable::Type::Grain},
   m_color_srm                {0.0                     },
   m_origin                   {""                      },
   m_supplier                 {""                      },
   m_notes                    {""                      },
   m_coarseFineDiff_pct       {std::nullopt            },
   m_moisture_pct             {std::nullopt            },
   m_diastaticPower_lintner   {std::nullopt            },
   m_protein_pct              {std::nullopt            },
   m_maxInBatch_pct           {std::nullopt            },
   m_recommendMash            {std::nullopt            },
   m_ibuGalPerLb              {std::nullopt            },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_grainGroup               {std::nullopt            },
   m_producer                 {""                      },
   m_productId                {""                      },
   m_fineGrindYield_pct       {std::nullopt            },
   m_coarseGrindYield_pct     {std::nullopt            },
   m_potentialYield_sg        {std::nullopt            },
   m_alphaAmylase_dextUnits   {std::nullopt            },
   m_kolbachIndex_pct         {std::nullopt            },
   m_hardnessPrpGlassy_pct    {std::nullopt            },
   m_hardnessPrpHalf_pct      {std::nullopt            },
   m_hardnessPrpMealy_pct     {std::nullopt            },
   m_kernelSizePrpPlump_pct   {std::nullopt            },
   m_kernelSizePrpThin_pct    {std::nullopt            },
   m_friability_pct           {std::nullopt            },
   m_di_ph                    {std::nullopt            },
   m_viscosity_cP             {std::nullopt            },
   m_dmsP_ppm                 {std::nullopt            },
   m_fan_ppm                  {std::nullopt            },
   m_fermentability_pct       {std::nullopt            },
   m_betaGlucan_ppm           {std::nullopt            } {

   CONSTRUCTOR_END
   return;
}

Fermentable::Fermentable(NamedParameterBundle const & namedParameterBundle) :
   Ingredient   {namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_type                               , namedParameterBundle, PropertyNames::Fermentable::type                             ),
   SET_REGULAR_FROM_NPB (m_color_srm                          , namedParameterBundle, PropertyNames::Fermentable::color_srm                        ),
   SET_REGULAR_FROM_NPB (m_origin                             , namedParameterBundle, PropertyNames::Fermentable::origin                , QString()),
   SET_REGULAR_FROM_NPB (m_supplier                           , namedParameterBundle, PropertyNames::Fermentable::supplier              , QString()),
   SET_REGULAR_FROM_NPB (m_notes                              , namedParameterBundle, PropertyNames::Fermentable::notes                 , QString()),
   SET_REGULAR_FROM_NPB (m_coarseFineDiff_pct                 , namedParameterBundle, PropertyNames::Fermentable::coarseFineDiff_pct               ),
   SET_REGULAR_FROM_NPB (m_moisture_pct                       , namedParameterBundle, PropertyNames::Fermentable::moisture_pct                     ),
   SET_REGULAR_FROM_NPB (m_diastaticPower_lintner             , namedParameterBundle, PropertyNames::Fermentable::diastaticPower_lintner           ),
   SET_REGULAR_FROM_NPB (m_protein_pct                        , namedParameterBundle, PropertyNames::Fermentable::protein_pct                      ),
   SET_REGULAR_FROM_NPB (m_maxInBatch_pct                     , namedParameterBundle, PropertyNames::Fermentable::maxInBatch_pct                   ),
   SET_REGULAR_FROM_NPB (m_recommendMash                      , namedParameterBundle, PropertyNames::Fermentable::recommendMash                    ),
   SET_REGULAR_FROM_NPB (m_ibuGalPerLb                        , namedParameterBundle, PropertyNames::Fermentable::ibuGalPerLb                      ),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_OPT_ENUM_FROM_NPB(m_grainGroup, Fermentable::GrainGroup, namedParameterBundle, PropertyNames::Fermentable::grainGroup            ),
   SET_REGULAR_FROM_NPB (m_producer                           , namedParameterBundle, PropertyNames::Fermentable::producer              , QString()   ),
   SET_REGULAR_FROM_NPB (m_productId                          , namedParameterBundle, PropertyNames::Fermentable::productId             , QString()   ),
   SET_REGULAR_FROM_NPB (m_fineGrindYield_pct                 , namedParameterBundle, PropertyNames::Fermentable::fineGrindYield_pct    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_coarseGrindYield_pct               , namedParameterBundle, PropertyNames::Fermentable::coarseGrindYield_pct  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_potentialYield_sg                  , namedParameterBundle, PropertyNames::Fermentable::potentialYield_sg     , std::nullopt),
   SET_REGULAR_FROM_NPB (m_alphaAmylase_dextUnits             , namedParameterBundle, PropertyNames::Fermentable::alphaAmylase_dextUnits, std::nullopt),
   SET_REGULAR_FROM_NPB (m_kolbachIndex_pct                   , namedParameterBundle, PropertyNames::Fermentable::kolbachIndex_pct      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hardnessPrpGlassy_pct              , namedParameterBundle, PropertyNames::Fermentable::hardnessPrpGlassy_pct , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hardnessPrpHalf_pct                , namedParameterBundle, PropertyNames::Fermentable::hardnessPrpHalf_pct   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hardnessPrpMealy_pct               , namedParameterBundle, PropertyNames::Fermentable::hardnessPrpMealy_pct  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_kernelSizePrpPlump_pct             , namedParameterBundle, PropertyNames::Fermentable::kernelSizePrpPlump_pct, std::nullopt),
   SET_REGULAR_FROM_NPB (m_kernelSizePrpThin_pct              , namedParameterBundle, PropertyNames::Fermentable::kernelSizePrpThin_pct , std::nullopt),
   SET_REGULAR_FROM_NPB (m_friability_pct                     , namedParameterBundle, PropertyNames::Fermentable::friability_pct        , std::nullopt),
   SET_REGULAR_FROM_NPB (m_di_ph                              , namedParameterBundle, PropertyNames::Fermentable::di_ph                 , std::nullopt),
   SET_REGULAR_FROM_NPB (m_viscosity_cP                       , namedParameterBundle, PropertyNames::Fermentable::viscosity_cP          , std::nullopt),
   SET_REGULAR_FROM_NPB (m_dmsP_ppm                           , namedParameterBundle, PropertyNames::Fermentable::dmsP_ppm              , std::nullopt),
   SET_REGULAR_FROM_NPB (m_fan_ppm                            , namedParameterBundle, PropertyNames::Fermentable::fan_ppm               , std::nullopt),
   SET_REGULAR_FROM_NPB (m_fermentability_pct                 , namedParameterBundle, PropertyNames::Fermentable::fermentability_pct    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_betaGlucan_ppm                     , namedParameterBundle, PropertyNames::Fermentable::betaGlucan_ppm        , std::nullopt) {

   CONSTRUCTOR_END
   return;
}

Fermentable::Fermentable(Fermentable const & other) :
   Ingredient                 {other                         },
   m_type                     {other.m_type                  },
   m_color_srm                {other.m_color_srm             },
   m_origin                   {other.m_origin                },
   m_supplier                 {other.m_supplier              },
   m_notes                    {other.m_notes                 },
   m_coarseFineDiff_pct       {other.m_coarseFineDiff_pct    },
   m_moisture_pct             {other.m_moisture_pct          },
   m_diastaticPower_lintner   {other.m_diastaticPower_lintner},
   m_protein_pct              {other.m_protein_pct           },
   m_maxInBatch_pct           {other.m_maxInBatch_pct        },
   m_recommendMash            {other.m_recommendMash         },
   m_ibuGalPerLb              {other.m_ibuGalPerLb           },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_grainGroup               {other.m_grainGroup            },
   m_producer                 {other.m_producer              },
   m_productId                {other.m_productId             },
   m_fineGrindYield_pct       {other.m_fineGrindYield_pct    },
   m_coarseGrindYield_pct     {other.m_coarseGrindYield_pct  },
   m_potentialYield_sg        {other.m_potentialYield_sg     },
   m_alphaAmylase_dextUnits   {other.m_alphaAmylase_dextUnits},
   m_kolbachIndex_pct         {other.m_kolbachIndex_pct      },
   m_hardnessPrpGlassy_pct    {other.m_hardnessPrpGlassy_pct },
   m_hardnessPrpHalf_pct      {other.m_hardnessPrpHalf_pct   },
   m_hardnessPrpMealy_pct     {other.m_hardnessPrpMealy_pct  },
   m_kernelSizePrpPlump_pct   {other.m_kernelSizePrpPlump_pct},
   m_kernelSizePrpThin_pct    {other.m_kernelSizePrpThin_pct },
   m_friability_pct           {other.m_friability_pct        },
   m_di_ph                    {other.m_di_ph                 },
   m_viscosity_cP             {other.m_viscosity_cP          },
   m_dmsP_ppm                 {other.m_dmsP_ppm              },
   m_fan_ppm                  {other.m_fan_ppm               },
   m_fermentability_pct       {other.m_fermentability_pct    },
   m_betaGlucan_ppm           {other.m_betaGlucan_ppm        } {

   CONSTRUCTOR_END
   return;
}

Fermentable::~Fermentable() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Fermentable::Type                      Fermentable::type                     () const { return                    this->m_type                     ; }
double                                 Fermentable::color_srm                () const { return                    this->m_color_srm                ; }
QString                                Fermentable::origin                   () const { return                    this->m_origin                   ; }
QString                                Fermentable::supplier                 () const { return                    this->m_supplier                 ; }
QString                                Fermentable::notes                    () const { return                    this->m_notes                    ; }
std::optional<double>                  Fermentable::coarseFineDiff_pct       () const { return                    this->m_coarseFineDiff_pct       ; }
std::optional<double>                  Fermentable::moisture_pct             () const { return                    this->m_moisture_pct             ; }
std::optional<double>                  Fermentable::diastaticPower_lintner   () const { return                    this->m_diastaticPower_lintner   ; }
std::optional<double>                  Fermentable::protein_pct              () const { return                    this->m_protein_pct              ; }
std::optional<double>                  Fermentable::maxInBatch_pct           () const { return                    this->m_maxInBatch_pct           ; }
std::optional<bool>                    Fermentable::recommendMash            () const { return                    this->m_recommendMash            ; }
std::optional<double>                  Fermentable::ibuGalPerLb              () const { return                    this->m_ibuGalPerLb              ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
std::optional<Fermentable::GrainGroup> Fermentable::grainGroup               () const { return                    this->m_grainGroup               ; }
std::optional<int>                     Fermentable::grainGroupAsInt          () const { return Optional::toOptInt(this->m_grainGroup)              ; }
QString                                Fermentable::producer                 () const { return                    this->m_producer                 ; }
QString                                Fermentable::productId                () const { return                    this->m_productId                ; }
std::optional<double>                  Fermentable::fineGrindYield_pct       () const { return                    this->m_fineGrindYield_pct       ; }
std::optional<double>                  Fermentable::coarseGrindYield_pct     () const { return                    this->m_coarseGrindYield_pct     ; }
std::optional<double>                  Fermentable::potentialYield_sg        () const { return                    this->m_potentialYield_sg        ; }
std::optional<double>                  Fermentable::alphaAmylase_dextUnits   () const { return                    this->m_alphaAmylase_dextUnits   ; }
std::optional<double>                  Fermentable::kolbachIndex_pct         () const { return                    this->m_kolbachIndex_pct         ; }
std::optional<double>                  Fermentable::hardnessPrpGlassy_pct    () const { return                    this->m_hardnessPrpGlassy_pct    ; }
std::optional<double>                  Fermentable::hardnessPrpHalf_pct      () const { return                    this->m_hardnessPrpHalf_pct      ; }
std::optional<double>                  Fermentable::hardnessPrpMealy_pct     () const { return                    this->m_hardnessPrpMealy_pct     ; }
std::optional<double>                  Fermentable::kernelSizePrpPlump_pct   () const { return                    this->m_kernelSizePrpPlump_pct   ; }
std::optional<double>                  Fermentable::kernelSizePrpThin_pct    () const { return                    this->m_kernelSizePrpThin_pct    ; }
std::optional<double>                  Fermentable::friability_pct           () const { return                    this->m_friability_pct           ; }
std::optional<double>                  Fermentable::di_ph                    () const { return                    this->m_di_ph                    ; }
std::optional<double>                  Fermentable::viscosity_cP             () const { return                    this->m_viscosity_cP             ; }
std::optional<double>                  Fermentable::dmsP_ppm                 () const { return                    this->m_dmsP_ppm                 ; }
std::optional<double>                  Fermentable::fan_ppm                  () const { return                    this->m_fan_ppm                  ; }
std::optional<double>                  Fermentable::fermentability_pct       () const { return                    this->m_fermentability_pct       ; }
std::optional<double>                  Fermentable::betaGlucan_ppm           () const { return                    this->m_betaGlucan_ppm           ; }

bool Fermentable::isExtract() const {
   return ((type() == Fermentable::Type::Extract) || (type() == Fermentable::Type::Dry_Extract));
}

bool Fermentable::isSugar() const {
   return (type() == Fermentable::Type::Sugar);
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Fermentable::setType                     (Type                      const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::type                     , this->m_type                     , val); return; }
void Fermentable::setOrigin                   (QString                   const & val) { SET_AND_NOTIFY(PropertyNames::Fermentable::origin                   , this->m_origin                   , val); return; }
void Fermentable::setSupplier                 (QString                   const & val) { SET_AND_NOTIFY(PropertyNames::Fermentable::supplier                 , this->m_supplier                 , val); return; }
void Fermentable::setNotes                    (QString                   const & val) { SET_AND_NOTIFY(PropertyNames::Fermentable::notes                    , this->m_notes                    , val); return; }
void Fermentable::setRecommendMash            (std::optional<bool>       const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::recommendMash            , this->m_recommendMash            , val); return; }
void Fermentable::setIbuGalPerLb              (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::ibuGalPerLb              , this->m_ibuGalPerLb              , val); return; }
void Fermentable::setColor_srm                (double                    const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::color_srm                , this->m_color_srm                , this->enforceMin      (val, "color"));                      return; }
void Fermentable::setCoarseFineDiff_pct       (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::coarseFineDiff_pct       , this->m_coarseFineDiff_pct       , this->enforceMinAndMax(val, "coarseFineDiff", 0.0, 100.0)); return; }
void Fermentable::setMoisture_pct             (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::moisture_pct             , this->m_moisture_pct             , this->enforceMinAndMax(val, "moisture",       0.0, 100.0)); return; }
void Fermentable::setDiastaticPower_lintner   (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::diastaticPower_lintner   , this->m_diastaticPower_lintner   , this->enforceMin      (val, "diastatic power"));            return; }
void Fermentable::setProtein_pct              (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::protein_pct              , this->m_protein_pct              , this->enforceMinAndMax(val, "protein",        0.0, 100.0)); return; }
void Fermentable::setMaxInBatch_pct           (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::maxInBatch_pct           , this->m_maxInBatch_pct           , this->enforceMinAndMax(val, "max in batch",   0.0, 100.0)); return; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Fermentable::setGrainGroup               (std::optional<GrainGroup> const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::grainGroup               , this->m_grainGroup               , val                                  ); return; }
void Fermentable::setGrainGroupAsInt          (std::optional<int>        const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::grainGroup               , this->m_grainGroup               , Optional::fromOptInt<GrainGroup>(val)); return; }
void Fermentable::setProducer                 (QString                   const & val) { SET_AND_NOTIFY(PropertyNames::Fermentable::producer                 , this->m_producer                 , val                                  ); return; }
void Fermentable::setProductId                (QString                   const & val) { SET_AND_NOTIFY(PropertyNames::Fermentable::productId                , this->m_productId                , val                                  ); return; }
void Fermentable::setFineGrindYield_pct       (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::fineGrindYield_pct       , this->m_fineGrindYield_pct       , val                                  ); return; }
void Fermentable::setCoarseGrindYield_pct     (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::coarseGrindYield_pct     , this->m_coarseGrindYield_pct     , val                                  ); return; }
void Fermentable::setPotentialYield_sg        (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::potentialYield_sg        , this->m_potentialYield_sg        , val                                  ); return; }
void Fermentable::setAlphaAmylase_dextUnits   (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::alphaAmylase_dextUnits   , this->m_alphaAmylase_dextUnits   , val                                  ); return; }
void Fermentable::setKolbachIndex_pct         (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::kolbachIndex_pct         , this->m_kolbachIndex_pct         , val                                  ); return; }
void Fermentable::setHardnessPrpGlassy_pct    (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::hardnessPrpGlassy_pct    , this->m_hardnessPrpGlassy_pct    , val                                  ); return; }
void Fermentable::setHardnessPrpHalf_pct      (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::hardnessPrpHalf_pct      , this->m_hardnessPrpHalf_pct      , val                                  ); return; }
void Fermentable::setHardnessPrpMealy_pct     (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::hardnessPrpMealy_pct     , this->m_hardnessPrpMealy_pct     , val                                  ); return; }
void Fermentable::setKernelSizePrpPlump_pct   (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::kernelSizePrpPlump_pct   , this->m_kernelSizePrpPlump_pct   , val                                  ); return; }
void Fermentable::setKernelSizePrpThin_pct    (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::kernelSizePrpThin_pct    , this->m_kernelSizePrpThin_pct    , val                                  ); return; }
void Fermentable::setFriability_pct           (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::friability_pct           , this->m_friability_pct           , val                                  ); return; }
void Fermentable::setDi_ph                    (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::di_ph                    , this->m_di_ph                    , val                                  ); return; }
void Fermentable::setViscosity_cP             (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::viscosity_cP             , this->m_viscosity_cP             , val                                  ); return; }
void Fermentable::setDmsP_ppm                 (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::dmsP_ppm                 , this->m_dmsP_ppm                     , val                                  ); return; }
void Fermentable::setFan_ppm                  (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::fan_ppm                  , this->m_fan_ppm                      , val                                  ); return; }
void Fermentable::setFermentability_pct       (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::fermentability_pct       , this->m_fermentability_pct       , val                                  ); return; }
void Fermentable::setBetaGlucan_ppm           (std::optional<double>     const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::betaGlucan_ppm           , this->m_betaGlucan_ppm               , val                                  ); return; }

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Fermentable)
