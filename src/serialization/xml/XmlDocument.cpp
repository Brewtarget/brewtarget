/*======================================================================================================================
 * serialization/xml/XmlDocument.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "serialization/xml/XmlDocument.h"

#include <QDebug>

XmlDocument::XmlDocument(QByteArray const & documentData,
                         QString const & fileName) :
   m_fileName{fileName} {
   //
   // For the final parameter is a combination of xmlParserOption flags:
   //    XML_PARSE_NONET     = Disable network access with the built-in HTTP or FTP clients.  Serves no purpose after 2.15.0
   //    XML_PARSE_NO_XXE    = Disables loading of external DTDs or entities. NB: Can't use until we're on 2.13.0 or later
   //    XML_PARSE_BIG_LINES = Enable reporting of line numbers larger than 65535.
   //
   this->m_document.reset(
      xmlReadMemory(
         documentData.constData(),
         documentData.length(),
         nullptr,
         nullptr,
         XML_PARSE_NONET | /*XML_PARSE_NO_XXE |*/ XML_PARSE_BIG_LINES
      )
   );
   if (!this->m_document) {
      qWarning() << Q_FUNC_INFO << "Unable to read XML document" << fileName;
      throw std::runtime_error("Error reading XML document -- see log file for more details");
   }

   this->m_context.reset(xmlXPathNewContext(this->m_document.get()));
   if (!this->m_context) {
      // We're not expecting this to happen, but it doesn't hurt to be able to handle it
      qCritical() << Q_FUNC_INFO << "Unable to create context struct for XML document" << fileName;
      throw std::runtime_error("Context creation error while reading XML document -- see log file for more details");
   }
   return;
}

XmlDocument::~XmlDocument() = default;

xmlDoc * XmlDocument::get() const {
   return this->m_document.get();
}

XmlLibHelpers::XPathResult const XmlDocument::xPathResult(xmlNode & node, QString const & xPath) {
   this->m_context->node = &node;
   QByteArray const rawPath = xPath.toUtf8();
   return XmlLibHelpers::XPathResult{xmlXPathEvalExpression(XmlLibHelpers::asXmlCharString(rawPath), this->m_context.get())};
}
