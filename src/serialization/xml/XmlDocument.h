/*======================================================================================================================
 * serialization/xml/XmlDocument.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef SERIALIZATION_XML_XMLDOCUMENT_H
#define SERIALIZATION_XML_XMLDOCUMENT_H
#pragma once

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/xpath.h>

#include <QByteArray>
#include <QString>

#include "utils/CWrappers.h"
#include "serialization/xml/XmlLibHelpers.h"

//! RAII wrapper around libxml2's xmlDoc
class XmlDocument {
public:

   explicit XmlDocument(QByteArray const & documentData,
                        QString const & fileName);
   ~XmlDocument();

   xmlDoc * get() const;

   XmlLibHelpers::XPathResult const xPathResult(xmlNode & node, QString const & xPath);

private:
   //=============================================== Member Variables ===============================================
   //
   // Yes, it is mildly annoying that libxml2 free function naming is not consistent (eg xmlSchemaFree for xmlSchema
   // but xmlFreeDoc for xmlDoc).  AIUI this is just because the library evolved over the course of time with
   // multiple contributors taking slightly different approaches.
   //
   QString m_fileName = "";
   CWrappers::unique_ptr<xmlDoc, xmlFreeDoc> m_document = nullptr;

public:
   //! NB: Putting this here assumes we are only reading one XML document at a time.
   CWrappers::unique_ptr<xmlXPathContext, xmlXPathFreeContext> m_context = nullptr;
};


#endif
