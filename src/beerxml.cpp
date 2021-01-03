/*
 * beerxml.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
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

#include "beerxml.h"

#include <stdexcept>

#include <QList>
#include <QDomDocument>
#include <QIODevice>
#include <QDomNodeList>
#include <QDomNode>
#include <QTextStream>
#include <QTextCodec>
#include <QObject>
#include <QString>
#include <QStringBuilder>
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>
#include <QThread>
#include <QDebug>
#include <QPair>

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

#include "Algorithms.h"
#include "DatabaseSchema.h"
#include "brewnote.h"
#include "equipment.h"
#include "fermentable.h"
#include "hop.h"
#include "instruction.h"
#include "mash.h"
#include "mashstep.h"
#include "misc.h"
#include "recipe.h"
#include "style.h"
#include "water.h"
#include "salt.h"
#include "yeast.h"
#include "xml/BtDomDocumentOwner.h"
#include "xml/BtDomErrorHandler.h"
#include "xml/XercesHelpers.h"
#include "xml/XPathRecordLoader.h"
#include "xml/XQString.h"
#include "xml/BeerXmlFermentableRecordLoader.h"
#include "xml/BeerXmlHopRecordLoader.h"
#include "xml/BeerXmlYeastRecordLoader.h"

#include "TableSchema.h"




// This private implementation class holds all private non-virtual members of BeerXML
class BeerXML::impl {
public:

   /**
    * Constructor
    */
   impl() : grammarPool(xercesc::XMLPlatformUtils::fgMemoryManager) {
      this->loadSchemas();
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   //
   // Frustratingly, although Qt has support for XML parsing, it would  not be wise to use it for dealing with XML
   // Schemas.  In mid-2019, in release 5.13, Qt deprecated its "XML Patterns" package which included QXmlSchema etc,
   // and the package was removed from Qt in the Qt6.0 release of December 2020.  It's not entirely clear why these
   // features are being dropped, though, it's conceivable it may be related to the fact that some of the other Qt XML
   // classes are not standards-compliant (see https://www.qt.io/blog/parsing-xml-with-qt-updates-for-qt-6) and Qt have
   // decided to offer a slimmed-down but standards-compliant support for XML via QXmlStreamReader and
   // QXmlStreamWriter.
   //
   // For those who want to manipulate XML schemas (or, for that matter, use standards-compliant DOM or SAX APIs for
   // accessing XML documents), the official advice from Qt's developers seems simply to be (according to
   // https://forum.qt.io/topic/102834/proper-successor-for-qxmlschemavalidator/6) to use another library.  For now,
   // we've decided to use Xerces as it's mature, open-source, cross-platform, widely-used and AFAICT reasonably
   // complete and up-to-date with standards.  (We might at some point also want to look at CodeSynthesis XSD, which is
   // built on top of Apache Xerces and adds some extra features, but seems to be somewhat less widely used than
   // Xerces.)
   //
   // The documentation for Xerces is not bad, but, in places, it seems to assume the reader has deep knowledge not
   // only of various different XML API standards but also of the history of their evolution - in particular when faced
   // with several similar but different classes/methods that ostensibly do more-or-less the same thing.  This is not
   // entirely surprising given that (per https://xerces.apache.org/xerces-c/api-3.html) Xerces is implementing several
   // different specifications:
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

   /**
    * \brief Load in the schema(s) we're going to use for validating XML documents.
    *
    * This is the complicated bit of using Xerces.  Once this is done, remaining usage is pretty straightforward!
    */
   void loadSchemas() {
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
      config->setParameter(xercesc::XMLUni::fgDOMComments, false); // =====

      // "datatype-normalization" - true = Let validation process do datatype normalization
      config->setParameter(xercesc::XMLUni::fgDOMDatatypeNormalization, true); // =====

      // "entities" - false = Do not create EntityReference nodes
      config->setParameter(xercesc::XMLUni::fgDOMEntities, false); // =====

      // "namespaces"
      // true = Perform Namespace processing
      //        NB: This must be turned on if "http://apache.org/xml/features/validation/schema" is enabled.  (It's a
      //        logical requirement, given that schemas need to use namespaces, but it's worth remembering because the
      //        errors that you get trying to use schemas without namespace processing enabled are pretty cryptic!)
      config->setParameter(xercesc::XMLUni::fgDOMNamespaces, true); // =====

      // "whitespace-in-element-content" - false = Do not include ignorable whitespace in DOM tree
      config->setParameter(xercesc::XMLUni::fgDOMElementContentWhitespace, false); // =====

      // "validation" - true = Report all validation errors
      config->setParameter(xercesc::XMLUni::fgDOMValidate, true); // =====

      // "http://apache.org/xml/features/validation/schema"
      // true = Enable parser's schema support
      //        NB: If set to true, namespace processing must also be turned on.
      config->setParameter(xercesc::XMLUni::fgXercesSchema, true); // =====

      // "http://apache.org/xml/features/validation/schema-full-checking"
      // false = Disable full schema constraint checking.
      //         (Setting this to true would merely check the schema grammar itself for additional errors that are
      //         time-consuming or memory intensive to perform.  Given that we know in advance all the schemas we are
      //         going to use, this is something only to enable in dev when tweaking one or more of those schemas.)
      config->setParameter(xercesc::XMLUni::fgXercesSchemaFullChecking, false); // =====

      // "http://apache.org/xml/features/validation/schema/handle-multiple-imports"
      // true = During schema validation allow multiple schemas with the same namespace to be imported
      config->setParameter(xercesc::XMLUni::fgXercesHandleMultipleImports, true); // =====

      // "http://apache.org/xml/features/validation/cache-grammarFromParse"
      // true = Cache the grammar in the pool for re-use in subsequent parses
//      config->setParameter(xercesc::XMLUni::fgXercesCacheGrammarFromParse, true);



      BtDomErrorHandler domErrorHandler;
      config->setParameter(xercesc::XMLUni::fgDOMErrorHandler, &domErrorHandler);

      QFile schemaFile(":/beerxml/v1/BeerXml.xsd");
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
         Q_FUNC_INFO << "Settings for reading schema file " << schemaFile.fileName() << ": " << XercesHelpers::getParameterSettings(*config);

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
      xercesc::Grammar * grammar = this->parser->loadGrammar(&schemaAsDOMLSInput, xercesc::Grammar::SchemaGrammarType, true);
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
    * \brief Validate XML file against schema and load its contents
    *
    * \param fileName Fully-qualified name of the file to validate
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validateAndLoad(QString const & fileName, QTextStream & userMessage) {

      QFile inputFile;
      inputFile.setFileName(fileName);

      if(! inputFile.open(QIODevice::ReadOnly)) {
         qWarning() << Q_FUNC_INFO << ": Could not open " << fileName << " for reading";
         return false;
      }

      //
      // Rather than just read the XML file into memory, we actually make a small on-the-fly modification to it to
      // place all the top-level content inside a <BEER_XML>...</BEER_XML> field.  This massively simplifies the XSD
      // (as explained in a comment therein) at the cost of some minor complexity here.  Essentially, the added tag
      // pair is (much as we might have wished it were part of the original BeerXML 1.0 Specification) something we
      // need to hide from the user to avoid confusion (as the tag does not and is not supposed to exist in the
      // document they are asking us to process).
      //
      // Fortunately the edit is simple:
      //  - We retain unchanged the first line of the file (which will be something along the lines of
      //    "<?xml version="1.0" blah blah ?>"
      //  - We insert a new line 2 that says "<BEER_XML>"
      //  - We read in the rest of the file unchanged (so what was line 2 on disk will be line 3 in memory and so on)
      //  - We append a new final line that says "</BEER_XML>"
      //
      // We then give enough information to our instance of BtDomErrorHandler to allow it to correct the line numbers
      // for any errors it needs to log.  (And we get a bit of help from this class when we need to make similar
      // adjustments during exception processing.)
      //
      // Note here that we are assuming the on-disk format of the file is single-byte (UTF-8 or ASCII or ISO-8859-1).
      // This is a reasonably safe assumption but, in theory, we could examine the first line to verify it.
      //
      QByteArray documentData = inputFile.readLine();
      qDebug() << Q_FUNC_INFO << "First line of " << inputFile.fileName() << " was " << QString(documentData);
      documentData += "<";
      documentData += INSERTED_ROOT_NODE_NAME;
      documentData += ">\n";
      documentData += inputFile.readAll();
      documentData += "\n</";
      documentData += INSERTED_ROOT_NODE_NAME;
      documentData += ">";
      qDebug() << Q_FUNC_INFO << "Input file " << inputFile.fileName() << ": " << documentData.length() << " bytes";

      // It is sometimes helpful to uncomment the next line for debugging, but usually leave it commented out as can
      // put a _lot_ of data in the logs in DEBUG mode.
      // qDebug().noquote() << Q_FUNC_INFO << "Full content of " << inputFile.fileName() << " is:\n" << QString(documentData);

      //
      // Some errors we explicitly want to ignore.  In particular, the BeerXML 1.0 standard says:
      //
      //    "Non-Standard Tags
      //    "Per the XML standard, all non-standard tags will be ignored by the importing program.  This allows programs
      //    to store additional information if desired using their own tags.  Any tags not defined as part of this
      //    standard may safely be ignored by the importing program."
      //
      // There are two problems with this.  One is that it does not prevent two different programs creating
      // identically-named custom tags with different meanings.  (And note that it is observably NOT the case that
      // existing implementations take any care to make their custom tag names unique to the program using them.)
      //
      // The second problem is that, because the BeerXML 1.0 standard also says that tags inside a containing element
      // may occur in any order, we cannot easily tell the XSD to ignore unkonwn tags.  (The issue is that, in the XSD,
      // we have to to use <xs:all> rather than <xs:sequence> for the containing tags, as this allows the contained
      // tags to appear in any order.  In turn, this means we cannot use <xs:any> to allow unrecognised tags.  This is
      // disallowed by the W3C XML Schema standard because it would make validation harder (and slower).  See
      // https://stackoverflow.com/questions/3347822/validating-xml-with-xsds-but-still-allow-extensibility for a good
      // explanation.)
      //
      // So, our workaround for this is to ignore errors that say:
      //   • "no declaration found for element 'ABC'"
      //   • "element 'ABC' is not allowed for content model 'XYZ'.
      //
      static QVector<BtDomErrorHandler::PatternAndReason> const errorPatternsToIgnore {
         //       Reg-ex to match                                               Reason to ignore errors matching this pattern
         {QString("^no declaration found for element"),                 QString("we are assuming unrecognised tags are just non-standard tags in the BeerXML")},
         {QString("^element '[^']*' is not allowed for content model"), QString("we are assuming unrecognised tags are just non-standard tags in the BeerXML")}
      };
      BtDomErrorHandler domErrorHandler(&errorPatternsToIgnore, 1 , 1);

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
            Q_FUNC_INFO << "Settings for reading input " << inputFile.fileName() <<
            ": " << XercesHelpers::getParameterSettings(*config);

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
         qDebug() << Q_FUNC_INFO << "Parse of input file " << inputFile.fileName() << (parsedOk ? "succeeded" : "FAILED");

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
         return this->loadValidated(domDocumentOwner.getDomDocument(), userMessage);

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
    * \param domDocument Pointer to the Xerces document created by loading and validating the XML file.  Caller owns
    *                    the Xerces document and is responsible for releasing its resources (after this function
    *                    returns).
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool loadValidated(xercesc::DOMDocument * domDocument, QTextStream & userMessage) {

      //
      // Now we've used Xerces to validate the XML against an XSD, we switch over to using Xalan to explore the
      // document structure because it has much superior XPath implementation to Xerces.
      //
      // Xalan is pretty tried-and-tested, but there's a bit of a learning curve as the documentation isn't quite as
      // good as that of Xerces.  Fortunately, a lot of the basics are very similar to Xerces.
      //
      // Some of the initial things we're doing here are just as easy to do in Xerces, but it's easiest to start
      // with the document when we switch over to Xalan (rather than some node inside it).
      //
      xalanc::XercesParserLiaison xalanXercesLiaison;
      xalanc::XercesDOMSupport domSupport(xalanXercesLiaison);

      xalanc::XalanDocument * xalanDocument {xalanXercesLiaison.createDocument(domDocument)};

      //
      // It should be a slam-dunk for us to get the root <BEER_XML> node we inserted in the document - unless the
      // file we were reading was really broken (in which case we would usually expect it to have failed validation
      // prior to this point).  Nevertheless, if there is some problem here, we bail out gracefully.
      //
      // One way to get the root node in Xerces would be:
      //    xercesc::DOMNodeList * listOfRootNodes =
      //       domDocument->getElementsByTagName(XQString("BEER_XML").getXercesString());
      //    xercesc::DOMNode * rootNode = listOfRootNodes->item(0); // 0 is the first node
      //
      xalanc::XalanNode * rootNode = xalanDocument->getFirstChild();
      if (nullptr == rootNode) {
         qCritical() << Q_FUNC_INFO << "Couldn't find any nodes in the document!";
         userMessage << tr("Contents of file were not readable");
         return false;
      }
      XQString firstChildName{rootNode->getNodeName()};
      if (firstChildName != INSERTED_ROOT_NODE_NAME) {
         qCritical() <<
            Q_FUNC_INFO << "First node in document was not the one we inserted!  Found " << firstChildName <<
            "instead of " << INSERTED_ROOT_NODE_NAME;
         userMessage << tr("Could not understand file format");
         return false;
      }

      //
      // It should now be the case that all the children of our inserted root node are the "record sets" of
      // the BeerXML document, ie one ore more of the following:
      //    <HOPS>...</HOPS>
      //    <FERMENTABLES>...</FERMENTABLES>
      //    <YEASTS>...</YEASTS>
      //    <MISCS>...</MISCS>
      //    <WATERS>...</WATERS>
      //    <STYLES>...</STYLES>
      //    <MASHS>...</MASHS>
      //    <RECIPES>...</RECIPES>
      //    <EQUIPMENTS>...</EQUIPMENTS>
      //
      // In Xerces, we would just do this:
      //    xercesc::DOMNodeList * recordSets = rootNode->getChildNodes();
      //
      // Note, that, in the extreme case that there are no child nodes (eg someone tried to import an empty
      // XML doc) getChildNodes() returns a list containing no nodes, NOT nullptr.  (This is the same in Xerces
      // and Xalan.)
      //
      xalanc::XalanNodeList const * recordSets = rootNode->getChildNodes();
      int numberOfRecordSets = recordSets->getLength();
      qDebug() << Q_FUNC_INFO << "Found " << numberOfRecordSets << "record sets";
      if (0 == numberOfRecordSets) {
         userMessage << tr("Found no data to import in the file");
         return false;
      }

      //
      // So now let's look at, and process, each record set
      //
      QHash<QString, int> importCount{};
      for (int ii = 0; ii < numberOfRecordSets; ++ii) {
         xalanc::XalanNode * currentRecordSet = recordSets->item(ii);
         XQString recordSetName (currentRecordSet->getNodeName());
         xalanc::XalanNodeList const * records = currentRecordSet->getChildNodes();
         int numberOfRecords = records->getLength();
         qDebug() <<
            Q_FUNC_INFO << "Record set" << ii << ":" << recordSetName << "has" << numberOfRecords << "records";
         if (!RECORD_SET_TO_LOADER_LOOKUP.contains(recordSetName)) {
            //
            // This should only happen if the BeerXML file contains non-standard record sets -- something that is
            // (IMHO incorrectly) technically allowed by the BeerXML 1.0 Standard.
            //
            qWarning() << Q_FUNC_INFO << "Ignoring unrecognised record set: " << recordSetName;
         } else {
            //
            // Process each record in the set
            //
            XPathRecordLoader::Factory const xPathRecordLoaderFactory{RECORD_SET_TO_LOADER_LOOKUP.value(recordSetName)};
            for (int ii = 0; ii < numberOfRecords; ++ii) {
               //
               // In theory each record set only contains (as its immediate children) one type of record...
               // ...but, because the BeerXML 1.0 Standard allows non-standard tags to appear pretty much anywhere
               // in a document, we have to check each child node and skip lightly over anything unrecognised (eg
               // <YEASTS><HUMBUG>...</HUMBUG></YEAST>) or  unexpected (eg <YEASTS><HOP>...</HOP></YEAST>).
               //
               // We've already eliminated the possibility that we hit an unrecognised record set, so it's a
               // programming error if we can't find the XPathRecordLoader the record type we're expecting.
               //
               std::unique_ptr<XPathRecordLoader> xPathRecordLoader{xPathRecordLoaderFactory()};
               Q_ASSERT(nullptr != xPathRecordLoader.get());
               QString const expectedRecordName = xPathRecordLoader->getRecordName();

               xalanc::XalanNode * currentRecord = records->item(ii);
               XQString actualRecordName{currentRecord->getNodeName()};
               if (expectedRecordName != actualRecordName) {
                  qWarning() <<
                     Q_FUNC_INFO << "Ignoring unexpected record " << actualRecordName << " inside record set " <<
                     recordSetName << " (was expecting " << expectedRecordName << ")";
               } else {
                  if (!xPathRecordLoader->load(domSupport, currentRecord, userMessage)) {
                     //
                     // Technically we could just continue on to the next record and try to load that, but, given that we've
                     // already done XSD validation, it's likely that we've got substantive problem if we couldn't load the
                     // record.  So, it's better to break off processing now and show the user the first error we've hit, rather
                     // than potentially produce some enormous list of errors by trying to process the rest of the input file.
                     //
                     return false;
                  }
                  if (!xPathRecordLoader->normalise(userMessage)) {
                     return false;
                  }
                  // TBD: Should distinguish between error and skipped duplicates
                  if (!xPathRecordLoader->storeInDb(userMessage)) {
                     return false;
                  }

                  // To get here, we must have read something in OK!  Add it to the tally
                  int tally = 0;
                  if (importCount.contains(expectedRecordName)) {
                     tally = importCount.value(expectedRecordName);
                  }
                  ++tally;
                  importCount.insert(expectedRecordName, tally);
               }
            }
         }
      }

      // Summarise what we read in into the message displayed on-screen to the user
      userMessage << tr("Read ");
      int typesOfRecordsRead = 0;
      for (auto ii = importCount.constBegin(); ii != importCount.constEnd(); ++ii, ++typesOfRecordsRead) {
         if (0 != typesOfRecordsRead) {
            userMessage << ", ";
         }
         userMessage << ii.value() << " " << ii.key() << (1 == ii.value() ? tr(" record") : " records");
      }

      return true;
   }


private:
   // XMLGrammarPoolImpl is a bit lacking in documentation, probably because it used to be an "internal" class of
   // Xerces.  However, since Xerces 3.0.0 release, it is now part of the public API -- see
   // https://xerces.apache.org/xerces-c/migrate-archive-3.html#NewAPI300
   xercesc::XMLGrammarPoolImpl grammarPool;
   xercesc::DOMImplementation * domImplementation;
   xercesc::DOMLSParser * parser;

   // Note that any change to this needs to be reflected in beerxml/v1/BeerXml.xsd
   constexpr static char const * const INSERTED_ROOT_NODE_NAME = "BEER_XML";

   static QHash<QString, XPathRecordLoader::Factory> RECORD_SET_TO_LOADER_LOOKUP;
};

// In theory, once we know the record set name, we can deduce the name of the records and vice versa (HOPS <-> HOP etc)
// but this is only because BeerXML uses "MASHS" as a mangled plural of "MASH" (instead of "MASHES").  Doing a proper
// mapping keeps things open for, say, possible future improved versions of BeerXML.
//
// And we can't just look at the child node of the record set, as the BeerXML 1.0 Standard allows extra undefined tags
// to be added to a document.
QHash<QString, XPathRecordLoader::Factory> BeerXML::impl::RECORD_SET_TO_LOADER_LOOKUP {
   {"HOPS",          &XPathRecordLoaderFactory<BeerXmlHopRecordLoader>},
   {"FERMENTABLES",  &XPathRecordLoaderFactory<BeerXmlFermentableRecordLoader>},
   {"YEASTS",        &XPathRecordLoaderFactory<BeerXmlYeastRecordLoader>},
   {"MISCS",         nullptr}, //TODO
   {"WATERS",        nullptr}, //TODO
   {"STYLES",        nullptr}, //TODO
   {"MASHS",         nullptr}, //TODO
   {"RECIPES",       nullptr}, //TODO
   {"EQUIPMENTS",    nullptr} //TODO
};


BeerXML::BeerXML(DatabaseSchema* tables) : QObject(),
                                           pimpl{ new impl{} },
                                           m_tables(tables) {
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
BeerXML::~BeerXML() = default;


QString BeerXML::textFromValue(QVariant value, QString type)
{
   QString retval = value.toString();

   if (type == "boolean" )
      retval = Ingredient::text(value.toBool());
   else if (type == "real")
      retval = Ingredient::text(value.toDouble());
   else if (type == "timestamp")
      retval = Ingredient::text(value.toDate());

   return retval;
}

void BeerXML::toXml( BrewNote* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::BREWNOTETABLE);

   node = doc.createElement("BREWNOTE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Equipment* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::EQUIPTABLE);

   node = doc.createElement("EQUIPMENT");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Fermentable* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::FERMTABLE);

   node = doc.createElement("FERMENTABLE");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Hop* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   node = doc.createElement("HOP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Instruction* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::INSTRUCTIONTABLE);

   node = doc.createElement("INSTRUCTION");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Mash* a, QDomDocument& doc, QDomNode& parent )
{

   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   int i, size;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHTABLE);

   node = doc.createElement("MASH");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   tmpElement = doc.createElement("MASH_STEPS");
   QList<MashStep*> mashSteps = a->mashSteps();
   size = mashSteps.size();
   for( i = 0; i < size; ++i )
      toXml( mashSteps[i], doc, tmpElement);
   node.appendChild(tmpElement);

   parent.appendChild(node);

}

void BeerXML::toXml( MashStep* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MASHSTEPTABLE);

   node = doc.createElement("MASH_STEP");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         QString val;
         // flySparge and batchSparge aren't part of the BeerXML spec.
         // This makes sure we give BeerXML something it understands.
         if ( element == kpropType ) {
            if ( (a->type() == MashStep::flySparge) || (a->type() == MashStep::batchSparge ) ) {
               val = MashStep::types[0];
            }
            else {
               val = a->typeString();
            }
         }
         else {
            val = textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element));
         }
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(val);
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Misc* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);

   node = doc.createElement("MISC");

   // This sucks. Not quite sure what to do, but hard code it
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

void BeerXML::toXml( Recipe* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::RECTABLE);

   int i;

   node = doc.createElement("RECIPE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }

   Style* style = a->style();
   if( style != nullptr )
      toXml( style, doc, node);

   tmpElement = doc.createElement("HOPS");
   QList<Hop*> hops = a->hops();
   for( i = 0; i < hops.size(); ++i )
      toXml( hops[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("FERMENTABLES");
   QList<Fermentable*> ferms = a->fermentables();
   for( i = 0; i < ferms.size(); ++i )
      toXml( ferms[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("MISCS");
   QList<Misc*> miscs = a->miscs();
   for( i = 0; i < miscs.size(); ++i )
      toXml( miscs[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("YEASTS");
   QList<Yeast*> yeasts = a->yeasts();
   for( i = 0; i < yeasts.size(); ++i )
      toXml( yeasts[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("WATERS");
   QList<Water*> waters = a->waters();
   for( i = 0; i < waters.size(); ++i )
      toXml( waters[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Mash* mash = a->mash();
   if( mash != nullptr )
      toXml( mash, doc, node);

   tmpElement = doc.createElement("INSTRUCTIONS");
   QList<Instruction*> instructions = a->instructions();
   for( i = 0; i < instructions.size(); ++i )
      toXml( instructions[i], doc, tmpElement);
   node.appendChild(tmpElement);

   tmpElement = doc.createElement("BREWNOTES");
   QList<BrewNote*> brewNotes = a->brewNotes();
   for(i=0; i < brewNotes.size(); ++i)
      toXml(brewNotes[i], doc, tmpElement);
   node.appendChild(tmpElement);

   Equipment* equip = a->equipment();
   if( equip )
      toXml( equip, doc, node);

   parent.appendChild(node);
}

void BeerXML::toXml( Style* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::STYLETABLE);

   node = doc.createElement("STYLE");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);

}

void BeerXML::toXml( Water* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::WATERTABLE);

   node = doc.createElement("WATER");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);


}

void BeerXML::toXml( Yeast* a, QDomDocument& doc, QDomNode& parent )
{
   QDomElement node;
   QDomElement tmpElement;
   QDomText tmpText;
   TableSchema* tbl = m_tables->table(Brewtarget::YEASTTABLE);

   node = doc.createElement("YEAST");

   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(Ingredient::text(a->version()));
   tmpElement.appendChild(tmpText);
   node.appendChild(tmpElement);

   foreach (QString element, tbl->allPropertyNames()) {
      if ( ! tbl->propertyToXml(element).isEmpty() ) {
         tmpElement = doc.createElement(tbl->propertyToXml(element));
         tmpText    = doc.createTextNode(textFromValue(a->property(element.toUtf8().data()), tbl->propertyColumnType(element)));
         tmpElement.appendChild(tmpText);
         node.appendChild(tmpElement);
      }
   }
   parent.appendChild(node);
}

// fromXml ====================================================================
void BeerXML::fromXml(Ingredient* element, QHash<QString,QString> const& xmlTagsToProperties, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         qWarning() << QString("Node at line %1 is not an element.").arg(textNode.lineNumber());
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();

      if( xmlTagsToProperties.contains(xmlTag) )
      {
         switch( element->metaProperty(xmlTagsToProperties[xmlTag]).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(xmlTagsToProperties[xmlTag].toStdString().c_str(), stringVal);
               break;
            default:
               qWarning() << QString("%1: don't understand property type.").arg(Q_FUNC_INFO);
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            qCritical() << QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag);
            return;
         }
      }
   }

}

void BeerXML::fromXml(Ingredient* element, QDomNode const& elementNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString xmlTag;
   int intVal;
   double doubleVal;
   bool boolVal;
   QString stringVal;
   QDateTime dateTimeVal;
   QDate dateVal;
   TableSchema* schema = m_tables->table( element->table() );

   for( node = elementNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         qWarning() << QString("Node at line %1 is not an element.").arg(textNode.lineNumber());
         continue;
      }

      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;

      xmlTag = node.nodeName();
      textNode = child.toText();
      QString pTag = schema->xmlToProperty(xmlTag);

      if( pTag.size() ) {
         switch( element->metaProperty(pTag).type() )
         {
            case QVariant::Bool:
               boolVal = Ingredient::getBool(textNode);
               element->setProperty(pTag.toStdString().c_str(), boolVal);
               break;
            case QVariant::Double:
               doubleVal = Ingredient::getDouble(textNode);
               element->setProperty(pTag.toStdString().c_str(), doubleVal);
               break;
            case QVariant::Int:
               intVal = Ingredient::getInt(textNode);
               element->setProperty(pTag.toStdString().c_str(), intVal);
               break;
            case QVariant::DateTime:
               dateTimeVal = Ingredient::getDateTime(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateTimeVal);
               break;
            case QVariant::Date:
               dateVal = Ingredient::getDate(textNode);
               element->setProperty(pTag.toStdString().c_str(), dateVal);
               break;
               // NOTE: I believe that enum types like Fermentable::Type will go
               // here since Q_ENUMS() converts enums to strings. So, need to make
               // sure that the enums match exactly what we expect in the XML.
            case QVariant::String:
               stringVal = Ingredient::getString(textNode);
               element->setProperty(pTag.toStdString().c_str(), stringVal);
               break;
            default:
               qWarning() << QString("%1: don't understand property type. xmlTag=%2")
                     .arg(Q_FUNC_INFO)
                     .arg(xmlTag);
               break;
         }
         // Not sure if we should keep processing or just dump?
         if ( ! element->isValid() ) {
            qCritical() << QString("%1 could not populate %2 from XML").arg(Q_FUNC_INFO).arg(xmlTag);
            return;
         }
      }
   }
}

// Brewnotes can never be created w/ a recipe, so we will always assume the
// calling method has the transactions
BrewNote* BeerXML::brewNoteFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   BrewNote* ret = nullptr;
   QDateTime theDate;

   n = node.firstChildElement("BREWDATE");
   theDate = QDateTime::fromString(n.firstChild().toText().nodeValue(),Qt::ISODate);

   try {
      ret = new BrewNote(theDate);

      if ( ! ret ) {
         QString error = "Could not create new brewnote.";
         qCritical() << QString(error);
         QMessageBox::critical(nullptr, tr("Import error."),
               error.append("\nUnable to create brew note."));
         return ret;
      }
      // Need to tell the brewnote not to perform the calculations
      ret->setLoading(true);
      fromXml(ret, node);
      ret->setLoading(false);

      ret->setRecipe(parent);
      ret->insertInDatabase();
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }
   return ret;
}

Equipment* BeerXML::equipmentFromXml( QDomNode const& node, Recipe* parent )
{
   // When loading from XML, we need to delay the signals until after
   // everything is done. This should significantly speed up the load times

   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Equipment* ret;
   QString name;
   QList<Equipment*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing an equip by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is an equip already in the DB with the same name.
         db.getElementsByName<Equipment>( matching, Brewtarget::EQUIPTABLE, name, db.allEquipments);

         // If we find a match, use it
         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Equipment(name,true);
         }
      }
      else {
         ret = new Equipment(name,true);
      }

      if ( createdNew ) {
         fromXml(ret, node);
         if ( ! ret->isValid() )
            throw QString("There was an error loading equipment profile from XML");

         // If we are importing one of our beerXML files, the utilization is always
         // 0%. We need to fix that.
         if ( ret->hopUtilization_pct() == 0.0 )
            ret->setHopUtilization_pct(100.0);

         ret->insertInDatabase();
      }

      if( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      throw;
   }

   blockSignals(false);


   if ( createdNew ) {
      emit db.changed( db.metaProperty("equipments"), QVariant() );
      emit db.newEquipmentSignal(ret);
   }

   return ret;
}

Fermentable* BeerXML::fermentableFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Fermentable* ret = nullptr;
   QString name;
   QList<Fermentable*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a ferm by itself, need to do some dupe-checking.
      if ( parent == nullptr )
      {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if we already have a Fermentable with this name
         db.getElementsByName<Fermentable>( matching,
               Brewtarget::FERMTABLE,
               name, db.allFermentables);

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Fermentable(name);
         }
      }
      else {
         ret = new Fermentable(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() )
            throw QString("Error reading fermentable from XML");


         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = Fermentable::types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType( static_cast<Fermentable::Type>(ndx));
            else
               ret->invalidate();
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::fermentableFromXml: Could convert a recognized type");
         }
         ret->insertInDatabase();

      }

      if ( parent ) {
         ret->setDisplay(false);
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      if ( parent == nullptr ) {
         db.sqlDatabase().rollback();
      }
      qCritical () << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
   }

   if ( parent == nullptr ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("fermentables"), QVariant() );
      emit db.newFermentableSignal(ret);
   }

   return ret;
}

int BeerXML::getQualifiedHopTypeIndex(QString type, Hop* hop)
{
   Database & db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::types.indexOf(type) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      // Check to see if there is an hop already in the DB with the same name.
      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString htype = q.record().value(0).toString();
            q.finish();
            if ( htype != "" &&  Hop::types.indexOf(htype) >= 0 ) {
               return Hop::types.indexOf(htype);
            }
         }
      }
      // out of ideas at this point so default to Both
      return Hop::types.indexOf(QString("Both"));
   }
   else {
      return Hop::types.indexOf(type);
   }
}

int BeerXML::getQualifiedHopUseIndex(QString use, Hop* hop)
{
   Database & db = Database::instance();
   TableSchema* tbl = m_tables->table(Brewtarget::HOPTABLE);

   if ( Hop::uses.indexOf(use) < 0 ) {
      // look for a valid hop type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropUse))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));

      QSqlQuery q(db.sqlDatabase());
      q.prepare(query);
      q.bindValue(":name", hop->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString hUse = q.record().value(0).toString();
            q.finish();
            if ( hUse != "" &&  Hop::uses.indexOf(hUse) >= 0 ) {
               return Hop::uses.indexOf(hUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Hop::uses.indexOf(QString("Flavor"));
   }
   else {
      return Hop::uses.indexOf(use);
   }
}

Hop* BeerXML::hopFromXml( QDomNode const& node, Recipe* parent )
{
   Database & db = Database::instance();
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Hop* ret;
   QString name;
   QList<Hop*> matching;

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a hop by itself, need to do some dupe-checking.
      if( parent == nullptr )
      {
         // as always, start the transaction if no parent
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Hop>( matching,
               Brewtarget::HOPTABLE,
               name,
               db.allHops);

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Hop(name);
         }
      }
      else {
         ret = new Hop(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );
         if ( ! ret->isValid() ) {
            throw QString("Error reading Hop from XML");
         }

         // Handle enums separately.
         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() )
            ret->invalidate();
         else {
            int ndx = getQualifiedHopUseIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setUse( static_cast<Hop::Use>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = getQualifiedHopTypeIndex(n.firstChild().toText().nodeValue(), ret);
            if ( ndx != -1 ) {
               ret->setType( static_cast<Hop::Type>(ndx) );
            }
            else {
               ret->invalidate();
            }
         }

         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Hop::forms.indexOf(n.firstChild().toText().nodeValue());
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Hop::Form>(ndx));
            }
            else {
               ret->invalidate();
            }
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::hopFromXml: Could convert %1 to a recognized type");
         }
         ret->insertInDatabase();
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      if ( ! parent )
         db.sqlDatabase().rollback();

      abort();
   }

   if ( ! parent ) {
      db.sqlDatabase().commit();
   }

   blockSignals(false);

   if( createdNew ) {
      emit db.changed( db.metaProperty("hops"), QVariant() );
      emit db.newHopSignal(ret);
   }

   return ret;
}

// like brewnotes, we will assume here that the caller has the transaction
// block
Instruction* BeerXML::instructionFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   QString name;
   Instruction* ret;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   blockSignals(true);
   try {
      ret = new Instruction(name);

      fromXml(ret, node);

      ret->setRecipe(parent);
      ret->insertInDatabase();

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      throw;
   }

   blockSignals(false);
   emit db.changed( db.metaProperty("instructions"), QVariant() );
   return ret;
}

Mash* BeerXML::mashFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   Mash* ret;
   QString name;
   QList<Mash*> matching;
   Database & db = Database::instance();

   blockSignals(true);

   // Mashes are weird. We need to know if this is a duplicate, but we need to
   // make a copy of it anyway.
   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();

   try {
      ret = new Mash(name);

      // If the mash has a name
      if ( ! name.isEmpty() ) {
         db.getElementsByName<Mash>( matching, Brewtarget::MASHTABLE, name, db.allMashs );

         // If there are no other matches in the database
         if( matching.isEmpty() ) {
            ret->setDisplay(true);
         }
      }
      // First, get all the standard properties.
      fromXml( ret, node );

      //Need to insert the Mash before the Mash steps to get
      //the ID for foreign key contraint in Maststep table.
      db.sqlDatabase().transaction();
      ret->insertInDatabase();

      // Now, get the individual mash steps.
      n = node.firstChildElement("MASH_STEPS");
      if( ! n.isNull() ) {
         // Iterate through all the mash steps.
         for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         {
            MashStep* temp = mashStepFromXml( n, ret );
            if ( ! temp->isValid() ) {
               QString error = QString("Error importing mash step %1. Importing as infusion").arg(temp->name());
               qCritical() << error;
            }
         }
      }

      if ( parent ) {
         db.addToRecipe( parent, ret, true, false);
      }

   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      blockSignals(false);
      db.sqlDatabase().rollback();
      abort();
   }

   db.sqlDatabase().commit();

   blockSignals(false);

   emit db.changed( db.metaProperty("mashs"), QVariant() );
   emit db.newMashSignal(ret);
   emit ret->mashStepsChanged();

   return ret;
}

// mashsteps don't exist without a mash. It is up to mashFromXml or
// recipeFromXml to deal with the transaction
MashStep* BeerXML::mashStepFromXml( QDomNode const& node, Mash* parent )
{
   QDomNode n;
   QString str;
   Database & db = Database::instance();

   try {
      MashStep* ret = new MashStep(true);

      fromXml(ret,node);

      // Handle enums separately.
      n = node.firstChildElement("TYPE");
      if ( n.firstChild().isNull() )
         ret->invalidate();
      else {
         //Try to make sure incoming format matches
         //e.g. convert INFUSION to Infusion
         str = n.firstChild().toText().nodeValue();
         str = str.toLower();
         str[0] = str.at(0).toTitleCase();
         int ndx =  MashStep::types.indexOf(str);

         if ( ndx != -1 )
            ret->setType( static_cast<MashStep::Type>(ndx) );
         else
            ret->invalidate();
      }

      ret->setMash(parent);
      ret->insertInDatabase();

      if (! signalsBlocked() )
      {
         emit db.changed( db.metaProperty("mashs"), QVariant() );
         emit parent->mashStepsChanged();
      }
      return ret;
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      abort();
   }

}

int BeerXML::getQualifiedMiscTypeIndex(QString type, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database & db = Database::instance();

   if ( Misc::types.indexOf(type) < 0 ) {
      // look for a valid mash type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:name AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropName));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() )
         {
            QString mtype = q.record().value(0).toString();
            q.finish();
            if ( mtype != "" &&  Misc::types.indexOf(mtype) >= 0 ) {
               return Misc::types.indexOf(mtype);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::types.indexOf(QString("Flavor"));
   }
   else {
      return Misc::types.indexOf(type);
   }
}

int BeerXML::getQualifiedMiscUseIndex(QString use, Misc* misc)
{
   TableSchema* tbl = m_tables->table(Brewtarget::MISCTABLE);
   Database & db = Database::instance();

   if ( Misc::uses.indexOf(use) < 0 ) {
      // look for a valid misc type from our database to use
      QString query = QString("SELECT %1 FROM %2 WHERE %3=:use AND %1 != ''")
         .arg(tbl->propertyToColumn(kpropType))
         .arg(tbl->tableName())
         .arg(tbl->propertyToColumn(kpropUse));
      QSqlQuery q(db.sqlDatabase());

      q.prepare(query);
      q.bindValue(":name", misc->name());

      if ( q.exec() ) {
         q.first();
         if ( q.isValid() ) {
            QString mUse = q.record().value(0).toString();
            q.finish();
            if ( mUse != "" &&  Misc::uses.indexOf(mUse) >= 0 ) {
               return Misc::uses.indexOf(mUse);
            }
         }
      }
      // out of ideas at this point so default to Flavor
      return Misc::uses.indexOf(QString("Flavor"));
   }
   else {
      return Misc::uses.indexOf(use);
   }
}

Misc* BeerXML::miscFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Misc* ret;
   QString name;
   QList<Misc*> matching;

   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a misc by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         // Check to see if there is a hop already in the DB with the same name.
         db.sqlDatabase().transaction();

         db.getElementsByName<Misc>( matching, Brewtarget::MISCTABLE, name, db.allMiscs );

         if( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Misc(name);
         }
      }
      else {
         ret = new Misc(name);
      }

      if ( createdNew ) {

         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         // Assuming these return anything is a bad idea. So far, several other brewing programs are not generating
         // valid XML.
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setType( static_cast<Misc::Type>(getQualifiedMiscTypeIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         n = node.firstChildElement("USE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            ret->setUse(static_cast<Misc::Use>(getQualifiedMiscUseIndex(n.firstChild().toText().nodeValue(), ret)));
         }

         if ( ! ret->isValid() ) {
            qWarning() << QString("BeerXML::miscFromXml: Could convert %1 to a recognized type");
         }
         ret->insertInDatabase();
      }

      if( parent )
         db.addToRecipe( parent, ret, true );
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("miscs"), QVariant() );
      emit db.newMiscSignal(ret);
   }
   return ret;
}

Recipe* BeerXML::recipeFromXml( QDomNode const& node )
{
   QDomNode n;
   blockSignals(true);
   Recipe *ret;
   QString name;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {

      // This is all one long, gnarly transaction.
      db.sqlDatabase().transaction();

      // Oh sweet mercy
      ret = new Recipe(name);

      if ( ! ret ) {
         return nullptr;
      }
      // Get standard properties.
      fromXml(ret, node);

      // I need to insert this now, because I need a valid key for adding
      // things to the recipe
      ret->insertInDatabase();

      // Get style. Note: styleFromXml requires the entire node, not just the
      // firstchild of the node.
      n = node.firstChildElement("STYLE");
      styleFromXml(n, ret);
      if ( ! ret->style()->isValid())
         ret->invalidate();

      // Get equipment. equipmentFromXml requires the entire node, not just the
      // first child
      n = node.firstChildElement("EQUIPMENT");
      equipmentFromXml(n, ret);
      if ( !ret->equipment()->isValid() )
         ret->invalidate();

      // Get hops.
      n = node.firstChildElement("HOPS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Hop* temp = hopFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // Get ferms.
      n = node.firstChildElement("FERMENTABLES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Fermentable* temp = fermentableFromXml(n, ret);
         if ( ! temp->isValid() )
            ret->invalidate();
      }

      // get mashes. There is only one mash per recipe, so this needs the entire
      // node.
      n = node.firstChildElement("MASH");
      mashFromXml(n, ret);
      if ( ! ret->mash()->isValid() )
         ret->invalidate();

      // Get miscs.
      n = node.firstChildElement("MISCS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Misc* temp = miscFromXml(n, ret);
         if (! temp->isValid())
            ret->invalidate();
      }

      // Get yeasts.
      n = node.firstChildElement("YEASTS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
      {
         Yeast* temp = yeastFromXml(n, ret);
         if ( !temp->isValid() )
            ret->invalidate();
      }

      // Get waters. Odd. Waters don't invalidate.
      n = node.firstChildElement("WATERS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         waterFromXml(n, ret);

      /* That ends the beerXML defined objects. I'm not going to do the
       * validation for these last two. We write em, and we had better be
       * writing them properly
       */
      // Get instructions.
      n = node.firstChildElement("INSTRUCTIONS");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         instructionFromXml(n, ret);

      // Get brew notes
      n = node.firstChildElement("BREWNOTES");
      for( n = n.firstChild(); !n.isNull(); n = n.nextSibling() )
         brewNoteFromXml(n, ret);

      // If we get here, commit
      db.sqlDatabase().commit();

      // Recalc everything, just for grins and giggles.
      ret->recalcAll();
      blockSignals(false);
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   emit db.newRecipeSignal(ret);
   return ret;
}

Style* BeerXML::styleFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   bool createdNew = true;
   blockSignals(true);
   Style* ret;
   QString name;
   QList<Style*> matching;

   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // No parent means we handle the transaction
         db.sqlDatabase().transaction();
         // Check to see if there is a style already in the DB with the same name.
         db.getElementsByName<Style>( matching, Brewtarget::STYLETABLE, name, db.allStyles );

         // If we found a match, use it.
         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            // We could find no matching style in the db
            ret = new Style(name,true);
         }
      }
      else {
         // If we are inserting this as part of a recipe, we can skip straight
         // to creating a new one
         ret = new Style(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            ret->invalidate();
         }
         else {
            int ndx = Style::m_types.indexOf( n.firstChild().toText().nodeValue());
            if ( ndx != -1 )
               ret->setType(static_cast<Style::Type>(ndx));
            else
               ret->invalidate();
         }

         // If translating the enums craps out, give a warning
         if (! ret->isValid() ) {
            qWarning() << QString("BeerXML::styleFromXml: Could convert %1 to a recognized type");
         }
         // we need to poke this into the database
         ret->insertInDatabase();
      }
      if ( parent ) {
         db.addToRecipe( parent, ret, true );
      }
   }
   catch (QString e) {
      qCritical() << QString("%1 %2").arg(Q_FUNC_INFO).arg(e);
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if ( createdNew ) {
      emit db.changed( db.metaProperty("styles"), QVariant() );
      emit db.newStyleSignal(ret);
   }

   return ret;
}

Water* BeerXML::waterFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Water* ret;
   QString name;
   QList<Water*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a style by itself, need to do some dupe-checking.
      if( parent == nullptr ) {
         db.sqlDatabase().transaction();
         // Check to see if there is a hop already in the DB with the same name.
         db.getElementsByName<Water>( matching, Brewtarget::WATERTABLE, name, db.allWaters );

         if( matching.length() > 0 )
         {
            createdNew = false;
            ret = matching.first();
         }
         else
            ret = new Water(name);
      }
      else
         ret = new Water(name);

      if ( createdNew ) {
         fromXml( ret, node );
         ret->insertInDatabase();
         if( parent ) {
            db.addToRecipe( parent, ret, false );
         }
      }
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO << e;
      if ( parent == nullptr )
         db.sqlDatabase().rollback();
      blockSignals(false);
      abort();
   }

   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("waters"), QVariant() );
      emit db.newWaterSignal(ret);
   }

   return ret;
}

Yeast* BeerXML::yeastFromXml( QDomNode const& node, Recipe* parent )
{
   QDomNode n;
   blockSignals(true);
   bool createdNew = true;
   Yeast* ret;
   QString name;
   QList<Yeast*> matching;
   Database & db = Database::instance();

   n = node.firstChildElement("NAME");
   name = n.firstChild().toText().nodeValue();
   try {
      // If we are just importing a yeast by itself, need to do some dupe-checking.
      if ( parent == nullptr ) {
         // start the transaction, just in case
         db.sqlDatabase().transaction();
         // Check to see if there is a yeast already in the DB with the same name.
         db.getElementsByName<Yeast>( matching, Brewtarget::YEASTTABLE, name, db.allYeasts );

         if ( matching.length() > 0 ) {
            createdNew = false;
            ret = matching.first();
         }
         else {
            ret = new Yeast(name);
         }
      }
      else {
         ret = new Yeast(name);
      }

      if ( createdNew ) {
         fromXml( ret, node );

         // Handle type enums separately.
         n = node.firstChildElement("TYPE");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find TYPE in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::types.indexOf( tname );
            if ( ndx != -1) {
               ret->setType( static_cast<Yeast::Type>(ndx) );
            }
            else {
               ret->setType(static_cast<Yeast::Type>(0));
               qCritical() <<
                     QString("Could not translate the type %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }
         // Handle form enums separately.
         n = node.firstChildElement("FORM");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find FORM in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::forms.indexOf( tname );
            if ( ndx != -1 ) {
               ret->setForm( static_cast<Yeast::Form>(ndx) );
            }
            else {
               ret->setForm( static_cast<Yeast::Form>(0) );
               qCritical() <<
                     QString("Could not translate the form %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }
         // Handle flocc enums separately.
         n = node.firstChildElement("FLOCCULATION");
         if ( n.firstChild().isNull() ) {
            qCritical() << QString("Could not find FLOCCULATION in %1.  Please select an appropriate value once the yeast is imported").arg(name);
         }
         else {
            QString tname = n.firstChild().toText().nodeValue();
            int ndx = Yeast::flocculations.indexOf( tname );
            if (ndx != -1) {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(ndx) );
            }
            else {
               ret->setFlocculation( static_cast<Yeast::Flocculation>(0) );
               qCritical() <<
                     QString("Could not translate the flocculation %1 in %2.  Please select an appropriate value once the yeast is imported")
                     .arg(tname)
                     .arg(name);
            }
         }

         ret->insertInDatabase();
      }

      if( parent ) {
         // we are in a transaction boundary, so tell addToRecipe not to start
         // another
         db.addToRecipe( parent, ret, false );
         parent->recalcOgFg();
         parent->recalcABV_pct();
      }
   }
   catch (QString e) {
      qCritical() << Q_FUNC_INFO<< e;
      if ( ! parent )
         db.sqlDatabase().rollback();
      blockSignals(false);
      throw;
   }

   db.sqlDatabase().commit();
   blockSignals(false);
   if( createdNew )
   {
      emit db.changed( db.metaProperty("yeasts"), QVariant() );
      emit db.newYeastSignal(ret);
   }

   return ret;
}


bool BeerXML::importFromXML(QString const & filename, QTextStream & userMessage)
{
   //
   // Call the new code that attempts to validate the file before reading in its data
   //
   return this->pimpl->validateAndLoad(filename, userMessage);

   /////////////////////////////^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^!!!!!!!!!!!!!!!!!!1
//
// STOP PROCESSING HERE .  SHOULD ULTIMATELY BE ABLE TO DELETE THE REST OF THIS CODE
/////////////////////////

   int count;
   int line, col;
   QDomDocument xmlDoc;
   QDomElement root;
   QDomNodeList list;
   QString err;
   QFile inFile;
   QStringList tags = QStringList() << "EQUIPMENT" << "FERMENTABLE" << "HOP" << "MISC" << "STYLE" << "YEAST" << "WATER" << "MASHS";
   inFile.setFileName(filename);
   bool ret = true;

   if( ! inFile.open(QIODevice::ReadOnly) )
   {
      qWarning() << Q_FUNC_INFO << "Could not open " << filename << " for reading";
      return false;
   }

   if( ! xmlDoc.setContent(&inFile, false, &err, &line, &col) )
      qWarning() << Q_FUNC_INFO << ": Bad document formatting in " << filename << " " << line << ":" << col << ". " << err;

   list = xmlDoc.elementsByTagName("RECIPE");
   if ( list.count() )
   {
      for(int i = 0; i < list.count(); ++i )
      {
         Recipe* temp = this->recipeFromXml( list.at(i) );
         if ( ! temp || ! temp->isValid() )
            ret = false;
      }
   }
   else
   {
      foreach (QString tag, tags)
      {
         list = xmlDoc.elementsByTagName(tag);
         count = list.size();

         if ( count > 0 )
         {
            // Tell how many there were in the status bar.
            //statusBar()->showMessage( tr("Found %1 %2.").arg(count).arg(tag.toLower()), 5000 );

            if (tag == "RECIPE")
            {
            }
            else if ( tag == "EQUIPMENT" )
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Equipment* temp = this->equipmentFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "FERMENTABLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Fermentable* temp = this->fermentableFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }

            }
            else if (tag == "HOP")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Hop* temp = this->hopFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "MISC")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Misc* temp = this->miscFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "STYLE" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Style* temp = this->styleFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if (tag == "YEAST")
            {
               for(int i = 0; i < list.count(); ++i )
               {
                  Yeast* temp = this->yeastFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "WATER" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Water* temp = this->waterFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
            else if( tag == "MASHS" )
            {
               for( int i = 0; i < list.count(); ++i )
               {
                  Mash* temp = this->mashFromXml( list.at(i) );
                  if ( ! temp->isValid() )
                     ret = false;
               }
            }
         }
      }
   }
   return ret;
}
