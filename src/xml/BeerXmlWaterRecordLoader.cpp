/*
 * xml/BeerXmlWaterRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlWaterRecordLoader.h"

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

BeerXmlWaterRecordLoader::BeerXmlWaterRecordLoader() : XPathRecordLoader{"WATER",
                                                                         XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                         WATER_RECORD_FIELDS,
                                                                         new Water{"Empty Water Object"},
                                                                         "waters"} {
   return;
}
