/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlCoding.h is part of Brewtarget, and is copyright the following authors 2020-2022:
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
#ifndef SERIALIZATION_XML_XMLCODING_H
#define SERIALIZATION_XML_XMLCODING_H
#pragma once

#include <memory> // For smart pointers
#include <QHash>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>

#include "serialization/xml/BtDomErrorHandler.h"
#include "serialization/xml/XmlRecord.h"
#include "serialization/xml/XmlNamedEntityRecord.h"
#include "serialization/xml/XmlMashRecord.h"
#include "serialization/xml/XmlRecipeRecord.h"

/**
 * \brief An instance of this class holds information about a particular XML encoding (eg BeerXML 1.0) including the
 *        parameters needed to construct the various \b XmlRecord objects used to parse a document of this encoding.
 *
 * Broadly speaking we think of a "brewing" XML document (eg a BeerXML document) as a tree of "records" represented by
 * \b XmlRecord and its child classes.  (Note that this is a simpler, and subtly different, way of abstracting things
 * than the traditional BeerXML terminology, which distinguishes between "records" and "record sets".)
 *
 * In our model, the contents of a record usually correspond to an instance of some subclass of \b NamedEntity (ie a
 * Hop, a Yeast, a Recipe, an Equipment, a Style, etc).  So, a record can contain other records when this makes sense.
 * Eg a recipe record will usually contain a style record, one ore more hop records, and so on.  Similarly, a mash
 * record will contain a number of mash step records.  However, there is one special circumstance, the root record of
 * the document, which does not correspond to a \b NamedEntity and is _just_ a container for other records.
 *
 * Most of the generic functionality for a record is implemented in the \b XmlRecord class (which includes handling the
 * special case of the root record).  Some code is class-specific (eg creating a Hop object to read a hop record into)
 * but a lot of this can be handled by the templated class \b XmlNamedEntityRecord which extends \b XmlRecord:
 *    \b XmlNamedEntityRecord<Equipment> suffices to read Equipment records
 *    \b XmlNamedEntityRecord<Fermentable> suffices to read Fermentable records
 *    \b XmlNamedEntityRecord<Hop> suffices to read Hop records
 *    \b XmlNamedEntityRecord<Mash> suffices to read Misc records
 *    \b XmlNamedEntityRecord<Misc> suffices to read Misc records
 *    \b XmlNamedEntityRecord<Style> suffices to read Style records
 *    \b XmlNamedEntityRecord<Water> suffices to read Water records
 *    \b XmlNamedEntityRecord<Yeast> suffices to read Yeast records
 * For a couple of other cases, this needs to be extended further:
 *    \b XmlRecipeRecord : public XmlNamedEntityRecord<Recipe> - handles fact that \b Recipe contains lots
 *                                                               of other things
 *
 * Each \b XmlRecord object has one or more fields.   Each field is either another \b XmlRecord or a simple value (ie a
 * string, some sort of number, a boolean or an enum).  There may be further constraints on these fields, eg a number
 * that is only supposed to be between 0 and 100, but we mostly do not need to worry about them here.  Data that we are
 * exporting to an XML document is assumed to be valid already.  And data we are importing from an XML document is
 * validated against an XML Schema Document (XSD).  NB: See comment in xml/XmlCoding.cpp for why XSDs steer us towards
 * using Xerces and Xalan for XML processing.
 *
 * In a given type of record, each field has a type (\b XmlRecordDefinition::FieldType) and an XPath within that containing
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
 * (In BeerXML, there is a simple correlation between the "record set" tag (eg HOPS) and the "record" tag (eg HOP) but
 * this is only because BeerXML uses "MASHS" as a mangled plural of "MASH" instead of "MASHES".  Future versions of
 * BeerXML (if there ever are any) or other XML-based encodings might be different, so we don't want to bake in this
 * assumption.)
 *
 * For a given XML encoding (eg BeerXML 1.0) we have various bits of mapping data stored in an instance of \b XmlCoding.
 * These suffice for it to know, for any XML element in this encoding that holds a record (eg <HOP>...</HOP>):
 *    • Which class handles that type of record (eg \b XmlNamedEntityRecord<Hop>).  (Actually what we need to know is
 *      how to construct an instance of the class that handles this type of record.  So instead of storing a class name,
 *      we store a pointer to a function that can invoke the constructor on the class we want.)
 *    • For this class, what mappings are needed to construct an instance of it that works with this encoding.
 *      Specifically this is a list of field definitions, each being an \b XmlRecord::Field that holds.
 *       ‣ \b fieldType : Field type (encodes as \b XmlRecordDefinition::FieldType enum)
 *       ‣ \b xPath : The XPath within this record of this field
 *       ‣ \b propertyName : Where to store the field (via the name of the name of the Q_PROPERTY of the subclass of
 *                           \b NamedEntity that this type of record corresponds to).  This can be null for fields that
 *                           either will not be stored or will be handled by special processing.
 *                           (Strictly speaking we don't need field definitions for fields we don't want to store, but
 *                           the "no store" definitions might be useful in future.)
 *       ‣ \b stringToEnum : If the field type is an enum (XmlRecord::Enum) then we also have a mapping between the
 *         string representation (in the XML file) and our internal numeric representation.
 * Thus very little of the _code_ for handling XML is specific to a particular encoding, which should make it easy to
 * add support for new encodings (eg future versions of BeerXML).
 */
class XmlCoding {
   // Per https://doc.qt.io/qt-5/i18n-source-translation.html#translating-non-qt-classes, this gives us a tr() function
   // without having to inherit from QObject.
   Q_DECLARE_TR_FUNCTIONS(XmlCoding)

public:

   /**
    * \brief Constructor
    * \param name The name of this encoding (eg "BeerXML 1.0").  Used primarily for logging.
    * \param schemaResource The name of the Qt Resource holding the XML Schema Document (XSD) for this coding
    * \param rootRecordDefinition
    */
   XmlCoding(QString const name,
             QString const schemaResource,
             XmlRecordDefinition const & rootRecordDefinition);

   /**
    * \brief Destructor
    */
   ~XmlCoding();

   /**
    * \brief Get the root definition element, ie what we use to start processing a document
    */
   XmlRecordDefinition const & getRoot() const;

   /**
    * \brief Validate XML file against schema, load its contents into objects, and store then in the DB
    *
    * \param documentData The contents of the XML file, which the caller should already have loaded into memory
    * \param fileName Used only for logging / error message
    * \param domErrorHandler The rules for handling any errors encountered in the file - in particular which errors
    *                        should ignored and whether any adjustment needs to be made to the line numbers where
    *                        errors are found when creating user-readable messages.  (This latter is needed because in
    *                        some encodings, eg BeerXML, we need to modify the in-memory copy of the XML file before
    *                        parsing it.  See comments in the BeerXML-specific files for more details.)
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validateLoadAndStoreInDb(QByteArray const & documentData,
                                 QString const & fileName,
                                 BtDomErrorHandler & domErrorHandler,
                                 QTextStream & userMessage) const;

private:

   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
