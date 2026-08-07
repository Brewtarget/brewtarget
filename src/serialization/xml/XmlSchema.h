/*======================================================================================================================
 * serialization/xml/XmlSchema.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef SERIALIZATION_XML_XMLSCHEMA_H
#define SERIALIZATION_XML_XMLSCHEMA_H
#pragma once

#include <memory>

#include <libxml2/libxml/xmlschemas.h>

#include <QString>
#include <QTextStream>

#include "serialization/xml/XmlDocument.h"
#include "serialization/xml/XmlErrorHandler.h"
#include "utils/CWrappers.h"

//! RAII wrapper around libxml2's xmlSchema / xmlSchemaValidCtxt
class XmlSchema {
public:
   explicit XmlSchema(QString const & schemaResource, XmlErrorHandler & errorHandler);
   ~XmlSchema();

   bool validate(XmlDocument const & xmlDocument,
                 XmlErrorHandler & errorHandler,
                 QTextStream & userMessage) const;

private:
   //=============================================== Member Variables ===============================================
   //
   // Resource management is handled automatically via CWrappers::unique_ptr.  However, we have to declare things in the
   // right order here.  Member variables are destroyed in the reverse order of their declaration, and we want
   // xmlSchemaFreeValidCtxt() called before xmlSchemaFree().
   //
   CWrappers::unique_ptr<xmlSchema         , xmlSchemaFree         > m_schema                  = nullptr;
   CWrappers::unique_ptr<xmlSchemaValidCtxt, xmlSchemaFreeValidCtxt> m_schemaValidationContext = nullptr;
};

#endif