/*
 * xml/BtDomErrorHandler.h is part of Brewtarget, and is Copyright the following
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

#ifndef _XML_BTDOMERRORHANDLER_H
#define _XML_BTDOMERRORHANDLER_H
#pragma once

class QString;

#include <memory> // For std::unique_ptr for PImpl
#include <utility> // For std:pair

#include <QVector>

#include <xercesc/dom/DOMErrorHandler.hpp>

/**
 * Although some Xerces errors generate exceptions, others are handled through a callback to an object you provide
 * which needs to implement the xercesc::DOMErrorHandler interface.
 *
 * Aside from "just" logging errors passed to us we need to:
 *  - decide whether the error is one we can safely deal with (including by ignoring!) or whether it should prevent
 *    further processing of the document,
 *  - apply any "corrections" needed the location of the error, which are required when we have made temporary
 *    modifications to the document being parsed (see comments elsewhere for why we would want to do this)
 */
class BtDomErrorHandler: public xercesc::DOMErrorHandler {
public:
   struct PatternAndReason {
      QString const regExMatchingErrorMessage;
      QString const reasonToIgnore;
   };

   /**
    * \brief Constructor
    *
    * \param errorPatternsToIgnore If not null, this is a list Xerces errors that we may safely ignore.  Specifically
    *                              for each error to ignore, there are two things: a regular expression that will
    *                              match (only) against the message of the error we want to ignore, and a reason why we
    *                              are ignoring this error (so we can log "Ignored error X for reason Y".
    *
    * \param numberOfLinesInserted  If we have (post-reading in but pre-parsing) inserted a block of text other than at
    *                               the end of the document, this says how many lines we inserted.  Default is 0.
    * \param lineAfterWhichInserted If numberOfLinesInserted is not 0 then this says at which point in the document the
    *                               insertion was made.
    */
   BtDomErrorHandler(QVector<PatternAndReason> const * errorPatternsToIgnore = nullptr,
                     unsigned int numberOfLinesInserted = 0,
                     unsigned int lineAfterWhichInserted = 0);

   ~BtDomErrorHandler();

   bool failed() const;

   void reset();

   /**
    * This is intended to return something suitable for showing to the user on the screen (probably with the advice to
    * look in the log file for more detailed info).
    */
   QString getlastError();

   /**
    * Adjusts the location of an error to take account of any insertions we made to the file after reading it in (but
    * before parsing).  See comments elsewhere for _why_ we want to make such insertions.  Note that we assume the
    * insertions themselves will never cause an error!
    */
   unsigned int correctErrorLine(unsigned int lineNumberOfError);

   /**
    * If the handleError method returns true the DOM implementation should continue as if the error didn't happen when
    * possible, if the method returns false then the DOM implementation should stop the current processing when possible.
    */
   virtual bool handleError(xercesc::DOMError const & domError);

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as no need for people to make copies (and keeps PImpl implementation simpler)
   BtDomErrorHandler(BtDomErrorHandler const&) = delete;
   //! No assignment operator, as no need for people to make copies (and keeps PImpl implementation simpler)
   BtDomErrorHandler& operator=(BtDomErrorHandler const&) = delete;

};


#endif
