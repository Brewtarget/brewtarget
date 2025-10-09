/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlMashRecord.h is part of Brewtarget, and is copyright the following authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef SERIALIZATION_XML_XMLMASHRECORD_H
#define SERIALIZATION_XML_XMLMASHRECORD_H
#pragma once

#include "serialization/xml/XmlNamedEntityRecord.h"
#include "model/Mash.h"

/**
 * \brief Read and write a \c Mash record (including any records it contains) from or to an XML file
 */
class XmlMashRecord : public XmlNamedEntityRecord<Mash> {
public:
   // We only want to override a couple of member functions, so the parent class's constructors are fine for us
   using XmlNamedEntityRecord<Mash>::XmlNamedEntityRecord;

protected:
   /**
    * \brief We need to override \c XmlRecord::propertiesToXml for similar reasons that that
    *        \c XmlRecipeRecord does.  (Note that we do not need to override \c XmlRecord::normaliseAndStoreInDb as the
    *        connection between a \c MashStep and its \c Mash is handled in
    *        \c Serialization::NamedEntityRecordBase::doSetContainingEntity.)
    */
   virtual void subRecordToXml(XmlRecordDefinition::FieldDefinition const & fieldDefinition,
                               XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               QTextStream & out,
                               int indentLevel,
                               char const * const indentString) const;
};

#endif
