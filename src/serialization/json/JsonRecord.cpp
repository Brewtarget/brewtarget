/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonRecord.cpp is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#include "serialization/json/JsonRecord.h"

#include <optional>
#include <string_view>
#include <system_error>

#include <QDate>
#include <QDebug>
#include <QMetaType>

#include "serialization/json/JsonCoding.h"
#include "serialization/json/JsonRecordDefinition.h"
#include "serialization/json/JsonUtils.h"
#include "model/Hop.h"  // Only needed for workaround/hack for Hop year property
#include "model/NamedEntity.h"
#include "model/NamedParameterBundle.h"
#include "model/OutlineableNamedEntity.h"
#include "utils/ErrorCodeToStream.h"
#include "utils/ImportRecordCount.h"
#include "utils/OptionalHelpers.h"

//
// Variables and constant definitions that we need only in this file
//
namespace {
   /**
    * \brief Read value and unit fields from a JSON record
    *
    *        We assume that the requested fields exist and are of the correct type (double and string respectively)
    *        because this should have been enforced already by JSON schema validation.
    *
    * \param type
    * \param xPath
    * \param unitField
    * \param valueField
    * \param recordData
    * \param value Variable in which value is returned
    * \param unitName Variable in which unit is returned
    * \return \c true if succeeded, \c false otherwise
    */
   bool readValueAndUnit(JsonRecordDefinition::FieldType const type,
                         JsonXPath const & xPath,
                         JsonXPath const & unitField,
                         JsonXPath const & valueField,
                         boost::json::value const * recordData,
                         double & value,
                         std::string_view & unitName) {
      // It's a coding error to supply a null pointer for recordData
      Q_ASSERT(recordData);

      // It's a coding error if we're trying to read sub-values from something that is not a JSON object
      Q_ASSERT(recordData->is_object());

      //
      // Read the value and unit fields.  We assert that they exist and are of the correct type (double and string
      // respectively) because this should have been enforced already by JSON schema validation.
      //

      // Usually leave next line commented as otherwise generates too much logging
//      qDebug() <<
//         Q_FUNC_INFO << "Reading" << valueField << "and" << unitField << "sub-fields from" << xPath << "record:" <<
//         *recordData;

      std::error_code errCode;
      boost::json::value const * valueRaw = valueField.followPathFrom(recordData, errCode);
      if (!valueRaw) {
         // Not expecting this to happen given that we've already validated the JSON file against its schema.
         qWarning() << Q_FUNC_INFO << "Error parsing value from" << xPath << " (" << type << "): " << errCode;
         return false;
      }
      // Usually leave next line commented as otherwise generates too much logging
//      qDebug() << Q_FUNC_INFO << "Raw Value=" << *valueRaw << "(" << valueRaw->kind() << ")";

      // The JSON type should be number.  Boost.JSON will have chosen either double or int64  (or conceivably uint64) to
      // store the number, depending eg on whether it has a decimal separator.  So we cannot assert that
      // valueRaw->is_double().  Fortunately, Boost.JSON helps us with the necessary casting.
      Q_ASSERT(valueRaw->is_number());
      value = valueRaw->to_number<double>(errCode);
      if (errCode) {
         // Not expecting this to happen as doco says "If T is a floating point type and the stored value is a number,
         // the conversion is performed without error. The converted number is returned, with a possible loss of
         // precision. "
         qWarning() <<
            Q_FUNC_INFO << "Error extracting double from" << *valueRaw << "(" << valueRaw->kind() << ") for" <<
            xPath << " (" << type << "): " << errCode;
         return false;
      }
      // Usually leave next line commented as otherwise generates too much logging
//      qDebug() << Q_FUNC_INFO << "Value=" << value;

      boost::json::value const * unitNameRaw = unitField.followPathFrom(recordData, errCode);
      if (!unitNameRaw) {
         // Not expecting this to happen given that we've already validated the JSON file against its schema.
         qWarning() << Q_FUNC_INFO << "Error parsing units from" << xPath << " (" << type << "): " << errCode;
         return false;
      }
      Q_ASSERT(unitNameRaw);
      Q_ASSERT(unitNameRaw->is_string());
      unitName = unitNameRaw->get_string();

      // Usually leave next line commented as otherwise generates too much logging
//      qDebug() << Q_FUNC_INFO << "Read" << xPath << " (" << type << ") as" << value << " " <<
//         std::string(unitName).c_str();
      return true;
   }

   /**
    * \brief Read value and unit fields from a JSON record with a single mapping (ie relating to a single physical
    *        quantity) and convert to canonical units
    *
    *        We assume that the requested fields exist and are of the correct type (double and string respectively)
    *        because this should have been enforced already by JSON schema validation.
    *
    * \param fieldDefinition
    * \param recordData
    * \return The value, converted to canonical scale, or \c std::nullopt if there was an error
    */
   std::optional<Measurement::Amount> readMeasurementWithUnits(
      JsonRecordDefinition::FieldDefinition const & fieldDefinition,
      boost::json::value const * recordData
   ) {
      // It's a coding error to supply a null pointer for recordData
      Q_ASSERT(recordData);

      double value{0};
      std::string_view unitName{""};
      if (!readValueAndUnit(fieldDefinition.type,
                            fieldDefinition.xPath,
                            std::get<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder)->unitField,
                            std::get<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder)->valueField,
                            recordData,
                            value,
                            unitName)) {
         return std::nullopt;
      }

      //
      // The schema validation should have ensured that the unit name is constrained to one of the values we are
      // expecting, so it's almost certainly a coding error if it doesn't.
      //
      // Note that we have to do case-insensitive matching here.  Eg, although the BeerJSON schema defines ColorUnitType
      // as an enum of "EBC", "Lovi" and "SRM", schema validation will accept "srm" as valid.  AFAIK, it doesn't matter
      // if the case is "wrong" because there are no enums where members differ only by case.
      //
      auto mapper = std::get<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder);
      if (!mapper->containsUnit(unitName, JsonMeasureableUnitsMapping::MatchType::CaseInsensitive)) {
         qCritical() << Q_FUNC_INFO << "Unexpected unit name:" << std::string(unitName).c_str();
         // Stop here on debug build
         Q_ASSERT(false);
         return std::nullopt;
      }

      Measurement::Unit const * unit = mapper->findUnit(unitName,
                                                        JsonMeasureableUnitsMapping::MatchType::CaseInsensitive);
      Measurement::Amount canonicalValue = unit->toCanonical(value);

      qDebug() <<
         Q_FUNC_INFO << "Converted" << value << " " << std::string(unitName).c_str() << "to" << canonicalValue;

      return canonicalValue;
   }

   /**
    * \brief Read value and unit fields from a JSON record with a multiple mappings (eg one for mass and one for volume)
    *        and convert to canonical units
    *
    *        We assume that the requested fields exist and are of the correct type (double and string respectively)
    *        because this should have been enforced already by JSON schema validation.
    *
    * \param fieldDefinition
    * \param recordData
    * \return The value, converted to canonical scale, or \c std::nullopt if there was an error
    */
   std::optional<Measurement::Amount> readOneOfMeasurementsWithUnits(
      JsonRecordDefinition::FieldDefinition const & fieldDefinition,
      boost::json::value const * recordData
   ) {
      // It's a coding error to supply a null pointer for recordData
      Q_ASSERT(recordData);

      // It's a coding error if the list of JsonMeasureableUnitsMapping objects has less than two elements.  (For
      // one element you should use JsonRecordDefinition::FieldType::MeasurementWithUnits instead of
      // JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits.)
      Q_ASSERT(std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)->size() > 1);

      // Per the comment in json/JsonRecordDefinition.h, we assume that unitField and valueField are the same for each
      // JsonMeasureableUnitsMapping in the list, so we just use the first entry here.
      JsonXPath const & unitField  =
         std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)->at(0)->unitField;
      JsonXPath const & valueField =
         std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)->at(0)->valueField;

      double value{0};
      std::string_view unitName{""};
      if (!readValueAndUnit(fieldDefinition.type,
                            fieldDefinition.xPath,
                            unitField,
                            valueField,
                            recordData,
                            value,
                            unitName)) {
         return std::nullopt;
      }

      Measurement::Unit const * unit = nullptr;
      for (auto const unitsMapping :
           *std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)) {
         // As above, we need to be case insensitive here and this should not create ambiguity
         if (unitsMapping->containsUnit(unitName, JsonMeasureableUnitsMapping::MatchType::CaseInsensitive)) {
            unit = unitsMapping->findUnit(unitName, JsonMeasureableUnitsMapping::MatchType::CaseInsensitive);
            break;
         }
      }

      // The schema validation should have ensured that the unit name is constrained to one of the values we are
      // expecting, so it's almost certainly a coding error if it doesn't.
      if (!unit) {
         qCritical() <<
            Q_FUNC_INFO << "Unexpected unit name:" << std::string(unitName).c_str() << "for field" <<
            fieldDefinition.xPath << "(" << fieldDefinition.type << ")";
         // Stop here on debug build
         Q_ASSERT(false);
         return std::nullopt;
      }

      Measurement::Amount canonicalValue = unit->toCanonical(value);

      qDebug() <<
         Q_FUNC_INFO << "Converted" << value << " " << std::string(unitName).c_str() << "to" << canonicalValue;

      return canonicalValue;
   }

   /**
    * \brief Read value and unit fields where the units are expected to always be the same (eg "%")
    *
    *        We assume that the requested fields exist and are of the correct type (double and string respectively)
    *        because this should have been enforced already by JSON schema validation.
    *
    * \param fieldDefinition
    * \param recordData
    * \return The value, or \c std::nullopt if there was an error
    */
   std::optional<double> readSingleUnitValue(JsonRecordDefinition::FieldDefinition const & fieldDefinition,
                                             boost::json::value const * recordData) {
      // It's a coding error to supply a null pointer for recordData
      Q_ASSERT(recordData);

      double value{0};
      std::string_view unitName{""};
      if (!readValueAndUnit(fieldDefinition.type,
                            fieldDefinition.xPath,
                            std::get<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder)->unitField,
                            std::get<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder)->valueField,
                            recordData,
                            value,
                            unitName)) {
         return std::nullopt;
      }

      // The schema validation should have ensured that the unit name is what we're expecting, so it's almost certainly
      // a coding error if it doesn't.
      if (!std::get<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder)->validUnits.contains(unitName)) {
         qCritical() <<
            Q_FUNC_INFO << "Unit name" << std::string(unitName).c_str() << "does not match expected (" <<
            std::get<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder)->validUnits.first().data() <<
            "etc)";
         // Stop here on debug build
         Q_ASSERT(false);
         return std::nullopt;
      }
      return value;
   }

}

JsonRecord::JsonRecord(JsonCoding const & jsonCoding,
                       boost::json::value & recordData,
                       JsonRecordDefinition const & recordDefinition) :
   SerializationRecord{jsonCoding, recordDefinition},
   m_recordData{recordData} {
   return;
}

JsonRecord::~JsonRecord() = default;

SerializationRecordDefinition const & JsonRecord::recordDefinition() const {
   return this->m_recordDefinition;
}

[[nodiscard]] bool JsonRecord::load(QTextStream & userMessage) {
   Q_ASSERT(this->m_recordData.is_object());
   qDebug() <<
      Q_FUNC_INFO << "Loading" << this->m_recordDefinition.m_recordName << "record containing" <<
      this->m_recordData.as_object().size() << "elements";

   //
   // If we know this record contains only outline/base info (see comments in model/OutlineableNamedEntity.h) then we
   // record that now so that the right things happen when we are doing duplicate matching later (in
   // JsonRecord::normaliseAndStoreInDb).
   //
   if (this->m_recordDefinition.isOutlineRecord) {
      this->m_namedParameterBundle.insert(PropertyNames::OutlineableNamedEntity::outline, QVariant{true});
   }

   //
   // Loop through all the fields that we know/care about.  Anything else is intentionally ignored.  (We won't know
   // what to do with it, and, if it weren't allowed to be there, it would have generated an error at schema parsing.)
   //
   // Note that it's a coding error if there are no fields in the record definition.  (This usually means a template
   // specialisation was omitted in serialization/json/BeerJson.cpp.)
   //
   qDebug() <<
      Q_FUNC_INFO << "Examining" << this->m_recordDefinition.fieldDefinitions.size() << "field definitions for" <<
      this->m_recordDefinition.m_recordName;
   Q_ASSERT(this->m_recordDefinition.fieldDefinitions.size() > 0);
   for (auto & fieldDefinition : this->m_recordDefinition.fieldDefinitions) {
      //
      // NB: As with XML processing in XmlRecord::load, if we don't find a node, there's nothing for us to do.  The
      // schema validation should already flagged up an error if there are missing _required_ fields.  Equally,
      // although we only look for nodes we know about, some of these we won't use for one reason or another (eg "write
      // only" fields such as IBU on Recipe).
      //
      std::error_code errorCode;
      boost::json::value * container = fieldDefinition.xPath.followPathFrom(&this->m_recordData, errorCode);
      if (!container) {
         // As noted above this is usually not an error, but _sometimes_ useful to log for debugging.  Usually leave
         // this logging commented out though as otherwise it fills up the log files
//         qDebug() <<
//            Q_FUNC_INFO << fieldDefinition.xPath << " (" << fieldDefinition.type << ") not present (error code " <<
//            errorCode.value() << ":" << errorCode.message().c_str() << ")";
      } else {
         // Again, it can be useful to uncomment this logging statement for debugging, but usually we don't want it
         // taking up space in the log files.
//         qDebug() <<
//            Q_FUNC_INFO << "Found" << fieldDefinition.xPath << " (" << fieldDefinition.type << "/" <<
//            container->kind() << ")";


         //
         // We used to parse everything and then decide whether we needed to store it.  The problem with doing this
         // is when we have non-trivial bits of structure that we don't currently support (eg a record or an enum) then
         // we won't have the expected contents of fieldDefinition.valueDecoder.  So it's safer to bail out here.
         //
         // Note that we have to check fieldDefinition.valueDecoder too because, eg, top-level lists of items (Hops,
         // Recipes, etc) don't have property paths!
         //
         if (fieldDefinition.propertyPath.isNull() &&
             std::holds_alternative<std::monostate>(fieldDefinition.valueDecoder)) {
            qInfo() <<
               Q_FUNC_INFO << "Ignoring unsupported field at" << fieldDefinition.xPath << " (" <<
               fieldDefinition.type << "/" << container->kind() << ")";
            continue;

         } else if (JsonRecordDefinition::FieldType::Record        == fieldDefinition.type ||
                    JsonRecordDefinition::FieldType::ListOfRecords == fieldDefinition.type) {
            Q_ASSERT(std::holds_alternative<JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
            Q_ASSERT(std::get              <JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
            JsonRecordDefinition const & childRecordDefinition{
               *std::get<JsonRecordDefinition const *>(fieldDefinition.valueDecoder)
            };
            if (JsonRecordDefinition::FieldType::Record == fieldDefinition.type) {
               //
               // Note that the call to JsonXPath::followPathFrom does the "right thing" in the event that we are
               // handling the special "Base Record" case described in serialization/json/JsonRecordDefinition.h (that
               // being the only case in which fieldDefinition.xPath should be empty).
               //
               if (!this->loadChildRecord(fieldDefinition,
                                          childRecordDefinition,
                                          *container,
                                          userMessage)) {
                  return false;
               }
            } else {
               //
               // One difference between XML and JSON when it comes to arrays is that the latter has one less layer of
               // tags.  In XML (eg BeerXML), we have "<HOPS><HOP>...</HOP><HOP>...</HOP>...</HOPS>".  In JSON (eg
               // BeerJSON) we have hop_varieties: [{...},{...},...].
               //
               // Schema should have already enforced that this field is an array, so we assert that here
               //
               Q_ASSERT(container->is_array());
               boost::json::array & childRecordsData = container->get_array();

               // Note that there are some arrays that we don't treat as lists-of-things because we use Named Array Item Id
               // part of a JsonXPath to access a specific "named" member of the array.  Those are not handled in this code
               // branch.
               if (!this->loadChildRecords(fieldDefinition,
                                           childRecordDefinition,
                                           childRecordsData,
                                           userMessage)) {
                  return false;
               }
            }
         } else {
            //
            // If it's not an array then it's fields on the object we're currently populating
            //
            bool parsedValueOk = false;
            QVariant parsedValue;

            //
            // As per the equivalent point in XmlRecord, we're going to need to know whether this field is "optional" in
            // our internal data model.  If it is, then, for whatever underlying type T it is, we need the parsedValue
            // QVariant to hold std::optional<T> instead of just T.
            //
            // NB: propertyPath is not actually a property path when fieldType is RequiredConstant
            //
            bool const propertyIsOptional {
               (fieldDefinition.type == JsonRecordDefinition::FieldType::RequiredConstant ||
                fieldDefinition.propertyPath.isNull()) ?
                  false :
                  fieldDefinition.propertyPath.getTypeInfo(*this->m_recordDefinition.m_typeLookup).isOptional()
            };

            //
            // JSON Schema validation should have ensured this field really is what we're expecting, so it's a coding
            // error if it's not, which is what most of the asserts below are saying.
            //
            // HOWEVER, note that we need to take care with numeric types.  JSON only has one base numeric type
            // (number).  Boost.JSON handles this correctly but also offers access to the underlying type it has used to
            // store the number (std::int64_t, std::uint64_t or double).  So, eg, you can first call
            // container->is_double() to check whether the underlying storage is double and then, if that returns true,
            // call container->get_double() to get the value.  This seems like an attractive short-cut (which it is when
            // you have full control over the JSON input) but in can catch you out.  Eg if a field that usually has a
            // decimal point happens to be an integer and was stored (validly) without the decimal point in the JSON
            // file, then Boost.JSON will put it in eg std::int64_t rather than double, and get_double() will barf an
            // assertion failure.
            //
            // The correct thing to do for general purpose handling is to assert is_number() and use the templated
            // to_number() function to get back the type WE want rather than Boost.JSON's internal storage type.
            //
            // Of course, having extracted std::int64_t or std::uint64_t, we then just cast them to int and unsigned
            // int.  This should be OK for the foreseeable future on the platforms we target and for the likely ranges
            // of values that we're reading in.
            //
            switch(fieldDefinition.type) {

               case JsonRecordDefinition::FieldType::Bool:
                  Q_ASSERT(container->is_bool());
                  {
                     auto rawValue = container->get_bool();
                     parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     parsedValueOk = true;
                  }
                  break;

               case JsonRecordDefinition::FieldType::Int:
                  Q_ASSERT(container->is_number());
                  {
                     int rawValue = container->to_number<std::int64_t>();
                     parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     parsedValueOk = true;
                  }
                  break;

               case JsonRecordDefinition::FieldType::UInt:
                  Q_ASSERT(container->is_number());
                  {
                     unsigned int rawValue = container->to_number<std::uint64_t>();
                     parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     parsedValueOk = true;
                  }
                  break;

               case JsonRecordDefinition::FieldType::Double:
                  Q_ASSERT(container->is_number());
                  {
                     auto rawValue = container->to_number<double>();
                     parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     parsedValueOk = true;
                  }
                  break;

               case JsonRecordDefinition::FieldType::String:
                  Q_ASSERT(container->is_string());
                  {
                     QString rawValue{container->get_string().c_str()};
                     parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     parsedValueOk = true;
                  }
                  break;

               case JsonRecordDefinition::FieldType::Enum:
                  // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
                  Q_ASSERT(std::holds_alternative<EnumStringMapping const *>(fieldDefinition.valueDecoder));
                  Q_ASSERT(std::get              <EnumStringMapping const *>(fieldDefinition.valueDecoder));
                  {
                     Q_ASSERT(container->is_string());
                     QString value{container->get_string().c_str()};

                     auto match =
                        std::get<EnumStringMapping const *>(fieldDefinition.valueDecoder)->stringToEnumAsInt(value);
                     if (!match) {
                        // This is probably a coding error as the JSON Schema should already have verified that the
                        // value is one of the expected ones.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->m_recordDefinition.m_namedEntityClassName << " node " <<
                           fieldDefinition.xPath << "=" << value << " as value not recognised";
                     } else {
                        auto rawValue = match.value();
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        parsedValueOk = true;
                     }
                  }
                  break;

               case JsonRecordDefinition::FieldType::Record:
               case JsonRecordDefinition::FieldType::ListOfRecords:
                  // This should be unreachable as we dealt with these cases separately above, but having case
                  // statements for them eliminates a compiler warning whilst still retaining the useful warning if we
                  // have ever omitted processing for another field type.
                  Q_ASSERT(false);
                  break;

               case JsonRecordDefinition::FieldType::MeasurementWithUnits:
                  // It's definitely a coding error if there is no unit decoder mapping for a field declared to require
                  // one
                  Q_ASSERT(std::holds_alternative<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder));
                  Q_ASSERT(std::get              <JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder));
                  // JSON schema validation should have ensured that the field is actually one with subfields for value
                  // and unit
                  Q_ASSERT(container->is_object());
                  {
                     std::optional<Measurement::Amount> canonicalValue = readMeasurementWithUnits(fieldDefinition,
                                                                                                  container);
                     if (canonicalValue) {
                        auto rawValue = canonicalValue->quantity;
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        parsedValueOk = true;
                     }
                  }
                  break;

               case JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits:
                  // It's definitely a coding error if there is no list of unit decoder mappings for a field declared to
                  // require such
                  Q_ASSERT(std::holds_alternative<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder));
                  Q_ASSERT(std::get              <ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder));
                  // JSON schema validation should have ensured that the field is actually one with subfields for value
                  // and unit
                  Q_ASSERT(container->is_object());
                  {
                     // Logic similar to that for MeasurementWithUnits.  See comment in utils/MetaTypes.h for why we use
                     // Measurement::Amount rather than MassOrVolumeAmt.
                     std::optional<Measurement::Amount> canonicalValue = readOneOfMeasurementsWithUnits(fieldDefinition,
                                                                                                        container);
                     if (canonicalValue) {
                        auto rawValue = canonicalValue.value();
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        parsedValueOk = true;
                     }
                  }
                  break;

               case JsonRecordDefinition::FieldType::SingleUnitValue:
                  // It's definitely a coding error if there is no unit specifier for a field declared to require one
                  Q_ASSERT(std::holds_alternative<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder));
                  Q_ASSERT(std::get              <JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder));
                  // JSON schema validation should have ensured that the field is actually one with subfields for value
                  // and unit
                  Q_ASSERT(container->is_object());
                  {
                     std::optional<double> value = readSingleUnitValue(fieldDefinition, container);
                     qDebug() <<
                        Q_FUNC_INFO << "Read:" << value << "for" << fieldDefinition.xPath << "/" <<
                        fieldDefinition.propertyPath;
                     if (value) {
                        auto rawValue = *value;
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                        parsedValueOk = true;
                     }
                  }
                  break;

               //
               // From here on, we have BeerJSON-specific types.  If we ever wanted to parse some other type of JSON,
               // then we might need to make this code more generic, but, for now, we're not going to worry too much as
               // it seems unlikely there will be other JSON encodings we want to deal with in the foreseeable future.
               //

               case JsonRecordDefinition::FieldType::Date:
                  // In BeerJSON, DateType is a string matching this regexp:
                  //   "\\d{4}-\\d{2}-\\d{2}|\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}"
                  // This is One True Date Format™ (aka ISO 8601), which makes our life somewhat easier
                  Q_ASSERT(container->is_string());
                  {
                     QString value{container->get_string().c_str()};
                     QDate rawValue = QDate::fromString(value, Qt::ISODate);
                     parsedValueOk = rawValue.isValid();
                     if (parsedValueOk) {
                        parsedValue = Optional::variantFromRaw(rawValue, propertyIsOptional);
                     } else {
                        // The JSON schema validation doesn't guarantee the date is valid, just that it's the right
                        // digit groupings.  So, we do need to handle cases such as 2022-13-13 which are the right
                        // format but not valid dates.
                        qWarning() <<
                           Q_FUNC_INFO << "Ignoring " << this->m_recordDefinition.m_namedEntityClassName << " node " <<
                           fieldDefinition.xPath << "=" << value << " as could not be parsed as ISO 8601 date";
                     }
                  }
                  break;

               case JsonRecordDefinition::FieldType::RequiredConstant:
                  //
                  // This is a field that is required to be in the JSON, but whose value we don't need (and for which
                  // we always write a constant value on output).  At the moment it's only needed for the VERSION tag
                  // in BeerJSON.
                  //
                  // Note that, because we abuse the propertyName field to hold the default value (ie what we write
                  // out), we can't carry on to normal processing below.  So jump straight to processing the next
                  // node in the loop (via continue).
                  //
                  qDebug() <<
                     Q_FUNC_INFO << "Skipping " << this->m_recordDefinition.m_namedEntityClassName << " node " <<
                     fieldDefinition.xPath << "=" << *container << "(" << fieldDefinition.propertyPath.asXPath() <<
                     ") as not useful";
                  continue; // NB: _NOT_break here.  We want to jump straight to the next run through the for loop.

               // Don't need a default case.  Compiler should warn us if we didn't have a case for one of the
               // JsonRecordDefinition::FieldType values.  This is one of the benefits of strongly-typed enums
            }

            //
            // What we do if we couldn't parse the value depends.  If it was a value that we didn't need to set on
            // the supplied Hop/Yeast/Recipe/Etc object, then we can just ignore the problem and carry on processing.
            // But, if this was a field we were expecting to use, then it's a problem that we couldn't parse it and
            // we should bail.
            //
            if (!parsedValueOk && !fieldDefinition.propertyPath.isNull()) {
               userMessage <<
                  "Could not parse " << this->m_recordDefinition.m_namedEntityClassName << " node " <<
                  fieldDefinition.xPath << "=" << *container << " into " << fieldDefinition.propertyPath.asXPath();
               return false;
            }

            //
            // So we've either parsed the value OK or we don't need it (or both)
            //
            // If we do need it, we now store the value
            //
            if (!fieldDefinition.propertyPath.isNull()) {
               this->m_namedParameterBundle.insert(fieldDefinition.propertyPath, parsedValue);
            }
         }
      }
   }

   //
   // For everything but the root record, we now construct a suitable object (Hop, Recipe, etc) from the
   // NamedParameterBundle (which will be empty for the root record).
   //
   if (!this->m_namedParameterBundle.isEmpty()) {
      this->constructNamedEntity();
   }

   return true;
}

[[nodiscard]] JsonRecord::ProcessingResult JsonRecord::normaliseAndStoreInDb(
   std::shared_ptr<NamedEntity> containingEntity,
   QTextStream & userMessage,
   ImportRecordCount & stats
) {
   // Most of the work is done in the base class
   auto processingResult = this->SerializationRecord::normaliseAndStoreInDb(containingEntity, userMessage, stats);
///   if (nullptr != this->m_namedEntity) {
///      qDebug() <<
///         Q_FUNC_INFO << "Normalise and store " << this->m_recordDefinition.m_namedEntityClassName << "(" <<
///         this->m_namedEntity->metaObject()->className() << "):" << this->m_namedEntity->name();
///
///      //
///      // If the object we are reading in is a duplicate of something we already have (and duplicates are not allowed)
///      // then skip over this record (and any records it contains).  (This is _not_ an error, so we return true not
///      // false in this event.)
///      //
///      // Note, however, that some objects -- in particular those such as Recipe that contain other objects -- need
///      // to be further along in their construction (ie have had all their contained objects added) before we can
///      // determine whether they are duplicates.  This is why we check again, after storing in the DB, below.
///      //
///      if (this->resolveDuplicates()) {
///         qDebug() <<
///            Q_FUNC_INFO << "(Early found) duplicate" << this->m_recordDefinition.m_namedEntityClassName <<
///            (this->includedInStats() ? " will" : " won't") << " be included in stats";
///         if (this->includedInStats()) {
///            stats.skipped(*this->m_recordDefinition.m_namedEntityClassName);
///         }
///         return SerializationRecord::ProcessingResult::FoundDuplicate;
///      }
///
///      this->normaliseName();
///
///      // Some classes of object are owned by their containing entity and can't sensibly be saved without knowing what it
///      // is.  Subclasses of JsonRecord will override setContainingEntity() to pass the info in if it is needed (or ignore
///      // it if not).
///      this->setContainingEntity(containingEntity);
///
///      // Now we're ready to store in the DB
///      int id = this->storeNamedEntityInDb();
///      if (id <= 0) {
///         userMessage << "Error storing " << this->m_namedEntity->metaObject()->className() <<
///         " in database.  See logs for more details";
///         return SerializationRecord::ProcessingResult::Failed;
///      }
///   }
///
///   SerializationRecord::ProcessingResult processingResult;
///
///   //
///   // Finally (well, nearly) orchestrate storing any contained records
///   //
///   // Note, of course, that this still needs to be done, even if nullptr == this->m_namedEntity, because that just means
///   // we're processing the root node.
///   //
///   if (this->normaliseAndStoreChildRecordsInDb(userMessage, stats)) {
///      //
///      // Now all the processing succeeded, we do that final duplicate check for any complex object such as Recipe that
///      // had to be fully constructed before we could meaningfully check whether it's the same as something we already
///      // have in the object store.
///      //
///      if (nullptr == this->m_namedEntity.get()) {
///         // Child records OK and no duplicate check needed (root record), which also means no further processing
///         // required
///         return SerializationRecord::ProcessingResult::Succeeded;
///      }
///      processingResult = this->isDuplicate() ? SerializationRecord::ProcessingResult::FoundDuplicate :
///                                               SerializationRecord::ProcessingResult::Succeeded;
///   } else {
///      // There was a problem with one of our child records
///      processingResult = SerializationRecord::ProcessingResult::Failed;
///   }
///
///   if (nullptr != this->m_namedEntity.get()) {
///      //
///      // We potentially do stats for everything except failure
///      //
///      if (SerializationRecord::ProcessingResult::FoundDuplicate == processingResult) {
///         qDebug() <<
///            Q_FUNC_INFO << "(Late found) duplicate" << this->m_recordDefinition.m_namedEntityClassName << "(" <<
///            this->m_recordDefinition.m_localisedEntityName << ")" << (this->includedInStats() ? " will" : " won't") <<
///            " be included in stats";
///         if (this->includedInStats()) {
///            stats.skipped(this->m_recordDefinition.m_localisedEntityName);
///         }
///      } else if (SerializationRecord::ProcessingResult::Succeeded == processingResult && this->includedInStats()) {
///         stats.processedOk(this->m_recordDefinition.m_localisedEntityName);
///      }
///
///      //
///      // Clean-up
///      //
///      if (SerializationRecord::ProcessingResult::FoundDuplicate == processingResult ||
///          SerializationRecord::ProcessingResult::Failed == processingResult) {
///         //
///         // If we reach here, it means either there was a problem with one of our child records or we ourselves are a
///         // late-detected duplicate.  We've already stored our NamedEntity record in the DB, so we need to try to undo
///         // that by deleting it.  It is the responsibility of each NamedEntity subclass to take care of deleting any
///         // owned stored objects, via the virtual member function NamedEntity::hardDeleteOwnedEntities().  So we don't
///         // have to worry about child records that have already been stored.  (Eg if this is a Mash, and we stored it
///         // and 2 MashSteps before hitting an error on the 3rd MashStep, then deleting the Mash from the DB will also
///         // result in those 2 stored MashSteps getting deleted from the DB.)
///         //
///         qDebug() <<
///            Q_FUNC_INFO << "Deleting stored" << this->m_recordDefinition.m_namedEntityClassName << "as" <<
///            (SerializationRecord::ProcessingResult::FoundDuplicate == processingResult ? "duplicate" : "failed to read all child records");
///         this->deleteNamedEntityFromDb();
///      } else {
///         //
///         // If we read in and stored an outline Fermentable/Hop/etc object (because we could not find any existing
///         // object for which it is a match) then the newly-read in object is no longer an outline.
///         //
///         if (this->m_recordDefinition.isOutlineRecord) {
///            // It would be slightly more robust to set the property via Qt properties, but OTOH it's a coding error if
///            // we can't cast the newly created object to OutlineableNamedEntity.
///            auto createdFromOutline = static_pointer_cast<OutlineableNamedEntity>(this->m_namedEntity);
///            createdFromOutline->setOutline(false);
///         }
///      }
///   }

   //
   // If we read in and stored an outline Fermentable/Hop/etc object (because we could not find any existing
   // object for which it is a match) then the newly-read in object is no longer an outline.
   //
   if (JsonRecord::ProcessingResult::Succeeded == processingResult &&
      this->m_recordDefinition.isOutlineRecord) {
      // It would be slightly more robust to set the property via Qt properties, but OTOH it's a coding error if
      // we can't cast the newly created object to OutlineableNamedEntity.
      auto createdFromOutline = static_pointer_cast<OutlineableNamedEntity>(this->m_namedEntity);
      createdFromOutline->setOutline(false);
   }

   return processingResult;
}

///[[nodiscard]] bool JsonRecord::normaliseAndStoreChildRecordsInDb(QTextStream & userMessage,
///                                                                 ImportRecordCount & stats) {
///   qDebug() << Q_FUNC_INFO << this->m_childRecordSets.size() << "child record sets";
///   //
///   // We are assuming it does not matter which order different types of children are processed in.
///   //
///   // Where there are several children of the same type, we need to process them in the same order as they were read in
///   // from the JSON document because, in some cases, this order matters.  In particular, in BeerJSON, the Mash Steps
///   // inside a Mash are stored in order without any other means of identifying order.
///   //
///   for (auto & childRecordSet : this->m_childRecordSets) {
///      if (childRecordSet.parentFieldDefinition) {
///         qDebug() <<
///            Q_FUNC_INFO << childRecordSet.parentFieldDefinition->propertyPath << "has" <<
///            childRecordSet.records.size() << "entries";
///      } else {
///         qDebug() << Q_FUNC_INFO << "Top-level record has" << childRecordSet.records.size() << "entries";
///      }
///
///      QList<std::shared_ptr<NamedEntity>> processedChildren;
///      for (auto & childRecord : childRecordSet.records) {
///         // The childRecord variable is a reference to a std::unique_ptr (because the vector we're looping over owns the
///         // records it contains), which is why we have all the "member of pointer" (->) operators below.
///         qDebug() <<
///            Q_FUNC_INFO << "Storing" << childRecord->m_recordDefinition.m_namedEntityClassName << "child of" <<
///            this->m_recordDefinition.m_namedEntityClassName;
///         if (SerializationRecord::ProcessingResult::Failed ==
///            childRecord->normaliseAndStoreInDb(this->m_namedEntity, userMessage, stats)) {
///            return false;
///         }
///         processedChildren.append(childRecord->m_namedEntity);
///      }
///
///      //
///      // Now we've stored (and/or recognised as duplicates) the child records of one particular type, we want to link
///      // them (and/or, as the case may be, the records they are duplicates of) to the parent.  If this is possible via a
///      // property (eg the style on a recipe), then we can just do that here.  Otherwise the work needs to be done in the
///      // appropriate subclass of JsonNamedEntityRecord.
///      //
///      // We can't just use the presence or absence of a property name to determine whether the child record can be set
///      // via a property.  It's a necessary but not sufficient condition.  This is because some properties are read-only
///      // in the code (eg because they are calculated values) but need to be present in the FieldDefinition for export to
///      // JSON to work.  However, we can tell whether a property is read-only by calling QMetaProperty::isWritable().
///      //
///      if (childRecordSet.parentFieldDefinition) {
///         auto const & fieldDefinition{*childRecordSet.parentFieldDefinition};
///         Q_ASSERT(std::holds_alternative<JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
///         Q_ASSERT(std::get              <JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
///         JsonRecordDefinition const & childRecordDefinition{
///            *std::get<JsonRecordDefinition const *>(fieldDefinition.valueDecoder)
///         };
///
///         auto const & propertyPath = fieldDefinition.propertyPath;
///         if (!propertyPath.isNull()) {
///            // It's a coding error if we had a property defined for a record that's not trying to populate a NamedEntity
///            // (ie for the root record).
///            Q_ASSERT(this->m_namedEntity);
///
///            QVariant valueToSet;
///            //
///            // How we set the property depends on whether this is a single child record or an array of them
///            //
///            if (fieldDefinition.type != JsonRecordDefinition::FieldType::ListOfRecords) {
///               // It's a coding error if we ended up with more than on child when there's only supposed to be one!
///               if (processedChildren.size() > 1) {
///                  qCritical() <<
///                     Q_FUNC_INFO << "Only expecting one record for" << propertyPath << "property on" <<
///                     this->m_recordDefinition.m_namedEntityClassName << "object, but found" << processedChildren.size();
///                  Q_ASSERT(false);
///               }
///
///               //
///               // We need to pass a pointer to the relevant subclass of NamedEntity (call it of class ChildEntity for
///               // the sake of argument) in to the property setter (inside a QVariant).  As in JsonRecord::toJson, we
///               // need to handle any of the following forms:
///               //    ChildEntity *                               -- eg Equipment *
///               //    std::shared_ptr<ChildEntity>                -- eg std::shared_ptr<Mash>
///               //    std::optional<std::shared_ptr<ChildEntity>> -- eg std::optional<std::shared_ptr<Boil>>
///               //
///               // First we assert that they type is _some_ sort of pointer, otherwise it's a coding error.
///               //
///               auto const & typeInfo = fieldDefinition.propertyPath.getTypeInfo(*this->m_recordDefinition.m_typeLookup);
///               Q_ASSERT(typeInfo.pointerType != TypeInfo::PointerType::NotPointer);
///
///               if (typeInfo.pointerType == TypeInfo::PointerType::RawPointer) {
///                  // For a raw pointer, we don't have to upcast as the pointer will get upcast in the setter during the
///                  // extraction from QVariant
///                  valueToSet = QVariant::fromValue(processedChildren.first().get());
///               } else {
///                  // Should be the only possibility left.
///                  Q_ASSERT(typeInfo.pointerType == TypeInfo::PointerType::SharedPointer);
///                  Q_ASSERT(childRecordDefinition.m_upAndDownCasters.m_pointerUpcaster);
///                  valueToSet = QVariant::fromValue(
///                     childRecordDefinition.m_upAndDownCasters.m_pointerUpcaster(processedChildren.first())
///                  );
///               }
///
///            } else {
///               //
///               // Multi-item setters all take a list of shared pointers
///               //
///               // At this point we have QList<std::shared_ptr<NamedEntity>>, which the setter will not be able to
///               // handle.  We need to convert it to a list of upcast elements -- eg QList<shared_ptr<Hop>>
///               //
///               Q_ASSERT(std::holds_alternative<JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
///               Q_ASSERT(std::get              <JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
///               JsonRecordDefinition const & childRecordDefinition{
///                  *std::get<JsonRecordDefinition const *>(fieldDefinition.valueDecoder)
///               };
///               Q_ASSERT(childRecordDefinition.m_upAndDownCasters.m_listUpcaster);
///               valueToSet = childRecordDefinition.m_upAndDownCasters.m_listUpcaster(processedChildren);
///            }
///
///            if (!propertyPath.setValue(*this->m_namedEntity, valueToSet)) {
///               qCritical() <<
///                  Q_FUNC_INFO << "Could not write" << propertyPath << "property on" <<
///                  this->m_recordDefinition.m_namedEntityClassName;
///                  Q_ASSERT(false);
///            }
///         }
///      }
///   }
///
///   return true;
///}

[[nodiscard]] bool JsonRecord::loadChildRecord(JsonRecordDefinition::FieldDefinition const & parentFieldDefinition,
                                               JsonRecordDefinition const & childRecordDefinition,
                                               boost::json::value & childRecordData,
                                               QTextStream & userMessage) {
   qDebug() << Q_FUNC_INFO;
   // TODO: We could move these 3 lines to the caller to save duplication with loadChildRecords
   auto constructorWrapper = childRecordDefinition.jsonRecordConstructorWrapper;
   this->m_childRecordSets.push_back(JsonRecord::ChildRecordSet{&parentFieldDefinition, {}});
   JsonRecord::ChildRecordSet & childRecordSet = this->m_childRecordSets.back();

   Q_ASSERT(childRecordData.is_object());
   std::unique_ptr<JsonRecord> childRecord{
      constructorWrapper(this->m_coding, childRecordData, childRecordDefinition)
   };
   if (!childRecord->load(userMessage)) {
      return false;
   }
   childRecordSet.records.push_back(std::move(childRecord));
   return true;
}


[[nodiscard]] bool JsonRecord::loadChildRecords(JsonRecordDefinition::FieldDefinition const & parentFieldDefinition,
                                                JsonRecordDefinition const & childRecordDefinition,
                                                boost::json::array & childRecordsData,
                                                QTextStream & userMessage) {
   qDebug() << Q_FUNC_INFO;
   //
   // This is where we have a list of one or more substantive records of a particular type, which may be either at top
   // level (eg hop_varieties) or inside another record that we are in the process of reading (eg hop_additions inside a
   // recipe).  Either way, we need to loop though these "child" records and read each one in with an JsonRecord object
   // of the relevant type.
   //
   auto constructorWrapper = childRecordDefinition.jsonRecordConstructorWrapper;
   this->m_childRecordSets.push_back(JsonRecord::ChildRecordSet{&parentFieldDefinition, {}});
   JsonRecord::ChildRecordSet & childRecordSet = this->m_childRecordSets.back();
   for (auto & recordData : childRecordsData) {
      // Iterating through an array gives us boost::json::value objects
      // We assert that these are boost::json::object key:value containers (because we don't use arrays of other types)
      Q_ASSERT(recordData.is_object());
      std::unique_ptr<JsonRecord> childRecord{
         constructorWrapper(this->m_coding, recordData, childRecordDefinition)
      };
      if (!childRecord->load(userMessage)) {
         return false;
      }
      childRecordSet.records.push_back(std::move(childRecord));
   }

   return true;
}

/**
 * \brief Add a value to a JSON object
 *
 * \param fieldDefinition
 * \param recordDataAsObject
 * \param key
 * \param value
 */
void JsonRecord::insertValue(JsonRecordDefinition::FieldDefinition const & fieldDefinition,
                             boost::json::object & recordDataAsObject,
                             std::string_view const & key,
                             QVariant & value) {
   qDebug() <<
      Q_FUNC_INFO << "Writing" << std::string(key).c_str() << "=" << value << "(type" << fieldDefinition.type <<
      ") for xPath" << fieldDefinition.xPath << ", path" << fieldDefinition.propertyPath;

   //
   // If the Qt property is an optional value, we need to unwrap it from std::optional and then, if it's null, skip
   // writing it out.  Strong typing of std::optional makes this a bit more work here (but it helps us in other ways
   // elsewhere).
   //
   // NB: propertyPath is not actually a property path when fieldType is RequiredConstant
   //
   bool const propertyIsOptional {
      (fieldDefinition.type == JsonRecordDefinition::FieldType::RequiredConstant) ?
         false : fieldDefinition.propertyPath.getTypeInfo(*this->m_recordDefinition.m_typeLookup).isOptional()
   };

   switch(fieldDefinition.type) {
      case JsonRecordDefinition::FieldType::Bool:
         if (Optional::removeOptionalWrapperIfPresent<bool>(value, propertyIsOptional)) {
            recordDataAsObject.emplace(key, value.toBool());
         }
         break;

      case JsonRecordDefinition::FieldType::Int:
         if (Optional::removeOptionalWrapperIfPresent<int>(value, propertyIsOptional)) {
            recordDataAsObject.emplace(key, value.toInt());
         }
         break;

      case JsonRecordDefinition::FieldType::UInt:
         if (Optional::removeOptionalWrapperIfPresent<unsigned int>(value, propertyIsOptional)) {
            recordDataAsObject.emplace(key, value.toUInt());
         }
         break;

      case JsonRecordDefinition::FieldType::Double:
         if (Optional::removeOptionalWrapperIfPresent<double>(value, propertyIsOptional)) {
            recordDataAsObject.emplace(key, value.toDouble());
         }
         break;

      case JsonRecordDefinition::FieldType::String:
         // We mostly don't bother making string fields optional because empty string will suffice for a string value
         // that is not specified.  But we keep the logic consistent here anyway.
         if (Optional::removeOptionalWrapperIfPresent<QString>(value, propertyIsOptional)) {

            std::string valueAsString = value.toString().toStdString();
            // You might think there's no benefit in writing out a field for which we don't have a value.  However, some
            // string fields are required in the BeerJSON schema, which means a file won't validate if they are not
            // present.
            recordDataAsObject.emplace(key, valueAsString);
         }
         break;

      case JsonRecordDefinition::FieldType::Enum:
         // It's definitely a coding error if there is no stringToEnum mapping for a field declared as Enum!
         Q_ASSERT(nullptr != std::get<EnumStringMapping const *>(fieldDefinition.valueDecoder));
         // A non-optional enum should always be convertible to an int; and we always ensure that an optional one is
         // returned as std::optional<int> when accessed via the Qt property system.
         if (Optional::removeOptionalWrapperIfPresent<int>(value, propertyIsOptional)) {
            auto match =
               std::get<EnumStringMapping const *>(fieldDefinition.valueDecoder)->enumAsIntToString(value.toInt());
            // It's a coding error if we couldn't find a string representation for the enum
            Q_ASSERT(match);
            recordDataAsObject.emplace(key, match->toStdString());
         }
         break;

      case JsonRecordDefinition::FieldType::Record:
      case JsonRecordDefinition::FieldType::ListOfRecords:
         // This should be unreachable as JsonRecord::toJson dealt with these cases separately before calling this
         // function, but having an case statement for it eliminates a compiler warning whilst still retaining the
         // useful warning if we have ever omitted processing for another field type.
         Q_ASSERT(false);
         break;

      case JsonRecordDefinition::FieldType::MeasurementWithUnits:
         //
         // Ideally, value would be something we could convert to Measurement::Amount, which would give us units.
         //
         // In practice, it's usually the case that the NamedEntity property will just be a double and the rest of
         // the code "knows" the corresponding Measurement::PhysicalQuantity and therefore the canonical
         // Measurement::Unit that the measurement is in.  Eg if something is a Measurement::PhysicalQuantity::Mass,
         // we always store it in Measurement::Units::kilograms.
         //
         // One day, maybe, we might perhaps change all "measurement" double properties to Measurement::Amount, but,
         // in the meantime, we can get what we need here another way.  We have a list of possible units that could
         // be used in BeerJSON to measure the amount we're looking at.  So we grab the first Measurement::Unit in
         // the list, and, from that, we can trivially get the corresponding canonical Measurement::Unit which will,
         // by the above-mentioned convention, be the right one for the NamedEntity property.
         //
         if (Optional::removeOptionalWrapperIfPresent<double>(value, propertyIsOptional)) {
            // It's definitely a coding error if there is no unit decoder mapping for a field declared to require
            // one
            Q_ASSERT(std::holds_alternative<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder));
            JsonMeasureableUnitsMapping const * const unitsMapping =
               std::get<JsonMeasureableUnitsMapping const *>(fieldDefinition.valueDecoder);
            Q_ASSERT(unitsMapping);
            qDebug() << Q_FUNC_INFO << *unitsMapping;
            Measurement::Unit const * const aUnit = unitsMapping->defaultUnit();
            Measurement::Unit const & canonicalUnit = aUnit->getCanonical();
            qDebug() << Q_FUNC_INFO << canonicalUnit;

            // Now we found canonical units, we need to find the right string to represent them
            auto unitName = unitsMapping->getNameForUnit(canonicalUnit);
            qDebug() << Q_FUNC_INFO << std::string(unitName).c_str();
            recordDataAsObject[key].emplace_object();
            auto & measurementWithUnits = recordDataAsObject[key].as_object();
            measurementWithUnits.emplace(unitsMapping->unitField.asKey(),  unitName);
            measurementWithUnits.emplace(unitsMapping->valueField.asKey(), value.toDouble());
         }
         break;

      case JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits:
         // See comment in utils/MetaTypes.h for why we use Measurement::Amount rather than MassOrVolumeAmt.
         if (Optional::removeOptionalWrapperIfPresent<Measurement::Amount>(value, propertyIsOptional)) {
            // It's definitely a coding error if there is no list of unit decoder mappings for a field declared to
            // require such
            Q_ASSERT(
               nullptr != std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)
            );

            Measurement::Amount amount = value.value<Measurement::Amount>();

            //
            // Logic is similar to MeasurementWithUnits above, except we already have the canonical units
            //
            for (auto const unitsMapping :
                 *std::get<ListOfJsonMeasureableUnitsMappings const *>(fieldDefinition.valueDecoder)) {
               //
               // Each JsonMeasureableUnitsMapping in the ListOfJsonMeasureableUnitsMappings holds units for a single
               // PhysicalQuantity -- ie we have a list of units for mass and another list of units for volume.
               //
               // So the first thing to do is to find the right JsonMeasureableUnitsMapping
               //
               if (unitsMapping->getPhysicalQuantity() == amount.unit->getPhysicalQuantity()) {
                  // Now we have the right PhysicalQuantity, we just need the entry for our Units
                  auto unitName = unitsMapping->getNameForUnit(*amount.unit);
                  qDebug() << Q_FUNC_INFO << std::string(unitName).c_str();
                  recordDataAsObject[key].emplace_object();
                  auto & measurementWithUnits = recordDataAsObject[key].as_object();
                  measurementWithUnits.emplace(unitsMapping->unitField.asKey(),  unitName);
                  measurementWithUnits.emplace(unitsMapping->valueField.asKey(), amount.quantity);
                  break;
               }
            }
         }
         break;

      case JsonRecordDefinition::FieldType::SingleUnitValue:
         if (Optional::removeOptionalWrapperIfPresent<double>(value, propertyIsOptional)) {
            // It's definitely a coding error if there is no unit specifier for a field declared to require one
            JsonSingleUnitSpecifier const * const jsonSingleUnitSpecifier =
               std::get<JsonSingleUnitSpecifier const *>(fieldDefinition.valueDecoder);
            Q_ASSERT(jsonSingleUnitSpecifier);
            // There can be multiple valid (and equivalent) unit names, but we always use the first one for
            // writing.  See json/JsonSingleUnitSpecifier.h for more detail.
            recordDataAsObject[key].emplace_object();
            auto & measurementWithUnits = recordDataAsObject[key].as_object();
            measurementWithUnits.emplace(jsonSingleUnitSpecifier->unitField.asKey(),  jsonSingleUnitSpecifier->validUnits[0]);
            measurementWithUnits.emplace(jsonSingleUnitSpecifier->valueField.asKey(), value.toDouble());
         }
         break;

      //
      // From here on, we have BeerJSON-specific types.  If we ever wanted to parse some other type of JSON,
      // then we might need to make this code more generic, but, for now, we're not going to worry too much as
      // it seems unlikely there will be other JSON encodings we want to deal with in the foreseeable future.
      //
      case JsonRecordDefinition::FieldType::Date:
         if (Optional::removeOptionalWrapperIfPresent<QDate>(value, propertyIsOptional)) {
            // In BeerJSON, DateType is a string matching this regexp:
            //   "\\d{4}-\\d{2}-\\d{2}|\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}"
            // This is One True Date Format™ (aka ISO 8601), which makes our life somewhat easier
            std::string formattedDate = value.toDate().toString(Qt::ISODate).toStdString();
            recordDataAsObject.emplace(key, formattedDate);
         }
         break;

      case JsonRecordDefinition::FieldType::RequiredConstant:
         //
         // This is a field that is required to be in the JSON, but whose value we don't need, and for which we
         // always write a constant value on output.  At the moment it's only needed for the VERSION tag in
         // BeerJSON.
         //
         // Because it's such an edge case, we abuse the propertyPath field to hold the default value (ie what we
         // write out).  This saves having an extra almost-never-used field on
         // JsonRecordDefinition::FieldDefinition.
         //
         recordDataAsObject.emplace(key, fieldDefinition.propertyPath.asXPath().toStdString());
         break;

      // Don't need a default case as we want the compiler to warn us if we didn't cover everything explicitly above
   }

   return;
}

bool JsonRecord::listToJson(QList< std::shared_ptr<NamedEntity> > const & objectsToWrite,
                            boost::json::array & outputArray,
                            JsonCoding const & coding,
                            JsonRecordDefinition const & recordDefinition) {
   for (auto obj : objectsToWrite) {
      // We need the containing entity to be a value of type object.  See comments on JsonRecord constructor in
      // json/JsonRecord.h for why we have to take care about object vs value.
      boost::json::value neJson(boost::json::object_kind); // Can't use braces on this constructor until Boost 1.81!

      std::unique_ptr<JsonRecord> jsonRecord{
         recordDefinition.makeRecord(coding, neJson)
      };
      if (!jsonRecord->toJson(*obj)) {
         return false;
      }

      // boost::json::array::push_back() appends a copy of its argument to the array, so we don't have to worry
      // that neJson and jsonRecord are about to go out of scope
      outputArray.push_back(neJson);
   }
   return true;
}

bool JsonRecord::toJson(NamedEntity const & namedEntityToExport) {
   Q_ASSERT(this->m_recordData.is_object());
   qDebug() <<
      Q_FUNC_INFO << "Exporting JSON for" << namedEntityToExport.metaObject()->className() << "#" <<
      namedEntityToExport.key();

//   boost::json::object & recordDataAsObject = this->recordData.get_object();

   // BeerJSON doesn't care about field order, so we don't either (though it would be relatively small additional work
   // to control field order precisely).
   //
   // Note that it's a coding error if there are no fields in the record definition.  (This usually means a template
   // specialisation was omitted in serialization/json/BeerJson.cpp.)
   //
   qDebug() <<
      Q_FUNC_INFO << "Examining" << this->m_recordDefinition.fieldDefinitions.size() << "field definitions for" <<
      this->m_recordDefinition.m_recordName;
   Q_ASSERT(this->m_recordDefinition.fieldDefinitions.size() > 0);

   for (auto & fieldDefinition : this->m_recordDefinition.fieldDefinitions) {
      qDebug() << Q_FUNC_INFO <<
         "fieldDefinition.xPath:" << fieldDefinition.xPath << ", fieldDefinition.propertyPath:" <<
         fieldDefinition.propertyPath;
      // If there isn't a property name that means this is not a field we support so there's nothing to write out.
      if (fieldDefinition.propertyPath.isNull()) {
         // At the moment at least, we support all sub-record fields, so it's a coding error if one of them does not
         // have a property name.
         Q_ASSERT(JsonRecordDefinition::FieldType::ListOfRecords != fieldDefinition.type);
         continue;
      }

      // Note we have to handle the case where we (ab)use the propertyPath field to hold the value of a required
      // constant.
      QVariant value = (
         fieldDefinition.type == JsonRecordDefinition::FieldType::RequiredConstant ?
         fieldDefinition.propertyPath.asXPath() :
         fieldDefinition.propertyPath.getValue(namedEntityToExport)
      );
      Q_ASSERT(value.isValid());

      //
      // Normally, for each field, we are creating a key:value pair to insert in the current JSON object, where the key
      // comes from fieldDefinition.xPath and the value comes from reading fieldDefinition.propertyPath.  However, there
      // is special case where we are doing something different.
      //
      // If the current field has an empty XPath then it should be a JsonRecordDefinition::FieldType::Record that we are
      // treating differently.  Instead of inserting the subrecord against a key, we insert all the fields (ie all the
      // key:value pairs) of the subrecord into the current object.
      //
      // This is to handle cases such as hop_additions in BeerJSON, where each element of the hop_additions array
      // contains fields both from a RecipeAdditionHop and from a Hop "contained" in that RecipeAdditionHop.
      //
      // See also comment on JsonRecordDefinition::FieldType::Record in serialization/json/JsonRecordDefinition.h
      //
      if (fieldDefinition.xPath.isEmpty()) {
         // It's a coding error if we're trying to give something other than a Record an empty XPath
         Q_ASSERT(JsonRecordDefinition::FieldType::Record == fieldDefinition.type);

         qDebug() <<
            Q_FUNC_INFO << "Empty XPath for property path" << fieldDefinition.propertyPath << "means put its fields in "
            "this record";
      }

      //
      // If we have a non-trivial XPath then we'll need to traverse through any sub-objects and sub-arrays (creating
      // any that are not present) before arriving at the leaf object where we can set the value.
      // JsonXPath::makePointerToLeaf() does all the heaving lifting here -- including correctly handling the case of an
      // empty XPath (where it will just return empty string and leave valuePointer unmodified).  The only problem is
      // when we have made a leaf object and end up not setting the value (eg because it is optional and unset).
      // Having an empty leaf object in the document would give validation problems when we read in.  We solve this in
      // JsonUtils::serialize() by skipping over the output of empty objects.
      //
      // Note that, although we start and end with a boost::json::object, we need to pass in the containing
      // boost::json::value.  (If you have a boost::json::value, you can trivially get to its contained
      // boost::json::object, but you can't do the reverse.  So passing the boost::json::value makes things easier in
      // the function we're calling.)
      //
      boost::json::value * valuePointer = &this->m_recordData;
//      qDebug() <<
//         Q_FUNC_INFO << "valuePointer (" << valuePointer->kind() << ") pre move:" << *valuePointer;
      auto key = fieldDefinition.xPath.makePointerToLeaf(&valuePointer);

      // valuePointer should now be pointing at an object in which we can insert a key:value pair
//      qDebug() <<
//         Q_FUNC_INFO << "valuePointer (" << valuePointer->kind() << ") post move:" << *valuePointer;
      Q_ASSERT(valuePointer->is_object());

      if (JsonRecordDefinition::FieldType::Record        == fieldDefinition.type ||
          JsonRecordDefinition::FieldType::ListOfRecords == fieldDefinition.type) {
         // Comments from the relevant part of JsonRecord::load apply equally here
         Q_ASSERT(std::holds_alternative<JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
         Q_ASSERT(std::get              <JsonRecordDefinition const *>(fieldDefinition.valueDecoder));
         JsonRecordDefinition const & childRecordDefinition{
            *std::get<JsonRecordDefinition const *>(fieldDefinition.valueDecoder)
         };

         if (JsonRecordDefinition::FieldType::Record == fieldDefinition.type) {
            //
            // Things get a bit tricky here because we want a pointer to the child entity (call it of class ChildEntity
            // for the sake of argument) and moreover we need to be able to cast that pointer to a pointer to
            // NamedEntity.  However, the pointer we get back could be any of the following forms:
            //    ChildEntity *                               -- eg Equipment *
            //    std::shared_ptr<ChildEntity>                -- eg std::shared_ptr<Mash>
            //    std::optional<std::shared_ptr<ChildEntity>> -- eg std::optional<std::shared_ptr<Boil>>
            //
            // So we need to handle each possibility.
            //
            // First we assert that they type is _some_ sort of pointer, otherwise it's a coding error.
            //
            auto const & typeInfo = fieldDefinition.propertyPath.getTypeInfo(*this->m_recordDefinition.m_typeLookup);
            Q_ASSERT(typeInfo.pointerType != TypeInfo::PointerType::NotPointer);
            QVariant childNamedEntityVariant = fieldDefinition.propertyPath.getValue(namedEntityToExport);

            // Normally leave this log statement commented out to avoid cluttering the logs
            qDebug() <<
               Q_FUNC_INFO << "childNamedEntityVariant:" << childNamedEntityVariant << ", childRecordDefinition:" <<
               childRecordDefinition;

            std::shared_ptr<NamedEntity> childNamedEntitySp{};
            NamedEntity * childNamedEntity{};
            if (typeInfo.pointerType == TypeInfo::PointerType::RawPointer) {
               // For a raw pointer, the cast is simple as it can happen during the extraction from QVariant
               childNamedEntity = childNamedEntityVariant.value<NamedEntity *>();
            } else {
               // Should be the only possibility left
               Q_ASSERT(typeInfo.pointerType == TypeInfo::PointerType::SharedPointer);
               // For a shared pointer it's a bit more tricky as we can't directly extract the uncast pointer from the
               // QVariant, so we need a little help to apply std::static_pointer_cast.
               Q_ASSERT(childRecordDefinition.m_upAndDownCasters.m_pointerDowncaster);
               childNamedEntitySp =
                  childRecordDefinition.m_upAndDownCasters.m_pointerDowncaster(childNamedEntityVariant);
               childNamedEntity = childNamedEntitySp.get();
            }

            if (childNamedEntity) {
               //
               // We can share more code with the "empty XPath" case if we get any JSON object creation and insertion
               // done first.
               //
               if (!fieldDefinition.xPath.isEmpty()) {
                  boost::json::value newObject(boost::json::object_kind); // Can't use braces on this constructor until Boost 1.81!
                  // boost::json::object::emplace copies the object being inserted, and returns a std::pair where first
                  // is an iterator to the existing or inserted element, and second is true if the insertion took place
                  // or false otherwise.
                  auto emplaceResult = valuePointer->get_object().emplace(key, newObject);
                  if (!emplaceResult.second) {
                     // In the general case, we would expect failure to occur if the key is already present.  Here, it's
                     // probably a coding error but, for now at least, we'll allow for the possibility that there was a
                     // run-time error.
                     qCritical() << Q_FUNC_INFO << "Error inserting new JSON object at key" << key.c_str();
                     return false;
                  }

                  valuePointer = &emplaceResult.first->value();
               }

               //
               // If we made a new sub-object in the JSON document (non-empty XPath), valuePointer is now pointing to
               // it.
               //
               // Otherwise (empty XPath), valuePointer is still pointing to the "current" object.
               //
               qDebug() << Q_FUNC_INFO << "Creating JsonRecord for" << fieldDefinition.propertyPath;
               std::unique_ptr<JsonRecord> subRecord{
                  childRecordDefinition.makeRecord(this->m_coding, *valuePointer)
               };
               if (!subRecord->toJson(*childNamedEntity)) {
                  return false;
               }

            } else {
               qDebug() <<
                  Q_FUNC_INFO << "No child NamedEntity for xPath" << fieldDefinition.xPath << "/ propertyPath:" <<
                  fieldDefinition.propertyPath;
            }
         } else {
            boost::json::array outputArray;

            //
            // We have to be careful about how we get the list of objects we want to write out.  Accessing lists of
            // objects via the Qt Property system, we'd get a bunch of different things inside the returned QVariant
            // (QList<std::shared_ptr<BrewNote>>, QList<std::shared_ptr<RecipeAdditionHop>> etc) that have no common
            // base class.  So we would not normally be able to easily extract from the QVariant in generic code here.
            // However, we have a pointer to the relevant instantiation of NamedEntity::downcastListFromVariant, which
            // will correctly convert the QVariant to QList<std::shared_ptr<NamedEntity>>.
            //
            qDebug() << Q_FUNC_INFO << "value: " << value;
            Q_ASSERT(childRecordDefinition.m_upAndDownCasters.m_listDowncaster);
            QList< std::shared_ptr<NamedEntity> > objectsToWrite =
               childRecordDefinition.m_upAndDownCasters.m_listDowncaster(value);
            qDebug() << Q_FUNC_INFO << "value (" << value << ") gives" << objectsToWrite.size() << "objects";

            //
            // In theory we could add some logic here to decide whether to write the array out if it is of zero length.
            // However, we would need to know whether the field is optional or required in the JSON schema and we do not
            // currently record this.  For now, we write out all arrays, even if they are empty, as some of them are
            // required fields (eg recipe/ingredients/fermentable_additions in BeerJSON).
            //
            JsonRecord::listToJson(objectsToWrite, outputArray, this->m_coding, childRecordDefinition);
            valuePointer->get_object().emplace(key, outputArray);
         }

      } else {
         this->insertValue(fieldDefinition, valuePointer->get_object(), key, value);
      }
   }

   return true;
}
