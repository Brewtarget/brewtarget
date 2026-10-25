/*======================================================================================================================
 * serialization/json/dotBeer/DotBeer.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#include "serialization/json/dotBeer/DotBeer.h"

#include <cstdlib>

// Ubuntu 22.04 ships with an old version of GCC which doesn't have support for std::format
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
   #include <format>
#endif

// We could just include <boost/json.hpp> which pulls all the Boost.JSON headers in, but that seems overkill
#include <boost/json/kind.hpp>
#include <boost/json/parse_options.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/string.hpp>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QTextStream>
#include <QTimeZone>
#include <QVersionNumber>

#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/IbuMethods.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUtils.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "serialization/json/JsonCoding.h"
#include "serialization/json/JsonMeasureableUnitsMapping.h"
#include "serialization/json/JsonNamedEntityRecord.h"
#include "serialization/json/JsonRecord.h"
#include "serialization/json/JsonRecordDefinition.h"
#include "serialization/json/JsonSchema.h"
#include "serialization/json/JsonUtils.h"
#include "utils/OStreamWriterForQFile.h"

namespace {
   // See below for more comments on this.  If and when DotBeer evolves then we will want separate constants for
   // min/max versions we can read plus whatever version we write.
   BtStringConst  const dotBeerVersionWeSupportAsString{"1.0.0"};
   QVersionNumber const dotBeerVersionWeSupport        {1, 0, 0};

   auto const outputDocumentName = "DotBeer";

   //
   // These are mappings we use in multiple places
   //
   JsonMeasureableUnitsMapping const DOT_BEER_MASS_UNIT_MAPPER {
      // MassUnitType in measurable_units.json in DotBeer schema
      {{"mg", &Measurement::Units::milligrams},
       { "g", &Measurement::Units::grams     },
       {"kg", &Measurement::Units::kilograms },
       {"lb", &Measurement::Units::pounds    },
       {"oz", &Measurement::Units::ounces    }}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_VOLUME_UNIT_MAPPER {
      {{"ml"   , &Measurement::Units::milliliters         },
       {"l"    , &Measurement::Units::liters              },
       {"tsp"  , &Measurement::Units::us_teaspoons        },
       {"tbsp" , &Measurement::Units::us_tablespoons      },
       {"floz" , &Measurement::Units::us_fluidOunces      },
       {"cup"  , &Measurement::Units::us_cups             },
       {"pt"   , &Measurement::Units::us_pints            },
       {"qt"   , &Measurement::Units::us_quarts           },
       {"gal"  , &Measurement::Units::us_gallons          },
       {"bbl"  , &Measurement::Units::us_barrels          },
       {"itsp" , &Measurement::Units::imperial_teaspoons  },
       {"itbsp", &Measurement::Units::imperial_tablespoons},
       {"ifloz", &Measurement::Units::imperial_fluidOunces},
       {"icup" , &Measurement::Units::imperial_cups       },
       {"ipt"  , &Measurement::Units::imperial_pints      },
       {"iqt"  , &Measurement::Units::imperial_quarts     },
       {"igal" , &Measurement::Units::imperial_gallons    },
       {"ibbl" , &Measurement::Units::imperial_barrels    }}
   };

   // Length is part of DotBeer, but not (yet) BeerJSON.
   JsonMeasureableUnitsMapping const DOT_BEER_LENGTH_MAPPER {
      {{"cm" , &Measurement::Units::centimeters},
       {"mm" , &Measurement::Units::millimeters},
       {"m"  , &Measurement::Units::meters     },
       {"in" , &Measurement::Units::inches     },
       {"ft" , &Measurement::Units::feet       }}
   };

   //
   // We use JsonMeasureableUnitsMapping instead of JsonSingleUnitSpecifier here because it simplifies
   // ListOfJsonMeasureableUnitsMappings below for recipe additions, stock purchases/uses and so on.
   //
   // (In other places where a count is needed, we typically just use a raw integer.)
   //
   JsonMeasureableUnitsMapping const DOT_BEER_COUNT_UNIT_MAPPER {
      //
      // Strictly, we can ignore the "unit" field of a DotBeer UnitType, but it's easier to just do the trivial mapping
      // here so that we can reuse all the same measurement handling code.
      //
      {{"count", &Measurement::Units::numberOf}}
   };

   ListOfJsonMeasureableUnitsMappings const DOT_BEER_MASS_OR_VOLUME_UNIT_MAPPER {
      {&DOT_BEER_MASS_UNIT_MAPPER, &DOT_BEER_VOLUME_UNIT_MAPPER}
   };

   ListOfJsonMeasureableUnitsMappings const DOT_BEER_MASS_VOLUME_OR_COUNT_UNIT_MAPPER {
      {&DOT_BEER_MASS_UNIT_MAPPER, &DOT_BEER_VOLUME_UNIT_MAPPER, &DOT_BEER_COUNT_UNIT_MAPPER}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_TEMPERATURE_UNIT_MAPPER {
      // TemperatureUnitType in measurable_units.json in DotBeer schema
      {{"C", &Measurement::Units::celsius   },
       {"F", &Measurement::Units::fahrenheit}}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_COLOR_UNIT_MAPPER {
      // ColorUnitType in measurable_units.json in DotBeer schema
      {{"EBC" , &Measurement::Units::ebc     },
       {"SRM" , &Measurement::Units::srm     },
       {"Lovi", &Measurement::Units::lovibond}}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_DIASTATIC_POWER_UNIT_MAPPER {
      // DiastaticPowerUnitType in measurable_units.json in DotBeer schema
      {{"Lintner", &Measurement::Units::lintner},
       {"WK",      &Measurement::Units::wk}}
   };

   // BitternessUnitType in measurable_units.json in DotBeer schema
   JsonSingleUnitSpecifier const DOT_BEER_BITTERNESS_UNIT{{"IBUs"}};

   JsonMeasureableUnitsMapping const DOT_BEER_CARBONATION_UNIT_MAPPER {
      // CarbonationUnitType in measurable_units.json in DotBeer schema
      {{"vols", &Measurement::Units::carbonationVolumes      },
       {"g/l" , &Measurement::Units::carbonationGramsPerLiter}}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER {
      // ConcentrationUnitType in measurable_units.json in DotBeer schema
      {{"ppm" , &Measurement::Units::partsPerMillionMass},
       {"ppb" , &Measurement::Units::partsPerBillionMass},
       {"mg/l", &Measurement::Units::milligramsPerLiter }}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_DENSITY_UNIT_MAPPER {
      // GravityUnitType in measurable_units.json in DotBeer schema
      // (See comments in measurement/Unit.h and measurement/PhysicalQuantity.h for why we stick with "density" in our
      // naming.)
      // Note that DensityUnitType is identically defined in measurable_units.json, but does not appear to be referenced
      // anywhere else.
      {{"sg"   , &Measurement::Units::specificGravity},
       {"plato", &Measurement::Units::plato          },
       {"brix" , &Measurement::Units::brix           }}
   };

   // PercentUnitType in measurable_units.json in DotBeer schema
   JsonSingleUnitSpecifier const DOT_BEER_PERCENT_UNIT{{"%"}};

   // AcidityUnitType in measurable_units.json in DotBeer schema
   JsonSingleUnitSpecifier const DOT_BEER_ACIDITY_UNIT{{"pH"}};

   JsonMeasureableUnitsMapping const DOT_BEER_TIME_UNIT_MAPPER {
      // TimeUnitType in measurable_units.json in DotBeer schema
      {{"sec" , &Measurement::Units::seconds},
       {"min" , &Measurement::Units::minutes},
       {"hr"  , &Measurement::Units::hours  },
       {"day" , &Measurement::Units::days   },
       {"week", &Measurement::Units::weeks  }}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_VISCOSITY_UNIT_MAPPER {
      // ViscosityUnitType in measurable_units.json in DotBeer schema
      {{"cP",    &Measurement::Units::centipoise       },
       {"mPa-s", &Measurement::Units::millipascalSecond}}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_SPECIFIC_HEAT_UNIT_MAPPER {
      // SpecificHeatCapacity in Measurement.beer.schema
      {{"c/g·C"   , &Measurement::Units::caloriesPerCelsiusPerGram},
       {"J/kg·K"  , &Measurement::Units::joulesPerKelvinPerKg     },
       {"BTU/lb·F", &Measurement::Units::btuPerFahrenheitPerPound }}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_VOLUME_PER_MASS_UNIT_MAPPER {
      // VolumePerMass in Measurement.beer.schema
      {{"l/kg"   , &Measurement::Units::litresPerKilogram     },
       {"l/g"    , &Measurement::Units::litresPerGram         },
       {"m^3/kg" , &Measurement::Units::cubicMetersPerKilogram},
       {"qt/lb"  , &Measurement::Units::us_quartsPerPound     },
       {"gal/lb" , &Measurement::Units::us_gallonsPerPound    },
       {"gal/oz" , &Measurement::Units::us_gallonsPerOunce    },
       {"floz/oz", &Measurement::Units::us_fluidOuncesPerOunce},
       {"ft^3/lb", &Measurement::Units::cubicFeetPerPound     }}
   };

   JsonMeasureableUnitsMapping const DOT_BEER_ENERGY_PER_VOLUME_UNIT_MAPPER {
      // EnergyPerVolume in Measurement.beer.schema
      {{"kcal/l"     , &Measurement::Units::kilocaloriesPerLiter     },
       {"kcal/50cl"  , &Measurement::Units::kilocaloriesPer50cl      },
       {"kcal/33cl"  , &Measurement::Units::kilocaloriesPer33cl      },
       {"kcal/25cl"  , &Measurement::Units::kilocaloriesPer25cl      },
       {"kcal/100ml" , &Measurement::Units::kilocaloriesPer100ml     },
       {"kcal/pt"    , &Measurement::Units::kilocaloriesPer16us_floz },
       {"kcal/12floz", &Measurement::Units::kilocaloriesPer12us_floz },
       {"kcal/ipt"   , &Measurement::Units::kilocaloriesPer20imp_floz},
       {"kJ/100ml"   , &Measurement::Units::kilojoulesPer100ml       }}
   };

   //
   // See comment on JsonRecordDefinition::StringOutputValidator
   //
   std::optional<QString> DOT_BEER_STYLE_LETTER_VALIDATOR(QString const & str) {
      //
      // In dotBeer, the style letter can be a "specific subcategory letter (or a one- or two-digit sub-style number)",
      // and "Letters should be upper case".  We upper-case our stored value, and write it out only if it then matches
      // the dotBeer pattern (which we expect should be most of the time!).  Otherwise, we omit the field rather than
      // guess (eg "21B" is probably a whole style code, and truncating it to "21" would give the category number, not
      // the style letter).
      //
      static QRegularExpression const styleLetterPattern{QRegularExpression::anchoredPattern("[A-Z]|[0-9]{1,2}")};
      QString const candidate = str.trimmed().toUpper();
      if (styleLetterPattern.match(candidate).hasMatch()) {
         return candidate;
      }
      if (!candidate.isEmpty()) {
         qWarning() << Q_FUNC_INFO << "Omitting style letter" << str << "as it is not valid for dotBeer";
      }
      return std::nullopt;
   }

   //
   // We use a templated variable name as small short-cut for exporting lists of top-level objects.  Eg, if we have a
   // `QList<Hop const *>` and `QList<Fermentable const *>` that we want to export, then the compiler can automatically
   // work out that the JsonRecordDefinition objects for mapping them to DotBeer are DOT_BEER_RECORD_DEFN<Hop>
   // and DOT_BEER_RECORD_DEFN<Fermentable> respectively.  This saves us having to have a look-up table in
   // DotBeer::Exporter::add().
   //
   // Note, however, that for reading things in from a JSON, things work differently (because we can't know at compile
   // time what a JSON file contains!), so the templated names don't buy us anything there.  Instead,
   // DOT_BEER_RECORD_DEFN_ROOT tells us how to read in top-level records from a DotBeer file.
   //
   // In both cases, each JsonRecordDefinition object contains links to any other JsonRecordDefinition objects needed to
   // read/write contained records (eg DOT_BEER_RECORD_DEFN<Mash> contains a link to
   // DOT_BEER_RECORD_DEFN<MashStep>).
   //
   // Note too, that although we mostly use them for consistency, not all of the JsonRecordDefinition objects _need_
   // templated names.  It's only used for top-level records (see ../schemas/DotBeer/1.0/beer.json and the parameters
   // of ImportExport::exportToFile).  So, eg, DOT_BEER_RECORD_DEFN<MashStep> could just as easily be called
   // DOT_BEER_RECORD_DEFN_MASH_STEP because it's only referred to inside the DOT_BEER_RECORD_DEFN<Mash>
   // definition.  In a small number of cases, we _need_ to use a different name.  (Eg DOT_BEER_RECORD_DEFN<Style> is
   // used for the top-level lists of Styles, but cannot be used inside DOT_BEER_RECORD_DEFN<Recipe>, where DotBeer
   // uses a cut-down style definition, so we use DOT_BEER_RECORD_DEFN_STYLE_IN_RECIPE instead.
   //
   // Also, some JsonRecordDefinition objects _cannot_ have templated DOT_BEER_RECORD_DEFN names, because they
   // would clash.  Eg, we need a slightly different Hop record mapping from DOT_BEER_RECORD_DEFN<Hop> inside
   // DOT_BEER_RECORD_DEFN<RecipeAdditionHop> (recipes/ingredients/hop_additions) than we do at
   // top level, so we need a separate DOT_BEER_RECORD_DEFN_HOP_IN_ADDITION record.
   //
   //
   // We only use specialisations of this template.  GCC doesn't mind not having a definition for the default cases (as
   // it's not used) but other compilers do.
   //
   // NOTE: If you find this default version of JsonRecordDefinition being used, it's a bug and probably means you
   //       forgot to define the appropriate specialisation below!
   //
   template<class NE> JsonRecordDefinition const DOT_BEER_RECORD_DEFN {
      "not_used", // DotBeer record name
      nullptr,
      "not_used", // namedEntityClassName
      "not_used", // localisedEntityName
      {},         // upAndDownCasters
      JsonRecordDefinition::create<JsonRecord>,
      std::initializer_list<JsonRecordDefinition::FieldDefinition>{},
      JsonRecordDefinition::RecordType::Normal
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Common field mappings for any free-standing entity (ie one that is not "owned" by another entity) that might be
   // cross-referenced by another (eg Recipe or StockPurchase).
   //
   // Eg, Boil, Hop, Fermentable, Mash, Style, Water are free-standing entities, but BoilStep, MashStep, BrewLog are
   // not.  Recipe is a free-standing entity, but it is not cross-referenced by any other entity (so it doesn't have a
   // local ID).
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_CrossReferencableBase {
      // Type                                       XPath          Q_PROPERTY                                                Value Decoder
      {JsonRecordDefinition::FieldType::String    , "name"       , PropertyNames::NamedEntity::name                        , },
      {JsonRecordDefinition::FieldType::LocalId   , "local_id"   , BtString::NULL_STR                                      , },
      {JsonRecordDefinition::FieldType::FolderPath, "folder_path", PropertyNames::FolderPropertyBase::containedInFolderPath, },
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for fermentables DotBeer records - see schemas/DotBeer/1.0/fermentable.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Fermentable_ExclBase {
      // Type                                                 XPath                           Q_PROPERTY                                        Value Decoder
      {JsonRecordDefinition::FieldType::Enum                , "fermentable_type"            , PropertyNames::Fermentable::type                , &Fermentable::typeStringMapping      },
      {JsonRecordDefinition::FieldType::String              , "origin"                      , PropertyNames::Fermentable::origin              ,                                      },
      {JsonRecordDefinition::FieldType::String              , "producer"                    , PropertyNames::Fermentable::producer            ,                                      },
      {JsonRecordDefinition::FieldType::String              , "product_id"                  , PropertyNames::Fermentable::productId           ,                                      },
      {JsonRecordDefinition::FieldType::Enum                , "grain_group"                 , PropertyNames::Fermentable::grainGroup          , &Fermentable::grainGroupStringMapping},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield_fine_grind"            , PropertyNames::Fermentable::fineGrindYield_pct  , &DOT_BEER_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield_coarse_grind"          , PropertyNames::Fermentable::coarseGrindYield_pct, &DOT_BEER_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield_fine_coarse_difference", PropertyNames::Fermentable::coarseFineDiff_pct  , &DOT_BEER_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "yield_potential"             , PropertyNames::Fermentable::potentialYield_sg   , &DOT_BEER_DENSITY_UNIT_MAPPER       },
      // Note that, when reading from dotBeer, we always convert things to canonical units for storage, hence why we use
      // Fermentable::color_srm here even though we use Fermentable::color_lovibond for BeerXML and for DB storage.
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "color"                , PropertyNames::Fermentable::color_srm           , &DOT_BEER_COLOR_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::String                    , "notes"                , PropertyNames::Fermentable::notes                 ,                                      },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "moisture"             , PropertyNames::Fermentable::moisture_pct          , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::Double                    , "alpha_amylase"        , PropertyNames::Fermentable::alphaAmylase_dextUnits,                                      },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "diastatic_power"      , PropertyNames::Fermentable::diastaticPower_lintner, &DOT_BEER_DIASTATIC_POWER_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "protein"              , PropertyNames::Fermentable::protein_pct           , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "kolbach_index"        , PropertyNames::Fermentable::kolbachIndex_pct      , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "max_in_batch"         , PropertyNames::Fermentable::maxInBatch_pct        , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::Bool                      , "recommend_mash"       , PropertyNames::Fermentable::recommendMash         ,                                      },
      // DotBeer deliberately excludes `inventory` in favour of separate lists of stock purchases
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "hardness_prp_glassy"  , PropertyNames::Fermentable::hardnessPrpGlassy_pct , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "hardness_prp_half"    , PropertyNames::Fermentable::hardnessPrpHalf_pct   , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "hardness_prp_mealy"   , PropertyNames::Fermentable::hardnessPrpMealy_pct  , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "kernel_size_prp_plump", PropertyNames::Fermentable::kernelSizePrpPlump_pct, &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "kernel_size_prp_thin" , PropertyNames::Fermentable::kernelSizePrpThin_pct , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "friability"           , PropertyNames::Fermentable::friability_pct        , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "di_pH"                , PropertyNames::Fermentable::di_ph                 , &DOT_BEER_ACIDITY_UNIT               },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "viscosity"            , PropertyNames::Fermentable::viscosity_cP          , &DOT_BEER_VISCOSITY_UNIT_MAPPER      },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "dms_p"                , PropertyNames::Fermentable::dmsP_ppm              , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "fan"                  , PropertyNames::Fermentable::fan_ppm               , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "fermentability"       , PropertyNames::Fermentable::fermentability_pct    , &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "beta_glucan"          , PropertyNames::Fermentable::betaGlucan_ppm        , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "lactic_acid_by_weight", PropertyNames::Fermentable::lacticAcidByWeight_pct, &DOT_BEER_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "fermentable_fraction" , PropertyNames::Fermentable::fermentableFraction_pct, &DOT_BEER_PERCENT_UNIT              },
   };

   // As mentioned above, it would be really nice to do this at compile time, but haven't yet found a nice way to do so
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Fermentable> {
      std::in_place_type_t<Fermentable>{},
      "fermentables", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Fermentable_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for hops DotBeer records - see schemas/DotBeer/1.0/hop.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Hop_ExclBase {
      // Type                                                       XPath                                Q_PROPERTY                                 Value Decoder
      {JsonRecordDefinition::FieldType::String                    , "producer"                         , PropertyNames::Hop::producer             },
      {JsonRecordDefinition::FieldType::String                    , "product_id"                       , PropertyNames::Hop::productId            },
      {JsonRecordDefinition::FieldType::String                    , "origin"                           , PropertyNames::Hop::origin               },
      {JsonRecordDefinition::FieldType::String                    , "year"                             , PropertyNames::Hop::year                 },
      {JsonRecordDefinition::FieldType::Enum                      , "form"                             , PropertyNames::Hop::form                 , &Hop::formStringMapping},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "alpha_acid"                       , PropertyNames::Hop::alpha_pct            , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "alpha_acid_range/minimum"         , PropertyNames::Hop::alphaMin_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "alpha_acid_range/maximum"         , PropertyNames::Hop::alphaMax_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "beta_acid"                        , PropertyNames::Hop::beta_pct             , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "beta_acid_range/minimum"          , PropertyNames::Hop::betaMin_pct          , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "beta_acid_range/maximum"          , PropertyNames::Hop::betaMax_pct          , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::Enum                      , "hop_type"                         , PropertyNames::Hop::type                 , &Hop::typeStringMapping},
      {JsonRecordDefinition::FieldType::String                    , "notes"                            , PropertyNames::Hop::notes                },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "six_month_alpha_loss"             , PropertyNames::Hop::sixMonthAlphaLoss_pct, &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::String                    , "substitutes"                      , PropertyNames::Hop::substitutes          },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "cohumulone"                       , PropertyNames::Hop::cohumulone_pct       , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::Double                    , "oil_content/total_oil_ml_per_100g", PropertyNames::Hop::totalOil_mlPer100g   },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/humulene"             , PropertyNames::Hop::humulene_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/caryophyllene"        , PropertyNames::Hop::caryophyllene_pct    , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/myrcene"              , PropertyNames::Hop::myrcene_pct          , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/farnesene"            , PropertyNames::Hop::farnesene_pct        , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/geraniol"             , PropertyNames::Hop::geraniol_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/b_pinene"             , PropertyNames::Hop::bPinene_pct          , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/a_pinene"             , PropertyNames::Hop::pinene_pct           , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/linalool"             , PropertyNames::Hop::linalool_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/limonene"             , PropertyNames::Hop::limonene_pct         , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/nerol"                , PropertyNames::Hop::nerol_pct            , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "polyphenols"                      , PropertyNames::Hop::polyphenols_pct      , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "xanthohumol"                      , PropertyNames::Hop::xanthohumol_pct      , &DOT_BEER_PERCENT_UNIT},
      // DotBeer deliberately excludes `inventory` in favour of separate lists of stock purchases
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Hop> {
      std::in_place_type_t<Hop>{},
      "hops", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Hop_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for miscs DotBeer records - see schemas/DotBeer/1.0/misc.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Misc_ExclBase {
      // Type                                            XPath                       Q_PROPERTY                                  Value Decoder
      {JsonRecordDefinition::FieldType::String         , "producer"                , PropertyNames::Misc::producer             },
      {JsonRecordDefinition::FieldType::String         , "product_id"              , PropertyNames::Misc::productId            },
      {JsonRecordDefinition::FieldType::Enum           , "misc_type"               , PropertyNames::Misc::type                 , &Misc::typeStringMapping},
      {JsonRecordDefinition::FieldType::Enum           , "water_agent_type"        , PropertyNames::Misc::waterAgentType       , &Misc::waterAgentTypeStringMapping},
      {JsonRecordDefinition::FieldType::SingleUnitValue, "water_agent_percent_acid", PropertyNames::Misc::waterAgentPercentAcid, &DOT_BEER_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::String                    , "use_for"         , PropertyNames::Misc::useFor              },
      {JsonRecordDefinition::FieldType::String                    , "notes"           , PropertyNames::Misc::notes               },
      // DotBeer deliberately excludes `inventory` in favour of separate lists of stock purchases
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Misc> {
      std::in_place_type_t<Misc>{},
      "miscs", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Misc_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for cultures DotBeer records - see schemas/DotBeer/1.0/culture.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Yeast_ExclBase {
      // Type                                                       XPath                          Q_PROPERTY                                       Value Decoder
      {JsonRecordDefinition::FieldType::Enum                      , "culture_type"               , PropertyNames::Yeast::type                     , &Yeast::typeStringMapping},
      {JsonRecordDefinition::FieldType::Enum                      , "form"                       , PropertyNames::Yeast::form                     , &Yeast::formStringMapping},
      {JsonRecordDefinition::FieldType::String                    , "producer"                   , PropertyNames::Yeast::laboratory               },
      {JsonRecordDefinition::FieldType::String                    , "product_id"                 , PropertyNames::Yeast::productId                },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "temperature_range/minimum"  , PropertyNames::Yeast::minTemperature_c         , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "temperature_range/maximum"  , PropertyNames::Yeast::maxTemperature_c         , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "alcohol_tolerance"          , PropertyNames::Yeast::alcoholTolerance_pct     , &DOT_BEER_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::Enum                      , "flocculation"               , PropertyNames::Yeast::flocculation             , &Yeast::flocculationStringMapping },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "attenuation_range/minimum"  , PropertyNames::Yeast::attenuationMin_pct       , &DOT_BEER_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "attenuation_range/maximum"  , PropertyNames::Yeast::attenuationMax_pct       , &DOT_BEER_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::String                    , "notes"                      , PropertyNames::Yeast::notes                    },
      {JsonRecordDefinition::FieldType::String                    , "best_for"                   , PropertyNames::Yeast::bestFor                  },
      {JsonRecordDefinition::FieldType::Int                       , "max_reuse"                  , PropertyNames::Yeast::maxReuse                 },
      {JsonRecordDefinition::FieldType::Bool                      , "pof"                        , PropertyNames::Yeast::phenolicOffFlavorPositive},
      {JsonRecordDefinition::FieldType::Bool                      , "glucoamylase"               , PropertyNames::Yeast::glucoamylasePositive     },
      // DotBeer deliberately excludes `inventory` in favour of separate lists of stock purchases
      {JsonRecordDefinition::FieldType::Bool                      , "killer/producing_K1_toxin"  , PropertyNames::Yeast::killerProducingK1Toxin   },
      {JsonRecordDefinition::FieldType::Bool                      , "killer/producing_K2_toxin"  , PropertyNames::Yeast::killerProducingK2Toxin   },
      {JsonRecordDefinition::FieldType::Bool                      , "killer/producing_K28_toxin" , PropertyNames::Yeast::killerProducingK28Toxin  },
      {JsonRecordDefinition::FieldType::Bool                      , "killer/producing_Klus_toxin", PropertyNames::Yeast::killerProducingKlusToxin },
      {JsonRecordDefinition::FieldType::Bool                      , "killer/neutral"             , PropertyNames::Yeast::killerNeutral            },
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Yeast> {
      std::in_place_type_t<Yeast>{},
      "cultures", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Yeast_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for water DotBeer records - see schemas/DotBeer/1.0/water.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Water_ExclBase {
      // Type                                                 XPath          Q_PROPERTY                             Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "calcium"    , PropertyNames::Water::calcium_ppm    , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "bicarbonate", PropertyNames::Water::bicarbonate_ppm, &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonate"  , PropertyNames::Water::carbonate_ppm  , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "potassium"  , PropertyNames::Water::potassium_ppm  , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "iron"       , PropertyNames::Water::iron_ppm       , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "nitrate"    , PropertyNames::Water::nitrate_ppm    , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "nitrite"    , PropertyNames::Water::nitrite_ppm    , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "fluoride"   , PropertyNames::Water::fluoride_ppm   , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "sulfate"    , PropertyNames::Water::sulfate_ppm    , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "chloride"   , PropertyNames::Water::chloride_ppm   , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "sodium"     , PropertyNames::Water::sodium_ppm     , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "magnesium"  , PropertyNames::Water::magnesium_ppm  , &DOT_BEER_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "pH"         , PropertyNames::Water::ph             , &DOT_BEER_ACIDITY_UNIT                  },
      {JsonRecordDefinition::FieldType::String              , "notes"      , PropertyNames::Water::notes          ,                                         },
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Water> {
      std::in_place_type_t<Water>{},
      "waters", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Water_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for styles DotBeer records - see schemas/DotBeer/1.0/style.json
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Style_ExclBase {
      // Type                                                 XPath                                     Q_PROPERTY                              Value Decoder
      {JsonRecordDefinition::FieldType::String              , "category"                              , PropertyNames::Style::category         },
      // See comment in model/Style.h for why "category number" is actually a string
      {JsonRecordDefinition::FieldType::String              , "category_number"                       , PropertyNames::Style::categoryNumber   },
      {JsonRecordDefinition::FieldType::String              , "style_letter"                          , PropertyNames::Style::styleLetter      , &DOT_BEER_STYLE_LETTER_VALIDATOR},
      {JsonRecordDefinition::FieldType::String              , "style_guide"                           , PropertyNames::Style::styleGuide       },
      {JsonRecordDefinition::FieldType::Enum                , "style_type"                            , PropertyNames::Style::type             , &Style::typeStringMapping},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity/minimum"              , PropertyNames::Style::ogMin            , &DOT_BEER_DENSITY_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity/maximum"              , PropertyNames::Style::ogMax            , &DOT_BEER_DENSITY_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity/minimum"                 , PropertyNames::Style::fgMin            , &DOT_BEER_DENSITY_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity/maximum"                 , PropertyNames::Style::fgMax            , &DOT_BEER_DENSITY_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "ibu/minimum"                           , PropertyNames::Style::ibuMin           , &DOT_BEER_BITTERNESS_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "ibu/maximum"                           , PropertyNames::Style::ibuMax           , &DOT_BEER_BITTERNESS_UNIT},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color/minimum"                         , PropertyNames::Style::colorMin_srm     , &DOT_BEER_COLOR_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color/maximum"                         , PropertyNames::Style::colorMax_srm     , &DOT_BEER_COLOR_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonation/minimum"                   , PropertyNames::Style::carbMin_vol      , &DOT_BEER_CARBONATION_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonation/maximum"                   , PropertyNames::Style::carbMax_vol      , &DOT_BEER_CARBONATION_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume/minimum"             , PropertyNames::Style::abvMin_pct       , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume/maximum"             , PropertyNames::Style::abvMax_pct       , &DOT_BEER_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::String              , "notes"                                 , PropertyNames::Style::notes            },
      {JsonRecordDefinition::FieldType::String              , "aroma"                                 , PropertyNames::Style::aroma            },
      {JsonRecordDefinition::FieldType::String              , "appearance"                            , PropertyNames::Style::appearance       },
      {JsonRecordDefinition::FieldType::String              , "flavor"                                , PropertyNames::Style::flavor           },
      {JsonRecordDefinition::FieldType::String              , "mouthfeel"                             , PropertyNames::Style::mouthfeel        },
      {JsonRecordDefinition::FieldType::String              , "overall_impression"                    , PropertyNames::Style::overallImpression},
      {JsonRecordDefinition::FieldType::String              , "ingredients"                           , PropertyNames::Style::ingredients      },
      {JsonRecordDefinition::FieldType::String              , "examples"                              , PropertyNames::Style::examples         },
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Style> {
      std::in_place_type_t<Style>{},
      "styles", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Style_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for mash steps DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<MashStep> {
      std::in_place_type_t<MashStep>{},
      "mash_steps", // DotBeer record name (not actually used as MashStep always part of a Mash
      {
         // Type                                                 XPath                 Q_PROPERTY                                       Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"              , PropertyNames::NamedEntity::name               },
         {JsonRecordDefinition::FieldType::Enum                , "step_type"         , PropertyNames::MashStep::type                  , &MashStep::typeStringMapping          },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "amount"            , PropertyNames::MashStep::amount_l              , &DOT_BEER_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_temperature" , PropertyNames::StepBase::startTemp_c           , &DOT_BEER_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"         , PropertyNames::StepBase::stepTime_mins         , &DOT_BEER_TIME_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "ramp_time"         , PropertyNames::StepBase::rampTime_mins         , &DOT_BEER_TIME_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_temperature"   , PropertyNames::    Step::endTemp_c             , &DOT_BEER_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::String              , "step_description"  , PropertyNames::    Step::description           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "water_grain_ratio" , PropertyNames::MashStep::liquorToGristRatio_lKg, &DOT_BEER_VOLUME_PER_MASS_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "infuse_temperature", PropertyNames::MashStep::infuseTemp_c          , &DOT_BEER_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_pH"          , PropertyNames::    Step::startAcidity_pH       , &DOT_BEER_ACIDITY_UNIT               },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "end_pH"            , PropertyNames::    Step::endAcidity_pH         , &DOT_BEER_ACIDITY_UNIT               },
      }
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for mashes DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Mash_ExclBase {
      // Type                                                 XPath                Q_PROPERTY                        Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "grain_temperature", PropertyNames::Mash::grainTemp_c, &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "notes"            , PropertyNames::Mash::notes      },
      {JsonRecordDefinition::FieldType::ListOfRecords       , "mash_steps"       , PropertyNames::StepOwnerBase::steps, &DOT_BEER_RECORD_DEFN<MashStep>  },
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Mash> {
      std::in_place_type_t<Mash>{},
      "mashes", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Mash_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for fermentation_steps DotBeer records
   //
   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<FermentationStep> {
      std::in_place_type_t<FermentationStep>{},
      "fermentation_steps", // DotBeer record name
      {
         // Type                                                 XPath                Q_PROPERTY                                    Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"             , PropertyNames::NamedEntity::name            },
         {JsonRecordDefinition::FieldType::String              , "step_description" , PropertyNames::Step::description            },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_temperature", PropertyNames::    StepBase::startTemp_c    , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits,   "end_temperature", PropertyNames::        Step::  endTemp_c    , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"        , PropertyNames::StepBase::stepTime_mins      , &DOT_BEER_TIME_UNIT_MAPPER       },
         {JsonRecordDefinition::FieldType::Bool                , "free_rise"        , PropertyNames::FermentationStep::freeRise   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_gravity"    , PropertyNames::StepExtended::startGravity_sg,  &DOT_BEER_DENSITY_UNIT_MAPPER   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits,   "end_gravity"    , PropertyNames::StepExtended::  endGravity_sg,  &DOT_BEER_DENSITY_UNIT_MAPPER   },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_pH"         , PropertyNames::Step::startAcidity_pH        ,  &DOT_BEER_ACIDITY_UNIT          },
         {JsonRecordDefinition::FieldType::SingleUnitValue     ,   "end_pH"         , PropertyNames::Step::  endAcidity_pH        ,  &DOT_BEER_ACIDITY_UNIT          },
         {JsonRecordDefinition::FieldType::String              , "vessel"           , PropertyNames::FermentationStep::vessel     },
      }
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for fermentations DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Fermentation_ExclBase {
      // Type                                          XPath                       Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::String       , "fermentation_description", PropertyNames::Fermentation::description},
      {JsonRecordDefinition::FieldType::String       , "notes"                   , PropertyNames::Fermentation::notes      },
      {JsonRecordDefinition::FieldType::ListOfRecords, "fermentation_steps"      , PropertyNames::StepOwnerBase::steps     , &DOT_BEER_RECORD_DEFN<FermentationStep>},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Fermentation> {
      std::in_place_type_t<Fermentation>{},
      "fermentations", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Fermentation_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for equipments DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Equipment_ExclBase {
      //
      // This is cleaned up and simplified in comparison with how BeerJSON does things, but still not perfect.  In
      // particular, we write out all possible vessels even when some of them do not exist.  It would be nice to add
      // more logic to our export to avoid this.
      //   - In DotBeer, each individual vessel (EquipmentItemType) must have a name, as well the group of vessels
      //     (EquipmentType) needing to have one.  Internally, we only have a name for the group (Equipment).
      //     For the moment, we get round this by writing a fixed name for each vessel and ignoring vessel names when
      //     we read a DotBeer file.
      //

      // Type                                                 XPath                                Q_PROPERTY                                             Value Decoder
      {JsonRecordDefinition::FieldType::String              , "hlt/vessel_type"                  , PropertyNames::Equipment::hltType                    },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "hlt/max_volume"                   , PropertyNames::Equipment::hltVolume_l                , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "hlt/loss"                         , PropertyNames::Equipment::hltLoss_l                  , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "hlt/weight"                       , PropertyNames::Equipment::hltWeight_kg               , &DOT_BEER_MASS_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "hlt/specific_heat_capacity"       , PropertyNames::Equipment::hltSpecificHeat_calGC      , &DOT_BEER_SPECIFIC_HEAT_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "hlt/notes"                        , PropertyNames::Equipment::hltNotes                   },
      {JsonRecordDefinition::FieldType::String              , "mash_tun/vessel_type"             , PropertyNames::Equipment::mashTunType                },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "mash_tun/max_volume"              , PropertyNames::Equipment::mashTunVolume_l            , &DOT_BEER_VOLUME_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "mash_tun/loss"                    , PropertyNames::Equipment::mashTunLoss_l              , &DOT_BEER_VOLUME_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "mash_tun/grain_absorption_rate"   , PropertyNames::Equipment::mashTunGrainAbsorption_LKg , &DOT_BEER_VOLUME_PER_MASS_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "mash_tun/weight"                  , PropertyNames::Equipment::mashTunWeight_kg           , &DOT_BEER_MASS_UNIT_MAPPER           },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "mash_tun/specific_heat_capacity"  , PropertyNames::Equipment::mashTunSpecificHeat_calGC  , &DOT_BEER_SPECIFIC_HEAT_UNIT_MAPPER  },
      {JsonRecordDefinition::FieldType::String              , "mash_tun/notes"                   , PropertyNames::Equipment::mashTunNotes               },
      {JsonRecordDefinition::FieldType::String              , "lauter_tun/vessel_type"           , PropertyNames::Equipment::lauterTunType              },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "lauter_tun/max_volume"            , PropertyNames::Equipment::lauterTunVolume_l          , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "lauter_tun/loss"                  , PropertyNames::Equipment::lauterTunDeadspaceLoss_l   , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "lauter_tun/weight"                , PropertyNames::Equipment::lauterTunWeight_kg         , &DOT_BEER_MASS_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "lauter_tun/specific_heat_capacity", PropertyNames::Equipment::lauterTunSpecificHeat_calGC, &DOT_BEER_SPECIFIC_HEAT_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "lauter_tun/notes"                 , PropertyNames::Equipment::lauterTunNotes             },
      {JsonRecordDefinition::FieldType::String              , "kettle/vessel_type"               , PropertyNames::Equipment::kettleType                 },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/max_volume"                , PropertyNames::Equipment::kettleBoilSize_l           , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/loss"                      , PropertyNames::Equipment::kettleTrubChillerLoss_l    , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/boil_rate_per_hour"        , PropertyNames::Equipment::kettleEvaporationPerHour_l , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/drain_rate_per_minute"     , PropertyNames::Equipment::kettleOutflowPerMinute_l   , &DOT_BEER_VOLUME_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/weight"                    , PropertyNames::Equipment::kettleWeight_kg            , &DOT_BEER_MASS_UNIT_MAPPER         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/specific_heat_capacity"    , PropertyNames::Equipment::kettleSpecificHeat_calGC   , &DOT_BEER_SPECIFIC_HEAT_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/internal_diameter"         , PropertyNames::Equipment::kettleInternalDiameter_cm  , &DOT_BEER_LENGTH_MAPPER            },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "kettle/opening_diameter"          , PropertyNames::Equipment::kettleOpeningDiameter_cm   , &DOT_BEER_LENGTH_MAPPER            },
      {JsonRecordDefinition::FieldType::String              , "kettle/notes"                     , PropertyNames::Equipment::kettleNotes                },
      {JsonRecordDefinition::FieldType::String              , "fermentation_vessel/vessel_type"  , PropertyNames::Equipment::fvType                     },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "fermentation_vessel/max_volume"   , PropertyNames::Equipment::fvBatchSize_l              , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "fermentation_vessel/loss"         , PropertyNames::Equipment::fvLoss_l                   , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "fermentation_vessel/notes"        , PropertyNames::Equipment::fvNotes                    },
      {JsonRecordDefinition::FieldType::String              , "aging_vessel/vessel_type"         , PropertyNames::Equipment::agingVesselType            },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "aging_vessel/max_volume"          , PropertyNames::Equipment::agingVesselVolume_l        , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "aging_vessel/loss"                , PropertyNames::Equipment::agingVesselLoss_l          , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "aging_vessel/notes"               , PropertyNames::Equipment::agingVesselNotes           },
      {JsonRecordDefinition::FieldType::String              , "packaging_vessel/vessel_type"     , PropertyNames::Equipment::packagingVesselType        },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "packaging_vessel/max_volume"      , PropertyNames::Equipment::packagingVesselVolume_l    , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "packaging_vessel/loss"            , PropertyNames::Equipment::packagingVesselLoss_l      , &DOT_BEER_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::String              , "packaging_vessel/notes"           , PropertyNames::Equipment::packagingVesselNotes       },
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Equipment> {
      std::in_place_type_t<Equipment>{},
      "equipments", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Equipment_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for fermentable_additions, hop_additions, misc_additions, yeast_additions DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // The schedule bit is the same across Fermentable, Hop, Misc and Yeast
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_IngredientAdditionType_Schedule {
      // Type                                                       XPath                      Q_PROPERTY                                       Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "schedule/time"            , PropertyNames::RecipeAddition::addAtTime_mins ,  &DOT_BEER_TIME_UNIT_MAPPER          },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "schedule/duration"        , PropertyNames::RecipeAddition::duration_mins  ,  &DOT_BEER_TIME_UNIT_MAPPER          },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "schedule/specific_gravity", PropertyNames::RecipeAddition::addAtGravity_sg,  &DOT_BEER_DENSITY_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "schedule/pH"              , PropertyNames::RecipeAddition::addAtAcidity_pH,  &DOT_BEER_ACIDITY_UNIT              },
      {JsonRecordDefinition::FieldType::Int                       , "schedule/step"            , PropertyNames::RecipeAddition::step           },
      {JsonRecordDefinition::FieldType::Enum                      , "schedule/stage"           , PropertyNames::RecipeAddition::stage          ,  &RecipeAddition::stageStringMapping  },
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_IngredientAdditionType_Volume {
      // Type                                                 XPath     Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "amount", PropertyNames::IngredientAmount::amount,  &DOT_BEER_VOLUME_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_IngredientAdditionType_MassOrVolume {
      // Type                                                       XPath     Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "amount", PropertyNames::IngredientAmount::amount,  &DOT_BEER_MASS_OR_VOLUME_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_IngredientAdditionType_MassVolumeOrCount {
      // Type                                                       XPath     Q_PROPERTY                               Value Decoder
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "amount", PropertyNames::IngredientAmount::amount, &DOT_BEER_MASS_VOLUME_OR_COUNT_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_FermentableAddition_ExclBase {
      // Type                                    XPath             Q_PROPERTY                                      Value Decoder
      {JsonRecordDefinition::FieldType::LocalId, "fermentable_id", PropertyNames::IngredientAmount::ingredientId,  &Fermentable::staticMetaObject},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<RecipeAdditionFermentable> {
      std::in_place_type_t<RecipeAdditionFermentable>{},
      "fermentable_additions", // DotBeer record name
      {DotBeer_IngredientAdditionType_Schedule, DotBeer_IngredientAdditionType_MassOrVolume, DotBeer_FermentableAddition_ExclBase}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_HopAddition_ExclBase {
      // Type                                    XPath     Q_PROPERTY                                     Value Decoder
      {JsonRecordDefinition::FieldType::LocalId, "hop_id", PropertyNames::IngredientAmount::ingredientId, &Hop::staticMetaObject},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<RecipeAdditionHop> {
      std::in_place_type_t<RecipeAdditionHop>{},
      "hop_additions", // DotBeer record name
      {DotBeer_IngredientAdditionType_Schedule, DotBeer_IngredientAdditionType_MassOrVolume, DotBeer_HopAddition_ExclBase}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_MiscAddition_ExclBase {
      // Type                                    XPath      Q_PROPERTY                                     Value Decoder
      {JsonRecordDefinition::FieldType::LocalId, "misc_id", PropertyNames::IngredientAmount::ingredientId, &Misc::staticMetaObject},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<RecipeAdditionMisc> {
      std::in_place_type_t<RecipeAdditionMisc>{},
      "misc_additions", // DotBeer record name
      {DotBeer_IngredientAdditionType_Schedule, DotBeer_IngredientAdditionType_MassVolumeOrCount, DotBeer_MiscAddition_ExclBase}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_YeastAddition_ExclBase {
      // Type                                    XPath         Q_PROPERTY                                     Value Decoder
      {JsonRecordDefinition::FieldType::LocalId        , "culture_id"         , PropertyNames::IngredientAmount::ingredientId        , &Yeast::staticMetaObject},
      {JsonRecordDefinition::FieldType::SingleUnitValue, "attenuation"        , PropertyNames::RecipeAdditionYeast::attenuation_pct  , &DOT_BEER_PERCENT_UNIT  },
      {JsonRecordDefinition::FieldType::Int            , "times_cultured"     , PropertyNames::RecipeAdditionYeast::timesCultured    },
      {JsonRecordDefinition::FieldType::Int            , "cell_count_billions", PropertyNames::RecipeAdditionYeast::cellCountBillions},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<RecipeAdditionYeast> {
      std::in_place_type_t<RecipeAdditionYeast>{},
      "culture_additions", // DotBeer record name
      {DotBeer_IngredientAdditionType_Schedule, DotBeer_IngredientAdditionType_MassVolumeOrCount, DotBeer_YeastAddition_ExclBase}
   };

   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<BoilStep> {
      std::in_place_type_t<BoilStep>{},
      "boil_steps", // DotBeer record name (not actually used as BoilStep always part of a Boil)
      {
         // Type                                                 XPath                Q_PROPERTY                                    Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"             , PropertyNames:: NamedEntity::name           },
         {JsonRecordDefinition::FieldType::String              , "step_description" , PropertyNames::        Step::description    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_temperature", PropertyNames::    StepBase::startTemp_c    , &DOT_BEER_TEMPERATURE_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_temperature"  , PropertyNames::        Step::endTemp_c      , &DOT_BEER_TEMPERATURE_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "ramp_time"        , PropertyNames::    StepBase::rampTime_mins  , &DOT_BEER_TIME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"        , PropertyNames::    StepBase::stepTime_mins  , &DOT_BEER_TIME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_gravity"    , PropertyNames::StepExtended::startGravity_sg, &DOT_BEER_DENSITY_UNIT_MAPPER      },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_gravity"      , PropertyNames::StepExtended::  endGravity_sg, &DOT_BEER_DENSITY_UNIT_MAPPER      },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_pH"         , PropertyNames::        Step::startAcidity_pH, &DOT_BEER_ACIDITY_UNIT             },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "end_pH"           , PropertyNames::        Step::endAcidity_pH  , &DOT_BEER_ACIDITY_UNIT             },
         {JsonRecordDefinition::FieldType::Enum                , "chilling_type"    , PropertyNames::    BoilStep::chillingType   , &BoilStep::chillingTypeStringMapping},
      }
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for boil DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const DotBeer_Boil_ExclBase {
      // Type                                                 XPath               Q_PROPERTY                           Value Decoder
      {JsonRecordDefinition::FieldType::String              , "boil_description", PropertyNames::Boil::description   },
      {JsonRecordDefinition::FieldType::String              , "notes"           , PropertyNames::Boil::notes         },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "pre_boil_size"   , PropertyNames::Boil::preBoilSize_l , &DOT_BEER_VOLUME_UNIT_MAPPER   },
      // Per comments in Boil.beer.schema, we don't store boil_time in dotBeer as it's calculated from the boil_steps
      // (by logic in Boil::boilTime_mins()).
      {JsonRecordDefinition::FieldType::ListOfRecords       , "boil_steps"      , PropertyNames::StepOwnerBase::steps, &DOT_BEER_RECORD_DEFN<BoilStep>},
   };
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Boil> {
      std::in_place_type_t<Boil>{},
      "boils", // DotBeer record name
      {DotBeer_CrossReferencableBase, DotBeer_Boil_ExclBase}
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for instructions DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Instruction> {
      std::in_place_type_t<Instruction>{},
      "instructions", // DotBeer record name (not actually used as Instruction always part of a Recipe
      {
         // Type                                   XPath         Q_PROPERTY                                 Value Decoder
         {JsonRecordDefinition::FieldType::String, "name"      , PropertyNames::NamedEntity::name           },
         {JsonRecordDefinition::FieldType::String, "directions", PropertyNames::Instruction::directions     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "duration", PropertyNames::Instruction::interval_mins, &DOT_BEER_TIME_UNIT_MAPPER},
      }
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for brew_logs DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<BrewLog> {
      std::in_place_type_t<BrewLog>{},
      "brew_logs", // DotBeer record name (not actually used as BrewLog always part of a Recipe
      {
         // Type                                                 XPath                              Q_PROPERTY                                             Value Decoder
         {JsonRecordDefinition::FieldType::String              , "batch_number"                   , PropertyNames::NamedEntity::name                     },
         {JsonRecordDefinition::FieldType::Date                , "brew_date"                      , PropertyNames::BrewLog::brewDate                     },
         {JsonRecordDefinition::FieldType::Date                , "ferment_date"                   , PropertyNames::BrewLog::fermentDate                  },
         {JsonRecordDefinition::FieldType::String              , "notes"                          , PropertyNames::BrewLog::notes                        },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_pre_boil_gravity"      , PropertyNames::BrewLog::expectedPreBoilGravity_sg    , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_pre_boil_gravity"      , PropertyNames::BrewLog::measuredPreBoilGravity_sg    , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_pre_boil_volume"       , PropertyNames::BrewLog::expectedPreBoilVolume_l      , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_pre_boil_volume"       , PropertyNames::BrewLog::measuredPreBoilVolume_l      , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_strike_temperature"    , PropertyNames::BrewLog::expectedStrikeTemp_c         , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_strike_temperature"    , PropertyNames::BrewLog::measuredStrikeTemp_c         , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_mash_final_temperature", PropertyNames::BrewLog::expectedMashFinalTemp_c      , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_mash_final_temperature", PropertyNames::BrewLog::measuredMashFinalTemp_c      , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_original_gravity"      , PropertyNames::BrewLog::expectedOriginalGravity_sg   , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_original_gravity"      , PropertyNames::BrewLog::measuredOriginalGravity_sg   , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_post_boil_volume"      , PropertyNames::BrewLog::measuredPostBoilVolume_l     , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_volume_into_fv"        , PropertyNames::BrewLog::expectedVolumeIntoFv_l       , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_volume_into_fv"        , PropertyNames::BrewLog::measuredVolumeIntoFv_l       , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_pitch_temperature"     , PropertyNames::BrewLog::measuredPitchTemp_c          , &DOT_BEER_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_final_gravity"         , PropertyNames::BrewLog::expectedFinalGravity_sg      , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_final_gravity"         , PropertyNames::BrewLog::measuredFinalGravity_sg      , &DOT_BEER_DENSITY_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "measured_final_volume"          , PropertyNames::BrewLog::measuredFinalVolume_l        , &DOT_BEER_VOLUME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "expected_alcohol_by_volume"     , PropertyNames::BrewLog::expectedAlcoholByVolume_pct  , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "computed_alcohol_by_volume"     , PropertyNames::BrewLog::computedAlcoholByVolume_pct  , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "expected_attenuation"           , PropertyNames::BrewLog::expectedAttenuation_pct      , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "computed_attenuation"           , PropertyNames::BrewLog::computedAttenuation_pct      , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "expected_efficiency"            , PropertyNames::BrewLog::expectedEfficiency_pct       , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "computed_efficiency"            , PropertyNames::BrewLog::computedEfficiency_pct       , &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "computed_pre_boil_efficiency"   , PropertyNames::BrewLog::computedPreBoilEfficiency_pct, &DOT_BEER_PERCENT_UNIT           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "expected_boil_off"              , PropertyNames::BrewLog::expectedBoilOff_l            , &DOT_BEER_VOLUME_UNIT_MAPPER     },
      }
   };

   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for recipes DotBeer records
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   template<> JsonRecordDefinition const DOT_BEER_RECORD_DEFN<Recipe> {
      std::in_place_type_t<Recipe>{},
      "recipes", // DotBeer record name
      {
         // Type                                                 XPath                      Q_PROPERTY                                Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"                   , PropertyNames::NamedEntity::name        },
         {JsonRecordDefinition::FieldType::FolderPath          , "folder_path"            , PropertyNames::FolderPropertyBase::containedInFolderPath},
         {JsonRecordDefinition::FieldType::Enum                , "recipe_type"            , PropertyNames::Recipe::type             , &Recipe::typeStringMapping},
         {JsonRecordDefinition::FieldType::String              , "author"                 , PropertyNames::Recipe::brewer           },
         {JsonRecordDefinition::FieldType::String              , "coauthor"               , PropertyNames::Recipe::assistantBrewer  },
         {JsonRecordDefinition::FieldType::DateOrDateTimeLocal , "created"                , PropertyNames::Recipe::date             },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "batch_size"             , PropertyNames::Recipe::batchSize_l      , &DOT_BEER_VOLUME_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "efficiency/brewhouse"   , PropertyNames::Recipe::efficiency_pct   , &DOT_BEER_PERCENT_UNIT      },
         {JsonRecordDefinition::FieldType::LocalId             , "style_id"               , PropertyNames::Recipe::styleId          , &Style::staticMetaObject    },
         {JsonRecordDefinition::FieldType::LocalId             , "equipment_id"           , PropertyNames::Recipe::equipmentId      , &Equipment::staticMetaObject},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "fermentable_additions"  , PropertyNames::Recipe::fermentableAdditions,
                                                                                            &DOT_BEER_RECORD_DEFN<RecipeAdditionFermentable>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "hop_additions"          , PropertyNames::Recipe::hopAdditions,
                                                                                            &DOT_BEER_RECORD_DEFN<RecipeAdditionHop>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "misc_additions"         , PropertyNames::Recipe::miscAdditions,
                                                                                            &DOT_BEER_RECORD_DEFN<RecipeAdditionMisc>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "culture_additions"      , PropertyNames::Recipe::yeastAdditions,
                                                                                            &DOT_BEER_RECORD_DEFN<RecipeAdditionYeast>},
         {JsonRecordDefinition::FieldType::LocalId             , "water_base_id"          , PropertyNames::Recipe::waterBaseId      , &Water::staticMetaObject},
         {JsonRecordDefinition::FieldType::LocalId             , "water_target_id"        , PropertyNames::Recipe::waterTargetId    , &Water::staticMetaObject},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "ro_water_mash"          , PropertyNames::Recipe::roWaterMash_pct  , &DOT_BEER_PERCENT_UNIT      },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "ro_water_sparge"        , PropertyNames::Recipe::roWaterSparge_pct, &DOT_BEER_PERCENT_UNIT      },
         {JsonRecordDefinition::FieldType::LocalId             , "mash_id"                , PropertyNames::Recipe::mashId        , &Mash::staticMetaObject},
         {JsonRecordDefinition::FieldType::String              , "notes"                  , PropertyNames::Recipe::notes         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity"       , PropertyNames::Recipe::og            , &DOT_BEER_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity"          , PropertyNames::Recipe::fg            , &DOT_BEER_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume"      , PropertyNames::Recipe::ABV_pct       , &DOT_BEER_PERCENT_UNIT       },
         // Note that we output 'ibu_estimate' and 'ibu_estimate_formula', but ignore them on reading in (by dint of the
         // Recipe constructor not looking for it in the NamedParameterBundle -- per comments in JsonRecord::load and XmlRecord::load).
         //
         // Note too, that we use the more human-friendly string mapping (IbuMethods::formulaStringMappingUc) than the
         // one we use for storing preferences (IbuMethods::formulaStringMapping).
         {JsonRecordDefinition::FieldType::Enum                , "ibu_estimate_formula"   , PropertyNames::Recipe::ibuFormula    , &IbuMethods::formulaStringMappingUc},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "ibu_estimate"           , PropertyNames::Recipe::IBU           , &DOT_BEER_BITTERNESS_UNIT    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color_estimate"         , PropertyNames::Recipe::color_srm     , &DOT_BEER_COLOR_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "beer_pH"                , PropertyNames::Recipe::beerAcidity_pH, &DOT_BEER_ACIDITY_UNIT       },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonation"            , PropertyNames::Recipe::carbonation_vols, &DOT_BEER_CARBONATION_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "apparent_attenuation"   , PropertyNames::Recipe::apparentAttenuation_pct,
                                                                                            &DOT_BEER_PERCENT_UNIT                       },
         {JsonRecordDefinition::FieldType::LocalId             , "fermentation_id"        , PropertyNames::Recipe::fermentationId, &Fermentation::staticMetaObject},
//         {JsonRecordDefinition::FieldType::Record              , "packaging"              , BtString::NULL_STR                }, // .:TODO:. We should add support for this
         {JsonRecordDefinition::FieldType::LocalId             , "boil_id"                , PropertyNames::Recipe::boilId       , &Boil::staticMetaObject},
         {JsonRecordDefinition::FieldType::String              , "taste/notes"            , PropertyNames::Recipe::tasteNotes},
         {JsonRecordDefinition::FieldType::Double              , "taste/rating"           , PropertyNames::Recipe::tasteRating},
         // Note that we write this out but ignore it on reading in (by dint of the Recipe constructor not looking for
         // it in the NamedParameterBundle -- per comments in JsonRecord::load and XmlRecord::load).
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "energy_content"         , PropertyNames::Recipe::caloriesPerUsPint, &DOT_BEER_ENERGY_PER_VOLUME_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "instructions"           , PropertyNames::Recipe::instructions, &DOT_BEER_RECORD_DEFN<Instruction>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "brew_logs"              , PropertyNames::Recipe::brewLogs    , &DOT_BEER_RECORD_DEFN<BrewLog>    },
      }
   };


   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   // Field mappings for root of DotBeer document
   //
   // Note that the order is important here: the "recipes" entry needs to be after all the things that a recipe could
   // reference.  (We don't care about the order in which records & fields are written in the file; it's just that, when
   // we read a file in, we need to read the recipes records last.)
   //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»
   JsonRecordDefinition const DOT_BEER_RECORD_DEFN_ROOT {
      "DotBeer", // DotBeer record name
      nullptr,
      "",         // NamedEntity class name
      "",         // Localised name
      {},         // upAndDownCasters
      JsonRecordDefinition::create<JsonRecord>,
      {
         // Type                                             Name             Q_PROPERTY            Value Decoder
         {JsonRecordDefinition::FieldType::RequiredConstant, "version"      , dotBeerVersionWeSupportAsString},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "fermentables" , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Fermentable >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "miscs"        , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Misc        >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "hops"         , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Hop         >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "cultures"     , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Yeast       >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "waters"       , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Water       >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "styles"       , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Style       >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "mashes"       , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Mash        >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "boils"        , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Boil        >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "fermentations", BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Fermentation>},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "equipments"   , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Equipment   >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "recipes"      , BtString::NULL_STR  , &DOT_BEER_RECORD_DEFN<Recipe      >},
      },
      JsonRecordDefinition::RecordType::Normal
   };

   //
   // The mapping we use between DotBeer structure and our own object structure
   //
   JsonCoding const DOT_BEER_1_CODING{
      "DotBeer 1.0",
      *dotBeerVersionWeSupportAsString,
      JsonSchema::Id::DotBeer_1_0,
      DOT_BEER_RECORD_DEFN_ROOT
   };

   //=-=-=-=-=-=-=-=-

   /**
    * \brief This function first validates the input file against a JSON schema (https://json-schema.org/)
    */
   bool validateAndLoad(QString const & targetFolderPath,
                        QString const & fileName,
                        QTextStream & userMessage) {
      boost::json::value inputDocument;
      try {
         inputDocument = JsonUtils::loadJsonDocument(fileName);
      } catch (std::exception const & exception) {
         qWarning() <<
            Q_FUNC_INFO << "Caught exception while reading" << fileName << ":" << exception.what();
         userMessage << exception.what();
         return false;
      }

      //
      // If there are ever multiple versions of DotBeer, this is where we'll work out which one to use for reading
      // this file.  For now, we just log some info.
      //
      // Note that, at this point, because we have not yet validated it against a JSON schema, we can't make any
      // assumptions about the input document - hence all the if statements in the block of code here.
      //
      // The root of a JSON document should be an object named "DotBeer"
      //
      QVersionNumber dotBeerVersion{};
      if (!inputDocument.is_object()) {
         qWarning() << Q_FUNC_INFO << "Root of" << fileName << "is not a JSON object";
      } else {
         boost::json::object const & documentRoot = inputDocument.as_object();
         if (!documentRoot.contains("DotBeer")) {
            qWarning() << Q_FUNC_INFO << "No DotBeer root object found in" << fileName;
         } else {
            boost::json::value const & dotBeerValue = documentRoot.at("DotBeer");
            if (!dotBeerValue.is_object()) {
               qWarning() << Q_FUNC_INFO << "DotBeer element in" << fileName << "is not a JSON object";
            } else {
               boost::json::object const & DotBeer = dotBeerValue.as_object();
               if (boost::json::value const * dotBeerRawVersion = DotBeer.if_contains("version");
                   !dotBeerRawVersion) {
                  qWarning() << Q_FUNC_INFO << "No version found in" << fileName;
               } else {
                  //
                  // DotBeer uses semantic versioning, which encodes a version by a three-part version number
                  // (Major.Minor.Patch) in a JSON string.  This is well-trodden ground, so, of course, there is a Qt
                  // class to help us manipulate such numbers.
                  //
                  qDebug() << Q_FUNC_INFO << "Version" << *dotBeerRawVersion << "(" << dotBeerRawVersion->kind() << ")";
                  if (boost::json::string const * dotBeerVersionJString = dotBeerRawVersion->if_string();
                      !dotBeerVersionJString) {
                     qDebug() << Q_FUNC_INFO << "Could not parse version" << dotBeerRawVersion << "in" << fileName;
                  } else {
                     QString const dotBeerVersionQString = QString::fromUtf8(dotBeerVersionJString->c_str());
                     dotBeerVersion = QVersionNumber::fromString(dotBeerVersionQString);
                     qDebug() <<
                        Q_FUNC_INFO << "DotBeer version of" << fileName << "is" << dotBeerVersionQString <<
                        "which parsed as" << dotBeerVersion;
                  }
               }
            }
         }
      }

      if (dotBeerVersion.isNull()) {
         qWarning() << Q_FUNC_INFO << "Unable to read DotBeer version from" << fileName;
         userMessage << "Invalid DotBeer file: could not read version number";
         return false;
      }

      //
      // For the moment, we are assuming that DotBeer will stay backwards compatible -- eg with a 1.1.0 schema we can
      // read a 1.0.0 file.  But if the schema used to create the file is newer than the one compiled in the program,
      // we don't know that for sure, so emit a warning.
      //
      //
      // Per the dotBeer versioning rules (see README.md in the dotBeer repository), a file written with a newer major
      // version of the schema is not something we can expect to read.  A file written with a newer minor version
      // might or might not validate against our schema (eg it will not if it uses fields that were added in the newer
      // version).  Either way, if we can't read the file, we want to tell the user that it's because it's too new,
      // rather than just showing them a schema validation error.
      //
      QString const tooNewMessage{
         QObject::tr("This file was written using version %1 of the dotBeer format, which is newer than the version "
                     "this version of %2 supports (%3).  You may need a newer version of %2 to read it.").arg(
            dotBeerVersion.toString()
         ).arg(
            CONFIG_APPLICATION_NAME_UC
         ).arg(
            dotBeerVersionWeSupport.toString()
         )
      };
      if (dotBeerVersion.majorVersion() > dotBeerVersionWeSupport.majorVersion()) {
         qWarning() <<
            Q_FUNC_INFO << "DotBeer version" << dotBeerVersion << "of" << fileName << "has newer major version than the "
            "one we support (" << dotBeerVersionWeSupport << ")";
         userMessage << tooNewMessage;
         return false;
      }

      bool const fileIsNewer{dotBeerVersion > dotBeerVersionWeSupport};
      if (fileIsNewer) {
         qWarning() <<
            Q_FUNC_INFO << "DotBeer version " << dotBeerVersion << "newer than the one we know about (" <<
            dotBeerVersionWeSupport << ")";
      }

      // If you want to check what Boost.JSON read from the file (eg to debug escaping issues etc), uncomment the next
      // line.
//      qDebug() << Q_FUNC_INFO << "JSON file read in is:" << inputDocument;

      QString loadMessage;
      QTextStream loadMessageStream{&loadMessage};
      bool const succeeded = DOT_BEER_1_CODING.validateLoadAndStoreInDb(targetFolderPath,
                                                                        inputDocument,
                                                                        loadMessageStream);
      loadMessageStream.flush();
      if (!succeeded && fileIsNewer) {
         userMessage << tooNewMessage << "\n\n";
      }
      userMessage << loadMessage;
      return succeeded;
   }

}


bool DotBeer::import(QString const & targetFolderPath,
                     QString const & filename,
                     QTextStream & userMessage) {
   // .:TODO:. This wrapper code is about the same as in BeerXML::importFromXML(), so let's try to pull out the common
   //          bits to one place.

   //
   // During importation, we do not want automatic versioning turned on because, during the process of reading in a
   // Recipe we'll end up creating load of versions of it.  The magic of RAII means it's a one-liner to suspend
   // automatic versioning, in an exception-safe way, until the end of this function.
   //
   RecipeUtils::SuspendRecipeVersioning suspendRecipeVersioning;

   //
   // Slightly more manually, we also change the cursor to show "busy" while we're doing the import as, for large
   // imports, processing can take a few seconds or so.
   //
   QApplication::setOverrideCursor(Qt::WaitCursor);
   QApplication::processEvents();
   bool const result = validateAndLoad(targetFolderPath, filename, userMessage);
   QApplication::restoreOverrideCursor();
   return result;
}

namespace {
   //
   // There are a few things that we allow (or, at least, don't make impossible) in our internal model, but which are
   // not valid in dotBeer, or which the JSON Schema can't check.
   // Some of these we fix in the model before we write it out (logging a warning in each case)...
   //
   template<class NE> void prepareForExport([[maybe_unused]] NE & ne) {
      return;
   }

   // In dotBeer, a Mash must have at least one step
   template<> void prepareForExport(Mash & mash) {
      if (mash.numSteps() > 0) {
         return;
      }
      qWarning() << Q_FUNC_INFO << mash << "has no steps, so adding a default one";
      auto mashStep = std::make_shared<MashStep>(QObject::tr("Mash step for %1").arg(mash.name()));
      // A temperature step rather than an infusion, as we don't know how much water would be used for an infusion
      mashStep->setType(MashStep::Type::Temperature);
      mashStep->setStartTemp_c(65.0);
      mashStep->setStepTime_mins(60.0);
      mash.add(mashStep);
      return;
   }

   // Similarly for Boil and Fermentation.  (In practice, we now try to make sure these always have steps -- see
   // Recipe::impl::nonOptionalItem and MainWindow::newRecipe -- but older data might not.)
   template<> void prepareForExport(Boil & boil) {
      if (boil.numSteps() > 0) {
         return;
      }
      qWarning() << Q_FUNC_INFO << boil << "has no steps, so adding standard profile";
      boil.ensureStandardProfile();
      return;
   }

   template<> void prepareForExport(Fermentation & fermentation) {
      if (fermentation.numSteps() > 0) {
         return;
      }
      qWarning() << Q_FUNC_INFO << fermentation << "has no steps, so adding a primary fermentation step";
      fermentation.add(
         std::make_shared<FermentationStep>(QObject::tr("Primary fermentation for %1").arg(fermentation.name()))
      );
      return;
   }

   // Ingredient additions must refer to steps that exist.  (This needs to be done after the Mash, Boil and Fermentation
   // have been prepared, which it will be because they are exported before Recipes.)
   template<> void prepareForExport(Recipe & recipe) {
      recipe.correctAdditionSteps();
      return;
   }

   //
   // ...and others we deal with in the JSON we have generated.
   //
   template<class NE> void tidyJson([[maybe_unused]] boost::json::array & outputArray) {
      return;
   }

   //
   // In the schedule for an addition, dotBeer allows at most one of time, gravity and pH.  We allow any combination.
   // We give priority to time, then gravity, as time is by far the most common way of specifying an addition.
   //
   template<> void tidyJson<Recipe>(boost::json::array & outputArray) {
      for (auto & recipeValue : outputArray) {
         auto & recipe = recipeValue.as_object();
         auto * ingredients = recipe.if_contains("ingredients");
         if (!ingredients || !ingredients->is_object()) {
            continue;
         }
         for (auto & [listName, additions] : ingredients->as_object()) {
            if (!additions.is_array()) {
               continue;
            }
            for (auto & addition : additions.as_array()) {
               auto * schedule = addition.is_object() ? addition.as_object().if_contains("schedule") : nullptr;
               if (!schedule || !schedule->is_object()) {
                  continue;
               }
               auto & scheduleObject = schedule->as_object();
               bool keyWritten = false;
               for (char const * const key : {"time", "specific_gravity", "pH"}) {
                  if (!scheduleObject.contains(key)) {
                     continue;
                  }
                  if (!keyWritten) {
                     keyWritten = true;
                     continue;
                  }
                  qWarning() <<
                     Q_FUNC_INFO << "Not writing" << key << "for addition in" << std::string(listName).c_str() <<
                     "of recipe" << recipe.at("name").as_string().c_str() << "as dotBeer only allows one of time, "
                     "specific gravity and pH";
                  scheduleObject.erase(key);
               }
            }
         }
      }
      return;
   }

   //
   // A dotBeer fermentable must have at least one of yield_fine_grind, yield_coarse_grind and yield_potential.  All
   // three are optional for us, and, eg, a fermentable with a yield of 0 (such as rice hulls) read in from BeerXML will
   // have none of them set (because we treat BeerXML YIELD of 0 as "not set").  So, if none is set, we write a fine
   // grind yield of 0, which is what Fermentable::effectiveFineGrindYield_pct and the calculations assume anyway.
   //
   template<> void tidyJson<Fermentable>(boost::json::array & outputArray) {
      for (auto & fermentableValue : outputArray) {
         auto & fermentable = fermentableValue.as_object();
         if (!fermentable.contains("yield_fine_grind"  ) &&
             !fermentable.contains("yield_coarse_grind") &&
             !fermentable.contains("yield_potential"   )) {
            qWarning() <<
               Q_FUNC_INFO << "Fermentable" << fermentable.at("name").as_string().c_str() << "has no yield, so writing "
               "yield_fine_grind of 0";
            fermentable["yield_fine_grind"] = boost::json::object{{"unit", "%"}, {"value", 0.0}};
         }
      }
      return;
   }

   //
   // Our Equipment has a value for every vessel, even if it's not used -- eg if the user only has a kettle, the mash
   // tun etc will have zero volume.  Rather than write out a lot of zero-volume vessels:
   //    - We omit any "optional" vessel (HLT, lauter tun, aging vessel, packaging vessel) that has zero volume, as
   //      it is treated as not being present (see Equipment::clearUnusedOptionalVessels).
   //    - We omit any other vessel (apart from the kettle, which is required) that has no non-default values.
   // When reading in, a missing vessel gives us the same zero values, so this round-trips correctly.
   //
   template<> void tidyJson<Equipment>(boost::json::array & outputArray) {
      auto vesselIsEmpty = [](boost::json::object const & vessel) {
         for (auto const & [key, value] : vessel) {
            if (value.is_string()) {
               if (!value.as_string().empty()) {
                  return false;
               }
            } else if (value.is_object() && value.as_object().contains("value") &&
                       value.as_object().at("value").is_number()) {
               if (value.as_object().at("value").to_number<double>() != 0.0) {
                  return false;
               }
            } else {
               return false;
            }
         }
         return true;
      };
      auto vesselHasNoVolume = [](boost::json::object const & vessel) {
         auto const * maxVolume = vessel.if_contains("max_volume");
         return !maxVolume ||
                (maxVolume->is_object() && maxVolume->as_object().contains("value") &&
                 maxVolume->as_object().at("value").is_number() &&
                 maxVolume->as_object().at("value").to_number<double>() <= 0.0);
      };
      for (auto & equipmentValue : outputArray) {
         auto & equipment = equipmentValue.as_object();
         for (char const * const vesselName : {"hlt", "lauter_tun", "aging_vessel", "packaging_vessel"}) {
            if (auto const * vessel = equipment.if_contains(vesselName);
                vessel && vessel->is_object() && vesselHasNoVolume(vessel->as_object())) {
               qDebug() << Q_FUNC_INFO << "Not writing optional vessel" << vesselName << "as it has zero volume";
               equipment.erase(vesselName);
            }
         }
         for (char const * const vesselName : {"mash_tun", "fermentation_vessel"}) {
            if (auto const * vessel = equipment.if_contains(vesselName);
                vessel && vessel->is_object() && vesselIsEmpty(vessel->as_object())) {
               qDebug() << Q_FUNC_INFO << "Not writing" << vesselName << "as it has no non-default values";
               equipment.erase(vesselName);
            }
         }
      }
      return;
   }
}

namespace DotBeer {
   //
   // This private implementation class holds all private non-virtual members of Exporter
   //
   class Exporter::impl {
   public:

      /**
       * Constructor
       */
      impl(Exporter & self,
         QFile & outFile,
         QTextStream & userMessage) : self{self},
                                      outFile{outFile},
                                      userMessage{userMessage},
                                      writtenToFile{false},
                                      outputDocument{} {
         // Ubuntu 22.04 ships with an old version of GCC which doesn't have support for std::format
         std::string const outputBy{
#if defined(__cpp_lib_format) && __cpp_lib_format >= 201907L
            std::format("{} v{}", CONFIG_APPLICATION_NAME_UC, CONFIG_VERSION_STRING)
#else
            QString("%1 v%2").arg(CONFIG_APPLICATION_NAME_UC, CONFIG_VERSION_STRING).toStdString()
#endif
         };

         // QDateTime::currentDateTime().toString(Qt::ISODate) omits timezone info, which we want in the timestamp,
         // hence the extra hoops here.
         QDateTime const now = QDateTime::currentDateTime();
         this->outputDocument[outputDocumentName] = {
            {"version", dotBeerVersionWeSupport.toString().toStdString()},
            //
            // We don't currently read either of the next two fields in.  They are to help diagnose problems.
            //
            {"output_by", outputBy},
            {"timestamp", now.toTimeZone(now.timeZone()).toString(Qt::ISODate).toStdString()},
         };
         return;
      }

      /**
       * Destructor
       */
      ~impl() = default;

      Exporter & self;
      QFile & outFile;
      QTextStream & userMessage;
      bool writtenToFile;

      boost::json::object outputDocument;

   };

   Exporter::Exporter(QFile & outFile, QTextStream & userMessage) :
      pimpl{std::make_unique<impl>(*this, outFile, userMessage)} {
      return;
   }

   Exporter::~Exporter() {
      this->close();
      return;
   }

   template<class NE> void Exporter::add(QString const & baseFolderPath,
                                         QList<NE const *> const & nes) {
      // Any corrections we make in prepareForExport should not create new versions of Recipes
      RecipeUtils::SuspendRecipeVersioning suspendRecipeVersioning;

      QList< std::shared_ptr<NamedEntity> > objectsToWrite;
      objectsToWrite.reserve(nes.size());
      for (NE const * ne : nes) {
         //
         // We have to cast away const on ne, as otherwise we'll end up with static_pointer to const that's harder to
         // cast away.  Or we'd have to write const and non-const versions of all the functions we're calling, which
         // is strictly correct but a bit overkill here.  (It also allows prepareForExport to make any corrections that
         // are needed for the output to be valid.)
         //
         auto neToWrite = ObjectStoreWrapper::getSharedFromRaw(const_cast<NE *>(ne));
         prepareForExport(*neToWrite);
         objectsToWrite.append(std::static_pointer_cast<NamedEntity>(neToWrite));
      }
      boost::json::array outputArray;
      JsonRecord::listToJson(baseFolderPath, objectsToWrite, outputArray, DOT_BEER_1_CODING, DOT_BEER_RECORD_DEFN<NE>);
      tidyJson<NE>(outputArray);
      this->pimpl->outputDocument[outputDocumentName].get_object()[*DOT_BEER_RECORD_DEFN<NE>.m_recordName] = outputArray;
      return;
   }

   //
   // Instantiate the above template function for the types that are going to use it
   // (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header,
   // which means, amongst other things, that we can reference the pimpl.)
   //
   template void Exporter::add(QString const & baseFolderPath, QList<Hop          const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Fermentable  const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Yeast        const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Misc         const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Water        const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Style        const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<MashStep     const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Mash         const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Fermentation const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Boil         const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Equipment    const *> const & nes);
   template void Exporter::add(QString const & baseFolderPath, QList<Recipe      const *> const & nes);

   void Exporter::close() {
      if (this->pimpl->writtenToFile) {
         return;
      }

      OStreamWriterForQFile outStream(this->pimpl->outFile);
      JsonUtils::serialize(outStream, this->pimpl->outputDocument, "  ");

      this->pimpl->writtenToFile = true;

      return;
   }

}