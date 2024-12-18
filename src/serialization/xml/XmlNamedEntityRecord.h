/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlNamedEntityRecord.h is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#ifndef SERIALIZATION_XML_XMLNAMEDENTITYRECORD_H
#define SERIALIZATION_XML_XMLNAMEDENTITYRECORD_H
#pragma once

#include <QDebug>
#include <QString>
#include <QList>

#include "database/ObjectStoreWrapper.h"
#include "model/BrewNote.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedEntity.h"
#include "model/Recipe.h"
#include "utils/TypeLookup.h"
#include "serialization/NamedEntityRecordBase.h"
#include "serialization/xml/XmlRecord.h"
#include "serialization/xml/XQString.h"


/**
 * \brief Provides class-specific extensions to \b XmlRecord.  See comment in xml/XmlCoding.h for more details.
 */
template<class NE>
class XmlNamedEntityRecord : public XmlRecord,
                             public Serialization::NamedEntityRecordBase<XmlNamedEntityRecord<NE>, NE> {
public:
   /**
    * \brief This constructor doesn't have to do much more than create an appropriate new subclass of \b NamedEntity.
    *        Everything else is done in the base class.
    */
   XmlNamedEntityRecord(XmlCoding           const & xmlCoding,
                        XmlRecordDefinition const & recordDefinition) :
      XmlRecord{xmlCoding, recordDefinition},
      Serialization::NamedEntityRecordBase<XmlNamedEntityRecord<NE>, NE>{} {
///      this->m_includeInStats = this->includedInStats();
      return;
   }

   NAMED_ENTITY_RECORD_COMMON_DECL(XmlNamedEntityRecord, NE)

};

#endif
