/*
 * xml/BtDomDocumentOwner.h is part of Brewtarget, and is Copyright the following
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
#ifndef _XML_BTDOMDOCUMENTOWNER_H
#define _XML_BTDOMDOCUMENTOWNER_H
#pragma once

#include <xercesc/dom/DOMDocument.hpp>

/**
 * An RAII holder for xercesc::DOMDocument
 *
 * If you have told Xerces that you are going to own a DOMDocument that it creates for you, then you need to call release() on that
 * document when you are done with it.  This class does that for you automatically in its destructor.
 */
class BtDomDocumentOwner {
public:
   BtDomDocumentOwner(xercesc::DOMDocument * domDocument) : domDocument(domDocument) { return; }
   ~BtDomDocumentOwner() {
      if (nullptr != this->domDocument) {
         this->domDocument->release();
         // AFAICT our responsibility is only to call release on the xercesc::DOMDocument, not to call its destructor.
         // Having called release(), we should no longer access the object.
         //
         // (If we were required to delete the object then we'd take a different approach of using a std::unique_ptr (or
         // std::shared_ptr) with a custom deleter.)
         this->domDocument = nullptr;
      }
      return;
   }

   xercesc::DOMDocument * getDomDocument() {
      return this->domDocument;
   }
private:
   xercesc::DOMDocument * domDocument;
};
#endif
