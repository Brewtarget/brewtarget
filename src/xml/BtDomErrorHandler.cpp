/*
 * xml/BtDomErrorHandler.cpp is part of Brewtarget, and is Copyright the following
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

#include "xml/BtDomErrorHandler.h"

#include <QDebug>
#include <QString>

#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMError.hpp>

#include "xml/XQString.h"

// This private implementation class holds all private non-virtual members of BtDomErrorHandler
class BtDomErrorHandler::impl {
public:

   /**
    * Constructor
    */
   impl(QVector<BtDomErrorHandler::PatternAndReason> const * errorPatternsToIgnore,
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
   QVector<BtDomErrorHandler::PatternAndReason> const * errorPatternsToIgnore;
   unsigned int numberOfLinesInserted;
   unsigned int lineAfterWhichInserted;

};

constexpr char const * const BtDomErrorHandler::impl::XercesErrorSeverities[] {
   "Not Used",
   "Warning",     // DOM_SEVERITY_WARNING = 1
   "Error",       // DOM_SEVERITY_ERROR = 2
   "Fatal Error"  // DOM_SEVERITY_FATAL_ERROR = 3
};

BtDomErrorHandler::BtDomErrorHandler(QVector<BtDomErrorHandler::PatternAndReason> const * errorPatternsToIgnore,
                                     unsigned int numberOfLinesInserted,
                                     unsigned int lineAfterWhichInserted) :
   pimpl{ new impl{errorPatternsToIgnore,
                   numberOfLinesInserted,
                   lineAfterWhichInserted} } {
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
BtDomErrorHandler::~BtDomErrorHandler() = default;


bool BtDomErrorHandler::failed() const {
   return this->pimpl->couldntHandleError;
}

void BtDomErrorHandler::reset() {
   this->pimpl->couldntHandleError = false;
   return;
}

QString BtDomErrorHandler::getlastError() {
   return this->pimpl->lastError;
}

unsigned int BtDomErrorHandler::correctErrorLine(unsigned int lineNumberOfError) {
   if (this->pimpl->numberOfLinesInserted > 0 &&
         lineNumberOfError > (this->pimpl->lineAfterWhichInserted + this->pimpl->numberOfLinesInserted)) {
      qDebug() <<
         Q_FUNC_INFO << "Removing " << this->pimpl->numberOfLinesInserted << " from raw line number of error ("<<
         lineNumberOfError << ")";
      return lineNumberOfError - this->pimpl->numberOfLinesInserted;
   }

   return lineNumberOfError;
}

bool BtDomErrorHandler::handleError(xercesc::DOMError const & domError) {
   //
   // Although they are often reasonably clear and straightforward, there can sometimes be a bit of an art to
   // decrypting Xerces error messages...
   //
   // Eg "no declaration found for element" for the first tag in an XML document can (but does not necessarily) mean
   // that Xerces could not find the schema to validate the document against (see
   // https://www.codesynthesis.com/pipermail/xsd-users/2009-February/002217.html and section 2.1 of
   // http://wiki.codesynthesis.com/Tree/FAQ).  It could also mean you've turned off namespace processing (which
   // breaks schema validation) or just that the first element in the document isn't specified in the schema.
   //
   // Nonetheless, even just knowing the first point in the document where there was a problem with parsing is
   // usually pretty helpful.
   //
   // Here we create two versions of the error message - "short" is (potentially) to show on the screen (unless we
   // deduce below we can ignore it) and "full" is for the log file
   //
   QString shortErrorMessage;
   QTextStream shortErrorMessageAsTextStream(&shortErrorMessage);
   xercesc::DOMLocator* location {domError.getLocation()};
   XQString message{domError.getMessage()};
   shortErrorMessageAsTextStream <<
      impl::XercesErrorSeverities[domError.getSeverity()] <<
      " at line " << this->correctErrorLine(location->getLineNumber()) <<
      ", column " << location->getColumnNumber() <<
      ": " << message;

   QString fullErrorMessage;
   QTextStream fullErrorMessageAsTextStream(&fullErrorMessage);
   fullErrorMessageAsTextStream << XQString(location->getURI()) << ": " << shortErrorMessage;

   //
   // Check whether the error we just hit is one we can actually ignore
   //
   if (nullptr != this->pimpl->errorPatternsToIgnore) {
      for (auto ii = this->pimpl->errorPatternsToIgnore->cbegin(); ii != this->pimpl->errorPatternsToIgnore->cend(); ++ii) {
         QRegExp pattern(ii->regExMatchingErrorMessage);
         if (pattern.indexIn(message) != -1) {
            // We want to force the parse error onto a separate line, as it will be quite long, hence
            // ".noquote()" here.
            qWarning().noquote() <<
               "IGNORING the following parse error because" << ii->reasonToIgnore << ":\n   " << fullErrorMessage;
            return true;
         }
      }
   }

   //
   // Other errors get logged as such and cause us to stop processing the document
   //
   qCritical() << fullErrorMessage;
   this->pimpl->lastError = shortErrorMessage;
   this->pimpl->couldntHandleError = true;
   return false;
}
