/*
 * xml/BeerXmlMiscRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlMiscRecordLoader.h"

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

BeerXmlMiscRecordLoader::BeerXmlMiscRecordLoader() : XPathRecordLoader{"MISC",
                                                                       XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                       MISC_RECORD_FIELDS,
                                                                       new Misc{"Empty Misc Object"},
                                                                       "miscs"} {
   return;
}
