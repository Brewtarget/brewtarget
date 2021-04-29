/*
 * xml/XmlRecipeRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLRECIPERECORD_H
#define _XML_XMLRECIPERECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "model/Recipe.h"

/**
 * \brief Loads a \b Recipe record (including any records it contains) in from an XML file.
 */
class XmlRecipeRecord : public XmlNamedEntityRecord<Recipe> {
public:
   // We only want to override one method, so the parent class's constructors are fine for us
   using XmlNamedEntityRecord<Recipe>::XmlNamedEntityRecord;
/*
         <!-- EFFICIENCY is not required for Extract but is required for other types
            so, ideally, we would have an assert here along the following lines:
               <xs:assert test="EFFICIENCY or (TYPE = 'Extract')"/>
            However, XSD asserts are part of the XML Schema 1.1 specifications and Xerces-C++
            (in contrast to Xerces-J) only supports XML Schema 1.0 - ie it doesn't understand
            the assert tag. -->


         <!-- The BeerXML 1.0 Standard defines the next field as required, but then goes on to say in its description
              that 'No Mash record is needed for “Extract” type brews'.  We'll take the description as accurate.
              As noted above for EFFICIENCY and INFUSE_AMOUNT, we ideally would include an xs:assert tag here to
              enforce the circumstances in which a MASH record is required, but this will have to wait until Xerces-C++
              supports the XML Schema 1.1 specification. -->
         <xs:element name="MASH" type="MashType"/> <!-- NB singular not plural here -->


         <xs:element name="FORCED_CARBONATION" type="CapsBoolean" minOccurs="0" maxOccurs="1"/>
         <!-- The next field should only be present if the previous one was present and set to TRUE
              See comments elsewhere about the desirability of enforcing this in future via xs:assert tags -->
         <xs:element name="PRIMING_SUGAR_NAME" type="xs:string" minOccurs="0" maxOccurs="1"/>


*/
   /**
    * \brief We override \b XmlNamedEntityRecord<Recipe>::normaliseAndStoreInDb because ... TO WRITE
    * \param containingEntity
    */
   virtual XmlRecord::ProcessingResult normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                             QTextStream & userMessage,
                                                             XmlRecordCount & stats);

private:
   /**
    * \brief Add to the recipe child (ie contained) objects of type CNE that have already been read in and stored
    */
   template<typename CNE> void addChildren();
};
#endif
