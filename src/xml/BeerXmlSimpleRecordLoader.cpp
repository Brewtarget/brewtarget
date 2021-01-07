/*
 * xml/BeerXmlSimpleRecordLoader.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "xml/BeerXmlSimpleRecordLoader.h"

#include "hop.h"
#include "fermentable.h"
#include "yeast.h"
#include "misc.h"
#include "water.h"
#include "style.h"
#include "equipment.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <HOP>...</HOP> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XPathRecordLoader::EnumLookupMap const HOP_USE_MAPPER {
   {"Boil",       Hop::Boil},
   {"Dry Hop",    Hop::Dry_Hop},
   {"Mash",       Hop::Mash},
   {"First Wort", Hop::First_Wort},
   {"Aroma",      Hop::UseAroma}
};

static XPathRecordLoader::EnumLookupMap const HOP_TYPE_MAPPER {
   {"Bittering", Hop::Bittering},
   {"Aroma",     Hop::Aroma},
   {"Both",      Hop::Both}
};

static XPathRecordLoader::EnumLookupMap const HOP_FORM_MAPPER {
   {"Pellet", Hop::Pellet},
   {"Plug",   Hop::Plug},
   {"Leaf",   Hop::Leaf}
};

static QVector<XPathRecordLoader::Field> const HOP_RECORD_FIELDS {
   // Type                      XML Name          Q_PROPERTY           Enum Mapper
   {XPathRecordLoader::String,  "NAME",           "name",              nullptr},
   {XPathRecordLoader::UInt,    "VERSION",        nullptr,             nullptr},
   {XPathRecordLoader::Double,  "ALPHA",          "alpha_pct",         nullptr},
   {XPathRecordLoader::Double,  "AMOUNT",         "amount_kg",         nullptr},
   {XPathRecordLoader::Enum,    "USE",            "use",               &HOP_USE_MAPPER},
   {XPathRecordLoader::Double,  "TIME",           "time_min",          nullptr},
   {XPathRecordLoader::String,  "NOTES",          "notes",             nullptr},
   {XPathRecordLoader::Enum,    "TYPE",           "type",              &HOP_TYPE_MAPPER},
   {XPathRecordLoader::Enum,    "FORM",           "form",              &HOP_FORM_MAPPER},
   {XPathRecordLoader::Double,  "BETA",           "beta_pct",          nullptr},
   {XPathRecordLoader::Double,  "HSI",            "hsi_pct",           nullptr},
   {XPathRecordLoader::String,  "ORIGIN",         "origin",            nullptr},
   {XPathRecordLoader::String,  "SUBSTITUTES",    "substitutes",       nullptr},
   {XPathRecordLoader::Double,  "HUMULENE",       "humulene_pct",      nullptr},
   {XPathRecordLoader::Double,  "CARYOPHYLLENE",  "caryophyllene_pct", nullptr},
   {XPathRecordLoader::Double,  "COHUMULONE",     "cohumulone_pct",    nullptr},
   {XPathRecordLoader::Double,  "MYRCENE",        "myrcene_pct",       nullptr},
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT", nullptr,             nullptr}, // Extension tag
   {XPathRecordLoader::String,  "INVENTORY",      nullptr,             nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TIME",   nullptr,             nullptr}  // Extension tag
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <FERMENTABLE>...</FERMENTABLE> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XPathRecordLoader::EnumLookupMap const FERMENTABLE_TYPE_MAPPER {
   {"Grain",       Fermentable::Grain},
   {"Sugar",       Fermentable::Sugar},
   {"Extract",     Fermentable::Extract},
   {"Dry Extract", Fermentable::Dry_Extract},
   {"Adjunct",     Fermentable::Adjunct}
};

static QVector<XPathRecordLoader::Field> const FERMENTABLE_RECORD_FIELDS {
   // Type                     XML Name             Q_PROPERTY                Enum Mapper
   {XPathRecordLoader::String,  "NAME",             "name",                   nullptr},
   {XPathRecordLoader::UInt,    "VERSION",          nullptr,                  nullptr},
   {XPathRecordLoader::Enum,    "TYPE",             "type",                   &FERMENTABLE_TYPE_MAPPER},
   {XPathRecordLoader::Double,  "AMOUNT",           "amount_kg",              nullptr},
   {XPathRecordLoader::Double,  "YIELD",            "yield_pct",              nullptr},
   {XPathRecordLoader::Double,  "COLOR",            "color_srm",              nullptr},
   {XPathRecordLoader::Bool,    "ADD_AFTER_BOIL",   "addAfterBoil",           nullptr},
   {XPathRecordLoader::String,  "ORIGIN",           "origin",                 nullptr},
   {XPathRecordLoader::String,  "SUPPLIER",         "supplier",               nullptr},
   {XPathRecordLoader::String,  "NOTES",            "notes",                  nullptr},
   {XPathRecordLoader::Double,  "COARSE_FINE_DIFF", "coarseFineDiff_pct",     nullptr},
   {XPathRecordLoader::Double,  "MOISTURE",         "moisture_pct",           nullptr},
   {XPathRecordLoader::Double,  "DIASTATIC_POWER",  "diastaticPower_lintner", nullptr},
   {XPathRecordLoader::Double,  "PROTEIN",          "protein_pct",            nullptr},
   {XPathRecordLoader::Double,  "MAX_IN_BATCH",     "maxInBatch_pct",         nullptr},
   {XPathRecordLoader::Bool,    "RECOMMEND_MASH",   "recommendMash",          nullptr},
   {XPathRecordLoader::Double,  "IBU_GAL_PER_LB",   "ibuGalPerLb",            nullptr},
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT",   nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "POTENTIAL",        nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "INVENTORY",        nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_COLOR",    nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::Bool,    "IS_MASHED",        "isMashed",               nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <YEAST>...</YEAST> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XPathRecordLoader::EnumLookupMap const YEAST_TYPE_MAPPER {
   {"Ale",       Yeast::Ale},
   {"Lager",     Yeast::Lager},
   {"Wheat",     Yeast::Wheat},
   {"Wine",      Yeast::Wine},
   {"Champagne", Yeast::Champagne}
};

static XPathRecordLoader::EnumLookupMap const YEAST_FORM_MAPPER {
   {"Liquid",  Yeast::Liquid},
   {"Dry",     Yeast::Dry},
   {"Slant",   Yeast::Slant},
   {"Culture", Yeast::Culture}
};

static XPathRecordLoader::EnumLookupMap const YEAST_FLOCCULATION_MAPPER {
   {"Low",       Yeast::Low},
   {"Medium",    Yeast::Medium},
   {"High",      Yeast::High},
   {"Very High", Yeast::Very_High}
};

static QVector<XPathRecordLoader::Field> const YEAST_RECORD_FIELDS {
   // Type                      XML Name            Q_PROPERTY                Enum Mapper
   {XPathRecordLoader::String,  "NAME",             "name",                   nullptr},
   {XPathRecordLoader::UInt,    "VERSION",          nullptr,                  nullptr},
   {XPathRecordLoader::Enum,    "TYPE",             "type",                   &YEAST_TYPE_MAPPER},
   {XPathRecordLoader::Enum,    "TYPE",             "type",                   &YEAST_FORM_MAPPER},
   {XPathRecordLoader::Double,  "AMOUNT",           "amount",                 nullptr},
   {XPathRecordLoader::Bool,    "AMOUNT_IS_WEIGHT", "amountIsWeight",         nullptr},
   {XPathRecordLoader::String,  "LABORATORY",       "laboratory",             nullptr},
   {XPathRecordLoader::String,  "PRODUCT_ID",       "productID",              nullptr},
   {XPathRecordLoader::Double,  "MIN_TEMPERATURE",  "minTemperature_c",       nullptr},
   {XPathRecordLoader::Double,  "MAX_TEMPERATURE",  "maxTemperature_c",       nullptr},
   {XPathRecordLoader::Enum,    "FLOCCULATION",     "flocculation",           &YEAST_FLOCCULATION_MAPPER},
   {XPathRecordLoader::Double,  "ATTENUATION",      "attenuation_pct",        nullptr},
   {XPathRecordLoader::String,  "NOTES",            "notes",                  nullptr},
   {XPathRecordLoader::String,  "BEST_FOR",         "bestFor",                nullptr},
   {XPathRecordLoader::Int,     "TIMES_CULTURED",   "timesCultured",          nullptr},
   {XPathRecordLoader::Int,     "MAX_REUSE",        "maxReuse",               nullptr},
   {XPathRecordLoader::Bool,    "ADD_TO_SECONDARY", "addToSecondary",         nullptr},
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT",   nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISP_MIN_TEMP",    nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISP_MAX_TEMP",    nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "INVENTORY",        nullptr,                  nullptr}, // Extension tag
   {XPathRecordLoader::String,  "CULTURE_DATE",     nullptr,                  nullptr}  // Extension tag
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MISC>...</MISC> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XPathRecordLoader::EnumLookupMap const MISC_TYPE_MAPPER {
   {"Spice",       Misc::Spice},
   {"Fining",      Misc::Fining},
   {"Water Agent", Misc::Water_Agent},
   {"Herb",        Misc::Herb},
   {"Flavor",      Misc::Flavor},
   {"Other",       Misc::Other}
};

static XPathRecordLoader::EnumLookupMap const MISC_USE_MAPPER {
   {"Boil",      Misc::Boil},
   {"Mash",      Misc::Mash},
   {"Primary",   Misc::Primary},
   {"Secondary", Misc::Secondary},
   {"Bottling",  Misc::Bottling}
};

static QVector<XPathRecordLoader::Field> const MISC_RECORD_FIELDS {
   // Type                      XML Name            Q_PROPERTY        Enum Mapper
   {XPathRecordLoader::String,  "NAME",             "name",           nullptr},
   {XPathRecordLoader::UInt,    "VERSION",          nullptr,          nullptr},
   {XPathRecordLoader::Enum,    "TYPE",             "type",           &MISC_TYPE_MAPPER},
   {XPathRecordLoader::Enum,    "USE",              "use",            &MISC_USE_MAPPER},
   {XPathRecordLoader::Double,  "TIME",             "time",           nullptr},
   {XPathRecordLoader::Double,  "AMOUNT",           "amount",         nullptr},
   {XPathRecordLoader::Bool,    "AMOUNT_IS_WEIGHT", "amountIsWeight", nullptr},
   {XPathRecordLoader::String,  "USE_FOR",          "useFor",         nullptr},
   {XPathRecordLoader::String,  "NOTES",            "notes",          nullptr},
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT", nullptr,            nullptr}, // Extension tag
   {XPathRecordLoader::String,  "INVENTORY",      nullptr,            nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TIME",   nullptr,            nullptr}  // Extension tag
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <WATER>...</WATER> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QVector<XPathRecordLoader::Field> const WATER_RECORD_FIELDS {
   // Type                      XML Name         Q_PROPERTY         Enum Mapper
   {XPathRecordLoader::String,  "NAME",          "name",            nullptr},
   {XPathRecordLoader::UInt,    "VERSION",       nullptr,           nullptr},
   {XPathRecordLoader::Double,  "AMOUNT",        "amount",          nullptr},
   {XPathRecordLoader::Double,  "CALCIUM",       "calcium_ppm",     nullptr},
   {XPathRecordLoader::Double,  "BICARBONATE",   "bicarbonate_ppm", nullptr},
   {XPathRecordLoader::Double,  "SULFATE",       "sulfate_ppm",     nullptr},
   {XPathRecordLoader::Double,  "CHLORIDE",      "chloride_ppm",    nullptr},
   {XPathRecordLoader::Double,  "SODIUM",        "sodium_ppm",      nullptr},
   {XPathRecordLoader::Double,  "MAGNESIUM",     "magnesium_ppm",   nullptr},
   {XPathRecordLoader::Double,  "PH",            "ph",              nullptr},
   {XPathRecordLoader::String,  "NOTES",         "notes",           nullptr},
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT", nullptr,          nullptr}  // Extension tag
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <STYLE>...</STYLE> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XPathRecordLoader::EnumLookupMap const STYLE_TYPE_MAPPER {
   {"Lager", Style::Lager},
   {"Ale",   Style::Ale},
   {"Mead",  Style::Mead},
   {"Wheat", Style::Wheat},
   {"Mixed", Style::Mixed},
   {"Cider", Style::Cider}
};

static QVector<XPathRecordLoader::Field> const STYLE_RECORD_FIELDS {
   // Type                      XML Name             Q_PROPERTY        Enum Mapper
   {XPathRecordLoader::String,  "NAME",              "name",           nullptr},
   {XPathRecordLoader::String,  "CATEGORY",          "category",       nullptr},
   {XPathRecordLoader::UInt,    "VERSION",           nullptr,          nullptr},
   {XPathRecordLoader::String,  "CATEGORY_NUMBER",   "categoryNumber", nullptr},
   {XPathRecordLoader::String,  "STYLE_LETTER",      "styleLetter",    nullptr},
   {XPathRecordLoader::String,  "STYLE_GUIDE",       "styleGuide",     nullptr},
   {XPathRecordLoader::Enum,    "TYPE",              "type",           &STYLE_TYPE_MAPPER},
   {XPathRecordLoader::Double,  "OG_MIN",            "ogMin",          nullptr},
   {XPathRecordLoader::Double,  "OG_MAX",            "ogMax",          nullptr},
   {XPathRecordLoader::Double,  "FG_MIN",            "fgMin",          nullptr},
   {XPathRecordLoader::Double,  "FG_MAX",            "fgMax",          nullptr},
   {XPathRecordLoader::Double,  "IBU_MIN",           "ibuMin",         nullptr},
   {XPathRecordLoader::Double,  "IBU_MAX",           "ibuMax",         nullptr},
   {XPathRecordLoader::Double,  "COLOR_MIN",         "colorMin_srm",   nullptr},
   {XPathRecordLoader::Double,  "COLOR_MAX",         "colorMax_srm",   nullptr},
   {XPathRecordLoader::Double,  "CARB_MIN",          "carbMin_vol",    nullptr},
   {XPathRecordLoader::Double,  "CARB_MAX",          "carbMax_vol",    nullptr},
   {XPathRecordLoader::Double,  "ABV_MIN",           "abvMin_pct",     nullptr},
   {XPathRecordLoader::Double,  "ABV_MAX",           "abvMax_pct",     nullptr},
   {XPathRecordLoader::String,  "NOTES",             "notes",          nullptr},
   {XPathRecordLoader::String,  "PROFILE",           "profile",        nullptr},
   {XPathRecordLoader::String,  "INGREDIENTS",       "ingredients",    nullptr},
   {XPathRecordLoader::String,  "EXAMPLES",          "examples",       nullptr},
   {XPathRecordLoader::String,  "DISPLAY_OG_MIN",    nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_OG_MAX",    nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_FG_MIN",    nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_FG_MAX",    nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_COLOR_MIN", nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_COLOR_MAX", nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "OG_RANGE",          nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "FG_RANGE",          nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "IBU_RANGE",         nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "CARB_RANGE",        nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "COLOR_RANGE",       nullptr,          nullptr}, // Extension tag
   {XPathRecordLoader::String,  "ABV_RANGE",         nullptr,          nullptr}  // Extension tag
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QVector<XPathRecordLoader::Field> const EQUIPMENT_RECORD_FIELDS {
   // Type                      XML Name                     Q_PROPERTY               Enum Mapper
   {XPathRecordLoader::String,  "NAME",                      "name",                  nullptr},
   {XPathRecordLoader::UInt,    "VERSION",                   nullptr,                 nullptr},
   {XPathRecordLoader::Double,  "BOIL_SIZE",                 "boilSize_l",            nullptr},
   {XPathRecordLoader::Double,  "BATCH_SIZE",                "batchSize_l",           nullptr},
   {XPathRecordLoader::Double,  "TUN_VOLUME",                "tunVolume_l",           nullptr},
   {XPathRecordLoader::Double,  "TUN_WEIGHT",                "tunWeight_kg",          nullptr},
   {XPathRecordLoader::Double,  "TUN_SPECIFIC_HEAT",         "tunSpecificHeat_calGC", nullptr},
   {XPathRecordLoader::Double,  "TOP_UP_WATER",              "topUpWater_l",          nullptr},
   {XPathRecordLoader::Double,  "TRUB_CHILLER_LOSS",         "trubChillerLoss_l",     nullptr},
   {XPathRecordLoader::Double,  "EVAP_RATE",                 "evapRate_pctHr",        nullptr},
   {XPathRecordLoader::Double,  "BOIL_TIME",                 "boilTime_min",          nullptr},
   {XPathRecordLoader::Bool,    "CALC_BOIL_VOLUME",          "calcBoilVolume",        nullptr},
   {XPathRecordLoader::Double,  "LAUTER_DEADSPACE",          "lauterDeadspace_l",     nullptr},
   {XPathRecordLoader::Double,  "TOP_UP_KETTLE",             "topUpKettle_l",         nullptr},
   {XPathRecordLoader::Double,  "HOP_UTILIZATION",           "hopUtilization_pct",    nullptr},
   {XPathRecordLoader::String,  "NOTES",                     "notes",                 nullptr},
   {XPathRecordLoader::String,  "DISPLAY_BOIL_SIZE",         nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_BATCH_SIZE",        nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TUN_VOLUME",        nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TUN_WEIGHT",        nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TOP_UP_WATER",      nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TRUB_CHILLER_LOSS", nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_LAUTER_DEADSPACE",  nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TOP_UP_KETTLE",     nullptr,                 nullptr}, // Extension tag
   {XPathRecordLoader::Double,  "REAL_EVAP_RATE",            "evapRate_lHr",          nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XPathRecordLoader::Double,  "ABSORPTION",                "grainAbsorption_LKg",   nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XPathRecordLoader::Double,  "BOILING_POINT",             "boilingPoint_c",        nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};


/**
 * \brief BeerXmlSimpleRecordLoader<Hop> specialisation for reading <HOP>...</HOP> BeerXML records into \b Hop objects
 */
template<>
BeerXmlSimpleRecordLoader<Hop>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"HOP",
                                                                                XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                HOP_RECORD_FIELDS,
                                                                                new Hop{"Empty Hop Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Fermentable> specialisation for reading <FERMENTABLE>...</FERMENTABLE> BeerXML
 * records into \b Fermentable objects
 */
template<>
BeerXmlSimpleRecordLoader<Fermentable>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"FERMENTABLE",
                                                                                        XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                        FERMENTABLE_RECORD_FIELDS,
                                                                                        new Fermentable{"Empty Fermentable Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Yeast> specialisation for reading <YEAST>...</YEAST> BeerXML records into \b Yeast
 * objects
 */
template<>
BeerXmlSimpleRecordLoader<Yeast>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"YEAST",
                                                                                  XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                  YEAST_RECORD_FIELDS,
                                                                                  new Yeast{"Empty Yeast Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Misc> specialisation for reading <MISC>...</MISC> BeerXML records into \b Misc
 * objects
 */
template<>
BeerXmlSimpleRecordLoader<Misc>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"MISC",
                                                                                 XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                 MISC_RECORD_FIELDS,
                                                                                 new Misc{"Empty Misc Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Water> specialisation for reading <WATER>...</WATER> BeerXML records into \b Water
 * objects
 */
template<>
BeerXmlSimpleRecordLoader<Water>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"WATER",
                                                                                  XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                  WATER_RECORD_FIELDS,
                                                                                  new Water{"Empty Water Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Style> specialisation for reading <STYLE>...</STYLE> BeerXML records into \b Style
 * objects
 */
template<>
BeerXmlSimpleRecordLoader<Style>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"STYLE",
                                                                                  XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                  STYLE_RECORD_FIELDS,
                                                                                  new Style{"Empty Style Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecordLoader<Equipment> specialisation for reading <EQUIPMENT>...</EQUIPMENT> BeerXML records
 * into \b Equipment objects
 */
template<>
BeerXmlSimpleRecordLoader<Equipment>::BeerXmlSimpleRecordLoader() : XPathRecordLoader{"EQUIPMENT",
                                                                                      XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                      EQUIPMENT_RECORD_FIELDS,
                                                                                      new Equipment{"Empty Equipment Object"}} {
   return;
}
