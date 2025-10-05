/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlRecordDefinition.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/xml/XmlRecordDefinition.h"

#include <QDebug>

#include "utils/EnumStringMapping.h"
#include "serialization/xml/XmlRecord.h"

namespace {
   EnumStringMapping const fieldTypeToName {
      {XmlRecordDefinition::FieldType::Bool            , QObject::tr("Bool"            )},
      {XmlRecordDefinition::FieldType::Int             , QObject::tr("Int"             )},
      {XmlRecordDefinition::FieldType::UInt            , QObject::tr("UInt"            )},
      {XmlRecordDefinition::FieldType::Double          , QObject::tr("Double"          )},
      {XmlRecordDefinition::FieldType::String          , QObject::tr("String"          )},
      {XmlRecordDefinition::FieldType::Date            , QObject::tr("Date"            )},
      {XmlRecordDefinition::FieldType::Enum            , QObject::tr("Enum"            )},
      {XmlRecordDefinition::FieldType::RequiredConstant, QObject::tr("RequiredConstant")},
      {XmlRecordDefinition::FieldType::Record          , QObject::tr("Record"          )},
      {XmlRecordDefinition::FieldType::ListOfRecords   , QObject::tr("ListOfRecords"   )},
   };
}

XmlRecordDefinition::FieldDefinition::FieldDefinition(FieldType    type,
                                                      XQString     xPath,
                                                      PropertyPath propertyPath,
                                                      ValueDecoder valueDecoder) :
   type{type},
   xPath{xPath},
   propertyPath{propertyPath},
   valueDecoder{valueDecoder} {
   // An XmlRecordDefinition address should be in the valueDecoder if and only if the record type is Record or
   // ListOfRecords.  Otherwise there's a coding error in the mappings in BeerXML.cpp.  We assert this also when we're
   // processing an XML file, but the advantage of doing so here is that we'll get a start-up error, so bugs will be
   // evident straight away.
   //
   // If this assert fires, grep for "::Record" and "::ListOfRecords" in BeerXML.cpp and see which lines are
   // missing the final parameter.
   Q_ASSERT((XmlRecordDefinition::FieldType::Record        == this->type ||
             XmlRecordDefinition::FieldType::ListOfRecords == this->type) ==
            std::holds_alternative<XmlRecordDefinition const *>(this->valueDecoder));
   return;
}

XmlRecordDefinition::XmlRecordDefinition(
   char const *                   const   recordName,
   TypeLookup const *             const   typeLookup,
   char const *                   const   namedEntityClassName,
   QString                        const & localisedEntityName,
   NamedEntityCasters             const   upAndDownCasters,
   XmlRecordConstructorWrapper xmlRecordConstructorWrapper,
   std::initializer_list<XmlRecordDefinition::FieldDefinition> fieldDefinitions
) :
   SerializationRecordDefinition{recordName, typeLookup, namedEntityClassName, localisedEntityName, upAndDownCasters},
   xmlRecordConstructorWrapper{xmlRecordConstructorWrapper},
   fieldDefinitions{fieldDefinitions} {
   return;
}

XmlRecordDefinition::XmlRecordDefinition(
   char const *                   const   recordName,
   TypeLookup const *             const   typeLookup,
   char const *                   const   namedEntityClassName,
   QString                        const & localisedEntityName,
   NamedEntityCasters             const   upAndDownCasters,
   XmlRecordConstructorWrapper xmlRecordConstructorWrapper,
   std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists
) :
   SerializationRecordDefinition{recordName, typeLookup, namedEntityClassName, localisedEntityName, upAndDownCasters},
   xmlRecordConstructorWrapper{xmlRecordConstructorWrapper},
   fieldDefinitions{} {
   // This is a bit clunky, but it works and the inefficiency is a one-off cost at start-up
   for (auto const & list : fieldDefinitionLists) {
      // After you've initialised a const, you can't modify it, even in the constructor, unless you cast away the
      // constness (is that a word?) via a pointer or reference to tell the compiler you really do want to modify the
      // member variable.
      std::vector<XmlRecordDefinition::FieldDefinition> & myFieldDefinitions =
         const_cast<std::vector<XmlRecordDefinition::FieldDefinition> &>(this->fieldDefinitions);
      // You can't do the following with QVector, which is why we're using std::vector here
      myFieldDefinitions.insert(myFieldDefinitions.end(), list.begin(), list.end());
   }
   return;
}

std::unique_ptr<XmlRecord> XmlRecordDefinition::makeRecord(XmlCoding const & xmlCoding) const {
   return this->xmlRecordConstructorWrapper(xmlCoding, *this);
}

template<class S>
S & operator<<(S & stream, XmlRecordDefinition::FieldType const fieldType) {
   try {
      stream << fieldTypeToName[fieldType];
   } catch (std::out_of_range & e) {
      // This is a coding error, so stop (after logging) on a debug build
      stream << "Unrecognised field type: " << static_cast<int>(fieldType);
      Q_ASSERT(false);
   }
   return stream;
}

//
// Instantiate the above template function for the types that are going to use it
// (This is all just a trick to allow the template definition to be here in the .cpp file and not in the header.)
//
template QDebug      & operator<<(QDebug      & stream, XmlRecordDefinition::FieldType const fieldType);
template QTextStream & operator<<(QTextStream & stream, XmlRecordDefinition::FieldType const fieldType);


template<class S>
S & operator<<(S & stream, XmlRecordDefinition::FieldDefinition const & fieldDefinition) {
   stream <<
      "FieldDefinition:" << fieldDefinition.type << "/" << fieldDefinition.xPath << "/" << fieldDefinition.propertyPath;
   return stream;
}

template QDebug      & operator<<(QDebug      & stream, XmlRecordDefinition::FieldDefinition const & fieldDefinition);
template QTextStream & operator<<(QTextStream & stream, XmlRecordDefinition::FieldDefinition const & fieldDefinition);
