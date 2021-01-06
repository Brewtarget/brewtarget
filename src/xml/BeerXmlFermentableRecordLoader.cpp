/*
 * xml/BeerXmlFermentableRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlFermentableRecordLoader.h"

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

BeerXmlFermentableRecordLoader::BeerXmlFermentableRecordLoader() :
   XPathRecordLoader{"FERMENTABLE",
                     XPathRecordLoader::EachInstanceNameShouldBeUnique,
                     FERMENTABLE_RECORD_FIELDS,
                     new Fermentable{"Empty Fermentable Object"},
                     "fermentables"} {
   return;
}
