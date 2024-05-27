/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonSchema.h is part of Brewtarget, and is copyright the following authors 2021-2022:
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
#ifndef SERIALIZATION_JSON_JSONSCHEMA_H
#define SERIALIZATION_JSON_JSONSCHEMA_H
#pragma once

#include <memory> // For PImpl

#include <boost/json/value.hpp>

class QTextStream;

/**
 * \class JsonSchema holds all the files for a single JSON schema (which we give to Valijson for it to validate a JSON
 *        document)
 *
 *        Note that this class ONLY wraps the JSON schema (see https://json-schema.org/).  It does not hold any of the
 *        info needed for us to process the file.  For that, see \c JsonCoding.  (Each \c JsonCoding has a corresponding
 *        \c JsonSchema.)
 *
 *        We could combine \c JsonCoding and \c JsonSchema into a single class, but, for the moment, we'd rather have
 *        two small classes than one big one, especially as neither class needs to know the inner workings of the other.
 */
class JsonSchema {
public:

   // Each JsonSchema is a const (after construction) singleton for the schema it represents (eg BeerJSON 2.1), so this
   // enum lists all the available ones.
   enum class Id {
      BEER_JSON_2_1
   };

   /*!
    * \brief This should be the ONLY way you get an instance.
    *
    *        Note there are two advantages of this over, say, global constant variables of type \c JsonSchema.  Firstly
    *        we only construct a \c JsonSchema if we are actually going to use it.  Secondly, and more importantly, we
    *        do not call the constructor until after all Qt start-up has happened, so we can guarantee that, eg, Qt
    *        resources are accessible.
    *
    * \param id Which schema you want to get.
    */
   static JsonSchema const & instance(JsonSchema::Id id);

   //! Destructor needs to be public as, internally, we manage instances of JsonSchema in std::unique_ptr
   ~JsonSchema();

   /**
    * \brief Validate a JSON document
    *
    * \param document JSON document loaded with \c JsonUtils::loadJsonDocument()
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return \c true if file validated OK (including if there were "errors" that we can safely ignore)
    *         \c false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validate(boost::json::value const & document, QTextStream & userMessage) const;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;


   /**
    * \brief Hidden Constructor
    *
    *        The only reason there are two parameters (directory and file name) rather than one (fully qualified file
    *        name) is because it makes reusing some code inside the class a little easier.
    *
    * \param baseDir The directory path in which these schema files live.  Usually a resource path, eg
    *                ":/schemas/beerjson/1.0"
    * \param fileName  The file name, inside \c baseDir, of the initial file of the schema, eg "beer.json".  (This may
    *                  reference other files via $ref tags in the schema JSON, these will be loaded automatically from
    *                  \c baseDir.)
    */
   JsonSchema(char const * const baseDir,
              char const * const fileName);


   //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
   JsonSchema(JsonSchema const&) = delete;
   //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
   JsonSchema & operator=(JsonSchema const&) = delete;
   //! No move constructor
   JsonSchema(JsonSchema &&) = delete;
   //! No move assignment
   JsonSchema & operator=(JsonSchema &&) = delete;


   /**
    * \brief This is the callback we give to Valijson, which then forwards it on to whatever the last JsonSchema object
    *        we were dealing with on this thread was (which should be the one that gave the callback to Valijson).
    */
   static boost::json::value const * fetchReferencedDocument(std::string const & uri);
};

#endif
