/*
 * xml/XmlRecord.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLRECORD_H
#define _XML_XMLRECORD_H
#pragma once

#include <memory>

#include <QTextStream>

#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XPath/NodeRefList.hpp>

#include "xml/XmlRecordCount.h"
#include "xml/XQString.h"

class XmlCoding;

///
/// XmlCoding - provides the look-up for XmlRecords
///
/// XmlRecord
///   recordName = name of the XML tag containing this record
///   fieldDefinitions = the fields we expect to find in this record (other fields will be ignored)
///   XALAN_NODE_TYPES
///
/// XmlNamedEntityRecord aka Record corresponding to a NamedEntity
///    entityToPopulate = namedEntityToPopulate
///    uniquenessOfInstanceNames
///    fieldsRead - though we might ditch this
///
/// XmlMultipleRecord aka RecordSet corresponding to a collection of XmlRecords records
///
///
/**
 * Broadly speaking we think of an "brewing" XML document (eg a BeerXML document) as a tree of "records", and of each
 * of these "records" being fundamentally one of two types.  (Note that this is a simpler, and subtly different, way of
 * abstracting things than the BeerXML terminology, which distinguishes between "records" and "record sets".)
 *
 * In our model, both types of record can themselves contain other records.  In fact, the simpler of our two types of
 * record, which we represent with \b XmlRecord, can only do this.  It is just a grouping record that contain other
 * records.  The other type of record, which we represent with (specialisations of) \b XmlNamedEntityRecord, is a data
 * record.  It corresponds to an object of a subclass of \b NamedEntity (ie a Hop, a Yeast, a Recipe, an  Equipment, a Style,
 * etc) and so can contain other records when this makes sense.  Eg a recipe record will usually contain a style record,
 * one ore more hop records, and so on.  Similarly, a mash record will contain a number of mash step records.
 *
 * Thus \b XmlNamedEntityRecord inherits from (ie generalises) \b XmlRecord.  (Actually \b XmlNamedEntityRecord is a
 * templated class so a hop record will be processed by \b XmlNamedEntityRecord<Hop>, a recipe record by
 * \b XmlNamedEntityRecord<Recipe>, and so on.)  In both cases, each record that we're parsing (either reading from or
 * writing to an XML file) is represented by an object of type \b XmlRecord or some subclass thereof.
 *
 * Each \b XmlRecord object has one or more fields.   Each field is either another \b XmlRecord or a simple value (ie a
 * string, some sort of number, a boolean or an enum).  There may be further constraints on these fields, eg a number
 * that is only supposed to be between 0 and 100, but we mostly do not need to worry about them here.  Data that we are
 * exporting to an XML document is assumed to be valid already.  And data we are importing from an XML document is
 * validated against an XML Schema Document (XSD).
 *
 * In a given type of record, each field has a type (\b XmlRecord::FieldType) and an XPath within that containing
 * record.  There can be multiple instances of a field in a record (but all these instances must be of the same type,
 * and we currently only need to support multiple instances where that type is \b Record).  Where a field is a record,
 * its XML tag tells us what type of record it is (via a look-up explained below).  The use of XPaths is a key
 * simplification, as the following example illustrates.  Consider a simplified BeerXML RECIPE record:
 *    <RECIPE>
 *       <NAME>Oatmeal Stout</NAME>
 *       <STYLE>...</STYLE>
 *       <HOPS>
 *          <HOP><NAME>Fuggle</NAME>...</HOP>
 *          <HOP><NAME>Golding</NAME>...</HOP>
 *       </HOPS>
 *       ...
 *    <RECIPE>
 * In our model the part of the RECIPE record shown here has three fields:
 *    • NAME, which is a string, and of which there is one instance
 *    • STYLE, which is a record (of type \b XmlNamedEntityRecord<Recipe>), and of which there is also one instance
 *    • HOPS/HOP, which is a record (of type \b XmlNamedEntityRecord<Hop>), and of which there are two instances
 * Using XPath saves us from having to create special processing to handle the <HOPS>...</HOPS> element.  We an just
 * "see through" it to the records it wraps.  Moreover, although the Fuggle and Golding hop fields have XPath
 * "HOPS/HOP", it is the HOP tag alone that tells us what type of record they are.
 *
 * For a given XML encoding (eg BeerXML 1.0) we have various bits of mapping data stored in an instance of \b XmlCoding.
 * These suffice for it to know, for any XML element in this encoding that holds a record (eg <HOP>...</HOP>):
 *    • Which class handles that type of record (eg \b XmlNamedEntityRecord<Hop>).  (Actually what we need to know is
 *      how to construct an instance of the class that handles this type of record.  So instead of storing a class name,
 *      we store a pointer to a function that can invoke the constructor on the class we want.)
 *    • For this class, what mappings are needed to construct an instance of it that works with this encoding.
 *      Specifically this is a list of field definitions, each being an \b XmlRecord::Field that holds.
 *       ‣ Field type (encodes as \b XmlRecord::FieldType enum)
 *       ‣ The XPath within this record of this field
 *       ‣ For a simple field that is not a record, where to store it (via the name of the name of the Q_PROPERTY of
 *         the subclass of \b NamedEntity that this type of record corresponds to).  This can be null for fields that
 *         either will not be stored or will be handled by special processing.  (TBD We could theoretically use
 *         Q_PROPERTY to store records inside their parents, provided we think about when to do it...)
 *       ‣ If the field type is an enum (XmlRecord::Enum) then we also have a mapping between the string representation (in
 *         the XML file) and our internal numeric representation.
 * Thus very little of the _code_ for handling XML is specific to a particular encoding, which should make it easy to
 * add support for new encodings (eg future versions of BeerXML).
 *
 *
 *
 *
 */


/**
 * \brief \b XmlRecord is one of a set of classes for dealing reading/writing data from/to XML documents.
 *
 *        An \b XmlCoding object is a stateless object representing a particular XML encoding, eg BeerXML 1.0.
 *        Each such an encoding needs a number of classes corresponding to different types of "records".  For the most
 *        part, a record stores a subclass of \b NamedEntity (eg Hop, Yeast, Recipe, Equipment, Style) and so, for each
 *        subclass of \b NamedEntity, there is a subclass of \b XmlNamedEntityRecord that knows *** its encoding
 *        rules (ie how to )
 *  CHOP IN THE TEXT FROM ABOVE TODO TODO
 *
 *
 * provides a base class for representing how part of an XML document maps to and from either (a) a
 *        particular NamedEntity (eg Hop, Yeast, Recipe) or (b) a list/set of a particular NamedEntity (eg multiple
 *        Hops, multiple Recipes, etc).  \b XmlRecord itself handles It has two child classes, \b XmlNamedEntityRecord and \b XmlMultipleRecord.
 *        \b XmlNamedEntityRecord
 */
class XmlRecord {
public:

   /**
    * \brief The types of fields that we know how to process.  Used in \b Field records
    */
   enum FieldType {
      Bool,
      Int,
      UInt,
      Double,
      String,
      Enum,
      Record
   };

   /**
    * \brief Map from a string in an XML file to the value of an enum in a Brewtarget class
    *
    * .:TODO:. Need to make this two-way
    *
    * Could use QMap or QHash here.  Doubt it makes much difference either way for the quantity of data /
    * number of look-ups we're doing.  (Documentation says QHash is "significantly faster" if you don't need ordering,
    * but some people say that's only true beyond a certain number of elements stored.  We could benchmark it if we
    * were anxious about performance here.)
    */
   typedef QHash<QString, int> EnumLookupMap;

   /**
    * \brief How to parse every field that we want to be able to read out of the XML file.  See class description for
    *        more details.
    */
   struct Field {
      FieldType           fieldType;
      XQString            xPath;
      char const * const  propertyName;
      EnumLookupMap const * stringToEnum;
   };

   typedef QVector<Field> FieldDefinitions;

   /**
    * \brief Constructor
    * \param xmlCoding An \b XmlCoding object representing the XML Coding we are using (eg BeerXML 1.0).  This is what
    *                  we'll need to look up how to handle nested records inside this one.
    * \param recordName The name of the XML tag containing this type of record, eg "HOP", "YEAST", etc
    * \param fieldDefinitions A list of fields we expect to find in this record (other fields will be ignored) and how
    *                         to parse them.
    */
   XmlRecord(XmlCoding const & xmlCoding,
             QString const recordName,
             FieldDefinitions const & fieldDefinitions);

   /**
    * \brief
    * \return
    */
   QString const & getRecordName() const;

   /**
    * \brief From the supplied record (ie node) in an XML document, load into memory the data it contains, including
    *        any other records nested inside it.  Although most validation should have been done by the XSD, if there
    *        are any rules that cannot be expressed in the XSD then they should be done in this method.
    *
    * TBD DOES THIS NEED TO BE VIRTUAL ANY MORE?
    *
    * \param domSupport
    * \param rootNodeOfRecord
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    * \param fieldsRead Optional pointer to an empty hashmap.  If set, this method should populate it with what fields
    *                   it read in.  (This is useful for child classes that override this method and want to do further
    *                   validation after calling the parent class method.)
    *
    * \return \b true if load succeeded, \b false if there was an error
    */
   virtual bool load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage,
                     std::shared_ptr< QHash<QString, QVariant> > fieldsRead = nullptr);

   /**
    * \brief Store the value of a field we have read in.  TBD NB the default implementation of this does nothing, as it is not required for pure container records (ie records that only contain other records).  Child
    *        classes should override this method.
    * \param fieldDefinition
    * \param parsedValue
    */
   virtual void storeField(Field const & fieldDefinition,
                           QVariant parsedValue);

   /**
    * \brief Once the record (including all its sub-records) is loaded into memory, we this function does any final
    *        validation and data correction before then storing the object(s) in the database.  Most validation should
    *        already have been done via the XSD, but there are some validation rules have to be done in code, in
    *        particular checking for duplicates and name clashes.
    *
    *        Child classes may override this function to extend functionality but should make sure to call this base
    *        class version to ensure child nodes are saved.
    *
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    * \param stats This object keeps tally of how many records (of each type) we skipped or stored
    *
    * \return \b true if processing succeeded, \b false if there was an unresolvable problem.  Note that neither
    *         skipping over duplicates nor amending names counts as an error.  The skipping or amending will be logged,
    *         but it will not prevent the funciton from returning \b true.
    */
   virtual bool normaliseAndStoreInDb(QTextStream & userMessage,
                                      XmlRecordCount & stats);

private:
   /**
    * \brief Load in child records.  It is for derived classes to determine whether and when they have child records to
    *        process (eg Hop records inside a Recipe).  But the algorithm for processing is generic, so we implement it
    *        in this base class.
    */
   bool loadChildRecords(xalanc::DOMSupport & domSupport,
                         xalanc::NodeRefList & nodesForCurrentXPath,
                         QTextStream & userMessage);

protected:
   static void modifyClashingName(QString & candidateName);

   XmlCoding const &      xmlCoding;
   QString const          recordName;
   FieldDefinitions const & fieldDefinitions;

   /**
    * Keep track of any child records
    */
   QMultiHash< QString, std::shared_ptr<XmlRecord> > childRecords;

   // See https://apache.github.io/xalan-c/api/XalanNode_8hpp_source.html for possible indexes into this array
   static char const * const XALAN_NODE_TYPES[];
};

#endif
