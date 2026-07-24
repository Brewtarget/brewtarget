/*======================================================================================================================
 * serialization/xml/XmlLibHelpers.h is part of Brewtarget, and is copyright the following authors 2020-2026:
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
 =====================================================================================================================*/
#ifndef SERIALIZATION_XML_XMLLIBHELPERS_H
#define SERIALIZATION_XML_XMLLIBHELPERS_H
#pragma once

#include <memory>

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xmlerror.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xmlschemas.h>

#include <QByteArray>
#include <QString>
#include <QTextStream>

#include "serialization/xml/XmlErrorHandler.h"

/**
 * Because std::unique_ptr can take a custom deleter, it can handle resource ownership even for libxml2 entities, even
 * though each of these is a C struct with its own custom "free" function to release the resource.
 *
 * By default, if you have a custom deleter type for std::unique_ptr, you need to specify the instance of it when you
 * initialise or assign to the unique_ptr.  In our case, each type always has a single global function to release
 * resources.  So, rather than keep repeating this, we create a default-constructable deleter functor that calls the
 * relevant global function.
 */
template<auto freeFunctionForT>
struct freeFunctionCaller {
   //
   // This struct has no member variables, so its default constructor and destructor are trivial.  (The constructor also
   // won't throw any exception, which is another requirement for using with std::unique_ptr.)  We just need to
   // provide the "functor" bit.  Although there is only one type to which it can apply, having this bit itself
   // templated simplifies the calling code -- and the compiler will tell us if we got it wrong because we'll be trying
   // to pass the wront pointer type to freeFunctionForT.
   //
   template<typename T>
   void operator()(T * pointer) const noexcept {
      if (pointer) {
         freeFunctionForT(pointer);
      }
      return;
   }
};
template<typename T, auto freeFunctionForT>
using unique_c_ptr = std::unique_ptr<T, freeFunctionCaller<freeFunctionForT>>;

namespace XmlLibHelpers {

   QString elementTypeToString(xmlElementType const elementType);

   class XmlDocument;

   //! RAII wrapper around libxml2's xmlSchema / xmlSchemaValidCtxt
   class XmlSchema {
   public:
      explicit XmlSchema(QString const & schemaResource, XmlErrorHandler & errorHandler);
      ~XmlSchema();

      bool validate(XmlDocument const & xmlDocument,
                    QTextStream & userMessage) const;

   private:
      //=============================================== Member Variables ===============================================
      //
      // Resource management is handled automatically via unique_c_pointer.  However, we have to declare things in the
      // right order here.  Member variables are destroyed in the reverse order of their declaration, and we want
      // xmlSchemaFreeValidCtxt() called before xmlSchemaFree().
      //
      unique_c_ptr<xmlSchema         , xmlSchemaFree         > m_schema                  = nullptr;
      unique_c_ptr<xmlSchemaValidCtxt, xmlSchemaFreeValidCtxt> m_schemaValidationContext = nullptr;
   };

   class XPathResult;

   //! RAII wrapper around libxml2's xmlDoc
   class XmlDocument {
   public:

      explicit XmlDocument(QByteArray const & documentData,
                           QString const & fileName);
      ~XmlDocument();

      xmlDoc * get() const;

      XPathResult const xPathResult(xmlNode & node, QString const & xPath);

   private:
      //=============================================== Member Variables ===============================================
      //
      // Yes, it is mildly annoying that libxml2 free function naming is not consistent (eg xmlSchemaFree for xmlSchema
      // but xmlFreeDoc for xmlDoc).  AIUI this is just because the library evolved over the course of time with
      // multiple contributors taking slightly different approaches.
      //
      QString m_fileName = "";
      unique_c_ptr<xmlDoc, xmlFreeDoc> m_document = nullptr;

   public:
      //! NB: Putting this here assumes we are only reading one XML document at a time.
      unique_c_ptr<xmlXPathContext, xmlXPathFreeContext> m_context = nullptr;
   };

   /**
    * Wrapper around xmlXPathObject
    *
    * NOTE that, unlike some of our other XML classes, this is NOT an owning wrapper.  The xmlXPathObject that we point
    * to is part of data owned by an xmlDoc struct, so there's nothing for us to free when we go out of scope.
    */
   class XPathResult {
   public:
      explicit XPathResult(xmlXPathObject * xPathObject);
      ~XPathResult();

      std::size_t numNodes() const;

      /**
       * Return the specified node in the set.  index >= 0 counts from start of list; index < 0 counts from end of list
       *
       * Caller's responsibility to establish that numNodes > 0 and |index| < numNodes before calling this
       */
      xmlNode * node(int index) const;

   private:
      xmlXPathObject const * m_xPathObject;
   };

   /**
    *
    * @return
    */
   QString toQString(xmlChar const * xmlCharString);

   /**
    * We don't directly go from QString to xmlChar const * because QString::toUtf8() returns a new QByteArray, so we'd
    * risk that going out of scope, eg if we wrote `xmlChar const * utf8String = someConverter(myQString);`
    *
    * Forcing the caller to explicitly handle the QByteArray hopefully makes it less likely we hit such an error.
    */
   xmlChar const * asXmlCharString(QByteArray const & qByteArray);

   /**
    * Although we can access child nodes directly through libxml2's xmlNode strut, the native storage is a doubly-linked
    * list (via xmlNode::next and xmlNode::prev).  This function gives us a vector of pointers to child nodes.
    *
    * @param node
    * @return
    */
   std::vector<xmlNode *> childNodes(xmlNode const * node);

}

#endif
