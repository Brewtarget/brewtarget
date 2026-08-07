/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/xml/XmlErrorHandler.h is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#ifndef SERIALIZATION_XML_XMLERRORHANDLER_H
#define SERIALIZATION_XML_XMLERRORHANDLER_H
#pragma once


#include <memory> // For std::unique_ptr for PImpl
#include <utility> // For std:pair

#include <QVector>

#include <libxml2/libxml/xmlerror.h>

class QString;

/**
 * XML libraries typically handle at least some of their errors through a callback.
 *
 * When we were using Xerces, some errors generated exceptions, but others were handled through a callback to a user
 * object that needed to implement the xercesc::DOMErrorHandler interface.  This was easy, because we could hold other
 * information in the same object.
 *
 * Now that we're using libxml2, things are slightly different.  Errors are written to stderr by default, unless and
 * until you set a callback function, whose signature must match  xmlStructuredErrorFunc in libxml2/libxml/xmlerror.h.
 * Because libxml2 is C rather than C++, we can't have a non-static member function as the callback.  However, when we
 * set the callback, we also give a memory address that gets passed back to us as the first parameter (void * userData)
 * when the callback is invoked.  So, with a bit of casting, we can have a static member function invoke a non-static
 * member function on the stateful object we want to use for error handling.
 *
 * Aside from "just" logging errors passed to us, we need to:
 *  - decide whether the error is one we can safely deal with (including by ignoring!) or whether it should prevent
 *    further processing of the document,
 *  - apply any "corrections" needed the location of the error, which are required when we have made temporary
 *    modifications to the document being parsed (see comments elsewhere for why we would want to do this)
 */
class XmlErrorHandler {
public:
   struct PatternAndReason {
      QString const regExMatchingErrorMessage;
      QString const reasonToIgnore;
   };

   /**
    * \brief Constructor
    *
    * \param errorPatternsToIgnore If not null, this is a list libxml2 errors that we may safely ignore.  Specifically
    *                              for each error to ignore, there are two things: a regular expression that will
    *                              match (only) against the message of the error we want to ignore, and a reason why we
    *                              are ignoring this error (so we can log "Ignored error X for reason Y".
    *
    * \param numberOfLinesInserted  If we have (post-reading in but pre-parsing) inserted a block of text other than at
    *                               the end of the document, this says how many lines we inserted.  Default is 0.
    * \param lineAfterWhichInserted If numberOfLinesInserted is not 0 then this says at which point in the document the
    *                               insertion was made.
    */
   explicit XmlErrorHandler(QVector<PatternAndReason> const * errorPatternsToIgnore = nullptr,
                            unsigned int numberOfLinesInserted = 0,
                            unsigned int lineAfterWhichInserted = 0);

   ~XmlErrorHandler();

   bool failed() const;

   void reset();

   /**
    * This is intended to return something suitable for showing to the user on the screen (probably with the advice to
    * look in the log file for more detailed info).
    */
   QString getlastError();

   /**
    * If the handleError method returns true the DOM implementation should continue as if the error didn't happen when
    * possible, if the method returns false then the DOM implementation should stop the current processing when possible.
    */
   void handleError(xmlError const * error);

   /**
    * Callback function that we pass to libxml2
    *
    * Although libxml2 is written in C, we do not need an `extern "C" { ... }` block here as the library gets passed a
    * function pointer and never needs to know the name of the function (so the C++ name mangling doesn't matter).
    *
    * The signature of this function needs to correspond with xmlStructuredErrorFunc in libxml2/libxml/xmlerror.h.  In
    * older versions of the library, this is:
    *    typedef void(* xmlStructuredErrorFunc) (void *userData, xmlError *error)
    * in newer ones it is:
    *    typedef void(* xmlStructuredErrorFunc) (void *userData, const xmlError *error)
    *
    * @param userData this void pointer should be castable to an instance of XmlErrorHandler
    * @param error
    */
#if LIBXML_VERSION < 21200
   static void xmlStructuredErrorFunc(void * userData, xmlError * error);
#else
   static void xmlStructuredErrorFunc(void * userData, xmlError const * error);
#endif

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   //! No copy constructor, as no need for people to make copies (and keeps PImpl implementation simpler)
   XmlErrorHandler(XmlErrorHandler const&) = delete;
   //! No assignment operator, as no need for people to make copies (and keeps PImpl implementation simpler)
   XmlErrorHandler& operator=(XmlErrorHandler const&) = delete;

};


#endif