/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonSchema.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#include "serialization/json/JsonSchema.h"

#include <map>
#include <memory>

#include <QDebug>
#include <QMap>
#include <QObject>
#include <QString>

#include <valijson/adapters/boost_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validator.hpp>

#include "serialization/json/JsonUtils.h"
#include "utils/BtStringStream.h"

//
//                                 ****************************************************
//                                 * General note about JSON libraries and frameworks *
//                                 ****************************************************
//
// There are several C++ JSON libraries, including some Qt classes (https://doc.qt.io/qt-5/json.html), RapidJSON
// (https://rapidjson.org/) "JSON for Modern C++" AKA "nlohmann JSON" (https://github.com/nlohmann/json) and Boost.JSON
// (https://www.boost.org/doc/libs/1_77_0/libs/json/doc/html/index.html).
//
// I am reluctant to use the Qt classes, even though we are a Qt app, because Qt have a history of dropping support for
// "non-core" features (see comments in xml/XmlCoding.cpp for example).
//
// The Boost library is one of the newer implementations but has some design advantages over other libraries (see
// https://www.boost.org/doc/libs/1_77_0/libs/json/doc/html/json/comparison.html).  Boost libraries in general are seen
// to be high quality and several of them have become the basis for C++ Standard Library features.  So, using the Boost
// library seems like a safe bet.
//
// Schema validation in JSON is also a relatively new thing.  There are several C++ validators (see
// https://json-schema.org/implementations.html#validator-c++) but we like Valijson
// (https://github.com/tristanpenman/valijson) in particular because it is not tied to one underlying JSON library.
//
//

// Private implementation details that don't need access to class member variables
namespace {
   // All the schemas that have been summoned into existence
   // We don't use QMap here because it doesn't support storing std::unique_ptr, and we'd like the map to "own" the
   // schemas.  (As noted elsewhere, we don't want the schemas to be constructed too early in program execution, hence
   // why we are not using static variables to hold them.)
   std::map<JsonSchema::Id, std::unique_ptr<JsonSchema const>> jsonSchemas;

   //
   // A JSON schema can be spread across several files linked together via "$ref" statements in the JSON.  Valijson uses
   // callbacks to fetch such referenced JSON documents when it is loading in a schema.  We cannot use a non-static
   // member function for a callback, and the callbacks do not pass in a context (because they were originally designed
   // to be used only for retrieving documents referenced by absolute URIs).  So, instead, use a static member function
   // for the callback and a thread local variable to remember the last JsonSchema object used on this thread, which
   // will be the one that asked Valijson to load in the schema.
   //
   // Note that the callbacks are not used when validating a JSON document against a schema.  (So, unless and until we
   // support multiple JSON schemas, this is a more robust solution than strictly needed.  OTOH, it's very little code,
   // so no harm in being future-proof here.)
   //
   thread_local JsonSchema * currentJsonSchema = nullptr;

   /**
    * \brief Called from Valijson to free a resource obtained from \c fetchReferencedDocument()
    *
    *        This can be an anonymous namespace function because it has no work to do - see comment below.
    */
   void freeReferencedDocument([[maybe_unused]] boost::json::value const * document) {
      // There isn't anything for us to do, because we hang on to all the JSON schema documents until the program
      // terminates.
      qDebug() << Q_FUNC_INFO;
      return;
   }

   /**
    * \brief Log a Valijson error (in English) and then turn it into something we can show on the screen (in the user's
    *        preferred language).
    */
   QString validationErrorToString(valijson::ValidationResults::Error const & validationError) {
      //
      // The path to the node that failed validation is stored as std::vector<std::string>
      // Typically, the sequence of elements will be something along the following lines:
      //    <root>
      //    [beerjson]
      //    [recipes]
      //    [0]
      // Since this already lots of brackets, the cleanest thing is arguably to put them all on the same line with
      // spaces in between, eg:
      //    At node <root> [beerjson] [recipes] [0], error was: ...
      //
      BtStringStream nodePath;
      for (auto cc : validationError.context) {
         nodePath << " " << cc.c_str();
      }

      qWarning() <<
         Q_FUNC_INFO << "At node" << nodePath.asString() << "error was" << validationError.description.c_str();
      return QObject::tr("At node %1, error was %2").arg(nodePath.asString()).arg(validationError.description.c_str());
   }
}

// This private implementation class holds all private non-virtual members of JsonSchema
class JsonSchema::impl {
public:
   /**
    * Constructor
    */
   impl(JsonSchema & self,
        char const * const baseDir,
        char const * const fileName) :
      self{self},
      baseDir{baseDir},
      fileName{fileName},
      schemaFileCache{},
      schemaAdapter{*this->getReferencedDocument(std::string(fileName))},
      jsonSchema{},
      schemaParser{} {

      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   /**
    * This function needs to be called from JsonSchema, after the JsonSchema::impl constructor has returned, so that
    * JsonSchema.pimpl is set.
    */
   void parseAndPopulateSchema() {
      // Having loaded in the base schema document to a Boost.JSON object, and wrapped it in a suitable adapter for
      // Valijson, we now ask Valijson to parse it, which will result in any referenced documents being loaded (via
      // JsonSchema::fetchReferencedDocument being passed in as callback function).
      //
      // Note that, for JsonSchema::fetchReferencedDocument to be able to call this->pimpl->getReferencedDocument(), we
      // have to have correct values in currentJsonSchema and this->self.pimpl.  (Hence why we can't do this in
      // JsonSchema::impl constructor, as this->self.pimpl is only set on return from that function.)
      //
      try {
         currentJsonSchema = &this->self;
         this->schemaParser.populateSchema(this->schemaAdapter,
                                             this->jsonSchema,
                                             &JsonSchema::fetchReferencedDocument,
                                             &freeReferencedDocument);
         qDebug() << Q_FUNC_INFO << "Schema populated";

      } catch (std::exception const & exception) {
         // Because we're only populating data from resources shipped with the program, we're not expecting exceptions,
         // either from our own code or either of the two libraries (Boost.JSON and Valijson), so, if we do get one,
         // it's likely a coding error.  Log something (in case we didn't already) and barf the exception up to wherever
         // the constructor was called from.
         qCritical() << Q_FUNC_INFO << "Caught exception:" << exception.what();
         throw;
      }

      return;
   }

   /**
    * \brief Read in the specified schema file from baseDir as a Boost.JSON document tree.
    *
    *        Note: Amongst other things, this is (indirectly, from non-member-function fetchReferencedDocument) the
    *              callback Valijson uses to obtain referenced schema documents, which is why the parameter is
    *              std::string rather, say, QString.
    *
    * \param uri Specifies the file to fetch.  (In the most general case this could theoretically be some URL on the
    *            internet, but, in reality, we only want to support relative URIs to load local JSON schema files.
    *            This is reasonable, because we're only envisaging using schema documents which we control and ship with
    *            the product.  So uri is actually just a file name inside this->baseDir.)
    *
    * \return Pointer to a Boost.JSON value which is the root of the document tree
    */
   boost::json::value const * getReferencedDocument(std::string const & uri) {
      // We assert that JsonSchema::fetchReferencedDocument is not calling us on a JsonSchema whose pimpl member
      // variable has not yet been set.  CLang thinks this assert is unnecessary ("warning: 'this' pointer cannot be
      // null in well-defined C++ code; comparison may be assumed to always evaluate to true
      // [-Wtautological-undefined-compare"]) so we disable the assert on that compiler (which is currently only MacOS).
#ifndef __clang__
      Q_ASSERT(this != nullptr);
#endif
      qDebug() << Q_FUNC_INFO << "Request for" << uri.c_str();
      QString schemaFilePath = QString("%1/%2").arg(this->baseDir, uri.c_str());
      if (!this->schemaFileCache.contains(schemaFilePath)) {
         //
         // We allow comments in our bundled-as-resource schema files (which come from the BeerJSON project), in case we
         // want to annotate them
         //
         std::shared_ptr<boost::json::value const> schemaDocument =
            std::make_shared<boost::json::value const>(JsonUtils::loadJsonDocument(schemaFilePath, true));

         qDebug() << Q_FUNC_INFO << "Read" << uri.c_str() << "as" << schemaFilePath;

         this->schemaFileCache.insert(schemaFilePath, schemaDocument);
      } else {
         qDebug() << Q_FUNC_INFO << schemaFilePath << "already in cache";
      }

      // We assert that we either already had the schema file in the cache or we just read it into the cache
      Q_ASSERT(this->schemaFileCache.contains(schemaFilePath));
      return this->schemaFileCache.value(schemaFilePath).get();
   }


   // Member variables
   JsonSchema & self;
   char const * const baseDir;
   char const * const fileName;
   QMap<QString, std::shared_ptr<boost::json::value const> > schemaFileCache;
   // This wrapper around boost::json::value allows us to pass it in to Valijson.  (There are a whole bunch of other
   // Valijson adapters so it can support other JSON libraries.)
   valijson::adapters::BoostJsonAdapter schemaAdapter;
   valijson::Schema jsonSchema;
   valijson::SchemaParser schemaParser;
};



JsonSchema::JsonSchema(char const * const baseDir,
                       char const * const fileName) :
   pimpl{std::make_unique<impl>(*this, baseDir, fileName)} {

   // Do the work that can't be done in the pimpl constructor
   this->pimpl->parseAndPopulateSchema();
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
JsonSchema::~JsonSchema() = default;

JsonSchema const & JsonSchema::instance(JsonSchema::Id id) {
   // Once we are using C++20, we can write the following:
   ///if (jsonSchemas.contains(id)) {
   ///   return *jsonSchemas.value(id);
   ///}
   auto result = jsonSchemas.find(id);
   if (result != jsonSchemas.end()) {
      return *result->second;
   }
   char const * baseDir = nullptr;
   char const * fileName = nullptr;
   switch (id) {
      case JsonSchema::Id::BEER_JSON_2_1:
         baseDir = ":/schemas/beerjson/1.0";
         fileName = "beer.json";
         break;
   }
   // We assert that all possibilities were covered in the switch statement above.  (We'd get a compiler warning if not,
   // as JsonSchema::Id is a strongly-typed enum.)
   Q_ASSERT(baseDir);
   Q_ASSERT(fileName);

   // We want the map to own the created object, so we construct it in place, rather than passing a copy
   // Note that we cannot use std::make_unique here as we have private constructor & destructor.  However,
   // std::unique_ptr<...>(new ...) is good enough for us.  (If we were ever at the point of new throwing exceptions
   // because of lack of memory, we'd have bigger problems than exception safety.)
   auto insertionResult = jsonSchemas.emplace(std::make_pair(id, std::unique_ptr<JsonSchema>{new JsonSchema(baseDir, fileName)}));
   // We assert that the insertion succeeded (because the map did not already contain an item with the specified key)
   Q_ASSERT(insertionResult.second);

   //
   return *insertionResult.first->second;
}


bool JsonSchema::validate(boost::json::value const & document, QTextStream & userMessage) const {

   // Now pass the input document into Valijson (via a wrapper as with the base schema document) and validate it against
   // the schema
   valijson::adapters::BoostJsonAdapter inputAdapter{document};
   valijson::Validator validator;
   valijson::ValidationResults validationResults;
   if (!validator.validate(this->pimpl->jsonSchema, inputAdapter, &validationResults)) {
      qWarning() << Q_FUNC_INFO << validationResults.numErrors() << "validation errors in JSON file";
      // If there is more than one error, then we'll log them all here but only show the first one to the user on
      // the screen.  (Otherwise we might risk information overload.)
      userMessage <<
         QObject::tr("%1 errors found in JSON file.  First error: ").arg(validationResults.numErrors()) <<
         validationErrorToString(*validationResults.begin());
      int errNum = 1;
      for (auto err = validationResults.begin(); err != validationResults.end(); ++err, ++errNum) {
         qWarning() << Q_FUNC_INFO << "Validation error #" << errNum << ":" << validationErrorToString(*err);
      }
      return false;
   }

   qDebug() << Q_FUNC_INFO << "Validation succeeded";
   return true;
}


boost::json::value const * JsonSchema::fetchReferencedDocument(std::string const & uri) {
   // It's a coding error if we asked Valijson to load a schema without setting currentJsonSchema
   Q_ASSERT(currentJsonSchema);
   return currentJsonSchema->pimpl->getReferencedDocument(uri);
}
