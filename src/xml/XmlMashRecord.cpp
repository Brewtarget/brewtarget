/*
 * xml/XmlMashRecord.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlMashRecord.h"

#include <memory> // For smart pointers

#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "database.h"

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
bool XmlMashRecord::normaliseAndStoreInDb(NamedEntity * containingEntity,
                                          QTextStream & userMessage,
                                          XmlRecordCount & stats) {
   //
   // The base class does all the heavy lifting for us - storing the Mash and the contained MashSteps.
   // If that succeeds, we're done.
   //
   if (XmlRecord::normaliseAndStoreInDb(containingEntity, userMessage, stats)) {
      return true;
   }

   //
   // Otherwise we may have some clean-up to do.  Specifically, if we stored the Mash and then hit a problem
   // storing one of the MashSteps, we want to remove the Mash and any of its MashSteps that did get stored.
   //
   if (nullptr == this->namedEntityRaiiContainer.get()) {
      //
      // We released ownership of the Mash pointer, which means it must have been stored in the database.
      // Since MashStep objects are associated only with their owning Mash (ie not shared between different Mash
      // objects), all we should need to do is delete the Mash the base class just stored, and let the existing
      // logic handle deleting MashSteps owned by that Mash.
      //
      Database::instance().remove(static_cast<Mash *>(this->namedEntity));

      //
      // And now we deleted it from the DB, we own it again
      //
      this->namedEntityRaiiContainer.reset(this->namedEntity);
   }

   return false;
}
