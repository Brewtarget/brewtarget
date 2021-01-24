/*
 * beerxml.cpp is part of Brewtarget, and is Copyright the following
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

#include "beerxml.h"

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
#include "brewnote.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "instruction.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "salt.h"
#include "yeast.h"
#include "xml/BtDomErrorHandler.h"
#include "xml/XmlMashRecord.h"
#include "xml/XmlMashStepRecord.h"
#include "xml/XmlNamedEntityRecord.h"
#include "xml/XmlRecipeRecord.h"
#include "xml/XmlCoding.h"
#include "xml/XmlRecord.h"
#include "xml/XQString.h"

#include "TableSchema.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Top-level field mappings for BeerXML files
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_ROOT_RECORD_FIELDS {
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
static XmlRecord::EnumLookupMap const BEER_XML_HOP_USE_MAPPER {
   {"Boil",       Hop::Boil},
   {"Dry Hop",    Hop::Dry_Hop},
   {"Mash",       Hop::Mash},
   {"First Wort", Hop::First_Wort},
   {"Aroma",      Hop::UseAroma}
};
static XmlRecord::EnumLookupMap const BEER_XML_HOP_TYPE_MAPPER {
   {"Bittering", Hop::Bittering},
   {"Aroma",     Hop::Aroma},
   {"Both",      Hop::Both}
};
static XmlRecord::EnumLookupMap const BEER_XML_HOP_FORM_MAPPER {
   {"Pellet", Hop::Pellet},
   {"Plug",   Hop::Plug},
   {"Leaf",   Hop::Leaf}
};
static XmlRecord::FieldDefinitions const BEER_XML_HOP_RECORD_FIELDS {
   // Type              XPath             Q_PROPERTY           Enum Mapper
   {XmlRecord::String,  "NAME",           "name",              nullptr},
   {XmlRecord::UInt,    "VERSION",        nullptr,             nullptr},
   {XmlRecord::Double,  "ALPHA",          "alpha_pct",         nullptr},
   {XmlRecord::Double,  "AMOUNT",         "amount_kg",         nullptr},
   {XmlRecord::Enum,    "USE",            "use",               &BEER_XML_HOP_USE_MAPPER},
   {XmlRecord::Double,  "TIME",           "time_min",          nullptr},
   {XmlRecord::String,  "NOTES",          "notes",             nullptr},
   {XmlRecord::Enum,    "TYPE",           "type",              &BEER_XML_HOP_TYPE_MAPPER},
   {XmlRecord::Enum,    "FORM",           "form",              &BEER_XML_HOP_FORM_MAPPER},
   {XmlRecord::Double,  "BETA",           "beta_pct",          nullptr},
   {XmlRecord::Double,  "HSI",            "hsi_pct",           nullptr},
   {XmlRecord::String,  "ORIGIN",         "origin",            nullptr},
   {XmlRecord::String,  "SUBSTITUTES",    "substitutes",       nullptr},
   {XmlRecord::Double,  "HUMULENE",       "humulene_pct",      nullptr},
   {XmlRecord::Double,  "CARYOPHYLLENE",  "caryophyllene_pct", nullptr},
   {XmlRecord::Double,  "COHUMULONE",     "cohumulone_pct",    nullptr},
   {XmlRecord::Double,  "MYRCENE",        "myrcene_pct",       nullptr},
   {XmlRecord::String,  "DISPLAY_AMOUNT", nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "INVENTORY",      nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TIME",   nullptr,             nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_FERMENTABLE_TYPE_MAPPER {
   {"Grain",       Fermentable::Grain},
   {"Sugar",       Fermentable::Sugar},
   {"Extract",     Fermentable::Extract},
   {"Dry Extract", Fermentable::Dry_Extract},
   {"Adjunct",     Fermentable::Adjunct}
};
static XmlRecord::FieldDefinitions const BEER_XML_FERMENTABLE_RECORD_FIELDS {
   // Type              XPath               Q_PROPERTY                Enum Mapper
   {XmlRecord::String,  "NAME",             "name",                   nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,                  nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",                   &BEER_XML_FERMENTABLE_TYPE_MAPPER},
   {XmlRecord::Double,  "AMOUNT",           "amount_kg",              nullptr},
   {XmlRecord::Double,  "YIELD",            "yield_pct",              nullptr},
   {XmlRecord::Double,  "COLOR",            "color_srm",              nullptr},
   {XmlRecord::Bool,    "ADD_AFTER_BOIL",   "addAfterBoil",           nullptr},
   {XmlRecord::String,  "ORIGIN",           "origin",                 nullptr},
   {XmlRecord::String,  "SUPPLIER",         "supplier",               nullptr},
   {XmlRecord::String,  "NOTES",            "notes",                  nullptr},
   {XmlRecord::Double,  "COARSE_FINE_DIFF", "coarseFineDiff_pct",     nullptr},
   {XmlRecord::Double,  "MOISTURE",         "moisture_pct",           nullptr},
   {XmlRecord::Double,  "DIASTATIC_POWER",  "diastaticPower_lintner", nullptr},
   {XmlRecord::Double,  "PROTEIN",          "protein_pct",            nullptr},
   {XmlRecord::Double,  "MAX_IN_BATCH",     "maxInBatch_pct",         nullptr},
   {XmlRecord::Bool,    "RECOMMEND_MASH",   "recommendMash",          nullptr},
   {XmlRecord::Double,  "IBU_GAL_PER_LB",   "ibuGalPerLb",            nullptr},
   {XmlRecord::String,  "DISPLAY_AMOUNT",   nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "POTENTIAL",        nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "INVENTORY",        nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_COLOR",    nullptr,                  nullptr}, // Extension tag
   {XmlRecord::Bool,    "IS_MASHED",        "isMashed",               nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <YEAST>...</YEAST> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_YEAST_TYPE_MAPPER {
   {"Ale",       Yeast::Ale},
   {"Lager",     Yeast::Lager},
   {"Wheat",     Yeast::Wheat},
   {"Wine",      Yeast::Wine},
   {"Champagne", Yeast::Champagne}
};
static XmlRecord::EnumLookupMap const BEER_XML_YEAST_FORM_MAPPER {
   {"Liquid",  Yeast::Liquid},
   {"Dry",     Yeast::Dry},
   {"Slant",   Yeast::Slant},
   {"Culture", Yeast::Culture}
};
static XmlRecord::EnumLookupMap const BEER_XML_YEAST_FLOCCULATION_MAPPER {
   {"Low",       Yeast::Low},
   {"Medium",    Yeast::Medium},
   {"High",      Yeast::High},
   {"Very High", Yeast::Very_High}
};
static XmlRecord::FieldDefinitions const BEER_XML_YEAST_RECORD_FIELDS {
   // Type              XPath               Q_PROPERTY                Enum Mapper
   {XmlRecord::String,  "NAME",             "name",                   nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,                  nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",                   &BEER_XML_YEAST_TYPE_MAPPER},
   {XmlRecord::Enum,    "FORM",             "type",                   &BEER_XML_YEAST_FORM_MAPPER},
   {XmlRecord::Double,  "AMOUNT",           "amount",                 nullptr},
   {XmlRecord::Bool,    "AMOUNT_IS_WEIGHT", "amountIsWeight",         nullptr},
   {XmlRecord::String,  "LABORATORY",       "laboratory",             nullptr},
   {XmlRecord::String,  "PRODUCT_ID",       "productID",              nullptr},
   {XmlRecord::Double,  "MIN_TEMPERATURE",  "minTemperature_c",       nullptr},
   {XmlRecord::Double,  "MAX_TEMPERATURE",  "maxTemperature_c",       nullptr},
   {XmlRecord::Enum,    "FLOCCULATION",     "flocculation",           &BEER_XML_YEAST_FLOCCULATION_MAPPER},
   {XmlRecord::Double,  "ATTENUATION",      "attenuation_pct",        nullptr},
   {XmlRecord::String,  "NOTES",            "notes",                  nullptr},
   {XmlRecord::String,  "BEST_FOR",         "bestFor",                nullptr},
   {XmlRecord::Int,     "TIMES_CULTURED",   "timesCultured",          nullptr},
   {XmlRecord::Int,     "MAX_REUSE",        "maxReuse",               nullptr},
   {XmlRecord::Bool,    "ADD_TO_SECONDARY", "addToSecondary",         nullptr},
   {XmlRecord::String,  "DISPLAY_AMOUNT",   nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "DISP_MIN_TEMP",    nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "DISP_MAX_TEMP",    nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "INVENTORY",        nullptr,                  nullptr}, // Extension tag
   {XmlRecord::String,  "CULTURE_DATE",     nullptr,                  nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MISC>...</MISC> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_MISC_TYPE_MAPPER {
   {"Spice",       Misc::Spice},
   {"Fining",      Misc::Fining},
   {"Water Agent", Misc::Water_Agent},
   {"Herb",        Misc::Herb},
   {"Flavor",      Misc::Flavor},
   {"Other",       Misc::Other}
};
static XmlRecord::EnumLookupMap const BEER_XML_MISC_USE_MAPPER {
   {"Boil",      Misc::Boil},
   {"Mash",      Misc::Mash},
   {"Primary",   Misc::Primary},
   {"Secondary", Misc::Secondary},
   {"Bottling",  Misc::Bottling}
};
static XmlRecord::FieldDefinitions const BEER_XML_MISC_RECORD_FIELDS {
   // Type              XPath               Q_PROPERTY        Enum Mapper
   {XmlRecord::String,  "NAME",             "name",           nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,          nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",           &BEER_XML_MISC_TYPE_MAPPER},
   {XmlRecord::Enum,    "USE",              "use",            &BEER_XML_MISC_USE_MAPPER},
   {XmlRecord::Double,  "TIME",             "time",           nullptr},
   {XmlRecord::Double,  "AMOUNT",           "amount",         nullptr},
   {XmlRecord::Bool,    "AMOUNT_IS_WEIGHT", "amountIsWeight", nullptr},
   {XmlRecord::String,  "USE_FOR",          "useFor",         nullptr},
   {XmlRecord::String,  "NOTES",            "notes",          nullptr},
   {XmlRecord::String,  "DISPLAY_AMOUNT", nullptr,            nullptr}, // Extension tag
   {XmlRecord::String,  "INVENTORY",      nullptr,            nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TIME",   nullptr,            nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <WATER>...</WATER> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_WATER_RECORD_FIELDS {
   // Type               XPath           Q_PROPERTY         Enum Mapper
   {XmlRecord::String,  "NAME",          "name",            nullptr},
   {XmlRecord::UInt,    "VERSION",       nullptr,           nullptr},
   {XmlRecord::Double,  "AMOUNT",        "amount",          nullptr},
   {XmlRecord::Double,  "CALCIUM",       "calcium_ppm",     nullptr},
   {XmlRecord::Double,  "BICARBONATE",   "bicarbonate_ppm", nullptr},
   {XmlRecord::Double,  "SULFATE",       "sulfate_ppm",     nullptr},
   {XmlRecord::Double,  "CHLORIDE",      "chloride_ppm",    nullptr},
   {XmlRecord::Double,  "SODIUM",        "sodium_ppm",      nullptr},
   {XmlRecord::Double,  "MAGNESIUM",     "magnesium_ppm",   nullptr},
   {XmlRecord::Double,  "PH",            "ph",              nullptr},
   {XmlRecord::String,  "NOTES",         "notes",           nullptr},
   {XmlRecord::String,  "DISPLAY_AMOUNT", nullptr,          nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <STYLE>...</STYLE> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_STYLE_TYPE_MAPPER {
   {"Lager", Style::Lager},
   {"Ale",   Style::Ale},
   {"Mead",  Style::Mead},
   {"Wheat", Style::Wheat},
   {"Mixed", Style::Mixed},
   {"Cider", Style::Cider}
};
static XmlRecord::FieldDefinitions const BEER_XML_STYLE_RECORD_FIELDS {
   // Type              XPath                Q_PROPERTY        Enum Mapper
   {XmlRecord::String,  "NAME",              "name",           nullptr},
   {XmlRecord::String,  "CATEGORY",          "category",       nullptr},
   {XmlRecord::UInt,    "VERSION",           nullptr,          nullptr},
   {XmlRecord::String,  "CATEGORY_NUMBER",   "categoryNumber", nullptr},
   {XmlRecord::String,  "STYLE_LETTER",      "styleLetter",    nullptr},
   {XmlRecord::String,  "STYLE_GUIDE",       "styleGuide",     nullptr},
   {XmlRecord::Enum,    "TYPE",              "type",           &BEER_XML_STYLE_TYPE_MAPPER},
   {XmlRecord::Double,  "OG_MIN",            "ogMin",          nullptr},
   {XmlRecord::Double,  "OG_MAX",            "ogMax",          nullptr},
   {XmlRecord::Double,  "FG_MIN",            "fgMin",          nullptr},
   {XmlRecord::Double,  "FG_MAX",            "fgMax",          nullptr},
   {XmlRecord::Double,  "IBU_MIN",           "ibuMin",         nullptr},
   {XmlRecord::Double,  "IBU_MAX",           "ibuMax",         nullptr},
   {XmlRecord::Double,  "COLOR_MIN",         "colorMin_srm",   nullptr},
   {XmlRecord::Double,  "COLOR_MAX",         "colorMax_srm",   nullptr},
   {XmlRecord::Double,  "CARB_MIN",          "carbMin_vol",    nullptr},
   {XmlRecord::Double,  "CARB_MAX",          "carbMax_vol",    nullptr},
   {XmlRecord::Double,  "ABV_MIN",           "abvMin_pct",     nullptr},
   {XmlRecord::Double,  "ABV_MAX",           "abvMax_pct",     nullptr},
   {XmlRecord::String,  "NOTES",             "notes",          nullptr},
   {XmlRecord::String,  "PROFILE",           "profile",        nullptr},
   {XmlRecord::String,  "INGREDIENTS",       "ingredients",    nullptr},
   {XmlRecord::String,  "EXAMPLES",          "examples",       nullptr},
   {XmlRecord::String,  "DISPLAY_OG_MIN",    nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_OG_MAX",    nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_FG_MIN",    nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_FG_MAX",    nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_COLOR_MIN", nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_COLOR_MAX", nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "OG_RANGE",          nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "FG_RANGE",          nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "IBU_RANGE",         nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "CARB_RANGE",        nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "COLOR_RANGE",       nullptr,          nullptr}, // Extension tag
   {XmlRecord::String,  "ABV_RANGE",         nullptr,          nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MASH_STEP>...</MASH_STEP> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_MASH_STEP_TYPE_MAPPER {
   {"Infusion",     MashStep::Infusion},
   {"Temperature",  MashStep::Temperature},
   {"Decoction",    MashStep::Decoction}
   // Inside Brewtarget we also have MashStep::flySparge and MashStep::batchSparge which are not mentioned in the
   // BeerXML 1.0 Standard.  They get treated as "Infusion" when we write to BeerXML
};
static XmlRecord::FieldDefinitions const BEER_XML_MASH_STEP_RECORD_FIELDS {
   // Type              XPath                 Q_PROPERTY           Enum Mapper
   {XmlRecord::String,  "NAME",               "name",              nullptr},
   {XmlRecord::UInt,    "VERSION",            nullptr,             nullptr},
   {XmlRecord::Enum,    "TYPE",               "type",              &BEER_XML_MASH_STEP_TYPE_MAPPER},
   {XmlRecord::Double,  "INFUSE_AMOUNT",      "infuseAmount_l",    nullptr}, // Should not be supplied if TYPE is "Decoction"
   {XmlRecord::Double,  "STEP_TEMP",          "stepTemp_c",        nullptr},
   {XmlRecord::Double,  "STEP_TIME",          "stepTime_min",      nullptr},
   {XmlRecord::Double,  "RAMP_TIME",          "rampTime_min",      nullptr},
   {XmlRecord::Double,  "END_TEMP",           "endTemp_c",         nullptr},
   {XmlRecord::String,  "DESCRIPTION",        nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "WATER_GRAIN_RATIO",  nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DECOCTION_AMT",      nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "INFUSE_TEMP",        nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_STEP_TEMP",  nullptr,             nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_INFUSE_AMT", nullptr,             nullptr}, // Extension tag
   {XmlRecord::Double,  "DECOCTION_AMOUNT",   "decoctionAmount_l", nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MASH>...</MASH> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_MASH_RECORD_FIELDS {
   // Type               XPath                  Q_PROPERTY               Enum Mapper
   {XmlRecord::String,  "NAME",                 "name",                  nullptr},
   {XmlRecord::UInt,    "VERSION",              nullptr,                 nullptr},
   {XmlRecord::Double,  "GRAIN_TEMP",           "grainTemp_c",           nullptr},
   {XmlRecord::Record,  "MASH_STEPS/MASH_STEP", nullptr,                 nullptr}, // Additional logic for "MASH-STEPS" is handled in code
   {XmlRecord::String,  "NOTES",                "notes",                 nullptr},
   {XmlRecord::Double,  "TUN_TEMP",             "tunTemp_c",             nullptr},
   {XmlRecord::Double,  "SPARGE_TEMP",          "spargeTemp_c",          nullptr},
   {XmlRecord::Double,  "PH",                   "ph",                    nullptr},
   {XmlRecord::Double,  "TUN_WEIGHT",           "tunWeight_kg",          nullptr},
   {XmlRecord::Double,  "TUN_SPECIFIC_HEAT",    "tunSpecificHeat_calGC", nullptr},
   {XmlRecord::Bool,    "EQUIP_ADJUST",         "equipAdjust",           nullptr},
   {XmlRecord::String,  "DISPLAY_GRAIN_TEMP",   nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TUN_TEMP",     nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_SPARGE_TEMP",  nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TUN_WEIGHT",   nullptr,                 nullptr}  // Extension tag
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_EQUIPMENT_RECORD_FIELDS {
   // Type              XPath                        Q_PROPERTY               Enum Mapper
   {XmlRecord::String,  "NAME",                      "name",                  nullptr},
   {XmlRecord::UInt,    "VERSION",                   nullptr,                 nullptr},
   {XmlRecord::Double,  "BOIL_SIZE",                 "boilSize_l",            nullptr},
   {XmlRecord::Double,  "BATCH_SIZE",                "batchSize_l",           nullptr},
   {XmlRecord::Double,  "TUN_VOLUME",                "tunVolume_l",           nullptr},
   {XmlRecord::Double,  "TUN_WEIGHT",                "tunWeight_kg",          nullptr},
   {XmlRecord::Double,  "TUN_SPECIFIC_HEAT",         "tunSpecificHeat_calGC", nullptr},
   {XmlRecord::Double,  "TOP_UP_WATER",              "topUpWater_l",          nullptr},
   {XmlRecord::Double,  "TRUB_CHILLER_LOSS",         "trubChillerLoss_l",     nullptr},
   {XmlRecord::Double,  "EVAP_RATE",                 "evapRate_pctHr",        nullptr},
   {XmlRecord::Double,  "BOIL_TIME",                 "boilTime_min",          nullptr},
   {XmlRecord::Bool,    "CALC_BOIL_VOLUME",          "calcBoilVolume",        nullptr},
   {XmlRecord::Double,  "LAUTER_DEADSPACE",          "lauterDeadspace_l",     nullptr},
   {XmlRecord::Double,  "TOP_UP_KETTLE",             "topUpKettle_l",         nullptr},
   {XmlRecord::Double,  "HOP_UTILIZATION",           "hopUtilization_pct",    nullptr},
   {XmlRecord::String,  "NOTES",                     "notes",                 nullptr},
   {XmlRecord::String,  "DISPLAY_BOIL_SIZE",         nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_BATCH_SIZE",        nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TUN_VOLUME",        nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TUN_WEIGHT",        nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TOP_UP_WATER",      nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TRUB_CHILLER_LOSS", nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_LAUTER_DEADSPACE",  nullptr,                 nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TOP_UP_KETTLE",     nullptr,                 nullptr}, // Extension tag
   {XmlRecord::Double,  "REAL_EVAP_RATE",            "evapRate_lHr",          nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XmlRecord::Double,  "ABSORPTION",                "grainAbsorption_LKg",   nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XmlRecord::Double,  "BOILING_POINT",             "boilingPoint_c",        nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <INSTRUCTION>...</INSTRUCTION> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_INSTRUCTION_RECORD_FIELDS {
   // Type              XPath            Q_PROPERTY      Enum Mapper
   {XmlRecord::String,  "NAME",          "name",         nullptr},
   {XmlRecord::UInt,    "VERSION",       nullptr,        nullptr},
   {XmlRecord::String,  "directions",    "directions",   nullptr},
   {XmlRecord::Bool,    "hasTimer",      "hasTimer",     nullptr},
   {XmlRecord::String,  "timervalue",    "timerValue",   nullptr}, // NB XPath is lowercase and property is camelCase
   {XmlRecord::Bool,    "completed",     "completed",    nullptr},
   {XmlRecord::Double,  "interval",      "interval",     nullptr}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <BREWNOTE>...</BREWNOTE> BeerXML records
// NB There is no NAME field on a BREWNOTE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::FieldDefinitions const BEER_XML_BREWNOTE_RECORD_FIELDS {
   // Type              XPath                         Q_PROPERTY              Enum Mapper
   {XmlRecord::UInt,    "VERSION",                    nullptr,                nullptr},
   {XmlRecord::Date,    "BREWDATE",                   "brewDate",             nullptr},
   {XmlRecord::Date,    "DATE_FERMENTED_OUT",         "fermentDate",          nullptr},
   {XmlRecord::String,  "NOTES",                      "notes",                nullptr},
   {XmlRecord::Double,  "SG",                         "sg",                   nullptr},
   {XmlRecord::Double,  "ACTUAL_ABV",                 "abv",                  nullptr},
   {XmlRecord::Double,  "EFF_INTO_BK",                "effIntoBK_pct",        nullptr},
   {XmlRecord::Double,  "BREWHOUSE_EFF",              "brewhouseEff_pct",     nullptr},
   {XmlRecord::Double,  "VOLUME_INTO_BK",             "volumeIntoBK_l",       nullptr},
   {XmlRecord::Double,  "STRIKE_TEMP",                "strikeTemp_c",         nullptr},
   {XmlRecord::Double,  "MASH_FINAL_TEMP",            "mashFinTemp_c",        nullptr},
   {XmlRecord::Double,  "OG",                         "og",                   nullptr},
   {XmlRecord::Double,  "POST_BOIL_VOLUME",           "postBoilVolume_l",     nullptr},
   {XmlRecord::Double,  "VOLUME_INTO_FERMENTER",      "volumeIntoFerm_l",     nullptr},
   {XmlRecord::Double,  "PITCH_TEMP",                 "pitchTemp_c",          nullptr},
   {XmlRecord::Double,  "FG",                         "fg",                   nullptr},
   {XmlRecord::Double,  "ATTENUATION",                "attenuation",          nullptr},
   {XmlRecord::Double,  "FINAL_VOLUME",               "finalVolume_l",        nullptr},
   {XmlRecord::Double,  "BOIL_OFF",                   "boilOff_l",            nullptr},
   {XmlRecord::Double,  "PROJECTED_BOIL_GRAV",        "projBoilGrav",         nullptr},
   {XmlRecord::Double,  "PROJECTED_VOL_INTO_BK",      "projVolIntoBK_l",      nullptr},
   {XmlRecord::Double,  "PROJECTED_STRIKE_TEMP",      "projStrikeTemp_c",     nullptr},
   {XmlRecord::Double,  "PROJECTED_MASH_FIN_TEMP",    "projMashFinTemp_c",    nullptr},
   {XmlRecord::Double,  "PROJECTED_OG",               "projOg",               nullptr},
   {XmlRecord::Double,  "PROJECTED_VOL_INTO_FERM",    "projVolIntoFerm_l",    nullptr},
   {XmlRecord::Double,  "FG",                         "projFg",               nullptr},
   {XmlRecord::Double,  "PROJECTED_EFF",              "projEff_pct",          nullptr},
   {XmlRecord::Double,  "PROJECTED_ABV",              "projABV_pct",          nullptr},
   {XmlRecord::Double,  "PROJECTED_POINTS",           "projPoints",           nullptr},
   {XmlRecord::Double,  "PROJECTED_FERM_POINTS",      "projFermPoints",       nullptr},
   {XmlRecord::Double,  "PROJECTED_ATTEN",            "projAtten",            nullptr}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <RECIPE>...</RECIPE> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const BEER_XML_RECIPE_STEP_TYPE_MAPPER {
   {"Extract",      Recipe::Extract},
   {"Partial Mash", Recipe::PartialMash},
   {"All Grain",    Recipe::AllGrain}
};
static XmlRecord::FieldDefinitions const BEER_XML_RECIPE_RECORD_FIELDS {
   // Type               XPath                      Q_PROPERTY               Enum Mapper
   {XmlRecord::String,  "NAME",                     "name",                  nullptr},
   {XmlRecord::UInt,    "VERSION",                  nullptr,                 nullptr},
   {XmlRecord::Enum,    "TYPE",                     "recipeType",            &BEER_XML_RECIPE_STEP_TYPE_MAPPER},
   {XmlRecord::Record,  "STYLE",                    "style",                 nullptr},
   {XmlRecord::Record,  "EQUIPMENT",                "equipment",             nullptr},
   {XmlRecord::String,  "BREWER",                   "brewer",                nullptr},
   {XmlRecord::String,  "ASST_BREWER",              "asstBrewer",            nullptr},
   {XmlRecord::Double,  "BATCH_SIZE",               "batchSize_l",           nullptr},
   {XmlRecord::Double,  "BOIL_SIZE",                "boilSize_l",            nullptr},
   {XmlRecord::Double,  "BOIL_TIME",                "boilTime_min",          nullptr},
   {XmlRecord::Double,  "EFFICIENCY",               "efficiency_pct",        nullptr},
   {XmlRecord::Record,  "HOPS/HOP",                 nullptr,                 nullptr}, // Additional logic for "HOPS" is handled in xml/XmlRecipeRecord.cpp
   {XmlRecord::Record,  "FERMENTABLES/FERMENTABLE", nullptr,                 nullptr}, // Additional logic for "FERMENTABLES" is handled in xml/XmlRecipeRecord.cpp
   {XmlRecord::Record,  "MISCS/MISC",               nullptr,                 nullptr}, // Additional logic for "MISCS" is handled in xml/XmlRecipeRecord.cpp
   {XmlRecord::Record,  "YEASTS/YEAST",             nullptr,                 nullptr}, // Additional logic for "YEASTS" is handled in xml/XmlRecipeRecord.cpp
   {XmlRecord::Record,  "WATERS/WATER",             nullptr,                 nullptr}, // Additional logic for "WATERS" is handled in xml/XmlRecipeRecord.cpp
   {XmlRecord::Record,  "MASH",                     "mash",                  nullptr},
   {XmlRecord::Record,  "INSTRUCTIONS/INSTRUCTION", nullptr,                 nullptr}, // Additional logic for "INSTRUCTIONS" is handled in xml/XmlNamedEntityRecord.h
   {XmlRecord::Record,  "BREWNOTES/BREWNOTE",       nullptr,                 nullptr}, // Additional logic for "BREWNOTES" is handled in xml/XmlNamedEntityRecord.h
   {XmlRecord::String,  "NOTES",                    "notes",                 nullptr},
   {XmlRecord::String,  "TASTE_NOTES",              "tasteNotes",            nullptr},
   {XmlRecord::Double,  "TASTE_RATING",             "tasteRating",           nullptr},
   {XmlRecord::Double,  "OG",                       "og",                    nullptr},
   {XmlRecord::Double,  "FG",                       "fg",                    nullptr},
   {XmlRecord::UInt,    "FERMENTATION_STAGES",      "fermentationStages",    nullptr},
   {XmlRecord::Double,  "PRIMARY_AGE",              "primaryAge_days",       nullptr},
   {XmlRecord::Double,  "PRIMARY_TEMP",             "primaryTemp_c",         nullptr},
   {XmlRecord::Double,  "SECONDARY_AGE",            "secondaryAge_days",     nullptr},
   {XmlRecord::Double,  "SECONDARY_TEMP",           "secondaryTemp_c",       nullptr},
   {XmlRecord::Double,  "TERTIARY_AGE",             "tertiaryAge_days",      nullptr},
   {XmlRecord::Double,  "TERTIARY_TEMP",            "tertiaryTemp_c",        nullptr},
   {XmlRecord::Double,  "AGE",                      "age",                   nullptr},
   {XmlRecord::Double,  "AGE_TEMP",                 "ageTemp_c",             nullptr},
   {XmlRecord::Date,    "DATE",                     "date",                  nullptr},
   {XmlRecord::Double,  "CARBONATION",              "carbonation_vols",      nullptr},
   {XmlRecord::Bool,    "FORCED_CARBONATION",       "forcedCarbonation",     nullptr},
   {XmlRecord::String,  "PRIMING_SUGAR_NAME",       "primingSugarName",      nullptr},
   {XmlRecord::Double,  "CARBONATION_TEMP",         "carbonationTemp_c",     nullptr},
   {XmlRecord::Double,  "PRIMING_SUGAR_EQUIV",      "primingSugarEquiv",     nullptr},
   {XmlRecord::Double,  "KEG_PRIMING_FACTOR",       "kegPrimingFactor",      nullptr},
   {XmlRecord::String,  "EST_OG",                    nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "EST_FG",                    nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "EST_COLOR",                 nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "IBU",                       nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "IBU_METHOD",                nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "EST_ABV",                   nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "ABV",                       nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "ACTUAL_EFFICIENCY",         nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "CALORIES",                  nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_BATCH_SIZE",        nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_BOIL_SIZE",         nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_OG",                nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_FG",                nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_PRIMARY_TEMP",      nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_SECONDARY_TEMP",    nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_TERTIARY_TEMP",     nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_AGE_TEMP",          nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "CARBONATION_USED",          nullptr,                nullptr}, // Extension tag
   {XmlRecord::String,  "DISPLAY_CARB_TEMP",         nullptr,                nullptr} // Extension tag
};


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
               // .:TODO:. We should make XmlCoding::construct dependent on the NamedEntity rather than the XmlRecord, and delegate those details to XmlRecord or XmlCoding
               QHash<QString, XmlCoding::XmlRecordDefinition>{
                  {"BEER_XML",    {&XmlCoding::construct< XmlRecord >,                         &BEER_XML_ROOT_RECORD_FIELDS} },
                  {"HOP",         {&XmlCoding::construct< XmlNamedEntityRecord<Hop> >,         &BEER_XML_HOP_RECORD_FIELDS} },
                  {"FERMENTABLE", {&XmlCoding::construct< XmlNamedEntityRecord<Fermentable> >, &BEER_XML_FERMENTABLE_RECORD_FIELDS} },
                  {"YEAST",       {&XmlCoding::construct< XmlNamedEntityRecord<Yeast> >,       &BEER_XML_YEAST_RECORD_FIELDS} },
                  {"MISC",        {&XmlCoding::construct< XmlNamedEntityRecord<Misc> >,        &BEER_XML_MISC_RECORD_FIELDS} },
                  {"WATER",       {&XmlCoding::construct< XmlNamedEntityRecord<Water> >,       &BEER_XML_WATER_RECORD_FIELDS} },
                  {"STYLE",       {&XmlCoding::construct< XmlNamedEntityRecord<Style> >,       &BEER_XML_STYLE_RECORD_FIELDS} },
                  {"MASH_STEP",   {&XmlCoding::construct< XmlMashStepRecord >,                 &BEER_XML_MASH_STEP_RECORD_FIELDS} },
                  {"MASH",        {&XmlCoding::construct< XmlNamedEntityRecord<Mash> >,        &BEER_XML_MASH_RECORD_FIELDS} },
                  {"EQUIPMENT",   {&XmlCoding::construct< XmlNamedEntityRecord<Equipment> >,   &BEER_XML_EQUIPMENT_RECORD_FIELDS} },
                  {"INSTRUCTION", {&XmlCoding::construct< XmlNamedEntityRecord<Instruction> >, &BEER_XML_INSTRUCTION_RECORD_FIELDS} },
                  {"BREWNOTE",    {&XmlCoding::construct< XmlNamedEntityRecord<BrewNote> >,    &BEER_XML_BREWNOTE_RECORD_FIELDS} },
                  {"RECIPE",      {&XmlCoding::construct< XmlRecipeRecord >,                   &BEER_XML_RECIPE_RECORD_FIELDS} }
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
      //  - We retain unchanged the first line of the file (which will be something along the lines of
      //    "<?xml version="1.0" blah blah ?>"
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
      qDebug() << Q_FUNC_INFO << "First line of " << inputFile.fileName() << " was " << QString(documentData);
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


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
BeerXML::~BeerXML() = default;


QString BeerXML::textFromValue(QVariant value, QString type)
{
   QString retval = value.toString();

   if (type == "boolean" )
      retval = Ingredient::text(value.toBool());
   else if (type == "real")
      retval = Ingredient::text(value.toDouble());
   else if (type == "timestamp")
      retval = Ingredient::text(value.toDate());

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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString val;
         // flySparge and batchSparge aren't part of the BeerXML spec.
         // This makes sure we give BeerXML something it understands.
         if ( element == kpropType ) {
            if ( (a->type() == MashStep::flySparge) || (a->type() == MashStep::batchSparge ) ) {
               val = MashStep::types[0];
            }
            else {
               val = a->typeString();
            }
         }
         else {
            val = textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
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
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

/*
// fromXml ====================================================================
void BeerXML::fromXml(Ingredient* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         qWarning() << QString("Node at line %1 is not an element.").arg(textNode.lineNumber());
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();

      if( xmlTagsToProperties.contains(xmlTag) )
      {
         switch( element->metaProperty(xmlTagsToProperties[xmlTag]).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), stringVal);
               break;
            default:
               qWarning() << QString("%1: don't understand property type.").arg(Q_FUNC_INFO);
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            qCritical() << QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag);
            return;
         }
      }
   }

}

void BeerXML::fromXml(Ingredient* element, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;
   TableSchema* schema = m_tables->table( element->table() );

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         qWarning() << QString("Node at line %1 is not an element.").arg(textNode.lineNumber());
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();
      QString pTag = schema->xmlToProperty(xmlTag);

      if( pTag.size() ) {
         switch( element->metaProperty(pTag).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(pTag.toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(pTag.toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(pTag.toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(pTag.toStdString().c_str(), stringVal);
               break;
            default:
               qWarning() << QString("%1: don't understand property type. xmlTag=%2")
                     .arg(Q_FUNC_INFO)
                     .arg(xmlTag);
               break;
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            qCritical() << QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag);
            return;
         }
      }
   }
}

// Brewnotes can never be created w/ a recipe, so we will always assume the
// calling method has the transactions
BrewNote* BeerXML::brewNoteFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   BrewNote* ret = nullptr;
   QDateTime theDate;

   n = node.firstChildElement("BREWDATE");
   theDate = QDateTime::fromString(n.firstChild().toText().nodeValue(),Qt::ISODate);

   try {
      ret = new BrewNote(theDate);

      if ( ! ret ) {
         QString error = "Could not create new brewnote.";
         qCritical() << QString(error);
         QMessageBox::critical(nullptr, tr("Import error."),
               error.append("\nUnable to create brew note."));
         return ret;
      }
      // Need to tell the brewnote not to perform the calculations
      ret->setLoading(true);
      fromXml(ret, node);
      ret->setLoading(false);

      ret->setRecipe(parent);
      ret->insertInDatabase();
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }
   return ret;
}

Equipment* BeerXML::equipmentFromXml( QDomNode const& node, Recipe* parent )
{
   // When loading from XML, we need to delay the signals until after
   // everything is done. This should significantly speed up the load times

   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Equipment* ret;
   QString name;
   QList<Equipment*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing an equip by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is an equip already in the DB with the same name.
         db.getElementsByName<Equipment>( matching, Brewtarget::EQUIPTABLE, name, db.allEquipments);

         // If we find a match, use it
         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Equipment(name,true);
         }
      }
      else {
         ret = new Equipment(name,true);
      }

      if ( createdNew ) {
         fromXml(ret, node);
         if ( ! ret->isValid() )
            throw QString("There was an error loading equipment profile from XML");

         // If we are importing one of our beerXML files, the utilization is always
         // 0%. We need to fix that.
         if ( ret->hopUtilization_pct() == 0.0 )
            ret->setHopUtilization_pct(100.0);

         ret->insertInDatabase();
      }

      if( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      throw;
   }

   blockSignals(false);


   if ( createdNew ) {
      emit db.changed( db.metaProperty("equipments"), QVariant() );
      emit db.newEquipmentSignal(ret);
   }

   return ret;
}

Fermentable* BeerXML::fermentableFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Fermentable* ret = nullptr;
   QString name;
   QList<Fermentable*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a ferm by itself, need to do some dupe-checking.
      if ( parent == nullptr )
      {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if we already have a Fermentable with this name
         db.getElementsByName<Fermentable>( matching,
               Brewtarget::FERMTABLE,
               name, db.allFermentables);

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Fermentable(name);
         }
      }
      else {
         ret = new Fermentable(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() )
            throw QString("Error reading fermentable from XML");


         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = Fermentable::types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType( static_cast<Fermentable::Type>(ndx));
            else
               ret->invalidate();
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::fermentableFromXml: Could convert a recognized type");
         }
         ret->insertInDatabase();

      }

      if ( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      if ( parent == nullptr ) {
         db.sqlDatabase().rollback();
      }
      qCritical () << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }

   if ( parent == nullptr ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("fermentables"), QVariant() );
      emit db.newFermentableSignal(ret);
   }

   return ret;
}

int BeerXML::getQualifiedHopTypeIndex(QString type, Hop* hop)
{
   Database & db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::types.indexOf(type) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      // Check to see if there is an hop already in the DB with the same name.
      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString htype = q.record().value(0).toString();
            q.finish();
            if ( htype != "" &&  Hop::types.indexOf(htype) >= 0 ) {
               return Hop::types.indexOf(htype);
            }
         }
      }
      // out of ideas at this point so default to Both
      return Hop::types.indexOf(QString("Both"));
   }
   else {
      return Hop::types.indexOf(type);
   }
}

int BeerXML::getQualifiedHopUseIndex(QString use, Hop* hop)
{
   Database & db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::uses.indexOf(use) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropUse))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));

      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString hUse = q.record().value(0).toString();
            q.finish();
            if ( hUse != "" &&  Hop::uses.indexOf(hUse) >= 0 ) {
               return Hop::uses.indexOf(hUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Hop::uses.indexOf(QString("Flavor"));
   }
   else {
      return Hop::uses.indexOf(use);
   }
}

Hop* BeerXML::hopFromXml( QDomNode const& node, Recipe* parent )
{
   Database & db = Database::instance();
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Hop* ret;
   QString name;
   QList<Hop*> matching;

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a hop by itself, need to do some dupe-checking.
      if( parent == nullptr )
      {
         // as always, start the transaction if no parent
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Hop>( matching,
               Brewtarget::HOPTABLE,
               name,
               db.allHops);

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Hop(name);
         }
      }
      else {
         ret = new Hop(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() ) {
            throw QString("Error reading Hop from XML");
         }

         // Handle enums separately.
         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = getQualifiedHopUseIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setUse( static_cast<Hop::Use>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = getQualifiedHopTypeIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setType( static_cast<Hop::Type>(ndx) );
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Hop::forms.indexOf(n.firstChild().toText().nodeValue());
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Hop::Form>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::hopFromXml: Could convert %1 to a recognized type");
         }
         ret->insertInDatabase();
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      if ( ! parent )
         db.sqlDatabase().rollback();

      abort();
   }

   if ( ! parent ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);

   if( createdNew ) {
      emit db.changed( db.metaProperty("hops"), QVariant() );
      emit db.newHopSignal(ret);
   }

   return ret;
}

// like brewnotes, we will assume here that the caller has the transaction
// block
Instruction* BeerXML::instructionFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   QString name;
   Instruction* ret;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   blockSignals(true);
   try {
      ret = new Instruction(name);

      fromXml(ret, node);

      ret->setRecipe(parent);
      ret->insertInDatabase();

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   emit db.changed( db.metaProperty("instructions"), QVariant() );
   return ret;
}


Mash* BeerXML::mashFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Mash* ret;
   QString name;
   QList<Mash*> matching;
   Database & db = Database::instance();

   blockSignals(true);

   // Mashes are weird. We need to know if this is a duplicate, but we need to
   // make a copy of it anyway.
   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   try {
      ret = new Mash(name);

      // If the mash has a name
      if ( ! name.isEmpty() ) {
         db.getElementsByName<Mash>( matching, Brewtarget::MASHTABLE, name, db.allMashs );

         // If there are no other matches in the database
         if( matching.isEmpty() ) {
            ret->setDisplay(true);
         }
      }
      // First, get all the standard properties.
      fromXml( ret, node );

      //Need to insert the Mash before the Mash steps to get
      //the ID for foreign key contraint in Maststep table.
      db.sqlDatabase().transaction();
      ret->insertInDatabase();

      // Now, get the individual mash steps.
      n = node.firstChildElement("MASH_STEPS");
      if( ! n.isNull() ) {
         // Iterate through all the mash steps.
         for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         {
            MashStep* temp = mashStepFromXml( n, ret );
            if ( ! temp->isValid() ) {
               QString error = QString("Error importing mash step %1. Importing as infusion").arg(temp->name());
               qCritical() << error;
            }
         }
      }

      if ( parent ) {
         db.addToRecipe( parent, ret, true, false);
      }

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      db.sqlDatabase().rollback();
      abort();
   }

   db.sqlDatabase().commit();

   blockSignals(false);

   emit db.changed( db.metaProperty("mashs"), QVariant() );
   emit db.newMashSignal(ret);
   emit ret->mashStepsChanged();

   return ret;
}

// mashsteps don't exist without a mash. It is up to mashFromXml or
// recipeFromXml to deal with the transaction
MashStep* BeerXML::mashStepFromXml( QDomNode const& node, Mash* parent )
{
   QDomNode n;
   QString str;
   Database & db = Database::instance();

   try {
      MashStep* ret = new MashStep("", true);

      fromXml(ret,node);

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         //Try to make sure incoming format matches
         //e.g. convert INFUSION to Infusion
         str = n.firstChild().toText().nodeValue();
         str = str.toLower();
         str[0] = str.at(0).toTitleCase();
         int ndx =  MashStep::types.indexOf(str);

         if ( ndx != -1 )
            ret->setType( static_cast<MashStep::Type>(ndx) );
         else
            ret->invalidate();
      }

      ret->setMash(parent);
      ret->insertInDatabase();

      if (! signalsBlocked() )
      {
         emit db.changed( db.metaProperty("mashs"), QVariant() );
         emit parent->mashStepsChanged();
      }
      return ret;
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      abort();
   }

}

int BeerXML::getQualifiedMiscTypeIndex(QString type, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database & db = Database::instance();

   if ( Misc::types.indexOf(type) < 0 ) {
      // look for a valid mash type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() )
         {
            QString mtype = q.record().value(0).toString();
            q.finish();
            if ( mtype != "" &&  Misc::types.indexOf(mtype) >= 0 ) {
               return Misc::types.indexOf(mtype);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::types.indexOf(QString("Flavor"));
   }
   else {
      return Misc::types.indexOf(type);
   }
}

int BeerXML::getQualifiedMiscUseIndex(QString use, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database & db = Database::instance();

   if ( Misc::uses.indexOf(use) < 0 ) {
      // look for a valid misc type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:use AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropUse));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString mUse = q.record().value(0).toString();
            q.finish();
            if ( mUse != "" &&  Misc::uses.indexOf(mUse) >= 0 ) {
               return Misc::uses.indexOf(mUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::uses.indexOf(QString("Flavor"));
   }
   else {
      return Misc::uses.indexOf(use);
   }
}

Misc* BeerXML::miscFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Misc* ret;
   QString name;
   QList<Misc*> matching;

   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a misc by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is a hop already in the DB with the same name.
         db.sqlDatabase().transaction();

         db.getElementsByName<Misc>( matching, Brewtarget::MISCTABLE, name, db.allMiscs );

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Misc(name);
         }
      }
      else {
         ret = new Misc(name);
      }

      if ( createdNew ) {

         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         // Assuming these return anything is a bad idea. So far, several other brewing programs are not generating
         // valid XML.
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setType( static_cast<Misc::Type>(getQualifiedMiscTypeIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setUse(static_cast<Misc::Use>(getQualifiedMiscUseIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::miscFromXml: Could convert %1 to a recognized type");
         }
         ret->insertInDatabase();
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("miscs"), QVariant() );
      emit db.newMiscSignal(ret);
   }
   return ret;
}

Recipe* BeerXML::recipeFromXml( QDomNode const& node )
{
   QDomNode n;
   blockSignals(true);
   Recipe *ret;
   QString name;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {

      // This is all one long, gnarly transaction.
      db.sqlDatabase().transaction();

      // Oh sweet mercy
      ret = new Recipe(name);

      if ( ! ret ) {
         return nullptr;
      }
      // Get standard properties.
      fromXml(ret, node);

      // I need to insert this now, because I need a valid key for adding
      // things to the recipe
      ret->insertInDatabase();

      // Get style. Note: styleFromXml requires the entire node, not just the
      // firstchild of the node.
      n = node.firstChildElement("STYLE");
      styleFromXml(n, ret);
      if ( ! ret->style()->isValid())
         ret->invalidate();

      // Get equipment. equipmentFromXml requires the entire node, not just the
      // first child
      n = node.firstChildElement("EQUIPMENT");
      equipmentFromXml(n, ret);
      if ( !ret->equipment()->isValid() )
         ret->invalidate();

      // Get hops.
      n = node.firstChildElement("HOPS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Hop* temp = hopFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // Get ferms.
      n = node.firstChildElement("FERMENTABLES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Fermentable* temp = fermentableFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // get mashes. There is only one mash per recipe, so this needs the entire
      // node.
      n = node.firstChildElement("MASH");
      mashFromXml(n, ret);
      if ( ! ret->mash()->isValid() )
         ret->invalidate();

      // Get miscs.
      n = node.firstChildElement("MISCS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Misc* temp = miscFromXml(n, ret);
         if (! temp->isValid())
            ret->invalidate();
      }

      // Get yeasts.
      n = node.firstChildElement("YEASTS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Yeast* temp = yeastFromXml(n, ret);
         if ( !temp->isValid() )
            ret->invalidate();
      }

      // Get waters. Odd. Waters don't invalidate.
      n = node.firstChildElement("WATERS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         waterFromXml(n, ret);

      // That ends the beerXML defined objects. I'm not going to do the
      // validation for these last two. We write em, and we had better be
      // writing them properly
      //
      // Get instructions.
      n = node.firstChildElement("INSTRUCTIONS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         instructionFromXml(n, ret);

      // Get brew notes
      n = node.firstChildElement("BREWNOTES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         brewNoteFromXml(n, ret);

      // If we get here, commit
      db.sqlDatabase().commit();

      // Recalc everything, just for grins and giggles.
      ret->recalcAll();
      blockSignals(false);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   emit db.newRecipeSignal(ret);
   return ret;
}

Style* BeerXML::styleFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Style* ret;
   QString name;
   QList<Style*> matching;

   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if there is a style already in the DB with the same name.
         db.getElementsByName<Style>( matching, Brewtarget::STYLETABLE, name, db.allStyles );

         // If we found a match, use it.
         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            // We could find no matching style in the db
            ret = new Style(name,true);
         }
      }
      else {
         // If we are inserting this as part of a recipe, we can skip straight
         // to creating a new one
         ret = new Style(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Style::m_types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType(static_cast<Style::Type>(ndx));
            else
               ret->invalidate();
         }

         // If translating the enums craps out, give a warning
         if (! ret->isValid() ) {
            qWarning() << QString("BeerXML::styleFromXml: Could convert %1 to a recognized type");
         }
         // we need to poke this into the database
         ret->insertInDatabase();
      }
      if ( parent ) {
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("styles"), QVariant() );
      emit db.newStyleSignal(ret);
   }

   return ret;
}

Water* BeerXML::waterFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Water* ret;
   QString name;
   QList<Water*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Water>( matching, Brewtarget::WATERTABLE, name, db.allWaters );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = new Water(name);
      }
      else
         ret = new Water(name);

      if ( createdNew ) {
         fromXml( ret, node );
         ret->insertInDatabase();
         if( parent ) {
            db.addToRecipe( parent, ret, false );
         }
      }
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      if ( parent == nullptr )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("waters"), QVariant() );
      emit db.newWaterSignal(ret);
   }

   return ret;
}

Yeast* BeerXML::yeastFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Yeast* ret;
   QString name;
   QList<Yeast*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a yeast by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // start the transaction, just in case
         db.sqlDatabase().transaction();
         // Check to see if there is a yeast already in the DB with the same name.
         db.getElementsByName<Yeast>( matching, Brewtarget::YEASTTABLE, name, db.allYeasts );

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Yeast(name);
         }
      }
      else {
         ret = new Yeast(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle type enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find TYPE in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::types.indexOf( tname );
            if ( ndx != -1) {
               ret->setType( static_cast<Yeast::Type>(ndx) );
            }
            else {
               ret->setType(static_cast<Yeast::Type>(0));
               qCritical() <<
                     QString("Could not translate the type %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }
         // Handle form enums separately.
         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find FORM in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::forms.indexOf( tname );
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Yeast::Form>(ndx) );
            }
            else {
               ret->setForm( static_cast<Yeast::Form>(0) );
               qCritical() <<
                     QString("Could not translate the form %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }
         // Handle flocc enums separately.
         n = node.firstChildElement("FLOCCULATION");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find FLOCCULATION in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::flocculations.indexOf( tname );
            if (ndx != -1) {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(ndx) );
            }
            else {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(0) );
               qCritical() <<
                     QString("Could not translate the flocculation %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }

         ret->insertInDatabase();
      }

      if( parent ) {
         // we are in a transaction boundary, so tell addToRecipe not to start
         // another
         db.addToRecipe( parent, ret, false );
         parent->recalcOgFg();
         parent->recalcABV_pct();
      }
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO<< e;
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   db.sqlDatabase().commit();
   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("yeasts"), QVariant() );
      emit db.newYeastSignal(ret);
   }

   return ret;
}
*/

bool BeerXML::importFromXML(QString const & filename, QTextStream & userMessage)
{
   //
   // Call the new code that attempts to validate the file before reading in its data
   //
   return this->pimpl->validateAndLoad(filename, userMessage);

/*
   int count;
   int line, col;
   QDomDocument xmlDoc;
   QDomElement root;
   QDomNodeList list;
   QString err;
   QFile inFile;
   QStringList tags = QStringList() << "EQUIPMENT" << "FERMENTABLE" << "HOP" << "MISC" << "STYLE" << "YEAST" << "WATER" << "MASHS";
   inFile.setFileName(filename);
   bool ret = true;

   if( ! inFile.open(QIODevice::ReadOnly) )
   {
      qWarning() << Q_FUNC_INFO << "Could not open " << filename << " for reading";
      return false;
   }

   if( ! xmlDoc.setContent(&inFile, false, &err, &line, &col) )
      qWarning() << Q_FUNC_INFO << ": Bad document formatting in " << filename << " " << line << ":" << col << ". " << err;

   list = xmlDoc.elementsByTagName("RECIPE");
   if ( list.count() )
   {
      for(int i = 0; i < list.count(); ++i )
      {
         Recipe* temp = this->recipeFromXml( list.at(i) );
         if ( ! temp || ! temp->isValid() )
            ret = false;
      }
   }
   else
   {
      foreach (QString tag, tags)
      {
         list = xmlDoc.elementsByTagName(tag);
         count = list.size();

         if ( count > 0 )
         {
            // Tell how many there were in the status bar.
            //statusBar()->showMessage( tr("Found %1 %2.").arg(count).arg(tag.toLower()), 5000 );

            if (tag == "RECIPE")
            {
            }
            else if ( tag == "EQUIPMENT" )
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Equipment* temp = this->equipmentFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "FERMENTABLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Fermentable* temp = this->fermentableFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }

            }
            else if (tag == "HOP")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Hop* temp = this->hopFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "MISC")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Misc* temp = this->miscFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "STYLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Style* temp = this->styleFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "YEAST")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Yeast* temp = this->yeastFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "WATER" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Water* temp = this->waterFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "MASHS" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Mash* temp = this->mashFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
         }
      }
   }
   return ret;
   */
}
