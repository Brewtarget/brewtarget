/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonRecordDefinition.h is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#ifndef SERIALIZATION_JSON_JSONRECORDDEFINITION_H
#define SERIALIZATION_JSON_JSONRECORDDEFINITION_H
#pragma once

#include <memory>
#include <utility> // For std::in_place_type_t
#include <variant>

#include <QVector>

#include "model/NamedEntity.h"
#include "serialization/SerializationRecordDefinition.h"
#include "serialization/json/JsonMeasureableUnitsMapping.h"
#include "serialization/json/JsonSingleUnitSpecifier.h"
#include "serialization/json/JsonXPath.h"
#include "utils/EnumStringMapping.h"
#include "utils/PropertyPath.h"

// Forward declarations
namespace boost::json {
   class object;
   class value;
}
class JsonCoding;
class JsonRecord;
template<class NE>
class JsonNamedEntityRecord;

// See comment below about OneOfMeasurementsWithUnits for more on this.  It's useful to have a type name for a list of
// pointers to JsonMeasureableUnitsMapping objects, but we don't need to create a whole new class just for that.
using ListOfJsonMeasureableUnitsMappings = QVector<JsonMeasureableUnitsMapping const *>;

/**
 * \brief \c JsonRecordDefinition represents a type of data record in a JSON document.  Each instance of this class is a
 *        constant entity that tells us how to map between a particular JSON record type and our internal data
 *        structures.
 *
 *        The related \c JsonRecord class holds data about a specific individual record that we are reading from or
 *        writing to a JSON document.  It also does all the reading and writing, and is subclassed where we need special
 *        processing for different types of \c NamedEntity.
 *
 *        NB: In theory we should separate out BeerJSON specifics from more generic JSON capabilities, in case there is
 *        ever some other format of JSON that we want to use, or in case future versions of BeerJSON change radically.
 *        In practice, these things seem sufficiently unlikely that we can cross that bridge if and when we come to it.
 */
class JsonRecordDefinition : public SerializationRecordDefinition {
public:
   /**
    * \brief The types of fields that we know how to process.  Used in \b FieldDefinition records
    *
    *        JSON
    *        ----
    *        Per https://www.json.org/json-en.html, in JSON, a value is one of the following:
    *           object
    *           array
    *           string
    *           number
    *           "true"
    *           "false"
    *           "null"
    *
    *        JSON also offers "integer" as a specialisation of number (integer being a JSON type used in the definition
    *        of number).
    *
    *        Correspondingly (more or less), if you have a boost::json::value (see comment in json/JsonSchema.cpp for
    *        why we are using Boost.JSON) then its kind() member function will return one of the following values:
    *           boost::json::kind::object
    *           boost::json::kind::array
    *           boost::json::kind::string
    *           boost::json::kind::uint64
    *           boost::json::kind::int64
    *           boost::json::kind::double_
    *           boost::json::kind::bool_
    *           boost::json::kind::null
    *
    *       JSON Schemas Generally
    *       ----------------------
    *       JSON itself doesn't have an enum type, but a JSON schema (see https://json-schema.org/) can achieve the same
    *       effect by restricting the values a string can take to those in a fixed list.
    *
    *       Similarly, a JSON schema can enforce restrictions on string values via regular expressions (see
    *       https://json-schema.org/understanding-json-schema/reference/regular_expressions.html).  This is used in
    *       BeerJSON for its DateType -- see below.
    *
    *       BeerJSON Specifically
    *       ---------------------
    *       In contrast with BeerXML and our database store, where we specify a canonical unit of measure for each field
    *       (eg temperatures are always stored as degrees celcius), BeerJSON allows lots of different units of measure.
    *       Thus a lot of the base types in BeerJSON consist of unit & value, where unit is an enum (ie string with
    *       restricted set of values) and value is a decimal or integer number.  This is a more universal approach in
    *       allowing multiple units to be used for temperature, time, color, etc, but it also means we have a lot more
    *       "base" types than for BeerXML or ObjectStore.  (It also means that it's harder for the schema to do bounds
    *       validation on such values.)
    *
    *       In some cases, BeerJSON only allows one unit of measurement, but the same structure of {unit, value} is
    *       maintained, presumably for extensibility.
    *
    *       The main BeerJSON base types are:
    *          AcidityType:        unit ∈ {"pH"} (NB: one-element set)
    *                              value : decimal
    *          BitternessType:     unit ∈ {"IBUs"} (NB: one-element set)
    *                              value : decimal
    *          CarbonationType:    unit ∈ {"vols", "g/l"}
    *                              value : decimal
    *          ColorType:          unit ∈ {"EBC", "Lovi", "SRM"}
    *                              value : decimal
    *          ConcentrationType:  unit ∈ {"ppm", "ppb", "mg/l"}
    *                              value : decimal
    *          DiastaticPowerType: unit ∈ {"Lintner", "WK"}
    *                              value : decimal
    *          GravityType:        unit ∈ {"sg", "plato", "brix" }
    *                              value : decimal
    *          MassType:           unit ∈ {"mg", "g", "kg", "lb", "oz"}
    *                              value : decimal
    *          PercentType:        unit ∈ {"%"} (NB: one-element set)
    *                              value : decimal
    *          PressureType:       unit ∈ {"kPa", "psi", "bar" }
    *                              value : decimal
    *          SpecificHeatType:   unit ∈ {"Cal/(g C)", "J/(kg K)", "BTU/(lb F)" }
    *                              value : decimal
    *          SpecificVolumeType: unit ∈ {"qt/lb", "gal/lb", "gal/oz", "l/g", "l/kg", "floz/oz", "m^3/kg", "ft^3/lb"}
    *                              value : decimal
    *          TemperatureType:    unit ∈ {"C", "F"}
    *                              value : decimal
    *          TimeType:           unit ∈ {"sec", "min", "hr", "day", "week"}
    *                              value : integer
    *          UnitType:           unit ∈ {"1", "unit", "each", "dimensionless", "pkg"}
    *                              value : decimal
    *          ViscosityType:      unit ∈ {"cP", "mPa-s"}
    *                              value : decimal
    *          VolumeType:         unit ∈ {"ml", "l", "tsp", "tbsp", "floz", "cup", "pt", "qt", "gal", "bbl", "ifloz",
    *                                      "ipt", "iqt", "igal", "ibbl"}
    *                              value : decimal
    *
    *       Furthermore, for many of these types, an additional "range" type is defined - eg GravityRangeType,
    *       BitternessRangeType, etc are used in beer styles.  The range type is just an object with two required
    *       elements, minimum and maximum, of the underlying type (eg GravityType for the members of GravityRangeType,
    *       BitternessType for the members of BitternessRangeType, etc).  This means we can treat it as two separate
    *       entries that differ by XPath (blah/minimum and blah/maximum).
    *
    *       BeerJSON also has DateType which is a regexp restriction on a string.  The regexp is a bit cumbersome, but
    *       it boils down to allowing either of the following formats where 'd' is a digit (ie 0-9):
    *          dddd-dd-dd
    *          dddd-dd-ddTdd:dd:dd
    *       We take this to mean ISO 8601 is used for date fields.  (Hurrah!)
    */
   enum class FieldType {
      //
      // These values correspond more-or-less with base JSON types
      //
      Bool,     // boost::json::kind::bool_
      Int,      // boost::json::kind::int64
      UInt,     // boost::json::kind::uint64
      Double,   // boost::json::kind::double_
      String,   // boost::json::kind::string
      Enum,     // A string that we need to map to/from our own enum
      //
      // A Record is usually a single directly-contained record - eg the Mash inside a Recipe.  However, when the xPath
      // field is empty (ie set to "") then we have a special case of a "Base Record".
      //
      // In some places, BeerJSON uses inheritance where we use "refers to".  For instance, the HopAdditionType record,
      // used for 'recipes[]/ingredients/hop_additions[]', inherits from HopBase (and adds 'timing' and 'amount'
      // fields).  This is at odds with our RecipeAdditionHop class, which refers to a Hop (by storing its ID).  So,
      // when we're reading/writing a HopAdditionType BeerJSON record, we want to be able to use different fields of the
      // same BeerJSON record to map to the Hop that's "inside" RecipeAdditionHop.
      //
      // This Base Record special case sounds more painful than it is(!)  Actually, because we always drive reading and
      // writing from our side of the model (ie we loop through the fields of each of our objects and either write them
      // to the output file or see if they are present in the input file), the two-records-in-one largely "just works".
      //
      Record,
      ListOfRecords,    // An array containing Zero, one or more contained records in an array structure
      //
      // === Other types start here ===
      //
      // For now we treat Date as synonymous with the BeerJSON date type (DateType in measurable_units.json in the
      // BeerJSON schema).  We'll wait to worry about doing something more generic until another JSON format comes
      // along (if ever).
      //
      Date,
      //
      // MeasurementWithUnits covers most cases where we need to convert between a value plus unit (used a lot in
      // BeerJSON) and our internal canonical (usually SI) units via a Measurement::Unit constant.
      //
      // All the units should correspond to the same Measurement::PhysicalQuantity, otherwise OneOfMeasurementsWithUnits
      // should be used.
      //
      MeasurementWithUnits,
      //
      // In a few circumstances, a field is allowed to be measured in two different ways - eg some quantities can be
      // measured by mass or by volume.  Of course, this only works when there are no shared abbreviations between the
      // two different types of units, but, eg in the case of BeerJSON, the schema creators have taken care to ensure
      // this is the case.
      //
      // NOTE that we ASSUME that each of the JsonMeasureableUnitsMapping objects in the list has the SAME values for
      // unitField and valueField.  (We could have done yet another data structure to enforce this, instead of using
      // a list of JsonMeasureableUnitsMapping objects, but the latter has the benefit of allowing us to use the same
      // definitions for fields that can only be measured in one way.  Eg, we might have some fields that are allowed
      // to be specified by mass or by volume and others that can only be specified by mass or only by volume.)
      //
      OneOfMeasurementsWithUnits,
      //
      // SingleUnitValue is a special case of MeasurementWithUnits for where we don't want to use a Measurement::Unit
      // internally because there is only ever one unit.  Eg BeerJSON stores percentages as value plus unit with unit
      // always set to "%".  (We could just XPath to the value field, but this (a) gives us a mechanism to assert that
      // the unit field holds the value we think it should, and (b) tells us how to write these fields out when we are
      // exporting to JSON.)
      //
      // Note that we have to be capable of handling synonyms here.  Eg in BeerJSON, there is a unit type called
      // UnitUnitType, which can be "1", "unit", "each", "dimensionless" or "pkg" but which in all these cases means
      // "number without units" (eg number of apples or number of packets of yeast).
      //
      SingleUnitValue,
      RequiredConstant  // A fixed value we have to write out in the record (used for BeerJSON VERSION tag)
   };

   /**
    * \brief How to parse every field that we want to be able to read out of the JSON file.  See class description for
    *        more details.
    *
    *        In general, \c xPath ‡ is simply the key of a key:value pair to read/write from/to a JSON object and
    *        \c propertyName tells us which property of the \c NamedEntity we are reading/writing holds this field.  (In
    *        such cases, if it is \c BtString::NULL_STR then that means we don't support this property and neither read
    *        nor write it.)
    *
    *        ‡ JSON's equivalent of XML's XPath is a JSON Pointer -- see RFC 6901 "JavaScript Object Notation (JSON)
    *        Pointer" -- which, since Boost version 1.79.0, is supported by Boost.JSON (via its \c at_pointer() and
    *        \c find_pointer() functions).  However, we've used "xPath" rather than "pointer" or "jsonPointer" as the
    *        name here because "pointer" already has other meanings in C++ so (IHMO) XPath gets the meaning across
    *        better.  It's also easier to grep for in the code base!
    *
    *        If \c type is \c JsonRecordDefinition::FieldType::ListOfRecords, then \c valueDecoder holds a pointer to
    *        the \c JsonRecordDefinition needed for the array elements.
    *
    *        If \c type is \c RequiredConstant, then \c propertyName is actually a constant value that we need to write
    *        but not read (eg BeerJSON version number).
    *
    *        If \c type is \c JsonRecordDefinition::FieldType::Enum, then \c valueDecoder.enumMapping tells us how to
    *        map between string values and our own enums.
    *
    *        If \c type is \c JsonRecordDefinition::FieldType::MeasurementWithUnits, then \c valueDecoder.unitsMapping
    *        tells us how to map between the string values for units (eg "oz") and our own \c Measurement::Unit
    *        constants.
    *
    *        If \c type is \c JsonRecordDefinition::FieldType::OneOfMeasurementsWithUnits, then
    *        \c valueDecoder.listOfUnitsMappings gives us the list of valid mappings (each of which individually would
    *        be valid in the case of \c JsonRecordDefinition::FieldType::MeasurementWithUnits).  Most commonly this is
    *        used where there is the choice of measuring something by mass or by volume.
    *
    *        If \c type is \c JsonRecordDefinition::FieldType::SingleUnitValue, then this is a field that either only
    *        has one unit or has no units, and \c valueDecoder.singleUnitSpecifier tells us what name(s) of unit is/are
    *        valid.
    */
   struct FieldDefinition {
      FieldType    type;
      JsonXPath    xPath;
      PropertyPath propertyPath;
      using ValueDecoder =
         std::variant<std::monostate,
                      EnumStringMapping                  const *,  // FieldType::Enum
                      JsonMeasureableUnitsMapping        const *,  // FieldType::MeasurementWithUnits
                      ListOfJsonMeasureableUnitsMappings const *,  // FieldType::OneOfMeasurementsWithUnits
                      JsonSingleUnitSpecifier            const *,  // FieldType::SingleUnitValue
                      JsonRecordDefinition               const *>; // FieldType::ListOfRecords
      ValueDecoder valueDecoder;
      /**
       * \brief Trivial constructor allows us to default \c valueDecoder and saves us having to put & before every
       *        property name!
       */
      FieldDefinition(FieldType type,
                      JsonXPath xPath,
                      PropertyPath propertyPath,
                      ValueDecoder valueDecoder = ValueDecoder{});
   };

   /**
    * \brief Part of the data we want to store in a \c JsonRecordDefinition is something that tells it what subclass (if
    *        any) of \c JsonRecord needs to be created to handle this type of record.  We can't pass a pointer to a
    *        constructor as that's not permitted in C++.  But we can pass a pointer to a static templated wrapper
    *        function that just invokes the constructor to create the object on the heap, which is good enough for our
    *        purposes, eg:
    *           JsonRecordDefinition::create< JsonRecord >
    *           JsonRecordDefinition::create< JsonRecipeRecord >
    *           JsonRecordDefinition::create< JsonNamedEntityRecord< Hop > >
    *           JsonRecordDefinition::create< JsonNamedEntityRecord< Yeast > >
    *
    *        (We maybe could have called this function jsonRecordConstructorWrapper but it makes things rather long-
    *        winded in the definitions.)
    */
   template<typename JRT>
   static std::unique_ptr<JsonRecord> create(JsonCoding           const & jsonCoding,
                                             boost::json::value         & recordData,
                                             JsonRecordDefinition const & recordDefinition) {
      return std::make_unique<JRT>(jsonCoding, recordData, recordDefinition);
   }

   /**
    * \brief This is just a convenience typedef representing a pointer to a template instantiation of
    *        \b JsonRecordDefinition::create().
    */
   typedef std::unique_ptr<JsonRecord> (*JsonRecordConstructorWrapper)(JsonCoding const & jsonCoding,
                                                                       boost::json::value & recordData,
                                                                       JsonRecordDefinition const & recordDefinition);

   /**
    * \brief This enum is used for initialising \c isOutlineRecord
    */
   enum class RecordType {
      Normal,
      Outline
   };

   /**
    * \brief Constructor
    * \param recordName The name of the JSON object for this type of record, eg "fermentables" for a list of
    *                   fermentables in BeerJSON.   TODO: I think we can get rid of this.
    * \param typeLookup The \c TypeLookup object that, amongst other things allows us to tell whether Qt properties on
    *                   this object type are "optional" (ie wrapped in \c std::optional)
    * \param namedEntityClassName The class name of the \c NamedEntity to which this record relates, eg "Fermentable",
    *                             or empty string if there is none
    * \param upAndDownCasters gives us all the up- and down-cast functions for the class to which this record relates
    * \param jsonRecordConstructorWrapper
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    */
   JsonRecordDefinition(char                   const * const   recordName,
                        TypeLookup             const * const   typeLookup,
                        char                   const * const   namedEntityClassName,
                        QString                        const & localisedEntityName,
                        NamedEntityCasters             const   upAndDownCasters,
                        JsonRecordConstructorWrapper           jsonRecordConstructorWrapper,
                        std::initializer_list<FieldDefinition> fieldDefinitions,
                        RecordType                     const   recordType);

   /**
    * \brief Of course we want to be able to deduce some of the parameters to the constructor rather than laboriously
    *        specify "the same but for this model class" each time.  The trick is that we must have one constructor
    *        parameter that depends on the type.  This is what std::in_place_type_t does for us.
    */
   template<typename T>
   JsonRecordDefinition(std::in_place_type_t<T>,
                        char                     const * const recordName,
                        std::initializer_list<FieldDefinition> fieldDefinitions,
                        RecordType                       const recordType = RecordType::Normal) :
      JsonRecordDefinition(recordName,
                           &T::typeLookup,
                           T::staticMetaObject.className(),
                           T::localisedName(),
                           NamedEntityCasters::construct<T>(),
                           JsonRecordDefinition::create<JsonNamedEntityRecord<T>>,
                           fieldDefinitions,
                           recordType) {
      return;
   }

   /**
    * \brief Alternate Constructor allowing a list of lists of fields
    * \param fieldDefinitions A list of lists of fields we expect to find in this record (other fields will be ignored)
    *                         and how to parse them.  Effectively the constructor just concatenates all the lists.
    *                         See comments fin BeerJson.cpp for why we want to do this.
    */
   JsonRecordDefinition(char                   const * const   recordName,
                        TypeLookup             const * const   typeLookup,
                        char                   const * const   namedEntityClassName,
                        QString                        const & localisedEntityName,
                        NamedEntityCasters             const   upAndDownCasters,
                        JsonRecordConstructorWrapper           jsonRecordConstructorWrapper,
                        std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists,
                        RecordType                     const   recordType);
   template<typename T>
   JsonRecordDefinition(std::in_place_type_t<T>,
                        char                     const * const recordName,
                        std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists,
                        RecordType                       const recordType = RecordType::Normal) :
      JsonRecordDefinition(recordName,
                           &T::typeLookup,
                           T::staticMetaObject.className(),
                           T::localisedName(),
                           NamedEntityCasters::construct<T>(),
                           JsonRecordDefinition::create<JsonNamedEntityRecord<T>>,
                           fieldDefinitionLists,
                           recordType) {
      return;
   }

   /**
    * \brief This is the simplest way to get the right type of \c JsonRecord for this \c JsonRecordDefinition.  It
    *        ensures you get the right subclass (if any) of \c JsonRecord.
    */
   std::unique_ptr<JsonRecord> makeRecord(JsonCoding const & jsonCoding, boost::json::value & recordData) const;

public:

   JsonRecordConstructorWrapper jsonRecordConstructorWrapper;

   std::vector<FieldDefinition> const fieldDefinitions;

   //! See comments in model/OutlineableNamedEntity.h
   bool isOutlineRecord;
};


/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, JsonRecordDefinition::FieldType const fieldType);

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, JsonRecordDefinition::FieldDefinition const & fieldDefinition);


#endif
