/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonCoding.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_JSON_JSONCODING_H
#define SERIALIZATION_JSON_JSONCODING_H
#pragma once

#include <memory> // For smart pointers
#include <QHash>
#include <QObject>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include "serialization/json/JsonRecordDefinition.h"
#include "serialization/json/JsonSchema.h"

/**
 * \brief An instance of this class holds information about a particular JSON encoding (eg BeerJSON 2.1).  Specifically,
 *        that information includes:
 *          • the corresponding \c JsonSchema that we use to validate a JSON document
 *          • the \c JsonRecordDefinition objects that define how we map BeerJSON objects to our own data structures.
 *
 *        As we are parsing or creating a JSON document, we'll create a \c JsonRecord for each record we are reading /
 *        writing, using the relevant \c JsonRecordDefinition as a template.
 *
 *        Similar to xml/XmlCoding.h
 */
class JsonCoding {

public:
   /**
    * \brief Constructor
    * \param name The name of this encoding (eg "BeerJSON 1.0").  Used primarily for logging.
    * \param version The version to write out to BeerJSON records
    * \param schema The wrapper around the JSON schema that we'll use to validate the input (if we are reading from,
    *               rather than writing to, JSON).
    * \param jsonRecordDefinitions  TODO: I don't think we need all of these.  It should suffice to have the root record.
    */
   JsonCoding(char const * const name,
              char const * const version,
              JsonSchema::Id const schemaId,
              JsonRecordDefinition const & rootRecordDefinition);

   /**
    * \brief Destructor
    */
   ~JsonCoding();

   /**
    * \brief Check whether we know how to process a record of a given (JSON tag) name
    * \param recordName
    * \return \c true if we know how to process (ie we have the address of a function that can create a suitable
    *         \c JsonRecord object), \c false if not
    */
   [[nodiscard]] bool isKnownJsonRecordDefinition(QString recordName) const;

   /**
    * \brief Get the root definition element, ie what we use to start processing a document
    */
   JsonRecordDefinition const & getRoot() const;

   /**
    * \brief Validate JSON file against schema, load its contents into objects, and store then in the DB
    *
    * \param inputDocument The JSON file to validate and read
    * \param userMessage Any message that we want the top-level caller to display to the user (either about an error
    *                    or, in the event of success, summarising what was read in) should be appended to this string.
    *
    * \return true if file validated OK (including if there were "errors" that we can safely ignore)
    *         false if there was a problem that means it's not worth trying to read in the data from the file
    */
   bool validateLoadAndStoreInDb(boost::json::value & inputDocument,
                                 QTextStream & userMessage) const;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};

#endif
