/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/BeerXml.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
#include "serialization/xml/BeerXml.h"

#include <stdexcept>

#include <QApplication>
#include <QDebug>
#include <QDomNodeList>
#include <QFile>
#include <QHash>
#include <QList>
#include <QStringConverter>
#include <QTextStream>

#include "config.h" // For CONFIG_VERSION_STRING
#include "model/Boil.h" // But NB model/BoilStep.h is not needed
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h" // But NB model/FermentationStep.h is not needed
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionMisc.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUseOfWater.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "serialization/xml/BtDomErrorHandler.h"
#include "serialization/xml/MibEnum.h"
#include "serialization/xml/XmlCoding.h"
#include "serialization/xml/XmlRecord.h"

//
// Variables and constant definitions that we need only in this file
//
namespace {
   // See comment in XmlRecord.cpp about how we slightly abuse propertyName field of FieldDefinition when fieldType is
   // XmlRecord::RequiredConstant.  (This is when a required XML field holds data we don't need (and for which we always
   // write a constant value on output.)  Specifically, in BeerXML, we need to write every version of pretty much every
   // record as "1".
   BtStringConst const VERSION1{"1"};

   //
   // See comment in serialization/json/BeerJson.cpp on BEER_JSON_RECORD_DEFINITION for why we use templated variable
   // names here.
   //
   // We only use specialisations of this template.  GCC doesn't mind not having a definition for the default cases (as
   // it's not used) but other compilers do.
   //
   template<class NE> XmlRecordDefinition const BEER_XML_RECORD_DEFN {
      "not_used", // XML record name
      nullptr,
      "not_used", // namedEntityClassName
      "not_used", // localisedEntityName
      {},         // upAndDownCasters
      XmlRecordDefinition::create<XmlRecord>,
      std::initializer_list<XmlRecordDefinition::FieldDefinition>{}
   };

   //
   // Note that some parts of our model are completely outside the scope of BeerXML, including:
   //    - Freestanding BoilStep records
   //    - Freestanding FermentationStep records
   //

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <HOP>...</HOP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_HOP_TYPE_MAPPER {
      {Hop::Type::Bittering              , "Bittering"                         },
      {Hop::Type::Aroma                  , "Aroma"                             },
      {Hop::Type::AromaAndBittering      , "Both"                              },
      // These other types are in BeerJSON but are not mentioned in the BeerXML 1.0 Standard.  They get an approximate
      // mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Hop::Type::Flavor                 , "Aroma<!--Flavor-->"                },
      {Hop::Type::BitteringAndFlavor     , "Both<!--BitteringAndFlavor-->"     },
      {Hop::Type::AromaAndFlavor         , "Aroma<!--AromaAndFlavor-->"        },
      {Hop::Type::AromaBitteringAndFlavor, "Both<!--AromaBitteringAndFlavor-->"},
   };
   EnumStringMapping const BEER_XML_HOP_FORM_MAPPER {
      {Hop::Form::Leaf   , "Leaf"                },
      {Hop::Form::Pellet , "Pellet"              },
      {Hop::Form::Plug   , "Plug"                },
      // These other types are in BeerJSON but are not mentioned in the BeerXML 1.0 Standard.  They get an approximate
      // mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Hop::Form::Extract, "Pellet<!--Extract-->"},
      {Hop::Form::WetLeaf, "Leaf<!--WetLeaf-->"  },
      {Hop::Form::Powder , "Pellet<!--Powder-->" },
   };
   //
   // BeerXML has the exact same fields in a <HOP>...</HOP> record regardless of whether it's a "freestanding" hop or a
   // "recipe addition".  We need to interpret a few of the fields differently in these two circumstances.  But, since
   // we don't want to duplicate the mapping for the common fields (ie the ones that have the same interpretation in
   // both cases), we pull them out to BeerXml_HopBase (which is by analogy with BeerJson_HopBase).
   //
   // The `use` field of Hop is not part of BeerJSON and becomes an optional value now that we support BeerJSON.
   // See comment on BEER_XML_RECORD_FIELDS<Misc> for the consequences of this...
   //
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_HopBase {
      // Type                                            XPath                    Q_PROPERTY                              Value Decoder
      {XmlRecordDefinition::FieldType::String          , "NAME"                 , PropertyNames::NamedEntity::name      },
      {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"              , VERSION1                              },
      {XmlRecordDefinition::FieldType::Double          , "ALPHA"                , PropertyNames::Hop::alpha_pct         },
      {XmlRecordDefinition::FieldType::String          , "NOTES"                , PropertyNames::Hop::notes             },
      {XmlRecordDefinition::FieldType::Enum            , "TYPE"                 , PropertyNames::Hop::type              , &BEER_XML_HOP_TYPE_MAPPER},
      {XmlRecordDefinition::FieldType::Enum            , "FORM"                 , PropertyNames::Hop::form              , &BEER_XML_HOP_FORM_MAPPER},
      {XmlRecordDefinition::FieldType::Double          , "BETA"                 , PropertyNames::Hop::beta_pct          },
      {XmlRecordDefinition::FieldType::Double          , "HSI"                  , PropertyNames::Hop::hsi_pct           },
      {XmlRecordDefinition::FieldType::String          , "ORIGIN"               , PropertyNames::Hop::origin            },
      {XmlRecordDefinition::FieldType::String          , "SUBSTITUTES"          , PropertyNames::Hop::substitutes       },
      {XmlRecordDefinition::FieldType::Double          , "HUMULENE"             , PropertyNames::Hop::humulene_pct      },
      {XmlRecordDefinition::FieldType::Double          , "CARYOPHYLLENE"        , PropertyNames::Hop::caryophyllene_pct },
      {XmlRecordDefinition::FieldType::Double          , "COHUMULONE"           , PropertyNames::Hop::cohumulone_pct    },
      {XmlRecordDefinition::FieldType::Double          , "MYRCENE"              , PropertyNames::Hop::myrcene_pct       },
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_AMOUNT"       , BtString::NULL_STR                    }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "INVENTORY"            , BtString::NULL_STR                    }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_TIME"         , BtString::NULL_STR                    }, // Extension tag
      // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::String          , "PRODUCER"             , PropertyNames::Hop::producer          },
      {XmlRecordDefinition::FieldType::String          , "PRODUCT_ID"           , PropertyNames::Hop::productId         },
      {XmlRecordDefinition::FieldType::String          , "YEAR"                 , PropertyNames::Hop::year              },
      {XmlRecordDefinition::FieldType::Double          , "TOTAL_OIL_ML_PER_100G", PropertyNames::Hop::totalOil_mlPer100g},
      {XmlRecordDefinition::FieldType::Double          , "FARNESENE"            , PropertyNames::Hop::farnesene_pct     },
      {XmlRecordDefinition::FieldType::Double          , "GERANIOL"             , PropertyNames::Hop::geraniol_pct      },
      {XmlRecordDefinition::FieldType::Double          , "B_PINENE"             , PropertyNames::Hop::bPinene_pct       },
      {XmlRecordDefinition::FieldType::Double          , "LINALOOL"             , PropertyNames::Hop::linalool_pct      },
      {XmlRecordDefinition::FieldType::Double          , "LIMONENE"             , PropertyNames::Hop::limonene_pct      },
      {XmlRecordDefinition::FieldType::Double          , "NEROL"                , PropertyNames::Hop::nerol_pct         },
      {XmlRecordDefinition::FieldType::Double          , "PINENE"               , PropertyNames::Hop::pinene_pct        },
      {XmlRecordDefinition::FieldType::Double          , "POLYPHENOLS"          , PropertyNames::Hop::polyphenols_pct   },
      {XmlRecordDefinition::FieldType::Double          , "XANTHOHUMOL"          , PropertyNames::Hop::xanthohumol_pct   },
   };
   //
   // The remaining bits of freestanding Hop records in BeerXML don't make a whole lot of sense (which is almost
   // certainly why they were dropped in BeerJSON).  Essentially they relate to adding the Hop to a Recipe, except that
   // there is no Recipe.  Since they are non-optional fields in BeerXML, we have to write something when we export, but
   // we ignore the values on import.
   //
   BtStringConst const DUMMY_FREESTANDING_AMOUNT{"0.123"};
   BtStringConst const HOP_FREESTANDING_USE     {"Boil"};
   BtStringConst const DUMMY_FREESTANDING_TIME  {"12.3"};
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_HopType_ExclBase {
      // Type                                            XPath     Q_PROPERTY               Value Decoder
      {XmlRecordDefinition::FieldType::RequiredConstant, "AMOUNT", DUMMY_FREESTANDING_AMOUNT},
      {XmlRecordDefinition::FieldType::RequiredConstant, "USE"   , HOP_FREESTANDING_USE   },
      {XmlRecordDefinition::FieldType::RequiredConstant, "TIME"  , DUMMY_FREESTANDING_TIME  },
   };
   //
   // Put the two bits together and we can parse freestanding hop records
   //
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Hop> {
      std::in_place_type_t<Hop>{},
      "HOP",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord< Hop > >,
      {BeerXml_HopBase, BeerXml_HopType_ExclBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_FERMENTABLE_TYPE_MAPPER {
      {Fermentable::Type::Grain        , "Grain"              },
      {Fermentable::Type::Sugar        , "Sugar"              },
      {Fermentable::Type::Extract      , "Extract"            },
      {Fermentable::Type::Dry_Extract  , "Dry Extract"        },
      {Fermentable::Type::Other_Adjunct, "Adjunct"            },
      // These other types are in BeerJSON but are not mentioned in the BeerXML 1.0 Standard.  They get an approximate
      // mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Fermentable::Type::Fruit        , "Adjunct<!--Fruit-->"},
      {Fermentable::Type::Juice        , "Adjunct<!--Juice-->"},
      {Fermentable::Type::Honey        , "Adjunct<!--Honey-->"},
   };
   //
   // The comments above about fields in a <HOP>...</HOP> record apply, in a similar manner, to fields in a
   // <FERMENTABLE>...</FERMENTABLE> record, so we take the same approach.
   //
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_FermentableBase {
      //
      // NOTE that, with the introduction of BeerJSON, we no longer support the following BeerXML fields:
      //    ADD_AFTER_BOIL (Optional boolean) : "May be TRUE if this item is normally added after the boil.  The default
      //                                         value is FALSE since most grains are added during the mash or boil."
      //                                        This is not present in BeerJSON, and not needed for recipe formulation.
      //                                        We assume that, if a fermentable is normally added after the boil, this
      //                                        should be recorded in the NOTES field.
      // NOTE too that the yield_pct property of Fermentable is retired, and we now map the required BeerXML YIELD field
      // to the optional internal fineGrindYield_pct property of Fermentable.  (Hence the need for a default value.)
      //

      // Type                                            XPath                             Q_PROPERTY                                             Value Decoder
      {XmlRecordDefinition::FieldType::String          , "NAME"                          , PropertyNames::NamedEntity::name                     },
      {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                       , VERSION1                                             },
      {XmlRecordDefinition::FieldType::Enum            , "TYPE"                          , PropertyNames::Fermentable::type                     , &BEER_XML_FERMENTABLE_TYPE_MAPPER},
      {XmlRecordDefinition::FieldType::Double          , "YIELD"                         , PropertyNames::Fermentable::fineGrindYield_pct       , 0.0},
      {XmlRecordDefinition::FieldType::Double          , "COLOR"                         , PropertyNames::Fermentable::color_srm                },
      {XmlRecordDefinition::FieldType::Bool            , "ADD_AFTER_BOIL"                , BtString::NULL_STR                                   }, // No longer supported
      {XmlRecordDefinition::FieldType::String          , "ORIGIN"                        , PropertyNames::Fermentable::origin                   },
      {XmlRecordDefinition::FieldType::String          , "SUPPLIER"                      , PropertyNames::Fermentable::supplier                 },
      {XmlRecordDefinition::FieldType::String          , "NOTES"                         , PropertyNames::Fermentable::notes                    },
      {XmlRecordDefinition::FieldType::Double          , "COARSE_FINE_DIFF"              , PropertyNames::Fermentable::coarseFineDiff_pct       },
      {XmlRecordDefinition::FieldType::Double          , "MOISTURE"                      , PropertyNames::Fermentable::moisture_pct             },
      {XmlRecordDefinition::FieldType::Double          , "DIASTATIC_POWER"               , PropertyNames::Fermentable::diastaticPower_lintner   },
      {XmlRecordDefinition::FieldType::Double          , "PROTEIN"                       , PropertyNames::Fermentable::protein_pct              },
      {XmlRecordDefinition::FieldType::Double          , "MAX_IN_BATCH"                  , PropertyNames::Fermentable::maxInBatch_pct           },
      {XmlRecordDefinition::FieldType::Bool            , "RECOMMEND_MASH"                , PropertyNames::Fermentable::recommendMash            },
      {XmlRecordDefinition::FieldType::Double          , "IBU_GAL_PER_LB"                , PropertyNames::Fermentable::ibuGalPerLb              },
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_AMOUNT"                , BtString::NULL_STR                                   }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "POTENTIAL"                     , BtString::NULL_STR                                   }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "INVENTORY"                     , BtString::NULL_STR                                   }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_COLOR"                 , BtString::NULL_STR                                   }, // Extension tag
///      {XmlRecordDefinition::FieldType::Bool            , "IS_MASHED"                     , PropertyNames::Fermentable::isMashed                 }, // Non-standard: not part of BeerXML 1.0
      // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::Enum            , "GRAIN_GROUP"                   , PropertyNames::Fermentable::grainGroup               , &Fermentable::grainGroupStringMapping},
      {XmlRecordDefinition::FieldType::String          , "PRODUCER"                      , PropertyNames::Fermentable::producer                 },
      {XmlRecordDefinition::FieldType::String          , "PRODUCT_ID"                    , PropertyNames::Fermentable::productId                },
      {XmlRecordDefinition::FieldType::Double          , "COARSE_GRIND_YIELD"            , PropertyNames::Fermentable::coarseGrindYield_pct     },
      {XmlRecordDefinition::FieldType::Double          , "POTENTIAL_YIELD"               , PropertyNames::Fermentable::potentialYield_sg        },
      {XmlRecordDefinition::FieldType::Double          , "ALPHA_AMYLASE"                 , PropertyNames::Fermentable::alphaAmylase_dextUnits   },
      {XmlRecordDefinition::FieldType::Double          , "KOLBACH_INDEX"                 , PropertyNames::Fermentable::kolbachIndex_pct         },
      {XmlRecordDefinition::FieldType::Double          , "HARDNESS_PRP_GLASSY"           , PropertyNames::Fermentable::hardnessPrpGlassy_pct    },
      {XmlRecordDefinition::FieldType::Double          , "HARDNESS_PRP_HALF"             , PropertyNames::Fermentable::hardnessPrpHalf_pct      },
      {XmlRecordDefinition::FieldType::Double          , "HARDNESS_PRP_MEALY"            , PropertyNames::Fermentable::hardnessPrpMealy_pct     },
      {XmlRecordDefinition::FieldType::Double          , "KERNEL_SIZE_PRP_PLUMP"         , PropertyNames::Fermentable::kernelSizePrpPlump_pct   },
      {XmlRecordDefinition::FieldType::Double          , "KERNEL_SIZE_PRP_THIN"          , PropertyNames::Fermentable::kernelSizePrpThin_pct    },
      {XmlRecordDefinition::FieldType::Double          , "FRIABILITY"                    , PropertyNames::Fermentable::friability_pct           },
      {XmlRecordDefinition::FieldType::Double          , "DI"                            , PropertyNames::Fermentable::di_ph                    },
      {XmlRecordDefinition::FieldType::Double          , "VISCOSITY"                     , PropertyNames::Fermentable::viscosity_cP             },
      {XmlRecordDefinition::FieldType::Double          , "DMS_P_PPM"                     , PropertyNames::Fermentable::dmsP_ppm                 },
      {XmlRecordDefinition::FieldType::Double          , "FAN_PPM"                       , PropertyNames::Fermentable::fan_ppm                  },
      {XmlRecordDefinition::FieldType::Double          , "FERMENTABILITY"                , PropertyNames::Fermentable::fermentability_pct       },
      {XmlRecordDefinition::FieldType::Double          , "BETA_GLUCAN_PPM"               , PropertyNames::Fermentable::betaGlucan_ppm           },
   };
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_FermentableType_ExclBase {
      // Type                                            XPath     Q_PROPERTY               Value Decoder
      {XmlRecordDefinition::FieldType::RequiredConstant, "AMOUNT", DUMMY_FREESTANDING_AMOUNT},
   };
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Fermentable> {
      std::in_place_type_t<Fermentable>{},
      "FERMENTABLE",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord< Fermentable > >,
      {BeerXml_FermentableBase, BeerXml_FermentableType_ExclBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <YEAST>...</YEAST> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_YEAST_TYPE_MAPPER {
      {Yeast::Type::Ale      , "Ale"      },
      {Yeast::Type::Lager    , "Lager"    },
      {Yeast::Type::Other    , "Wheat"    }, // Wheat doesn't exist in BeerJSON and Other doesn't exist in BeerXML.  This is a bit of a bodge.
      {Yeast::Type::Wine     , "Wine"     },
      {Yeast::Type::Champagne, "Champagne"},
      // These other types are in BeerJSON but are not mentioned in the BeerXML 1.0 Standard.  They get an (extremely)
      // approximate mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Yeast::Type::Bacteria    , "Ale<!--Bacteria-->"     },
      {Yeast::Type::Brett       , "Ale<!--Brett-->"        },
      {Yeast::Type::Kveik       , "Ale<!--Kveik-->"        },
      {Yeast::Type::Lacto       , "Ale<!--Lacto-->"        },
      {Yeast::Type::Malolactic  , "Ale<!--Malolactic-->"   },
      {Yeast::Type::MixedCulture, "Ale<!--Mixed-culture-->"},
      {Yeast::Type::Pedio       , "Ale<!--Pedio-->"        },
      {Yeast::Type::Spontaneous , "Ale<!--Spontaneous-->"  },
   };
   EnumStringMapping const BEER_XML_YEAST_FORM_MAPPER {
      {Yeast::Form::Liquid , "Liquid"            },
      {Yeast::Form::Dry    , "Dry"               },
      {Yeast::Form::Slant  , "Slant"             },
      {Yeast::Form::Culture, "Culture"           },
      // This other form is in BeerJSON but is not mentioned in the BeerXML 1.0 Standard.  It gets an approximate
      // mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Yeast::Form::Dregs  , "Liquid<!--dregs-->"},
   };
   EnumStringMapping const BEER_XML_YEAST_FLOCCULATION_MAPPER {
      // The flocculations below with comments (both types!) are in BeerJSON but are not mentioned in the BeerXML 1.0
      // Standard.  They get an approximate mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      //
      // Note that we have to maintain the entries here in numerical order, otherwise we'll get an assert from
      // EnumStringMapping (because it relies on that ordering for an optimisation in how it works).
      {Yeast::Flocculation::VeryLow   , "Low<!--very low-->"     }, // ⮜⮜⮜ Added for BeerJSON support ⮞⮞⮞
      {Yeast::Flocculation::Low       , "Low"                    },
      {Yeast::Flocculation::MediumLow , "Medium<!--medium low-->"}, // ⮜⮜⮜ Added for BeerJSON support ⮞⮞⮞
      {Yeast::Flocculation::Medium    , "Medium"                 },
      {Yeast::Flocculation::MediumHigh, "High<!--medium high-->" }, // ⮜⮜⮜ Added for BeerJSON support ⮞⮞⮞
      {Yeast::Flocculation::High      , "High"                   },
      {Yeast::Flocculation::VeryHigh  , "Very High"              },
   };

   //
   // The comments above about fields in a <HOP>...</HOP> record apply, in a similar manner, to fields in a
   // <YEAST>...</YEAST> record, so we take the same approach.
   //
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_YeastBase {
      // Type                                            XPath                           Q_PROPERTY                                          Value Decoder
      {XmlRecordDefinition::FieldType::String          , "NAME"                        , PropertyNames::NamedEntity::name                  },
      {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                     , VERSION1                                          },
      {XmlRecordDefinition::FieldType::Enum            , "TYPE"                        , PropertyNames::Yeast::type                        , &BEER_XML_YEAST_TYPE_MAPPER},
      {XmlRecordDefinition::FieldType::Enum            , "FORM"                        , PropertyNames::Yeast::form                        , &BEER_XML_YEAST_FORM_MAPPER},
      {XmlRecordDefinition::FieldType::String          , "LABORATORY"                  , PropertyNames::Yeast::laboratory                  },
      {XmlRecordDefinition::FieldType::String          , "PRODUCT_ID"                  , PropertyNames::Yeast::productId                   },
      {XmlRecordDefinition::FieldType::Double          , "MIN_TEMPERATURE"             , PropertyNames::Yeast::minTemperature_c            }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::Double          , "MAX_TEMPERATURE"             , PropertyNames::Yeast::maxTemperature_c            }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::Enum            , "FLOCCULATION"                , PropertyNames::Yeast::flocculation                , &BEER_XML_YEAST_FLOCCULATION_MAPPER}, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::Double          , "ATTENUATION"                 , PropertyNames::Yeast::attenuationTypical_pct      }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::String          , "NOTES"                       , PropertyNames::Yeast::notes                       },
      {XmlRecordDefinition::FieldType::String          , "BEST_FOR"                    , PropertyNames::Yeast::bestFor                     },
      {XmlRecordDefinition::FieldType::Int             , "MAX_REUSE"                   , PropertyNames::Yeast::maxReuse                    }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_AMOUNT"              , BtString::NULL_STR                                }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "DISP_MIN_TEMP"               , BtString::NULL_STR                                }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "DISP_MAX_TEMP"               , BtString::NULL_STR                                }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "INVENTORY"                   , BtString::NULL_STR                                }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "CULTURE_DATE"                , BtString::NULL_STR                                }, // Extension tag
      // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
      // See comments in model/Yeast.h for why the optional BeerXML field "ATTENUATION" is now used only for RecipeAdditionYeast
      {XmlRecordDefinition::FieldType::Double          , "ALCOHOL_TOLERANCE"           , PropertyNames::Yeast::alcoholTolerance_pct        },
      {XmlRecordDefinition::FieldType::Double          , "ATTENUATION_MIN"             , PropertyNames::Yeast::attenuationMin_pct          },
      {XmlRecordDefinition::FieldType::Double          , "ATTENUATION_MAX"             , PropertyNames::Yeast::attenuationMax_pct          },
      {XmlRecordDefinition::FieldType::Bool            , "PHENOLIC_OFF_FLAVOR_POSITIVE", PropertyNames::Yeast::phenolicOffFlavorPositive   },
      {XmlRecordDefinition::FieldType::Bool            , "GLUCOAMYLASE_POSITIVE"       , PropertyNames::Yeast::glucoamylasePositive        },
      {XmlRecordDefinition::FieldType::Bool            , "KILLER_PRODUCING_K1_TOXIN"   , PropertyNames::Yeast::killerProducingK1Toxin      },
      {XmlRecordDefinition::FieldType::Bool            , "KILLER_PRODUCING_K2_TOXIN"   , PropertyNames::Yeast::killerProducingK2Toxin      },
      {XmlRecordDefinition::FieldType::Bool            , "KILLER_PRODUCING_K28_TOXIN"  , PropertyNames::Yeast::killerProducingK28Toxin     },
      {XmlRecordDefinition::FieldType::Bool            , "KILLER_PRODUCING_KLUS_TOXIN" , PropertyNames::Yeast::killerProducingKlusToxin    },
      {XmlRecordDefinition::FieldType::Bool            , "KILLER_NEUTRAL"              , PropertyNames::Yeast::killerNeutral               },

   };
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_YeastType_ExclBase {
      // Type                                            XPath               Q_PROPERTY                 Value Decoder
      {XmlRecordDefinition::FieldType::RequiredConstant, "AMOUNT"          , DUMMY_FREESTANDING_AMOUNT},
      // Note that the following fields are all optional in BeerXML, so we don't need to write out meaningless values
      // for them.
      {XmlRecordDefinition::FieldType::Bool            , "AMOUNT_IS_WEIGHT", BtString::NULL_STR       },
      {XmlRecordDefinition::FieldType::Double          , "ATTENUATION"     , BtString::NULL_STR       }, // Moved to RecipeAdditionYeast
      {XmlRecordDefinition::FieldType::Int             , "TIMES_CULTURED"  , BtString::NULL_STR       }, // Moved to RecipeAdditionYeast
      {XmlRecordDefinition::FieldType::Bool            , "ADD_TO_SECONDARY", BtString::NULL_STR       }, // Superseded by other RecipeAddition fields
   };
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Yeast> {
      std::in_place_type_t<Yeast>{},
      "YEAST",            // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<Yeast>>,
      {BeerXml_YeastBase, BeerXml_YeastType_ExclBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MISC>...</MISC> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_MISC_TYPE_MAPPER {
      {Misc::Type::Spice      , "Spice"           },
      {Misc::Type::Fining     , "Fining"          },
      {Misc::Type::Water_Agent, "Water Agent"     },
      {Misc::Type::Herb       , "Herb"            },
      {Misc::Type::Flavor     , "Flavor"          },
      {Misc::Type::Other      , "Other"           },
      // This other type is in BeerJSON but is not mentioned in the BeerXML 1.0 Standard.  It gets an approximate
      // mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Misc::Type::Wood       , "Other<!--Wood-->"},
   };
   // The `use` field of Misc is not part of BeerJSON and becomes an optional value now that we support BeerJSON.
   // Strictly speaking, in BeerXML, it remains a required field.  That means that, if we export a Misc that has no
   // value for `use` it will not be "correct" BeerXML.  For the moment, I think this is just something we live with.
   // However, if it turns out to create a lot of problems in real life then we'll need some special case handling to
   // force a default value in XML files.
   EnumStringMapping const BEER_XML_MISC_USE_MAPPER {
      {RecipeAdditionMisc::Use::Mash     , "Mash"     },
      {RecipeAdditionMisc::Use::Boil     , "Boil"     },
      {RecipeAdditionMisc::Use::Primary  , "Primary"  },
      {RecipeAdditionMisc::Use::Secondary, "Secondary"},
      {RecipeAdditionMisc::Use::Bottling , "Bottling" },
   };


   //
   // The comments above about fields in a <HOP>...</HOP> record apply, in a similar manner, to fields in a
   // <MISC>...</MISC> record, so we take the same approach.
   //
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_MiscBase {
      // Type                                            XPath                           Q_PROPERTY                                          Value Decoder
      {XmlRecordDefinition::FieldType::String          , "NAME"            , PropertyNames::NamedEntity::name   },
      {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"         , VERSION1                           },
      {XmlRecordDefinition::FieldType::Enum            , "TYPE"            , PropertyNames::Misc::type          , &BEER_XML_MISC_TYPE_MAPPER},
      {XmlRecordDefinition::FieldType::String          , "USE_FOR"         , PropertyNames::Misc::useFor        },
      {XmlRecordDefinition::FieldType::String          , "NOTES"           , PropertyNames::Misc::notes         },
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_AMOUNT"  , BtString::NULL_STR                 }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "INVENTORY"       , BtString::NULL_STR                 }, // Extension tag
      {XmlRecordDefinition::FieldType::String          , "DISPLAY_TIME"    , BtString::NULL_STR                 }, // Extension tag
      // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::String          , "PRODUCER"        , PropertyNames::Misc::producer      },
      {XmlRecordDefinition::FieldType::String          , "PRODUCT_ID"      , PropertyNames::Misc::productId     },
   };
   BtStringConst const MISC_FREESTANDING_USE     {"Boil"};
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_MiscType_ExclBase {
      // Type                                            XPath     Q_PROPERTY               Value Decoder
      {XmlRecordDefinition::FieldType::RequiredConstant, "AMOUNT", DUMMY_FREESTANDING_AMOUNT},
      {XmlRecordDefinition::FieldType::RequiredConstant, "USE"   , MISC_FREESTANDING_USE   },
      {XmlRecordDefinition::FieldType::RequiredConstant, "TIME"  , DUMMY_FREESTANDING_TIME  },
   };
   //
   // Put the two bits together and we can parse freestanding misc records
   //
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Misc> {
      std::in_place_type_t<Misc>{},
      "MISC",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord< Misc > >,
      {BeerXml_MiscBase, BeerXml_MiscType_ExclBase}
   };

   //
   // The comments above about fields in a <HOP>...</HOP> record apply, in a similar manner, to fields in a
   // <WATER>...</WATER> record, so we take the same approach.
   //
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_WaterBase {
      // Type                                            XPath             Q_PROPERTY                             Value Decoder
      {XmlRecordDefinition::FieldType::String          , "NAME"          , PropertyNames::NamedEntity::name     },
      {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"       , VERSION1                             },
      {XmlRecordDefinition::FieldType::Double          , "CALCIUM"       , PropertyNames::Water::calcium_ppm    },
      {XmlRecordDefinition::FieldType::Double          , "BICARBONATE"   , PropertyNames::Water::bicarbonate_ppm},
      {XmlRecordDefinition::FieldType::Double          , "SULFATE"       , PropertyNames::Water::sulfate_ppm    },
      {XmlRecordDefinition::FieldType::Double          , "CHLORIDE"      , PropertyNames::Water::chloride_ppm   },
      {XmlRecordDefinition::FieldType::Double          , "SODIUM"        , PropertyNames::Water::sodium_ppm     },
      {XmlRecordDefinition::FieldType::Double          , "MAGNESIUM"     , PropertyNames::Water::magnesium_ppm  },
      {XmlRecordDefinition::FieldType::Double          , "PH"            , PropertyNames::Water::ph             },
      {XmlRecordDefinition::FieldType::String          , "NOTES"         , PropertyNames::Water::notes          },
      // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
      {XmlRecordDefinition::FieldType::Double          , "CARBONATE_PPM" , PropertyNames::Water::carbonate_ppm  },
      {XmlRecordDefinition::FieldType::Double          , "POTASSIUM_PPM" , PropertyNames::Water::potassium_ppm  },
      {XmlRecordDefinition::FieldType::Double          , "IRON_PPM"      , PropertyNames::Water::iron_ppm       },
      {XmlRecordDefinition::FieldType::Double          , "NITRATE_PPM"   , PropertyNames::Water::nitrate_ppm    },
      {XmlRecordDefinition::FieldType::Double          , "NITRITE_PPM"   , PropertyNames::Water::nitrite_ppm    },
      {XmlRecordDefinition::FieldType::Double          , "FLUORIDE_PPM"  , PropertyNames::Water::fluoride_ppm   },
   };
   std::initializer_list<XmlRecordDefinition::FieldDefinition> const BeerXml_WaterType_ExclBase {
      // Type                                            XPath     Q_PROPERTY               Value Decoder
      {XmlRecordDefinition::FieldType::RequiredConstant, "AMOUNT", DUMMY_FREESTANDING_AMOUNT},
      // Note that DISPLAY_AMOUNT is an extension tag, thus optional in BeerXML, so we don't need to write it out when
      // there is no meaningful value to record.
   };
   //
   // Put the two bits together and we can parse freestanding water records
   //
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Water> {
      std::in_place_type_t<Water>{},
      "WATER",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord< Water > >,
      {BeerXml_WaterBase, BeerXml_WaterType_ExclBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <STYLE>...</STYLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_STYLE_TYPE_MAPPER {
      // See comment in model/Style.h for more on the mapping here.  TLDR is that our style types are now based on those
      // in BeerJSON, which are somewhat different than those in BeerXML.  This is tricky as we still need to be able to
      // map in both directions, ie to and from BeerXML.  The least inaccurate way to do this would be to have two
      // mappings: one for each direction.  However, I'm loathe to extend the BeerXML code to add support for dual
      // mappings just for this one field.  So, for the moment at least, we make do with a suboptimal bidirectional mapping
      {Style::Type::Beer    , "Ale"             },
      {Style::Type::Cider   , "Cider"           },
      {Style::Type::Mead    , "Mead"            },
      {Style::Type::Kombucha, "Wheat"           },
      {Style::Type::Soda    , "Mixed"           },
      {Style::Type::Wine    , "Mixed<!--Wine-->"},
      {Style::Type::Other   , "Lager"           },
   };
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Style> {
      std::in_place_type_t<Style>{},
      "STYLE",            // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<Style>>,
      {
         // Type                                            XPath                 Q_PROPERTY                               Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"              , PropertyNames::NamedEntity::name       },
         {XmlRecordDefinition::FieldType::String          , "CATEGORY"          , PropertyNames::Style::category         },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"           , VERSION1                               },
         {XmlRecordDefinition::FieldType::String          , "CATEGORY_NUMBER"   , PropertyNames::Style::categoryNumber   }, // NB: Despite the name, this is specified as Text in the BeerXML 1.0 standard
         {XmlRecordDefinition::FieldType::String          , "STYLE_LETTER"      , PropertyNames::Style::styleLetter      },
         {XmlRecordDefinition::FieldType::String          , "STYLE_GUIDE"       , PropertyNames::Style::styleGuide       },
         {XmlRecordDefinition::FieldType::Enum            , "TYPE"              , PropertyNames::Style::type             , &BEER_XML_STYLE_TYPE_MAPPER},
         {XmlRecordDefinition::FieldType::Double          , "OG_MIN"            , PropertyNames::Style::ogMin            },
         {XmlRecordDefinition::FieldType::Double          , "OG_MAX"            , PropertyNames::Style::ogMax            },
         {XmlRecordDefinition::FieldType::Double          , "FG_MIN"            , PropertyNames::Style::fgMin            },
         {XmlRecordDefinition::FieldType::Double          , "FG_MAX"            , PropertyNames::Style::fgMax            },
         {XmlRecordDefinition::FieldType::Double          , "IBU_MIN"           , PropertyNames::Style::ibuMin           },
         {XmlRecordDefinition::FieldType::Double          , "IBU_MAX"           , PropertyNames::Style::ibuMax           },
         {XmlRecordDefinition::FieldType::Double          , "COLOR_MIN"         , PropertyNames::Style::colorMin_srm     },
         {XmlRecordDefinition::FieldType::Double          , "COLOR_MAX"         , PropertyNames::Style::colorMax_srm     },
         {XmlRecordDefinition::FieldType::Double          , "CARB_MIN"          , PropertyNames::Style::carbMin_vol      },
         {XmlRecordDefinition::FieldType::Double          , "CARB_MAX"          , PropertyNames::Style::carbMax_vol      },
         {XmlRecordDefinition::FieldType::Double          , "ABV_MIN"           , PropertyNames::Style::abvMin_pct       },
         {XmlRecordDefinition::FieldType::Double          , "ABV_MAX"           , PropertyNames::Style::abvMax_pct       },
         {XmlRecordDefinition::FieldType::String          , "NOTES"             , PropertyNames::Style::notes            },
         // BeerXML's profile field becomes two fields, aroma and flavor, in BeerJSON (which our properties now follow).
         // Strictly, when writing to BeerXML we should concatenate our aroma and flavour properties into profile.  But
         // that's not an easily-reversible operation.  So, for now, we map profile to flavor and treat aroma as an
         // extension tag.
         {XmlRecordDefinition::FieldType::String          , "PROFILE"           , PropertyNames::Style::flavor           }, // Was PropertyNames::Style::profile -- see comment immediately above
         {XmlRecordDefinition::FieldType::String          , "INGREDIENTS"       , PropertyNames::Style::ingredients      },
         {XmlRecordDefinition::FieldType::String          , "EXAMPLES"          , PropertyNames::Style::examples         },
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_OG_MIN"    , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_OG_MAX"    , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_FG_MIN"    , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_FG_MAX"    , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_COLOR_MIN" , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_COLOR_MAX" , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "OG_RANGE"          , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "FG_RANGE"          , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "IBU_RANGE"         , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "CARB_RANGE"        , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "COLOR_RANGE"       , BtString::NULL_STR                     }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "ABV_RANGE"         , BtString::NULL_STR                     }, // Extension tag
         // ⮜⮜⮜ Following are new fields that BeerJSON adds, and which we include when we write to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::String          , "AROMA"             , PropertyNames::Style::aroma            },
         {XmlRecordDefinition::FieldType::String          , "APPEARANCE"        , PropertyNames::Style::appearance       },
         {XmlRecordDefinition::FieldType::String          , "MOUTHFEEL"         , PropertyNames::Style::mouthfeel        },
         {XmlRecordDefinition::FieldType::String          , "OVERALL_IMPRESSION", PropertyNames::Style::overallImpression},
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH_STEP>...</MASH_STEP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_MASH_STEP_TYPE_MAPPER {
      {MashStep::Type::Infusion   , "Infusion"                      },
      {MashStep::Type::Temperature, "Temperature"                   },
      {MashStep::Type::Decoction  , "Decoction"                     },
      // We also have MashStep::FlySparge and MashStep::BatchSparge which are not mentioned in the
      // BeerXML 1.0 Standard.  They get treated as "Infusion" when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings from "Infusion"
      {MashStep::Type::FlySparge  , "Infusion<!-- Fly Sparge -->"   },
      {MashStep::Type::BatchSparge, "Infusion<!-- Batch Sparge -->" },
      // Similarly, BeerJSON adds another couple of mash step types
      {MashStep::Type::SouringMash, "Decoction<!-- Souring Mash -->"},
      {MashStep::Type::SouringWort, "Decoction<!-- Souring Wort -->"},

   };
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<MashStep> {
      std::in_place_type_t<MashStep>{},
      "MASH_STEP",           // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<MashStep>>,
      {
         // Type                                            XPath                        Q_PROPERTY                                       Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"                     , PropertyNames::NamedEntity::name               },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                  , VERSION1                                       },
         {XmlRecordDefinition::FieldType::Enum            , "TYPE"                     , PropertyNames::MashStep::type                  , &BEER_XML_MASH_STEP_TYPE_MAPPER},
         {XmlRecordDefinition::FieldType::Double          , "INFUSE_AMOUNT"            , PropertyNames::MashStep::infuseAmount_l        }, // Should not be supplied if TYPE is "Decoction"
         {XmlRecordDefinition::FieldType::Double          , "STEP_TEMP"                , PropertyNames::StepBase::startTemp_c           },
         {XmlRecordDefinition::FieldType::Double          , "STEP_TIME"                , PropertyNames::StepBase::stepTime_mins         },
         {XmlRecordDefinition::FieldType::Double          , "RAMP_TIME"                , PropertyNames::StepBase::rampTime_mins         },
         {XmlRecordDefinition::FieldType::Double          , "END_TEMP"                 , PropertyNames::    Step::endTemp_c             },
         {XmlRecordDefinition::FieldType::String          , "DESCRIPTION"              , PropertyNames::    Step::description           }, // Extension tag ⮜⮜⮜ Support added as part of BeerJSON work ⮞⮞⮞
         {XmlRecordDefinition::FieldType::String          , "WATER_GRAIN_RATIO"        , BtString::NULL_STR                             }, // Extension tag NB: Similar to LIQUOR_TO_GRIST_RATIO_LKG below, but STRING including unit names
         {XmlRecordDefinition::FieldType::String          , "DECOCTION_AMT"            , BtString::NULL_STR                             }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "INFUSE_TEMP"              , BtString::NULL_STR                             }, // Extension tag NB: Similar to INFUSE_TEMP_C below, but STRING including unit names
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_STEP_TEMP"        , BtString::NULL_STR                             }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_INFUSE_AMT"       , BtString::NULL_STR                             }, // Extension tag
         {XmlRecordDefinition::FieldType::Double          , "INFUSE_TEMP_C"            , PropertyNames::MashStep::infuseTemp_c          }, // Non-standard: not part of BeerXML 1.0
         {XmlRecordDefinition::FieldType::Double          , "DECOCTION_AMOUNT"         , PropertyNames::MashStep::decoctionAmount_l     }, // Non-standard: not part of BeerXML 1.0
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Double          , "LIQUOR_TO_GRIST_RATIO_LKG", PropertyNames::MashStep::liquorToGristRatio_lKg},
         {XmlRecordDefinition::FieldType::Double          , "START_ACIDITY_PH"         , PropertyNames::    Step::startAcidity_pH       },
         {XmlRecordDefinition::FieldType::Double          , "END_ACIDITY_PH"           , PropertyNames::    Step::endAcidity_pH         },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH>...</MASH> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Mash> {
      std::in_place_type_t<Mash>{},
      "MASH",            // XML record name
      XmlRecordDefinition::create<XmlMashRecord>,
      {
         // Type                                            XPath                   Q_PROPERTY                                      Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"                , PropertyNames::NamedEntity::name              },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"             , VERSION1                                      },
         {XmlRecordDefinition::FieldType::Double          , "GRAIN_TEMP"          , PropertyNames::Mash::grainTemp_c              },
         {XmlRecordDefinition::FieldType::ListOfRecords   , "MASH_STEPS/MASH_STEP", PropertyNames::StepOwnerBase::steps        , &BEER_XML_RECORD_DEFN<MashStep>}, // Additional logic for "MASH-STEPS" is handled in code
         {XmlRecordDefinition::FieldType::String          , "NOTES"               , PropertyNames::Mash::notes                    },
         {XmlRecordDefinition::FieldType::Double          , "TUN_TEMP"            , PropertyNames::Mash::tunTemp_c                },
         {XmlRecordDefinition::FieldType::Double          , "SPARGE_TEMP"         , PropertyNames::Mash::spargeTemp_c             },
         {XmlRecordDefinition::FieldType::Double          , "PH"                  , PropertyNames::Mash::ph                       },
         {XmlRecordDefinition::FieldType::Double          , "TUN_WEIGHT"          , PropertyNames::Mash::mashTunWeight_kg         },
         {XmlRecordDefinition::FieldType::Double          , "TUN_SPECIFIC_HEAT"   , PropertyNames::Mash::mashTunSpecificHeat_calGC},
         {XmlRecordDefinition::FieldType::Bool            , "EQUIP_ADJUST"        , PropertyNames::Mash::equipAdjust              },
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_GRAIN_TEMP"  , BtString::NULL_STR                            }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TUN_TEMP"    , BtString::NULL_STR                            }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_SPARGE_TEMP" , BtString::NULL_STR                            }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TUN_WEIGHT"  , BtString::NULL_STR                            }, // Extension tag
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
   //
   // Fields marked ‡ are non-standard, in that they are not part of BeerXML 1.0, so likely will not be recognised by
   // other programs.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Equipment> {
      std::in_place_type_t<Equipment>{},
      "EQUIPMENT",            // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<Equipment>>,
      {
         // Type                                            XPath                             Q_PROPERTY                                             Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"                          , PropertyNames::NamedEntity::name                     },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                       , VERSION1                                             },
         {XmlRecordDefinition::FieldType::Double          , "BOIL_SIZE"                     , PropertyNames::Equipment::kettleBoilSize_l           },
         {XmlRecordDefinition::FieldType::Double          , "BATCH_SIZE"                    , PropertyNames::Equipment::fermenterBatchSize_l       },
         {XmlRecordDefinition::FieldType::Double          , "TUN_VOLUME"                    , PropertyNames::Equipment::mashTunVolume_l            },
         {XmlRecordDefinition::FieldType::Double          , "TUN_WEIGHT"                    , PropertyNames::Equipment::mashTunWeight_kg           },
         {XmlRecordDefinition::FieldType::Double          , "TUN_SPECIFIC_HEAT"             , PropertyNames::Equipment::mashTunSpecificHeat_calGC  },
         {XmlRecordDefinition::FieldType::Double          , "TOP_UP_WATER"                  , PropertyNames::Equipment::topUpWater_l               },
         {XmlRecordDefinition::FieldType::Double          , "TRUB_CHILLER_LOSS"             , PropertyNames::Equipment::kettleTrubChillerLoss_l    },
         {XmlRecordDefinition::FieldType::Double          , "EVAP_RATE"                     , PropertyNames::Equipment::evapRate_pctHr             },
         {XmlRecordDefinition::FieldType::Double          , "BOIL_TIME"                     , PropertyNames::Equipment::boilTime_min               },
         {XmlRecordDefinition::FieldType::Bool            , "CALC_BOIL_VOLUME"              , PropertyNames::Equipment::calcBoilVolume             },
         {XmlRecordDefinition::FieldType::Double          , "LAUTER_DEADSPACE"              , PropertyNames::Equipment::lauterTunDeadspaceLoss_l   },
         {XmlRecordDefinition::FieldType::Double          , "TOP_UP_KETTLE"                 , PropertyNames::Equipment::topUpKettle_l              },
         {XmlRecordDefinition::FieldType::Double          , "HOP_UTILIZATION"               , PropertyNames::Equipment::hopUtilization_pct         },
         // See comment in model/Equipment.h for why NOTES maps to brewKettleNotes
         {XmlRecordDefinition::FieldType::String          , "NOTES"                         , PropertyNames::Equipment::kettleNotes                },
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_BOIL_SIZE"             , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_BATCH_SIZE"            , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TUN_VOLUME"            , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TUN_WEIGHT"            , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TOP_UP_WATER"          , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TRUB_CHILLER_LOSS"     , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_LAUTER_DEADSPACE"      , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TOP_UP_KETTLE"         , BtString::NULL_STR                                   }, // Extension tag
         {XmlRecordDefinition::FieldType::Double          , "REAL_EVAP_RATE"                , PropertyNames::Equipment::kettleEvaporationPerHour_l }, // ‡
         {XmlRecordDefinition::FieldType::Double          , "ABSORPTION"                    , PropertyNames::Equipment::mashTunGrainAbsorption_LKg }, // ‡
         {XmlRecordDefinition::FieldType::Double          , "BOILING_POINT"                 , PropertyNames::Equipment::boilingPoint_c             }, // ‡
         {XmlRecordDefinition::FieldType::Double          , "KETTLEINTERNALDIAMETER_CM"     , PropertyNames::Equipment::kettleInternalDiameter_cm  }, // ‡
         {XmlRecordDefinition::FieldType::Double          , "KETTLEOPENINGDIAMETER_CM"      , PropertyNames::Equipment::kettleOpeningDiameter_cm   }, // ‡
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::String          , "HLT_TYPE"                      , PropertyNames::Equipment::hltType                    },
         {XmlRecordDefinition::FieldType::String          , "MASH_TUN_TYPE"                 , PropertyNames::Equipment::mashTunType                },
         {XmlRecordDefinition::FieldType::String          , "LAUTER_TUN_TYPE"               , PropertyNames::Equipment::lauterTunType              },
         {XmlRecordDefinition::FieldType::String          , "KETTLE_TYPE"                   , PropertyNames::Equipment::kettleType                 },
         {XmlRecordDefinition::FieldType::String          , "FERMENTER_TYPE"                , PropertyNames::Equipment::fermenterType              },
         {XmlRecordDefinition::FieldType::String          , "AGINGVESSEL_TYPE"              , PropertyNames::Equipment::agingVesselType            },
         {XmlRecordDefinition::FieldType::String          , "PACKAGING_VESSEL_TYPE"         , PropertyNames::Equipment::packagingVesselType        },
         {XmlRecordDefinition::FieldType::Double          , "HLT_VOLUME_L"                  , PropertyNames::Equipment::hltVolume_l                },
         {XmlRecordDefinition::FieldType::Double          , "LAUTER_TUN_VOLUME_L"           , PropertyNames::Equipment::lauterTunVolume_l          },
         {XmlRecordDefinition::FieldType::Double          , "AGING_VESSEL_VOLUME_L"         , PropertyNames::Equipment::agingVesselVolume_l        },
         {XmlRecordDefinition::FieldType::Double          , "PACKAGING_VESSEL_VOLUME_L"     , PropertyNames::Equipment::packagingVesselVolume_l    },
         {XmlRecordDefinition::FieldType::Double          , "HLT_LOSS_L"                    , PropertyNames::Equipment::hltLoss_l                  },
         {XmlRecordDefinition::FieldType::Double          , "MASH_TUN_LOSS_L"               , PropertyNames::Equipment::mashTunLoss_l              },
         {XmlRecordDefinition::FieldType::Double          , "FERMENTER_LOSS_L"              , PropertyNames::Equipment::fermenterLoss_l            },
         {XmlRecordDefinition::FieldType::Double          , "AGING_VESSEL_LOSS_L"           , PropertyNames::Equipment::agingVesselLoss_l          },
         {XmlRecordDefinition::FieldType::Double          , "PACKAGING_VESSEL_LOSS_L"       , PropertyNames::Equipment::packagingVesselLoss_l      },
         {XmlRecordDefinition::FieldType::Double          , "KETTLE_OUTFLOW_PER_MINUTE_L"   , PropertyNames::Equipment::kettleOutflowPerMinute_l   },
         {XmlRecordDefinition::FieldType::Double          , "HLT_WEIGHT_KG"                 , PropertyNames::Equipment::hltWeight_kg               },
         {XmlRecordDefinition::FieldType::Double          , "LAUTER_TUN_WEIGHT_KG"          , PropertyNames::Equipment::lauterTunWeight_kg         },
         {XmlRecordDefinition::FieldType::Double          , "KETTLE_WEIGHT_KG"              , PropertyNames::Equipment::kettleWeight_kg            },
         {XmlRecordDefinition::FieldType::Double          , "HLT_SPECIFIC_HEAT_CALGC"       , PropertyNames::Equipment::hltSpecificHeat_calGC      },
         {XmlRecordDefinition::FieldType::Double          , "LAUTER_TUN_SPECIFIC_HEAT_CALGC", PropertyNames::Equipment::lauterTunSpecificHeat_calGC},
         {XmlRecordDefinition::FieldType::Double          , "KETTLE_SPECIFIC_HEAT_CALGC"    , PropertyNames::Equipment::kettleSpecificHeat_calGC   },
         {XmlRecordDefinition::FieldType::String          , "HLT_NOTES"                     , PropertyNames::Equipment::hltNotes                   },
         {XmlRecordDefinition::FieldType::String          , "MASH_TUN_NOTES"                , PropertyNames::Equipment::mashTunNotes               },
         {XmlRecordDefinition::FieldType::String          , "LAUTER_TUN_NOTES"              , PropertyNames::Equipment::lauterTunNotes             },
         {XmlRecordDefinition::FieldType::String          , "FERMENTER_NOTES"               , PropertyNames::Equipment::fermenterNotes             },
         {XmlRecordDefinition::FieldType::String          , "AGING_VESSEL_NOTES"            , PropertyNames::Equipment::agingVesselNotes           },
         {XmlRecordDefinition::FieldType::String          , "PACKAGING_VESSEL_NOTES"        , PropertyNames::Equipment::packagingVesselNotes       },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <INSTRUCTION>...</INSTRUCTION> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Instruction> {
      std::in_place_type_t<Instruction>{},
      "INSTRUCTION",            // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<Instruction>>,
      {
         // Type                                            XPath         Q_PROPERTY                              Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"      , PropertyNames::NamedEntity::name      },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"   , VERSION1                              },
         {XmlRecordDefinition::FieldType::String          , "directions", PropertyNames::Instruction::directions},
         {XmlRecordDefinition::FieldType::Bool            , "hasTimer"  , PropertyNames::Instruction::hasTimer  },
         {XmlRecordDefinition::FieldType::String          , "timervalue", PropertyNames::Instruction::timerValue}, // NB XPath is lowercase and property is camelCase
         {XmlRecordDefinition::FieldType::Bool            , "completed" , PropertyNames::Instruction::completed },
         {XmlRecordDefinition::FieldType::Double          , "interval"  , PropertyNames::Instruction::interval  },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <BREWNOTE>...</BREWNOTE> BeerXML records
   // NB There is no NAME field on a BREWNOTE
   //
   // Since this is only used by Brewtarget/Brewken, we could probably lose the VERSION field here (with  corresponding
   // changes to BeerXml.xsd), at the cost of creating files that would not be readable by old versions of those
   // programs.  But it seems small bother to leave it be.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<BrewNote> {
      std::in_place_type_t<BrewNote>{},
      "BREWNOTE",            // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<BrewNote>>,
      {
         // Type                                            XPath                      Q_PROPERTY                                  Value Decoder
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                , VERSION1                                  },
         {XmlRecordDefinition::FieldType::Date            , "BREWDATE"               , PropertyNames::BrewNote::brewDate         },
         {XmlRecordDefinition::FieldType::Date            , "DATE_FERMENTED_OUT"     , PropertyNames::BrewNote::fermentDate      },
         {XmlRecordDefinition::FieldType::String          , "NOTES"                  , PropertyNames::BrewNote::notes            },
         {XmlRecordDefinition::FieldType::Double          , "SG"                     , PropertyNames::BrewNote::sg               },
         {XmlRecordDefinition::FieldType::Double          , "ACTUAL_ABV"             , PropertyNames::BrewNote::abv              },
         {XmlRecordDefinition::FieldType::Double          , "EFF_INTO_BK"            , PropertyNames::BrewNote::effIntoBK_pct    },
         {XmlRecordDefinition::FieldType::Double          , "BREWHOUSE_EFF"          , PropertyNames::BrewNote::brewhouseEff_pct },
         {XmlRecordDefinition::FieldType::Double          , "VOLUME_INTO_BK"         , PropertyNames::BrewNote::volumeIntoBK_l   },
         {XmlRecordDefinition::FieldType::Double          , "STRIKE_TEMP"            , PropertyNames::BrewNote::strikeTemp_c     },
         {XmlRecordDefinition::FieldType::Double          , "MASH_FINAL_TEMP"        , PropertyNames::BrewNote::mashFinTemp_c    },
         {XmlRecordDefinition::FieldType::Double          , "OG"                     , PropertyNames::BrewNote::og               },
         {XmlRecordDefinition::FieldType::Double          , "POST_BOIL_VOLUME"       , PropertyNames::BrewNote::postBoilVolume_l },
         {XmlRecordDefinition::FieldType::Double          , "VOLUME_INTO_FERMENTER"  , PropertyNames::BrewNote::volumeIntoFerm_l },
         {XmlRecordDefinition::FieldType::Double          , "PITCH_TEMP"             , PropertyNames::BrewNote::pitchTemp_c      },
         {XmlRecordDefinition::FieldType::Double          , "FG"                     , PropertyNames::BrewNote::fg               },
         {XmlRecordDefinition::FieldType::Double          , "ATTENUATION"            , PropertyNames::BrewNote::attenuation      },
         {XmlRecordDefinition::FieldType::Double          , "FINAL_VOLUME"           , PropertyNames::BrewNote::finalVolume_l    },
         {XmlRecordDefinition::FieldType::Double          , "BOIL_OFF"               , PropertyNames::BrewNote::boilOff_l        },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_BOIL_GRAV"    , PropertyNames::BrewNote::projBoilGrav     },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_VOL_INTO_BK"  , PropertyNames::BrewNote::projVolIntoBK_l  },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_STRIKE_TEMP"  , PropertyNames::BrewNote::projStrikeTemp_c },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_MASH_FIN_TEMP", PropertyNames::BrewNote::projMashFinTemp_c},
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_OG"           , PropertyNames::BrewNote::projOg           },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_VOL_INTO_FERM", PropertyNames::BrewNote::projVolIntoFerm_l},
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_FG"           , PropertyNames::BrewNote::projFg           },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_EFF"          , PropertyNames::BrewNote::projEff_pct      },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_ABV"          , PropertyNames::BrewNote::projABV_pct      },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_POINTS"       , PropertyNames::BrewNote::projPoints       },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_FERM_POINTS"  , PropertyNames::BrewNote::projFermPoints   },
         {XmlRecordDefinition::FieldType::Double          , "PROJECTED_ATTEN"        , PropertyNames::BrewNote::projAtten        },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the fermentable part(!) of <FERMENTABLE>...</FERMENTABLE> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP below!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_FERMENTABLE_IN_RECIPE_ADDITION_FERMENTABLE {
      std::in_place_type_t<Fermentable>{},
      "FERMENTABLE",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord<Fermentable> >,
      {BeerXml_FermentableBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN<RecipeAdditionHop> below!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<RecipeAdditionFermentable> {
      std::in_place_type_t<RecipeAdditionFermentable>{},
      "FERMENTABLE",                          // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<RecipeAdditionFermentable>>,
      {
         // Type                                  XPath                Q_PROPERTY                                      Value Decoder
         {XmlRecordDefinition::FieldType::Double, "AMOUNT"           , PropertyNames::IngredientAmount::quantity     },
         {XmlRecordDefinition::FieldType::Double, "TIME"             , PropertyNames::RecipeAddition::addAtTime_mins },
         {XmlRecordDefinition::FieldType::Record, ""                 , PropertyNames::RecipeAdditionFermentable::fermentable, &BEER_XML_RECORD_DEFN_FERMENTABLE_IN_RECIPE_ADDITION_FERMENTABLE},
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Unit  , "UNIT"             , PropertyNames::IngredientAmount::unit         , &Measurement::Units::unitStringMapping},
         {XmlRecordDefinition::FieldType::Enum  , "STAGE"            , PropertyNames::RecipeAddition::stage          , &RecipeAddition::stageStringMapping},
         {XmlRecordDefinition::FieldType::Int   , "STEP"             , PropertyNames::RecipeAddition::step           },
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_GRAVITY_SG", PropertyNames::RecipeAddition::addAtGravity_sg},
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_ACIDITY_PH", PropertyNames::RecipeAddition::addAtAcidity_pH},
         {XmlRecordDefinition::FieldType::Double, "DURATION_MINS"    , PropertyNames::RecipeAddition::duration_mins  },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the hop part(!) of <HOP>...</HOP> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // In BeerXML, a <HOP>...</HOP> record inside a <RECIPE>...</RECIPE> records is a "hop addition".  We now model this
   // internally with the RecipeAdditionHop class.  Broadly speaking, that combines a "hop part" (ie what hop are we
   // adding) with amount and timing parts (ie how much are we adding and when).  In our internal model, the "hop part"
   // is just the ID of the hop we want to add.  In BeerXML, it's all the same fields as in a freestanding hop.  To
   // square this circle, we use the "Base Record" trick (descibed in serialization/json/JsonRecordDefinition.h, but
   // equally applicable to XML processing) to treat some of the HOP fields as though they were a separate sub-record.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP {
      std::in_place_type_t<Hop>{},
      "HOP",                    // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord< Hop > >,
      {BeerXml_HopBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <HOP>...</HOP> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // Note that the AMOUNT field here is quantity in kilograms (unless the extension field UNIT is present).  Because
   // Hop::defaultMeasure is Measurement::PhysicalQuantity::Mass and our canonical Unit for Mass is
   // Measurement::Units::kilograms, we don't have to do anything special to read in a record without the UNIT field.
   // Everything should "just work".
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<RecipeAdditionHop> {
      std::in_place_type_t<RecipeAdditionHop>{},
      "HOP",                          // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<RecipeAdditionHop>>,
      {
         // Type                                  XPath                Q_PROPERTY                                      Value Decoder
         {XmlRecordDefinition::FieldType::Double, "AMOUNT"           , PropertyNames::IngredientAmount::quantity     },
         {XmlRecordDefinition::FieldType::Double, "TIME"             , PropertyNames::RecipeAddition::addAtTime_mins },
         {XmlRecordDefinition::FieldType::Enum  , "USE"              , PropertyNames::RecipeAdditionHop::use         , &RecipeAdditionHop::useStringMapping},
         {XmlRecordDefinition::FieldType::Record, ""                 , PropertyNames::RecipeAdditionHop::hop         , &BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP},
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Unit  , "UNIT"             , PropertyNames::IngredientAmount::unit         , &Measurement::Units::unitStringMapping},
         {XmlRecordDefinition::FieldType::Enum  , "STAGE"            , PropertyNames::RecipeAddition::stage          , &RecipeAddition::stageStringMapping},
         {XmlRecordDefinition::FieldType::Int   , "STEP"             , PropertyNames::RecipeAddition::step           },
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_GRAVITY_SG", PropertyNames::RecipeAddition::addAtGravity_sg},
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_ACIDITY_PH", PropertyNames::RecipeAddition::addAtAcidity_pH},
         {XmlRecordDefinition::FieldType::Double, "DURATION_MINS"    , PropertyNames::RecipeAddition::duration_mins  },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the misc part(!) of <MISC>...</MISC> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_MISC_IN_RECIPE_ADDITION_MISC {
      std::in_place_type_t<Misc>{},
      "MISC",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord<Misc> >,
      {BeerXml_MiscBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MISC>...</MISC> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN<RecipeAdditionHop> above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<RecipeAdditionMisc> {
      std::in_place_type_t<RecipeAdditionMisc>{},
      "MISC",                          // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<RecipeAdditionMisc>>,
      {
         // Type                                  XPath                Q_PROPERTY                                      Value Decoder
         {XmlRecordDefinition::FieldType::Double, "AMOUNT"           , PropertyNames::IngredientAmount::quantity     },
         {XmlRecordDefinition::FieldType::Bool  , "AMOUNT_IS_WEIGHT" , PropertyNames::IngredientAmount::isWeight     },
         {XmlRecordDefinition::FieldType::Double, "TIME"             , PropertyNames::RecipeAddition::addAtTime_mins },
         {XmlRecordDefinition::FieldType::Record, ""                 , PropertyNames::RecipeAdditionMisc::misc       , &BEER_XML_RECORD_DEFN_MISC_IN_RECIPE_ADDITION_MISC},
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Unit  , "UNIT"             , PropertyNames::IngredientAmount::unit         , &Measurement::Units::unitStringMapping},
         {XmlRecordDefinition::FieldType::Enum  , "STAGE"            , PropertyNames::RecipeAddition::stage          , &RecipeAddition::stageStringMapping},
         {XmlRecordDefinition::FieldType::Int   , "STEP"             , PropertyNames::RecipeAddition::step           },
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_GRAVITY_SG", PropertyNames::RecipeAddition::addAtGravity_sg},
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_ACIDITY_PH", PropertyNames::RecipeAddition::addAtAcidity_pH},
         {XmlRecordDefinition::FieldType::Double, "DURATION_MINS"    , PropertyNames::RecipeAddition::duration_mins  },
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the fermentable part(!) of <YEAST>...</YEAST> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_YEAST_IN_RECIPE_ADDITION_YEAST {
      std::in_place_type_t<Yeast>{},
      "YEAST",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord<Yeast> >,
      {BeerXml_YeastBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <YEAST>...</YEAST> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN<RecipeAdditionHop> above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<RecipeAdditionYeast> {
      std::in_place_type_t<RecipeAdditionYeast>{},
      "YEAST",                          // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<RecipeAdditionYeast>>,
      {
         // Type                                  XPath                  Q_PROPERTY                                           Value Decoder
         {XmlRecordDefinition::FieldType::Double, "AMOUNT"             , PropertyNames::IngredientAmount::quantity            },
         {XmlRecordDefinition::FieldType::Bool  , "AMOUNT_IS_WEIGHT"   , PropertyNames::IngredientAmount::isWeight            },
         {XmlRecordDefinition::FieldType::Record, ""                   , PropertyNames::RecipeAdditionYeast::yeast            , &BEER_XML_RECORD_DEFN_YEAST_IN_RECIPE_ADDITION_YEAST},
         {XmlRecordDefinition::FieldType::Double, "ATTENUATION"        , PropertyNames::RecipeAdditionYeast::attenuation_pct  }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Int   , "TIMES_CULTURED"     , PropertyNames::RecipeAdditionYeast::timesCultured    }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Int   , "CELL_COUNT_BILLIONS", PropertyNames::RecipeAdditionYeast::cellCountBillions}, // Non-standard: not part of BeerXML 1.0
         {XmlRecordDefinition::FieldType::Bool  , "ADD_TO_SECONDARY"   , PropertyNames::RecipeAdditionYeast::addToSecondary   }, // ⮜⮜⮜ Optional in BeerXML ⮞⮞⮞
         // ⮜⮜⮜ Following are new fields that BeerJSON adds to BeerXML, so all extension tags in BeerXML ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Double, "TIME"               , PropertyNames::RecipeAddition::addAtTime_mins        },
         {XmlRecordDefinition::FieldType::Unit  , "UNIT"               , PropertyNames::IngredientAmount::unit              , &Measurement::Units::unitStringMapping},
         {XmlRecordDefinition::FieldType::Enum  , "STAGE"              , PropertyNames::RecipeAddition::stage               , &RecipeAddition::stageStringMapping},
         {XmlRecordDefinition::FieldType::Int   , "STEP"               , PropertyNames::RecipeAddition::step                },
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_GRAVITY_SG"  , PropertyNames::RecipeAddition::addAtGravity_sg     },
         {XmlRecordDefinition::FieldType::Double, "ADD_AT_ACIDITY_PH"  , PropertyNames::RecipeAddition::addAtAcidity_pH     },
         {XmlRecordDefinition::FieldType::Double, "DURATION_MINS"      , PropertyNames::RecipeAddition::duration_mins       },
      }
   };


   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for the water part(!) of <WATER>...</WATER> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN_HOP_IN_RECIPE_ADDITION_HOP above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_WATER_IN_RECIPE_USE_OF_WATER {
      std::in_place_type_t<Water>{},
      "Water",            // XML record name
      XmlRecordDefinition::create< XmlNamedEntityRecord<Water> >,
      {BeerXml_WaterBase}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <WATER>...</WATER> BeerXML records inside <RECIPE>...</RECIPE> records
   //
   // See comment on BEER_XML_RECORD_DEFN<RecipeAdditionHop> above!
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<RecipeUseOfWater> {
      std::in_place_type_t<RecipeUseOfWater>{},
      "WATER",                          // XML record name
      XmlRecordDefinition::create<XmlNamedEntityRecord<RecipeUseOfWater>>,
      {
         // Type                                  XPath     Q_PROPERTY                                 Value Decoder
         {XmlRecordDefinition::FieldType::Double, "AMOUNT", PropertyNames::RecipeUseOfWater::volume_l},
         {XmlRecordDefinition::FieldType::Record, ""      , PropertyNames::RecipeUseOfWater::water   , &BEER_XML_RECORD_DEFN_WATER_IN_RECIPE_USE_OF_WATER},
      }
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <RECIPE>...</RECIPE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   EnumStringMapping const BEER_XML_RECIPE_STEP_TYPE_MAPPER {
      {Recipe::Type::Extract    , "Extract"                   },
      {Recipe::Type::PartialMash, "Partial Mash"              },
      {Recipe::Type::AllGrain   , "All Grain"                 },
      // These other types are in BeerJSON but are not mentioned in the BeerXML 1.0 Standard.  They get an (extremely)
      // approximate mapping when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings for the same strings
      {Recipe::Type::Cider      , "All Grain<!-- Cider -->"   },
      {Recipe::Type::Kombucha   , "All Grain<!-- Kombucha -->"},
      {Recipe::Type::Soda       , "All Grain<!-- Soda -->"    },
      {Recipe::Type::Other      , "All Grain<!-- Other -->"   },
      {Recipe::Type::Mead       , "All Grain<!-- Mead -->"    },
      {Recipe::Type::Wine       , "All Grain<!-- Wine -->"    },
   };
   template<> XmlRecordDefinition const BEER_XML_RECORD_DEFN<Recipe> {
      std::in_place_type_t<Recipe>{},
      "RECIPE",            // XML record name
      //      XmlRecordDefinition::create<XmlNamedEntityRecord<Recipe>>,
      XmlRecordDefinition::create<XmlRecipeRecord>,
      {
         // Type                                            XPath                       Q_PROPERTY                                 Value Decoder
         {XmlRecordDefinition::FieldType::String          , "NAME"                    , PropertyNames::NamedEntity::name         },
         {XmlRecordDefinition::FieldType::RequiredConstant, "VERSION"                 , VERSION1                                 },
         {XmlRecordDefinition::FieldType::Enum            , "TYPE"                    , PropertyNames::Recipe::type              , &BEER_XML_RECIPE_STEP_TYPE_MAPPER},
         {XmlRecordDefinition::FieldType::Record          , "STYLE"                   , PropertyNames::Recipe::style             , &BEER_XML_RECORD_DEFN<Style>},
         {XmlRecordDefinition::FieldType::Record          , "EQUIPMENT"               , PropertyNames::Recipe::equipment         , &BEER_XML_RECORD_DEFN<Equipment>},
         {XmlRecordDefinition::FieldType::String          , "BREWER"                  , PropertyNames::Recipe::brewer            },
         {XmlRecordDefinition::FieldType::String          , "ASST_BREWER"             , PropertyNames::Recipe::asstBrewer        },
         {XmlRecordDefinition::FieldType::Double          , "BATCH_SIZE"              , PropertyNames::Recipe::batchSize_l       },
         {XmlRecordDefinition::FieldType::Double          , "BOIL_SIZE"               , {PropertyNames::Recipe::boil,
                                                                                         PropertyNames::Boil::preBoilSize_l}     },
         {XmlRecordDefinition::FieldType::Double          , "BOIL_TIME"               , {PropertyNames::Recipe::boil,
                                                                                         PropertyNames::Boil::boilTime_mins}     },
         {XmlRecordDefinition::FieldType::Double          , "EFFICIENCY"              , PropertyNames::Recipe::efficiency_pct    },
         {XmlRecordDefinition::FieldType::ListOfRecords   , "HOPS/HOP"                , PropertyNames::Recipe::hopAdditions      , &BEER_XML_RECORD_DEFN<RecipeAdditionHop>  }, // Additional logic for "HOPS" is handled in xml/XmlRecipeRecord.cpp
         {XmlRecordDefinition::FieldType::ListOfRecords   , "FERMENTABLES/FERMENTABLE", PropertyNames::Recipe::fermentableAdditions, &BEER_XML_RECORD_DEFN<RecipeAdditionFermentable>}, // Additional logic for "FERMENTABLES" is handled in xml/XmlRecipeRecord.cpp
         {XmlRecordDefinition::FieldType::ListOfRecords   , "MISCS/MISC"              , PropertyNames::Recipe::miscAdditions     , &BEER_XML_RECORD_DEFN<RecipeAdditionMisc> }, // Additional logic for "MISCS" is handled in xml/XmlRecipeRecord.cpp
         {XmlRecordDefinition::FieldType::ListOfRecords   , "YEASTS/YEAST"            , PropertyNames::Recipe::yeastAdditions    , &BEER_XML_RECORD_DEFN<RecipeAdditionYeast>}, // Additional logic for "YEASTS" is handled in xml/XmlRecipeRecord.cpp
         {XmlRecordDefinition::FieldType::ListOfRecords   , "WATERS/WATER"            , PropertyNames::Recipe::waterUses         , &BEER_XML_RECORD_DEFN<RecipeUseOfWater>   }, // Additional logic for "WATERS" is handled in xml/XmlRecipeRecord.cpp
         {XmlRecordDefinition::FieldType::Record          , "MASH"                    , PropertyNames::Recipe::mash              , &BEER_XML_RECORD_DEFN<Mash>             },
         {XmlRecordDefinition::FieldType::ListOfRecords   , "INSTRUCTIONS/INSTRUCTION", PropertyNames::Recipe::instructions      , &BEER_XML_RECORD_DEFN<Instruction>      }, // Additional logic for "INSTRUCTIONS" is handled in xml/XmlNamedEntityRecord.h
         {XmlRecordDefinition::FieldType::ListOfRecords   , "BREWNOTES/BREWNOTE"      , PropertyNames::Recipe::brewNotes         , &BEER_XML_RECORD_DEFN<BrewNote>         }, // Additional logic for "BREWNOTES" is handled in xml/XmlNamedEntityRecord.h
         {XmlRecordDefinition::FieldType::String          , "NOTES"                   , PropertyNames::Recipe::notes             },
         {XmlRecordDefinition::FieldType::String          , "TASTE_NOTES"             , PropertyNames::Recipe::tasteNotes        },
         {XmlRecordDefinition::FieldType::Double          , "TASTE_RATING"            , PropertyNames::Recipe::tasteRating       },
         {XmlRecordDefinition::FieldType::Double          , "OG"                      , PropertyNames::Recipe::og                },
         {XmlRecordDefinition::FieldType::Double          , "FG"                      , PropertyNames::Recipe::fg                },
         {XmlRecordDefinition::FieldType::UInt            , "FERMENTATION_STAGES"     , {PropertyNames::Recipe::fermentation,
                                                                                         PropertyNames::StepOwnerBase::numSteps}}, // We write but ignore on read if present.
         {XmlRecordDefinition::FieldType::Double          , "PRIMARY_AGE"             , {PropertyNames::Recipe::fermentation ,
                                                                                         PropertyNames::Fermentation::primary,
                                                                                         PropertyNames::StepBase::stepTime_days} }, // Replaces PropertyNames::Recipe::primaryAge_days
         {XmlRecordDefinition::FieldType::Double          , "PRIMARY_TEMP"            , {PropertyNames::Recipe::fermentation ,
                                                                                         PropertyNames::Fermentation::primary,
                                                                                         PropertyNames::StepBase::startTemp_c}   }, // Replaces PropertyNames::Recipe::primaryTemp_c
         {XmlRecordDefinition::FieldType::Double          , "SECONDARY_AGE"           , {PropertyNames::Recipe::fermentation   ,
                                                                                         PropertyNames::Fermentation::secondary,
                                                                                         PropertyNames::StepBase::stepTime_days} }, // Replaces PropertyNames::Recipe::secondaryAge_days
         {XmlRecordDefinition::FieldType::Double          , "SECONDARY_TEMP"          , {PropertyNames::Recipe::fermentation   ,
                                                                                         PropertyNames::Fermentation::secondary,
                                                                                         PropertyNames::StepBase::startTemp_c  } }, // Replaces PropertyNames::Recipe::secondaryTemp_c
         {XmlRecordDefinition::FieldType::Double          , "TERTIARY_AGE"            , {PropertyNames::Recipe::fermentation  ,
                                                                                         PropertyNames::Fermentation::tertiary,
                                                                                         PropertyNames::StepBase::stepTime_days} }, // Replaces PropertyNames::Recipe::tertiaryAge_days
         {XmlRecordDefinition::FieldType::Double          , "TERTIARY_TEMP"           , {PropertyNames::Recipe::fermentation  ,
                                                                                         PropertyNames::Fermentation::tertiary,
                                                                                         PropertyNames::StepBase::startTemp_c }  }, // Replaces PropertyNames::Recipe::tertiaryTemp_c
         {XmlRecordDefinition::FieldType::Double          , "AGE"                     , PropertyNames::Recipe::age_days          },
         {XmlRecordDefinition::FieldType::Double          , "AGE_TEMP"                , PropertyNames::Recipe::ageTemp_c         },
         {XmlRecordDefinition::FieldType::Date            , "DATE"                    , PropertyNames::Recipe::date              },
         {XmlRecordDefinition::FieldType::Double          , "CARBONATION"             , PropertyNames::Recipe::carbonation_vols  },
         {XmlRecordDefinition::FieldType::Bool            , "FORCED_CARBONATION"      , PropertyNames::Recipe::forcedCarbonation },
         {XmlRecordDefinition::FieldType::String          , "PRIMING_SUGAR_NAME"      , PropertyNames::Recipe::primingSugarName  },
         {XmlRecordDefinition::FieldType::Double          , "CARBONATION_TEMP"        , PropertyNames::Recipe::carbonationTemp_c },
         {XmlRecordDefinition::FieldType::Double          , "PRIMING_SUGAR_EQUIV"     , PropertyNames::Recipe::primingSugarEquiv },
         {XmlRecordDefinition::FieldType::Double          , "KEG_PRIMING_FACTOR"      , PropertyNames::Recipe::kegPrimingFactor  },
         {XmlRecordDefinition::FieldType::String          , "EST_OG"                  , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "EST_FG"                  , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "EST_COLOR"               , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "IBU"                     , PropertyNames::Recipe::IBU               }, // Extension tag.  We write but ignore on read if present.
         {XmlRecordDefinition::FieldType::String          , "IBU_METHOD"              , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "EST_ABV"                 , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "ABV"                     , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "ACTUAL_EFFICIENCY"       , BtString::NULL_STR                       }, // Extension tag
         // We don't write the calories field in BeerXML as it's a free-form text field that would require special processing to
         // output.  In BeerJSON the equivalent field is more structured, so we do write it.
         {XmlRecordDefinition::FieldType::String          , "CALORIES"                , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_BATCH_SIZE"      , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_BOIL_SIZE"       , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_OG"              , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_FG"              , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_PRIMARY_TEMP"    , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_SECONDARY_TEMP"  , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_TERTIARY_TEMP"   , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_AGE_TEMP"        , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "CARBONATION_USED"        , BtString::NULL_STR                       }, // Extension tag
         {XmlRecordDefinition::FieldType::String          , "DISPLAY_CARB_TEMP"       , BtString::NULL_STR                       }, // Extension tag
         // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
         {XmlRecordDefinition::FieldType::Double          , "BEER_ACIDITY_PH"         , PropertyNames::Recipe::beerAcidity_pH         }, // Non-standard: not part of BeerXML 1.0
         {XmlRecordDefinition::FieldType::Double          , "APPARENT_ATTENUATION_PCT", PropertyNames::Recipe::apparentAttenuation_pct}, // Non-standard: not part of BeerXML 1.0

      }
   };

   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for root of BeerXML document
   /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecordDefinition const BEER_XML_RECORD_DEFN_ROOT {
      "BEER_XML", // XML record name
      nullptr,    // Type Lookup for our corresponding model object
      "",         // NamedEntity class name
      "",         // Localised name
      {},         // upAndDownCasters
      XmlRecordDefinition::create<XmlRecord>,
      {
         // Type                                         XPath                       Q_PROPERTY           Value Decoder
         {XmlRecordDefinition::FieldType::ListOfRecords, "HOPS/HOP"                , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Hop        >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "FERMENTABLES/FERMENTABLE", BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Fermentable>},
         {XmlRecordDefinition::FieldType::ListOfRecords, "YEASTS/YEAST"            , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Yeast      >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "MISCS/MISC"              , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Misc       >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "WATERS/WATER"            , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Water      >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "STYLES/STYLE"            , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Style      >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "MASH_STEPS/MASH_STEP"    , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<MashStep   >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "MASHS/MASH"              , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Mash       >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "EQUIPMENTS/EQUIPMENT"    , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Equipment  >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "INSTRUCTIONS/INSTRUCTION", BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Instruction>},
         {XmlRecordDefinition::FieldType::ListOfRecords, "BREWNOTES/BREWNOTE"      , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<BrewNote   >},
         {XmlRecordDefinition::FieldType::ListOfRecords, "RECIPES/RECIPE"          , BtString::NULL_STR  , &BEER_XML_RECORD_DEFN<Recipe     >},
      }
   };

   //
   // The mapping we use between BeerXML structure and our own object structure
   //
   XmlCoding const BEER_XML_1_CODING{
      "BeerXML 1.0",
      ":/schemas/beerxml/v1/BeerXml.xsd",
      BEER_XML_RECORD_DEFN_ROOT
   };

   /**
    * \brief Validate XML file against schema and load its contents
    *
    * \param fileName Fully-qualified name of the file to validate
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validateAndLoad(QString const & fileName, QTextStream & userMessage) {

      QFile inputFile;
      inputFile.setFileName(fileName);

      if(!inputFile.open(QIODevice::ReadOnly)) {
         qWarning() << Q_FUNC_INFO << ": Could not open " << fileName << " for reading";
         return false;
      }

      //
      // Rather than just read the XML file into memory, we actually make a small on-the-fly modification to it to
      // place all the top-level content inside a <BEER_XML>...</BEER_XML> field.  This massively simplifies the XSD
      // (as explained in a comment therein) at the cost of some minor complexity here.  Essentially, the added tag
      // pair is (much as we might have wished it were part of the original BeerXML 1.0 Specification to make BeerXML
      // actually valid XML †) something we need to hide from the user to avoid confusion (as the tag does not and is
      // not supposed to exist in the document they are asking us to process).
      //
      // † The BeerXML 1.0 standard diverges from valid/standard XML in a few ways:
      //    • It mandates an XML Declaration (which it calls the "XML Header"), which is normally an optional part of
      //      any UTF-8 encoded XML document.  (This is perhaps because it seems to mandate an ISO-8859-1 aka "Latin 1"
      //      coding of BeerXML files, though there is no explicit discussion of file encodings in the standard, and
      //      this seems an unnecessary constraint.)
      //    • It omits to specify a single root element, even though this is a required part of any valid XML document.
      //    • It uses "TRUE" and "FALSE" (ie caps) for boolean values instead of the XML standard "true" and "false"
      //      (ie lower case).
      //
      // Fortunately the edit is simple:
      //  - We retain unchanged the first line of the file, which should, for valid BeerXML, be something along the
      //    lines of "<?xml version="1.0" blah blah ?>"
      //    (Of course, we also check that the first line is what we expect!)
      //  - We insert a new line 2 that says "<BEER_XML>"
      //  - We read in the rest of the file unchanged (so what was line 2 on disk will be line 3 in memory and so on)
      //  - We append a new final line that says "</BEER_XML>"
      //
      // We then give enough information to our instance of BtDomErrorHandler to allow it to correct the line numbers
      // for any errors it needs to log.  (And we get a bit of help from this class when we need to make similar
      // adjustments during exception processing.)
      //
      // Note here that we are assuming the on-disk format of the file is single-byte (UTF-8 or ASCII or ISO-8859-1).
      // This is a reasonably safe assumption but, in theory, we could examine the first line to verify it.
      //
      // We further assume the first line of the file can be treated as ISO-8859-1 aka "Latin 1", but, again, this
      // should be safe as it's just one or two tags, not user content.
      //
      // We _could_ make "BEER_XML" some sort of constant eg:
      //    constexpr static char const * const INSERTED_ROOT_NODE_NAME = "BEER_XML";
      // but we wouldn't be able to use that constant in beerxml/v1/BeerXml.xsd, and using it in the few C++ places we
      // need it (this file and BeerXmlRootRecord.cpp) would be cumbersome, making the code more difficult to follow.
      // Since we're unlikely ever to need to change (or make much more widespread use of) this tag, we've gone with
      // readability over purity, and left it hard-coded, for now at least.
      //
      QByteArray documentData = inputFile.readLine();
      QString firstLine{QString::fromLatin1(documentData)};
      qDebug() << Q_FUNC_INFO << "First line of " << inputFile.fileName() << " was " << firstLine;
      if (!firstLine.startsWith(QString("<?xml version="))) {
         //
         // For the moment, we're being strict and bailing out here.  An alternative approach would be to accept files
         // missing the XML declaration (which is, after all, optional in most types of XML file)
         //
         qCritical() <<
            Q_FUNC_INFO << "Unexpected first line of file (should begin with '<?xml version=' but doesn't): " <<
            firstLine;
         userMessage << "Unexpected first line (not the XML declaration mandated by BeerXML).";
         return false;
      }
      //
      // Some software, such as the Grainfather online recipe editor at https://community.grainfather.com/, omits to put
      // a linebreak after the initial tag, so it will export a first line such as:
      //    <?xml version="1.0" encoding="UTF-8" ?><RECIPES>
      // So, rather than just append our root tag after line 1, we find the end of the first tag and insert it there.
      // It's easier to do the insertions in QString and then overwrite the raw data we read in for the first line.
      //
      auto const tagEnd = firstLine.indexOf(QChar{'>'});
      firstLine.insert(tagEnd + 1, "\n<BEER_XML>");
      documentData = firstLine.toLatin1();
      documentData += inputFile.readAll();
      documentData += "\n</BEER_XML>";
      qDebug() << Q_FUNC_INFO << "Input file " << inputFile.fileName() << ": " << documentData.length() << " bytes";

      // It is sometimes helpful to uncomment the next line for debugging, but usually leave it commented out as can
      // put a _lot_ of data in the logs in DEBUG mode.
      // qDebug().noquote() << Q_FUNC_INFO << "Full content of " << inputFile.fileName() << " is:\n" << QString(documentData);

      //
      // Some errors we explicitly want to ignore.  In particular, the BeerXML 1.0 standard says:
      //
      //    "Non-Standard Tags
      //    "Per the XML standard, all non-standard tags will be ignored by the importing program.  This allows programs
      //    to store additional information if desired using their own tags.  Any tags not defined as part of this
      //    standard may safely be ignored by the importing program."
      //
      // There are two problems with this.  One is that it does not prevent two different programs creating
      // identically-named custom tags with different meanings.  (And note that it is observably NOT the case that
      // existing implementations take any care to make their custom tag names unique to the program using them.)
      //
      // The second problem is that, because the BeerXML 1.0 standard also says that tags inside a containing element
      // may occur in any order, we cannot easily tell the XSD to ignore unkonwn tags.  (The issue is that, in the XSD,
      // we have to to use <xs:all> rather than <xs:sequence> for the containing tags, as this allows the contained
      // tags to appear in any order.  In turn, this means we cannot use <xs:any> to allow unrecognised tags.  This is
      // disallowed by the W3C XML Schema standard because it would make validation harder (and slower).  See
      // https://stackoverflow.com/questions/3347822/validating-xml-with-xsds-but-still-allow-extensibility for a good
      // explanation.)
      //
      // So, our workaround for this is to ignore errors that say:
      //   • "no declaration found for element 'ABC'"
      //   • "element 'ABC' is not allowed for content model 'XYZ'.
      //
      static QVector<BtDomErrorHandler::PatternAndReason> const errorPatternsToIgnore {
         //       Reg-ex to match                                               Reason to ignore errors matching this pattern
         {QString("^no declaration found for element"),                 QString("we are assuming unrecognised tags are just non-standard tags in the BeerXML")},
         {QString("^element '[^']*' is not allowed for content model"), QString("we are assuming unrecognised tags are just non-standard tags in the BeerXML")}
      };
      BtDomErrorHandler domErrorHandler(&errorPatternsToIgnore, 1, 1);

      return BEER_XML_1_CODING.validateLoadAndStoreInDb(documentData, fileName, domErrorHandler, userMessage);

   }

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


BeerXML & BeerXML::getInstance() {
   //
   // As of C++11, simple "Meyers singleton" is now thread-safe -- see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime
   //
   static BeerXML singleton;

   return singleton;
}


BeerXML::BeerXML() {
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
BeerXML::~BeerXML() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BeerXML::createXmlFile(QFile & outFile) const {
   QTextStream out(&outFile);
   // BeerXML specifies the ISO-8859-1 (Latin 1) encoding
   out.setEncoding(QStringConverter::Latin1);

   out <<
      "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
      "<!-- BeerXML Format generated by " << CONFIG_APPLICATION_NAME_UC << " " << CONFIG_VERSION_STRING << " on " <<
      QDateTime::currentDateTime().date().toString(Qt::ISODate) << " -->\n";

   return;
}

template<class NE> void BeerXML::toXml(QList<NE const *> const & nes, QFile & outFile) const {
   // We don't want to output empty container records
   if (nes.empty()) {
      return;
   }

   // It is a feature of BeerXML that the tag name for a list of elements is just the tag name for an individual
   // element with an S on the end, even when this is not grammatically correct.  Thus a list of <HOP>...</HOP> records
   // is contained inside <HOPS>...</HOPS> tags, a list of <MISC>...</MISC> records is contained inside
   // <MISCS>...</MISCS> tags and so on.
   QTextStream out(&outFile);
   // BeerXML specifies the ISO-8859-1 encoding
   out.setEncoding(QStringConverter::Latin1);
   out << "<" << BEER_XML_RECORD_DEFN<NE>.m_recordName << "S>\n";
   for (NE const * ne : nes) {
      Q_ASSERT(ne);
      std::unique_ptr<XmlRecord> xmlRecord{
         BEER_XML_RECORD_DEFN<NE>.makeRecord(BEER_XML_1_CODING)
      };
      xmlRecord->toXml(*ne, out, true);
   }
   out << "</" << BEER_XML_RECORD_DEFN<NE>.m_recordName << "S>\n";
   return;
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template void BeerXML::toXml(QList<Hop         const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Fermentable const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Yeast       const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Misc        const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Water       const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Style       const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<MashStep    const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Mash        const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Equipment   const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Instruction const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<BrewNote    const *> const & nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Recipe      const *> const & nes, QFile & outFile) const;

// fromXml ====================================================================
bool BeerXML::importFromXML(QString const & filename, QTextStream & userMessage) {
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
