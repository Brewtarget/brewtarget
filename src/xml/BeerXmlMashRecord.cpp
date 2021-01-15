/*
 * xml/BeerXmlMashRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlMashRecord.h"

#include <memory> // For smart pointers

#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "database.h"
/*
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Field mappings for <MASH>...</MASH> BeerXML records
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static QVector<XmlRecord::Field> const MASH_RECORD_FIELDS {
   // Type               XPath                  Q_PROPERTY               Enum Mapper
   {XmlRecord::String,  "NAME",                 "name",                  nullptr},
   {XmlRecord::UInt,    "VERSION",              nullptr,                 nullptr},
   {XmlRecord::Double,  "GRAIN_TEMP",           "grainTemp_c",           nullptr},
   {XmlRecord::Records, "MASH_STEPS/MASH_STEP", nullptr,                 nullptr}, // Additional logic for "MASH-STEPS" is handled in code
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
*/

/**
 * \brief BeerXmlSimpleRecord<Mash> specialisation for reading <MASH>...</MASH> BeerXML records
 * into \b Mash objects.  Note that this class is further specialised by \b BeerXmlMashRecord
 *
template<>
BeerXmlSimpleRecord<Mash>::BeerXmlSimpleRecord(XmlCoding const & xmlCoding) :
   XmlNamedEntityRecord{xmlCoding,
                        "MASH",
                        XmlNamedEntityRecord::InstancesWithDuplicateNamesOk,
                        MASH_RECORD_FIELDS,
                        new Mash{"Empty Mash Object"}} {
   return;
}

BeerXmlMashRecord::BeerXmlMashRecord(XmlCoding const & xmlCoding) :
   BeerXmlSimpleRecord<Mash>{xmlCoding} {
   return;
}
*/
bool BeerXmlMashRecord::normaliseAndStoreInDb(QTextStream & userMessage,
                                              XmlRecordCount & stats) {
   //
   // It's easier to store the Mash first.  If that fails, there's no DB clean-up to do and we can bail out.
   // Conversely, once the Mash is stored, adding a MashStep via its public API then automatically stores that in the
   // DB.
   //
   // But first, get the pointer to the Mash object, as it will be released by storeInDb()
   //
   Mash * mash = dynamic_cast<Mash *>(this->entityToPopulate.get());
   if (!XmlNamedEntityRecord::normaliseAndStoreInDb(userMessage, stats)) {
      return false;
   }

   //
   // Now we can store the MashStep objects.  Note that we don't tell the user how many MashStep records we read in
   // because they are not "free-standing" -- ie they only exist as part of a Mash.
   // The base class already knows how to store child objects, so it's a single call for us.
   //
   // TODO We need to tell each MashStep which Mash it's in
   //
   if (!XmlRecord::normaliseAndStoreInDb(userMessage, stats)) {
      //
      // If storing the MashStep failed, we're going to bail, but we should clean up first.  Since MashStep objects
      // are associated only with their owning Mash (ie not shared between different Mash objects), all we should
      // need to do is delete the mash we just stored above, and let the existing logic handle deleting MashSteps
      // owned by that Mash.
      //
      Database::instance().remove(mash);
      return false;
   }

   return true;
}
