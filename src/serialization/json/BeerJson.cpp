/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/BeerJson.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "serialization/json/BeerJson.h"

#include <cstdlib>

// We could just include <boost/json.hpp> which pulls all the Boost.JSON headers in, but that seems overkill
#include <boost/json/kind.hpp>
#include <boost/json/parse_options.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/string.hpp>

#include <valijson/adapters/boost_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QTextStream>

#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"
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

// TODO: WE should upgrade our copy of the BeerJSON schema to the 1.0.2 release at https://github.com/beerjson/beerjson/releases/tag/v1.0.2

namespace {
   // See below for more comments on this.  If and when BeerJSON evolves then we will want separate constants for
   // min/max versions we can read plus whatever version we write.
   BtStringConst const jsonVersionWeSupport{"2.06"};

   //
   // These are mappings we use in multiple places
   //
   JsonMeasureableUnitsMapping const BEER_JSON_MASS_UNIT_MAPPER {
      // MassUnitType in measurable_units.json in BeerJSON schema
      {{"mg", &Measurement::Units::milligrams},
       { "g", &Measurement::Units::grams     },
       {"kg", &Measurement::Units::kilograms },
       {"lb", &Measurement::Units::pounds    },
       {"oz", &Measurement::Units::ounces    }}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_VOLUME_UNIT_MAPPER {
      // VolumeUnitType in measurable_units.json in BeerJSON schema
      // Note that BeerJSON does not support imperial cups, imperial tablespoons or imperial teaspoons
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
       {"ifloz", &Measurement::Units::imperial_fluidOunces},
       {"ipt"  , &Measurement::Units::imperial_pints      },
       {"iqt"  , &Measurement::Units::imperial_quarts     },
       {"igal" , &Measurement::Units::imperial_gallons    },
       {"ibbl" , &Measurement::Units::imperial_barrels    }}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_COUNT_UNIT_MAPPER {
      //
      // Strictly, we can ignore the "unit" field of a BeerJSON UnitType, but it's easier to just do the trivial mapping
      // here so that we can reuse all the same measurement handling code.
      //
      {{"1"            , &Measurement::Units::numberOf},
       {"unit"         , &Measurement::Units::numberOf},
       {"each"         , &Measurement::Units::numberOf},
       {"dimensionless", &Measurement::Units::numberOf},
       {"pkg"          , &Measurement::Units::numberOf}}
   };

   ListOfJsonMeasureableUnitsMappings const BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER {
      {&BEER_JSON_MASS_UNIT_MAPPER, &BEER_JSON_VOLUME_UNIT_MAPPER}
   };

   ListOfJsonMeasureableUnitsMappings const BEER_JSON_MASS_VOLUME_OR_COUNT_UNIT_MAPPER {
      {&BEER_JSON_MASS_UNIT_MAPPER, &BEER_JSON_VOLUME_UNIT_MAPPER, &BEER_JSON_COUNT_UNIT_MAPPER}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_TEMPERATURE_UNIT_MAPPER {
      // TemperatureUnitType in measurable_units.json in BeerJSON schema
      {{"C", &Measurement::Units::celsius   },
       {"F", &Measurement::Units::fahrenheit}}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_COLOR_UNIT_MAPPER {
      // ColorUnitType in measurable_units.json in BeerJSON schema
      {{"EBC" , &Measurement::Units::ebc     },
       {"SRM" , &Measurement::Units::srm     },
       {"Lovi", &Measurement::Units::lovibond}}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_DIASTATIC_POWER_UNIT_MAPPER {
      // DiastaticPowerUnitType in measurable_units.json in BeerJSON schema
      {{"Lintner", &Measurement::Units::lintner},
       {"WK",      &Measurement::Units::wk}}
   };

   // BitternessUnitType in measurable_units.json in BeerJSON schema
   JsonSingleUnitSpecifier const BEER_JSON_BITTERNESS_UNIT{{"IBUs"}};

   JsonMeasureableUnitsMapping const BEER_JSON_CARBONATION_UNIT_MAPPER {
      // CarbonationUnitType in measurable_units.json in BeerJSON schema
      {{"vols", &Measurement::Units::carbonationVolumes      },
       {"g/l" , &Measurement::Units::carbonationGramsPerLiter}}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER {
      // ConcentrationUnitType in measurable_units.json in BeerJSON schema
      {{"ppm" , &Measurement::Units::partsPerMillionMass},
       {"ppb" , &Measurement::Units::partsPerBillionMass},
       {"mg/l", &Measurement::Units::milligramsPerLiter }}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_DENSITY_UNIT_MAPPER {
      // GravityUnitType in measurable_units.json in BeerJSON schema
      // (See comments in measurement/Unit.h and measurement/PhysicalQuantity.h for why we stick with "density" in our
      // naming.)
      // Note that DensityUnitType is identically defined in measurable_units.json, but does not appear to be referenced
      // anywhere else.
      {{"sg"   , &Measurement::Units::specificGravity},
       {"plato", &Measurement::Units::plato          },
       {"brix" , &Measurement::Units::brix           }}
   };

   // PercentUnitType in measurable_units.json in BeerJSON schema
   JsonSingleUnitSpecifier const BEER_JSON_PERCENT_UNIT{{"%"}};

   // AcidityUnitType in measurable_units.json in BeerJSON schema
   JsonSingleUnitSpecifier const BEER_JSON_ACIDITY_UNIT{{"pH"}};

   JsonMeasureableUnitsMapping const BEER_JSON_TIME_UNIT_MAPPER {
      // TimeUnitType in measurable_units.json in BeerJSON schema
      {{"sec" , &Measurement::Units::seconds},
       {"min" , &Measurement::Units::minutes},
       {"hr"  , &Measurement::Units::hours  },
       {"day" , &Measurement::Units::days   },
       {"week", &Measurement::Units::weeks  }}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_VISCOSITY_UNIT_MAPPER {
      // ViscosityUnitType in measurable_units.json in BeerJSON schema
      {{"cP",    &Measurement::Units::centipoise       },
       {"mPa-s", &Measurement::Units::millipascalSecond}}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER {
      // SpecificHeatUnitType in measurable_units.json in BeerJSON schema
      {{"Cal/(g C)" , &Measurement::Units::caloriesPerCelsiusPerGram},
       {"J/(kg K)"  , &Measurement::Units::joulesPerKelvinPerKg     },
       {"BTU/(lb F)", &Measurement::Units::btuPerFahrenheitPerPound }}
   };

   JsonMeasureableUnitsMapping const BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER {
      // SpecificVolumeUnitType in measurable_units.json in BeerJSON schema
      {{"l/kg"   , &Measurement::Units::litresPerKilogram     },
       {"l/g"    , &Measurement::Units::litresPerGram         },
       {"m^3/kg" , &Measurement::Units::cubicMetersPerKilogram},
       {"qt/lb"  , &Measurement::Units::us_quartsPerPound     },
       {"gal/lb" , &Measurement::Units::us_gallonsPerPound    },
       {"gal/oz" , &Measurement::Units::us_gallonsPerOunce    },
       {"floz/oz", &Measurement::Units::us_fluidOuncesPerOunce},
       {"ft^3/lb", &Measurement::Units::cubicFeetPerPound     }}
   };

   //
   // We use a templated variable name as small short-cut for exporting lists of top-level objects.  Eg, if we have a
   // `QList<Hop const *>` and `QList<Fermentable const *>` that we want to export, then the compiler can automatically
   // work out that the JsonRecordDefinition objects for mapping them to BeerJSON are BEER_JSON_RECORD_DEFN<Hop>
   // and BEER_JSON_RECORD_DEFN<Fermentable> respectively.  This saves us having to have a look-up table in
   // BeerJson::Exporter::add().
   //
   // Note, however, that for reading things in from a JSON, things work differently (because we can't know at compile
   // time what a JSON file contains!), so the templated names don't buy us anything there.  Instead,
   // BEER_JSON_RECORD_DEFN_ROOT tells us how to read in top-level records from a BeerJSON file.
   //
   // In both cases, each JsonRecordDefinition object contains links to any other JsonRecordDefinition objects needed to
   // read/write contained records (eg BEER_JSON_RECORD_DEFN<Mash> contains a link to
   // BEER_JSON_RECORD_DEFN<MashStep>).
   //
   // Note too, that although we mostly use them for consistency, not all of the JsonRecordDefinition objects _need_
   // templated names.  It's only used for top-level records (see ../schemas/beerjson/1.0/beer.json and the parameters
   // of ImportExport::exportToFile).  So, eg, BEER_JSON_RECORD_DEFN<MashStep> could just as easily be called
   // BEER_JSON_RECORD_DEFN_MASH_STEP because it's only referred to inside the BEER_JSON_RECORD_DEFN<Mash>
   // definition.
   //
   // Also, some JsonRecordDefinition objects _cannot_ have templated BEER_JSON_RECORD_DEFN names, because they
   // would clash.  Eg, we need a slightly different Hop record mapping from BEER_JSON_RECORD_DEFN<Hop> inside
   // BEER_JSON_RECORD_DEFN<RecipeAdditionHop> (recipes/ingredients/hop_additions) than we do at
   // top level, so we need a separate BEER_JSON_RECORD_DEFN_HOP_IN_ADDITION record.
   //
   //
   // We only use specialisations of this template.  GCC doesn't mind not having a definition for the default cases (as
   // it's not used) but other compilers do.
   //
   // NOTE: If you find this default version of JsonRecordDefinition being used, it's a bug and probably means you
   //       forgot to define the appropriate specialisation below!
   //
   template<class NE> JsonRecordDefinition const BEER_JSON_RECORD_DEFN {
      "not_used", // JSON record name
      nullptr,
      "not_used", // namedEntityClassName
      "not_used", // localisedEntityName
      {},         // upAndDownCasters
      JsonRecordDefinition::create<JsonRecord>,
      std::initializer_list<JsonRecordDefinition::FieldDefinition>{},
      JsonRecordDefinition::RecordType::Normal
   };

   // NOTE: Field mappings below are mostly in the same order as in schemas/beerjson/1.0/beer.json  HOWEVER, we vary the
   //       order slightly to allow for the fact that some records need to refer to each other -- eg
   //       BEER_JSON_RECORD_DEFN<Mash> refers to BEER_JSON_RECORD_DEFN<MashStep>, so the latter is defined
   //       before the former.
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Top-level field mappings for BeerJSON files
   //
   // The root of a BeerJSON document is an object named "beerjson".  Inside this are some or all of the following
   // objects (where "[]" means "array of"):
   //
   //   Object Name                BeerJSON Type                Required or Optional
   //   -----------                -------------                --------------------
   //   version:                   VersionType                  required
   //   fermentables:              FermentableType[]            optional
   //   miscellaneous_ingredients: MiscellaneousType[]          optional
   //   hop_varieties:             VarietyInformation[]         optional
   //   cultures:                  CultureInformation[]         optional
   //   profiles:                  WaterBase[]                  optional
   //   styles:                    StyleType[]                  optional
   //   mashes:                    MashProcedureType[]          optional
   //   fermentations:             FermentationProcedureType[]  optional
   //   recipes:                   RecipeType[]                 optional
   //   equipments:                EquipmentType[]              optional
   //   boil:                      BoilProcedureType[]          optional
   //   packaging:                 PackagingProcedureType[]     optional
   //
   // The BeerJSON schema is strict about some things but not about others.  Eg, you can't add in your own top-level
   // object (which, eg, since JSON doesn't allow comments, would be useful to use to record information about the
   // program that wrote the file), but you can add extra fields to individual records (eg we could add a "foobar" field
   // inside each hop record and it would pass validation against the BeerJSON schema.
   //
   // Note that the way ingredients are included inside recipes is more nuanced than in BeerXML.  In BeerXML, you can
   // have eg a Hop record both as an element inside a top-level list of Hops (ie hop varieties) and as an ingredient
   // inside a Recipe.  In BeerJSON, the distinction is made between records in a top-level list of hops, which are
   // stored in VarietyInformation objects, and hop additions in a recipe, which are stored in HopAdditionType records.
   // (We might wish that VarietyInformation were named HopVariety, and HopAdditionType simply HopAddition, but such
   // naming oddities are considerably less egregious than a number of the problems with BeerXML, so it's still
   // progress.)
   //
   // VarietyInformation and HopAdditionType share some common fields, specifically those in HopVarietyBase.  (Although
   // it's NOT actually what's going on, it's helpful in some respects to think of HopVarietyBase as an abstract base
   // class from which VarietyInformation and HopAdditionType both inherit, because that's APPROXIMATELY the effect we
   // get.  Actually, JSON schemas do not actually support inheritance and, strictly speaking, what's happening is
   // schema combination which is not quite the same thing, but this is more something we would need to worry about if
   // we were designing our own schema.)  Anyway, we try to avoid duplicating definitions by having a similar structure.
   //
   // There are similar distinctions for fermentables, miscellaneous ingredients and so on.
   //
   // For historical reasons, we use Hop objects both for "hop variety" (when the object has no parent) and "use of a
   // hop in a recipe" (when the object has a parent Hop object, which should be its hop variety).  (And, again, the
   // same applies to Fermentable, Misc, Yeast and so on.)
   //
   // TBD: BeerJSON makes the distinction between, eg, a Style that you load in from the top-level "styles" array and
   //      one that you specify in a Recipe.  The latter is a subset of the former.  We need to decide how to resolve
   //      this.  Similarly, a FermentableAddition has different fields from a Fermentable.
   //
   // It would be nice to be able to make the JsonRecordDefinition::FieldDefinitions constants constexpr rather than
   // just const, but this is not yet easy because QVector cannot be constexpr, std::vector cannot yet be constexpr,
   // std::array (which can be constexpr) cannot deduce its own length when used with non-trivial types, and the
   // proposed std::make_array is still experimental and not yet actually part of std.  (There are various workarounds
   // with template metaprogramming but it's all a bit painful compared with the marginal benefit we would get.)
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for fermentables BeerJSON records - see schemas/beerjson/1.0/fermentable.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_FermentableBase {
      // Type                                                 XPath                           Q_PROPERTY                                        Value Decoder
      {JsonRecordDefinition::FieldType::String              , "name"                        , PropertyNames::NamedEntity::name                ,                                      },
      {JsonRecordDefinition::FieldType::Enum                , "type"                        , PropertyNames::Fermentable::type                , &Fermentable::typeStringMapping      },
      {JsonRecordDefinition::FieldType::String              , "origin"                      , PropertyNames::Fermentable::origin              ,                                      },
      {JsonRecordDefinition::FieldType::String              , "producer"                    , PropertyNames::Fermentable::producer            ,                                      },
      {JsonRecordDefinition::FieldType::String              , "product_id"                  , PropertyNames::Fermentable::productId           ,                                      },
      {JsonRecordDefinition::FieldType::Enum                , "grain_group"                 , PropertyNames::Fermentable::grainGroup          , &Fermentable::grainGroupStringMapping},
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield/fine_grind"            , PropertyNames::Fermentable::fineGrindYield_pct  , &BEER_JSON_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield/coarse_grind"          , PropertyNames::Fermentable::coarseGrindYield_pct, &BEER_JSON_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::SingleUnitValue     , "yield/fine_coarse_difference", PropertyNames::Fermentable::coarseFineDiff_pct  , &BEER_JSON_PERCENT_UNIT              },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "yield/potential"             , PropertyNames::Fermentable::potentialYield_sg   , &BEER_JSON_DENSITY_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color"                       , PropertyNames::Fermentable::color_srm           , &BEER_JSON_COLOR_UNIT_MAPPER         },
   };
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_FermentableType_ExclBase {
      // Type                                                       XPath               Q_PROPERTY                                          Value Decoder
      {JsonRecordDefinition::FieldType::String                    , "notes"           , PropertyNames::Fermentable::notes                 ,                                       },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "moisture"        , PropertyNames::Fermentable::moisture_pct          , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::Double                    , "alpha_amylase"   , PropertyNames::Fermentable::alphaAmylase_dextUnits,                                       },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "diastatic_power" , PropertyNames::Fermentable::diastaticPower_lintner, &BEER_JSON_DIASTATIC_POWER_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "protein"         , PropertyNames::Fermentable::protein_pct           , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::Double                    , "kolbach_index"   , PropertyNames::Fermentable::kolbachIndex_pct      ,                                       },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "max_in_batch"    , PropertyNames::Fermentable::maxInBatch_pct        , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::Bool                      , "recommend_mash"  , PropertyNames::Fermentable::recommendMash         ,                                       },
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "inventory/amount", PropertyNames::Ingredient::totalInventory         , &BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "glassy"          , PropertyNames::Fermentable::hardnessPrpGlassy_pct , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "plump"           , PropertyNames::Fermentable::kernelSizePrpPlump_pct, &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "half"            , PropertyNames::Fermentable::hardnessPrpHalf_pct   , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "mealy"           , PropertyNames::Fermentable::hardnessPrpMealy_pct  , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "thru"            , PropertyNames::Fermentable::kernelSizePrpThin_pct , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "friability"      , PropertyNames::Fermentable::friability_pct        , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "di_ph"           , PropertyNames::Fermentable::di_ph                 , &BEER_JSON_ACIDITY_UNIT               },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "viscosity"       , PropertyNames::Fermentable::viscosity_cP          , &BEER_JSON_VISCOSITY_UNIT_MAPPER      },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "dms_p"           , PropertyNames::Fermentable::dmsP_ppm              , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "fan"             , PropertyNames::Fermentable::fan_ppm               , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "fermentability"  , PropertyNames::Fermentable::fermentability_pct    , &BEER_JSON_PERCENT_UNIT               },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "beta_glucan"     , PropertyNames::Fermentable::betaGlucan_ppm        , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
   };

   // As mentioned above, it would be really nice to do this at compile time, but haven't yet found a nice way to do so
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Fermentable> {
      std::in_place_type_t<Fermentable>{},
      "fermentables", // JSON record name
      {BeerJson_FermentableBase, BeerJson_FermentableType_ExclBase}
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for hop_varieties BeerJSON records - see schemas/beerjson/1.0/hop.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_HopBase {
      // Type                                            XPath         Q_PROPERTY                        Value Decoder
      {JsonRecordDefinition::FieldType::String         , "name"      , PropertyNames::NamedEntity::name},
      {JsonRecordDefinition::FieldType::String         , "producer"  , PropertyNames::Hop::producer    },
      {JsonRecordDefinition::FieldType::String         , "product_id", PropertyNames::Hop::productId   },
      {JsonRecordDefinition::FieldType::String         , "origin"    , PropertyNames::Hop::origin      },
      {JsonRecordDefinition::FieldType::String         , "year"      , PropertyNames::Hop::year        },
      {JsonRecordDefinition::FieldType::Enum           , "form"      , PropertyNames::Hop::form        , &Hop::formStringMapping},
      {JsonRecordDefinition::FieldType::SingleUnitValue, "alpha_acid", PropertyNames::Hop::alpha_pct   , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue, "beta_acid" , PropertyNames::Hop::beta_pct    , &BEER_JSON_PERCENT_UNIT},
   };
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_HopType_ExclBase {
      // Type                                                       XPath                                Q_PROPERTY                              Value Decoder
      {JsonRecordDefinition::FieldType::Enum                      , "type"                             , PropertyNames::Hop::type              , &Hop::typeStringMapping},
      {JsonRecordDefinition::FieldType::String                    , "notes"                            , PropertyNames::Hop::notes             },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "percent_lost"                     , PropertyNames::Hop::hsi_pct           , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::String                    , "substitutes"                      , PropertyNames::Hop::substitutes       },
      {JsonRecordDefinition::FieldType::Double                    , "oil_content/total_oil_ml_per_100g", PropertyNames::Hop::totalOil_mlPer100g},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/humulene"             , PropertyNames::Hop::humulene_pct      , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/caryophyllene"        , PropertyNames::Hop::caryophyllene_pct , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/cohumulone"           , PropertyNames::Hop::cohumulone_pct    , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/myrcene"              , PropertyNames::Hop::myrcene_pct       , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/farnesene"            , PropertyNames::Hop::farnesene_pct     , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/geraniol"             , PropertyNames::Hop::geraniol_pct      , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/b_pinene"             , PropertyNames::Hop::bPinene_pct       , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/linalool"             , PropertyNames::Hop::linalool_pct      , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/limonene"             , PropertyNames::Hop::limonene_pct      , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/nerol"                , PropertyNames::Hop::nerol_pct         , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/pinene"               , PropertyNames::Hop::pinene_pct        , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/polyphenols"          , PropertyNames::Hop::polyphenols_pct   , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "oil_content/xanthohumol"          , PropertyNames::Hop::xanthohumol_pct   , &BEER_JSON_PERCENT_UNIT},
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "inventory/amount"                 , PropertyNames::Ingredient::totalInventory, &BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER},

      // .:TODO.JSON:. Note that we'll need to look at HopAdditionType, IBUEstimateType, IBUMethodType when we use Hops in Recipes
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Hop> {
      std::in_place_type_t<Hop>{},
      "hop_varieties", // JSON record name
      {BeerJson_HopBase, BeerJson_HopType_ExclBase}
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for miscellaneous_ingredients BeerJSON records - see schemas/beerjson/1.0/misc.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_MiscBase {
      // Type                                   XPath         Q_PROPERTY                        Value Decoder
      {JsonRecordDefinition::FieldType::String, "name"      , PropertyNames::NamedEntity::name},
      {JsonRecordDefinition::FieldType::String, "producer"  , PropertyNames::Misc::producer   },
      {JsonRecordDefinition::FieldType::String, "product_id", PropertyNames::Misc::productId  },
      {JsonRecordDefinition::FieldType::Enum  , "type"      , PropertyNames::Fermentable::type, &Misc::typeStringMapping},
   };
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_MiscType_ExclBase {
      // Type                                                       XPath               Q_PROPERTY                                 Value Decoder
      {JsonRecordDefinition::FieldType::String                    , "use_for"         , PropertyNames::Misc::useFor              },
      {JsonRecordDefinition::FieldType::String                    , "notes"           , PropertyNames::Misc::notes               },
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "inventory/amount", PropertyNames::Ingredient::totalInventory, &BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Misc> {
      std::in_place_type_t<Misc>{},
      "miscellaneous_ingredients", // JSON record name
      {BeerJson_MiscBase, BeerJson_MiscType_ExclBase}
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for cultures BeerJSON records - see schemas/beerjson/1.0/culture.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_YeastBase {
      // Type                                   XPath         Q_PROPERTY                        Value Decoder
      {JsonRecordDefinition::FieldType::String, "name"      , PropertyNames::NamedEntity::name},
      {JsonRecordDefinition::FieldType::Enum  , "type"      , PropertyNames::Yeast::type      , &Yeast::typeStringMapping},
      {JsonRecordDefinition::FieldType::Enum  , "form"      , PropertyNames::Yeast::form      , &Yeast::formStringMapping},
      {JsonRecordDefinition::FieldType::String, "producer"  , PropertyNames::Yeast::laboratory},
      {JsonRecordDefinition::FieldType::String, "product_id", PropertyNames::Yeast::productId },
   };
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_YeastType_ExclBase {
      // Type                                                       XPath               Q_PROPERTY                                          Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "temperature_range/minimum", PropertyNames::Yeast::minTemperature_c         , &BEER_JSON_TEMPERATURE_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "temperature_range/maximum", PropertyNames::Yeast::maxTemperature_c         , &BEER_JSON_TEMPERATURE_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "alcohol_tolerance"        , PropertyNames::Yeast::alcoholTolerance_pct     , &BEER_JSON_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::Enum                      , "flocculation"             , PropertyNames::Yeast::flocculation             , &Yeast::flocculationStringMapping },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "attenuation_range/minimum", PropertyNames::Yeast::attenuationMin_pct       , &BEER_JSON_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "attenuation_range/maximum", PropertyNames::Yeast::attenuationMax_pct       , &BEER_JSON_PERCENT_UNIT           },
      {JsonRecordDefinition::FieldType::String                    , "notes"                    , PropertyNames::Yeast::notes                    },
      {JsonRecordDefinition::FieldType::String                    , "best_for"                 , PropertyNames::Yeast::bestFor                  },
      {JsonRecordDefinition::FieldType::Int                       , "max_reuse"                , PropertyNames::Yeast::maxReuse                 },
      {JsonRecordDefinition::FieldType::Bool                      , "pof"                      , PropertyNames::Yeast::phenolicOffFlavorPositive},
      {JsonRecordDefinition::FieldType::Bool                      , "glucoamylase"             , PropertyNames::Yeast::glucoamylasePositive     },
//    TODO: Another complexity is that, for yeast/culture, inventory/amount has sub-fields:
//          liquid  -- VolumeType
//          dry     -- MassType
//          slant   -- VolumeType
//          culture -- VolumeType
//      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "inventory/amount"         , PropertyNames::Ingredient::totalInventory   , &BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::Bool                      , "zymocide/no1"             , PropertyNames::Yeast::killerProducingK1Toxin   },
      {JsonRecordDefinition::FieldType::Bool                      , "zymocide/no2"             , PropertyNames::Yeast::killerProducingK2Toxin   },
      {JsonRecordDefinition::FieldType::Bool                      , "zymocide/no28"            , PropertyNames::Yeast::killerProducingK28Toxin  },
      {JsonRecordDefinition::FieldType::Bool                      , "zymocide/klus"            , PropertyNames::Yeast::killerProducingKlusToxin },
      {JsonRecordDefinition::FieldType::Bool                      , "zymocide/neutral"         , PropertyNames::Yeast::killerNeutral            },
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Yeast> {
      std::in_place_type_t<Yeast>{},
      "cultures", // JSON record name
      {BeerJson_YeastBase, BeerJson_YeastType_ExclBase}
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for water BeerJSON records - see schemas/beerjson/1.0/water.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_WaterBase {
      // Type                                                 XPath          Q_PROPERTY                             Value Decoder
      {JsonRecordDefinition::FieldType::String              , "name"       , PropertyNames::NamedEntity::name     },
      {JsonRecordDefinition::FieldType::String              , "producer"   , BtString::NULL_STR                   }, // Not supported as not sure what this means for water...
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "calcium"    , PropertyNames::Water::calcium_ppm    , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "bicarbonate", PropertyNames::Water::bicarbonate_ppm, &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonate"  , PropertyNames::Water::carbonate_ppm  , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "potassium"  , PropertyNames::Water::potassium_ppm  , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "iron"       , PropertyNames::Water::iron_ppm       , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "nitrate"    , PropertyNames::Water::nitrate_ppm    , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "nitrite"    , PropertyNames::Water::nitrite_ppm    , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "flouride"   , PropertyNames::Water::flouride_ppm   , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "sulfate"    , PropertyNames::Water::sulfate_ppm    , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "chloride"   , PropertyNames::Water::chloride_ppm   , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "sodium"     , PropertyNames::Water::sodium_ppm     , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "magnesium"  , PropertyNames::Water::magnesium_ppm  , &BEER_JSON_MASS_FRACT_OR_CONC_UNIT_MAPPER},
   };
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_WaterType_ExclBase {
      // Type                                            XPath    Q_PROPERTY                   Value Decoder
      {JsonRecordDefinition::FieldType::SingleUnitValue, "pH"   , PropertyNames::Water::ph   , &BEER_JSON_ACIDITY_UNIT},
      {JsonRecordDefinition::FieldType::String         , "notes", PropertyNames::Water::notes,                        },
   };

   // As mentioned above, it would be really nice to do this at compile time, but haven't yet found a nice way to do so
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Water> {
      std::in_place_type_t<Water>{},
      "profiles", // JSON record name -- note that it's "profiles" NOT "waters".  (But in a Recipe, it's "water_additions"!)
      {BeerJson_WaterBase, BeerJson_WaterType_ExclBase}
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for styles BeerJSON records - see schemas/beerjson/1.0/style.json
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Style> {
      std::in_place_type_t<Style>{},
      "styles", // JSON record name
      {
         // Type                                                 XPath                                     Q_PROPERTY                               Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"                                  , PropertyNames::NamedEntity::name       },
         {JsonRecordDefinition::FieldType::String              , "category"                              , PropertyNames::Style::category         },
         {JsonRecordDefinition::FieldType::Int                 , "category_number"                       , PropertyNames::Style::categoryNumber   },
         {JsonRecordDefinition::FieldType::String              , "style_letter"                          , PropertyNames::Style::styleLetter      },
         {JsonRecordDefinition::FieldType::String              , "style_guide"                           , PropertyNames::Style::styleGuide       },
         {JsonRecordDefinition::FieldType::Enum                , "type"                                  , PropertyNames::Style::type             , &Style::typeStringMapping},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity/minimum"              , PropertyNames::Style::ogMin            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity/maximum"              , PropertyNames::Style::ogMax            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity/minimum"                 , PropertyNames::Style::fgMin            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity/maximum"                 , PropertyNames::Style::fgMax            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "international_bitterness_units/minimum", PropertyNames::Style::ibuMin           , &BEER_JSON_BITTERNESS_UNIT},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "international_bitterness_units/maximum", PropertyNames::Style::ibuMax           , &BEER_JSON_BITTERNESS_UNIT},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color/minimum"                         , PropertyNames::Style::colorMin_srm     , &BEER_JSON_COLOR_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color/maximum"                         , PropertyNames::Style::colorMax_srm     , &BEER_JSON_COLOR_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonation/minimum"                   , PropertyNames::Style::carbMin_vol      , &BEER_JSON_CARBONATION_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "carbonation/maximum"                   , PropertyNames::Style::carbMax_vol      , &BEER_JSON_CARBONATION_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume/minimum"             , PropertyNames::Style::abvMin_pct       , &BEER_JSON_PERCENT_UNIT},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume/maximum"             , PropertyNames::Style::abvMax_pct       , &BEER_JSON_PERCENT_UNIT},
         {JsonRecordDefinition::FieldType::String              , "notes"                                 , PropertyNames::Style::notes            },
         {JsonRecordDefinition::FieldType::String              , "aroma"                                 , PropertyNames::Style::aroma            },
         {JsonRecordDefinition::FieldType::String              , "appearance"                            , PropertyNames::Style::appearance       },
         {JsonRecordDefinition::FieldType::String              , "flavor"                                , PropertyNames::Style::flavor           },
         {JsonRecordDefinition::FieldType::String              , "mouthfeel"                             , PropertyNames::Style::mouthfeel        },
         {JsonRecordDefinition::FieldType::String              , "overall_impression"                    , PropertyNames::Style::overallImpression},
         {JsonRecordDefinition::FieldType::String              , "ingredients"                           , PropertyNames::Style::ingredients      },
         {JsonRecordDefinition::FieldType::String              , "examples"                              , PropertyNames::Style::examples         },
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for mash steps BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<MashStep> {
      std::in_place_type_t<MashStep>{},
      "mash_steps", // JSON record name (not actually used as MashStep always part of a Mash
      {
         // Type                                                 XPath                 Q_PROPERTY                                       Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"              , PropertyNames::NamedEntity::name               },
         {JsonRecordDefinition::FieldType::Enum                , "type"              , PropertyNames::MashStep::type                  , &MashStep::typeStringMapping          },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "amount"            , PropertyNames::MashStep::amount_l              , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_temperature"  , PropertyNames::    Step::startTemp_c           , &BEER_JSON_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"         , PropertyNames::    Step::stepTime_mins         , &BEER_JSON_TIME_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "ramp_time"         , PropertyNames::    Step::rampTime_mins         , &BEER_JSON_TIME_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_temperature"   , PropertyNames::    Step::endTemp_c             , &BEER_JSON_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::String              , "description"       , PropertyNames::    Step::description           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "water_grain_ratio" , PropertyNames::MashStep::liquorToGristRatio_lKg, &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "infuse_temperature", PropertyNames::MashStep::infuseTemp_c          , &BEER_JSON_TEMPERATURE_UNIT_MAPPER    },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_ph"          , PropertyNames::    Step::startAcidity_pH       , &BEER_JSON_ACIDITY_UNIT               },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "end_ph"            , PropertyNames::    Step::endAcidity_pH         , &BEER_JSON_ACIDITY_UNIT               },
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for mashes BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Mash> {
      std::in_place_type_t<Mash>{},
      "mashes", // JSON record name
      {
         // Type                                                 XPath                Q_PROPERTY                        Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"             , PropertyNames::NamedEntity::name},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "grain_temperature", PropertyNames::Mash::grainTemp_c, &BEER_JSON_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::String              , "notes"            , PropertyNames::Mash::notes      },
         {JsonRecordDefinition::FieldType::ListOfRecords       , "mash_steps"       , PropertyNames::Mash::mashSteps  , &BEER_JSON_RECORD_DEFN<MashStep>  },
      }
   };


   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for fermentation_steps BeerJSON records
   //
   // NB: Although FermentationStep inherits (via StepExtended) from Step, the rampTime_mins field is not used and
   //     should not be stored in the DB or serialised.  See comment in model/Step.h.
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<FermentationStep> {
      std::in_place_type_t<FermentationStep>{},
      "fermentation_steps", // JSON record name
      {
         // Type                                                 XPath                Q_PROPERTY                                    Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"             , PropertyNames::NamedEntity::name            },
         {JsonRecordDefinition::FieldType::String              , "description"      , PropertyNames::Step::description            },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_temperature", PropertyNames::        Step::startTemp_c    , &BEER_JSON_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits,   "end_temperature", PropertyNames::        Step::  endTemp_c    , &BEER_JSON_TEMPERATURE_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"        , PropertyNames::Step::stepTime_mins          , &BEER_JSON_TIME_UNIT_MAPPER       },
         {JsonRecordDefinition::FieldType::Bool                , "free_rise  "      , PropertyNames::FermentationStep::freeRise   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_gravity"    , PropertyNames::StepExtended::startGravity_sg,  &BEER_JSON_DENSITY_UNIT_MAPPER   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits,   "end_gravity"    , PropertyNames::StepExtended::  endGravity_sg,  &BEER_JSON_DENSITY_UNIT_MAPPER   },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_ph"         , PropertyNames::Step::startAcidity_pH        ,  &BEER_JSON_ACIDITY_UNIT          },
         {JsonRecordDefinition::FieldType::SingleUnitValue     ,   "end_ph"         , PropertyNames::Step::  endAcidity_pH        ,  &BEER_JSON_ACIDITY_UNIT          },
         {JsonRecordDefinition::FieldType::String              , "vessel"           , PropertyNames::FermentationStep::vessel     },
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for fermentations BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Fermentation> {
      std::in_place_type_t<Fermentation>{},
      "fermentations", // JSON record name
      {
         // Type                                          XPath                 Q_PROPERTY                                      Value Decoder
         {JsonRecordDefinition::FieldType::String       , "name"              , PropertyNames::NamedEntity::name              },
         {JsonRecordDefinition::FieldType::String       , "description"       , PropertyNames::Fermentation::description      },
         {JsonRecordDefinition::FieldType::String       , "notes"             , PropertyNames::Fermentation::notes            },
         {JsonRecordDefinition::FieldType::ListOfRecords, "fermentation_steps", PropertyNames::Fermentation::fermentationSteps, &BEER_JSON_RECORD_DEFN<FermentationStep>},
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for equipments BeerJSON records
   //
   // Yes, all the permutations below are technically allowed in BeerJSON.  I think this is a place where simplifying
   // the schema won out over precision.  Where possible, we simply ignore the field permutations that don't seem to
   // make sense (eg grain_absorption_rate on Hot Liquor Tank) or seem unimportant (eg drain_rate_per_minute on HLT).
   // However, note that some fields are required on all vessels, in particular "loss" and "maximum_volume".
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Equipment> {
      std::in_place_type_t<Equipment>{},
      "equipments", // JSON record name
      {
         // NOTE, per comment above, that we deliberately do not support certain things, on the grounds that they are
         // either meaningless or unimportant.

         // Type                                                 XPath                                                               Q_PROPERTY                                             Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"                                                            , PropertyNames::NamedEntity::name                     },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"HLT\"]/type"                              , PropertyNames::Equipment::hltType                    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/maximum_volume"                    , PropertyNames::Equipment::hltVolume_l                },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/loss"                              , PropertyNames::Equipment::hltLoss_l                  , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/grain_absorption_rate"             , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for HLT.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/boil_rate_per_hour"                , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for HLT.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/drain_rate_per_minute"             , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume unimportant for HLT.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/weight"                            , PropertyNames::Equipment::hltWeight_kg               , &BEER_JSON_MASS_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"HLT\"]/specific_heat"                     , PropertyNames::Equipment::hltSpecificHeat_calGC      , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"HLT\"]/notes"                             , PropertyNames::Equipment::hltNotes                   },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Mash Tun\"]/type"                         , PropertyNames::Equipment::mashTunType                },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/maximum_volume"               , PropertyNames::Equipment::mashTunVolume_l            , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/loss"                         , PropertyNames::Equipment::mashTunLoss_l              , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/grain_absorption_rate"        , PropertyNames::Equipment::mashTunGrainAbsorption_LKg , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/boil_rate_per_hour"           , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Mash Tun.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/drain_rate_per_minute"        , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Mash Tun.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/weight"                       , PropertyNames::Equipment::mashTunWeight_kg           , &BEER_JSON_MASS_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Mash Tun\"]/specific_heat"                , PropertyNames::Equipment::mashTunSpecificHeat_calGC  , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Mash Tun\"]/notes"                        , PropertyNames::Equipment::mashTunNotes               },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Lauter Tun\"]/type"                       , PropertyNames::Equipment::lauterTunType              },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/maximum_volume"             , PropertyNames::Equipment::lauterTunVolume_l          , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/loss"                       , PropertyNames::Equipment::lauterTunDeadspaceLoss_l   , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/grain_absorption_rate"      , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for Lauter Tun.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/boil_rate_per_hour"         , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Lauter Tun.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/drain_rate_per_minute"      , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume unimportant for Lauter Tun.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/weight"                     , PropertyNames::Equipment::lauterTunWeight_kg         , &BEER_JSON_MASS_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Lauter Tun\"]/specific_heat"              , PropertyNames::Equipment::lauterTunSpecificHeat_calGC, &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Lauter Tun\"]/notes"                      , PropertyNames::Equipment::lauterTunNotes             },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Brew Kettle\"]/type"                      , PropertyNames::Equipment::kettleType                 },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/maximum_volume"            , PropertyNames::Equipment::kettleBoilSize_l           , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/loss"                      , PropertyNames::Equipment::kettleTrubChillerLoss_l    , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/grain_absorption_rate"     , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for Kettle.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/boil_rate_per_hour"        , PropertyNames::Equipment::kettleEvaporationPerHour_l , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/drain_rate_per_minute"     , PropertyNames::Equipment::kettleOutflowPerMinute_l   , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/weight"                    , PropertyNames::Equipment::kettleWeight_kg            , &BEER_JSON_MASS_UNIT_MAPPER           },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Brew Kettle\"]/specific_heat"             , PropertyNames::Equipment::kettleSpecificHeat_calGC   , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Brew Kettle\"]/notes"                     , PropertyNames::Equipment::kettleNotes                },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Fermenter\"]/type"                        , PropertyNames::Equipment::fermenterType              },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/maximum_volume"              , PropertyNames::Equipment::fermenterBatchSize_l       , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/loss"                        , PropertyNames::Equipment::fermenterLoss_l            , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/grain_absorption_rate"       , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for Fermenter.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/boil_rate_per_hour"          , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Fermenter.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/drain_rate_per_minute"       , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume unimportant for Fermenter.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/weight"                      , BtString::NULL_STR                                   , &BEER_JSON_MASS_UNIT_MAPPER           }, // Assume unimportant for Fermenter.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Fermenter\"]/specific_heat"               , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  }, // Assume unimportant for Fermenter.
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Fermenter\"]/notes"                       , PropertyNames::Equipment::fermenterNotes             },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Aging Vessel\"]/type"                     , PropertyNames::Equipment::agingVesselType            },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/maximum_volume"           , PropertyNames::Equipment::agingVesselVolume_l        , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/loss"                     , PropertyNames::Equipment::agingVesselLoss_l          , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/grain_absorption_rate"    , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for Aging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/boil_rate_per_hour"       , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Aging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/drain_rate_per_minute"    , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume unimportant for Aging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/weight"                   , BtString::NULL_STR                                   , &BEER_JSON_MASS_UNIT_MAPPER           }, // Assume unimportant for Aging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Aging Vessel\"]/specific_heat"            , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  }, // Assume unimportant for Aging Vessel.
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Aging Vessel\"]/notes"                    , PropertyNames::Equipment::agingVesselNotes           },
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Packaging Vessel\"]/type"                 , PropertyNames::Equipment::packagingVesselType        },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/maximum_volume"       , PropertyNames::Equipment::packagingVesselVolume_l    , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/loss"                 , PropertyNames::Equipment::packagingVesselLoss_l      , &BEER_JSON_VOLUME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/grain_absorption_rate", BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_VOLUME_UNIT_MAPPER}, // Assume meaningless for Packaging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/boil_rate_per_hour"   , BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume meaningless for Packaging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/drain_rate_per_minute", BtString::NULL_STR                                   , &BEER_JSON_VOLUME_UNIT_MAPPER         }, // Assume unimportant for Packaging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/weight"               , BtString::NULL_STR                                   , &BEER_JSON_MASS_UNIT_MAPPER           }, // Assume unimportant for Packaging Vessel.
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "equipment_items[form=\"Packaging Vessel\"]/specific_heat"        , BtString::NULL_STR                                   , &BEER_JSON_SPECIFIC_HEAT_UNIT_MAPPER  }, // Assume unimportant for Packaging Vessel.
         {JsonRecordDefinition::FieldType::String              , "equipment_items[form=\"Packaging Vessel\"]/notes"                , PropertyNames::Equipment::packagingVesselNotes       },
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the FermentableBase part of FermentableAdditionType BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_FERMENTABLE_IN_ADDITION {
      std::in_place_type_t<Fermentable>{},
      "fermentable base", // JSON record name
      {BeerJson_FermentableBase},
      JsonRecordDefinition::RecordType::Outline
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the HopBase part of HopAdditionType BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_HOP_IN_ADDITION {
      std::in_place_type_t<Hop>{},
      "hop base", // JSON record name
      {BeerJson_HopBase},
      JsonRecordDefinition::RecordType::Outline
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the MiscBase part of MiscAdditionType BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_MISC_IN_ADDITION {
      std::in_place_type_t<Misc>{},
      "misc base", // JSON record name
      {BeerJson_MiscBase},
      JsonRecordDefinition::RecordType::Outline
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the YeastBase part of YeastAdditionType BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_YEAST_IN_ADDITION {
      std::in_place_type_t<Yeast>{},
      "yeast base", // JSON record name
      {BeerJson_YeastBase},
      JsonRecordDefinition::RecordType::Outline
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the WaterBase part of WaterAdditionType BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_WATER_IN_ADDITION {
      std::in_place_type_t<Water>{},
      "water base", // JSON record name
      {BeerJson_WaterBase},
      JsonRecordDefinition::RecordType::Outline
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for fermentable_additions, hop_additions, misc_additions, yeast_additions, water_additions BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // The timing bit is the same across Fermentable, Hop, Misc and Yeast
   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_IngredientAdditionType_Timing {
      // Type                                                       XPath                      Q_PROPERTY                                       Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "timing/time"            , PropertyNames::RecipeAddition::addAtTime_mins ,  &BEER_JSON_TIME_UNIT_MAPPER          },
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "timing/duration"        , PropertyNames::RecipeAddition::duration_mins  ,  &BEER_JSON_TIME_UNIT_MAPPER          },
      {JsonRecordDefinition::FieldType::Bool                      , "timing/continuous"      , BtString::NULL_STR                            }, // Not supported -- see comment in model/RecipeAddition.h
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , "timing/specific_gravity", PropertyNames::RecipeAddition::addAtGravity_sg,  &BEER_JSON_DENSITY_UNIT_MAPPER       },
      {JsonRecordDefinition::FieldType::SingleUnitValue           , "timing/pH"              , PropertyNames::RecipeAddition::addAtAcidity_pH,  &BEER_JSON_ACIDITY_UNIT              },
      {JsonRecordDefinition::FieldType::Int                       , "timing/step"            , PropertyNames::RecipeAddition::step           },
      {JsonRecordDefinition::FieldType::Enum                      , "timing/use"             , PropertyNames::RecipeAddition::stage          ,  &RecipeAddition::stageStringMapping  },
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_IngredientAdditionType_Volume {
      // Type                                                 XPath     Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::MeasurementWithUnits, "amount", PropertyNames::IngredientAmount::amount,  &BEER_JSON_VOLUME_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_IngredientAdditionType_MassOrVolume {
      // Type                                                       XPath     Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "amount", PropertyNames::IngredientAmount::amount,  &BEER_JSON_MASS_OR_VOLUME_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_IngredientAdditionType_MassVolumeOrCount {
      // Type                                                       XPath     Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, "amount", PropertyNames::IngredientAmount::amount,  &BEER_JSON_MASS_VOLUME_OR_COUNT_UNIT_MAPPER},
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_FermentableAdditionType_Base {
      // Type                                   XPath  Q_PROPERTY                                              Value Decoder
      {JsonRecordDefinition::FieldType::Record, ""   , PropertyNames::RecipeAdditionFermentable::fermentable,  &BEER_JSON_RECORD_DEFN_FERMENTABLE_IN_ADDITION},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<RecipeAdditionFermentable> {
      std::in_place_type_t<RecipeAdditionFermentable>{},
      "fermentable_additions", // JSON record name
      {BeerJson_IngredientAdditionType_Timing, BeerJson_IngredientAdditionType_MassOrVolume, BeerJson_FermentableAdditionType_Base}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_HopAdditionType_Base {
      // Type                                   XPath  Q_PROPERTY                              Value Decoder
      {JsonRecordDefinition::FieldType::Record, ""   , PropertyNames::RecipeAdditionHop::hop,  &BEER_JSON_RECORD_DEFN_HOP_IN_ADDITION},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<RecipeAdditionHop> {
      std::in_place_type_t<RecipeAdditionHop>{},
      "hop_additions", // JSON record name
      {BeerJson_IngredientAdditionType_Timing, BeerJson_IngredientAdditionType_MassOrVolume, BeerJson_HopAdditionType_Base}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_MiscAdditionType_Base {
      // Type                                   XPath  Q_PROPERTY                                Value Decoder
      {JsonRecordDefinition::FieldType::Record, ""   , PropertyNames::RecipeAdditionMisc::misc,  &BEER_JSON_RECORD_DEFN_MISC_IN_ADDITION},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<RecipeAdditionMisc> {
      std::in_place_type_t<RecipeAdditionMisc>{},
      "misc_additions", // JSON record name
      {BeerJson_IngredientAdditionType_Timing, BeerJson_IngredientAdditionType_MassVolumeOrCount, BeerJson_MiscAdditionType_Base}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_YeastAdditionType_Base {
      // Type                                   XPath  Q_PROPERTY                                  Value Decoder
      {JsonRecordDefinition::FieldType::Record, ""   , PropertyNames::RecipeAdditionYeast::yeast,  &BEER_JSON_RECORD_DEFN_YEAST_IN_ADDITION},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<RecipeAdditionYeast> {
      std::in_place_type_t<RecipeAdditionYeast>{},
      "yeast_additions", // JSON record name
      {BeerJson_IngredientAdditionType_Timing, BeerJson_IngredientAdditionType_MassVolumeOrCount, BeerJson_YeastAdditionType_Base}
   };

   std::initializer_list<JsonRecordDefinition::FieldDefinition> const BeerJson_WaterUseType_Base {
      // Type                                   XPath  Q_PROPERTY                               Value Decoder
      {JsonRecordDefinition::FieldType::Record, ""   , PropertyNames::RecipeUseOfWater::water,  &BEER_JSON_RECORD_DEFN_WATER_IN_ADDITION},
   };
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<RecipeUseOfWater> {
      std::in_place_type_t<RecipeUseOfWater>{},
      "water_additions", // JSON record name
      // NB: No timing for water_additions
      {BeerJson_IngredientAdditionType_Volume, BeerJson_WaterUseType_Base}
   };

   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<BoilStep> {
      std::in_place_type_t<BoilStep>{},
      "boil_steps", // JSON record name (not actually used as BoilStep always part of a Boil)
      {
         // Type                                                 XPath                Q_PROPERTY                                    Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"             , PropertyNames:: NamedEntity::name           },
         {JsonRecordDefinition::FieldType::String              , "description"      , PropertyNames::        Step::description    },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_temperature", PropertyNames::        Step::startTemp_c    , &BEER_JSON_TEMPERATURE_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_temperature"  , PropertyNames::        Step::endTemp_c      , &BEER_JSON_TEMPERATURE_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "ramp_time"        , PropertyNames::        Step::rampTime_mins  , &BEER_JSON_TIME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "step_time"        , PropertyNames::        Step::stepTime_mins  , &BEER_JSON_TIME_UNIT_MAPPER         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "start_gravity"    , PropertyNames::StepExtended::startGravity_sg, &BEER_JSON_DENSITY_UNIT_MAPPER      },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "end_gravity"      , PropertyNames::StepExtended::  endGravity_sg, &BEER_JSON_DENSITY_UNIT_MAPPER      },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "start_ph"         , PropertyNames::        Step::startAcidity_pH, &BEER_JSON_ACIDITY_UNIT             },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "end_ph"           , PropertyNames::        Step::endAcidity_pH  , &BEER_JSON_ACIDITY_UNIT             },
         {JsonRecordDefinition::FieldType::Enum                , "chilling_type"    , PropertyNames::    BoilStep::chillingType   , &BoilStep::chillingTypeStringMapping},
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for boil BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Boil> {
      std::in_place_type_t<Boil>{},
      "boil", // JSON record name.  Note that it's singular (for some unexplained reason)
      {
         // Type                                                 XPath            Q_PROPERTY                          Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"         , PropertyNames::NamedEntity::name  },
         {JsonRecordDefinition::FieldType::String              , "description"  , PropertyNames::Boil::description  },
         {JsonRecordDefinition::FieldType::String              , "notes"        , PropertyNames::Boil::notes        },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "pre_boil_size", PropertyNames::Boil::preBoilSize_l, &BEER_JSON_VOLUME_UNIT_MAPPER   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "boil_time"    , PropertyNames::Boil::boilTime_mins, &BEER_JSON_TIME_UNIT_MAPPER     },
         {JsonRecordDefinition::FieldType::ListOfRecords       , "boil_steps"   , PropertyNames::Boil::boilSteps    , &BEER_JSON_RECORD_DEFN<BoilStep>},
      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for recipes BeerJSON records
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> JsonRecordDefinition const BEER_JSON_RECORD_DEFN<Recipe> {
      std::in_place_type_t<Recipe>{},
      "recipes", // JSON record name
      {
         // Type                                                 XPath                      Q_PROPERTY                                Value Decoder
         {JsonRecordDefinition::FieldType::String              , "name"                   , PropertyNames::NamedEntity::name        },
         {JsonRecordDefinition::FieldType::Enum                , "type"                   , PropertyNames::Recipe::type             , &Recipe::typeStringMapping},
         {JsonRecordDefinition::FieldType::String              , "author"                 , PropertyNames::Recipe::brewer           },
         {JsonRecordDefinition::FieldType::String              , "coauthor"               , PropertyNames::Recipe::asstBrewer       },
         {JsonRecordDefinition::FieldType::Date                , "created"                , PropertyNames::Recipe::date             },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "batch_size"             , PropertyNames::Recipe::batchSize_l      , &BEER_JSON_VOLUME_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "efficiency/brewhouse"   , PropertyNames::Recipe::efficiency_pct   , &BEER_JSON_PERCENT_UNIT      },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "efficiency/conversion"  , BtString::NULL_STR                      , &BEER_JSON_PERCENT_UNIT      }, // .:TBD:. Do we want to support this optional BeerJSON field?
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "efficiency/lauter"      , BtString::NULL_STR                      , &BEER_JSON_PERCENT_UNIT      }, // .:TBD:. Do we want to support this optional BeerJSON field?
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "efficiency/mash"        , BtString::NULL_STR                      , &BEER_JSON_PERCENT_UNIT      }, // .:TBD:. Do we want to support this optional BeerJSON field?
         {JsonRecordDefinition::FieldType::Record              , "style"                  , PropertyNames::Recipe::style,
                                                                                            &BEER_JSON_RECORD_DEFN<Style>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "fermentable_additions"  , PropertyNames::Recipe::fermentableAdditions,
                                                                                            &BEER_JSON_RECORD_DEFN<RecipeAdditionFermentable>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "hop_additions"          , PropertyNames::Recipe::hopAdditions,
                                                                                            &BEER_JSON_RECORD_DEFN<RecipeAdditionHop>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "miscellaneous_additions", PropertyNames::Recipe::miscAdditions,
                                                                                            &BEER_JSON_RECORD_DEFN<RecipeAdditionMisc>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "culture_additions"      , PropertyNames::Recipe::yeastAdditions,
                                                                                            &BEER_JSON_RECORD_DEFN<RecipeAdditionYeast>},
         {JsonRecordDefinition::FieldType::ListOfRecords       , "ingredients/"
                                                                 "water_additions"        , PropertyNames::Recipe::waterUses,
                                                                                            &BEER_JSON_RECORD_DEFN<RecipeUseOfWater>},
         {JsonRecordDefinition::FieldType::Record              , "mash"                   , PropertyNames::Recipe::mash,
                                                                                            &BEER_JSON_RECORD_DEFN<Mash>},
         {JsonRecordDefinition::FieldType::String              , "notes"                  , PropertyNames::Recipe::notes         },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "original_gravity"       , PropertyNames::Recipe::og            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "final_gravity"          , PropertyNames::Recipe::fg            , &BEER_JSON_DENSITY_UNIT_MAPPER},
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "alcohol_by_volume"      , PropertyNames::Recipe::ABV_pct       , &BEER_JSON_PERCENT_UNIT       },
         // TODO: BeerJSON ibu_estimate field of recipe is IBUEstimateType ("Rager", "Tinseth", "Garetz", "Other").  We
         //       do not currently support having recipe-specific IBU estimate type, but could consider this as a future
         //       enhancement.
         {JsonRecordDefinition::FieldType::Enum                , "ibu_estimate"           , BtString::NULL_STR                   },
         {JsonRecordDefinition::FieldType::MeasurementWithUnits, "color_estimate"         , PropertyNames::Recipe::color_srm     , &BEER_JSON_COLOR_UNIT_MAPPER  },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "beer_pH"                , PropertyNames::Recipe::beerAcidity_pH, &BEER_JSON_ACIDITY_UNIT       },
         // TBD: IDK why this has no units in contrast with styles/carbonation/minimum and styles/carbonation/maximum
         //      See https://github.com/beerjson/beerjson/issues/207
         {JsonRecordDefinition::FieldType::Double              , "carbonation"            , PropertyNames::Recipe::carbonation_vols       },
         {JsonRecordDefinition::FieldType::SingleUnitValue     , "apparent_attenuation"   , PropertyNames::Recipe::apparentAttenuation_pct,
                                                                                            &BEER_JSON_PERCENT_UNIT                       },
         {JsonRecordDefinition::FieldType::Record              , "fermentation"           , PropertyNames::Recipe::fermentation           ,
                                                                                            &BEER_JSON_RECORD_DEFN<Fermentation>    },
         {JsonRecordDefinition::FieldType::Record              , "packaging"              , BtString::NULL_STR                }, // .:TODO:. We should add support for this
         {JsonRecordDefinition::FieldType::Record              , "boil"                   , PropertyNames::Recipe::boil       ,
                                                                                            &BEER_JSON_RECORD_DEFN<Boil>},
         {JsonRecordDefinition::FieldType::String              , "taste/notes"            , PropertyNames::Recipe::tasteNotes},
         {JsonRecordDefinition::FieldType::Double              , "taste/rating"           , PropertyNames::Recipe::tasteRating},
         // BeerJSON and BeerXML don't say explicitly, but it's reasonable to assume this is a US pint
         // Note that we write this out but ignore it on reading in (by dint of the Recipe constructor not looking for
         // it in the NamedParameterBundle -- per comments in JsonRecord::load and XmlRecord::load).
         {JsonRecordDefinition::FieldType::Double              , "calories_per_pint"      , PropertyNames::Recipe::caloriesPerUsPint},
      }
   };


   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for packaging BeerJSON records TODO
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      JsonRecordDefinition::create< JsonNamedEntityRecord< Packaging > >,


   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for root of BeerJSON document
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   JsonRecordDefinition const BEER_JSON_RECORD_DEFN_ROOT {
      "beerjson", // JSON record name
      nullptr,
      "",         // NamedEntity class name
      "",         // Localised name
      {},         // upAndDownCasters
      JsonRecordDefinition::create<JsonRecord>,
      {
         // Type                                             Name                         Q_PROPERTY            Value Decoder
         {JsonRecordDefinition::FieldType::RequiredConstant, "version"                  , jsonVersionWeSupport},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "fermentables"             , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Fermentable>},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "miscellaneous_ingredients", BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Misc       >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "hop_varieties"            , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Hop        >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "cultures"                 , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Yeast      >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "profiles"                 , BtString::NULL_STR  , /* TODO */},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "styles"                   , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Style      >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "mashes"                   , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Mash       >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "recipes"                  , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Recipe     >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "equipments"               , BtString::NULL_STR  , &BEER_JSON_RECORD_DEFN<Equipment  >},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "fermentations"            , BtString::NULL_STR  , /* TODO */},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "boil"                     , BtString::NULL_STR  , /* TODO */},
         {JsonRecordDefinition::FieldType::ListOfRecords   , "packaging"                , BtString::NULL_STR  , /* TODO */}
      },
      JsonRecordDefinition::RecordType::Normal
   };

   //
   // The mapping we use between BeerJSON structure and our own object structure
   //
   JsonCoding const BEER_JSON_1_CODING{
      // Yes, it is odd that BeerJSON 1.0 uses version number 2.06.  AFAICT this is because BeerJSON 1.0 was took its
      // starting point as the unfinished BeerXML 2.01 specification.
      "BeerJSON 1.0",
      *jsonVersionWeSupport, // "2.06",
      JsonSchema::Id::BEER_JSON_2_1,
      BEER_JSON_RECORD_DEFN_ROOT
   };

   //=-=-=-=-=-=-=-=-

   /**
    * \brief This function first validates the input file against a JSON schema (https://json-schema.org/)
    */
   bool validateAndLoad(QString const & fileName, QTextStream & userMessage) {
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
      // If there are ever multiple versions of BeerJSON, this is where we'll work out which one to use for reading
      // this file.  For now, we just log some info.
      //
      // Note that, at this point, because we have not yet validated it against a JSON schema, we can't make any
      // assumptions about the input document - hence all the if statements in the block of code here.
      //
      // The root of a JSON document should be an object named "beerjson"
      //
      QString beerJsonVersion = "";
      if (!inputDocument.is_object()) {
         qWarning() << Q_FUNC_INFO << "Root of" << fileName << "is not a JSON object";
      } else {
         boost::json::object const & documentRoot = inputDocument.as_object();
         if (!documentRoot.contains("beerjson")) {
            qWarning() << Q_FUNC_INFO << "No beerjson root object found in" << fileName;
         } else {
            boost::json::value const & beerJsonValue = documentRoot.at("beerjson");
            if (!beerJsonValue.is_object()) {
               qWarning() << Q_FUNC_INFO << "beerjson element in" << fileName << "is not a JSON object";
            } else {
               boost::json::object const & beerJson = beerJsonValue.as_object();
               boost::json::value const * bjVer = beerJson.if_contains("version");
               if (!bjVer) {
                  qWarning() << Q_FUNC_INFO << "No version found in" << fileName;
               } else {
                  //
                  // Version is a JSON number (in JavaScript’s double-precision floating-point format).  It would be
                  // nice if we could get hold of the raw string from the JSON file (because, really, version is
                  // integer-dot-integer so a string would be easier to parse).  However, AFAICT, there isn't a way to
                  // do this with Boost.JSON.
                  //
                  qDebug() << Q_FUNC_INFO << "Version" << *bjVer << "(" << bjVer->kind() << ")";
                  double const * bjVersion = bjVer->if_double();
                  if (!bjVersion) {
                     qDebug() << Q_FUNC_INFO << "Could not parse version" << bjVer << "in" << fileName;
                  } else {
                     qDebug() << Q_FUNC_INFO << "BeerJSON version of" << fileName << "is" << *bjVersion;
                     beerJsonVersion = QString::number(*bjVersion);
                  }
               }
            }
         }
      }

      if (beerJsonVersion.isEmpty()) {
         qWarning() << Q_FUNC_INFO << "Unable to read BeerJSON version from" << fileName;
         userMessage << "Invalid BeerJSON file: could not read version number";
         return false;
      }

      //
      // Per above, for the moment, we assume everything is BeerJSON 1.0 (using version number 2.06 per comment above)
      // and validate against that schema.
      //
      // Obviously, in time, if and when BeerJSON evolves, we'll want to do something less hard-coded here!
      //
      if (beerJsonVersion != jsonVersionWeSupport) {
         qWarning() <<
            Q_FUNC_INFO << "BeerJSON version " << beerJsonVersion << "differs from what we are expecting (" <<
            jsonVersionWeSupport << ")";
      }

      // If you want to check what Boost.JSON read from the file (eg to debug escaping issues etc), uncomment the next
      // line.
//      qDebug() << Q_FUNC_INFO << "JSON file read in is:" << inputDocument;

      return BEER_JSON_1_CODING.validateLoadAndStoreInDb(inputDocument, userMessage);
   }

}


bool BeerJson::import(QString const & filename, QTextStream & userMessage) {
   // .:TODO:. This wrapper code is about the same as in BeerXML::importFromXML(), so let's try to pull out the common
   //          bits to one place.

   //
   // During importation we do not want automatic versioning turned on because, during the process of reading in a
   // Recipe we'll end up creating load of versions of it.  The magic of RAII means it's a one-liner to suspend
   // automatic versioning, in an exception-safe way, until the end of this function.
   //
   RecipeHelper::SuspendRecipeVersioning suspendRecipeVersioning;

   //
   // Slightly more manually, we also change the cursor to show "busy" while we're doing the import as, for large
   // imports, processing can take a few seconds or so.
   //
   QApplication::setOverrideCursor(Qt::WaitCursor);
   QApplication::processEvents();
   bool result = validateAndLoad(filename, userMessage);
   QApplication::restoreOverrideCursor();
   return result;
}

namespace BeerJson {
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
         // We have to pass in jsonVersionWeSupport as a double, not a char * or a std::string, otherwise it will get
         // quotes put around it.
         this->outputDocument["beerjson"] = { {"version", std::atof(*jsonVersionWeSupport)} };
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

   template<class NE> void Exporter::add(QList<NE const *> const & nes) {
      QList< std::shared_ptr<NamedEntity> > objectsToWrite;
      objectsToWrite.reserve(nes.size());
      for (NE const * ne : nes) {
         //
         // We have to cast away const on ne, as otherwise we'll end up with static_pointer to const that's harder to
         // cast away.  Or we'd have to write const and non-const versions of all the functions we're calling, which
         // is strictly correct but a bit overkill here.
         //
         objectsToWrite.append(
            std::static_pointer_cast<NamedEntity>(ObjectStoreWrapper::getSharedFromRaw(const_cast<NE *>(ne)))
         );
      }
      boost::json::array outputArray;
      JsonRecord::listToJson(objectsToWrite, outputArray, BEER_JSON_1_CODING, BEER_JSON_RECORD_DEFN<NE>);
      this->pimpl->outputDocument["beerjson"].get_object()[*BEER_JSON_RECORD_DEFN<NE>.m_recordName] = outputArray;
      return;
   }

   //
   // Instantiate the above template function for the types that are going to use it
   // (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header,
   // which means, amongst other things, that we can reference the pimpl.)
   //
   template void Exporter::add(QList<Hop         const *> const & nes);
   template void Exporter::add(QList<Fermentable const *> const & nes);
   template void Exporter::add(QList<Yeast       const *> const & nes);
   template void Exporter::add(QList<Misc        const *> const & nes);
   template void Exporter::add(QList<Water       const *> const & nes);
   template void Exporter::add(QList<Style       const *> const & nes);
   template void Exporter::add(QList<MashStep    const *> const & nes);
   template void Exporter::add(QList<Mash        const *> const & nes);
   template void Exporter::add(QList<Equipment   const *> const & nes);
   template void Exporter::add(QList<Instruction const *> const & nes);
   template void Exporter::add(QList<BrewNote    const *> const & nes); // TBD: Not part of BeerJSON
   template void Exporter::add(QList<Recipe      const *> const & nes);

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
