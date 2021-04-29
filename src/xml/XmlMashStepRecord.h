/*
 * xml/XmlMashStepRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLMASHSTEPRECORD_H
#define _XML_XMLMASHSTEPRECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "model/MashStep.h"

/**
 * \brief Loads a \b MashStep record in from an XML file.
 */
class XmlMashStepRecord : public XmlNamedEntityRecord<MashStep> {
public:
   // We only want to override one method, so the parent class's constructors are fine for us
   using XmlNamedEntityRecord<MashStep>::XmlNamedEntityRecord;

   /**
    * \brief We override \b XmlNamedEntityRecord<MashStep>::normaliseAndStoreInDb because a MashStep can only be stored
    *        in the DB other in association with its Mash.
    * \param containingEntity The Mash with which the MashStep needs to be associated
    */
   virtual XmlRecord::ProcessingResult normaliseAndStoreInDb(NamedEntity * containingEntity,
                                                             QTextStream & userMessage,
                                                             XmlRecordCount & stats);
};
#endif
