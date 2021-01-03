/*
 * xml/XPathRecordLoader.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XPATHRECORDLOADER_H
#define _XML_XPATHRECORDLOADER_H
#pragma once

#include <memory> // For smart pointers

#include <QHash>
#include <QMetaType>
#include <QString>
#include <QVariant>
#include <QVector>

#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>

#include "model/NamedEntity.h"
#include "xml/XQString.h"

class XPathRecordLoader;

/**
 * C++ does not permit you to have a function pointer to a class constructor, so this is a "trick" that allows us to
 * come close enough for our purposes.
 */
template<typename T>
XPathRecordLoader * XPathRecordLoaderFactory () {
   return new T();
}

/**
 * This is a base class for parsing individual "object" records (ie corresponding to Hop, Yeast, Equipment, Recipe etc)
 * from an XML document.  An object of this base class (initialised with a suitable mapping) can read fields in to a
 * hash map and simultaneously populate scalar attributes of an object.  Child classes (such as BeerXmlHopRecordLoader)
 * extend this functionality with any processing specific to the type of object being read in.
 *
 * Eg say you have the following section in an XML document (amended and simplified from real BeerXML):
 *    <HOPS>
 *       <HOP>
 *          <NAME>Fuggle</NAME>
 *          <ALPHA>4.2</ALPHA>
 *          <WEIGHT>
 *             <UNIT_OF_MEASUREMENT>g<UNIT_OF_MEASUREMENT>
 *             <VALUE>100</VALUE>
 *          </WEIGHT>
 *          <TYPE>Aroma</TYPE>
 *       </HOP>
 *       <HOP>
 *          <NAME>East Kent Golding</NAME>
 *          <ALPHA>3.7</ALPHA>
 *          <WEIGHT>
 *             <UNIT_OF_MEASUREMENT>g<UNIT_OF_MEASUREMENT>
 *             <VALUE>50</VALUE>
 *          </WEIGHT>
 *          <TYPE>Both</TYPE>
 *       </HOP>
 *    </HOPS>
 * As we process the XML document, at each of the <HOP> tags, we can use a BeerXmlHopRecordLoader object (which
 * inherits from XPathRecordLoader) to parse the contents (ie everything inside the current <HOP>...</HOP> pair) into a
 * new Hop object.  If that parsing succeeds then the object can be saved (or, as the case may be, passed back to code
 * in BeerXmlRecipeRecordLoader that is processing a containing <RECIPE>...</RECIPE> tag pair).
 *
 * For every field in the XML that we want to be able to process, we need an \b XPathRecordLoader::Field entry which
 * has:
 *
 *    \b fieldType    The base data type we want to read the XML field into (QString, double, int, etc).  Though NB for
 *                    something that we'll end up storing as an enum, we first read it into a QString
 *
 *    \b xPath        The relative XPath of the tag in the XML inside the record we're looking at.  Usually this is
 *                    just the tag name (eg "NAME", "ALPHA", "TYPE" in our example above) but nested tags are supported
 *                    too(eg "WEIGHT/UNIT_OF_MEASUREMENT" and "WEIGHT/VALUE" in our example above).
 *
 *                    Note, however, that where there is a complex contained item, we'll usually want to process that
 *                    separately with its own XPathRecordLoader.  Eg, inside a RECIPE, we'll want separate
 *                    XPathRecordLoaders to process HOPS, MISCS, STYLE, etc.
 *
 *    \b propertyName If set, this is the name of the Q_PROPERTY on the object we're constructing from this XML record.
   *                  Via the magic of the Qt Property System, we'll then be able to pass the field value in to the
 *                    object via its setter without having to deal with a lot of pointers to functions etc.
 *
 *                    If not set, then we'll read the property from the file and make it available to the caller via
 *                    getField(), but won't try to store it in the object.
 *
 *    \b stringToEnum This is an optional parameter that maps a string to an enum value.  Eg for the USE field of a HOP
 *                    record, we want to map "TYPE" string field to a Hop::Type enum value.
 *
 */
class XPathRecordLoader {
public:
   ~XPathRecordLoader() = default;
   QString const & getRecordName() const;

   /**
    * Used in conjunction with \b XPathRecordLoaderFactory to allow caller's to map from a record set name to the
    * address of a function that will create the appropriate subclass of \b XPathRecordLoader
    */
   typedef XPathRecordLoader * (*Factory)();


   // Could use QMap or QHash here.  Doubt it makes much difference either way for the quantity of data /
   // number of look-ups we're doing.  (Documentation says QHash is "significantly faster" if you don't need ordering,
   // but some people say that's only true beyond a certain number of elements stored.  We could benchmark it if we
   // were anxious about performance here.)
   typedef QHash<QString, int> EnumLookupMap;

   /**
    * The types of fields that we know how to process
    */
   enum FieldType {
      Bool,
      Int,
      UInt,
      Double,
      String,
      Enum
   };

   /**
    * \brief How to parse every field that we want to be able to read out of the XML file.  See class description for
    *        more details.
    */
   struct Field {
      XPathRecordLoader::FieldType             fieldType;
      XQString                                 xPath;
      char const * const                       propertyName;
      XPathRecordLoader::EnumLookupMap const * stringToEnum;
   };

protected:

   /**
    * \brief Constructor for derived classes to call
    * \param recordName Name of the type of XML record we are parsing (eg "HOP").
    * \param fieldDefinitions List of field infos for this record type (see class description for more details).
    * \param entityToPopulate
    */
   XPathRecordLoader(QString const recordName,
                     QVector<Field> const & fieldDefinitions,
                     NamedEntity * entityToPopulate);

public:
   /**
    * \brief Load a data record in from given the sub-section of the XML document.   Child classes should extend this
    * member function for any class-specific load logic.
    *
    * \param domSupport
    * \param rootNodeOfRecord
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    *
    * \return \b true if load succeeded, \b false if there was an error
    */
   virtual bool load(xalanc::DOMSupport & domSupport,
                     xalanc::XalanNode * rootNodeOfRecord,
                     QTextStream & userMessage);

   /**
    * \brief Having loaded in the entity, this function will attempt to ensure it is usable.  If there are problems
    * with the data that can be easily and safely fixed, it will do so.  If there are more fundamental problems, eg the
    * entity read in contains some nonsensical combination of field values that could not be validated at the XML level,
    * this will be reported as an error.  Child classes should extend this member function for any class-specific
    * normalisation and verification.
    *
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    *
    * \return \b true if everything went well succeeded, \b false if there was an error
    */
   virtual bool normalise(QTextStream & userMessage);

   /**
    * \brief Once the entity is loaded and normalised, this function will check whether it is a duplicate of an
    * existing entity and, if not, store it in the database.  Child classes should extend this member function for any
    * class-specific logic.
    *
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    *
    * \return \b true if entity was stored, \b false if we detected a duplicate or there was another problem
    */
   virtual bool storeInDb(QTextStream & userMessage);


protected:
   QString const            recordName; // Eg "HOP", "YEAST", etc
   QVector<Field> const &   fieldDefinitions;

   // It's the job of each subclass to create a suitable object (Hop, Yeast, etc)
   // Once the object is populated, if we give ownership to something else (eg Database class), we should call
   // release(), otherwise, entityToPopulate will get destroyed when the XPathRecordLoader containing it goes out of
   // scope.
   std::unique_ptr<NamedEntity> entityToPopulate;
   QHash<QString, QVariant> fieldsRead; // Keeps track of all the fields we read in

   // See https://apache.github.io/xalan-c/api/XalanNode_8hpp_source.html for possible indexes into this array
   static char const * const XALAN_NODE_TYPES[];

};

#endif
