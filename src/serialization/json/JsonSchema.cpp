/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonSchema.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
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

#include <boost/json/serialize.hpp>

#include <sourcemeta/blaze/compiler.h>
#include <sourcemeta/core/json.h>

#include "serialization/json/JsonUtils.h"

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
// Schema validation in JSON is also a relatively new thing.  There are several C++ validators.  Originally we want with
// Valijson (https://github.com/tristanpenman/valijson) because it is not tied to one underlying JSON library.  However,
// it does not support the most recent JSON Schema standards, so we have switched to Blaze
// (https://github.com/sourcemeta/blaze).
//

// Private implementation details that don't need access to class member variables
namespace {
   // All the schemas that have been summoned into existence
   // We don't use QMap here because it doesn't support storing std::unique_ptr, and we'd like the map to "own" the
   // schemas.  (As noted elsewhere, we don't want the schemas to be constructed too early in program execution, hence
   // why we are not using static variables to hold them.)
   std::map<JsonSchema::Id, std::unique_ptr<JsonSchema const>> jsonSchemas;

   /**
    * Used in JsonSchema::validate()
    */
   struct BlazeValidationError {
      //! Human readable description of the error
      QString description;
      //! The schema rule that was violated
      QString evaluatePath;
      //! The location in the document that violated the schema rule
      QString instanceLocation;
   };

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
      m_self{self},
      m_baseDir{baseDir},
      m_fileName{fileName} {

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

      try {
         this->m_compiledSchema = sourcemeta::blaze::compile(
            this->getBlazeReferencedDoc(std::string(this->m_fileName)),
            sourcemeta::core::schema_walker,
            //
            // We do our own schema resolution.  The required callback signature is a function that takes on parameter,
            // `std::string_view const identifier`, and returns
            // `sourcemeta::core::OwnedOrReference<sourcemeta::core::JSON>`.  Using a lambda here allows us to match
            // this but delegate the substantive work to a member function.
            //
            [this](std::string_view const uri){ return this->getBlazeReferencedDoc(uri); },
            sourcemeta::blaze::default_schema_compiler,
            // This next parameter can be either FastValidation or Exhaustive.  The former attempt to get to a boolean
            // result as fast as possible.  The latter perform exhaustive evaluation, including annotations.
            sourcemeta::blaze::Mode::Exhaustive
         );

         qDebug() << Q_FUNC_INFO << "Schema populated";

      } catch (std::exception const & exception) {
         // Because we're only populating data from resources shipped with the program, we're not expecting exceptions,
         // either from our own code or either of the two libraries (Boost.JSON and Blaze), so, if we do get one, it's
         // likely a coding error.  Log something (in case we didn't already) and barf the exception up to wherever the
         // constructor was called from.
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
    * \return Pointer to a Boost.JSON value which is the root of the document tree.  NB: Caller doesn't own this.  It
    *         is stored in \c m_schemaFileCache.
    */
   boost::json::value const * getReferencedDocument(std::string const & uri) {
      qDebug() << Q_FUNC_INFO << "Request for" << uri.c_str();
      QString const schemaFilePath = QString("%1/%2").arg(this->m_baseDir, uri.c_str());
      if (!this->m_schemaFileCache.contains(schemaFilePath)) {
         //
         // We allow comments in our bundled-as-resource schema files (which come from the BeerJSON project), in case we
         // want to annotate them
         //
         std::shared_ptr<boost::json::value const> const schemaDocument =
            std::make_shared<boost::json::value const>(JsonUtils::loadJsonDocument(schemaFilePath, true));

         qDebug() << Q_FUNC_INFO << "Read" << uri.c_str() << "as" << schemaFilePath;

         this->m_schemaFileCache.insert(schemaFilePath, schemaDocument);
      } else {
         qDebug() << Q_FUNC_INFO << schemaFilePath << "already in cache";
      }

      // We assert that we either already had the schema file in the cache or we just read it into the cache
      Q_ASSERT(this->m_schemaFileCache.contains(schemaFilePath));
      return this->m_schemaFileCache.value(schemaFilePath).get();
   }

   /**
    * \brief Wraps \c getReferencedDocument to convert a schema file from Boost format to Blaze format
    *
    * \param fullUri The full URI of the file to fetch
    */
   sourcemeta::core::JSON const & getBlazeReferencedDoc(std::string_view const fullUri) {
      //
      // Suppose we have a JSON schema document along the following lines:
      //
      //    {
      //      "$schema": "https://json-schema.org/draft/2020-12/schema",
      //      "$id": "https://dotbeer.org/schema/DotBeer.beer.schema",
      //      ...
      //      "properties": {
      //        "timestamp": {
      //          ...
      //          "$ref": "Measurement.beer.schema#/$defs/Date"
      //        },
      //        ...
      //      }
      //    }
      //
      // Strictly speaking, the ID of the referenced Measurement.beer.schema file is derived from the $ref and the $id
      // of the referring document: "https://dotbeer.org/schema/Measurement.beer.schema".
      //
      // When we were using Valijson, it would have requested simply "Measurement.beer.schema" in its callback, leaving
      // us to do any further resolution of the URI.  This suits us because we just want to pull the file from flat
      // local storage.  Blaze however, does the full resolution itself, so its callback to us requests
      // "https://dotbeer.org/schema/Measurement.beer.schema".
      //
      // If we wanted to be super correct, we should have a mapping from the "$id" field of each document to its local
      // file path.  However, this is overkill.  Our schemas are all sufficiently small and simple that we know all
      // all schema files are in the same directory.  So, when we get a URI request we just chop it down to everything
      // after the last slash, and treat that as a filename.
      //
      std::string const uri{fullUri.substr(fullUri.find_last_of('/') + 1)};
      qDebug() <<
         Q_FUNC_INFO << "Assuming" << uri.c_str() << "for request of" <<
         QString::fromUtf8(fullUri.data(), fullUri.size());
      boost::json::value const * boostDoc = this->getReferencedDocument(uri);
      if (!this->m_blazeSchemaFileCache.contains(boostDoc)) {
         auto const blazeSchemaDocument =
            std::make_shared<sourcemeta::core::JSON const>(
               // See comment below in JsonSchema::validate for conversion between Boost.JSON and Blaze
               sourcemeta::core::parse_json(boost::json::serialize(*boostDoc))
            );
         this->m_blazeSchemaFileCache.insert(boostDoc, blazeSchemaDocument);
      }
      return *this->m_blazeSchemaFileCache.value(boostDoc);
   }

   // Member variables
   JsonSchema & m_self;
   char const * const m_baseDir;
   char const * const m_fileName;
   QMap<QString, std::shared_ptr<boost::json::value const> > m_schemaFileCache = {};
   QMap<boost::json::value const *, std::shared_ptr<sourcemeta::core::JSON const> > m_blazeSchemaFileCache = {};
   sourcemeta::blaze::Template m_compiledSchema = {};
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
   auto const result = jsonSchemas.find(id);
   if (result != jsonSchemas.end()) {
      return *result->second;
   }
   char const * baseDir = nullptr;
   char const * fileName = nullptr;
   switch (id) {
      case JsonSchema::Id::DotBeer_1_0:
         baseDir = ":/schemas/dotBeer";
         fileName = "DotBeer.beer.schema";
         break;
      case JsonSchema::Id::BeerJSON_2_1:
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
   auto const insertionResult = jsonSchemas.emplace(
      std::make_pair(id, std::unique_ptr<JsonSchema>{new JsonSchema(baseDir, fileName)})
   );
   // We assert that the insertion succeeded (because the map did not already contain an item with the specified key)
   Q_ASSERT(insertionResult.second);

   //
   return *insertionResult.first->second;
}


bool JsonSchema::validate(boost::json::value const & document, QTextStream & userMessage) const {

   //
   // We could, and perhaps one day should, write and adapter to recursively construct a sourcemeta::core::JSON tree
   // from a boost::json::value one.  There are a couple of minor (but probably ignorable) wrinkles to this:
   //    - Boost.JSON has separate types for signed and unsigned integers (boost::json::kind::int64 and
   //      boost::json::kind::uint64) whereas Blaze only has signed integers (sourcemeta::core::JSON::Type::Integer)
   //    - Blaze has two "floating point types" (sourcemeta::core::JSON::Type::Real and
   //      sourcemeta::core::JSON::Type::Decimal), whereas Boost.JSON only has one (boost::json::kind::double_)
   //
   // However, for the moment, we "cheat" and use the fact that Blaze can parse JSON from scratch, ie we push:
   //    Boost.JSON representation --> String --> Blaze representation
   //
   // It's obviously not optimal to serialise out from Boost simply to deserialise into Blaze, but it's the simplest
   // approach.  So we'll start with this and see if we need to optimise in future.
   //
   sourcemeta::core::JSON blazeDocument{
      sourcemeta::core::parse_json(boost::json::serialize(document))
   };

   //
   // Blaze offers two extremes for reporting the results of validation.  The minimal approach is to call the two
   // parameter version of sourcemeta::blaze::Evaluator::validate(), and get a boolean return value for whether the
   // validation succeeded.  If we want more than this (eg to know the cause of a validation failure), we jump to the
   // other extreme and pass a third parameter to sourcemeta::blaze::Evaluator::validate().  This extra parameter is a
   // callback function that gets invoked before and after every step of the validation.  Inside that callback, we have
   // to pick out the cases where the step ran and failed (and do nothing when it it didn't yet run or it succeeded).
   //
   sourcemeta::blaze::Evaluator evaluator;
   QList<BlazeValidationError> blazeValidationErrors;
   bool const succeeded{
      evaluator.validate(
         this->pimpl->m_compiledSchema,
         blazeDocument,
         [&blazeDocument,
          &blazeValidationErrors](sourcemeta::blaze::EvaluationType   const    callBack_type,
                                  bool                                const    callBack_valid,
                                  sourcemeta::blaze::Instruction      const &  callBack_instruction,
                 [[maybe_unused]] sourcemeta::blaze::InstructionExtra const &  callBack_instructionExtra,
                                  sourcemeta::core::WeakPointer       const &  callBack_evaluatePath,
                                  sourcemeta::core::WeakPointer       const &  callBack_instanceLocation,
                                  sourcemeta::core::JSON              const &  callBack_annotation) {
            if (callBack_type != sourcemeta::blaze::EvaluationType::Post || callBack_valid) {
               // Step succeeded or didn't yet run.  In either case, nothing for us to do.
               return;
            }
            blazeValidationErrors.append(
               //
               // Doco for sourcemeta::blaze::describe says:
               //
               //    This function translates a "post" step execution into a human-readable string. Useful as the
               //    building block for producing user-friendly evaluation results.  Note that describing a "pre" step
               //    execution is NOT supported.
               //
               BlazeValidationError{
                  QString::fromStdString(sourcemeta::blaze::describe(callBack_valid,
                                                                     callBack_instruction,
                                                                     callBack_evaluatePath,
                                                                     callBack_instanceLocation,
                                                                     blazeDocument,
                                                                     callBack_annotation)),
                  QString::fromStdString(sourcemeta::core::to_string(callBack_evaluatePath)),
                  QString::fromStdString(sourcemeta::core::to_string(callBack_instanceLocation))
               }
            );
            return;
         }
      )
   };
   qDebug() << Q_FUNC_INFO << "Schema validation via Blaze" << (succeeded ? "succeeded": "failed");
   if (!succeeded) {
      int errorNumber = 0;
      // If there is more than one error, then we'll log them all here but only show the first one to the user on
      // the screen.  (Otherwise we might risk information overload.)
      for (auto const & blazeValidationError : blazeValidationErrors) {
         //
         // We'll put the log file error in English on the assumption that many users will want to report a bug and
         // include log files (or extracts thereof).
         //
         qWarning() <<
            Q_FUNC_INFO << "Validation error #" << ++errorNumber << " at " << blazeValidationError.instanceLocation <<
            "schema condition" << blazeValidationError.evaluatePath << "is violated:" <<
            blazeValidationError.description;
         if (1 == errorNumber) {
            //
            // For displaying on the screen we can translate the text under our control, but the stuff from Blaze will
            // still be in English -- for now at least.
            //
            userMessage <<
               QObject::tr(
                  "%1 errors found in JSON file.  First error at %2: schema condition %3 is violated because \"%4\""
               ).arg(
                  blazeValidationErrors.size()
               ).arg(
                  blazeValidationError.instanceLocation
               ).arg(
                  blazeValidationError.evaluatePath
               ).arg(
                  blazeValidationError.description
               );
         }
      }
   }

   return succeeded;
}