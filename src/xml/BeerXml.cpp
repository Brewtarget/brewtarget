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

#include <QList>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QTextCodec>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QPair>

#include "Algorithms.h"
#include "DatabaseSchema.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Salt.h"
#include "model/Yeast.h"
#include "xml/BtDomErrorHandler.h"
#include "xml/XmlCoding.h"
#include "xml/XmlRecord.h"

#include "TableSchema.h"
//
// Variables and constant definitions that we need only in this file
//
namespace {

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Top-level field mappings for BeerXML files
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_ROOT_RECORD_FIELDS {
      // Type              XPath                      Q_PROPERTY   Enum Mapper
      {XmlRecord::Record, "HOPS/HOP",                 nullptr,     nullptr},
      {XmlRecord::Record, "FERMENTABLES/FERMENTABLE", nullptr,     nullptr},
      {XmlRecord::Record, "YEASTS/YEAST",             nullptr,     nullptr},
      {XmlRecord::Record, "MISCS/MISC",               nullptr,     nullptr},
      {XmlRecord::Record, "WATERS/WATER",             nullptr,     nullptr},
      {XmlRecord::Record, "STYLES/STYLE",             nullptr,     nullptr},
      {XmlRecord::Record, "MASHS/MASH",               nullptr,     nullptr},
      {XmlRecord::Record, "RECIPES/RECIPE",           nullptr,     nullptr},
      {XmlRecord::Record, "EQUIPMENTS/EQUIPMENT",     nullptr,     nullptr},
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <HOP>...</HOP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_HOP_USE_MAPPER {
      {"Boil",       Hop::Boil},
      {"Dry Hop",    Hop::Dry_Hop},
      {"Mash",       Hop::Mash},
      {"First Wort", Hop::First_Wort},
      {"Aroma",      Hop::UseAroma}
   };
   XmlRecord::EnumLookupMap const BEER_XML_HOP_TYPE_MAPPER {
      {"Bittering", Hop::Bittering},
      {"Aroma",     Hop::Aroma},
      {"Both",      Hop::Both}
   };
   XmlRecord::EnumLookupMap const BEER_XML_HOP_FORM_MAPPER {
      {"Pellet", Hop::Pellet},
      {"Plug",   Hop::Plug},
      {"Leaf",   Hop::Leaf}
   };
   XmlRecord::FieldDefinitions const BEER_XML_HOP_RECORD_FIELDS {
      // Type              XPath             Q_PROPERTY                             Enum Mapper
      {XmlRecord::String,  "NAME",           PropertyNames::NamedEntity::name,      nullptr},
      {XmlRecord::UInt,    "VERSION",        nullptr,                               nullptr},
      {XmlRecord::Double,  "ALPHA",          PropertyNames::Hop::alpha_pct,         nullptr},
      {XmlRecord::Double,  "AMOUNT",         PropertyNames::Hop::amount_kg,         nullptr},
      {XmlRecord::Enum,    "USE",            PropertyNames::Hop::use,               &BEER_XML_HOP_USE_MAPPER},
      {XmlRecord::Double,  "TIME",           PropertyNames::Hop::time_min,          nullptr},
      {XmlRecord::String,  "NOTES",          PropertyNames::Hop::notes,             nullptr},
      {XmlRecord::Enum,    "TYPE",           PropertyNames::Hop::type,              &BEER_XML_HOP_TYPE_MAPPER},
      {XmlRecord::Enum,    "FORM",           PropertyNames::Hop::form,              &BEER_XML_HOP_FORM_MAPPER},
      {XmlRecord::Double,  "BETA",           PropertyNames::Hop::beta_pct,          nullptr},
      {XmlRecord::Double,  "HSI",            PropertyNames::Hop::hsi_pct,           nullptr},
      {XmlRecord::String,  "ORIGIN",         PropertyNames::Hop::origin,            nullptr},
      {XmlRecord::String,  "SUBSTITUTES",    PropertyNames::Hop::substitutes,       nullptr},
      {XmlRecord::Double,  "HUMULENE",       PropertyNames::Hop::humulene_pct,      nullptr},
      {XmlRecord::Double,  "CARYOPHYLLENE",  PropertyNames::Hop::caryophyllene_pct, nullptr},
      {XmlRecord::Double,  "COHUMULONE",     PropertyNames::Hop::cohumulone_pct,    nullptr},
      {XmlRecord::Double,  "MYRCENE",        PropertyNames::Hop::myrcene_pct,       nullptr},
      {XmlRecord::String,  "DISPLAY_AMOUNT", nullptr,                               nullptr}, // Extension tag
      {XmlRecord::String,  "INVENTORY",      nullptr,                               nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TIME",   nullptr,                               nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_FERMENTABLE_TYPE_MAPPER {
      {"Grain",       Fermentable::Grain},
      {"Sugar",       Fermentable::Sugar},
      {"Extract",     Fermentable::Extract},
      {"Dry Extract", Fermentable::Dry_Extract},
      {"Adjunct",     Fermentable::Adjunct}
   };
   XmlRecord::FieldDefinitions const BEER_XML_FERMENTABLE_RECORD_FIELDS {
      // Type              XPath               Q_PROPERTY                                          Enum Mapper
      {XmlRecord::String,  "NAME",             PropertyNames::NamedEntity::name,                   nullptr},
      {XmlRecord::UInt,    "VERSION",          nullptr,                                            nullptr},
      {XmlRecord::Enum,    "TYPE",             PropertyNames::Fermentable::type,                   &BEER_XML_FERMENTABLE_TYPE_MAPPER},
      {XmlRecord::Double,  "AMOUNT",           PropertyNames::Fermentable::amount_kg,              nullptr},
      {XmlRecord::Double,  "YIELD",            PropertyNames::Fermentable::yield_pct,              nullptr},
      {XmlRecord::Double,  "COLOR",            PropertyNames::Fermentable::color_srm,              nullptr},
      {XmlRecord::Bool,    "ADD_AFTER_BOIL",   PropertyNames::Fermentable::addAfterBoil,           nullptr},
      {XmlRecord::String,  "ORIGIN",           PropertyNames::Fermentable::origin,                 nullptr},
      {XmlRecord::String,  "SUPPLIER",         PropertyNames::Fermentable::supplier,               nullptr},
      {XmlRecord::String,  "NOTES",            PropertyNames::Fermentable::notes,                  nullptr},
      {XmlRecord::Double,  "COARSE_FINE_DIFF", PropertyNames::Fermentable::coarseFineDiff_pct,     nullptr},
      {XmlRecord::Double,  "MOISTURE",         PropertyNames::Fermentable::moisture_pct,           nullptr},
      {XmlRecord::Double,  "DIASTATIC_POWER",  PropertyNames::Fermentable::diastaticPower_lintner, nullptr},
      {XmlRecord::Double,  "PROTEIN",          PropertyNames::Fermentable::protein_pct,            nullptr},
      {XmlRecord::Double,  "MAX_IN_BATCH",     PropertyNames::Fermentable::maxInBatch_pct,         nullptr},
      {XmlRecord::Bool,    "RECOMMEND_MASH",   PropertyNames::Fermentable::recommendMash,          nullptr},
      {XmlRecord::Double,  "IBU_GAL_PER_LB",   PropertyNames::Fermentable::ibuGalPerLb,            nullptr},
      {XmlRecord::String,  "DISPLAY_AMOUNT",   nullptr,                                            nullptr}, // Extension tag
      {XmlRecord::String,  "POTENTIAL",        nullptr,                                            nullptr}, // Extension tag
      {XmlRecord::String,  "INVENTORY",        nullptr,                                            nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_COLOR",    nullptr,                                            nullptr}, // Extension tag
      {XmlRecord::Bool,    "IS_MASHED",        PropertyNames::Fermentable::isMashed,               nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <YEAST>...</YEAST> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_YEAST_TYPE_MAPPER {
      {"Ale",       Yeast::Ale},
      {"Lager",     Yeast::Lager},
      {"Wheat",     Yeast::Wheat},
      {"Wine",      Yeast::Wine},
      {"Champagne", Yeast::Champagne}
   };
   XmlRecord::EnumLookupMap const BEER_XML_YEAST_FORM_MAPPER {
      {"Liquid",  Yeast::Liquid},
      {"Dry",     Yeast::Dry},
      {"Slant",   Yeast::Slant},
      {"Culture", Yeast::Culture}
   };
   XmlRecord::EnumLookupMap const BEER_XML_YEAST_FLOCCULATION_MAPPER {
      {"Low",       Yeast::Low},
      {"Medium",    Yeast::Medium},
      {"High",      Yeast::High},
      {"Very High", Yeast::Very_High}
   };
   XmlRecord::FieldDefinitions const BEER_XML_YEAST_RECORD_FIELDS {
      // Type              XPath               Q_PROPERTY                                    Enum Mapper
      {XmlRecord::String,  "NAME",             PropertyNames::NamedEntity::name,             nullptr},
      {XmlRecord::UInt,    "VERSION",          nullptr,                                      nullptr},
      {XmlRecord::Enum,    "TYPE",             PropertyNames::Yeast::type,                   &BEER_XML_YEAST_TYPE_MAPPER},
      {XmlRecord::Enum,    "FORM",             PropertyNames::Yeast::form,                   &BEER_XML_YEAST_FORM_MAPPER},
      {XmlRecord::Double,  "AMOUNT",           PropertyNames::Yeast::amount,                 nullptr},
      {XmlRecord::Bool,    "AMOUNT_IS_WEIGHT", PropertyNames::Yeast::amountIsWeight,         nullptr},
      {XmlRecord::String,  "LABORATORY",       PropertyNames::Yeast::laboratory,             nullptr},
      {XmlRecord::String,  "PRODUCT_ID",       PropertyNames::Yeast::productID,              nullptr},
      {XmlRecord::Double,  "MIN_TEMPERATURE",  PropertyNames::Yeast::minTemperature_c,       nullptr},
      {XmlRecord::Double,  "MAX_TEMPERATURE",  PropertyNames::Yeast::maxTemperature_c,       nullptr},
      {XmlRecord::Enum,    "FLOCCULATION",     PropertyNames::Yeast::flocculation,           &BEER_XML_YEAST_FLOCCULATION_MAPPER},
      {XmlRecord::Double,  "ATTENUATION",      PropertyNames::Yeast::attenuation_pct,        nullptr},
      {XmlRecord::String,  "NOTES",            PropertyNames::Yeast::notes,                  nullptr},
      {XmlRecord::String,  "BEST_FOR",         PropertyNames::Yeast::bestFor,                nullptr},
      {XmlRecord::Int,     "TIMES_CULTURED",   PropertyNames::Yeast::timesCultured,          nullptr},
      {XmlRecord::Int,     "MAX_REUSE",        PropertyNames::Yeast::maxReuse,               nullptr},
      {XmlRecord::Bool,    "ADD_TO_SECONDARY", PropertyNames::Yeast::addToSecondary,         nullptr},
      {XmlRecord::String,  "DISPLAY_AMOUNT",   nullptr,                                      nullptr}, // Extension tag
      {XmlRecord::String,  "DISP_MIN_TEMP",    nullptr,                                      nullptr}, // Extension tag
      {XmlRecord::String,  "DISP_MAX_TEMP",    nullptr,                                      nullptr}, // Extension tag
      {XmlRecord::String,  "INVENTORY",        nullptr,                                      nullptr}, // Extension tag
      {XmlRecord::String,  "CULTURE_DATE",     nullptr,                                      nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MISC>...</MISC> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_MISC_TYPE_MAPPER {
      {"Spice",       Misc::Spice},
      {"Fining",      Misc::Fining},
      {"Water Agent", Misc::Water_Agent},
      {"Herb",        Misc::Herb},
      {"Flavor",      Misc::Flavor},
      {"Other",       Misc::Other}
   };
   XmlRecord::EnumLookupMap const BEER_XML_MISC_USE_MAPPER {
      {"Boil",      Misc::Boil},
      {"Mash",      Misc::Mash},
      {"Primary",   Misc::Primary},
      {"Secondary", Misc::Secondary},
      {"Bottling",  Misc::Bottling}
   };
   XmlRecord::FieldDefinitions const BEER_XML_MISC_RECORD_FIELDS {
      // Type              XPath               Q_PROPERTY                           Enum Mapper
      {XmlRecord::String,  "NAME",             PropertyNames::NamedEntity::name,    nullptr},
      {XmlRecord::UInt,    "VERSION",          nullptr,                             nullptr},
      {XmlRecord::Enum,    "TYPE",             PropertyNames::Misc::type,           &BEER_XML_MISC_TYPE_MAPPER},
      {XmlRecord::Enum,    "USE",              PropertyNames::Misc::use,            &BEER_XML_MISC_USE_MAPPER},
      {XmlRecord::Double,  "TIME",             PropertyNames::Misc::time,           nullptr},
      {XmlRecord::Double,  "AMOUNT",           PropertyNames::Misc::amount,         nullptr},
      {XmlRecord::Bool,    "AMOUNT_IS_WEIGHT", PropertyNames::Misc::amountIsWeight, nullptr},
      {XmlRecord::String,  "USE_FOR",          PropertyNames::Misc::useFor,         nullptr},
      {XmlRecord::String,  "NOTES",            PropertyNames::Misc::notes,          nullptr},
      {XmlRecord::String,  "DISPLAY_AMOUNT",   nullptr,                             nullptr}, // Extension tag
      {XmlRecord::String,  "INVENTORY",        nullptr,                             nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TIME",     nullptr,                             nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <WATER>...</WATER> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_WATER_RECORD_FIELDS {
      // Type               XPath            Q_PROPERTY                             Enum Mapper
      {XmlRecord::String,  "NAME",           PropertyNames::NamedEntity::name,      nullptr},
      {XmlRecord::UInt,    "VERSION",        nullptr,                               nullptr},
      {XmlRecord::Double,  "AMOUNT",         PropertyNames::Water::amount,          nullptr},
      {XmlRecord::Double,  "CALCIUM",        PropertyNames::Water::calcium_ppm,     nullptr},
      {XmlRecord::Double,  "BICARBONATE",    PropertyNames::Water::bicarbonate_ppm, nullptr},
      {XmlRecord::Double,  "SULFATE",        PropertyNames::Water::sulfate_ppm,     nullptr},
      {XmlRecord::Double,  "CHLORIDE",       PropertyNames::Water::chloride_ppm,    nullptr},
      {XmlRecord::Double,  "SODIUM",         PropertyNames::Water::sodium_ppm,      nullptr},
      {XmlRecord::Double,  "MAGNESIUM",      PropertyNames::Water::magnesium_ppm,   nullptr},
      {XmlRecord::Double,  "PH",             PropertyNames::Water::ph,              nullptr},
      {XmlRecord::String,  "NOTES",          PropertyNames::Water::notes,           nullptr},
      {XmlRecord::String,  "DISPLAY_AMOUNT", nullptr,                               nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <STYLE>...</STYLE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_STYLE_TYPE_MAPPER {
      {"Lager", Style::Lager},
      {"Ale",   Style::Ale},
      {"Mead",  Style::Mead},
      {"Wheat", Style::Wheat},
      {"Mixed", Style::Mixed},
      {"Cider", Style::Cider}
   };
   XmlRecord::FieldDefinitions const BEER_XML_STYLE_RECORD_FIELDS {
      // Type              XPath                Q_PROPERTY                            Enum Mapper
      {XmlRecord::String,  "NAME",              PropertyNames::NamedEntity::name,     nullptr},
      {XmlRecord::String,  "CATEGORY",          PropertyNames::Style::category,       nullptr},
      {XmlRecord::UInt,    "VERSION",           nullptr,                              nullptr},
      {XmlRecord::String,  "CATEGORY_NUMBER",   PropertyNames::Style::categoryNumber, nullptr},
      {XmlRecord::String,  "STYLE_LETTER",      PropertyNames::Style::styleLetter,    nullptr},
      {XmlRecord::String,  "STYLE_GUIDE",       PropertyNames::Style::styleGuide,     nullptr},
      {XmlRecord::Enum,    "TYPE",              PropertyNames::Style::type,           &BEER_XML_STYLE_TYPE_MAPPER},
      {XmlRecord::Double,  "OG_MIN",            PropertyNames::Style::ogMin,          nullptr},
      {XmlRecord::Double,  "OG_MAX",            PropertyNames::Style::ogMax,          nullptr},
      {XmlRecord::Double,  "FG_MIN",            PropertyNames::Style::fgMin,          nullptr},
      {XmlRecord::Double,  "FG_MAX",            PropertyNames::Style::fgMax,          nullptr},
      {XmlRecord::Double,  "IBU_MIN",           PropertyNames::Style::ibuMin,         nullptr},
      {XmlRecord::Double,  "IBU_MAX",           PropertyNames::Style::ibuMax,         nullptr},
      {XmlRecord::Double,  "COLOR_MIN",         PropertyNames::Style::colorMin_srm,   nullptr},
      {XmlRecord::Double,  "COLOR_MAX",         PropertyNames::Style::colorMax_srm,   nullptr},
      {XmlRecord::Double,  "CARB_MIN",          PropertyNames::Style::carbMin_vol,    nullptr},
      {XmlRecord::Double,  "CARB_MAX",          PropertyNames::Style::carbMax_vol,    nullptr},
      {XmlRecord::Double,  "ABV_MIN",           PropertyNames::Style::abvMin_pct,     nullptr},
      {XmlRecord::Double,  "ABV_MAX",           PropertyNames::Style::abvMax_pct,     nullptr},
      {XmlRecord::String,  "NOTES",             PropertyNames::Style::notes,          nullptr},
      {XmlRecord::String,  "PROFILE",           PropertyNames::Style::profile,        nullptr},
      {XmlRecord::String,  "INGREDIENTS",       PropertyNames::Style::ingredients,    nullptr},
      {XmlRecord::String,  "EXAMPLES",          PropertyNames::Style::examples,       nullptr},
      {XmlRecord::String,  "DISPLAY_OG_MIN",    nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_OG_MAX",    nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_FG_MIN",    nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_FG_MAX",    nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_COLOR_MIN", nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_COLOR_MAX", nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "OG_RANGE",          nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "FG_RANGE",          nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "IBU_RANGE",         nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "CARB_RANGE",        nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "COLOR_RANGE",       nullptr,                              nullptr}, // Extension tag
      {XmlRecord::String,  "ABV_RANGE",         nullptr,                              nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH_STEP>...</MASH_STEP> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_MASH_STEP_TYPE_MAPPER {
      {"Infusion",     MashStep::Infusion},
      {"Temperature",  MashStep::Temperature},
      {"Decoction",    MashStep::Decoction}
      // Internally we also have MashStep::flySparge and MashStep::batchSparge which are not mentioned in the
      // BeerXML 1.0 Standard.  They get treated as "Infusion" when we write to BeerXML.
   };
   XmlRecord::FieldDefinitions const BEER_XML_MASH_STEP_RECORD_FIELDS {
      // Type              XPath                 Q_PROPERTY                                  Enum Mapper
      {XmlRecord::String,  "NAME",               PropertyNames::NamedEntity::name,           nullptr},
      {XmlRecord::UInt,    "VERSION",            nullptr,                                    nullptr},
      {XmlRecord::Enum,    "TYPE",               PropertyNames::MashStep::type,              &BEER_XML_MASH_STEP_TYPE_MAPPER},
      {XmlRecord::Double,  "INFUSE_AMOUNT",      PropertyNames::MashStep::infuseAmount_l,    nullptr}, // Should not be supplied if TYPE is "Decoction"
      {XmlRecord::Double,  "STEP_TEMP",          PropertyNames::MashStep::stepTemp_c,        nullptr},
      {XmlRecord::Double,  "STEP_TIME",          PropertyNames::MashStep::stepTime_min,      nullptr},
      {XmlRecord::Double,  "RAMP_TIME",          PropertyNames::MashStep::rampTime_min,      nullptr},
      {XmlRecord::Double,  "END_TEMP",           PropertyNames::MashStep::endTemp_c,         nullptr},
      {XmlRecord::String,  "DESCRIPTION",        nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "WATER_GRAIN_RATIO",  nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "DECOCTION_AMT",      nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "INFUSE_TEMP",        PropertyNames::MashStep::infuseTemp_c,      nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_STEP_TEMP",  nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_INFUSE_AMT", nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::Double,  "DECOCTION_AMOUNT",   PropertyNames::MashStep::decoctionAmount_l, nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <MASH>...</MASH> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_MASH_RECORD_FIELDS {
      // Type               XPath                  Q_PROPERTY                                  Enum Mapper
      {XmlRecord::String,  "NAME",                 PropertyNames::NamedEntity::name,           nullptr},
      {XmlRecord::UInt,    "VERSION",              nullptr,                                    nullptr},
      {XmlRecord::Double,  "GRAIN_TEMP",           PropertyNames::Mash::grainTemp_c,           nullptr},
      {XmlRecord::Record,  "MASH_STEPS/MASH_STEP", nullptr,                                    nullptr}, // Additional logic for "MASH-STEPS" is handled in code
      {XmlRecord::String,  "NOTES",                PropertyNames::Mash::notes,                 nullptr},
      {XmlRecord::Double,  "TUN_TEMP",             PropertyNames::Mash::tunTemp_c,             nullptr},
      {XmlRecord::Double,  "SPARGE_TEMP",          PropertyNames::Mash::spargeTemp_c,          nullptr},
      {XmlRecord::Double,  "PH",                   PropertyNames::Mash::ph,                    nullptr},
      {XmlRecord::Double,  "TUN_WEIGHT",           PropertyNames::Mash::tunWeight_kg,          nullptr},
      {XmlRecord::Double,  "TUN_SPECIFIC_HEAT",    PropertyNames::Mash::tunSpecificHeat_calGC, nullptr},
      {XmlRecord::Bool,    "EQUIP_ADJUST",         PropertyNames::Mash::equipAdjust,           nullptr},
      {XmlRecord::String,  "DISPLAY_GRAIN_TEMP",   nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TUN_TEMP",     nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_SPARGE_TEMP",  nullptr,                                    nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TUN_WEIGHT",   nullptr,                                    nullptr}  // Extension tag
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_EQUIPMENT_RECORD_FIELDS {
      // Type              XPath                        Q_PROPERTY                                       Enum Mapper
      {XmlRecord::String,  "NAME",                      PropertyNames::NamedEntity::name,                nullptr},
      {XmlRecord::UInt,    "VERSION",                   nullptr,                                         nullptr},
      {XmlRecord::Double,  "BOIL_SIZE",                 PropertyNames::Equipment::boilSize_l,            nullptr},
      {XmlRecord::Double,  "BATCH_SIZE",                PropertyNames::Equipment::batchSize_l,           nullptr},
      {XmlRecord::Double,  "TUN_VOLUME",                PropertyNames::Equipment::tunVolume_l,           nullptr},
      {XmlRecord::Double,  "TUN_WEIGHT",                PropertyNames::Equipment::tunWeight_kg,          nullptr},
      {XmlRecord::Double,  "TUN_SPECIFIC_HEAT",         PropertyNames::Equipment::tunSpecificHeat_calGC, nullptr},
      {XmlRecord::Double,  "TOP_UP_WATER",              PropertyNames::Equipment::topUpWater_l,          nullptr},
      {XmlRecord::Double,  "TRUB_CHILLER_LOSS",         PropertyNames::Equipment::trubChillerLoss_l,     nullptr},
      {XmlRecord::Double,  "EVAP_RATE",                 PropertyNames::Equipment::evapRate_pctHr,        nullptr},
      {XmlRecord::Double,  "BOIL_TIME",                 PropertyNames::Equipment::boilTime_min,          nullptr},
      {XmlRecord::Bool,    "CALC_BOIL_VOLUME",          PropertyNames::Equipment::calcBoilVolume,        nullptr},
      {XmlRecord::Double,  "LAUTER_DEADSPACE",          PropertyNames::Equipment::lauterDeadspace_l,     nullptr},
      {XmlRecord::Double,  "TOP_UP_KETTLE",             PropertyNames::Equipment::topUpKettle_l,         nullptr},
      {XmlRecord::Double,  "HOP_UTILIZATION",           PropertyNames::Equipment::hopUtilization_pct,    nullptr},
      {XmlRecord::String,  "NOTES",                     PropertyNames::Equipment::notes,                 nullptr},
      {XmlRecord::String,  "DISPLAY_BOIL_SIZE",         nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_BATCH_SIZE",        nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TUN_VOLUME",        nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TUN_WEIGHT",        nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TOP_UP_WATER",      nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TRUB_CHILLER_LOSS", nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_LAUTER_DEADSPACE",  nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TOP_UP_KETTLE",     nullptr,                                         nullptr}, // Extension tag
      {XmlRecord::Double,  "REAL_EVAP_RATE",            PropertyNames::Equipment::evapRate_lHr,          nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
      {XmlRecord::Double,  "ABSORPTION",                PropertyNames::Equipment::grainAbsorption_LKg,   nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
      {XmlRecord::Double,  "BOILING_POINT",             PropertyNames::Equipment::boilingPoint_c,        nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <INSTRUCTION>...</INSTRUCTION> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_INSTRUCTION_RECORD_FIELDS {
      // Type              XPath         Q_PROPERTY                              Enum Mapper
      {XmlRecord::String,  "NAME",       PropertyNames::NamedEntity::name,       nullptr},
      {XmlRecord::UInt,    "VERSION",    nullptr,                                nullptr},
      {XmlRecord::String,  "directions", PropertyNames::Instruction::directions, nullptr},
      {XmlRecord::Bool,    "hasTimer",   PropertyNames::Instruction::hasTimer,   nullptr},
      {XmlRecord::String,  "timervalue", PropertyNames::Instruction::timerValue, nullptr}, // NB XPath is lowercase and property is camelCase
      {XmlRecord::Bool,    "completed",  PropertyNames::Instruction::completed,  nullptr},
      {XmlRecord::Double,  "interval",   PropertyNames::Instruction::interval,   nullptr}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <BREWNOTE>...</BREWNOTE> BeerXML records
   // NB There is no NAME field on a BREWNOTE
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::FieldDefinitions const BEER_XML_BREWNOTE_RECORD_FIELDS {
      // Type              XPath                      Q_PROPERTY                                     Enum Mapper
      {XmlRecord::UInt,    "VERSION",                 nullptr,                                       nullptr},
      {XmlRecord::Date,    "BREWDATE",                PropertyNames::BrewNote::brewDate,             nullptr},
      {XmlRecord::Date,    "DATE_FERMENTED_OUT",      PropertyNames::BrewNote::fermentDate,          nullptr},
      {XmlRecord::String,  "NOTES",                   PropertyNames::BrewNote::notes,                nullptr},
      {XmlRecord::Double,  "SG",                      PropertyNames::BrewNote::sg,                   nullptr},
      {XmlRecord::Double,  "ACTUAL_ABV",              PropertyNames::BrewNote::abv,                  nullptr},
      {XmlRecord::Double,  "EFF_INTO_BK",             PropertyNames::BrewNote::effIntoBK_pct,        nullptr},
      {XmlRecord::Double,  "BREWHOUSE_EFF",           PropertyNames::BrewNote::brewhouseEff_pct,     nullptr},
      {XmlRecord::Double,  "VOLUME_INTO_BK",          PropertyNames::BrewNote::volumeIntoBK_l,       nullptr},
      {XmlRecord::Double,  "STRIKE_TEMP",             PropertyNames::BrewNote::strikeTemp_c,         nullptr},
      {XmlRecord::Double,  "MASH_FINAL_TEMP",         PropertyNames::BrewNote::mashFinTemp_c,        nullptr},
      {XmlRecord::Double,  "OG",                      PropertyNames::BrewNote::og,                   nullptr},
      {XmlRecord::Double,  "POST_BOIL_VOLUME",        PropertyNames::BrewNote::postBoilVolume_l,     nullptr},
      {XmlRecord::Double,  "VOLUME_INTO_FERMENTER",   PropertyNames::BrewNote::volumeIntoFerm_l,     nullptr},
      {XmlRecord::Double,  "PITCH_TEMP",              PropertyNames::BrewNote::pitchTemp_c,          nullptr},
      {XmlRecord::Double,  "FG",                      PropertyNames::BrewNote::fg,                   nullptr},
      {XmlRecord::Double,  "ATTENUATION",             PropertyNames::BrewNote::attenuation,          nullptr},
      {XmlRecord::Double,  "FINAL_VOLUME",            PropertyNames::BrewNote::finalVolume_l,        nullptr},
      {XmlRecord::Double,  "BOIL_OFF",                PropertyNames::BrewNote::boilOff_l,            nullptr},
      {XmlRecord::Double,  "PROJECTED_BOIL_GRAV",     PropertyNames::BrewNote::projBoilGrav,         nullptr},
      {XmlRecord::Double,  "PROJECTED_VOL_INTO_BK",   PropertyNames::BrewNote::projVolIntoBK_l,      nullptr},
      {XmlRecord::Double,  "PROJECTED_STRIKE_TEMP",   PropertyNames::BrewNote::projStrikeTemp_c,     nullptr},
      {XmlRecord::Double,  "PROJECTED_MASH_FIN_TEMP", PropertyNames::BrewNote::projMashFinTemp_c,    nullptr},
      {XmlRecord::Double,  "PROJECTED_OG",            PropertyNames::BrewNote::projOg,               nullptr},
      {XmlRecord::Double,  "PROJECTED_VOL_INTO_FERM", PropertyNames::BrewNote::projVolIntoFerm_l,    nullptr},
      {XmlRecord::Double,  "PROJECTED_FG",            PropertyNames::BrewNote::projFg,               nullptr},
      {XmlRecord::Double,  "PROJECTED_EFF",           PropertyNames::BrewNote::projEff_pct,          nullptr},
      {XmlRecord::Double,  "PROJECTED_ABV",           PropertyNames::BrewNote::projABV_pct,          nullptr},
      {XmlRecord::Double,  "PROJECTED_POINTS",        PropertyNames::BrewNote::projPoints,           nullptr},
      {XmlRecord::Double,  "PROJECTED_FERM_POINTS",   PropertyNames::BrewNote::projFermPoints,       nullptr},
      {XmlRecord::Double,  "PROJECTED_ATTEN",         PropertyNames::BrewNote::projAtten,            nullptr}
   };

   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // Field mappings for <RECIPE>...</RECIPE> BeerXML records
   ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   XmlRecord::EnumLookupMap const BEER_XML_RECIPE_STEP_TYPE_MAPPER {
      {"Extract",      Recipe::Extract},
      {"Partial Mash", Recipe::PartialMash},
      {"All Grain",    Recipe::AllGrain}
   };
   XmlRecord::FieldDefinitions const BEER_XML_RECIPE_RECORD_FIELDS {
      // Type               XPath                      Q_PROPERTY                                 Enum Mapper
      {XmlRecord::String,  "NAME",                     PropertyNames::NamedEntity::name,          nullptr},
      {XmlRecord::UInt,    "VERSION",                  nullptr,                                   nullptr},
      {XmlRecord::Enum,    "TYPE",                     PropertyNames::Recipe::recipeType,         &BEER_XML_RECIPE_STEP_TYPE_MAPPER},
      {XmlRecord::Record,  "STYLE",                    PropertyNames::Recipe::style,              nullptr},
      {XmlRecord::Record,  "EQUIPMENT",                PropertyNames::Recipe::equipment,          nullptr},
      {XmlRecord::String,  "BREWER",                   PropertyNames::Recipe::brewer,             nullptr},
      {XmlRecord::String,  "ASST_BREWER",              PropertyNames::Recipe::asstBrewer,         nullptr},
      {XmlRecord::Double,  "BATCH_SIZE",               PropertyNames::Recipe::batchSize_l,        nullptr},
      {XmlRecord::Double,  "BOIL_SIZE",                PropertyNames::Recipe::boilSize_l,         nullptr},
      {XmlRecord::Double,  "BOIL_TIME",                PropertyNames::Recipe::boilTime_min,       nullptr},
      {XmlRecord::Double,  "EFFICIENCY",               PropertyNames::Recipe::efficiency_pct,     nullptr},
      {XmlRecord::Record,  "HOPS/HOP",                 nullptr,                                   nullptr}, // Additional logic for "HOPS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::Record,  "FERMENTABLES/FERMENTABLE", nullptr,                                   nullptr}, // Additional logic for "FERMENTABLES" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::Record,  "MISCS/MISC",               nullptr,                                   nullptr}, // Additional logic for "MISCS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::Record,  "YEASTS/YEAST",             nullptr,                                   nullptr}, // Additional logic for "YEASTS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::Record,  "WATERS/WATER",             nullptr,                                   nullptr}, // Additional logic for "WATERS" is handled in xml/XmlRecipeRecord.cpp
      {XmlRecord::Record,  "MASH",                     PropertyNames::Recipe::mash,               nullptr},
      {XmlRecord::Record,  "INSTRUCTIONS/INSTRUCTION", nullptr,                                   nullptr}, // Additional logic for "INSTRUCTIONS" is handled in xml/XmlNamedEntityRecord.h
      {XmlRecord::Record,  "BREWNOTES/BREWNOTE",       nullptr,                                   nullptr}, // Additional logic for "BREWNOTES" is handled in xml/XmlNamedEntityRecord.h
      {XmlRecord::String,  "NOTES",                    PropertyNames::Recipe::notes,              nullptr},
      {XmlRecord::String,  "TASTE_NOTES",              PropertyNames::Recipe::tasteNotes,         nullptr},
      {XmlRecord::Double,  "TASTE_RATING",             PropertyNames::Recipe::tasteRating,        nullptr},
      {XmlRecord::Double,  "OG",                       PropertyNames::Recipe::og,                 nullptr},
      {XmlRecord::Double,  "FG",                       PropertyNames::Recipe::fg,                 nullptr},
      {XmlRecord::UInt,    "FERMENTATION_STAGES",      PropertyNames::Recipe::fermentationStages, nullptr},
      {XmlRecord::Double,  "PRIMARY_AGE",              PropertyNames::Recipe::primaryAge_days,    nullptr},
      {XmlRecord::Double,  "PRIMARY_TEMP",             PropertyNames::Recipe::primaryTemp_c,      nullptr},
      {XmlRecord::Double,  "SECONDARY_AGE",            PropertyNames::Recipe::secondaryAge_days,  nullptr},
      {XmlRecord::Double,  "SECONDARY_TEMP",           PropertyNames::Recipe::secondaryTemp_c,    nullptr},
      {XmlRecord::Double,  "TERTIARY_AGE",             PropertyNames::Recipe::tertiaryAge_days,   nullptr},
      {XmlRecord::Double,  "TERTIARY_TEMP",            PropertyNames::Recipe::tertiaryTemp_c,     nullptr},
      {XmlRecord::Double,  "AGE",                      PropertyNames::Recipe::age,                nullptr},
      {XmlRecord::Double,  "AGE_TEMP",                 PropertyNames::Recipe::ageTemp_c,          nullptr},
      {XmlRecord::Date,    "DATE",                     PropertyNames::Recipe::date,               nullptr},
      {XmlRecord::Double,  "CARBONATION",              PropertyNames::Recipe::carbonation_vols,   nullptr},
      {XmlRecord::Bool,    "FORCED_CARBONATION",       PropertyNames::Recipe::forcedCarbonation,  nullptr},
      {XmlRecord::String,  "PRIMING_SUGAR_NAME",       PropertyNames::Recipe::primingSugarName,   nullptr},
      {XmlRecord::Double,  "CARBONATION_TEMP",         PropertyNames::Recipe::carbonationTemp_c,  nullptr},
      {XmlRecord::Double,  "PRIMING_SUGAR_EQUIV",      PropertyNames::Recipe::primingSugarEquiv,  nullptr},
      {XmlRecord::Double,  "KEG_PRIMING_FACTOR",       PropertyNames::Recipe::kegPrimingFactor,   nullptr},
      {XmlRecord::String,  "EST_OG",                   nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "EST_FG",                   nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "EST_COLOR",                nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "IBU",                      nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "IBU_METHOD",               nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "EST_ABV",                  nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "ABV",                      nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "ACTUAL_EFFICIENCY",        nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "CALORIES",                 nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_BATCH_SIZE",       nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_BOIL_SIZE",        nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_OG",               nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_FG",               nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_PRIMARY_TEMP",     nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_SECONDARY_TEMP",   nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_TERTIARY_TEMP",    nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_AGE_TEMP",         nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "CARBONATION_USED",         nullptr,                                   nullptr}, // Extension tag
      {XmlRecord::String,  "DISPLAY_CARB_TEMP",        nullptr,                                   nullptr} // Extension tag
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
               ":/xsd/beerxml/v1/BeerXml.xsd",
               QHash<QString, XmlCoding::XmlRecordDefinition>{
                  {"BEER_XML",    {&XmlCoding::construct<void>,        &BEER_XML_ROOT_RECORD_FIELDS} },
                  {"HOP",         {&XmlCoding::construct<Hop>,         &BEER_XML_HOP_RECORD_FIELDS} },
                  {"FERMENTABLE", {&XmlCoding::construct<Fermentable>, &BEER_XML_FERMENTABLE_RECORD_FIELDS} },
                  {"YEAST",       {&XmlCoding::construct<Yeast>,       &BEER_XML_YEAST_RECORD_FIELDS} },
                  {"MISC",        {&XmlCoding::construct<Misc>,        &BEER_XML_MISC_RECORD_FIELDS} },
                  {"WATER",       {&XmlCoding::construct<Water>,       &BEER_XML_WATER_RECORD_FIELDS} },
                  {"STYLE",       {&XmlCoding::construct<Style>,       &BEER_XML_STYLE_RECORD_FIELDS} },
                  {"MASH_STEP",   {&XmlCoding::construct<MashStep>,    &BEER_XML_MASH_STEP_RECORD_FIELDS} },
                  {"MASH",        {&XmlCoding::construct<Mash>,        &BEER_XML_MASH_RECORD_FIELDS} },
                  {"EQUIPMENT",   {&XmlCoding::construct<Equipment>,   &BEER_XML_EQUIPMENT_RECORD_FIELDS} },
                  {"INSTRUCTION", {&XmlCoding::construct<Instruction>, &BEER_XML_INSTRUCTION_RECORD_FIELDS} },
                  {"BREWNOTE",    {&XmlCoding::construct<BrewNote>,    &BEER_XML_BREWNOTE_RECORD_FIELDS} },
                  {"RECIPE",      {&XmlCoding::construct<Recipe>,      &BEER_XML_RECIPE_RECORD_FIELDS} }
               }
            } {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;


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

      if(! inputFile.open(QIODevice::ReadOnly)) {
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
      //       any UTF-8 encoded XML document.  (This is perhaps because it seems to mandate an ISO-8859-1 coding of
      //       BeerXML files, though there is no explicit discussion of file encodings in the standard, and this seems
      //       an unnecessary constraint to place on files.)
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


BeerXML::BeerXML(DatabaseSchema* tables) : QObject(),
                                           pimpl{ new impl{} },
                                           m_tables(tables) {
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
BeerXML::~BeerXML() = default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

QString BeerXML::textFromValue(QVariant value, QString type)
{
   QString retval = value.toString();

   if (type == "boolean" )
      retval = NamedEntity::text(value.toBool());
   else if (type == "real")
      retval = NamedEntity::text(value.toDouble());
   else if (type == "timestamp")
      retval = NamedEntity::text(value.toDate());

   return retval;
}

void BeerXML::toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::BREWNOTETABLE);

   node = doc.createElement("BREWNOTE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Equipment* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::EQUIPTABLE);

   node = doc.createElement("EQUIPMENT");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::FERMTABLE);

   node = doc.createElement("FERMENTABLE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Hop* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   node = doc.createElement("HOP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Instruction* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::INSTRUCTIONTABLE);

   node = doc.createElement("INSTRUCTION");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Mash* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   int i, size;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHTABLE);

   node = doc.createElement("MASH");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   tmpElement = doc.createElement("MASH_STEPS");
   QList<MashStep*> mashSteps = a->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
      toXml( mashSteps[i], doc, tmpElement);
   node.appendChild(tmpElement);

   parent.appendChild(node);

}

void BeerXML::toXml( MashStep* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHSTEPTABLE);

   node = doc.createElement("MASH_STEP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         QString val;
         // flySparge and batchSparge aren't part of the BeerXML spec.
         // prop makes sure we give BeerXML something it understands.
         if ( element == PropertyNames::MashStep::type ) {
            val = a->isSparge() ? MashStep::types[0] : a->typeString();
         }
         else {
            val = textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element));
         }
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(val);
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Misc* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);

   node = doc.createElement("MISC");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Recipe* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::RECTABLE);

   int i;

   node = doc.createElement("RECIPE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   Style* style = a->style();
   if( style != nullptr )
      toXml( style, doc, node);

   tmpElement = doc.createElement("HOPS");
   QList<Hop*> hops = a->hops();
   for( i = 0; i < hops.size(); ++i )
      toXml( hops[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("FERMENTABLES");
   QList<Fermentable*> ferms = a->fermentables();
   for( i = 0; i < ferms.size(); ++i )
      toXml( ferms[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("MISCS");
   QList<Misc*> miscs = a->miscs();
   for( i = 0; i < miscs.size(); ++i )
      toXml( miscs[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("YEASTS");
   QList<Yeast*> yeasts = a->yeasts();
   for( i = 0; i < yeasts.size(); ++i )
      toXml( yeasts[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("WATERS");
   QList<Water*> waters = a->waters();
   for( i = 0; i < waters.size(); ++i )
      toXml( waters[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Mash* mash = a->mash();
   if( mash != nullptr )
      toXml( mash, doc, node);

   tmpElement = doc.createElement("INSTRUCTIONS");
   QList<Instruction*> instructions = a->instructions();
   for( i = 0; i < instructions.size(); ++i )
      toXml( instructions[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("BREWNOTES");
   QList<BrewNote*> brewNotes = a->brewNotes();
   for(i=0; i < brewNotes.size(); ++i)
      toXml(brewNotes[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Equipment* equip = a->equipment();
   if( equip )
      toXml( equip, doc, node);

   parent.appendChild(node);
}

void BeerXML::toXml( Style* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::STYLETABLE);

   node = doc.createElement("STYLE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Water* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::WATERTABLE);

   node = doc.createElement("WATER");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);


}

void BeerXML::toXml( Yeast* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::YEASTTABLE);

   node = doc.createElement("YEAST");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(NamedEntity::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allProperties()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString prop = tbl->propertyName(element);
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(prop.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

// fromXml ====================================================================
bool BeerXML::importFromXML(QString const & filename, QTextStream & userMessage) {
   return this->pimpl->validateAndLoad(filename, userMessage);
}
