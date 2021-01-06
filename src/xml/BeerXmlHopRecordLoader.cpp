/*
 * xml/BeerXmlHopRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlHopRecordLoader.h"

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

BeerXmlHopRecordLoader::BeerXmlHopRecordLoader() : XPathRecordLoader{"HOP",
                                                                     XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                     HOP_RECORD_FIELDS,
                                                                     new Hop{"Empty Hop Object"},
                                                                     "hops"} {
   return;
}
