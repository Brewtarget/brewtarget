/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonRecordDefinition.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/json/JsonRecordDefinition.h"

#include <QDebug>

#include "serialization/json/JsonRecord.h"

namespace {
   EnumStringMapping const fieldTypeToName {
      {JsonRecordDefinition::FieldType::Bool                      , QObject::tr("Bool"                      )},
      {JsonRecordDefinition::FieldType::Int                       , QObject::tr("Int"                       )},
      {JsonRecordDefinition::FieldType::UInt                      , QObject::tr("UInt"                      )},
      {JsonRecordDefinition::FieldType::Double                    , QObject::tr("Double"                    )},
      {JsonRecordDefinition::FieldType::String                    , QObject::tr("String"                    )},
      {JsonRecordDefinition::FieldType::Enum                      , QObject::tr("Enum"                      )},
      {JsonRecordDefinition::FieldType::Record                    , QObject::tr("Record"                    )},
      {JsonRecordDefinition::FieldType::ListOfRecords             , QObject::tr("ListOfRecords"             )},
      {JsonRecordDefinition::FieldType::Date                      , QObject::tr("Date"                      )},
      {JsonRecordDefinition::FieldType::MeasurementWithUnits      , QObject::tr("MeasurementWithUnits"      )},
      {JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, QObject::tr("OneOfMeasurementsWithUnits")},
      {JsonRecordDefinition::FieldType::SingleUnitValue           , QObject::tr("SingleUnitValue"           )},
      {JsonRecordDefinition::FieldType::RequiredConstant          , QObject::tr("RequiredConstant"          )},
   };
}

JsonRecordDefinition::FieldDefinition::FieldDefinition(FieldType type,
                                                       JsonXPath xPath,
///                                                       BtStringConst const & propertyName,
                                                       PropertyPath propertyPath,
                                                       ValueDecoder valueDecoder) :
   type{type},
   xPath{xPath},
///   propertyName{&propertyName},
   propertyPath{propertyPath},
   valueDecoder{valueDecoder} {
///   // Per comment in header file, propertyName should never be nullptr; use BtString::NULL_STR instead if it is not set
///   Q_ASSERT(this->propertyName);
   return;
}

JsonRecordDefinition::JsonRecordDefinition(
   char const *                  const   recordName,
   TypeLookup const *            const   typeLookup,
   char const *                  const   namedEntityClassName,
   QString                       const & localisedEntityName,
   NamedEntity::UpAndDownCasters const   upAndDownCasters,
   JsonRecordConstructorWrapper          jsonRecordConstructorWrapper,
   std::initializer_list<JsonRecordDefinition::FieldDefinition> fieldDefinitions,
   RecordType                    const   recordType
) :
   SerializationRecordDefinition{recordName, typeLookup, namedEntityClassName, localisedEntityName, upAndDownCasters},
   jsonRecordConstructorWrapper{jsonRecordConstructorWrapper},
   fieldDefinitions{fieldDefinitions},
   isOutlineRecord{recordType == RecordType::Outline} {
   return;
}

JsonRecordDefinition::JsonRecordDefinition(
   char const *                  const   recordName,
   TypeLookup const *            const   typeLookup,
   char const *                  const   namedEntityClassName,
   QString                       const & localisedEntityName,
   NamedEntity::UpAndDownCasters const   upAndDownCasters,
   JsonRecordConstructorWrapper          jsonRecordConstructorWrapper,
   std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists,
   RecordType                    const   recordType
) :
   SerializationRecordDefinition{recordName, typeLookup, namedEntityClassName, localisedEntityName, upAndDownCasters},
   jsonRecordConstructorWrapper{jsonRecordConstructorWrapper},
   fieldDefinitions{},
   isOutlineRecord{recordType == RecordType::Outline} {
   // This is a bit clunky, but it works and the inefficiency is a one-off cost at start-up
   for (auto const & list : fieldDefinitionLists) {
      // After you've initialised a const, you can't modify it, even in the constructor, unless you cast away the
      // constness (is that a word?) via a pointer or reference to tell the compiler you really do want to modify the
      // member variable.
      std::vector<JsonRecordDefinition::FieldDefinition> & myFieldDefinitions =
         const_cast<std::vector<JsonRecordDefinition::FieldDefinition> &>(this->fieldDefinitions);
      // You can't do the following with QVector, which is why we're using std::vector here
      myFieldDefinitions.insert(myFieldDefinitions.end(), list.begin(), list.end());
   }
   return;
}

std::unique_ptr<JsonRecord> JsonRecordDefinition::makeRecord(JsonCoding const & jsonCoding,
                                                             boost::json::value & recordData) const {
   return this->jsonRecordConstructorWrapper(jsonCoding, recordData, *this);
}


template<class S>
S & operator<<(S & stream, JsonRecordDefinition::FieldType const fieldType) {
   std::optional<QString> fieldTypeAsString = fieldTypeToName.enumToString(fieldType);
   if (fieldTypeAsString) {
      stream << *fieldTypeAsString;
   } else {
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
template QDebug & operator<<(QDebug & stream, JsonRecordDefinition::FieldType const fieldType);
template QTextStream & operator<<(QTextStream & stream, JsonRecordDefinition::FieldType const fieldType);
