/*======================================================================================================================
 * serialization/xml/XmlLibHelpers.cpp is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#include "serialization/xml/XmlLibHelpers.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTextStream>

#include "XmlErrorHandler.h"

namespace {
   /**
    * RAII wrapper for libxml2's xmlSchemaParserCtxt
    */
   class SchemaParserContext {
   public:

      explicit SchemaParserContext(QByteArray const & schemaData, XmlErrorHandler & errorHandler) :
         m_schemaParserContext{xmlSchemaNewMemParserCtxt(schemaData.constData(), schemaData.size())} {
         if (!this->m_schemaParserContext) {
            //
            // Null return tells us there was an error, but nothing more than that.  Fortunately, most of what
            // xmlSchemaNewMemParserCtxt is doing is just allocating memory etc, so we're not expecting to need much in
            // the way of diagnostics.  (Would most likely fail if we're running out of memory or supplied a null buffer
            // or something.)
            //
            qCritical() << Q_FUNC_INFO << "Unable to allocate resources to parse schema";
            throw std::runtime_error("Error initialising schema parser -- see log file for (slightly) more details");
         }

         //
         // Now that we have a schema parser context, we can set a callback function on it for libxml2 to pass errors
         // back to us (instead of writing them to stderr).
         //
         xmlSchemaSetParserStructuredErrors(this->m_schemaParserContext.get(),
                                            &XmlErrorHandler::xmlStructuredErrorFunc,
                                            static_cast<void *>(&errorHandler));
         return;
      }

      // unique_c_ptr handles all the clean-up for us
      ~SchemaParserContext() = default;

      //! Wraps xmlSchemaParse().  Caller owns returned resource.
      xmlSchema * parseSchema() const {
         //
         // Calling xmlSchemaParse will "parse a schema definition resource and build an internal XML Schema structure
         // [viz xmlSchema] which can be used to validate instances".
         //
         return xmlSchemaParse(this->m_schemaParserContext.get());
      }

   private:
      //=============================================== Member Variables ===============================================
      unique_c_ptr<xmlSchemaParserCtxt, xmlSchemaFreeParserCtxt> m_schemaParserContext = nullptr;
   };

   char const * elementTypeToStringRaw(xmlElementType const elementType) {
      switch (elementType) {
         // See https://gnome.pages.gitlab.gnome.org/libxml2/html/tree_8h.html#a6d83aa11b89106b2ec14162b1474c8fc for
         // more details on these.
         //
         // Ignore compiler warnings about XML_DOCB_DOCUMENT_NODE with older versions of libxml2.  It is removed in more
         // recent versions of the library.
         case XML_ELEMENT_NODE      : return "XML_ELEMENT_NODE"       ": An element"                ;
         case XML_ATTRIBUTE_NODE    : return "XML_ATTRIBUTE_NODE"     ": An attribute"              ;
         case XML_TEXT_NODE         : return "XML_TEXT_NODE"          ": A text node"               ;
         case XML_CDATA_SECTION_NODE: return "XML_CDATA_SECTION_NODE" ": A CDATA section"           ;
         case XML_ENTITY_REF_NODE   : return "XML_ENTITY_REF_NODE"    ": An entity reference"       ;
         case XML_ENTITY_NODE       : return "XML_ENTITY_NODE"        ": Unused"                    ;
         case XML_PI_NODE           : return "XML_PI_NODE"            ": A processing instruction"  ;
         case XML_COMMENT_NODE      : return "XML_COMMENT_NODE"       ": A comment"                 ;
         case XML_DOCUMENT_NODE     : return "XML_DOCUMENT_NODE"      ": A document"                ;
         case XML_DOCUMENT_TYPE_NODE: return "XML_DOCUMENT_TYPE_NODE" ": Unused"                    ;
         case XML_DOCUMENT_FRAG_NODE: return "XML_DOCUMENT_FRAG_NODE" ": A document fragment"       ;
         case XML_NOTATION_NODE     : return "XML_NOTATION_NODE"      ": A notation, unused"        ;
         case XML_HTML_DOCUMENT_NODE: return "XML_HTML_DOCUMENT_NODE" ": An HTML document"          ;
         case XML_DTD_NODE          : return "XML_DTD_NODE"           ": A document type definition";
         case XML_ELEMENT_DECL      : return "XML_ELEMENT_DECL"       ": An element declaration"    ;
         case XML_ATTRIBUTE_DECL    : return "XML_ATTRIBUTE_DECL"     ": An attribute declaration"  ;
         case XML_ENTITY_DECL       : return "XML_ENTITY_DECL"        ": An entity declaration"     ;
         case XML_NAMESPACE_DECL    : return "XML_NAMESPACE_DECL"     ": An XPath namespace node"   ;
         case XML_XINCLUDE_START    : return "XML_XINCLUDE_START"     ": An XInclude start marker"  ;
         case XML_XINCLUDE_END      : return "XML_XINCLUDE_END"       ": An XInclude end marker"    ;
      }
      return "Unrecognised!";
   }
}

QString XmlLibHelpers::elementTypeToString(xmlElementType const elementType) {
   return QString{"%1 = %2"}.arg(elementType).arg(elementTypeToStringRaw(elementType));
}

XmlLibHelpers::XmlSchema::XmlSchema(QString const & schemaResource, XmlErrorHandler & errorHandler) {
   QFile schemaFile(schemaResource);
   if (!schemaFile.open(QIODevice::ReadOnly)) {
      // This should pretty much never happen, as we're loading from a QResource compiled into the binary rather
      // than reading from the file system at run-time.
      qCritical() <<
         Q_FUNC_INFO << "Could not open schema file resource " << schemaFile.fileName() << " for reading";
      throw std::runtime_error("Could not open schema file resource");
   }

   QByteArray const schemaData = schemaFile.readAll();
   qDebug() <<
      Q_FUNC_INFO << "Schema file " << schemaFile.fileName() << ": " << schemaData.length() << " bytes";

   // This is only needed for the xmlSchemaParse() call.  Once the schema is parsed into this->m_xmlSchema, it can be
   // discarded.
   SchemaParserContext const schemaParserContext(schemaData, errorHandler);
   this->m_schema.reset(schemaParserContext.parseSchema());
   if (!this->m_schema) {
      // XmlLibHelpers::xmlStructuredErrorFunc should have logged all the specifics
      qCritical() << Q_FUNC_INFO << "Unable to parse schema" << schemaFile.fileName();
      throw std::runtime_error("Error parsing schema parser -- see log file for more details");
   }

   //
   // Since we're only ever reading one XML file at a time, it's fine to have a single validator for the schema
   //
   this->m_schemaValidationContext.reset(xmlSchemaNewValidCtxt(this->m_schema.get()));
   if (!this->m_schemaValidationContext) {
      // Our destructor won't get called if we throw an exception in the constructor.  However, any already-constructed
      // member variables are destroyed, so the magic of unique_c_ptr (standing on the shoulders of std::unique_ptr)
      // means xmlSchemaFree is automatically called here.
      throw std::runtime_error("Unable to create validation context.");
   }

   //
   // And we want to log any errors the validator encounters (rather than have them written to stderr)
   //
   xmlSchemaSetValidStructuredErrors(this->m_schemaValidationContext.get(),
                                     &XmlErrorHandler::xmlStructuredErrorFunc,
                                     static_cast<void *>(&errorHandler));

   qDebug() << Q_FUNC_INFO << "Schema " << schemaFile.fileName() << " loaded OK.";

   return;
}

XmlLibHelpers::XmlSchema::~XmlSchema() = default;

bool XmlLibHelpers::XmlSchema::validate(XmlLibHelpers::XmlDocument const & xmlDocument,
                                        QTextStream & userMessage) const {
   // Return code is "0 if the document is schemas valid, a positive error code number otherwise and -1 in case of
   // internal or API error".
   int returnCode = xmlSchemaValidateDoc(this->m_schemaValidationContext.get(), xmlDocument.get());
   if (returnCode < 0) {
      qCritical() << Q_FUNC_INFO << "Internal or API error";
      throw std::runtime_error("Internal or API error -- see log file for more details");
   }
   if (returnCode > 0) {
      userMessage << "Invalid file -- see log file for more details";
   }
   return 0 == returnCode;
}

XmlLibHelpers::XmlDocument::XmlDocument(QByteArray const & documentData,
                                        QString const & fileName) :
   m_fileName{fileName} {
   //
   // For the final parameter is a combination of xmlParserOption flags:
   //    XML_PARSE_NONET     = Disable network access with the built-in HTTP or FTP clients.  Serves no purpose after 2.15.0
   //    XML_PARSE_NO_XXE    = Disables loading of external DTDs or entities. NB: Can't use until we're on 2.13.0 or later
   //    XML_PARSE_BIG_LINES = Enable reporting of line numbers larger than 65535.
   //
   this->m_document.reset(
      xmlReadMemory(
         documentData.constData(),
         documentData.length(),
         nullptr,
         nullptr,
         XML_PARSE_NONET | /*XML_PARSE_NO_XXE |*/ XML_PARSE_BIG_LINES
      )
   );
   if (!this->m_document) {
      qWarning() << Q_FUNC_INFO << "Unable to read XML document" << fileName;
      throw std::runtime_error("Error reading XML document -- see log file for more details");
   }

   this->m_context.reset(xmlXPathNewContext(this->m_document.get()));
   if (!this->m_context) {
      // We're not expecting this to happen, but it doesn't hurt to be able to handle it
      qCritical() << Q_FUNC_INFO << "Unable to create context struct for XML document" << fileName;
      throw std::runtime_error("Context creation error while reading XML document -- see log file for more details");
   }
   return;
}

XmlLibHelpers::XmlDocument::~XmlDocument() = default;

xmlDoc * XmlLibHelpers::XmlDocument::get() const {
   return this->m_document.get();
}

XmlLibHelpers::XPathResult const XmlLibHelpers::XmlDocument::xPathResult(xmlNode & node, QString const & xPath) {
   this->m_context->node = &node;
   QByteArray const rawPath = xPath.toUtf8();
   return XmlLibHelpers::XPathResult{xmlXPathEvalExpression(asXmlCharString(rawPath), this->m_context.get())};
}

XmlLibHelpers::XPathResult::XPathResult(xmlXPathObject * xPathObject) :
   m_xPathObject{xPathObject} {
   return;
}

XmlLibHelpers::XPathResult::~XPathResult() = default;

std::size_t XmlLibHelpers::XPathResult::numNodes() const {
   return this->m_xPathObject->nodesetval ? this->m_xPathObject->nodesetval->nodeNr : 0;
}

xmlNode * XmlLibHelpers::XPathResult::node(int index) const {
   if (index < 0) {
      // This is a bit glib, but if you think about it, it's correct.  Index of -1 gives the last node, of -2 gives the
      // penultimate one, etc.
      index += this->numNodes();
   }
   Q_ASSERT(index >= 0 && static_cast<std::size_t>(index) < this->numNodes());
   return this->m_xPathObject->nodesetval->nodeTab[index];
}

QString XmlLibHelpers::toQString(xmlChar const * xmlCharString) {
   return QString::fromUtf8(reinterpret_cast<char const *>(xmlCharString));
}

xmlChar const * XmlLibHelpers::asXmlCharString(QByteArray const & qByteArray) {
   return reinterpret_cast<xmlChar const *>(qByteArray.constData());
}

std::vector<xmlNode *> XmlLibHelpers::childNodes(xmlNode const * node) {
   std::vector<xmlNode *> result;
   for (xmlNode * child = node->children; child != nullptr; child = child->next) {
      result.push_back(child);
   }
   return result;
}
