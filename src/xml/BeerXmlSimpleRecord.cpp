/*
 * xml/BeerXmlSimpleRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlSimpleRecord.h"

#include "hop.h"
#include "fermentable.h"
#include "yeast.h"
#include "misc.h"
#include "water.h"
#include "style.h"
#include "equipment.h"
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <HOP>...</HOP> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static XmlRecord::EnumLookupMap const HOP_USE_MAPPER {
   {"Boil",       Hop::Boil},
   {"Dry Hop",    Hop::Dry_Hop},
   {"Mash",       Hop::Mash},
   {"First Wort", Hop::First_Wort},
   {"Aroma",      Hop::UseAroma}
};

static XmlRecord::EnumLookupMap const HOP_TYPE_MAPPER {
   {"Bittering", Hop::Bittering},
   {"Aroma",     Hop::Aroma},
   {"Both",      Hop::Both}
};

static XmlRecord::EnumLookupMap const HOP_FORM_MAPPER {
   {"Pellet", Hop::Pellet},
   {"Plug",   Hop::Plug},
   {"Leaf",   Hop::Leaf}
};

static QVector<XmlRecord::Field> const HOP_RECORD_FIELDS {
   // Type              XML Name          Q_PROPERTY           Enum Mapper
   {XmlRecord::String,  "NAME",           "name",              nullptr},
   {XmlRecord::UInt,    "VERSION",        nullptr,             nullptr},
   {XmlRecord::Double,  "ALPHA",          "alpha_pct",         nullptr},
   {XmlRecord::Double,  "AMOUNT",         "amount_kg",         nullptr},
   {XmlRecord::Enum,    "USE",            "use",               &HOP_USE_MAPPER},
   {XmlRecord::Double,  "TIME",           "time_min",          nullptr},
   {XmlRecord::String,  "NOTES",          "notes",             nullptr},
   {XmlRecord::Enum,    "TYPE",           "type",              &HOP_TYPE_MAPPER},
   {XmlRecord::Enum,    "FORM",           "form",              &HOP_FORM_MAPPER},
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
static XmlRecord::EnumLookupMap const FERMENTABLE_TYPE_MAPPER {
   {"Grain",       Fermentable::Grain},
   {"Sugar",       Fermentable::Sugar},
   {"Extract",     Fermentable::Extract},
   {"Dry Extract", Fermentable::Dry_Extract},
   {"Adjunct",     Fermentable::Adjunct}
};

static QVector<XmlRecord::Field> const FERMENTABLE_RECORD_FIELDS {
   // Type                     XML Name             Q_PROPERTY                Enum Mapper
   {XmlRecord::String,  "NAME",             "name",                   nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,                  nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",                   &FERMENTABLE_TYPE_MAPPER},
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
static XmlRecord::EnumLookupMap const YEAST_TYPE_MAPPER {
   {"Ale",       Yeast::Ale},
   {"Lager",     Yeast::Lager},
   {"Wheat",     Yeast::Wheat},
   {"Wine",      Yeast::Wine},
   {"Champagne", Yeast::Champagne}
};

static XmlRecord::EnumLookupMap const YEAST_FORM_MAPPER {
   {"Liquid",  Yeast::Liquid},
   {"Dry",     Yeast::Dry},
   {"Slant",   Yeast::Slant},
   {"Culture", Yeast::Culture}
};

static XmlRecord::EnumLookupMap const YEAST_FLOCCULATION_MAPPER {
   {"Low",       Yeast::Low},
   {"Medium",    Yeast::Medium},
   {"High",      Yeast::High},
   {"Very High", Yeast::Very_High}
};

static QVector<XmlRecord::Field> const YEAST_RECORD_FIELDS {
   // Type                      XML Name            Q_PROPERTY                Enum Mapper
   {XmlRecord::String,  "NAME",             "name",                   nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,                  nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",                   &YEAST_TYPE_MAPPER},
   {XmlRecord::Enum,    "TYPE",             "type",                   &YEAST_FORM_MAPPER},
   {XmlRecord::Double,  "AMOUNT",           "amount",                 nullptr},
   {XmlRecord::Bool,    "AMOUNT_IS_WEIGHT", "amountIsWeight",         nullptr},
   {XmlRecord::String,  "LABORATORY",       "laboratory",             nullptr},
   {XmlRecord::String,  "PRODUCT_ID",       "productID",              nullptr},
   {XmlRecord::Double,  "MIN_TEMPERATURE",  "minTemperature_c",       nullptr},
   {XmlRecord::Double,  "MAX_TEMPERATURE",  "maxTemperature_c",       nullptr},
   {XmlRecord::Enum,    "FLOCCULATION",     "flocculation",           &YEAST_FLOCCULATION_MAPPER},
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
static XmlRecord::EnumLookupMap const MISC_TYPE_MAPPER {
   {"Spice",       Misc::Spice},
   {"Fining",      Misc::Fining},
   {"Water Agent", Misc::Water_Agent},
   {"Herb",        Misc::Herb},
   {"Flavor",      Misc::Flavor},
   {"Other",       Misc::Other}
};

static XmlRecord::EnumLookupMap const MISC_USE_MAPPER {
   {"Boil",      Misc::Boil},
   {"Mash",      Misc::Mash},
   {"Primary",   Misc::Primary},
   {"Secondary", Misc::Secondary},
   {"Bottling",  Misc::Bottling}
};

static QVector<XmlRecord::Field> const MISC_RECORD_FIELDS {
   // Type                      XML Name            Q_PROPERTY        Enum Mapper
   {XmlRecord::String,  "NAME",             "name",           nullptr},
   {XmlRecord::UInt,    "VERSION",          nullptr,          nullptr},
   {XmlRecord::Enum,    "TYPE",             "type",           &MISC_TYPE_MAPPER},
   {XmlRecord::Enum,    "USE",              "use",            &MISC_USE_MAPPER},
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
static QVector<XmlRecord::Field> const WATER_RECORD_FIELDS {
   // Type                      XML Name         Q_PROPERTY         Enum Mapper
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
static XmlRecord::EnumLookupMap const STYLE_TYPE_MAPPER {
   {"Lager", Style::Lager},
   {"Ale",   Style::Ale},
   {"Mead",  Style::Mead},
   {"Wheat", Style::Wheat},
   {"Mixed", Style::Mixed},
   {"Cider", Style::Cider}
};

static QVector<XmlRecord::Field> const STYLE_RECORD_FIELDS {
   // Type                      XML Name             Q_PROPERTY        Enum Mapper
   {XmlRecord::String,  "NAME",              "name",           nullptr},
   {XmlRecord::String,  "CATEGORY",          "category",       nullptr},
   {XmlRecord::UInt,    "VERSION",           nullptr,          nullptr},
   {XmlRecord::String,  "CATEGORY_NUMBER",   "categoryNumber", nullptr},
   {XmlRecord::String,  "STYLE_LETTER",      "styleLetter",    nullptr},
   {XmlRecord::String,  "STYLE_GUIDE",       "styleGuide",     nullptr},
   {XmlRecord::Enum,    "TYPE",              "type",           &STYLE_TYPE_MAPPER},
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
// Field mappings for <EQUIPMENT>...</EQUIPMENT> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QVector<XmlRecord::Field> const EQUIPMENT_RECORD_FIELDS {
   // Type                      XML Name                     Q_PROPERTY               Enum Mapper
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
*/

/**
 * \brief BeerXmlSimpleRecord<Hop> specialisation for reading <HOP>...</HOP> BeerXML records into \b Hop objects
 *
template<>
BeerXmlSimpleRecord<Hop>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "HOP",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        HOP_RECORD_FIELDS,
                        new Hop{"Empty Hop Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Fermentable> specialisation for reading <FERMENTABLE>...</FERMENTABLE> BeerXML
 * records into \b Fermentable objects
 *
template<>
BeerXmlSimpleRecord<Fermentable>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "FERMENTABLE",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        FERMENTABLE_RECORD_FIELDS,
                        new Fermentable{"Empty Fermentable Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Yeast> specialisation for reading <YEAST>...</YEAST> BeerXML records into \b Yeast
 * objects
 *
template<>
BeerXmlSimpleRecord<Yeast>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "YEAST",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        YEAST_RECORD_FIELDS,
                        new Yeast{"Empty Yeast Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Misc> specialisation for reading <MISC>...</MISC> BeerXML records into \b Misc
 * objects
 *
template<>
BeerXmlSimpleRecord<Misc>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "MISC",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        MISC_RECORD_FIELDS,
                        new Misc{"Empty Misc Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Water> specialisation for reading <WATER>...</WATER> BeerXML records into \b Water
 * objects
 *
template<>
BeerXmlSimpleRecord<Water>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "WATER",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        WATER_RECORD_FIELDS,
                        new Water{"Empty Water Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Style> specialisation for reading <STYLE>...</STYLE> BeerXML records into \b Style
 * objects
 *
template<>
BeerXmlSimpleRecord<Style>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "STYLE",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        STYLE_RECORD_FIELDS,
                        new Style{"Empty Style Object"}} {
   return;
}

/**
 * \brief BeerXmlSimpleRecord<Equipment> specialisation for reading <EQUIPMENT>...</EQUIPMENT> BeerXML records
 * into \b Equipment objects
 *
template<>
BeerXmlSimpleRecord<Equipment>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "EQUIPMENT",
                        XmlNamedEntityRecord::EachInstanceNameShouldBeUnique,
                        EQUIPMENT_RECORD_FIELDS,
                        new Equipment{"Empty Equipment Object"}} {
   return;
}
*/
