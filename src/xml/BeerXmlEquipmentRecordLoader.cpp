/*
 * xml/BeerXmlEquipmentRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlEquipmentRecordLoader.h"

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
   {XPathRecordLoader::Double,    "REAL_EVAP_RATE",          "evapRate_lHr",          nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XPathRecordLoader::Double,    "ABSORPTION",              "grainAbsorption_LKg",   nullptr}, // Non-standard tag, not part of BeerXML 1.0 standard
   {XPathRecordLoader::Double,    "BOILING_POINT",           "boilingPoint_c",        nullptr}  // Non-standard tag, not part of BeerXML 1.0 standard
};

BeerXmlEquipmentRecordLoader::BeerXmlEquipmentRecordLoader() : XPathRecordLoader{"EQUIPMENT",
                                                                                 XPathRecordLoader::EachInstanceNameShouldBeUnique,
                                                                                 EQUIPMENT_RECORD_FIELDS,
                                                                                 new Equipment{"Empty Equipment Object"},
                                                                                 "equipments"} {
   return;
}
