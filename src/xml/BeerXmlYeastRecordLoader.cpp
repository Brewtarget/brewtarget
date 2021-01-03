/*
 * xml/BeerXmlYeastRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlYeastRecordLoader.h"

#include <QVector>

#include "yeast.h"


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
   // Type                     XML Name             Q_PROPERTY                Enum Mapper
   {XPathRecordLoader::String,  "NAME",             "name",                   nullptr},
   {XPathRecordLoader::String,  "VERSION",          nullptr,                  nullptr},
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
   {XPathRecordLoader::String,  "DISPLAY_AMOUNT",   nullptr,                  nullptr},
   {XPathRecordLoader::String,  "DISP_MIN_TEMP",    nullptr,                  nullptr},
   {XPathRecordLoader::String,  "DISP_MAX_TEMP",    nullptr,                  nullptr},
   {XPathRecordLoader::String,  "INVENTORY",        nullptr,                  nullptr},
   {XPathRecordLoader::String,  "CULTURE_DATE",     nullptr,                  nullptr}
};

BeerXmlYeastRecordLoader::BeerXmlYeastRecordLoader() : XPathRecordLoader{"YEAST",
                                                                         YEAST_RECORD_FIELDS,
                                                                         new Yeast{"Empty Yeast Object"}} {
   return;
}
