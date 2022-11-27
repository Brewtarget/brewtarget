/*
 * BeerXml.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "xml/BeerXml.h"

#include <stdexcept>

#include <QApplication>
#include <QDebug>
#include <QDomNodeList>
#include <QFile>
#include <QHash>
#include <QList>
#include <QTextCodec>
#include <QTextStream>

#include "config.h" // For CONFIG_VERSION_STRING
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "xml/BtDomErrorHandler.h"
#include "xml/MibEnum.h"
#include "xml/XmlCoding.h"
#include "xml/XmlRecord.h"

//
// Variables and constant definitions that we need only in this file
//
namespace {
   // See comment in XmlRecord.cpp about how we slightly abuse propertyName field of FieldDefinition when fieldType is
   // XmlRecord::RequiredConstant.  (This is when a required XML field holds data we don't need (and for which we always
   // write a constant value on output.)  Specifically, in BeerXML, we need to write every version of pretty much every
   // record as "1".
   BtStringConst const VERSION1{"1"};

   template<class NE> QString BEER_XML_RECORD_NAME;
   template<class NE> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS;

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Top-level field mappings for BeerXML files
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<void>{"BEER_XML"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<void> {
      // Type                               XPath                       Q_PROPERTY              Enum Mapper
      {XmlRecord::FieldType::RecordComplex, "HOPS/HOP",                 BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "FERMENTABLES/FERMENTABLE", BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "YEASTS/YEAST",             BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "MISCS/MISC",               BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "WATERS/WATER",             BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "STYLES/STYLE",             BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "MASHS/MASH",               BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "RECIPES/RECIPE",           BtString::NULL_STR,     nullptr},
      {XmlRecord::FieldType::RecordComplex, "EQUIPMENTS/EQUIPMENT",     BtString::NULL_STR,     nullptr},
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <HOP>...</HOP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Hop>{"HOP"};
   EnumStringMapping const BEER_XML_HOP_USE_MAPPER {
      {"Boil",       Hop::Use::Boil},
      {"Dry Hop",    Hop::Use::Dry_Hop},
      {"Mash",       Hop::Use::Mash},
      {"First Wort", Hop::Use::First_Wort},
      {"Aroma",      Hop::Use::UseAroma}
   };
   EnumStringMapping const BEER_XML_HOP_TYPE_MAPPER {
      {"Bittering",                          Hop::Type::Bittering},
      {"Aroma",                              Hop::Type::Aroma},
      {"Both",      Hop::Type::Both}
   };
   EnumStringMapping const BEER_XML_HOP_FORM_MAPPER {
      {"Pellet", Hop::Form::Pellet},
      {"Plug",   Hop::Form::Plug},
      {"Leaf",   Hop::Form::Leaf}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Hop> {
      // Type                                  XPath                    Q_PROPERTY                             Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",                  PropertyNames::NamedEntity::name,      nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",               VERSION1,                              nullptr},
      {XmlRecord::FieldType::Double,           "ALPHA",                 PropertyNames::Hop::alpha_pct,         nullptr},
      {XmlRecord::FieldType::Double,           "AMOUNT",                PropertyNames::Hop::amount_kg,         nullptr},
      {XmlRecord::FieldType::Enum,             "USE",            PropertyNames::Hop::use,               &BEER_XML_HOP_USE_MAPPER},
      {XmlRecord::FieldType::Double,           "TIME",                  PropertyNames::Hop::time_min,          nullptr},
      {XmlRecord::FieldType::String,           "NOTES",                 PropertyNames::Hop::notes,             nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",                  PropertyNames::Hop::type,              &BEER_XML_HOP_TYPE_MAPPER},
      {XmlRecord::FieldType::Enum,             "FORM",                  PropertyNames::Hop::form,              &BEER_XML_HOP_FORM_MAPPER},
      {XmlRecord::FieldType::Double,           "BETA",                  PropertyNames::Hop::beta_pct,          nullptr},
      {XmlRecord::FieldType::Double,           "HSI",                   PropertyNames::Hop::hsi_pct,           nullptr},
      {XmlRecord::FieldType::String,           "ORIGIN",                PropertyNames::Hop::origin,            nullptr},
      {XmlRecord::FieldType::String,           "SUBSTITUTES",           PropertyNames::Hop::substitutes,       nullptr},
      {XmlRecord::FieldType::Double,           "HUMULENE",              PropertyNames::Hop::humulene_pct,      nullptr},
      {XmlRecord::FieldType::Double,           "CARYOPHYLLENE",         PropertyNames::Hop::caryophyllene_pct, nullptr},
      {XmlRecord::FieldType::Double,           "COHUMULONE",            PropertyNames::Hop::cohumulone_pct,    nullptr},
      {XmlRecord::FieldType::Double,           "MYRCENE",               PropertyNames::Hop::myrcene_pct,       nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_AMOUNT",        BtString::NULL_STR,                    nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "INVENTORY",             BtString::NULL_STR,                    nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TIME",   BtString::NULL_STR,                    nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Fermentable>{"FERMENTABLE"};
   EnumStringMapping const BEER_XML_FERMENTABLE_TYPE_MAPPER {
      {"Grain",       Fermentable::Type::Grain},
      {"Sugar",       Fermentable::Type::Sugar},
      {"Extract",     Fermentable::Type::Extract},
      {"Dry Extract", Fermentable::Type::Dry_Extract},
      {"Adjunct",     Fermentable::Type::Adjunct}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Fermentable> {
      // Type                                  XPath               Q_PROPERTY                                          Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",             PropertyNames::NamedEntity::name,                   nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",          VERSION1,                                           nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",             PropertyNames::Fermentable::type,                   &BEER_XML_FERMENTABLE_TYPE_MAPPER},
      {XmlRecord::FieldType::Double,           "AMOUNT",           PropertyNames::Fermentable::amount_kg,              nullptr},
      {XmlRecord::FieldType::Double,           "YIELD",            PropertyNames::Fermentable::yield_pct,              nullptr},
      {XmlRecord::FieldType::Double,           "COLOR",            PropertyNames::Fermentable::color_srm,              nullptr},
      {XmlRecord::FieldType::Bool,             "ADD_AFTER_BOIL",   PropertyNames::Fermentable::addAfterBoil,           nullptr},
      {XmlRecord::FieldType::String,           "ORIGIN",           PropertyNames::Fermentable::origin,                 nullptr},
      {XmlRecord::FieldType::String,           "SUPPLIER",         PropertyNames::Fermentable::supplier,               nullptr},
      {XmlRecord::FieldType::String,           "NOTES",            PropertyNames::Fermentable::notes,                  nullptr},
      {XmlRecord::FieldType::Double,           "COARSE_FINE_DIFF", PropertyNames::Fermentable::coarseFineDiff_pct,     nullptr},
      {XmlRecord::FieldType::Double,           "MOISTURE",         PropertyNames::Fermentable::moisture_pct,           nullptr},
      {XmlRecord::FieldType::Double,           "DIASTATIC_POWER",  PropertyNames::Fermentable::diastaticPower_lintner, nullptr},
      {XmlRecord::FieldType::Double,           "PROTEIN",          PropertyNames::Fermentable::protein_pct,            nullptr},
      {XmlRecord::FieldType::Double,           "MAX_IN_BATCH",     PropertyNames::Fermentable::maxInBatch_pct,         nullptr},
      {XmlRecord::FieldType::Bool,             "RECOMMEND_MASH",   PropertyNames::Fermentable::recommendMash,          nullptr},
      {XmlRecord::FieldType::Double,           "IBU_GAL_PER_LB",   PropertyNames::Fermentable::ibuGalPerLb,            nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_AMOUNT",   BtString::NULL_STR,                                 nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "POTENTIAL",        BtString::NULL_STR,                                 nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "INVENTORY",        BtString::NULL_STR,                                 nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_COLOR",    BtString::NULL_STR,                                 nullptr}, // Extension tag
      {XmlRecord::FieldType::Bool,             "IS_MASHED",        PropertyNames::Fermentable::isMashed,               nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <YEAST>...</YEAST> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Yeast>{"YEAST"};
   EnumStringMapping const BEER_XML_YEAST_TYPE_MAPPER {
      {"Ale",       Yeast::Type::Ale},
      {"Lager",     Yeast::Type::Lager},
      {"Wheat",     Yeast::Type::Wheat},
      {"Wine",      Yeast::Type::Wine},
      {"Champagne", Yeast::Type::Champagne}
   };
   EnumStringMapping const BEER_XML_YEAST_FORM_MAPPER {
      {"Liquid",  Yeast::Form::Liquid},
      {"Dry",     Yeast::Form::Dry},
      {"Slant",   Yeast::Form::Slant},
      {"Culture", Yeast::Form::Culture}
   };
   EnumStringMapping const BEER_XML_YEAST_FLOCCULATION_MAPPER {
      {"Low",       Yeast::Flocculation::Low},
      {"Medium",    Yeast::Flocculation::Medium},
      {"High",      Yeast::Flocculation::High},
      {"Very High", Yeast::Flocculation::Very_High}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Yeast> {
      // Type                                  XPath               Q_PROPERTY                              Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",             PropertyNames::NamedEntity::name,       nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",          VERSION1,                               nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",             PropertyNames::Yeast::type,             &BEER_XML_YEAST_TYPE_MAPPER},
      {XmlRecord::FieldType::Enum,             "FORM",             PropertyNames::Yeast::form,             &BEER_XML_YEAST_FORM_MAPPER},
      {XmlRecord::FieldType::Double,           "AMOUNT",           PropertyNames::Yeast::amount,           nullptr},
      {XmlRecord::FieldType::Bool,             "AMOUNT_IS_WEIGHT", PropertyNames::Yeast::amountIsWeight,   nullptr},
      {XmlRecord::FieldType::String,           "LABORATORY",       PropertyNames::Yeast::laboratory,       nullptr},
      {XmlRecord::FieldType::String,           "PRODUCT_ID",       PropertyNames::Yeast::productID,        nullptr},
      {XmlRecord::FieldType::Double,           "MIN_TEMPERATURE",  PropertyNames::Yeast::minTemperature_c, nullptr},
      {XmlRecord::FieldType::Double,           "MAX_TEMPERATURE",  PropertyNames::Yeast::maxTemperature_c, nullptr},
      {XmlRecord::FieldType::Enum,             "FLOCCULATION",     PropertyNames::Yeast::flocculation,     &BEER_XML_YEAST_FLOCCULATION_MAPPER},
      {XmlRecord::FieldType::Double,           "ATTENUATION",      PropertyNames::Yeast::attenuation_pct,  nullptr},
      {XmlRecord::FieldType::String,           "NOTES",            PropertyNames::Yeast::notes,            nullptr},
      {XmlRecord::FieldType::String,           "BEST_FOR",         PropertyNames::Yeast::bestFor,          nullptr},
      {XmlRecord::FieldType::Int,              "TIMES_CULTURED",   PropertyNames::Yeast::timesCultured,    nullptr},
      {XmlRecord::FieldType::Int,              "MAX_REUSE",        PropertyNames::Yeast::maxReuse,         nullptr},
      {XmlRecord::FieldType::Bool,             "ADD_TO_SECONDARY", PropertyNames::Yeast::addToSecondary,   nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_AMOUNT",   BtString::NULL_STR,                     nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISP_MIN_TEMP",    BtString::NULL_STR,                     nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISP_MAX_TEMP",    BtString::NULL_STR,                     nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "INVENTORY",        BtString::NULL_STR,                     nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "CULTURE_DATE",     BtString::NULL_STR,                     nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MISC>...</MISC> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Misc>{"MISC"};
   EnumStringMapping const BEER_XML_MISC_TYPE_MAPPER {
      {"Spice",       Misc::Type::Spice},
      {"Fining",      Misc::Type::Fining},
      {"Water Agent", Misc::Type::Water_Agent},
      {"Herb",        Misc::Type::Herb},
      {"Flavor",      Misc::Type::Flavor},
      {"Other",       Misc::Type::Other}
   };
   EnumStringMapping const BEER_XML_MISC_USE_MAPPER {
      {"Boil",      Misc::Use::Boil},
      {"Mash",      Misc::Use::Mash},
      {"Primary",   Misc::Use::Primary},
      {"Secondary", Misc::Use::Secondary},
      {"Bottling",  Misc::Use::Bottling}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Misc> {
      // Type                                  XPath               Q_PROPERTY                           Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",             PropertyNames::NamedEntity::name,    nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",          VERSION1,                            nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",             PropertyNames::Misc::type,           &BEER_XML_MISC_TYPE_MAPPER},
      {XmlRecord::FieldType::Enum,             "USE",              PropertyNames::Misc::use,            &BEER_XML_MISC_USE_MAPPER},
      {XmlRecord::FieldType::Double,           "TIME",             PropertyNames::Misc::time,           nullptr},
      {XmlRecord::FieldType::Double,           "AMOUNT",           PropertyNames::Misc::amount,         nullptr},
      {XmlRecord::FieldType::Bool,             "AMOUNT_IS_WEIGHT", PropertyNames::Misc::amountIsWeight, nullptr},
      {XmlRecord::FieldType::String,           "USE_FOR",          PropertyNames::Misc::useFor,         nullptr},
      {XmlRecord::FieldType::String,           "NOTES",            PropertyNames::Misc::notes,          nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_AMOUNT",   BtString::NULL_STR,                  nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "INVENTORY",        BtString::NULL_STR,                  nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TIME",     BtString::NULL_STR,                  nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <WATER>...</WATER> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Water>{"WATER"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Water> {
      // Type                                  XPath             Q_PROPERTY                             Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",           PropertyNames::NamedEntity::name,      nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",        VERSION1,                              nullptr},
      {XmlRecord::FieldType::Double,           "AMOUNT",         PropertyNames::Water::amount,          nullptr},
      {XmlRecord::FieldType::Double,           "CALCIUM",        PropertyNames::Water::calcium_ppm,     nullptr},
      {XmlRecord::FieldType::Double,           "BICARBONATE",    PropertyNames::Water::bicarbonate_ppm, nullptr},
      {XmlRecord::FieldType::Double,           "SULFATE",        PropertyNames::Water::sulfate_ppm,     nullptr},
      {XmlRecord::FieldType::Double,           "CHLORIDE",       PropertyNames::Water::chloride_ppm,    nullptr},
      {XmlRecord::FieldType::Double,           "SODIUM",         PropertyNames::Water::sodium_ppm,      nullptr},
      {XmlRecord::FieldType::Double,           "MAGNESIUM",      PropertyNames::Water::magnesium_ppm,   nullptr},
      {XmlRecord::FieldType::Double,           "PH",             PropertyNames::Water::ph,              nullptr},
      {XmlRecord::FieldType::String,           "NOTES",          PropertyNames::Water::notes,           nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_AMOUNT", BtString::NULL_STR,                    nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <STYLE>...</STYLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Style>{"STYLE"};
   EnumStringMapping const BEER_XML_STYLE_TYPE_MAPPER {
      {"Lager", Style::Type::Lager},
      {"Ale",   Style::Type::Ale},
      {"Mead",  Style::Type::Mead},
      {"Wheat", Style::Type::Wheat},
      {"Mixed", Style::Type::Mixed},
      {"Cider", Style::Type::Cider}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Style> {
      // Type                                  XPath                Q_PROPERTY                            Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",              PropertyNames::NamedEntity::name,     nullptr},
      {XmlRecord::FieldType::String,           "CATEGORY",          PropertyNames::Style::category,       nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",           VERSION1,                             nullptr},
      {XmlRecord::FieldType::String,           "CATEGORY_NUMBER",   PropertyNames::Style::categoryNumber, nullptr}, // NB: Despite the name, this is specified as Text in the BeerXML 1.0 standard
      {XmlRecord::FieldType::String,           "STYLE_LETTER",      PropertyNames::Style::styleLetter,    nullptr},
      {XmlRecord::FieldType::String,           "STYLE_GUIDE",       PropertyNames::Style::styleGuide,     nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",              PropertyNames::Style::type,           &BEER_XML_STYLE_TYPE_MAPPER},
      {XmlRecord::FieldType::Double,           "OG_MIN",            PropertyNames::Style::ogMin,          nullptr},
      {XmlRecord::FieldType::Double,           "OG_MAX",            PropertyNames::Style::ogMax,          nullptr},
      {XmlRecord::FieldType::Double,           "FG_MIN",            PropertyNames::Style::fgMin,          nullptr},
      {XmlRecord::FieldType::Double,           "FG_MAX",            PropertyNames::Style::fgMax,          nullptr},
      {XmlRecord::FieldType::Double,           "IBU_MIN",           PropertyNames::Style::ibuMin,         nullptr},
      {XmlRecord::FieldType::Double,           "IBU_MAX",           PropertyNames::Style::ibuMax,         nullptr},
      {XmlRecord::FieldType::Double,           "COLOR_MIN",         PropertyNames::Style::colorMin_srm,   nullptr},
      {XmlRecord::FieldType::Double,           "COLOR_MAX",         PropertyNames::Style::colorMax_srm,   nullptr},
      {XmlRecord::FieldType::Double,           "CARB_MIN",          PropertyNames::Style::carbMin_vol,    nullptr},
      {XmlRecord::FieldType::Double,           "CARB_MAX",          PropertyNames::Style::carbMax_vol,    nullptr},
      {XmlRecord::FieldType::Double,           "ABV_MIN",           PropertyNames::Style::abvMin_pct,     nullptr},
      {XmlRecord::FieldType::Double,           "ABV_MAX",           PropertyNames::Style::abvMax_pct,     nullptr},
      {XmlRecord::FieldType::String,           "NOTES",             PropertyNames::Style::notes,          nullptr},
      {XmlRecord::FieldType::String,           "PROFILE",           PropertyNames::Style::profile,        nullptr},
      {XmlRecord::FieldType::String,           "INGREDIENTS",       PropertyNames::Style::ingredients,    nullptr},
      {XmlRecord::FieldType::String,           "EXAMPLES",          PropertyNames::Style::examples,       nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_OG_MIN",    BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_OG_MAX",    BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_FG_MIN",    BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_FG_MAX",    BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_COLOR_MIN", BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_COLOR_MAX", BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "OG_RANGE",          BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "FG_RANGE",          BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "IBU_RANGE",         BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "CARB_RANGE",        BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "COLOR_RANGE",       BtString::NULL_STR,                   nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "ABV_RANGE",         BtString::NULL_STR,                   nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH_STEP>...</MASH_STEP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<MashStep>{"MASH_STEP"};
   EnumStringMapping const BEER_XML_MASH_STEP_TYPE_MAPPER {
      {"Infusion",                      MashStep::Type::Infusion},
      {"Temperature",                   MashStep::Type::Temperature},
      {"Decoction",                     MashStep::Type::Decoction},
      // Inside Brewtarget we also have MashStep::flySparge and MashStep::batchSparge which are not mentioned in the
      // BeerXML 1.0 Standard.  They get treated as "Infusion" when we write to BeerXML
      // Note that we include a comment here to ensure we don't have multiple mappings from "Infusion"
      {"Infusion<!-- Fly Sparge -->",   MashStep::Type::flySparge},
      {"Infusion<!-- Batch Sparge -->", MashStep::Type::batchSparge}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<MashStep> {
      // Type                                  XPath                 Q_PROPERTY                                  Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",               PropertyNames::NamedEntity::name,           nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",            VERSION1,                                   nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",               PropertyNames::MashStep::type,              &BEER_XML_MASH_STEP_TYPE_MAPPER},
      {XmlRecord::FieldType::Double,           "INFUSE_AMOUNT",      PropertyNames::MashStep::infuseAmount_l,    nullptr}, // Should not be supplied if TYPE is "Decoction"
      {XmlRecord::FieldType::Double,           "STEP_TEMP",          PropertyNames::MashStep::stepTemp_c,        nullptr},
      {XmlRecord::FieldType::Double,           "STEP_TIME",          PropertyNames::MashStep::stepTime_min,      nullptr},
      {XmlRecord::FieldType::Double,           "RAMP_TIME",          PropertyNames::MashStep::rampTime_min,      nullptr},
      {XmlRecord::FieldType::Double,           "END_TEMP",           PropertyNames::MashStep::endTemp_c,         nullptr},
      {XmlRecord::FieldType::String,           "DESCRIPTION",        BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "WATER_GRAIN_RATIO",  BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DECOCTION_AMT",      BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "INFUSE_TEMP",        PropertyNames::MashStep::infuseTemp_c,      nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_STEP_TEMP",  BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_INFUSE_AMT", BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::Double,           "DECOCTION_AMOUNT",   PropertyNames::MashStep::decoctionAmount_l, nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH>...</MASH> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Mash>{"MASH"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Mash> {
      // Type                                  XPath                   Q_PROPERTY                                  Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",                 PropertyNames::NamedEntity::name,           nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",              VERSION1,                                   nullptr},
      {XmlRecord::FieldType::Double,           "GRAIN_TEMP",           PropertyNames::Mash::grainTemp_c,           nullptr},
      {XmlRecord::FieldType::RecordComplex,    "MASH_STEPS/MASH_STEP", PropertyNames::Mash::mashSteps,             nullptr}, // Additional logic for "MASH-STEPS" is handled in code
      {XmlRecord::FieldType::String,           "NOTES",                PropertyNames::Mash::notes,                 nullptr},
      {XmlRecord::FieldType::Double,           "TUN_TEMP",             PropertyNames::Mash::tunTemp_c,             nullptr},
      {XmlRecord::FieldType::Double,           "SPARGE_TEMP",          PropertyNames::Mash::spargeTemp_c,          nullptr},
      {XmlRecord::FieldType::Double,           "PH",                   PropertyNames::Mash::ph,                    nullptr},
      {XmlRecord::FieldType::Double,           "TUN_WEIGHT",           PropertyNames::Mash::tunWeight_kg,          nullptr},
      {XmlRecord::FieldType::Double,           "TUN_SPECIFIC_HEAT",    PropertyNames::Mash::tunSpecificHeat_calGC, nullptr},
      {XmlRecord::FieldType::Bool,             "EQUIP_ADJUST",         PropertyNames::Mash::equipAdjust,           nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_GRAIN_TEMP",   BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TUN_TEMP",     BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_SPARGE_TEMP",  BtString::NULL_STR,                         nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TUN_WEIGHT",   BtString::NULL_STR,                         nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Equipment>{"EQUIPMENT"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Equipment> {
      // Type                                  XPath                        Q_PROPERTY                                       Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",                      PropertyNames::NamedEntity::name,                nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",                   VERSION1,                                        nullptr},
      {XmlRecord::FieldType::Double,           "BOIL_SIZE",                 PropertyNames::Equipment::boilSize_l,            nullptr},
      {XmlRecord::FieldType::Double,           "BATCH_SIZE",                PropertyNames::Equipment::batchSize_l,           nullptr},
      {XmlRecord::FieldType::Double,           "TUN_VOLUME",                PropertyNames::Equipment::tunVolume_l,           nullptr},
      {XmlRecord::FieldType::Double,           "TUN_WEIGHT",                PropertyNames::Equipment::tunWeight_kg,          nullptr},
      {XmlRecord::FieldType::Double,           "TUN_SPECIFIC_HEAT",         PropertyNames::Equipment::tunSpecificHeat_calGC, nullptr},
      {XmlRecord::FieldType::Double,           "TOP_UP_WATER",              PropertyNames::Equipment::topUpWater_l,          nullptr},
      {XmlRecord::FieldType::Double,           "TRUB_CHILLER_LOSS",         PropertyNames::Equipment::trubChillerLoss_l,     nullptr},
      {XmlRecord::FieldType::Double,           "EVAP_RATE",                 PropertyNames::Equipment::evapRate_pctHr,        nullptr},
      {XmlRecord::FieldType::Double,           "BOIL_TIME",                 PropertyNames::Equipment::boilTime_min,          nullptr},
      {XmlRecord::FieldType::Bool,             "CALC_BOIL_VOLUME",          PropertyNames::Equipment::calcBoilVolume,        nullptr},
      {XmlRecord::FieldType::Double,           "LAUTER_DEADSPACE",          PropertyNames::Equipment::lauterDeadspace_l,     nullptr},
      {XmlRecord::FieldType::Double,           "TOP_UP_KETTLE",             PropertyNames::Equipment::topUpKettle_l,         nullptr},
      {XmlRecord::FieldType::Double,           "HOP_UTILIZATION",           PropertyNames::Equipment::hopUtilization_pct,    nullptr},
      {XmlRecord::FieldType::String,           "NOTES",                     PropertyNames::Equipment::notes,                 nullptr},
      {XmlRecord::FieldType::String,           "DISPLAY_BOIL_SIZE",         BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_BATCH_SIZE",        BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TUN_VOLUME",        BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TUN_WEIGHT",        BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TOP_UP_WATER",      BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TRUB_CHILLER_LOSS", BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_LAUTER_DEADSPACE",  BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TOP_UP_KETTLE",     BtString::NULL_STR,                              nullptr}, // Extension tag
      {XmlRecord::FieldType::Double,           "REAL_EVAP_RATE",            PropertyNames::Equipment::evapRate_lHr,          nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
      {XmlRecord::FieldType::Double,           "ABSORPTION",                PropertyNames::Equipment::grainAbsorption_LKg,   nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
      {XmlRecord::FieldType::Double,           "BOILING_POINT",             PropertyNames::Equipment::boilingPoint_c,        nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <INSTRUCTION>...</INSTRUCTION> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Instruction>{"INSTRUCTION"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Instruction> {
      // Type                                  XPath         Q_PROPERTY                              Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",       PropertyNames::NamedEntity::name,       nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",    VERSION1,                               nullptr},
      {XmlRecord::FieldType::String,           "directions", PropertyNames::Instruction::directions, nullptr},
      {XmlRecord::FieldType::Bool,             "hasTimer",   PropertyNames::Instruction::hasTimer,   nullptr},
      {XmlRecord::FieldType::String,           "timervalue", PropertyNames::Instruction::timerValue, nullptr}, // NB XPath is lowercase and property is camelCase
      {XmlRecord::FieldType::Bool,             "completed",  PropertyNames::Instruction::completed,  nullptr},
      {XmlRecord::FieldType::Double,           "interval",   PropertyNames::Instruction::interval,   nullptr}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <BREWNOTE>...</BREWNOTE> BeerXML records
   // NB There is no NAME field on a BREWNOTE
   //
   // Since this is only used by Brewtarget/Brewken, we could probably lose the VERSION field here (with  corresponding
   // changes to BeerXml.xsd), at the cost of creating files that would not be readable by old versions of those
   // programs.  But it seems small bother to leave it be.
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<BrewNote>{"BREWNOTE"};
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<BrewNote> {
      // Type                                  XPath                      Q_PROPERTY                                     Enum Mapper
      {XmlRecord::FieldType::RequiredConstant, "VERSION",                 VERSION1,                                      nullptr},
      {XmlRecord::FieldType::Date,             "BREWDATE",                PropertyNames::BrewNote::brewDate,             nullptr},
      {XmlRecord::FieldType::Date,             "DATE_FERMENTED_OUT",      PropertyNames::BrewNote::fermentDate,          nullptr},
      {XmlRecord::FieldType::String,           "NOTES",                   PropertyNames::BrewNote::notes,                nullptr},
      {XmlRecord::FieldType::Double,           "SG",                      PropertyNames::BrewNote::sg,                   nullptr},
      {XmlRecord::FieldType::Double,           "ACTUAL_ABV",              PropertyNames::BrewNote::abv,                  nullptr},
      {XmlRecord::FieldType::Double,           "EFF_INTO_BK",             PropertyNames::BrewNote::effIntoBK_pct,        nullptr},
      {XmlRecord::FieldType::Double,           "BREWHOUSE_EFF",           PropertyNames::BrewNote::brewhouseEff_pct,     nullptr},
      {XmlRecord::FieldType::Double,           "VOLUME_INTO_BK",          PropertyNames::BrewNote::volumeIntoBK_l,       nullptr},
      {XmlRecord::FieldType::Double,           "STRIKE_TEMP",             PropertyNames::BrewNote::strikeTemp_c,         nullptr},
      {XmlRecord::FieldType::Double,           "MASH_FINAL_TEMP",         PropertyNames::BrewNote::mashFinTemp_c,        nullptr},
      {XmlRecord::FieldType::Double,           "OG",                      PropertyNames::BrewNote::og,                   nullptr},
      {XmlRecord::FieldType::Double,           "POST_BOIL_VOLUME",        PropertyNames::BrewNote::postBoilVolume_l,     nullptr},
      {XmlRecord::FieldType::Double,           "VOLUME_INTO_FERMENTER",   PropertyNames::BrewNote::volumeIntoFerm_l,     nullptr},
      {XmlRecord::FieldType::Double,           "PITCH_TEMP",              PropertyNames::BrewNote::pitchTemp_c,          nullptr},
      {XmlRecord::FieldType::Double,           "FG",                      PropertyNames::BrewNote::fg,                   nullptr},
      {XmlRecord::FieldType::Double,           "ATTENUATION",             PropertyNames::BrewNote::attenuation,          nullptr},
      {XmlRecord::FieldType::Double,           "FINAL_VOLUME",            PropertyNames::BrewNote::finalVolume_l,        nullptr},
      {XmlRecord::FieldType::Double,           "BOIL_OFF",                PropertyNames::BrewNote::boilOff_l,            nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_BOIL_GRAV",     PropertyNames::BrewNote::projBoilGrav,         nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_VOL_INTO_BK",   PropertyNames::BrewNote::projVolIntoBK_l,      nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_STRIKE_TEMP",   PropertyNames::BrewNote::projStrikeTemp_c,     nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_MASH_FIN_TEMP", PropertyNames::BrewNote::projMashFinTemp_c,    nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_OG",            PropertyNames::BrewNote::projOg,               nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_VOL_INTO_FERM", PropertyNames::BrewNote::projVolIntoFerm_l,    nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_FG",            PropertyNames::BrewNote::projFg,               nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_EFF",           PropertyNames::BrewNote::projEff_pct,          nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_ABV",           PropertyNames::BrewNote::projABV_pct,          nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_POINTS",        PropertyNames::BrewNote::projPoints,           nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_FERM_POINTS",   PropertyNames::BrewNote::projFermPoints,       nullptr},
      {XmlRecord::FieldType::Double,           "PROJECTED_ATTEN",         PropertyNames::BrewNote::projAtten,            nullptr}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <RECIPE>...</RECIPE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   template<> QString const BEER_XML_RECORD_NAME<Recipe>{"RECIPE"};
   EnumStringMapping const BEER_XML_RECIPE_STEP_TYPE_MAPPER {
      {"Extract",      Recipe::Type::Extract},
      {"Partial Mash", Recipe::Type::PartialMash},
      {"All Grain",    Recipe::Type::AllGrain}
   };
   template<> XmlRecord::FieldDefinitions const BEER_XML_RECORD_FIELDS<Recipe> {
      // Type                                  XPath                       Q_PROPERTY                                 Enum Mapper
      {XmlRecord::FieldType::String,           "NAME",                     PropertyNames::NamedEntity::name,          nullptr},
      {XmlRecord::FieldType::RequiredConstant, "VERSION",                  VERSION1,                                  nullptr},
      {XmlRecord::FieldType::Enum,             "TYPE",                     PropertyNames::Recipe::recipeType,         &BEER_XML_RECIPE_STEP_TYPE_MAPPER},
      {XmlRecord::FieldType::RecordSimple,     "STYLE",                    PropertyNames::Recipe::style,              nullptr},
      {XmlRecord::FieldType::RecordSimple,     "EQUIPMENT",                PropertyNames::Recipe::equipment,          nullptr},
      {XmlRecord::FieldType::String,           "BREWER",                   PropertyNames::Recipe::brewer,             nullptr},
      {XmlRecord::FieldType::String,           "ASST_BREWER",              PropertyNames::Recipe::asstBrewer,         nullptr},
      {XmlRecord::FieldType::Double,           "BATCH_SIZE",               PropertyNames::Recipe::batchSize_l,        nullptr},
      {XmlRecord::FieldType::Double,           "BOIL_SIZE",                PropertyNames::Recipe::boilSize_l,         nullptr},
      {XmlRecord::FieldType::Double,           "BOIL_TIME",                PropertyNames::Recipe::boilTime_min,       nullptr},
      {XmlRecord::FieldType::Double,           "EFFICIENCY",               PropertyNames::Recipe::efficiency_pct,     nullptr},
      {XmlRecord::FieldType::RecordComplex,    "HOPS/HOP",                 PropertyNames::Recipe::hops,               nullptr}, // Additional logic for "HOPS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::FieldType::RecordComplex,    "FERMENTABLES/FERMENTABLE", PropertyNames::Recipe::fermentables,       nullptr}, // Additional logic for "FERMENTABLES" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::FieldType::RecordComplex,    "MISCS/MISC",               PropertyNames::Recipe::miscs,              nullptr}, // Additional logic for "MISCS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::FieldType::RecordComplex,    "YEASTS/YEAST",             PropertyNames::Recipe::yeasts,             nullptr}, // Additional logic for "YEASTS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::FieldType::RecordComplex,    "WATERS/WATER",             PropertyNames::Recipe::waters,             nullptr}, // Additional logic for "WATERS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::FieldType::RecordSimple,     "MASH",                     PropertyNames::Recipe::mash,               nullptr},
      {XmlRecord::FieldType::RecordComplex,    "INSTRUCTIONS/INSTRUCTION", PropertyNames::Recipe::instructions,       nullptr}, // Additional logic for "INSTRUCTIONS" is handled in xml/XmlNamedEntityRecord.h
      {XmlRecord::FieldType::RecordComplex,    "BREWNOTES/BREWNOTE",       PropertyNames::Recipe::brewNotes,          nullptr}, // Additional logic for "BREWNOTES" is handled in xml/XmlNamedEntityRecord.h
      {XmlRecord::FieldType::String,           "NOTES",                    PropertyNames::Recipe::notes,              nullptr},
      {XmlRecord::FieldType::String,           "TASTE_NOTES",              PropertyNames::Recipe::tasteNotes,         nullptr},
      {XmlRecord::FieldType::Double,           "TASTE_RATING",             PropertyNames::Recipe::tasteRating,        nullptr},
      {XmlRecord::FieldType::Double,           "OG",                       PropertyNames::Recipe::og,                 nullptr},
      {XmlRecord::FieldType::Double,           "FG",                       PropertyNames::Recipe::fg,                 nullptr},
      {XmlRecord::FieldType::UInt,             "FERMENTATION_STAGES",      PropertyNames::Recipe::fermentationStages, nullptr},
      {XmlRecord::FieldType::Double,           "PRIMARY_AGE",              PropertyNames::Recipe::primaryAge_days,    nullptr},
      {XmlRecord::FieldType::Double,           "PRIMARY_TEMP",             PropertyNames::Recipe::primaryTemp_c,      nullptr},
      {XmlRecord::FieldType::Double,           "SECONDARY_AGE",            PropertyNames::Recipe::secondaryAge_days,  nullptr},
      {XmlRecord::FieldType::Double,           "SECONDARY_TEMP",           PropertyNames::Recipe::secondaryTemp_c,    nullptr},
      {XmlRecord::FieldType::Double,           "TERTIARY_AGE",             PropertyNames::Recipe::tertiaryAge_days,   nullptr},
      {XmlRecord::FieldType::Double,           "TERTIARY_TEMP",            PropertyNames::Recipe::tertiaryTemp_c,     nullptr},
      {XmlRecord::FieldType::Double,           "AGE",                      PropertyNames::Recipe::age,                nullptr},
      {XmlRecord::FieldType::Double,           "AGE_TEMP",                 PropertyNames::Recipe::ageTemp_c,          nullptr},
      {XmlRecord::FieldType::Date,             "DATE",                     PropertyNames::Recipe::date,               nullptr},
      {XmlRecord::FieldType::Double,           "CARBONATION",              PropertyNames::Recipe::carbonation_vols,   nullptr},
      {XmlRecord::FieldType::Bool,             "FORCED_CARBONATION",       PropertyNames::Recipe::forcedCarbonation,  nullptr},
      {XmlRecord::FieldType::String,           "PRIMING_SUGAR_NAME",       PropertyNames::Recipe::primingSugarName,   nullptr},
      {XmlRecord::FieldType::Double,           "CARBONATION_TEMP",         PropertyNames::Recipe::carbonationTemp_c,  nullptr},
      {XmlRecord::FieldType::Double,           "PRIMING_SUGAR_EQUIV",      PropertyNames::Recipe::primingSugarEquiv,  nullptr},
      {XmlRecord::FieldType::Double,           "KEG_PRIMING_FACTOR",       PropertyNames::Recipe::kegPrimingFactor,   nullptr},
      {XmlRecord::FieldType::String,           "EST_OG",                   BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "EST_FG",                   BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "EST_COLOR",                BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "IBU",                      PropertyNames::Recipe::IBU,                nullptr}, // Extension tag.  We write but ignore on read if present.
      {XmlRecord::FieldType::String,           "IBU_METHOD",               BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "EST_ABV",                  BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "ABV",                      BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "ACTUAL_EFFICIENCY",        BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "CALORIES",                 BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_BATCH_SIZE",       BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_BOIL_SIZE",        BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_OG",               BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_FG",               BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_PRIMARY_TEMP",     BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_SECONDARY_TEMP",   BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_TERTIARY_TEMP",    BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_AGE_TEMP",         BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "CARBONATION_USED",         BtString::NULL_STR,                        nullptr}, // Extension tag
      {XmlRecord::FieldType::String,           "DISPLAY_CARB_TEMP",        BtString::NULL_STR,                        nullptr}  // Extension tag
   };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// This private implementation class holds all private non-virtual members of BeerXML
class BeerXML::impl {
public:

   /**
    * Constructor
    */
   impl() : BeerXml1Coding{
               "BeerXML 1.0",
               ":/schemas/beerxml/v1/BeerXml.xsd",
               QHash<QString, XmlCoding::XmlRecordDefinition>{
                  {BEER_XML_RECORD_NAME<void>       , {&XmlCoding::construct<void>,        &BEER_XML_RECORD_FIELDS<void>       } }, //Root
                  {BEER_XML_RECORD_NAME<Hop>        , {&XmlCoding::construct<Hop>,         &BEER_XML_RECORD_FIELDS<Hop>        } },
                  {BEER_XML_RECORD_NAME<Fermentable>, {&XmlCoding::construct<Fermentable>, &BEER_XML_RECORD_FIELDS<Fermentable>} },
                  {BEER_XML_RECORD_NAME<Yeast>      , {&XmlCoding::construct<Yeast>,       &BEER_XML_RECORD_FIELDS<Yeast>      } },
                  {BEER_XML_RECORD_NAME<Misc>       , {&XmlCoding::construct<Misc>,        &BEER_XML_RECORD_FIELDS<Misc>       } },
                  {BEER_XML_RECORD_NAME<Water>      , {&XmlCoding::construct<Water>,       &BEER_XML_RECORD_FIELDS<Water>      } },
                  {BEER_XML_RECORD_NAME<Style>      , {&XmlCoding::construct<Style>,       &BEER_XML_RECORD_FIELDS<Style>      } },
                  {BEER_XML_RECORD_NAME<MashStep>   , {&XmlCoding::construct<MashStep>,    &BEER_XML_RECORD_FIELDS<MashStep>   } },
                  {BEER_XML_RECORD_NAME<Mash>       , {&XmlCoding::construct<Mash>,        &BEER_XML_RECORD_FIELDS<Mash>       } },
                  {BEER_XML_RECORD_NAME<Equipment>  , {&XmlCoding::construct<Equipment>,   &BEER_XML_RECORD_FIELDS<Equipment>  } },
                  {BEER_XML_RECORD_NAME<Instruction>, {&XmlCoding::construct<Instruction>, &BEER_XML_RECORD_FIELDS<Instruction>} },
                  {BEER_XML_RECORD_NAME<BrewNote>   , {&XmlCoding::construct<BrewNote>,    &BEER_XML_RECORD_FIELDS<BrewNote>   } },
                  {BEER_XML_RECORD_NAME<Recipe>     , {&XmlCoding::construct<Recipe>,      &BEER_XML_RECORD_FIELDS<Recipe>     } }
               }
            } {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * Export an individual object to BeerXML
    */
   template<class NE> void toXml(NE & ne, QTextStream & out) {
      std::shared_ptr<XmlRecord> xmlRecord{
         XmlCoding::construct<NE>(BEER_XML_RECORD_NAME<NE>,
                                  this->BeerXml1Coding,
                                  BEER_XML_RECORD_FIELDS<NE>)
      };
      xmlRecord->toXml(ne, out);
      return;
   }

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
      //      any UTF-8 encoded XML document.  (This is perhaps because it seems to mandate an ISO-8859-1 coding of
      //      BeerXML files, though there is no explicit discussion of file encodings in the standard, and this seems
      //      an unnecessary constraint to place on files.)
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
      // We _could_ make "BEER_XML" some sort of constant eg:
      //    constexpr static char const * const INSERTED_ROOT_NODE_NAME = "BEER_XML";
      // but we wouldn't be able to use that constant in beerxml/v1/BeerXml.xsd, and using it in the few C++ places we
      // need it (this file and BeerXmlRootRecord.cpp) would be cumbersome, making the code more difficult to follow.
      // Since we're unlikely ever to need to change (or make much more widespread use of) this tag, we've gone with
      // readability over purity, and left it hard-coded, for now at least.
      //
      QByteArray documentData = inputFile.readLine();
      QString firstLine{documentData};
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
      documentData += "<BEER_XML>\n";
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

      return this->BeerXml1Coding.validateLoadAndStoreInDb(documentData, fileName, domErrorHandler, userMessage);

   }

private:

   XmlCoding const BeerXml1Coding;
};


BeerXML & BeerXML::getInstance() {
   //
   // As of C++11, simple "Meyers singleton" is now thread-safe -- see
   // https://www.modernescpp.com/index.php/thread-safe-initialization-of-a-singleton#h3-guarantees-of-the-c-runtime
   //
   static BeerXML singleton;

   return singleton;
}


BeerXML::BeerXML() : pimpl{ new impl{} } {
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
BeerXML::~BeerXML() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BeerXML::createXmlFile(QFile & outFile) const {
   QTextStream out(&outFile);
   // BeerXML specifies the ISO-8859-1 encoding
   out.setCodec(QTextCodec::codecForMib(CharacterSets::ISO_8859_1_1987));

   out <<
      "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
      "<!-- BeerXML Format generated by Brewtarget " << CONFIG_VERSION_STRING << " on " <<
      QDateTime::currentDateTime().date().toString(Qt::ISODate) << " -->\n";

   return;
}

template<class NE> void BeerXML::toXml(QList<NE *> const & nes, QFile & outFile) const {
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
   // .:TODO:. In Qt6, QTextCodec and QTextStream::setCodec have been removed and are replaced by QStringConverter
   // (which is new in Qt6).
   out.setCodec(QTextCodec::codecForMib(CharacterSets::ISO_8859_1_1987));
   out << "<" << BEER_XML_RECORD_NAME<NE> << "S>\n";
   for (auto ne : nes) {
      this->pimpl->toXml(*ne, out);
   }
   out << "</" << BEER_XML_RECORD_NAME<NE> << "S>\n";
   return;
}
//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header, which
// means, amongst other things, that we can reference the pimpl.)
//
template void BeerXML::toXml(QList<Hop *> const &        nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Fermentable *> const &nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Yeast *> const &      nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Misc *> const &       nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Water *> const &      nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Style *> const &      nes, QFile & outFile) const;
template void BeerXML::toXml(QList<MashStep *> const &   nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Mash *> const &       nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Equipment *> const &  nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Instruction *> const &nes, QFile & outFile) const;
template void BeerXML::toXml(QList<BrewNote *> const &   nes, QFile & outFile) const;
template void BeerXML::toXml(QList<Recipe *> const &     nes, QFile & outFile) const;

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
   bool result = this->pimpl->validateAndLoad(filename, userMessage);
   QApplication::restoreOverrideCursor();
   return result;
}
