/*======================================================================================================================
 * serialization/xml/XmlLibHelpers.h is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#ifndef SERIALIZATION_XML_XMLLIBHELPERS_H
#define SERIALIZATION_XML_XMLLIBHELPERS_H
#pragma once

#include <memory>

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xmlerror.h>
#include <libxml2/libxml/xpath.h>

#include <QByteArray>
#include <QString>
#include <QTextStream>

#include "serialization/xml/XmlErrorHandler.h"
#include "utils/CWrappers.h"

namespace XmlLibHelpers {

   QString elementTypeToString(xmlElementType const elementType);

   /**
    * Wrapper around xmlXPathObject
    *
    * NOTE that, unlike some of our other XML classes, this is NOT an owning wrapper.  The xmlXPathObject that we point
    * to is part of data owned by an xmlDoc struct, so there's nothing for us to free when we go out of scope.
    */
   class XPathResult {
   public:
      explicit XPathResult(xmlXPathObject * xPathObject);
      ~XPathResult();

      std::size_t numNodes() const;

      /**
       * Return the specified node in the set.  index >= 0 counts from start of list; index < 0 counts from end of list
       *
       * Caller's responsibility to establish that numNodes > 0 and |index| < numNodes before calling this
       */
      xmlNode * node(int index) const;

   private:
      xmlXPathObject const * m_xPathObject;
   };

   /**
    *
    * @return
    */
   QString toQString(xmlChar const * xmlCharString);

   /**
    * We don't directly go from QString to xmlChar const * because QString::toUtf8() returns a new QByteArray, so we'd
    * risk that going out of scope, eg if we wrote `xmlChar const * utf8String = someConverter(myQString);`
    *
    * Forcing the caller to explicitly handle the QByteArray hopefully makes it less likely we hit such an error.
    */
   xmlChar const * asXmlCharString(QByteArray const & qByteArray);

   /**
    * Although we can access child nodes directly through libxml2's xmlNode strut, the native storage is a doubly-linked
    * list (via xmlNode::next and xmlNode::prev).  This function gives us a vector of pointers to child nodes.
    *
    * @param node
    * @return
    */
   std::vector<xmlNode *> childNodes(xmlNode const * node);

}

#endif