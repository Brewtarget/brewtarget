/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Fermentable.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
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
#include "model/StockPurchaseFermentable.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"
#include "utils/OptionalHelpers.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Fermentable.cpp"
#endif

QString Fermentable::localisedName() { return tr("Fermentable"); }
QString Fermentable::localisedName_alphaAmylase_dextUnits() { return tr("Alpha Amylase"               ); }
QString Fermentable::localisedName_betaGlucan_ppm        () { return tr("Beta Glucan"                 ); }
QString Fermentable::localisedName_coarseFineDiff_pct    () { return tr("Coarse Fine Diff"            ); }
QString Fermentable::localisedName_coarseGrindYield_pct  () { return tr("Coarse Grind Yield"          ); }
QString Fermentable::localisedName_color_lovibond        () { return tr("Color"                       ); } // See header file comment
QString Fermentable::localisedName_color_srm             () { return tr("Color"                       ); }
QString Fermentable::localisedName_diastaticPower_lintner() { return tr("Diastatic Power"             ); }
QString Fermentable::localisedName_di_ph                 () { return tr("DI pH"                       ); }
QString Fermentable::localisedName_dmsP_ppm              () { return tr("DMS precursors"              ); }
QString Fermentable::localisedName_fan_ppm               () { return tr("Free Amino Nitrogen (FAN)"   ); }
QString Fermentable::localisedName_fermentability_pct    () { return tr("Fermentability"              ); }
QString Fermentable::localisedName_fineGrindYield_pct    () { return tr("Yield (DBFG) %"              ); } // We'll treat this as default yield, for now at least
QString Fermentable::localisedName_friability_pct        () { return tr("Friability"                  ); }
QString Fermentable::localisedName_grainGroup            () { return tr("Grain Group"                 ); }
QString Fermentable::localisedName_hardnessPrpGlassy_pct () { return tr("Hardness Proportion Glassy"  ); }
QString Fermentable::localisedName_hardnessPrpHalf_pct   () { return tr("Hardness Proportion Half"    ); }
QString Fermentable::localisedName_hardnessPrpMealy_pct  () { return tr("Hardness Proportion Mealy"   ); }
QString Fermentable::localisedName_ibuGalPerLb           () { return tr("IBU Gal Per Lb"              ); }
QString Fermentable::localisedName_kernelSizePrpPlump_pct() { return tr("Kernel Size Proportion Plump"); }
QString Fermentable::localisedName_kernelSizePrpThin_pct () { return tr("Kernel Size Proportion Thin" ); }
QString Fermentable::localisedName_kolbachIndex_pct      () { return tr("Kolbach Index"               ); }
QString Fermentable::localisedName_maxInBatch_pct        () { return tr("Max In Batch"                ); }
QString Fermentable::localisedName_moisture_pct          () { return tr("Moisture"                    ); }
QString Fermentable::localisedName_notes                 () { return tr("Notes"                       ); }
QString Fermentable::localisedName_origin                () { return tr("Origin"                      ); }
QString Fermentable::localisedName_potentialYield_sg     () { return tr("Potential Yield"             ); }
QString Fermentable::localisedName_producer              () { return tr("Producer"                    ); }
QString Fermentable::localisedName_productId             () { return tr("Product ID"                  ); }
QString Fermentable::localisedName_protein_pct           () { return tr("Protein"                     ); }
QString Fermentable::localisedName_recommendMash         () { return tr("Recommend Mash"              ); }
QString Fermentable::localisedName_supplier              () { return tr("Supplier"                    ); }
QString Fermentable::localisedName_type                  () { return tr("Type"                        ); }
QString Fermentable::localisedName_viscosity_cP          () { return tr("Viscosity"                   ); }

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

bool Fermentable::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentable const & rhs = static_cast<Fermentable const &>(other);

   // Base class will already have ensured names are equal
   bool const outlinesAreEqual{
      // "Outline" fields: In BeerJSON, all these fields are in the FermentableBase type

      AUTO_PROPERTY_COMPARE(this, rhs, m_type                , PropertyNames::Fermentable::type                , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_origin              , PropertyNames::Fermentable::origin              , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_color_lovibond      , PropertyNames::Fermentable::color_lovibond      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_producer            , PropertyNames::Fermentable::producer            , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_productId           , PropertyNames::Fermentable::productId           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_grainGroup          , PropertyNames::Fermentable::grainGroup          , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_fineGrindYield_pct  , PropertyNames::Fermentable::fineGrindYield_pct  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_coarseGrindYield_pct, PropertyNames::Fermentable::coarseGrindYield_pct, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_coarseFineDiff_pct  , PropertyNames::Fermentable::coarseFineDiff_pct  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_potentialYield_sg   , PropertyNames::Fermentable::potentialYield_sg   , propertiesThatDiffer)
   };

   // If either object is an outline (see comment in model/OutlineableNamedEntity.h) then there is no point comparing
   // any more fields.  Note that an object will only be an outline whilst it is being read in from a BeerJSON file.
   if (this->m_outline || rhs.m_outline) {
      return outlinesAreEqual;
   }

   return (
      outlinesAreEqual &&

      // Remaining BeerJSON fields -- excluding inventories
      AUTO_PROPERTY_COMPARE(this, rhs, m_notes                 , PropertyNames::Fermentable::notes                 , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_moisture_pct          , PropertyNames::Fermentable::moisture_pct          , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_alphaAmylase_dextUnits, PropertyNames::Fermentable::alphaAmylase_dextUnits, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_diastaticPower_lintner, PropertyNames::Fermentable::diastaticPower_lintner, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_protein_pct           , PropertyNames::Fermentable::protein_pct           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_kolbachIndex_pct      , PropertyNames::Fermentable::kolbachIndex_pct      , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_maxInBatch_pct        , PropertyNames::Fermentable::maxInBatch_pct        , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_recommendMash         , PropertyNames::Fermentable::recommendMash         , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_hardnessPrpGlassy_pct , PropertyNames::Fermentable::hardnessPrpGlassy_pct , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_kernelSizePrpPlump_pct, PropertyNames::Fermentable::kernelSizePrpPlump_pct, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_hardnessPrpHalf_pct   , PropertyNames::Fermentable::hardnessPrpHalf_pct   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_hardnessPrpMealy_pct  , PropertyNames::Fermentable::hardnessPrpMealy_pct  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_kernelSizePrpThin_pct , PropertyNames::Fermentable::kernelSizePrpThin_pct , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_friability_pct        , PropertyNames::Fermentable::friability_pct        , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_di_ph                 , PropertyNames::Fermentable::di_ph                 , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_viscosity_cP          , PropertyNames::Fermentable::viscosity_cP          , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dmsP_ppm              , PropertyNames::Fermentable::dmsP_ppm              , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_fan_ppm               , PropertyNames::Fermentable::fan_ppm               , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_fermentability_pct    , PropertyNames::Fermentable::fermentability_pct    , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_betaGlucan_ppm        , PropertyNames::Fermentable::betaGlucan_ppm        , propertiesThatDiffer) &&
      // Non-BeerJSON fields
      AUTO_PROPERTY_COMPARE(this, rhs, m_supplier              , PropertyNames::Fermentable::supplier              , propertiesThatDiffer)
   );
}

ObjectStore & Fermentable::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Fermentable>::getInstance();
}

TypeLookup const Fermentable::typeLookup {
   "Fermentable",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, type                   , m_type                 , ENUM_INFO(Fermentable::type) ),
      //
      // Historically, we always stored fermentable color as degrees Lovibond, but we also incorrectly assumed this was
      // the same as SRM.  Now that we've corrected the conversion, it's simpler to carry on storing fermentable color
      // as °L, even though SRM is our canonical color unit.  Hence why we have two color properties on Fermentable.
      //
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, color_lovibond        , m_color_lovibond        , Measurement::PhysicalQuantity::Color ),
      PROPERTY_TYPE_LOOKUP_NO_MV(Fermentable, color_srm             , color_srm               , Measurement::PhysicalQuantity::Color , DisplayInfo::Precision{1}),

      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, origin                , m_origin                ,           NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, supplier              , m_supplier              ,           NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, notes                 , m_notes                 ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, coarseFineDiff_pct    , m_coarseFineDiff_pct    ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, moisture_pct          , m_moisture_pct          ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, diastaticPower_lintner, m_diastaticPower_lintner, Measurement::PhysicalQuantity::DiastaticPower),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, protein_pct           , m_protein_pct           ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, maxInBatch_pct        , m_maxInBatch_pct        ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, recommendMash         , m_recommendMash         ,           NonPhysicalQuantity::Bool          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, ibuGalPerLb           , m_ibuGalPerLb           ,           NonPhysicalQuantity::Dimensionless ), // Not really dimensionless...
      // ⮜⮜⮜ All below_ENTRYd for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, grainGroup            , m_grainGroup            , ENUM_INFO(Fermentable::grainGroup)           ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, producer              , m_producer              ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, productId             , m_productId             ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, fineGrindYield_pct    , m_fineGrindYield_pct    ,           NonPhysicalQuantity::Percentage    , DisplayInfo::Precision{1}),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, coarseGrindYield_pct  , m_coarseGrindYield_pct  ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, potentialYield_sg     , m_potentialYield_sg     , Measurement::PhysicalQuantity::Density       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, alphaAmylase_dextUnits, m_alphaAmylase_dextUnits,           NonPhysicalQuantity::Dimensionless ), // Not really dimensionless...
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, kolbachIndex_pct      , m_kolbachIndex_pct      ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, hardnessPrpGlassy_pct , m_hardnessPrpGlassy_pct ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, hardnessPrpHalf_pct   , m_hardnessPrpHalf_pct   ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, hardnessPrpMealy_pct  , m_hardnessPrpMealy_pct  ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, kernelSizePrpPlump_pct, m_kernelSizePrpPlump_pct,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, kernelSizePrpThin_pct , m_kernelSizePrpThin_pct ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, friability_pct        , m_friability_pct        ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, di_ph                 , m_di_ph                 , Measurement::PhysicalQuantity::Acidity       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, viscosity_cP          , m_viscosity_cP          , Measurement::PhysicalQuantity::Viscosity     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, dmsP_ppm              , m_dmsP_ppm              , Measurement::PhysicalQuantity::MassFractionOrConc),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, fan_ppm               , m_fan_ppm               , Measurement::PhysicalQuantity::MassFractionOrConc),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, fermentability_pct    , m_fermentability_pct    ,           NonPhysicalQuantity::Percentage    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Fermentable, betaGlucan_ppm        , m_betaGlucan_ppm        , Measurement::PhysicalQuantity::MassFractionOrConc),
   },
   // Parent classes lookup
   {&Ingredient::typeLookup,
    &IngredientBase<Fermentable>::typeLookup}
};
static_assert(std::is_base_of<Ingredient, Fermentable>::value);

Fermentable::Fermentable(QString name) :
   Ingredient                 {name},
   m_type                     {Fermentable::Type::Grain},
   m_color_lovibond           {0.0                     },
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
   // See constructor body for initialisation of m_color_lovibond
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

   // The bundle should have either color_lovibond (BeerXML, DB) or color_srm (BeerJSON) set, but not both
   if (!SET_IF_PRESENT_FROM_NPB_NO_MV(Fermentable::setColor_lovibond, namedParameterBundle, PropertyNames::Fermentable::color_lovibond) &&
       !SET_IF_PRESENT_FROM_NPB_NO_MV(Fermentable::setColor_srm     , namedParameterBundle, PropertyNames::Fermentable::color_srm     )) {
      //
      // This is probably a coding error, but we can soldier on
      //
      qWarning() << Q_FUNC_INFO << "No color set on Fermentable" << *this;
      this->m_color_lovibond = 0.0;
   }

   CONSTRUCTOR_END
   return;
}

Fermentable::Fermentable(Fermentable const & other) :
   Ingredient              {other                         },
   m_type                  {other.m_type                  },
   m_color_lovibond        {other.m_color_lovibond        },
   m_origin                {other.m_origin                },
   m_supplier              {other.m_supplier              },
   m_notes                 {other.m_notes                 },
   m_coarseFineDiff_pct    {other.m_coarseFineDiff_pct    },
   m_moisture_pct          {other.m_moisture_pct          },
   m_diastaticPower_lintner{other.m_diastaticPower_lintner},
   m_protein_pct           {other.m_protein_pct           },
   m_maxInBatch_pct        {other.m_maxInBatch_pct        },
   m_recommendMash         {other.m_recommendMash         },
   m_ibuGalPerLb           {other.m_ibuGalPerLb           },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_grainGroup            {other.m_grainGroup            },
   m_producer              {other.m_producer              },
   m_productId             {other.m_productId             },
   m_fineGrindYield_pct    {other.m_fineGrindYield_pct    },
   m_coarseGrindYield_pct  {other.m_coarseGrindYield_pct  },
   m_potentialYield_sg     {other.m_potentialYield_sg     },
   m_alphaAmylase_dextUnits{other.m_alphaAmylase_dextUnits},
   m_kolbachIndex_pct      {other.m_kolbachIndex_pct      },
   m_hardnessPrpGlassy_pct {other.m_hardnessPrpGlassy_pct },
   m_hardnessPrpHalf_pct   {other.m_hardnessPrpHalf_pct   },
   m_hardnessPrpMealy_pct  {other.m_hardnessPrpMealy_pct  },
   m_kernelSizePrpPlump_pct{other.m_kernelSizePrpPlump_pct},
   m_kernelSizePrpThin_pct {other.m_kernelSizePrpThin_pct },
   m_friability_pct        {other.m_friability_pct        },
   m_di_ph                 {other.m_di_ph                 },
   m_viscosity_cP          {other.m_viscosity_cP          },
   m_dmsP_ppm              {other.m_dmsP_ppm              },
   m_fan_ppm               {other.m_fan_ppm               },
   m_fermentability_pct    {other.m_fermentability_pct    },
   m_betaGlucan_ppm        {other.m_betaGlucan_ppm        } {

   CONSTRUCTOR_END
   return;
}

Fermentable::~Fermentable() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Fermentable::Type                      Fermentable::type                     () const { return                    this->m_type                     ; }
double                                 Fermentable::color_lovibond           () const { return                    this->m_color_lovibond           ; }
double                                 Fermentable::color_srm                () const {
   // SRM is canonical color unit
   return Measurement::Units::lovibond.toCanonical(this->m_color_lovibond).quantity;
}
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
void Fermentable::setColor_lovibond           (double                    const   val) { SET_AND_NOTIFY(PropertyNames::Fermentable::color_lovibond           , this->m_color_lovibond           , this->enforceMin      (val, "color"));                      return; }

void Fermentable::setColor_srm                (double                    const   val) {
   this->setColor_lovibond(Measurement::Units::lovibond.fromCanonical(val));
   return;
}

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

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Fermentable)

// Insert the boiler-plate stuff for inventory
INGREDIENT_BASE_COMMON_CODE(Fermentable)
