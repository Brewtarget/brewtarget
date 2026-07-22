/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlErrorHandler.cpp is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#include "serialization/xml/XmlErrorHandler.h"

#include <QDebug>
#include <QRegularExpression>
#include <QString>


namespace {
   /**
    *
    * @param level An xmlErrorLevel value (as defined in libxml2/libxml/xmlerror.h)
    * @return A string suitable for logging
    */
   QString errorLevelToString(int const level) {
      //
      // The libxml2 documentation differentiates between XML_ERR_ERROR as "Recoverable Error" and XML_ERR_FATAL as
      // "Fatal Error".  However, it is typically not feasible for us to continue trying to read the document after a
      // "Recoverable Error", so we just show "error" instead.
      //
      // We keep the "raw" code in the message in case it helps with diagnosing future issues.
      //
      switch (level) {
         case XML_ERR_NONE   : return QObject::tr("Success"           " (%1)").arg("XML_ERR_NONE"   );
         case XML_ERR_WARNING: return QObject::tr("Warning"           " (%1)").arg("XML_ERR_WARNING");
         case XML_ERR_ERROR  : return QObject::tr("Recoverable Error" " (%1)").arg("XML_ERR_ERROR"  );
         case XML_ERR_FATAL  : return QObject::tr("Fatal Error"       " (%1)").arg("XML_ERR_FATAL"  );
         default             : return QObject::tr("Unrecognised"      " (%1)").arg(level            );
      }
   }

   /**
    *
    * @param domain An xmlErrorDomain value (as defined in libxml2/libxml/xmlerror.h)
    * @return A string suitable for logging
    */
   char const * errorDomainToStringRaw(int const domain) {
      switch (domain) {
         case XML_FROM_NONE       : return "XML_FROM_NONE"        ": Unknown";
         case XML_FROM_PARSER     : return "XML_FROM_PARSER"      ": The XML parser";
         case XML_FROM_TREE       : return "XML_FROM_TREE"        ": The tree module (unused)";
         case XML_FROM_NAMESPACE  : return "XML_FROM_NAMESPACE"   ": The XML Namespace module";
         case XML_FROM_DTD        : return "XML_FROM_DTD"         ": The XML DTD validation with parser context";
         case XML_FROM_HTML       : return "XML_FROM_HTML"        ": The HTML parser";
         case XML_FROM_MEMORY     : return "XML_FROM_MEMORY"      ": The memory allocator (unused)";
         case XML_FROM_OUTPUT     : return "XML_FROM_OUTPUT"      ": The serialization code";
         case XML_FROM_IO         : return "XML_FROM_IO"          ": The Input/Output stack";
         case XML_FROM_FTP        : return "XML_FROM_FTP"         ": The FTP module (unused)";
         case XML_FROM_HTTP       : return "XML_FROM_HTTP"        ": The HTTP module (unused)";
         case XML_FROM_XINCLUDE   : return "XML_FROM_XINCLUDE"    ": The XInclude processing";
         case XML_FROM_XPATH      : return "XML_FROM_XPATH"       ": The XPath module";
         case XML_FROM_XPOINTER   : return "XML_FROM_XPOINTER"    ": The XPointer module";
         case XML_FROM_REGEXP     : return "XML_FROM_REGEXP"      ": The regular expressions module";
         case XML_FROM_DATATYPE   : return "XML_FROM_DATATYPE"    ": The W3C XML Schemas Datatype module";
         case XML_FROM_SCHEMASP   : return "XML_FROM_SCHEMASP"    ": The W3C XML Schemas parser module";
         case XML_FROM_SCHEMASV   : return "XML_FROM_SCHEMASV"    ": The W3C XML Schemas validation module";
         case XML_FROM_RELAXNGP   : return "XML_FROM_RELAXNGP"    ": The Relax-NG parser module";
         case XML_FROM_RELAXNGV   : return "XML_FROM_RELAXNGV"    ": The Relax-NG validator module";
         case XML_FROM_CATALOG    : return "XML_FROM_CATALOG"     ": The Catalog module";
         case XML_FROM_C14N       : return "XML_FROM_C14N"        ": The Canonicalization module";
         case XML_FROM_XSLT       : return "XML_FROM_XSLT"        ": The XSLT engine from libxslt (unused)";
         case XML_FROM_VALID      : return "XML_FROM_VALID"       ": The XML DTD validation with valid context";
         case XML_FROM_CHECK      : return "XML_FROM_CHECK"       ": The error checking module (unused)";
         case XML_FROM_WRITER     : return "XML_FROM_WRITER"      ": The xmlwriter module";
         case XML_FROM_MODULE     : return "XML_FROM_MODULE"      ": The dynamically loaded module module (unused)";
         case XML_FROM_I18N       : return "XML_FROM_I18N"        ": The module handling character conversion (unused)";
         case XML_FROM_SCHEMATRONV: return "XML_FROM_SCHEMATRONV" ": The Schematron validator module";
         case XML_FROM_BUFFER     : return "XML_FROM_BUFFER"      ": The buffers module (unused)";
         case XML_FROM_URI        : return "XML_FROM_URI"         ": The URI module (unused)";
         default                  : return "?"                    ": Undefined!";
      }
   }

   QString errorDomainToString(int const domain) {
      return QString{"%1 = %2"}.arg(domain).arg(errorDomainToStringRaw(domain));
   }

   //
   // TBD: We could do something similar to the above for xmlParserErrors value (also as defined in
   //      libxml2/libxml/xmlerror.h).  However, there are a _lot_ (over 700) values, the vast majority of which are
   //      not documented.  We could just print something friendly for the codes that are documented and/or that we
   //      expect we might hit.  For the moment though, we'll log the raw values and look them up by hand.
   //

}

// This private implementation class holds all private non-virtual members of XmlErrorHandler
class XmlErrorHandler::impl {
public:

   /**
    * Constructor
    */
   impl(QVector<XmlErrorHandler::PatternAndReason> const * errorPatternsToIgnore,
        unsigned int numberOfLinesInserted,
        unsigned int lineAfterWhichInserted) : couldntHandleError(false),
                                               lastError(),
                                               errorPatternsToIgnore(errorPatternsToIgnore),
                                               numberOfLinesInserted(numberOfLinesInserted),
                                               lineAfterWhichInserted(lineAfterWhichInserted) {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // See https://xerces.apache.org/xerces-c/apiDocs-3/classDOMError.html for possible indexes into this array
   static char const * const XercesErrorSeverities[];

   bool couldntHandleError;
   QString lastError;
   QVector<XmlErrorHandler::PatternAndReason> const * errorPatternsToIgnore;
   unsigned int numberOfLinesInserted;
   unsigned int lineAfterWhichInserted;

};

constexpr char const * const XmlErrorHandler::impl::XercesErrorSeverities[] {
   "Not Used",
   "Warning",     // DOM_SEVERITY_WARNING = 1
   "Error",       // DOM_SEVERITY_ERROR = 2
   "Fatal Error"  // DOM_SEVERITY_FATAL_ERROR = 3
};

XmlErrorHandler::XmlErrorHandler(QVector<XmlErrorHandler::PatternAndReason> const * errorPatternsToIgnore,
                                     unsigned int numberOfLinesInserted,
                                     unsigned int lineAfterWhichInserted) :
   pimpl{std::make_unique<impl>(errorPatternsToIgnore, numberOfLinesInserted, lineAfterWhichInserted) } {
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
XmlErrorHandler::~XmlErrorHandler() = default;


bool XmlErrorHandler::failed() const {
   return this->pimpl->couldntHandleError;
}

void XmlErrorHandler::reset() {
   this->pimpl->couldntHandleError = false;
   return;
}

QString XmlErrorHandler::getlastError() {
   return this->pimpl->lastError;
}

unsigned int XmlErrorHandler::correctErrorLine(unsigned int lineNumberOfError) {
   if (this->pimpl->numberOfLinesInserted > 0 &&
         lineNumberOfError > (this->pimpl->lineAfterWhichInserted + this->pimpl->numberOfLinesInserted)) {
      qDebug() <<
         Q_FUNC_INFO << "Removing " << this->pimpl->numberOfLinesInserted << " from raw line number of error ("<<
         lineNumberOfError << ")";
      return lineNumberOfError - this->pimpl->numberOfLinesInserted;
   }

   return lineNumberOfError;
}

void XmlErrorHandler::handleError(xmlError const * error) {
   qWarning() <<
      Q_FUNC_INFO << errorLevelToString(error->level) << errorDomainToString(error->domain);

   //
   // Here we create two versions of the error message - "short" is (potentially) to show on the screen (unless we
   // deduce below we can ignore it) and "full" is for the log file
   //
   QString shortErrorMessage;
   QTextStream shortErrorMessageAsTextStream(&shortErrorMessage);
   shortErrorMessageAsTextStream <<
      errorLevelToString(error->level) << " at line " << this->correctErrorLine(error->line) <<
      // Yes, the column number field really is called "int2"
      ", column " << error->int2 << ": " << error->message;

   QString fullErrorMessage;
   QTextStream fullErrorMessageAsTextStream(&fullErrorMessage);
   fullErrorMessageAsTextStream <<
      error->file << ": " << shortErrorMessage << "(" << error->str1 << ";" << error->str2 << ";" << error->str3 << ")";

   //
   // Check whether the error we just hit is one we can actually ignore
   //
   if (nullptr != this->pimpl->errorPatternsToIgnore) {
      for (auto ii = this->pimpl->errorPatternsToIgnore->cbegin(); ii != this->pimpl->errorPatternsToIgnore->cend(); ++ii) {
         QRegularExpression pattern(ii->regExMatchingErrorMessage);
         QRegularExpressionMatch match = pattern.match(error->message);
         if (match.hasMatch()) {
            // We want to force the parse error onto a separate line, as it will be quite long, hence
            // ".noquote()" here.
            qWarning().noquote() <<
               "IGNORING the following parse error because" << ii->reasonToIgnore << ":\n   " << fullErrorMessage;
            return;
         }
      }
   }

   //
   // Other errors get logged as such and cause us to stop processing the document
   //
   qCritical() << fullErrorMessage;
   this->pimpl->lastError = shortErrorMessage;
   this->pimpl->couldntHandleError = true;
   return;
}

void XmlErrorHandler::xmlStructuredErrorFunc(void * userData, xmlError * error) {
   auto xmlErrorHandler = static_cast<XmlErrorHandler *>(userData);
   xmlErrorHandler->handleError(error);
   return;
}
