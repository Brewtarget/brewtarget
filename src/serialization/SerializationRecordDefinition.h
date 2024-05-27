/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/SerializationRecordDefinition.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_SERIALIZATIONRECORDDEFINITION_H
#define SERIALIZATION_SERIALIZATIONRECORDDEFINITION_H
#pragma once

#include <memory>

#include <QList>
#include <QString>
#include <QVariant>

#include "model/NamedEntity.h"
#include "utils/BtStringConst.h"
#include "utils/TypeLookup.h"

/**
 * \brief Common base class for \c XmlRecordDefinition and \c JsonRecordDefinition
 *
 * TODO: We could probably do some templating to bring more up into this class
 */
class SerializationRecordDefinition {
public:
   /**
    * \param recordName The name of the XML or JSON object for this type of record, eg "fermentables" for a list of
    *                   fermentables in BeerXML.
    */
   SerializationRecordDefinition(char       const *            const   recordName,
                                 TypeLookup const *            const   typeLookup,
                                 char       const *            const   namedEntityClassName,
                                 QString                       const & localisedEntityName,
                                 NamedEntity::UpAndDownCasters const   upAndDownCasters) :
      m_recordName          {recordName},
      m_typeLookup          {typeLookup},
      m_namedEntityClassName{namedEntityClassName},
      m_localisedEntityName {localisedEntityName},
      m_upAndDownCasters    {upAndDownCasters} {
      return;
   }
   ~SerializationRecordDefinition() = default;

   BtStringConst const      m_recordName;

   TypeLookup const * const m_typeLookup;

   /**
    * The name of the class of object contained in this type of record, eg "Hop", "Yeast", etc.
    * Blank for the root record (which is just a container and doesn't have a NamedEntity).
    */
   BtStringConst const m_namedEntityClassName;

   /**
    * The localised name of the object, suitable for showing on the screen (eg if we want to tell the user how many Hop
    * records were read in, etc).
    */
   QString const m_localisedEntityName;

   NamedEntity::UpAndDownCasters const m_upAndDownCasters;

///   //! Pointer to relevant NamedEntity::upcastListToVariant function
///   QVariant                            (*m_listUpcaster)(QList<std::shared_ptr<NamedEntity>> const &);
///   //! Pointer to relevant NamedEntity::downcastListFromVariant function
///   QList<std::shared_ptr<NamedEntity>> (*m_listDowncaster)(QVariant const &);

};

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, SerializationRecordDefinition const & serializationRecordDefinition) {
   stream <<
      serializationRecordDefinition.m_recordName << "(" << serializationRecordDefinition.m_namedEntityClassName << ")";
   return stream;
}

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, SerializationRecordDefinition const * serializationRecordDefinition) {
   if (serializationRecordDefinition) {
      stream << *serializationRecordDefinition;
   } else {
      stream << "nullptr";
   }
   return stream;
}

#endif
