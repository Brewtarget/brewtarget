/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlCoding.cpp is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#include "serialization/xml/XmlCoding.h"

#include <QDebug>
#include <QFile>

#include <libxml2/libxml/xmlschemas.h>
#include <libxml2/libxml/tree.h>

#include "serialization/xml/XmlErrorHandler.h"
#include "serialization/xml/XmlLibHelpers.h"
#include "utils/ImportRecordCount.h"

//
//                              ***************************************************
//                              * General note about XML libraries and frameworks *
//                              ***************************************************
//
// Frustratingly, although Qt has support for XML parsing, it would not be wise to use it for dealing with XML Schemas.
// In mid-2019, in release 5.13, Qt deprecated its "XML Patterns" package which included QXmlSchema etc, and the
// package was removed from Qt in the Qt6.0 release of December 2020.  It's not entirely clear why these features are
// being dropped, though, it's conceivable it may be related to the fact that some of the other Qt XML classes are not
// standards-compliant (see https://www.qt.io/blog/parsing-xml-with-qt-updates-for-qt-6) and Qt have decided to offer a
// slimmed-down but standards-compliant support for XML via QXmlStreamReader and QXmlStreamWriter.
//
// For those who want to manipulate XML schemas (or, for that matter, use standards-compliant DOM or SAX APIs for
// accessing XML documents), the official advice from Qt's developers seems simply to be (according to
// https://forum.qt.io/topic/102834/proper-successor-for-qxmlschemavalidator/6) to use another library.
//
// Part of the problem with choosing and using an XML library is that XML itself is big, does a lot of things and has a
// lot of history.  Few, if any, libraries support the entirety of XML.  For example, per
// https://xerces.apache.org/xerces-c/api-3.html, Apache Xerces-C++ implements the following XML specifications:
//    • Xerces-C++ SAX implements the SAX 1.0/2.0 specification
//    • Xerces-C++ DOM implements:
//        ‣ W3C DOM Level 1 Specification
//        ‣ W3C DOM Level 2 Core Specification
//        ‣ W3C DOM Level 2 Traversal and Range Specification
//        ‣ W3C DOM Level 3.0 Core Specification
//        ‣ W3C DOM Level 3.0 Load and Save Specification
// But, per https://en.wikipedia.org/wiki/Apache_Xerces, this is only a portion of what Xerces-Jave implements!
//
// Documentation of different libraries varies between sparse and OK.  Eg the documentation for Xerces is not bad, but,
// in places, it seems to assume the reader has deep knowledge not only of various different XML API standards but also
// of the history of their evolution.  This is particularly the case when faced with several similar but different
// classes/methods that ostensibly do more-or-less the same thing.
//
// Originally, when we switched away from using Qt for XML, we opted for Apache Xerces-C++ and Xalan-C++.  Upsides were:
//    - They were mature, open-source, cross-platform, widely-used and AFAICT reasonably complete and up-to-date with
//      standards.
//    - They have a C++ interface.
//    - Although it is two libraries, once we have a document loaded in and validated via Xerces, it is then almost free
//      to bring in the companion Apache Xalan library which allows us to parse XPath expressions.  (Xalan has a
//      considerably superior XPath implementation to Xerces.)
//
// However, after a few years, we have had to revisit this decision.  Circa 2022, it was announced that Xalan-C++ would
// no longer be maintained (see eg https://github.com/microsoft/vcpkg/pull/27401).  In mid-2026, the Xerces-C++ homepage
// (https://xerces.apache.org/xerces-c/index.html) stated "Please note that Xerces-C++ currently lacks active
// maintainers and therefore may not be able to promptly address all bugs and security risks".
//
// We have therefore switched to libxml2 (https://gitlab.gnome.org/GNOME/libxml2).  It's a C interface rather than a c++
// one, but, unlike a lot of the alternatives, it supports both XSD and XPath.
//


//
// Private implementation class for XmlCoding
//
class XmlCoding::impl {
public:

   /**
    * Constructor
    */
   impl(XmlCoding & self,
        QString const & name,
        QString const & schemaResource,
        XmlRecordDefinition const & rootRecordDefinition) :
      m_self{self},
      m_initialised{false},
      m_name{name},
      m_schemaResource{schemaResource},
      m_rootRecordDefinition{rootRecordDefinition} {
      // We don't want to call loadSchema yet, as the main application will not have initialised Xerces and Xalan
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * \brief Load in the schema(s) we're going to use for validating XML documents.
    *
    * \param schemaResource The XSD schema file to load in.  The expectation is that this has been compiled into the
    *                       app as a Qt resource, so we don't need to bother with a lot of boilerplate error-handling
    *                       for file permissions or file not found etc.
    */
   void loadSchema(QString const & schemaResource, XmlErrorHandler & errorHandler) {
      // We leave it to our caller to catch any exceptions thrown in XmlSchema's constructor
      this->m_schema = std::make_unique<XmlLibHelpers::XmlSchema>(schemaResource, errorHandler);

      return;
   }

   /**
    * \brief Validate XML file against schema, then call other functions to load its contents and store them in the DB
    *
    * \param documentData The contents of the XML file, which the caller should already have loaded into memory
    * \param fileName     Used only for logging / error message
    * \param errorHandler The rules for handling any errors encountered in the file - in particular which errors should
    *                     be ignored and whether any adjustment needs to be made to the line numbers where errors are
    *                     found when creating user-readable messages.  (This latter is needed because in some encodings,
    *                     eg BeerXML, we need to modify the in-memory copy of the XML file before parsing it.  See
    *                     comments in the BeerXML-specific files for more details.)
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validateLoadAndStoreInDb(QByteArray const & documentData,
                                 QString const & fileName,
                                 XmlErrorHandler & errorHandler,
                                 QTextStream & userMessage) {
      try {
         if (!m_initialised) {
            this->loadSchema(m_schemaResource, errorHandler);
            m_initialised = true;
         }

         XmlLibHelpers::XmlDocument xmlDocument{documentData, fileName};
         if (!this->m_schema->validate(xmlDocument, userMessage)) {
            userMessage << "\n" << errorHandler.getlastError();
            return false;
         }

         // If we got this far, the validation has succeeded, and we can now proceed to loading
         return this->loadValidated(xmlDocument, userMessage);

      } catch (std::runtime_error const & re) {
         qCritical() << Q_FUNC_INFO << "Caught std::exception: " << re.what();
         userMessage << "Caught std::exception: " << re.what();
      }

      //
      // If we reach here it's because we caught an exception
      //
      return false;
   }

   /**
    * \brief Read data in from a validated & loaded XML file
    *
    * \param xmlDocument
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool loadValidated(XmlLibHelpers::XmlDocument & xmlDocument,
                      QTextStream & userMessage) {

      xmlNode * rootNode = xmlDocGetRootElement(xmlDocument.get());
      if (!rootNode) {
         qCritical() << Q_FUNC_INFO << "Couldn't find any nodes in the document!";
         userMessage << XmlCoding::tr("Contents of file were not readable");
         return false;
      }

      QString const rootNodeName{XmlLibHelpers::toQString(rootNode->name)};
      if (rootNodeName != "BEER_XML") {
         qCritical() <<
            Q_FUNC_INFO << "First node in document was not the one we inserted!  Found " << rootNodeName <<
            "instead of BEER_XML";
         userMessage << XmlCoding::tr("Could not understand file format");
         return false;
      }

      return this->loadNormaliseAndStoreInDb(xmlDocument,
                                             rootNode,
                                             rootNodeName,
                                             userMessage);
   }


   /**
    * \brief
    * \param xmlDocument
    * \param rootNode root node of document
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this.
    * \return
    */
   bool loadNormaliseAndStoreInDb(XmlLibHelpers::XmlDocument & xmlDocument,
                                  xmlNode * rootNode,
                                  QString const & rootNodeName,
                                  QTextStream & userMessage) const {

      qDebug() << Q_FUNC_INFO << "Processing root node: " << rootNodeName;

      //
      // Look at the root object first
      //
      XmlRecord rootRecord{this->m_self, this->m_rootRecordDefinition};
      qDebug() <<
         Q_FUNC_INFO << "Looking at field definitions of root element (" << this->m_rootRecordDefinition.m_recordName << ")";

      ImportRecordCount stats;

      if (!rootRecord.load(xmlDocument, *rootNode, userMessage)) {
         return false;
      }

      // At the root level, Succeeded and FoundDuplicate are both OK return values.  It's only Failed that indicates an
      // error (rather than in info) message for the user in userMessage.
      if (XmlRecord::ProcessingResult::Failed == rootRecord.normaliseAndStoreInDb(nullptr, userMessage, stats)) {
         return false;
      }

      // Everything went OK - unless we found no content to read.
      // Summarise what we read in into the message displayed on-screen to the user, and return false if no content,
      // true otherwise
      return stats.writeToUserMessage(userMessage);
   }

   // =========================================== Member variables for impl ============================================
   XmlCoding & m_self;
   bool m_initialised;
   QString const m_name;
   QString const m_schemaResource;
   XmlRecordDefinition const & m_rootRecordDefinition;

   std::unique_ptr<XmlLibHelpers::XmlSchema> m_schema = nullptr;

};

//======================================================================================================================

XmlCoding::XmlCoding(QString const name,
                     QString const schemaResource,
                     XmlRecordDefinition const & rootRecordDefinition) :
   pimpl{std::make_unique<impl>(*this, name, schemaResource, rootRecordDefinition)} {
   // As a general rule, it's not helpful to try to log anything in this constructor as the object will be created
   // before logging has been initialised.
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
XmlCoding::~XmlCoding() = default;

XmlRecordDefinition const & XmlCoding::getRoot() const {
   // The root element is the one with no corresponding named entity
   return this->pimpl->m_rootRecordDefinition;
}

bool XmlCoding::validateLoadAndStoreInDb(QByteArray const & documentData,
                                         QString const & fileName,
                                         XmlErrorHandler & errorHandler,
                                         QTextStream & userMessage) const {
   return this->pimpl->validateLoadAndStoreInDb(documentData,
                                                fileName,
                                                errorHandler,
                                                userMessage);
}
