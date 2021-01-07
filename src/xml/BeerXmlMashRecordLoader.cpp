/*
 * xml/BeerXmlMashRecordLoader.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/BeerXmlMashRecordLoader.h"

#include <memory> // For smart pointers

#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "database.h"

static QVector<XPathRecordLoader::Field> const MASH_RECORD_FIELDS {
   // Type                      XML Name              Q_PROPERTY               Enum Mapper
   {XPathRecordLoader::String,  "NAME",               "name",                  nullptr},
   {XPathRecordLoader::UInt,    "VERSION",            nullptr,                 nullptr},
   {XPathRecordLoader::Double,  "GRAIN_TEMP",         "grainTemp_c",           nullptr},
   //                           "MASH-STEPS" is handled in code
   {XPathRecordLoader::String,  "NOTES",              "notes",                 nullptr},
   {XPathRecordLoader::Double,  "TUN_TEMP",           "tunTemp_c",             nullptr},
   {XPathRecordLoader::Double,  "SPARGE_TEMP",        "spargeTemp_c",          nullptr},
   {XPathRecordLoader::Double,  "PH",                 "ph",                    nullptr},
   {XPathRecordLoader::Double,  "TUN_WEIGHT",         "tunWeight_kg",          nullptr},
   {XPathRecordLoader::Double,  "TUN_SPECIFIC_HEAT",  "tunSpecificHeat_calGC", nullptr},
   {XPathRecordLoader::Bool,    "EQUIP_ADJUST",       "equipAdjust",           nullptr},
   {XPathRecordLoader::String,  "DISPLAY_GRAIN_TEMP",  nullptr,                nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TUN_TEMP",    nullptr,                nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_SPARGE_TEMP", nullptr,                nullptr}, // Extension tag
   {XPathRecordLoader::String,  "DISPLAY_TUN_WEIGHT",  nullptr,                nullptr}  // Extension tag
};

BeerXmlMashRecordLoader::BeerXmlMashRecordLoader() : XPathRecordLoader{"MASH",
                                                                       XPathRecordLoader::InstancesWithDuplicateNamesOk,
                                                                       MASH_RECORD_FIELDS,
                                                                       new Mash{"Empty Mash Object"}},
                                                     mashStepRecordLoaders{} {
   return;
}

bool BeerXmlMashRecordLoader::load(xalanc::DOMSupport & domSupport,
                                   xalanc::XalanNode * rootNodeOfRecord,
                                   QTextStream & userMessage) {
   qDebug() << Q_FUNC_INFO;
   // Attempt to load in all the scalar fields first.  If this fails then we bail and don't bother trying to read in
   // the MASH_STEP records.
   if (!XPathRecordLoader::load(domSupport, rootNodeOfRecord, userMessage)) {
      return false;
   }

   // Now get all the MASH_STEP nodes inside the MASH_STEP one and turn them into MashStep objects
   xalanc::XPathEvaluator xPathEvaluator;
   xalanc::NodeRefList listOfMashStepNodes;
   xPathEvaluator.selectNodeList(listOfMashStepNodes,
                                 domSupport,
                                 rootNodeOfRecord,
                                 XQString("MASH_STEPS/MASH_STEP").getXalanString());
   auto numMashSteps = listOfMashStepNodes.getLength();
   qDebug() << Q_FUNC_INFO << "Found " << numMashSteps << " MASH_STEP nodes (inside MASH_STEPS)";

   // It's not 100% explicit in the BeerXML 1.0 specification, but it seems unlikely to be the intent that a MASH
   // with no steps is valid
   if (0 == numMashSteps) {
      Mash * mash = dynamic_cast<Mash *>(this->entityToPopulate.get());
      userMessage << "Mash " << mash->name() << " had no mash steps!";
      return false;
   }

   // Now we can use one BeerXmlMashStepRecordLoader per MASH_STEP to do the processing
   for (xalanc::NodeRefListBase::size_type ii = 0; ii < numMashSteps; ++ii) {
      xalanc::XalanNode * mashStepNode = listOfMashStepNodes.item(ii);

      std::shared_ptr<BeerXmlMashStepRecordLoader> mashStepRecordLoader{new BeerXmlMashStepRecordLoader{}};
      this->mashStepRecordLoaders.append(mashStepRecordLoader);

      if (!mashStepRecordLoader->load(domSupport, mashStepNode, userMessage)) {
         // If we couldn't parse one of the mash steps, there's no point soldiering on
         return false;
      }
   }

   // If we got this far, then we haven't hit any fatal errors
   return true;
}

bool BeerXmlMashRecordLoader::normalise(QTextStream & userMessage) {
   // Do normalisation on all the MashStep objects we read in
   for (auto ii = this->mashStepRecordLoaders.begin(); ii < this->mashStepRecordLoaders.end(); ++ii) {
      auto mashStepRecordLoader = ii->get();
      if (!mashStepRecordLoader->normalise(userMessage)) {
         // If we couldn't normalise one of the mash steps, there's no point soldiering on
         return false;
      }
   }

   // Call the base class method to do remaining normalisation on the Mash
   return XPathRecordLoader::normalise(userMessage);
}

bool BeerXmlMashRecordLoader::storeInDb(QTextStream & userMessage) {
   //
   // It's easier to store the Mash first.  If that fails, there's no DB clean-up to do and we can bail out.
   // Conversely, once the Mash is stored, adding a MashStep via its public API then automatically stores that in the
   // DB.
   //
   // But first, get the pointer to the Mash object, as it will be released by storeInDb()
   //
   Mash * mash = dynamic_cast<Mash *>(this->entityToPopulate.get());
   if (!XPathRecordLoader::storeInDb(userMessage)) {
      return false;
   }

   for (auto ii = this->mashStepRecordLoaders.begin(); ii < this->mashStepRecordLoaders.end(); ++ii) {
      auto mashStepRecordLoader = ii->get();
      if (!mashStepRecordLoader->storeInDb(mash, userMessage)) {
         //
         // If storing the MashStep failed, we're going to bail, but we should clean up first.  Since MashStep objects
         // are associated only with their owning Mash (ie not shared between different Mash objects), all we should
         // need to do is delete the mash we just stored above, and let the existing logic handle deleting MashSteps
         // owned by that Mash.
         //
         Database::instance().remove(mash);
         return false;
      }
   }

   return true;
}
