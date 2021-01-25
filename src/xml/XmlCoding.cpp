/*
 * xml/XmlCoding.cpp is part of Brewtarget, and is Copyright the following
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
#include "xml/XmlCoding.h"

#include <QDebug>

#include <xercesc/dom/DOMConfiguration.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMLSParser.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/framework/Wrapper4InputSource.hpp>
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

#include <xalanc/XalanDOM/XalanDocument.hpp>
#include <xalanc/XalanDOM/XalanNode.hpp>
#include <xalanc/XalanDOM/XalanNodeList.hpp>
#include <xalanc/XercesParserLiaison/XercesParserLiaisonDefinitions.hpp>
#include <xalanc/XercesParserLiaison/XercesParserLiaison.hpp>
#include <xalanc/XercesParserLiaison/XercesDOMSupport.hpp>
#include <xalanc/XPath/XPathEvaluator.hpp>

#include "xml/BtDomDocumentOwner.h"
#include "xml/XercesHelpers.h"
#include "xml/XmlRecordCount.h"

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
// https://forum.qt.io/topic/102834/proper-successor-for-qxmlschemavalidator/6) to use another library.  For now, we've
// decided to use Apache Xerces as it's mature, open-source, cross-platform, widely-used and AFAICT reasonably complete
// and up-to-date with standards.  (We might at some point also want to look at CodeSynthesis XSD, which is built on top
// of Xerces and adds some extra features, but seems to be somewhat less widely used than Xerces.)
//
// Once we have a document loaded in and validated via Xerces, it is then almost free to bring in the companion Apache
// Xalan library which allows us to parse XPath expressions.  (Xalan has much superior XPath implementation to Xerces.)
//
// The documentation for Xerces is not bad, but, in places, it seems to assume the reader has deep knowledge not only
// of various different XML API standards but also of the history of their evolution - in particular when faced with
// several similar but different classes/methods that ostensibly do more-or-less the same thing.  This is not entirely
// surprising given that (per https://xerces.apache.org/xerces-c/api-3.html) Xerces is implementing several different
// specifications:
//  • Xerces-C++ SAX implements the SAX 1.0/2.0 specification
//  • Xerces-C++ DOM imlements:
//    ‣ W3C DOM Level 1 Specification
//    ‣ W3C DOM Level 2 Core Specification
//    ‣ W3C DOM Level 2 Traversal and Range Specification
//    ‣ W3C DOM Level 3.0 Core Specification
//    ‣ W3C DOM Level 3.0 Load and Save Specification
// And, of course, the DOM specifications in particular are intentionally quite broad because DOM is a
// programming-language-neutral API specification for APIs for accessing both XML and HTML documents.
//
// All of this mostly does not prevent you doing things as you can just copy-and-paste example code, and if you hit
// a problem there's a good chance you can find how someone else already solved it by searching on Stackoverflow
// etc.  Nonetheless, you do sometimes need to do a bit of research to understand what's going on and what your
// options are.  We try to include such explanations in comments, which is partly why they are a bit more
// substantial than in some other areas of the code base.
//
// Like Xerces, Xalan is pretty tried-and-tested, but there's a bit more of a learning curve as the documentation isn't
// quite as good as that of Xerces.  Fortunately, a lot of the basics are very similar to Xerces and we're not actually
// using a huge number of different function calls.
//


//
// Private implementation class for XmlCoding
//
class XmlCoding::impl {
public:

   /**
    * Constructor
    */
   impl(QString const schemaResource) :
      grammarPool(xercesc::XMLPlatformUtils::fgMemoryManager) {
      this->loadSchema(schemaResource);
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * \brief Load in the schema(s) we're going to use for validating XML documents.
    *
    *        This is the complicated bit of using Xerces.  Once this is done, remaining usage is pretty
    *        straightforward!
    *
    * \param schemaResource The XSD schema file to load in.  The expectation is that this has been compiled into the
    *                       app as a Qt resource, so we don't need to bother with a lot of boilerplate error-handling
    *                       for file permissions or file not found etc.
    */
   void loadSchema(QString const & schemaResource) {
      //
      // See https://stackoverflow.com/questions/52275608/xerces-c-validate-xml-with-hardcoded-xsd and
      // http://www.codesynthesis.com/~boris/blog/2010/03/15/validating-external-schemas-xerces-cxx/ (plus linked
      // public-domain example code) for advice about using fixed application-determined XSDs rather than trying to pull
      // them off the internet on the fly.
      //
      // The mysterious "features" parameter that we need to pass in to DOMImplementationRegistry::getDOMImplementation()
      // come from W3C DOM specifications - see eg:
      //  • https://www.w3.org/TR/DOM-Level-3-Core/introduction.html#ID-Conformance
      //  • https://www.w3.org/TR/DOM-Level-2-Core/#introduction-ID-Conformance
      // According to https://c-dev.xerces.apache.narkive.com/yF69tsO8/list-of-dom-implementation-features, Xerces
      // implements the following features and levels thereof:
      //  • "XML"
      //  • "1.0"
      //  • "2.0"
      //  • "3.0"
      //  • "Traversal"
      //  • "Core"
      //  • "Range"
      //  • "LS" = Load and Save  (which means I think implements the "platform- and language-neutral interface" interface
      //                           defined in DOM Level 3 (https://www.w3.org/TR/2004/REC-DOM-Level-3-LS-20040407/)
      // In practice, since we are not extending Xerces (eg to parse other SGML-derived languages), I'm not sure how much
      // it matters what features we request.  (The xercesc::DOMImplementation class inherits from
      // xercesc::DOMImplementationLS for instance.)  Most of the easily-found example code seems to use "LS" (or
      // sometimes "Range") but this is perhaps because "LS" is the shortest!
      //
      XQString const features("LS");
      this->domImplementation = xercesc::DOMImplementationRegistry::getDOMImplementation(features.getXercesString());

      //
      // According to https://xerces.apache.org/xerces-c/program-dom-3.html, DOMLSParser is a new interface introduced by
      // the W3C DOM Level 3.0 Load and Save Specification.  DOMLSParser provides the "Load" interface for parsing XML
      // documents and building the corresponding DOM document tree from various input sources.  AIUI from
      // https://markmail.org/message/5ztcgzgb5a7ldys3, DOMLSParser supersedes XercesDOMParser (which is nonetheless still
      // available to use).
      //
      // The second parameter here is, per https://xerces.apache.org/xerces-c/apiDocs-3/classDOMImplementationLS.html,
      // set to null "to create a DOMLSParser for any kind of schema types (i.e. the DOMLSParser will be free to use
      // any schema found)".  (The description goes on to say you "must" use the value
      // "http://www.w3.org/2001/XMLSchema" for W3C XML Schema [XML Schema Part 1], but I think this actually just
      // means you _can_ do that _if_ you want to restrict schema types to XML Schemas (as opposed to DTDs or some
      // other schema language).   Since we completely control the schemas we're using, there seems little benefit in
      // trying to specify such restrictions here.
      //
      this->parser =
         domImplementation->createLSParser(xercesc::DOMImplementationLS::MODE_SYNCHRONOUS,
                                           nullptr  /*,
                                           xercesc::XMLPlatformUtils::fgMemoryManager, .:TBD:. Shall we reenable the grammar pool stuff?
                                           &this->grammarPool*/);

      //
      // See https://xerces.apache.org/xerces-c/program-dom-3.html for full details of these config options
      //
      // Note that, although each parameter is defined by a string name (eg "comments"), the name must be passed in as
      // a UTF-16 string.  To make this easy/efficient, Xerces has predefined all the names as suitable UTF-16 strings.
      // Thus, to set the "comments" parameter, you pass in xercesc::XMLUni::fgDOMComments, which is just a pointer to
      // a predefined UTF-16 string saying "comments".  The link above has both, but not every documentation page does.
      //
      // Note too that some of these parameters are defined by the W3C DOM Level 3 standard, and some are Xerces
      // extensions to that standard.  The latter have long names that begin with "http://apache.org/xml/features/"
      //
      // Finally, be aware that some parameter settings from the DOM standard are not supported, but there is no return
      // code from setParameter() to tell you this.  (You would have to call canSetParameter() first.)  So, for
      // example, setting "namespace-declarations" (aka fgDOMNamespaceDeclarations) to false will not immediately break
      // anything but will cause a subsequent error of "implementation does not support the requested type of object or
      // operation" when you, say, try to parse a document.
      //
      xercesc::DOMConfiguration * config = this->parser->getDomConfig();

      // "comments" - false = Discard Comment nodes in document
      config->setParameter(xercesc::XMLUni::fgDOMComments, false);

      // "datatype-normalization" - true = Let validation process do datatype normalization
      config->setParameter(xercesc::XMLUni::fgDOMDatatypeNormalization, true);

      // "entities" - false = Do not create EntityReference nodes
      config->setParameter(xercesc::XMLUni::fgDOMEntities, false);

      // "namespaces"
      // true = Perform Namespace processing
      //        NB: This must be turned on if "http://apache.org/xml/features/validation/schema" is enabled.  (It's a
      //        logical requirement, given that schemas need to use namespaces, but it's worth remembering because the
      //        errors that you get trying to use schemas without namespace processing enabled are pretty cryptic!)
      config->setParameter(xercesc::XMLUni::fgDOMNamespaces, true);

      // "whitespace-in-element-content" - false = Do not include ignorable whitespace in DOM tree
      config->setParameter(xercesc::XMLUni::fgDOMElementContentWhitespace, false);

      // "validation" - true = Report all validation errors
      config->setParameter(xercesc::XMLUni::fgDOMValidate, true);

      // "http://apache.org/xml/features/validation/schema"
      // true = Enable parser's schema support
      //        NB: If set to true, namespace processing must also be turned on.
      config->setParameter(xercesc::XMLUni::fgXercesSchema, true);

      // "http://apache.org/xml/features/validation/schema-full-checking"
      // false = Disable full schema constraint checking.
      //         (Setting this to true would merely check the schema grammar itself for additional errors that are
      //         time-consuming or memory intensive to perform.  Given that we know in advance all the schemas we are
      //         going to use, this is something only to enable in dev when tweaking one or more of those schemas.)
      config->setParameter(xercesc::XMLUni::fgXercesSchemaFullChecking, false);

      // "http://apache.org/xml/features/validation/schema/handle-multiple-imports"
      // true = During schema validation allow multiple schemas with the same namespace to be imported
      config->setParameter(xercesc::XMLUni::fgXercesHandleMultipleImports, true);

      // "http://apache.org/xml/features/validation/cache-grammarFromParse"
      // true = Cache the grammar in the pool for re-use in subsequent parses
//      config->setParameter(xercesc::XMLUni::fgXercesCacheGrammarFromParse, true);

      BtDomErrorHandler domErrorHandler;
      config->setParameter(xercesc::XMLUni::fgDOMErrorHandler, &domErrorHandler);

      QFile schemaFile(schemaResource);
      if (!schemaFile.open(QIODevice::ReadOnly)) {
         // This should pretty much never happen, as we're loading from a QResource compiled into the binary rather
         // than reading from the file system at run-time.
         qCritical() <<
            Q_FUNC_INFO << "Could not open schema file resource " << schemaFile.fileName() << " for reading";
         throw std::runtime_error("Could not open schema file resource");
      }

      QByteArray schemaData = schemaFile.readAll();
      qDebug() <<
         Q_FUNC_INFO << "Schema file " << schemaFile.fileName() << ": " << schemaData.length() << " bytes";

      // Don't want qDebug to escape newlines, as there will be lots in the list of parameter settings, hence
      // ".noquote()" here.
      qDebug().noquote() <<
         Q_FUNC_INFO << "Settings for reading schema file " << schemaFile.fileName() << ": " <<
         XercesHelpers::getParameterSettings(*config);

      // The third parameter is just a name for the object.  It's not used by Xerces, but does show up in error
      // messages (as the URI of the error location), so we use the file name as something vaguely helpful to show
      // there.
      QByteArray schemaFileNameAsCString = schemaFile.fileName().toLocal8Bit();
      xercesc::MemBufInputSource schemaAsInputSource{reinterpret_cast<const XMLByte *>(schemaData.constData()),
                                                     static_cast<XMLSize_t>(schemaData.length()),
                                                     schemaFileNameAsCString};

      xercesc::Wrapper4InputSource schemaAsDOMLSInput{&schemaAsInputSource, false};

      // Load the schema and cache its grammar (third parameter = true does the latter)
      // The returned preparsed schema grammar object (SchemaGrammar or DTDGrammar) is owned by the parser and should
      // not be deleted by the user.
      // Strictly, we should try/catch this for SAXException, XMLException. DOMException.  However, we are not
      // expecting any of these because we are parsing our own XSD file that is compiled into the program binary.
      xercesc::Grammar * grammar = this->parser->loadGrammar(&schemaAsDOMLSInput,
                                                             xercesc::Grammar::SchemaGrammarType,
                                                             true);
      if (!grammar) {
         // As above, this shouldn't happen "in production" as it's our own schema file, so we should make it parseable
         qCritical() << Q_FUNC_INFO << "Unable to parse schema " << schemaFile.fileName();
         throw std::runtime_error("Unable to parse schema -- see log file for more details");
      }

      if (domErrorHandler.failed()) {
         qCritical() << Q_FUNC_INFO << "Error parsing schema " << schemaFile.fileName();
         throw std::runtime_error("Error parsing schema -- see log file for more details");
      }

      xercesc::Grammar * rootGrammar = this->parser->getRootGrammar();

      qDebug() <<
         Q_FUNC_INFO << "Schema " << schemaFile.fileName() << " loaded OK.  Grammar:" << grammar << ", root grammar:" <<
         rootGrammar;

      // "http://apache.org/xml/features/validation/use-cachedGrammarInParse"
      // true = Use cached grammar if it exists in the pool
      config->setParameter(xercesc::XMLUni::fgXercesUseCachedGrammarInParse, true);

      // "http://apache.org/xml/features/validating/load-schema"
      // false = Don't load the schema if it wasn't found in the grammar pool, ie don't load schemas from any other
      //         source (e.g., from XML document's xsi:schemaLocation attributes).
      config->setParameter(xercesc::XMLUni::fgXercesLoadSchema, false);

      // "http://apache.org/xml/features/dom/user-adopts-DOMDocument"
      // true = The caller will adopt the DOMDocument that is returned from the parse method and thus is responsible to
      //        call xercesc::DOMDocument::release() to release the associated memory. The parser will not release it.
      //        The ownership is transferred from the parser to the caller.
      //
      // The reason for setting this to true is that we reuse the parser, so we don't want to wait until its destructor
      // is called for all the DOMDocument objects to be released.
      config->setParameter(xercesc::XMLUni::fgXercesUserAdoptsDOMDocument, true);

      return;
   }

   /**
    * \brief Validate XML file against schema, then call other functions to load its contents and store them in the DB
    *
    * \param xmlCoding Back pointer to the containing class
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
   bool validateLoadAndStoreInDb(XmlCoding const * xmlCoding,
                                 QByteArray const & documentData,
                                 QString const & fileName,
                                 BtDomErrorHandler & domErrorHandler,
                                 QTextStream & userMessage) {
      // See https://www.codesynthesis.com/pipermail/xsd-users/2010-April/002805.html for list of all exceptions Xerces
      // can throw.
      try {
         // Probably not 100% necessary to lock the pool against modifications, as we're not planning any after start-up, but...
         this->grammarPool.lockPool();

         /// TBD probably need to lock other things here ///


         xercesc::DOMConfiguration * config = this->parser->getDomConfig();
         config->setParameter(xercesc::XMLUni::fgDOMErrorHandler, &domErrorHandler);

         // Don't want qDebug to escape newlines, as there will be lots in the list of parameter settings, hence
         // ".noquote()" here.
         qDebug().noquote() <<
            Q_FUNC_INFO << "Settings for reading input " << fileName << ": " << XercesHelpers::getParameterSettings(*config);

         QByteArray fileNameAsCString = fileName.toLocal8Bit();

         // Per comment above, third parameter is just a name for the object, which will show up in error messages.
         // File name seems sensible.
         xercesc::MemBufInputSource documentAsInputSource{reinterpret_cast<const XMLByte *>(documentData.constData()),
                                                          static_cast<XMLSize_t>(documentData.length()),
                                                          fileNameAsCString.constData()};

         xercesc::Wrapper4InputSource documentAsDOMLSInput{&documentAsInputSource, false};


         // The BtDomDocumentOwner object will, in its destructor, handle telling Xerces to release resources related
         // to the document
         // std::shared_ptr<BtDomDocumentOwner> domDocumentOwner{new BtDomDocumentOwner{this->parser->parse(&documentAsDOMLSInput)}}
         BtDomDocumentOwner domDocumentOwner{this->parser->parse(&documentAsDOMLSInput)};

         bool parsedOk = !domErrorHandler.failed();
         qDebug() << Q_FUNC_INFO << "Parse of input file " << fileName << (parsedOk ? "succeeded" : "FAILED");

         if (!parsedOk) {
            userMessage << domErrorHandler.getlastError();
            return false;
         }

         if (nullptr == domDocumentOwner.getDomDocument()) {
            //
            // This really should never happen.  Xerces is only supposed to return null from parse() if it in
            // asynchronous mode (which it shouln't be).
            //
            qCritical() << Q_FUNC_INFO << "Got null pointer back from document parse!";
            userMessage << tr("Internal Error! (Document parse returned null pointer.)");
            return false;
         }

         // If we got this far, the validation has succeeded, and we can now proceed to loading
         return this->loadValidated(xmlCoding, domDocumentOwner.getDomDocument(), userMessage);

      } catch(const std::exception& se) {
         qCritical() << Q_FUNC_INFO << "Caught std::exception: " << se.what();
         userMessage << "Caught std::exception: " << se.what();
      } catch (const xercesc::XMLException & xe) {
         unsigned int lineNumberOfError = domErrorHandler.correctErrorLine(xe.getSrcLine());
         qCritical() <<
            Q_FUNC_INFO << "Caught xerces::XMLException at line " << lineNumberOfError << ": " <<
            XQString(xe.getType()) << ": " << XQString(xe.getMessage());
         userMessage <<
            "XMLException at line " << lineNumberOfError << ": " << XQString(xe.getType())  << ": " <<
            XQString(xe.getMessage());
      } catch (const xercesc::DOMException & de) {
         qCritical() <<
            Q_FUNC_INFO << "Caught xerces::DOMException #" << de.code << ": " << XQString(de.getMessage());
         userMessage << "DOMException #" << de.code << ": " << XQString(de.getMessage());
      } catch (const xercesc::SAXException & se) {
         qCritical() <<
            Q_FUNC_INFO << "Caught xerces::SAXException: " << XQString(se.getMessage());

         userMessage << "SAXException: " << XQString(se.getMessage());
      }
      //
      // If we reach here it's because we caught an exception
      //
      return false;
   }

   /**
    * \brief Read data in from a validated & loaded XML file
    *
    * \param xmlCoding Back pointer to the containing class
    * \param domDocument Pointer to the Xerces document created by loading and validating the XML file.  Caller owns
    *                    the Xerces document and is responsible for releasing its resources (after this function
    *                    returns).
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool loadValidated(XmlCoding const * xmlCoding, xercesc::DOMDocument * domDocument, QTextStream & userMessage) {

      //
      // Some of the initial things we're doing here are just as easy to do in Xerces, but it's easiest to start
      // with the document when we switch over to Xalan (rather than some node inside it).
      //
      xalanc::XercesParserLiaison xalanXercesLiaison;
      xalanc::XercesDOMSupport domSupport(xalanXercesLiaison);

      xalanc::XalanDocument * xalanDocument {xalanXercesLiaison.createDocument(domDocument)};

      //
      // One way to get the root node in Xerces would be:
      //    xercesc::DOMNodeList * listOfRootNodes =
      //       domDocument->getElementsByTagName(XQString("BEER_XML").getXercesString());
      //    xercesc::DOMNode * rootNode = listOfRootNodes->item(0); // 0 is the first node
      //
      xalanc::XalanNode * rootNode = xalanDocument->getFirstChild();
      if (nullptr == rootNode) {
         qCritical() << Q_FUNC_INFO << "Couldn't find any nodes in the document!";
         userMessage << xmlCoding->tr("Contents of file were not readable");
         return false;
      }
      XQString firstChildName{rootNode->getNodeName()};
      if (firstChildName != "BEER_XML") {
         qCritical() <<
            Q_FUNC_INFO << "First node in document was not the one we inserted!  Found " << firstChildName <<
            "instead of BEER_XML";
         userMessage << xmlCoding->tr("Could not understand file format");
         return false;
      }

      return this->loadNormaliseAndStoreInDb(xmlCoding, domSupport, rootNode, userMessage);
   }


   /**
    * \brief
    * \param xmlCoding Back pointer to the containing class
    * \param domSupport
    * \param rootNode root node of document
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this.
    * \return
    */
   bool loadNormaliseAndStoreInDb(XmlCoding const * xmlCoding,
                                  xalanc::DOMSupport & domSupport,
                                  xalanc::XalanNode * rootNode,
                                  QTextStream & userMessage) const {

      XQString rootNodeName{rootNode->getNodeName()};
      qDebug() << Q_FUNC_INFO << "Processing root node: " << rootNodeName;

      // It's usually a coding error if we don't understand how to process the root node, because it should have been
      // validated by the XSD.  (In the case of BeerXML, the root node is a manufactured one that we inserted, which is all
      // the more reason we should know how to process it!)
      // If asserts are turned off for any reason, we should just abort processing
      Q_ASSERT(xmlCoding->isKnownXmlRecordType(rootNodeName));
      if (!xmlCoding->isKnownXmlRecordType(rootNodeName)) {
         qCritical() << Q_FUNC_INFO << "First node in document (" << rootNodeName << ") was not recognised!";
         userMessage << xmlCoding->tr("Could not understand file format");
         return false;
      }

      std::shared_ptr<XmlRecord> rootRecord = xmlCoding->getNewXmlRecord(rootNodeName);

      XmlRecordCount stats;

      if (!rootRecord->load(domSupport, rootNode, userMessage)) {
         return false;
      }

      // At the root level, Succeeded and FoundDuplicate are both OK return values.  It's only Failed that indicates an
      // error (rather than in info) message for the user in userMessage.
      if (XmlRecord::Failed == rootRecord->normaliseAndStoreInDb(nullptr, userMessage, stats)) {
         return false;
      }

      // Everything went OK - unless we found no content to read.
      // Summarise what we read in into the message displayed on-screen to the user, and return false if no content,
      // true otherwise
      return stats.writeToUserMessage(userMessage);
   }

private:
   // XMLGrammarPoolImpl is a bit lacking in documentation, probably because it used to be an "internal" class of
   // Xerces.  However, since Xerces 3.0.0 release, it is now part of the public API -- see
   // https://xerces.apache.org/xerces-c/migrate-archive-3.html#NewAPI300
   xercesc::XMLGrammarPoolImpl grammarPool;

   xercesc::DOMImplementation * domImplementation;
   xercesc::DOMLSParser * parser;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

XmlCoding::XmlCoding(QString const name,
                     QString const schemaResource,
                     QHash<QString, XmlRecordDefinition> const & entityNameToXmlRecordDefinition) :
   name{name},
   entityNameToXmlRecordDefinition{entityNameToXmlRecordDefinition},
   pimpl{ new impl{schemaResource} } {
   qDebug() << Q_FUNC_INFO;
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
XmlCoding::~XmlCoding() = default;

bool XmlCoding::isKnownXmlRecordType(QString recordName) const {
   return this->entityNameToXmlRecordDefinition.contains(recordName);
}


std::shared_ptr<XmlRecord> XmlCoding::getNewXmlRecord(QString recordName) const {
   XmlCoding::XmlRecordConstructorWrapper constructorWrapper =
      this->entityNameToXmlRecordDefinition.value(recordName).constructorWrapper;

   XmlRecord::FieldDefinitions const * fieldDefinitions =
      this->entityNameToXmlRecordDefinition.value(recordName).fieldDefinitions;

   return std::shared_ptr<XmlRecord>(constructorWrapper(*this, *fieldDefinitions));
}


bool XmlCoding::validateLoadAndStoreInDb(QByteArray const & documentData,
                                         QString const & fileName,
                                         BtDomErrorHandler & domErrorHandler,
                                         QTextStream & userMessage) const {
   return this->pimpl->validateLoadAndStoreInDb(this, documentData, fileName, domErrorHandler, userMessage);
}
