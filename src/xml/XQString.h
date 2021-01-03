/*
 * xml/XQString.h is part of Brewtarget, and is Copyright the following
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

#ifndef _XML_XQSTRING_H
#define _XML_XQSTRING_H
#pragma once

#include <QString>

#include <xercesc/util/Xerces_autoconf_config.hpp>
#include <xalanc/PlatformSupport/PlatformSupportDefinitions.hpp>
#include <xalanc/XalanDOM/XalanDOMString.hpp>

/**
 * \brief This class extends QString to allow you to construct Qt QStrings from Xerces or Xalan strings without having
 *        to do lots of reinterpret_cast.
 *
 * Xerces and Qt both represent strings internally using UTF-16.  However, Xerces uses a base type (uint16_t) to define
 * its 16-bit chars (XMLCh), whereas Qt uses a small class (QChar) for the same data.  So we need a reinterpret_cast to
 * switch between the two.  This is preferable to using the Xerces built-in string handling, which is a bit clunky: you
 * either have to create fixed-sized arrays of XMLCh and call xercesc::XMLString::transcode() to fill them from a
 * char * source, or manually declare constant strings as null-terminated arrays, eg
 *   XMLCh features[] {xercesc::chLatin_L, xercesc::chLatin_S, xercesc::chNull};
 *
 * Xalan works pretty much the same way as Xerces, but has its own definitions (XalanDOMChar is the same as XMLCh) and
 * a proper string class (xalanc::XalanDOMString).
 */
class XQString : public QString {
public:
   // Ensure access to QString's existing constructors, even though we add another one below
   using QString::QString;

   /**
    * Construct from a Xerces null-terminated UTF-16 XMLCh string
    */
   XQString(XMLCh const * xercesString) : QString(reinterpret_cast<QChar const *>(xercesString)) {
      return;
   }

   /**
    * Construct from a Xalan string
    */
   XQString(xalanc::XalanDOMString const & xalanString) : QString(reinterpret_cast<QChar const *>(xalanString.data())) {
      return;
   }

   /**
    * Return a pointer to a Xerces-friendly null-terminated UTF-16 string
    */
   XMLCh const * getXercesString() const {
      // NB we need to use QString::data() rather than QString::constData() to guaranteed that the returned string is
      // null-terminated.
      return reinterpret_cast<XMLCh const *>(this->data());
   }

   xalanc::XalanDOMChar const * getXalanString() const { return reinterpret_cast<xalanc::XalanDOMChar const *>(this->data());}
};

#endif
