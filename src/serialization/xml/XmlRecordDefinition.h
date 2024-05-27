/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlRecordDefinition.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_XML_XMLRECORDDEFINITION_H
#define SERIALIZATION_XML_XMLRECORDDEFINITION_H
#pragma once

#include <memory>
#include <utility> // For std::in_place_type_t
#include <variant>

#include "measurement/Unit.h"
#include "serialization/xml/XQString.h"
#include "serialization/SerializationRecordDefinition.h"
#include "utils/EnumStringMapping.h"
#include "utils/PropertyPath.h"

// Forward declarations
class XmlCoding;
class XmlRecord;
template<class NE>
class XmlNamedEntityRecord;

/**
 * \brief \c XmlRecordDefinition represents a type of data record in an XML document.  Each instance of this class is a
 *        constant entity that tells us how to map between a particular XML record type and our internal data
 *        structures.
 *
 *        The related \c XmlRecord class holds data about a specific individual record that we are reading from or
 *        writing to an XML document.  It also does all the reading and writing, and is subclassed where we need special
 *        processing for different types of \c NamedEntity.
 *
 *        NB: In theory we should separate out BeerXML specifics from more generic XML capabilities, in case there is
 *        ever some other format of XML that we want to use.
 *        In practice, these things seem sufficiently unlikely that we can cross that bridge if and when we come to it.
 */
class XmlRecordDefinition : public SerializationRecordDefinition {
public:
   /**
    * \brief The types of fields that we know how to process.  Used in \b FieldDefinition records
    */
   enum class FieldType {
      Bool            ,
      Int             ,
      UInt            ,
      Double          ,
      String          ,
      Date            ,
      Enum            ,  // A string that we need to map to/from our own enum
      RequiredConstant,  // A fixed value we have to write out in the record (used for BeerXML VERSION tag)
      Record          ,  // Single contained record
      ListOfRecords   ,  // Zero, one or more contained records
      Unit            ,  // Stored as a string in the DB.  Only used in extension tags
   };

   /**
    * \brief How to parse every field that we want to be able to read out of the XML file.  See class description for
    *        more details.
    */
   struct FieldDefinition {
      FieldType    type;
      XQString     xPath;
      PropertyPath propertyPath; // If fieldType == ListOfRecords, then this is used only on export
                                 // If fieldType == RequiredConstant, then this is actually the constant value
      using ValueDecoder =
         std::variant<std::monostate,
                      EnumStringMapping              const *,  // FieldType::Enum
                      Measurement::UnitStringMapping const *,  // FieldType::Unit
                      XmlRecordDefinition            const *,  // FieldType::Array
                      double                         >;        // Default value (for fields that are required in the XML
                                                               // but optional in our internal data model).
      ValueDecoder valueDecoder;
      /**
       * Defining a constructor allows us to control the default value of valueDecoder
       */
      FieldDefinition(FieldType    type,
                      XQString     xPath,
                      PropertyPath propertyPath,
                      ValueDecoder valueDecoder = ValueDecoder{});
   };

   /**
    * \brief Part of the data we want to store in an \c XmlRecordDefinition is something that tells it what subclass (if
    *        any) of \c XmlRecord needs to be created to handle this type of record.  We can't pass a pointer to a
    *        constructor as that's not permitted in C++.  But we can pass a pointer to a static templated wrapper
    *        function that just invokes the constructor to create the object on the heap, which is good enough for our
    *        purposes, eg:
    *           XmlRecordDefinition::create< XmlRecord >
    *           XmlRecordDefinition::create< XmlRecipeRecord >
    *           XmlRecordDefinition::create< XmlNamedEntityRecord< Hop > >
    *           XmlRecordDefinition::create< XmlNamedEntityRecord< Yeast > >
    *
    *        (We maybe could have called this function xmlRecordConstructorWrapper but it makes things rather long-
    *        winded in the definitions.)
    */
   template<typename XRT>
   static std::unique_ptr<XmlRecord> create(XmlCoding           const & xmlCoding,
                                            XmlRecordDefinition const & recordDefinition) {
      return std::make_unique<XRT>(xmlCoding, recordDefinition);
   }

   /**
    * \brief This is just a convenience typedef representing a pointer to a template instantiation of
    *        \b XmlRecordDefinition::create().
    */
   typedef std::unique_ptr<XmlRecord> (*XmlRecordConstructorWrapper)(XmlCoding const & xmlCoding,
                                                                     XmlRecordDefinition const & recordDefinition);

   /**
    * \brief Constructor
    * \param recordName The name of the XML object for this type of record, eg "fermentables" for a list of
    *                   fermentables in BeerXML.
    * \param typeLookup The \c TypeLookup object that, amongst other things allows us to tell whether Qt properties on
    *                   this object type are "optional" (ie wrapped in \c std::optional)
    * \param namedEntityClassName The class name of the \c NamedEntity to which this record relates, eg "Fermentable",
    *                             or empty string if there is none
    * \param upAndDownCasters gives us all the up- and down-cast functions for the class to which this record relates
    * \param xmlRecordConstructorWrapper
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    */
   XmlRecordDefinition(char                   const * const   recordName,
                       TypeLookup             const * const   typeLookup,
                       char                   const * const   namedEntityClassName,
                       QString                        const & localisedEntityName,
                       NamedEntity::UpAndDownCasters  const   upAndDownCasters,
                       XmlRecordConstructorWrapper            xmlRecordConstructorWrapper,
                       std::initializer_list<FieldDefinition> fieldDefinitions);


   /**
    * \brief Of course we want to be able to deduce some of the parameters to the constructor rather than laboriously
    *        specify "the same but for this model class" each time.  The trick is that we must have one constructor
    *        parameter that depends on the type.  This is what std::in_place_type_t does for us.
    */
   template<typename T>
   XmlRecordDefinition(std::in_place_type_t<T>,
                       char                     const * const recordName,
                       XmlRecordConstructorWrapper            xmlRecordConstructorWrapper,
                       std::initializer_list<FieldDefinition> fieldDefinitions) :
      XmlRecordDefinition(recordName,
                          &T::typeLookup,
                          T::staticMetaObject.className(),
                          T::localisedName(),
                          NamedEntity::makeUpAndDownCasters<T>(),
                          xmlRecordConstructorWrapper,
                          fieldDefinitions) {
      return;
   }

   /**
    * \brief Alternate Constructor allowing a list of lists of fields
    * \param fieldDefinitions A list of lists of fields we expect to find in this record (other fields will be ignored)
    *                         and how to parse them.  Effectively the constructor just concatenates all the lists.
    *                         See comments fin BeerXml.cpp for why we want to do this.
    */
   XmlRecordDefinition(char                     const * const recordName,
                       TypeLookup               const * const typeLookup,
                       char                     const * const namedEntityClassName,
                       QString                        const & localisedEntityName,
                       NamedEntity::UpAndDownCasters  const   upAndDownCasters,
                       XmlRecordConstructorWrapper            xmlRecordConstructorWrapper,
                       std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists);
   template<typename T>
   XmlRecordDefinition(std::in_place_type_t<T>,
                       char                     const * const recordName,
                       XmlRecordConstructorWrapper            xmlRecordConstructorWrapper,
                       std::initializer_list< std::initializer_list<FieldDefinition> > fieldDefinitionLists) :
      XmlRecordDefinition(recordName,
                          &T::typeLookup,
                          T::staticMetaObject.className(),
                          T::localisedName(),
                          NamedEntity::makeUpAndDownCasters<T>(),
                          xmlRecordConstructorWrapper,
                          fieldDefinitionLists) {
      return;
   }

   /**
    * \brief This is the simplest way to get the right type of \c XmlRecord for this \c XmlRecordDefinition.  It
    *        ensures you get the right subclass (if any) of \c XmlRecord.
    */
   std::unique_ptr<XmlRecord> makeRecord(XmlCoding const & xmlCoding) const;

public:
   XmlRecordConstructorWrapper xmlRecordConstructorWrapper;

   std::vector<FieldDefinition> const fieldDefinitions;
};


/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, XmlRecordDefinition::FieldType const fieldType);

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, XmlRecordDefinition::FieldDefinition const & fieldDefinition);

#endif
