/*======================================================================================================================
 * serialization/xml/XmlLibHelpers.cpp is part of Brewtarget, and is copyright the following authors 2020-2026:
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
#include "serialization/xml/XmlLibHelpers.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QTextStream>

#include "XmlErrorHandler.h"

namespace {

   char const * elementTypeToStringRaw(xmlElementType const elementType) {
      switch (elementType) {
         // See https://gnome.pages.gitlab.gnome.org/libxml2/html/tree_8h.html#a6d83aa11b89106b2ec14162b1474c8fc for
         // more details on these.
         //
         // Ignore compiler warnings about XML_DOCB_DOCUMENT_NODE with older versions of libxml2.  It is removed in more
         // recent versions of the library.
         case XML_ELEMENT_NODE      : return "XML_ELEMENT_NODE"       ": An element"                ;
         case XML_ATTRIBUTE_NODE    : return "XML_ATTRIBUTE_NODE"     ": An attribute"              ;
         case XML_TEXT_NODE         : return "XML_TEXT_NODE"          ": A text node"               ;
         case XML_CDATA_SECTION_NODE: return "XML_CDATA_SECTION_NODE" ": A CDATA section"           ;
         case XML_ENTITY_REF_NODE   : return "XML_ENTITY_REF_NODE"    ": An entity reference"       ;
         case XML_ENTITY_NODE       : return "XML_ENTITY_NODE"        ": Unused"                    ;
         case XML_PI_NODE           : return "XML_PI_NODE"            ": A processing instruction"  ;
         case XML_COMMENT_NODE      : return "XML_COMMENT_NODE"       ": A comment"                 ;
         case XML_DOCUMENT_NODE     : return "XML_DOCUMENT_NODE"      ": A document"                ;
         case XML_DOCUMENT_TYPE_NODE: return "XML_DOCUMENT_TYPE_NODE" ": Unused"                    ;
         case XML_DOCUMENT_FRAG_NODE: return "XML_DOCUMENT_FRAG_NODE" ": A document fragment"       ;
         case XML_NOTATION_NODE     : return "XML_NOTATION_NODE"      ": A notation, unused"        ;
         case XML_HTML_DOCUMENT_NODE: return "XML_HTML_DOCUMENT_NODE" ": An HTML document"          ;
         case XML_DTD_NODE          : return "XML_DTD_NODE"           ": A document type definition";
         case XML_ELEMENT_DECL      : return "XML_ELEMENT_DECL"       ": An element declaration"    ;
         case XML_ATTRIBUTE_DECL    : return "XML_ATTRIBUTE_DECL"     ": An attribute declaration"  ;
         case XML_ENTITY_DECL       : return "XML_ENTITY_DECL"        ": An entity declaration"     ;
         case XML_NAMESPACE_DECL    : return "XML_NAMESPACE_DECL"     ": An XPath namespace node"   ;
         case XML_XINCLUDE_START    : return "XML_XINCLUDE_START"     ": An XInclude start marker"  ;
         case XML_XINCLUDE_END      : return "XML_XINCLUDE_END"       ": An XInclude end marker"    ;
      }
      return "Unrecognised!";
   }
}

QString XmlLibHelpers::elementTypeToString(xmlElementType const elementType) {
   return QString{"%1 = %2"}.arg(elementType).arg(elementTypeToStringRaw(elementType));
}


XmlLibHelpers::XPathResult::XPathResult(xmlXPathObject * xPathObject) :
   m_xPathObject{xPathObject} {
   return;
}

XmlLibHelpers::XPathResult::~XPathResult() = default;

std::size_t XmlLibHelpers::XPathResult::numNodes() const {
   return this->m_xPathObject->nodesetval ? this->m_xPathObject->nodesetval->nodeNr : 0;
}

xmlNode * XmlLibHelpers::XPathResult::node(int index) const {
   if (index < 0) {
      // This is a bit glib, but if you think about it, it's correct.  Index of -1 gives the last node, of -2 gives the
      // penultimate one, etc.
      index += this->numNodes();
   }
   Q_ASSERT(index >= 0 && static_cast<std::size_t>(index) < this->numNodes());
   return this->m_xPathObject->nodesetval->nodeTab[index];
}

QString XmlLibHelpers::toQString(xmlChar const * xmlCharString) {
   return QString::fromUtf8(reinterpret_cast<char const *>(xmlCharString));
}

xmlChar const * XmlLibHelpers::asXmlCharString(QByteArray const & qByteArray) {
   return reinterpret_cast<xmlChar const *>(qByteArray.constData());
}

std::vector<xmlNode *> XmlLibHelpers::childNodes(xmlNode const * node) {
   std::vector<xmlNode *> result;
   for (xmlNode * child = node->children; child != nullptr; child = child->next) {
      result.push_back(child);
   }
   return result;
}