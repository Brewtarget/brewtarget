/*
 * xml/BeerXmlStyleRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlStyleRecordLoader.h"

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

BeerXmlStyleRecordLoader::BeerXmlStyleRecordLoader() : XPathRecordLoader{"STYLE",
                                                                         XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                         STYLE_RECORD_FIELDS,
                                                                         new Style{"Empty Style Object"},
                                                                         "styles"} {
   return;
}
