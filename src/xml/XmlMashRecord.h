/*
 * XmlMashRecord.h is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef XML_XMLMASHRECORD_H
#define XML_XMLMASHRECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
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
    *        connection between a \c MashStep and its \c Mash is handled in \c XmlMashStepRecord::setContainingEntity.)
    */
   virtual void subRecordToXml(XmlRecord::FieldDefinition const & fieldDefinition,
                               XmlRecord const & subRecord,
                               NamedEntity const & namedEntityToExport,
                               QTextStream & out,
                               int indentLevel,
                               char const * const indentString) const;

   /**
    * \brief We need to know about our containing entity to decide whether to include the Mash record in the stats.
    *
    *        If the Mash is outside a Recipe, then we DO want to include it in stats.  If it's inside a Recipe then we
    *        don't call it out with a separate stats entry.  It suffices to tell the user how many Recipes we read in
    *        without also counting how many Mashes inside Recipes we read.
    *
    *        Additionally, if the Recipe gets deleted after being read in (because at that point we determine it's a
    *        duplicate), this means we don't have to try to unpick stats about Mashes.
    */
   virtual void setContainingEntity(std::shared_ptr<NamedEntity> containingEntity);
};

#endif
