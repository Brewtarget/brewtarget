/*
 * xml/XmlCoding.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_XMLCODING_H
#define _XML_XMLCODING_H
#pragma once

#include <memory> // For smart pointers
#include <QHash>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QVariant>


#include <xalanc/DOMSupport/DOMSupport.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>

#include "xml/XmlRecord.h"

/**
 * \brief An instance of this class holds information about a particular XML encoding (eg BeerXML 1.0)
 *
 * See comment in xml/XmlRecord.h for how classes interact
 */
class XmlCoding {
public:
   /**
    * \brief C++ does not permit you to have a function pointer to a class constructor, so this templated wrapper
    *        function is a "trick" that allows us to come close enough for our purposes.  Using a pointer to a
    *        template specialisation of this function for some subclass of XmlRecord effectively gives us a pointer to
    *        a create-on-the-heap constructor for that subclass, (provided it takes the same parameters as this
    *        function).
    *
    *        To make it easier for callers, we also typedef \b XmlCoding::XmlRecordConstructorWrapperto be a pointer to
    *        a function of this type.
    *
    * \param xmlCoding passed into the constructor of T (which should be \b XmlRecord or a subclass thereof)
    * \param entityName passed into the constructor of T (which should be \b XmlRecord or a subclass thereof)
    * \param fieldDefinitions passed into the constructor of T (which should be \b XmlRecord or a subclass thereof)
    * \return A new T constructed on the heap.  The caller owns this object and is responsible for its deletion.
    */
   template<typename T>
   static XmlRecord * construct(XmlCoding const & xmlCoding,
                                QString const & entityName,
                                XmlRecord::FieldDefinitions const & fieldDefinitions) {
      return new T{xmlCoding, entityName, fieldDefinitions};
   }

   /**
    * \brief This is just a convenience typedef representing a pointer to a template specialisation of
    *        \b XmlCoding::construct().
    */
   typedef XmlRecord * (*XmlRecordConstructorWrapper)(XmlCoding const &,
                                                      QString const &,
                                                      XmlRecord::FieldDefinitions const &);

   /**
    * Given an XML element that corresponds to a record, this is the info we need to construct an \b XmlRecord object
    * for this encoding.
    */
   struct XmlRecordDefinition {
      XmlRecordConstructorWrapper constructorWrapper;
      XmlRecord::FieldDefinitions const * fieldDefinitions;
   };

   /**
    * \brief Constructor
    * \param name The name of this encoding (eg "BeerXML 1.0").  Used primarily for logging.
    * \param entityNameToXmlRecordDefinition Mapping from XML tag name to the information we need to construct a suitable \b XmlRecord object.
    *                                        This is expected to be a static object, hence the pass-by-reference.
    */
   XmlCoding(QString const name,
             QHash<QString, XmlRecordDefinition> const & entityNameToXmlRecordDefinition);

   /**
    * \brief Check whether we know how to process a record of a given (XML tag) name
    * \param recordName
    * \return \b true if we know how to process (ie we have the address of a function that can create a suitable
    *         \b XmlRecord object), \b false if not
    */
   bool isKnownXmlRecordType(QString recordName) const;

   /**
    * \brief For a given record name (eg "HOPS", "HOP", "YEASTS", etc) retrieve a new instance of the corresponding
    *        subclass of \b XmlRecord.  Caller is responsible for ensuring that such a subclass exists, either by
    *        having supplied the \b nameToXmlRecordLookup to our constructor or by calling \b isKnownXmlRecordType().
    * \param recordName
    * \return A shared pointer to a new \b XmlRecord constructed on the heap.  (The caller will be the sole owner of
    *         this pointer.)
    */
   std::shared_ptr<XmlRecord> getNewXmlRecord(QString recordName) const;

   /**
    * \brief
    * \param domSupport
    * \param rootNode root node of document
    * \param userMessage
    * \return
    */
   bool loadNormaliseAndStoreInDb(xalanc::DOMSupport & domSupport,
                                  xalanc::XalanNode * rootNode,
                                  QTextStream & userMessage) const;

private:
   QString name;
   QHash<QString, XmlRecordDefinition> const entityNameToXmlRecordDefinition;
};

#endif
