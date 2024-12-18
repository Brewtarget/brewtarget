/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * serialization/json/JsonRecord.h is part of Brewtarget, and is copyright the following authors 2020-2023:
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
#ifndef SERIALIZATION_JSON_JSONRECORD_H
#define SERIALIZATION_JSON_JSONRECORD_H
#pragma once
#include <vector>

#include <boost/json/object.hpp>
#include <boost/json/array.hpp>

#include <QTextStream>

#include "serialization/json/JsonCoding.h"
#include "serialization/json/JsonRecordDefinition.h"
#include "serialization/SerializationRecord.h"
#include "utils/EnumStringMapping.h"
#include "utils/ImportRecordCount.h"

/**
 * \brief This class holds data about a specific individual record that we are reading from or writing to a JSON
 *        document.  It uses data from a corresponding singleton const \c JsonRecordDefinition to map between our
 *        internal data structures and fields in a JSON document.
 */
class JsonRecord : public SerializationRecord<JsonRecord, JsonCoding, JsonRecordDefinition> {
public:

   /**
    * \brief Constructor should only be called by \c JsonRecordDefinition
    *
    *        To create a new \c JsonRecord call \c JsonRecordDefinition::makeRecord
    *
    * \param jsonCoding
    * \param recordData  Note that this must be a reference to \c boost::json::value.  (If you pass in a reference to
    *                    a \c boost::json::object then the compiler will use it as a parameter to construct a temporary
    *                    \c boost::json::value object, and pass the reference to that into this constructor.  That
    *                    temporary object will go out of scope immediately this constructor returns, and subsequent
    *                    calls to \c JsonRecord will then be using an invalid reference to a \c boost::json::value that
    *                    no longer exists.  Cue garbage data, core dumps etc.
    *                       Long story short, we never want to implicitly construct a new \c boost::json::value from an
    *                    \c boost::json::object, so we use the template trick below to prevent that happening for this
    *                    constructor.
    *
    * \param recordDefinition
    */
   JsonRecord(JsonCoding const & jsonCoding,
              boost::json::value & recordData,
              JsonRecordDefinition const & recordDefinition);
   /**
    * \brief See constructor comment above for why we don't want to let the compiler do automatic conversions of the
    *        constructor arguments (which is what this template trick achieves).
    */
   template <typename P, typename Q, typename R> JsonRecord(P, Q, R) = delete;
   virtual ~JsonRecord();

   virtual SerializationRecordDefinition const & recordDefinition() const override;

   /**
    * \brief From the supplied record (ie node) in an JSON document, load into memory the data it contains, including
    *        any other records nested inside it.
    *
    * \param userMessage Where to append any error messages that we want the user to see on the screen
    *
    * \return \b true if load succeeded, \b false if there was an error
    */
   [[nodiscard]] bool load(QTextStream & userMessage);

   //! \brief Override base class member function
   [[nodiscard]] virtual ProcessingResult normaliseAndStoreInDb(std::shared_ptr<NamedEntity> containingEntity,
                                                                QTextStream & userMessage,
                                                                ImportRecordCount & stats) override;

   static bool listToJson(QList< std::shared_ptr<NamedEntity> > const & objectsToWrite,
                          boost::json::array & outputArray,
                          JsonCoding const & coding,
                          JsonRecordDefinition const & recordDefinition);

   /**
    * \brief Convert a \c NamedEntity to JSON
    * \param namedEntityToExport The object that we want to convert to JSON
    *
    * \return \c true if succeeded, \c false otherwise
    */
   [[nodiscard]] bool toJson(NamedEntity const & namedEntityToExport);

private:
   /**
    * \brief Load in a single child record.
    */
   [[nodiscard]] bool loadChildRecord(JsonRecordDefinition::FieldDefinition const & parentFieldDefinition,
                                      JsonRecordDefinition const & childRecordDefinition,
                                      boost::json::value & childRecordData,
                                      QTextStream & userMessage);

   /**
    * \brief Load in an array of child records.  It is for derived classes to determine whether and when they have child
    *        records to process (eg Hop records inside a Recipe).  But the algorithm for processing is generic, so we
    *        implement it in this base class.
    */
   [[nodiscard]] bool loadChildRecords(JsonRecordDefinition::FieldDefinition const & parentFieldDefinition,
                                       JsonRecordDefinition const & childRecordDefinition,
                                       boost::json::array & childRecordsData,
                                       QTextStream & userMessage);

protected:
///   [[nodiscard]] bool normaliseAndStoreChildRecordsInDb(QTextStream & userMessage, ImportRecordCount & stats);

private:
   /**
    * \brief Add a value to a JSON object
    *
    * \param fieldDefinition
    * \param recordDataAsObject
    * \param key
    * \param value  The value to add.  NB this can be modified by this function (specifically to change the contents
    *               from \c std::optional<T> to \c T).  Caller is not expected to need the value after this function
    *               returns.
    */
   void insertValue(JsonRecordDefinition::FieldDefinition const & fieldDefinition,
                    boost::json::object & recordDataAsObject,
                    std::string_view const & key,
                    QVariant & value);

protected:
///   JsonCoding const & m_coding;

   /**
    * The underlying type of the contents of \c recordData is \c boost::json::object.  However, we need to store it as
    * \c boost::json::value to be able to use JSON pointer (aka XPath) functions (because, although you can easily
    * extract the contained \c boost::json::object from a \c boost::json::value, you cannot go in the other direction
    * and get the containing \c boost::json::value from a \c boost::json::object).
    */
   boost::json::value & m_recordData;

///   JsonRecordDefinition const & m_recordDefinition;

///   //
///   // Keep track of any child (ie contained) records as we're reading in FROM a JSON file.  (NB: We don't need to do
///   // this when writing out TO a JSON file as we don't have to worry about duplicate detection or construction order
///   // etc.)
///   //
///   // This is used both for lists of children (eg hop additions in a recipe, or steps in a mash) and for single children
///   // (eg boil of a recipe, mash of a recipe).
///   //
///   // Note that we don't use QVector here or below as it always wants to be able to copy things, which doesn't play
///   // nicely with there being a std::unique_ptr inside the ChildRecordSet struct.  OTOH, std::vector is guaranteed to
///   // be able to hold std_unique_ptr (provided, of course, that we use move semantics to put the elements in the
///   // vector).
///   //
///   struct ChildRecordSet {
///      /**
///       * \brief This holds info about the attribute/field to which this set of child records relates.  Eg, if a recipe
///       *        record has hop addition and fermentable addition child records, then it needs to know which is which and
///       *        how to connect them with the recipe.
///       *
///       *        If it's \c nullptr then that means this is a top-level record (eg just a hop variety rather than a use
///       *        of a hop in a recipe), in which, once the records are read in, there's no further work to do: the
///       *        records do not need to be connected with anything else.
///       */
///      JsonRecordDefinition::FieldDefinition const * parentFieldDefinition;
///
///      /**
///       * \brief The actual child record(s)
///       */
///      std::vector< std::unique_ptr<JsonRecord> > records;
///   };
///
///   std::vector<ChildRecordSet> m_childRecordSets;
};

#endif
