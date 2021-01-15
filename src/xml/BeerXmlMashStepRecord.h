/*
 * xml/BeerXmlMashStepRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BEERXMLMASHSTEPRECORD_H
#define _XML_BEERXMLMASHSTEPRECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "mashstep.h"

class Mash;

/**
 * \brief Loads a <MASH_STEP>...</MASH_STEP> record in from a BeerXML file.  See comment in xml/XPathRecord.h for
 * more complete explanation.
 */
class BeerXmlMashStepRecord : public XmlNamedEntityRecord<MashStep> {
public:
//   BeerXmlMashStepRecord(XmlCoding const & xmlCoding);

   BeerXmlMashStepRecord(XmlCoding const & xmlCoding,
                        QString const recordName,
                        XmlRecord::FieldDefinitions const & fieldDefinitions) :
   XmlNamedEntityRecord<MashStep>{xmlCoding,
             recordName,
             fieldDefinitions} { return; }

   virtual bool load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage,
                     std::shared_ptr< QHash<QString, QVariant> > fieldsRead = nullptr);

   /**
    * \brief A MashStep does not get stored in the DB other than in association with a Mash, so it's simpler to have
    *        the Mash first.
    * \param mash The Mash with which the MashStep needs to be associated
    */
   virtual bool normaliseAndStoreInDb(Mash * mash, QTextStream & userMessage);
};
#endif
