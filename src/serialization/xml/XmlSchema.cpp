/*======================================================================================================================
 * serialization/xml/XmlSchema.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "serialization/xml/XmlSchema.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>

#include "serialization/xml/XmlLibHelpers.h"

XmlSchema::XmlSchema(QString const & schemaResource, XmlErrorHandler & errorHandler) {

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
   CWrappers::unique_ptr<xmlSchemaParserCtxt, xmlSchemaFreeParserCtxt> schemaParserContext{
      xmlSchemaNewMemParserCtxt(schemaData.constData(), schemaData.size())
   };
   if (!schemaParserContext) {
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
   // The libxml2 API documentation is not massively helpful -- eg describing both xmlSchemaSetParserStructuredErrors()
   // and xmlSchemaSetValidStructuredErrors() as "Set the structured error callback".
   //
   // AIUI, xmlSchemaSetParserStructuredErrors(), which we call here, sets the error callback used while parsing and
   // compiling the XSD schema itself.
   //
   xmlSchemaSetParserStructuredErrors(schemaParserContext.get(),
                                      &XmlErrorHandler::xmlStructuredErrorFunc,
                                      static_cast<void *>(&errorHandler));

   //
   // Calling xmlSchemaParse will "parse a schema definition resource and build an internal XML Schema structure
   // [viz xmlSchema] which can be used to validate instances".
   //
   this->m_schema.reset(xmlSchemaParse(schemaParserContext.get()));
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
      // member variables are destroyed, so the magic of CWrappers::unique_ptr (standing on the shoulders of
      // std::unique_ptr) means xmlSchemaFree is automatically called here.
      throw std::runtime_error("Unable to create validation context.");
   }

   qDebug() << Q_FUNC_INFO << "Schema " << schemaFile.fileName() << " loaded OK.";

   return;
}

XmlSchema::~XmlSchema() = default;

bool XmlSchema::validate(XmlDocument const & xmlDocument,
                         XmlErrorHandler & errorHandler,
                         QTextStream & userMessage) const {


   //
   // In contrast to xmlSchemaSetParserStructuredErrors(), which we call in our constructor, this call to
   // xmlSchemaSetValidStructuredErrors() sets the error callback used while validating an XML document against an
   // already-compiled schema.
   //
   xmlSchemaSetValidStructuredErrors(this->m_schemaValidationContext.get(),
                                     &XmlErrorHandler::xmlStructuredErrorFunc,
                                     static_cast<void *>(&errorHandler));

   // Return code is "0 if the document is schemas valid [sic], a positive error code number otherwise and -1 in case of
   // internal or API error".
   int const returnCode = xmlSchemaValidateDoc(this->m_schemaValidationContext.get(), xmlDocument.get());
   if (returnCode < 0) {
      qCritical() << Q_FUNC_INFO << "Internal or API error";
      throw std::runtime_error("Internal or API error -- see log file for more details");
      return false;
   }

   // Per comments in XmlCoding::impl::validateLoadAndStoreInDb, some errors are safe to ignore
   if (returnCode > 0) {
      if (errorHandler.failed()) {
         userMessage << "Invalid file -- see log file for more details";
         return false;
      }
   }
   return true;
}