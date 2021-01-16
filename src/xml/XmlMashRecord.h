/*
 * xml/XmlMashRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLMASHRECORD_H
#define _XML_XMLMASHRECORD_H
#pragma once

#include "xml/XmlNamedEntityRecord.h"
#include "xml/XmlMashStepRecord.h"

#include "mash.h"


/**
 * \brief Loads a \b Mash record in from an XML file, including the \b MashStep records it
 * contains (via \b XmlMashStepRecord).
 */
class XmlMashRecord : public XmlNamedEntityRecord<Mash> {
public:
   XmlMashRecord(XmlCoding const & xmlCoding,
                        QString const recordName,
                        XmlRecord::FieldDefinitions const & fieldDefinitions) :
   XmlNamedEntityRecord<Mash>{xmlCoding,
                              recordName,
                              fieldDefinitions} { return; }

   virtual bool normaliseAndStoreInDb(NamedEntity * containingEntity,
                                      QTextStream & userMessage,
                                      XmlRecordCount & stats);
};

#endif
