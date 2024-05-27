/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonCoding.cpp is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#include "serialization/json/JsonCoding.h"

#include <algorithm>
#include <stdexcept>

#include <boost/json/object.hpp>

#include <QDebug>
#include <QFile>

#include "serialization/json/JsonRecord.h"
#include "serialization/json/JsonUtils.h"
#include "utils/ImportRecordCount.h"

//
// Private implementation class for JsonCoding
//
class JsonCoding::impl {
public:

   /**
    * Constructor
    */
   impl(JsonCoding & self,
        char const * name,
        char const * const version,
        JsonSchema::Id const schemaId,
        JsonRecordDefinition const & rootRecordDefinition) :
      m_self{self},
      m_name{name},
      m_version{version},
      m_schemaId{schemaId},
      m_rootRecordDefinition{rootRecordDefinition} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // Member variables for impl
   JsonCoding & m_self;
   QString const m_name;
   QString const m_version;
   JsonSchema::Id const m_schemaId;
   JsonRecordDefinition const & m_rootRecordDefinition;

};

JsonCoding::JsonCoding(char const * name,
                       char const * const version,
                       JsonSchema::Id const schemaId,
                       JsonRecordDefinition const & rootRecordDefinition) :
   pimpl{std::make_unique<impl>(*this, name, version, schemaId, rootRecordDefinition)} {
   // As a general rule, it's not helpful to try to log anything in this constructor as the object will be created
   // before logging has been initialised.
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header
// file)
JsonCoding::~JsonCoding() = default;

JsonRecordDefinition const & JsonCoding::getRoot() const {
   // The root element is the one with no corresponding named entity
   return this->pimpl->m_rootRecordDefinition;
}

bool JsonCoding::validateLoadAndStoreInDb(boost::json::value & inputDocument,
                                          QTextStream & userMessage) const {
   try {
      JsonSchema const & schema = JsonSchema::instance(this->pimpl->m_schemaId);
      if (!schema.validate(inputDocument, userMessage)) {
         qWarning() << Q_FUNC_INFO << "Schema validation failed";
         return false;
      }

   } catch (std::exception const & exception) {
      qWarning() <<
         Q_FUNC_INFO << "Caught exception while validating JSON file:" << exception.what();
      userMessage << exception.what();
      return false;
   }

   qDebug() << Q_FUNC_INFO << "Schema validation succeeded";

   //
   // We're expecting the root of the JSON document to be an object named "beerjson".  This should have been
   // established by the validation above.
   //
   // Of course, if we were being truly general, we would not hard-code "beerjson" here but rather have it as some
   // construction parameter of JsonCoding.  But, we do not foresee this being necessary any time soon (or possibly
   // ever).
   //
   // It would be nice to make documentRoot and rootRecordData references to const objects, but we need the
   // boost::json::value references stored in JsonRecord _not_ to be const when we're exporting to JSON, and it feels
   // clunky eg to have different references for reading and writing.
   //
   Q_ASSERT(inputDocument.is_object());
   boost::json::object & documentRoot = inputDocument.as_object();
   Q_ASSERT(documentRoot.contains("beerjson"));

   boost::json::value & rootRecordData = *documentRoot.if_contains("beerjson"); //documentRoot["beerjson"];
   Q_ASSERT(rootRecordData.is_object());
   qDebug() << Q_FUNC_INFO << "Root record contains" << rootRecordData.as_object().size() << "elements";

   //
   // Now we've loaded the JSON document into memory and determined that it's valid against its schema, we need to
   // extract the data from it
   //
   // Per https://www.json.org/json-en.html, a JSON object is an unordered set of name/value pairs, so there's no
   // constraint about what order we parse things
   //

   //
   // Look at the root object first
   //
   JsonRecord rootRecord{*this, rootRecordData, this->pimpl->m_rootRecordDefinition};
   qDebug() << Q_FUNC_INFO << "Looking at field definitions of root element (" << this->pimpl->m_rootRecordDefinition.m_recordName << ")";

   ImportRecordCount stats;

   if (!rootRecord.load(userMessage)) {
      return false;
   }
   qDebug() << Q_FUNC_INFO;

   // At the root level, Succeeded and FoundDuplicate are both OK return values.  It's only Failed that indicates an
   // error (rather than in info) message for the user in userMessage.
   if (JsonRecord::ProcessingResult::Failed == rootRecord.normaliseAndStoreInDb(nullptr, userMessage, stats)) {
      return false;
   }

   // Everything went OK - unless we found no content to read.
   // Summarise what we read in into the message displayed on-screen to the user, and return false if no content,
   // true otherwise
   return stats.writeToUserMessage(userMessage);
}
