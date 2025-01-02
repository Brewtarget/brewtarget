/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/ObjectStore.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#include "database/ObjectStore.h"

#include <cstring>
#include <iostream> // For start-up errors!
#include <tuple>

#include <QDebug>
#include <QHash>
#include <QMap>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>
#include <QVector>

#include "database/BtSqlQuery.h"
#include "database/Database.h"
#include "database/DbTransaction.h"
#include "Logging.h"
#include "model/NamedParameterBundle.h"
#include "utils/MetaTypes.h"
#include "utils/OptionalHelpers.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_ObjectStore.cpp"

// Private implementation details that don't need access to class member variables
namespace {

   /**
    * \brief For a given field type, get the native database typename
    */
   char const * getDatabaseNativeTypeName(Database const & database, ObjectStore::FieldType const fieldType) {
      switch (fieldType) {
         case ObjectStore::FieldType::Bool:    return database.getDbNativeTypeName<bool>();
         case ObjectStore::FieldType::Int:     return database.getDbNativeTypeName<int>();
         case ObjectStore::FieldType::UInt:    return database.getDbNativeTypeName<unsigned int>();
         case ObjectStore::FieldType::Double:  return database.getDbNativeTypeName<double>();
         case ObjectStore::FieldType::String:  return database.getDbNativeTypeName<QString>();
         case ObjectStore::FieldType::Date:    return database.getDbNativeTypeName<QDate>();
         case ObjectStore::FieldType::Enum:    return database.getDbNativeTypeName<QString>();
         case ObjectStore::FieldType::Unit:    return database.getDbNativeTypeName<QString>();
         // No default case needed as compiler should warn us if any options covered above
      }
      // It's a coding error if we get here!
      Q_ASSERT(false);
      return nullptr; // Should never get here
   }

   /**
    * \brief Create a database table without foreign key constraints (allowing tables to be created in any order)
    *
    *        NB: In practice, this means omitting columns that are foreign keys.  They will be added in
    *            addForeignKeysToTable() below.  This is because of limitations in SQLite.
    *
    *            SQLite does not support adding a foreign key to an existing column.  In newer versions (since 3.35.0)
    *            you can work around this (for an empty table) by dropping a column and re-adding it with a foreign
    *            key.  However, older versions of SQLite do not even support "DROP COLUMN".  And, since we use the
    *            version of SQLite that's embedded in Qt, we can't easily just switch to a newer version.
    *
    *            So, instead, when we create tables, we miss out the foreign key columns altogether and then add them,
    *            with the foreign key constraints, in addForeignKeysToTable() below.
    *
    * \return true if succeeded, false otherwise
    */
   bool createTableWithoutForeignKeys(Database & database,
                                      QSqlDatabase & connection,
                                      ObjectStore::TableDefinition const & tableDefinition) {
      //
      // We're building a SQL string of the form
      //    CREATE TABLE foobar (
      //       bah INTEGER PRIMARY KEY,
      //       hum TEXT,
      //       ...
      //       bug DATE
      //    );
      // .:TBD:. At some future point we might extend our model to allow marking some columns as NOT NULL (eg by making
      //         the derived class of NamedEntity available in ObjectStore::TableDefinition so we can query isOptional()
      //         on each property), but it doesn't seem pressing at the moment.
      //
      QString queryString{"CREATE TABLE "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << tableDefinition.tableName << " (\n";
      bool firstFieldOutput = false;
      for (auto const & fieldDefn: tableDefinition.tableFields) {
         if (std::holds_alternative<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder)) {
            qDebug() << Q_FUNC_INFO << "Skipping" << fieldDefn.columnName << "as foreign key";
            // It's (currently) a coding error if a foreign key is anything other than an integer
            Q_ASSERT(fieldDefn.fieldType == ObjectStore::FieldType::Int);
            continue;
         }

         // If it's not the first field, we need a separator from the previous field
         if (firstFieldOutput) {
            queryStringAsStream << ", \n";
         }

         queryStringAsStream << fieldDefn.columnName;

         if (!firstFieldOutput) {
            //
            // If it's the first column then it's the primary key and we are going to need to add INTEGER PRIMARY KEY
            // (for SQLite) or SERIAL PRIMARY KEY (for PostgreSQL) or some such at the end.  The Database class knows
            // exactly what text is needed for each type of database.
            //
            // NOTE because PostgreSQL needs SERIAL instead of INTEGER for an integer primary key, we (a) ignore
            // fieldDefn.fieldType and (b) do not support non-integer primary keys.  This _could_ be fixed by making
            // Database::getDbNativePrimaryKeyDeclaration() take fieldType as a parameter, but this is a future exercise
            // to consider if we ever reach the point of needing non-integer primary keys.
            //
            firstFieldOutput = true;
            queryStringAsStream << " " << database.getDbNativePrimaryKeyDeclaration();
         } else {
            queryStringAsStream << " " << getDatabaseNativeTypeName(database, fieldDefn.fieldType);
         }
      }
      queryStringAsStream << "\n);";

      qDebug().noquote() << Q_FUNC_INFO << "Table creation: " << queryString;

      BtSqlQuery sqlQuery{connection};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }
      return true;
   }

   /**
    * \brief Add foreign key constraints to a newly-created table
    *
    *        Doing the foreign key columns after all the tables are created means we don't have to worry about the
    *        order in which table creation is done.
    *
    * \return true if succeeded, false otherwise
    */
   bool addForeignKeysToTable(Database & database,
                              QSqlDatabase & connection,
                              ObjectStore::TableDefinition const & tableDefinition) {
      //
      // The exact format for adding a column that's a foreign key varies by database.  Some accept:
      //
      //    ALTER TABLE foobar ADD COLUMN other_id INTEGER REFERENCES other(id);
      //
      // Others want:
      //
      //    ALTER TABLE foobar ADD COLUMN other_id INTEGER ADD CONSTRAINT (other_id) REFERENCES other(id);
      //
      // Rather than try to work it out here, we just ask the Database class to give us a suitable string template
      // where we can fill in the blanks.
      //
      // We don't particularly care about giving the foreign key constraint a name, so we don't use the more
      // complicated syntax that would be required for that.
      //
      // Note that, here, we do one column at a time because it keeps things simple - including in the case where the
      // table has no foreign keys.
      //
      BtSqlQuery sqlQuery{connection};
      for (auto const & fieldDefn: tableDefinition.tableFields) {
         if (fieldDefn.fieldType == ObjectStore::FieldType::Int &&
             std::holds_alternative<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder)) {
            auto const foreignKeyTo = std::get<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder);
            // It's obviously a programming error if the foreignKeyTo table doesn't have any fields.  (We only care
            // here about the first of those fields as, by convention, that's always the primary key on the table.)
            Q_ASSERT(foreignKeyTo->tableFields.size() > 0);

            QString queryString = QString(
               database.getSqlToAddColumnAsForeignKey()
            ).arg(
               *tableDefinition.tableName
            ).arg(
               *fieldDefn.columnName
            ).arg(
               *foreignKeyTo->tableName
            ).arg(
               *foreignKeyTo->tableFields[0].columnName
            );
            qDebug().noquote() << Q_FUNC_INFO << "Foreign keys: " << queryString;

            sqlQuery.prepare(queryString);
            if (!sqlQuery.exec()) {
               qCritical() <<
                  Q_FUNC_INFO << "Error executing database query " << queryString << ": " <<
                  sqlQuery.lastError().text();
               return false;
            }
         }
      }
      return true;
   }

   /**
    * \brief Converts a QVariant to a QString, but with a special value for a null QVariant
    */
   QString VariantToString(QVariant const & val) {
      if (val.isNull()) {
         return "[NULL]";
      }
      return val.toString();
   }

   /**
    * \brief Return a string containing all the bound values on a query.   This is quite a useful thing to have logged
    *        when you get an error!
    *
    *        NOTE: This can be a long string.  It includes newlines, and is intended to be logged with
    *              qDebug().noquote() or similar.
    */
   QString BoundValuesToString(BtSqlQuery const & sqlQuery) {
      QString result;
      QTextStream resultAsStream{&result};

      //
      // In Qt5, QSqlQuery::boundValues() returned a QMap<QString, QVariant> giving you the bound value names and
      // values.  In Qt6, QSqlQuery::boundValues() returns QVariantList of just the values.  It is not until Qt 6.6 that
      // QSqlQuery::boundValueNames() and QSqlQuery::boundValueName() are introduced.
      //
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
      for (auto const & bvn : sqlQuery.boundValueNames()) {
         resultAsStream << bvn << ": " << VariantToString(sqlQuery.boundValue(bvn)) << "\n";
      }
#else
      for (auto const & bv : sqlQuery.boundValues()) {
         resultAsStream << VariantToString(bv) << "\n";
      }
#endif
      return result;
   }

   /**
    * \brief Given a string value pulled out of the DB for an enum, look up and return its internal numerical enum
    *        equivalent.  Caller's responsibility to handle null values etc before deciding whether to call this
    *        function.
    */
   int stringToEnum(ObjectStore::TableDefinition const & primaryTable,
                    ObjectStore::TableField const &      fieldDefn,
                    QString const &                      stringValue) {
      // It's a coding error if we called this function for a non-enum field
      Q_ASSERT(fieldDefn.fieldType == ObjectStore::FieldType::Enum);
      Q_ASSERT(std::holds_alternative<EnumStringMapping const *>(fieldDefn.valueDecoder));
      auto const enumMapping = std::get<EnumStringMapping const *>(fieldDefn.valueDecoder);
      auto match = enumMapping->stringToEnumAsInt(stringValue);
      // If we didn't find a match, it's either a coding error or someone messed with the DB data
      if (!match) {
         qCritical() <<
            Q_FUNC_INFO << "Could not decode" << stringValue << "to enum when mapping column" <<
            fieldDefn.columnName << "to property" << fieldDefn.propertyName << "for" << primaryTable.tableName <<
            "so using 0";
         return 0;
      }
      return match.value();
   }

   /**
    * \brief Given a string value pulled out of the DB for a \c Measurement::Unit global constant, look up and return
    *        its address.  Caller's responsibility to handle null values etc before deciding whether to call this
    *        function.
    *
    *        See comment in \c utils/ObjectAddressStringMapping.h for when and why we store units in the DB even though
    *        we always use canonical metric units for storage.  TLDR is that when there are multiple possibilities, you
    *        can't tell the units from the "quantity" column name, so we want to be clear and unambiguous in the
    *        additional column.  (Eg "equipment.mash_tun_weight_kg" is already fine but "hop_in_inventory.quantity"
    *        could be kilograms or liters, so we want "hop_in_inventory.unit" to say which rather than have a layer of
    *        indirection behind, eg, "hop_in_inventory.measure" or some such.
    */
   Measurement::Unit const * stringToUnit(ObjectStore::TableDefinition const & primaryTable,
                                          ObjectStore::TableField const &      fieldDefn,
                                          QString const &                      stringValue) {
      // It's a coding error if we called this function for a non-unit field
      Q_ASSERT(fieldDefn.fieldType == ObjectStore::FieldType::Unit);
      Q_ASSERT(std::holds_alternative<Measurement::UnitStringMapping const *>(fieldDefn.valueDecoder));
      auto const unitMapping = std::get<Measurement::UnitStringMapping const *>(fieldDefn.valueDecoder);
      Measurement::Unit const * match = unitMapping->stringToObjectAddress(stringValue);
      // If we didn't find a match, it's either a coding error or someone messed with the DB data
      if (!match) {
         qCritical() <<
            Q_FUNC_INFO << "Could not decode" << stringValue << "to Unit when mapping column" <<
            fieldDefn.columnName << "to property" << fieldDefn.propertyName << "for" << primaryTable.tableName;
         // Stop here on debug build, as the code is unlikely to be able to recover
         Q_ASSERT(false);
      }
      return match;
   }

   //
   // Convenience functions for accessing specific fields of a JunctionTableDefinition struct
   //
   BtStringConst const & GetJunctionTableDefinitionPropertyName(ObjectStore::JunctionTableDefinition const & junctionTable) {
      return junctionTable.tableFields[2].propertyName;
   }
   BtStringConst const & GetJunctionTableDefinitionThisPrimaryKeyColumn(ObjectStore::JunctionTableDefinition const & junctionTable) {
      return junctionTable.tableFields[1].columnName;
   }
   BtStringConst const & GetJunctionTableDefinitionOtherPrimaryKeyColumn(ObjectStore::JunctionTableDefinition const & junctionTable) {
      return junctionTable.tableFields[2].columnName;
   }
   BtStringConst const & GetJunctionTableDefinitionOrderByColumn(ObjectStore::JunctionTableDefinition const & junctionTable) {
      return junctionTable.tableFields.size() > 3 ? junctionTable.tableFields[3].columnName : BtString::NULL_STR;
   }

   /**
    * \brief Insert data from an object property to a junction table
    *
    * \param junctionTable
    * \param object
    * \param primaryKey  Note that this must be supplied separately as, for a new object, we may not (yet) have set its
    *                    primary key (ie we cannot just read primary key from object)
    * \param connection
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool insertIntoJunctionTableDefinition(ObjectStore::JunctionTableDefinition const & junctionTable,
                                          QObject const & object,
                                          QVariant const & primaryKey,
                                          QSqlDatabase & connection) {
      qDebug() <<
         Q_FUNC_INFO << "Writing" << object.metaObject()->className() << "property" <<
         GetJunctionTableDefinitionPropertyName(junctionTable) << " into junction table " <<
         junctionTable.tableName;

      //
      // It's a coding error if the caller has supplied us anything other than an int inside the primaryKey QVariant.
      //
      // Here and elsewhere, although we could just do a Q_ASSERT, we prefer (a) some extra diagnostics on debug builds
      // and (b) to bail out immediately of the DB transaction on non-debug builds.
      //
      if (QMetaType::Type::Int != primaryKey.userType()) {
         qCritical() << Q_FUNC_INFO << "Unexpected contents of primaryKey QVariant: " << primaryKey.typeName();
         Q_ASSERT(false); // Stop here on debug builds
         return false;    // Continue but bail out of the current DB transaction on other builds
      }

      //
      // Construct the query
      //
      // We may be inserting more than one row.  In theory we COULD combine all the rows into a single insert statement
      // using either BtSqlQuery::execBatch() or directly constructing one of the common (but technically non-standard)
      // syntaxes, eg the following works on a lot of databases (including PostgreSQL and newer versions of SQLite) for
      // up to 1000 rows):
      //    INSERT INTO table (columnA, columnB, ..., columnN)
      //         VALUES       (r1_valA, r1_valB, ..., r1_valN),
      //                      (r2_valA, r2_valB, ..., r2_valN),
      //                      ...,
      //                      (rm_valA, rm_valB, ..., rm_valN);
      // However, we DON"T do this.  The variable binding is more complicated/error-prone than when just doing
      // individual inserts.  (Even with BtSqlQuery::execBatch(), we'd have to loop to construct the lists of bind
      // parameters.)  And there's likely no noticeable performance benefit given that we're typically inserting only
      // a handful of rows at a time (eg all the Hops in a Recipe).
      //
      // So instead, we just do individual inserts.  Note that orderByColumn column is only used if specified, and
      // that, if it is, we assume it's an integer type and that we create the values ourselves.
      //
      QString queryString{"INSERT INTO "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << junctionTable.tableName << " (" <<
         GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) << ", " <<
         GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable);
      if (!GetJunctionTableDefinitionOrderByColumn(junctionTable).isNull()) {
         queryStringAsStream << ", " << GetJunctionTableDefinitionOrderByColumn(junctionTable);
      }
      QString const thisPrimaryKeyBindName  = QString{":"} + *GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable);
      QString const otherPrimaryKeyBindName = QString{":"} + *GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable);
      QString const orderByBindName         = QString{":"} + *GetJunctionTableDefinitionOrderByColumn(junctionTable);
      queryStringAsStream << ") VALUES (" << thisPrimaryKeyBindName << ", " << otherPrimaryKeyBindName;
      if (!GetJunctionTableDefinitionOrderByColumn(junctionTable).isNull()) {
         queryStringAsStream << ", " << orderByBindName;
      }
      queryStringAsStream << ");";
      qDebug() << Q_FUNC_INFO << "Using query string" << queryString;

      //
      // Note that, when we are using bind values, we do NOT want to call the
      // BtSqlQuery::BtSqlQuery(const QString &, QSqlDatabase db) version of the BtSqlQuery constructor because that
      // would result in the supplied query being executed immediately (ie before we've had a chance to bind
      // parameters).
      //
      BtSqlQuery sqlQuery{connection};
      sqlQuery.prepare(queryString);

      // Get the list of data to bind to it
      QVariant propertyValuesWrapper = object.property(*GetJunctionTableDefinitionPropertyName(junctionTable));
      if (!propertyValuesWrapper.isValid()) {
         // It's a programming error if we couldn't read a property value
         qCritical() <<
            Q_FUNC_INFO << "Unable to read" << object.metaObject()->className() << "property" <<
            GetJunctionTableDefinitionPropertyName(junctionTable);
         Q_ASSERT(false); // Stop here on debug builds
         return false;
      }

      // We now need to extract the property values from their QVariant wrapper
      QVector<int> propertyValues;
      if (junctionTable.assumedNumEntries == ObjectStore::MAX_ONE_ENTRY) {
         // If it's single entry only, just turn it into a one-item list so that the remaining processing is the same
         bool succeeded = false;
         int theValue = propertyValuesWrapper.toInt(&succeeded);
         if (!succeeded) {
            qCritical() << Q_FUNC_INFO << "Can't convert QVariant of" << propertyValuesWrapper.typeName() << "to int";
            Q_ASSERT(false); // Stop here on debug builds
            return false;    // Continue but bail out of the current DB transaction on other builds
         }

         // If the foreign key returned is not valid, it's not an error, it just means there is no associated object,
         // eg this Hop does not have a parent.
         if (theValue <= 0) {
            qDebug() <<
               Q_FUNC_INFO << "Property" << GetJunctionTableDefinitionPropertyName(junctionTable) << "of" <<
               object.metaObject()->className() << "#" << primaryKey.toInt() << "is" << theValue <<
               "which we assume means \"unset\", so nothing to write to junction table" <<
               junctionTable.tableName;
            return true;
         }

         propertyValues.append(theValue);
      } else {
         //
         // The propertyValuesWrapper QVariant should hold QVector<int>.  If it doesn't it's a coding error (because we
         // have a property getter that's returning something else).
         //
         // Note that QVariant::toList() is NOT going to be useful to us here because that ONLY works if the contained
         // type is QList<QVariant> (aka QVariantList) or QStringList.  If your QVariant contains some other list-like
         // structure then toList() will just return an empty list.
         //
         if (!propertyValuesWrapper.canConvert< QVector<int> >()) {
            qCritical() <<
               Q_FUNC_INFO << "Can't convert QVariant of" << propertyValuesWrapper.typeName() << "to QVector<int>";
            Q_ASSERT(false); // Stop here on debug builds
            return false;    // Continue but bail out of the current DB transaction on other builds
         }
         propertyValues = propertyValuesWrapper.value< QVector<int> >();
      }

      // Now loop through and bind/run the insert query once for each item in the list
      int itemNumber = 1;
      qDebug() <<
         Q_FUNC_INFO << propertyValues.size() << "value(s) (in" << propertyValuesWrapper.typeName() <<
         ") for property" << GetJunctionTableDefinitionPropertyName(junctionTable) << "of" <<
         object.metaObject()->className() << "#" << primaryKey.toInt();
      for (int curValue : propertyValues) {
         sqlQuery.bindValue(thisPrimaryKeyBindName, primaryKey);
         sqlQuery.bindValue(otherPrimaryKeyBindName, curValue);
         if (!GetJunctionTableDefinitionOrderByColumn(junctionTable).isNull()) {
            sqlQuery.bindValue(orderByBindName, itemNumber);
         }
         qDebug() <<
            Q_FUNC_INFO <<
            GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) << " #" << primaryKey.toInt() << ":" <<
            GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable) << "N°" << itemNumber << " is #" << curValue;

         if (!sqlQuery.exec()) {
            qCritical() <<
               Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
            return false;
         }
         ++itemNumber;
      }

      return true;
   }

   /**
    * \brief Delete rows relating to a particular object from a junction table
    *
    * \param junctionTable
    * \param primaryKey
    * \param connection
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool deleteFromJunctionTableDefinition(ObjectStore::JunctionTableDefinition const & junctionTable,
                                          QVariant const & primaryKey,
                                          QSqlDatabase & connection) {

      qDebug() <<
         Q_FUNC_INFO << "Deleting property " << GetJunctionTableDefinitionPropertyName(junctionTable) <<
         " in junction table " << junctionTable.tableName;

      QString const thisPrimaryKeyBindName =
         QString{":"} + *GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable);

      // Construct the DELETE query
      QString queryString{"DELETE FROM "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream <<
         junctionTable.tableName << " WHERE " << GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) <<
         " = " << thisPrimaryKeyBindName << ";";

      BtSqlQuery sqlQuery{connection};
      sqlQuery.prepare(queryString);

      // Bind the primary key value
      sqlQuery.bindValue(thisPrimaryKeyBindName, primaryKey);
      qDebug().noquote() << Q_FUNC_INFO << "Bind values:" << BoundValuesToString(sqlQuery);

      // Run the query
      if (!sqlQuery.exec()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return false;
      }

      return true;
   }

   /**
    * \brief Force a QVariant to be a specific type.  Called from \c unwrapAndMapAsNeeded
    */
   template<typename T>
   void forceVariantToType(QVariant & propertyValue) {
      if (propertyValue.isNull()) {
         return;
      }
      propertyValue = QVariant::fromValue<T>(propertyValue.value<T>());
      return;
   }

   /**
    * \brief Used by \c wrapAndUnmapAsNeeded below.  Returns generally expected QVariant types for different types of
    *        DB fields.
    *
    *        Unfortunately, there's not an exact 1-1 mapping between our internal types and DB types, so we have to
    *        allow for a few possibilities here -- eg reading an integer out of the DB is likely to give you a QVariant
    *        of type QMetaType::LongLong.  But the check is still valuable.
    */
   QVector<int> const getExpectedTypes(ObjectStore::FieldType const fieldType) {
      switch (fieldType) {
         case ObjectStore::FieldType::Bool  : { return {QMetaType::Bool   , QMetaType::LongLong}; }
         case ObjectStore::FieldType::Int   : { return {QMetaType::Int    , QMetaType::LongLong}; }
         case ObjectStore::FieldType::UInt  : { return {QMetaType::UInt   , QMetaType::LongLong}; }
         case ObjectStore::FieldType::Double: { return {QMetaType::Double                      }; }
         case ObjectStore::FieldType::String: { return {QMetaType::QString                     }; }
         case ObjectStore::FieldType::Date  : { return {QMetaType::QDate  , QMetaType::QString }; }
         case ObjectStore::FieldType::Enum  : { return {QMetaType::QString                     }; }
         case ObjectStore::FieldType::Unit  : { return {QMetaType::QString                     }; }
         // No default case needed as compiler should warn us if any options covered above
      }
      // It's a coding error if we get here
      Q_ASSERT(false);
   }

   using TableColumnAndType = std::tuple<QString, QString, ObjectStore::FieldType>;
   /**
    * \brief Used by \c wrapAndUnmapAsNeeded below.  Returns "known bad" types of certain DB columns.  In the long run,
    *        we want to fix these!
    *
    *        There are some historical oddities in the DB schema that we have not yet fully smoothed away.  In
    *        particular:
    *          - Some booleans used to be stored as "true"/"false" strings in SQLite rather than 0/1 integers.  This
    *            does work with \c QVariant::toBool(), which will treat "", "0" and "false" as \c false and all other
    *            values as \c true, but in the longer run we'd prefer to use the DB's native bool value (0 and 1 in the
    *            case of SQLite).
    *          - Some integer foreign key columns were created without a type in SQLite, which means they get treated as
    *            strings
    *          - In another case, some foreign keys were created as a double instead of an int
    *        Rather than just say anything goes, we store the known problem columns here and log a warning about them.
    */
   QMap<TableColumnAndType, QVector<int>> const legacyBadTypes {
      {{"equipment",   "calc_boil_volume", ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"fermentable", "add_after_boil"  , ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"fermentable", "inventory_id"    , ObjectStore::FieldType::Int }, {QMetaType::Double }},
      {{"fermentable", "is_mashed"       , ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"fermentable", "recommend_mash"  , ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"hop",         "inventory_id"    , ObjectStore::FieldType::Int }, {QMetaType::Double }},
      {{"mash",        "equip_adjust"    , ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"misc",        "amount_is_weight", ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"misc",        "inventory_id"    , ObjectStore::FieldType::Int }, {QMetaType::QString}},
      {{"yeast",       "add_to_secondary", ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"yeast",       "amount_is_weight", ObjectStore::FieldType::Bool}, {QMetaType::QString}},
      {{"yeast",       "inventory_id"    , ObjectStore::FieldType::Int }, {QMetaType::Double }},
   };

}

ObjectStore::TableField::TableField(ObjectStore::FieldType                 const   fieldType,
                                    char const *                           const   columnName,
                                    BtStringConst                          const & propertyName,
                                    ObjectStore::TableField::ValueDecoder  const   valueDecoder) :
   fieldType{fieldType},
   columnName{columnName},
   propertyName{propertyName},
   valueDecoder{valueDecoder} {


   return;
}

ObjectStore::TableDefinition::TableDefinition(char const * const tableName,
                                              std::initializer_list<TableField> const tableFields) :
         tableName{tableName},
         tableFields{tableFields} {

   //
   // Uncomment the following if trying to debug issues with foreign keys.
   //
//   for (auto const & fieldDefn: this->tableFields) {
//      if (std::holds_alternative<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder)) {
//         auto tableDefinition{std::get<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder)};
//         // If something is wrong we need to log diagnostics before we exit, otherwise it will be hard to know which
//         // field definition caused the problem!
//         if (tableDefinition->tableName.isNull()) {
//            qCritical() << Q_FUNC_INFO << "Foreign key table for column" << fieldDefn.columnName << "has no name!";
//            exit(EXIT_FAILURE);
//         }
//         if (0 == tableDefinition->tableFields.size()) {
//            qCritical() << Q_FUNC_INFO << "Foreign key table for column" << fieldDefn.columnName << "has no columns!";
//            exit(EXIT_FAILURE);
//         }
//         qDebug() <<
//            Q_FUNC_INFO << "Table" << tableName << "foreign key" << fieldDefn.columnName << "points to table "
//            "definition for table" << *tableDefinition->tableName << "with" << tableDefinition->tableFields.size() <<
//            "columns";
//      }
//   }

   return;
}

// This private implementation class holds all private non-virtual members of ObjectStore
class ObjectStore::impl {
public:

   /**
    * Constructor
    */
   impl(char const *             const   className,
        TypeLookup               const & typeLookup,
        TableDefinition          const & primaryTable,
        JunctionTableDefinitions const & junctionTables) : m_className{className},
                                                           m_state{ObjectStore::State::NotYetInitialised},
                                                           typeLookup{typeLookup},
                                                           primaryTable{primaryTable},
                                                           junctionTables{junctionTables},
                                                           allObjects{},
                                                           database{nullptr} {
      return;
   }

   ~impl() = default;

   /**
    * \brief This function does any required special handling for optional and/or enum fields retrieved from a
    *        \c NamedEntity or subclass thereof.  It takes the \c QVariant returned from \c QObject::property() and does
    *        any necessary conversion to turn it into a \c QVariant that we can bind to a SQL query for inserting or
    *        updating data in the DB.
    *
    *        Enums are stored in the DB as strings rather than the raw int value of the enum.  This is because (a) it is
    *        (or at least should be) less fragile and (b) it makes debugging easier.
    *
    *        Optional (aka nullable) fields need some special handling.  Although \c QVariant already has the concept of
    *        null values, this is not an inherent part of the QProperty system.  If you have, say, a double QProperty
    *        that you want to be nullable then you have to put either a \c QVariant or \c std::optional wrapper around
    *        it, which in turn will get wrapped inside \c QVariant when getting the value via \c QObject::property().
    *        (Yes, you can have a \c QVariant inside a \c QVariant!  But we use \c std::optional as our inner wrapping
    *        because it's more strongly typed.)  Two layers of wrapping is one too many, so we need to remove the inner
    *        layer if it is present.
    *
    *        Optional enums need both sets of processing.
    *
    * \param primaryTable is only needed for logging
    * \param fieldDefn
    * \param propertyValue
    */
   void unwrapAndMapAsNeeded(ObjectStore::TableDefinition const & primaryTable,
                             ObjectStore::TableField const & fieldDefn,
                             QVariant & propertyValue) {

      // It's a coding error if we don't have an enum mapping for an enum field
      if (ObjectStore::FieldType::Enum == fieldDefn.fieldType &&
         !std::holds_alternative<EnumStringMapping const *>(fieldDefn.valueDecoder)) {
         qCritical() <<
            Q_FUNC_INFO << "Coding Error!  No enum mapping found to map property " << fieldDefn.propertyName <<
            " to column " << fieldDefn.columnName << "for" << primaryTable.tableName;
         Q_ASSERT(false);
      }

      // Similarly, it's a coding error if we don't have a unit name mapping for a unit field
      if (ObjectStore::FieldType::Unit == fieldDefn.fieldType &&
         !std::holds_alternative<Measurement::UnitStringMapping const *>(fieldDefn.valueDecoder)) {
         qCritical() <<
            Q_FUNC_INFO << "Coding Error!  No unit name mapping found to map property " << fieldDefn.propertyName <<
            " to column " << fieldDefn.columnName << "for" << primaryTable.tableName;
         Q_ASSERT(false);
      }

      if (this->typeLookup.getType(fieldDefn.propertyName).isOptional()) {
         //
         // This is an optional field, so we are converting a QVariant holding std::optional<T> to a QVariant holding
         // either T or null, with relevant special case handling for when T is actually an enum (where we need to
         // convert it to QString if it's not null).
         //
         // Removing the optional wrapper has a side-effect of "cleaning" the QVariant, so we don't need to do the
         // extra processing below.
         //
         switch (fieldDefn.fieldType) {
            case ObjectStore::FieldType::Bool:   { Optional::removeOptionalWrapper<bool        >(propertyValue); return; }
            case ObjectStore::FieldType::Int:    { Optional::removeOptionalWrapper<int         >(propertyValue); return; }
            case ObjectStore::FieldType::UInt:   { Optional::removeOptionalWrapper<unsigned int>(propertyValue); return; }
            case ObjectStore::FieldType::Double: { Optional::removeOptionalWrapper<double      >(propertyValue); return; }
            case ObjectStore::FieldType::String: { Optional::removeOptionalWrapper<QString     >(propertyValue); return; }
            case ObjectStore::FieldType::Date:   { Optional::removeOptionalWrapper<QDate       >(propertyValue); return; }
            case ObjectStore::FieldType::Enum:   {
               auto const enumMapping = std::get<EnumStringMapping const *>(fieldDefn.valueDecoder);
               auto val = propertyValue.value<std::optional<int> >();
               if (val.has_value()) {
                  propertyValue = QVariant(enumMapping->enumToString(val.value()));
                  return;
               }
               propertyValue = QVariant();
               return;
            }
            case ObjectStore::FieldType::Unit: {
               // Since Unit is stored as a pointer, it is never wrapped in std::optional, so it's a coding error if we
               // get here
               Q_ASSERT(false);
               propertyValue = QVariant();
               return;

            }
            // No default case needed as compiler should warn us if any options covered above
         }
         // It's a coding error if we get here!
         Q_ASSERT(false);
      }

      //
      // In certain circumstances, the QVariant we've been given from the Qt properties system could be unsuitable for
      // passing as a bind parameter to a QSqlQuery object.  The QSqlQuery object is always going to call the toString()
      // member function on the QVariant, and this may give the wrong result in the following circumstances:
      //   - If we have a minor coding error in the model class and, eg, specify an int property as being double then
      //     we'll end up inserting '123.0' instead of '123' into the DB.  If we're lucky, the DB converts this back to
      //     an integer (123).  If we're not (because SQLite without STRICT Tables mode will just take any value for any
      //     field) then the DB will actually store a double in an int column and we'll be at least confused down the
      //     line
      //   - If we are storing an enum as an int in the DB, then the QVariant holding the enum is going to be two-faced.
      //     It will give you the _name_ of the enum value when you call .toString() and the int _value_ of the enum
      //     value when you call toInt().  So, on PostgreSQL, we'll get a DB error (for trying to store a string in an
      //     int column) and on SQLite we'll get a string value stored in an int column.
      //
      // These are admittedly edge cases, but the safest thing is to take control over the QVariant type and effectively
      // force it to always give the expected results when its toString() member function is called.
      //
      // (One day we will perhaps move to STRICT Tables -- see https://www.sqlite.org/stricttables.html -- on SQLite,
      // which will alleviate this problem.)
      //
      switch (fieldDefn.fieldType) {
         case ObjectStore::FieldType::Bool:   { forceVariantToType<bool        >(propertyValue); return; }
         case ObjectStore::FieldType::Int:    { forceVariantToType<int         >(propertyValue); return; }
         case ObjectStore::FieldType::UInt:   { forceVariantToType<unsigned int>(propertyValue); return; }
         case ObjectStore::FieldType::Double: { forceVariantToType<double      >(propertyValue); return; }
         case ObjectStore::FieldType::String: { forceVariantToType<QString     >(propertyValue); return; }
         case ObjectStore::FieldType::Date:   { forceVariantToType<QDate       >(propertyValue); return; }
         case ObjectStore::FieldType::Enum:   {
            // This is a non-optional enum, so we need to map it to a QString
            auto const enumMapping = std::get<EnumStringMapping const *>(fieldDefn.valueDecoder);
            propertyValue = QVariant(enumMapping->enumToString(propertyValue.toInt()));
            return;
         }
         case ObjectStore::FieldType::Unit:   {
            auto const unitMapping = std::get<Measurement::UnitStringMapping const *>(fieldDefn.valueDecoder);

            propertyValue =
               QVariant(unitMapping->objectAddressToString(propertyValue.value<Measurement::Unit const *>()));
            return;
         }
         // No default case needed as compiler should warn us if any options covered above
      }

      // It's a coding error if we get here
      Q_ASSERT(false);
   }

   /**
    * \brief This is the inverse of \c unwrapAndMapAsNeeded, used for converting a \c QVariant value read out of the
    *        database into a \c QVariant value that we can put in a Qt property of a \c NamedEntity (or subclass
    *        thereof).
    *
    * \param primaryTable This is used only for logging errors (in case there is bad data in the DB, which could happen
    *                     if the DB has been manually edited or partially restored from an old verison etc.
    * \param fieldDefn
    * \param valueFromDb the QVariant that we may need to modify
    */
   void wrapAndUnmapAsNeeded(ObjectStore::TableDefinition const & primaryTable,
                             ObjectStore::TableField const & fieldDefn,
                             QVariant & propertyValue) {
      //
      // If it is not null (when the type info is not meaningful), we would like to check that the QVariant we've
      // received back from the QSqlQuery object is a sane type.  If it isn't then it could indicate either a past or
      // current coding error, or some manual edit of the DB.  Either way we at least want to log a warning.
      //
      if (!propertyValue.isNull()) {
         auto expectedTypes = getExpectedTypes(fieldDefn.fieldType);

         // NB: In Qt 6, QVariant::type() becomes QVariant::typeId()
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            int const propertyType = propertyValue.type();
#else
            int const propertyType = propertyValue.typeId();
#endif
         if (!expectedTypes.contains(propertyType)) {
            TableColumnAndType tableColumnAndType{*primaryTable.tableName, *fieldDefn.columnName, fieldDefn.fieldType};
            if (legacyBadTypes.contains(tableColumnAndType) &&
               legacyBadTypes.value(tableColumnAndType).contains(propertyType)) {
               // It's technically wrong but we know about it and it works, so just log it.  If this logging is
               // uncommented, you can get a list of all the things we need to fix with:
               //   grep "known ugliness" *.log | sed 's/^.*property /Property /; s/This is a known ugliness .*$//' | sort -u
///               qDebug() <<
///                  Q_FUNC_INFO << fieldDefn.fieldType << "property" << fieldDefn.propertyName << "on table" <<
///                  primaryTable.tableName << "(value " << propertyValue << ") is stored as " <<
///                  propertyValue.typeName() << "(" << propertyType << ") in column" << fieldDefn.columnName <<
///                  ".  This is a known ugliness that we intend to fix one day.";
            } else {
               //
               // It's not a known exception, so it's a coding error.  However, we can recover in the following cases:
               //    - If we are expecting a boolean and we get a string holding "true" or "false" etc, then we know
               //      what to do.
               //    - If we are expecting an int and we get a double then we can just ignore the non-integer part of
               //      the number.
               //    - If we are expecting an double and we got an int then we can just upcast it.
               //
               bool recovered = false;
               QVariant readPropertyValue = propertyValue;
               if (propertyType == QMetaType::QString && fieldDefn.fieldType == ObjectStore::FieldType::Bool) {
                  // We'll take any reasonable string representation of true/false.  For the moment, at least, I'm not
                  // worrying about optional fields here.  I think it's pretty rare, if ever, that we'd want an optional
                  // boolean.
                  QString propertyAsLcString = propertyValue.toString().trimmed().toLower();
                  if (propertyAsLcString == "true" || propertyAsLcString == "t" || propertyAsLcString == "1") {
                     recovered = true;
                     propertyValue = QVariant(true);
                  } else if (propertyAsLcString == "false" || propertyAsLcString == "f" || propertyAsLcString == "0") {
                     recovered = true;
                     propertyValue = QVariant(false);
                  }
               } else if (propertyType == QMetaType::Double && fieldDefn.fieldType == ObjectStore::FieldType::Int) {
                  propertyValue = QVariant(propertyValue.toInt(&recovered));
               } else if (propertyType == QMetaType::Int && fieldDefn.fieldType == ObjectStore::FieldType::Double) {
                  propertyValue = QVariant(propertyValue.toDouble(&recovered));
               }
               if (recovered) {
                  qWarning() <<
                     Q_FUNC_INFO << "Recovered from unexpected type #" << propertyType << "=" <<
                     readPropertyValue.typeName() << "in QVariant for property" << fieldDefn.propertyName <<
                     ", field type" << fieldDefn.fieldType << ", value" << readPropertyValue << ", table" <<
                     primaryTable.tableName << ", column" << fieldDefn.columnName << ".  Interpreted value as" <<
                     propertyValue;
               } else {
                  //
                  // Even in the case where we do not have a reasonable way to interpret the data in this column, we
                  // should probably NOT terminate the program.  We are still discovering lots of cases where folks
                  // using the software have old Brewtarget databases with quirky values in some columns.  It's better
                  // from a user point of view that the software carries on working even if some (hopefully) obscure
                  // field could not be read from the DB.
                  //
                  qCritical() <<
                     Q_FUNC_INFO << "Unexpected type #" << propertyType << "=" << propertyValue.typeName() <<
                     "in QVariant for property" << fieldDefn.propertyName << ", field type" << fieldDefn.fieldType <<
                     ", value" << propertyValue << ", table" << primaryTable.tableName << ", column" <<
                     fieldDefn.columnName;
                  // If, during development or debugging, you want to have the program stop when it cannot interpret
                  // one of the DB fields, then uncomment the following two lines.
///                  qCritical().noquote() << Q_FUNC_INFO << "Call stack is:" << Logging::getStackTrace();
///                  Q_ASSERT(false);
               }

            }
         }
      }

      if (this->typeLookup.getType(fieldDefn.propertyName).isOptional()) {
         //
         // This is an optional field, so we are converting from a QVariant holding either T or null to a QVariant
         // holding std::optional<T>, with relevant special case handling for when T is actually an enum (where we need
         // to convert it from QString to int if it's not null).
         //
         // Adding the optional wrapper has a side-effect of "cleaning" the QVariant, so we don't need to do the extra
         // processing below.
         //
         switch (fieldDefn.fieldType) {
            case ObjectStore::FieldType::Bool:   { Optional::insertOptionalWrapper<bool        >(propertyValue); return; }
            case ObjectStore::FieldType::Int:    { Optional::insertOptionalWrapper<int         >(propertyValue); return; }
            case ObjectStore::FieldType::UInt:   { Optional::insertOptionalWrapper<unsigned int>(propertyValue); return; }
            case ObjectStore::FieldType::Double: { Optional::insertOptionalWrapper<double      >(propertyValue); return; }
            case ObjectStore::FieldType::String: { Optional::insertOptionalWrapper<QString     >(propertyValue); return; }
            case ObjectStore::FieldType::Date:   { Optional::insertOptionalWrapper<QDate       >(propertyValue); return; }
            case ObjectStore::FieldType::Enum:   {
               if (propertyValue.isNull()) {
                  propertyValue = QVariant::fromValue(std::optional<int>());
               } else {
                  propertyValue = QVariant::fromValue(
                     std::optional<int>(stringToEnum(primaryTable, fieldDefn, propertyValue.toString()))
                  );
               }
               return;
            }
            case ObjectStore::FieldType::Unit:   {
               // We don't need to support optional units, so it's a coding error if we get here
               Q_ASSERT(false);
            }
            // No default case needed as compiler should warn us if any options covered above
         }
         // It's a coding error if we get here!
         Q_ASSERT(false);
      }

      //
      // Although it's less of a worry when reading from the DB, for consistency, we still want to "clean" the QVariant
      // as we do in unwrapAndMapAsNeeded() above.
      //
      // Additionally, if a non-optional int is NULL in the DB then we'll hold it as -1, as this is more ambiguously
      // "not set" than the "default" value of 0.
      //
      // I _think_ this is always a valid & sensible thing to do.  If we decide later we need to have exceptions then we
      // could either move this logic into the constructors and NamedParameterBundle or be more rigorous about
      // back-applying std::optional wrappers to existing int object properties.
      //
      //
      switch (fieldDefn.fieldType) {
         case ObjectStore::FieldType::Bool:   { forceVariantToType<bool        >(propertyValue); return; }
         case ObjectStore::FieldType::Int:    { forceVariantToType<int         >(propertyValue); // Continues to next line
                                                if (propertyValue.isNull()) { propertyValue = QVariant(-1); } return; }
         case ObjectStore::FieldType::UInt:   { forceVariantToType<unsigned int>(propertyValue); return; }
         case ObjectStore::FieldType::Double: { forceVariantToType<double      >(propertyValue); return; }
         case ObjectStore::FieldType::String: { forceVariantToType<QString     >(propertyValue); return; }
         case ObjectStore::FieldType::Date:   { forceVariantToType<QDate       >(propertyValue); return; }
         case ObjectStore::FieldType::Enum:   {
            // This is a non-optional enum, so we need to map from a QString to an int
            if (propertyValue.isNull()) {
               // This is either a coding error or someone messed with the DB data.
               qCritical() <<
                  Q_FUNC_INFO << "Found null value for non-optional enum when mapping column " <<
                  fieldDefn.columnName << " to property " << fieldDefn.propertyName << "for" <<
                  primaryTable.tableName << "so using 0";
               propertyValue = QVariant(0);
               return;
            }

            propertyValue = QVariant(stringToEnum(primaryTable, fieldDefn, propertyValue.toString()));
            return;
         }
         case ObjectStore::FieldType::Unit:   {
            if (propertyValue.isNull()) {
               // This is either a coding error or someone messed with the DB data.
               qCritical() <<
                  Q_FUNC_INFO << "Found null value for non-optional Unit when mapping column " <<
                  fieldDefn.columnName << " to property " << fieldDefn.propertyName << "for" <<
                  primaryTable.tableName;
               // Not sure we can recover here, so bail on debug builds
               Q_ASSERT(false);
               return;
            }

            propertyValue = QVariant::fromValue<Measurement::Unit const *>(
               stringToUnit(primaryTable, fieldDefn, propertyValue.toString())
            );
            return;
         }
         // No default case needed as compiler should warn us if any options covered above
      }

      // It's a coding error if we get here!
      Q_ASSERT(false);
   }

   /**
    * \brief Append, to the supplied query string we are constructing, a comma-separated list of all the column names
    *        for the table, in the order of this->primaryTable.tableFields
    *
    * \param queryStringAsStream
    * \param includePrimaryKey  Usually \c true for SELECT and UPDATE, and \c false for INSERT
    * \param prependColons Set to \c true if we are appending bind values
    */
   void appendColumNames(QTextStream & queryStringAsStream, bool includePrimaryKey, bool prependColons) {
      bool skippedPrimaryKey = false;
      bool firstFieldOutput = false;
      for (auto const & fieldDefn: this->primaryTable.tableFields) {
         if (!includePrimaryKey && !skippedPrimaryKey) {
            // By convention the first field is the primary key
            skippedPrimaryKey = true;
         } else {
            if (!firstFieldOutput) {
               firstFieldOutput = true;
            } else {
               queryStringAsStream << ", ";
            }
            if (prependColons) {
               queryStringAsStream << ":";
            }
            queryStringAsStream << fieldDefn.columnName;
         }
      }
      return;
   }

   /**
    * \brief Get the name of the DB column that holds the primary key
    */
   BtStringConst const & getPrimaryKeyColumn() {
      // By convention the first field is the primary key
      return this->primaryTable.tableFields[0].columnName;
   };

   /**
    * \brief Get the name of the object property that holds the primary key
    */
   BtStringConst const & getPrimaryKeyProperty() {
      // By convention the first field is the primary key
      return this->primaryTable.tableFields[0].propertyName;
   };

   /**
    * \brief Extract the primary key from an object
    */
   QVariant getPrimaryKey(QObject const & object) {
      return object.property(*getPrimaryKeyProperty());
   }

   /**
    * \brief Update the specified property on an object
    *
    *        NB: Caller is responsible for handling transactions
    *
    * \return \c true if succeeded, \c false otherwise
    */
   bool updatePropertyInDb(QSqlDatabase & connection, QObject const & object, BtStringConst const & propertyName) {
      // We'll need some of this info even if it's a junction table property we're updating
      BtStringConst const & primaryKeyColumn {this->getPrimaryKeyColumn()};
      QVariant const        primaryKey       {this->getPrimaryKey(object)};

      //
      // First check whether this is a simple property.  (If not we look for it in the ones we store in junction
      // tables.)
      //
      auto matchingFieldDefn = std::find_if(
         this->primaryTable.tableFields.begin(),
         this->primaryTable.tableFields.end(),
         [propertyName](TableField const & fd) {return fd.propertyName == propertyName;}
      );

      if (matchingFieldDefn != this->primaryTable.tableFields.end()) {
         //
         // We're updating a simple property
         //
         // Construct the SQL, which will be of the form
         //
         //    UPDATE tablename
         //    SET columnName = :columnName
         //    WHERE primaryKeyColumn = :primaryKeyColumn;
         //
         QString queryString{"UPDATE "};
         QTextStream queryStringAsStream{&queryString};
         queryStringAsStream << this->primaryTable.tableName << " SET ";

         BtStringConst const & columnToUpdateInDb = matchingFieldDefn->columnName;

         queryStringAsStream << " " << columnToUpdateInDb << " = :" << columnToUpdateInDb;
         queryStringAsStream << " WHERE " << primaryKeyColumn << " = :" << primaryKeyColumn << ";";

         qDebug() <<
            Q_FUNC_INFO << "Updating" << object.metaObject()->className() << "property" << propertyName <<
            "with database query" << queryString;
         // Normally leave the next debug output commented, as it can generate a lot of logging.  But it's useful to
         // uncomment if you're seeing a lot of DB updates and the cause is not clear.
//         qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();

         //
         // Bind the values
         //
         BtSqlQuery sqlQuery{connection};
         sqlQuery.prepare(queryString);
         QVariant propertyBindValue{object.property(*propertyName)};
         // It's a coding error if the property we are trying to read from does not exist
         Q_ASSERT(propertyBindValue.isValid());
         auto fieldDefn = std::find_if(
            this->primaryTable.tableFields.begin(),
            this->primaryTable.tableFields.end(),
            [propertyName](TableField const & fd){return propertyName == fd.propertyName;}
         );

         // It's a coding error if we're trying to update a property that's not in the field definitions
         Q_ASSERT(fieldDefn != this->primaryTable.tableFields.end());

         // Fix-up the QVariant if needed, including converting enums to strings
         this->unwrapAndMapAsNeeded(this->primaryTable, *fieldDefn, propertyBindValue);

         if (std::holds_alternative<ObjectStore::TableDefinition const *>(fieldDefn->valueDecoder)) {
            //
            // If the columns if a foreign key and the caller is setting it to a non-positive value then we actually
            // need to store NULL in the DB.  (In the code we store foreign key IDs as ints, and use -1 to mean null.
            // In the DB we need to store NULL explicitly because, if we try to store -1, we'll get a foreign key
            // constraint violation as the DB is unable to find a row in the related table with primary key -1.)
            //
            // Firstly, we assert it's a coding error if we've created a foreign key column that's not an int.  For the
            // moment at least, we don't support other types of primary/foreign key.
            //
            Q_ASSERT(ObjectStore::FieldType::Int == fieldDefn->fieldType);
            if (propertyBindValue.toInt() <= 0) {
               qDebug() << Q_FUNC_INFO << "Treating" << propertyBindValue << "foreign key value as NULL";
               propertyBindValue = QVariant{QMetaType{QMetaType::Int}};
            }
         }
         sqlQuery.bindValue(QString{":%1"}.arg(*columnToUpdateInDb), propertyBindValue);
         sqlQuery.bindValue(QString{":%1"}.arg(*primaryKeyColumn), primaryKey);
         qDebug().noquote() << Q_FUNC_INFO << "Bind values:" << BoundValuesToString(sqlQuery);

         //
         // Run the query
         //
         if (!sqlQuery.exec()) {
            qCritical() <<
               Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
            return false;
         }
      } else {
         //
         // The property we've been given isn't a simple property, so look for it in the ones we store in junction
         // tables
         //
         auto matchingJunctionTableDefinitionDefn = std::find_if(
            this->junctionTables.begin(),
            this->junctionTables.end(),
            [propertyName](JunctionTableDefinition const & jt) {
               return GetJunctionTableDefinitionPropertyName(jt) == propertyName;
            }
         );

         // It's a coding error if we couldn't find the property either as a simple field or an associative entity
         if (matchingJunctionTableDefinitionDefn == this->junctionTables.end()) {
            qCritical() <<
               Q_FUNC_INFO << "Unable to find rule for storing property" << object.metaObject()->className() << "::" <<
               propertyName << "in either" << this->primaryTable.tableName << "or any associated table";
            qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
            Q_ASSERT(false);
         }

         //
         // As elsewhere, the simplest way to update a junction table is to blat any rows relating to the current object
         // and then write out data based on the current property values.
         //
         qDebug() <<
            Q_FUNC_INFO << "Updating" << object.metaObject()->className() << "property" << propertyName <<
            "in junction table" << matchingJunctionTableDefinitionDefn->tableName;
         if (!deleteFromJunctionTableDefinition(*matchingJunctionTableDefinitionDefn, primaryKey, connection)) {
            return false;
         }
         if (!insertIntoJunctionTableDefinition(*matchingJunctionTableDefinitionDefn, object, primaryKey, connection)) {
            return false;
         }
      }

      // If we made it this far then everything worked
      return true;
   }

   /**
    * \brief Insert an object in the database
    *
    *        NB: Caller is responsible for handling transactions
    *
    * \param connection
    * \param object
    * \param writePrimaryKey Normally this is \c false, meaning we are going to let the DB assign a primary key to the
    *                        new object we are inserting.  However, if we are writing existing objects out to a new
    *                        database, then this will be \c true, meaning we write out the existing primary keys (to
    *                        keep any foreign key references to them valid.  (In this latter circumstance, we are also
    *                        assuming the caller has disabled foreign key constraints for the duration of the
    *                        transaction.)
    *
    * \return the primary key of the inserted object, or -1 if there was an error.  Note that, in the case that
    *         \c writePrimaryKey is \c false (ie we are inserting a new object), it is the \b caller's responsibility to
    *         update the object with its new primary key.
    */
   int insertObjectInDb(QSqlDatabase & connection, QObject const & object, bool writePrimaryKey) {
      //
      // Construct the SQL, which will be of the form
      //
      //    INSERT INTO tablename (firstColumn, secondColumn, ...)
      //    VALUES (:firstColumn, :secondColumn, ...);
      //
      // We omit the primary key column because we can't know its value in advance.  We'll find out what value the DB
      // assigned to it after the query was run -- see below.
      //
      QString queryString{"INSERT INTO "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << this->primaryTable.tableName << " (";
      this->appendColumNames(queryStringAsStream, writePrimaryKey, false);
      queryStringAsStream << ") VALUES (";
      this->appendColumNames(queryStringAsStream, writePrimaryKey, true);
      queryStringAsStream << ");";

      qDebug() <<
         Q_FUNC_INFO << "Inserting" << object.metaObject()->className() << "main table row with database query " <<
         queryString;
      // Uncomment the following to track down errors where we're trying to insert an object to the database twice
//      qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();

      //
      // Bind the values
      //
      BtSqlQuery sqlQuery{connection};
      sqlQuery.prepare(queryString);
      for (int ii = (writePrimaryKey ? 0 : 1); ii < this->primaryTable.tableFields.size(); ++ii) {
         auto const & fieldDefn = this->primaryTable.tableFields[ii];

         QVariant bindValue{object.property(*fieldDefn.propertyName)};
         // Uncomment the following line if the assert below is firing
         qDebug() << Q_FUNC_INFO << fieldDefn.propertyName << ":" << bindValue;

         // It's a coding error if the property we are trying to read from does not exist
         Q_ASSERT(bindValue.isValid());

         // Fix-up the QVariant if needed, including converting enums to strings
         this->unwrapAndMapAsNeeded(this->primaryTable, fieldDefn, bindValue);

         if (std::holds_alternative<ObjectStore::TableDefinition const *>(fieldDefn.valueDecoder) && bindValue.toInt() <= 0) {
            // If the field is a foreign key and the value we would otherwise put in it is not a valid key (eg we are
            // inserting a Recipe on which the Equipment has not yet been set) then the query would barf at the invalid
            // key.  So, in this case, we need to insert NULL.
            bindValue = QVariant();
         }

         sqlQuery.bindValue(QString{":"} + *fieldDefn.columnName, bindValue);
      }

      qDebug().noquote() << Q_FUNC_INFO << "Bind values:" << BoundValuesToString(sqlQuery);

      //
      // Run the query
      //
      if (!sqlQuery.exec()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return -1;
      }

      int currentPrimaryKey = object.property(*this->getPrimaryKeyProperty()).toInt();
      int primaryKeyInDb;
      if (writePrimaryKey) {
         //
         // We already know the ID of the object we wrote to the database, so we don't need to ask the database what it
         // was (and indeed attempts to do so may not work, per the comment below in the other branch of this if
         // statement.
         //
         primaryKeyInDb = currentPrimaryKey;
      } else {
         //
         // If we didn't have an ID and asked the DB to generate one, we need to find out what it was.
         //
         // Assert that we are only using database drivers that support returning the last insert ID.  (It is
         // frustratingly hard to find documentation about this, as, eg, https://doc.qt.io/qt-5/sql-driver.html does not
         // explicitly list which supplied drivers support which features.  However, in reality, we know SQLite and
         // PostgreSQL drivers both support this, so it would likely only be a problem if a new type of DB were
         // introduced.)
         //
         // It is important to be aware that this feature may not return a meaningful result in the case that we,
         // explicitly wrote the ID.  (Eg in PostgreSQL there will be a sequence for generating IDs that needs to get
         // reset after you've done one or more inserts with explicitly set IDs.  Asking for the last insert ID gets the
         // current state of the sequence and so only works when the sequence was used to generate the ID.)
         //
         // Note too that we have to explicitly put the primary key into an int, because, by default it might come back
         // as long long int rather than int (ie 64-bits rather than 32-bits in the C++ implementations we care about).
         //
         Q_ASSERT(sqlQuery.driver()->hasFeature(QSqlDriver::LastInsertId));
         QVariant rawPrimaryKey = sqlQuery.lastInsertId();
         Q_ASSERT(rawPrimaryKey.canConvert<int>());
         primaryKeyInDb = rawPrimaryKey.toInt();

         //
         // Where we're not explicitly writing the primary key it's because the object we are inserting should not
         // already have a valid primary key.
         //
         // Where we are writing the primary key, it shouldn't get changed by the write.
         //
         // .:TBD:. Maybe if we're doing undelete, this is the place to handle that case.
         //
         if (currentPrimaryKey > 0) {
            // This is almost certainly a coding error
            qCritical() <<
               Q_FUNC_INFO << "Wrote new" << object.metaObject()->className() << " to database (with primary key " <<
               primaryKeyInDb << ") but it already had primary key" << currentPrimaryKey;
            qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
            Q_ASSERT(false); // Stop here on debug build
         }
      }

      qDebug() <<
         Q_FUNC_INFO << object.metaObject()->className() << "#" << primaryKeyInDb << "inserted in database using" <<
         queryString;

      //
      // Now save data to the junction tables
      //
      for (auto const & junctionTable : this->junctionTables) {
         if (!insertIntoJunctionTableDefinition(junctionTable, object, primaryKeyInDb, connection)) {
            qCritical() <<
               Q_FUNC_INFO << "Error writing to junction tables:" << connection.lastError().text();
            return -1;
         }
      }

      return primaryKeyInDb;
   }

   char const * const m_className;
   ObjectStore::State m_state;
   TypeLookup const & typeLookup;
   TableDefinition const & primaryTable;
   JunctionTableDefinitions const & junctionTables;
   QHash<int, std::shared_ptr<QObject> > allObjects;
   Database * database;
};

QString ObjectStore::getDisplayName(ObjectStore::FieldType const fieldType) {
   switch (fieldType) {
      case ObjectStore::FieldType::Bool  : return "ObjectStore::FieldType::Bool"  ;
      case ObjectStore::FieldType::Int   : return "ObjectStore::FieldType::Int"   ;
      case ObjectStore::FieldType::UInt  : return "ObjectStore::FieldType::UInt"  ;
      case ObjectStore::FieldType::Double: return "ObjectStore::FieldType::Double";
      case ObjectStore::FieldType::String: return "ObjectStore::FieldType::String";
      case ObjectStore::FieldType::Date  : return "ObjectStore::FieldType::Date"  ;
      case ObjectStore::FieldType::Enum  : return "ObjectStore::FieldType::Enum"  ;
      case ObjectStore::FieldType::Unit  : return "ObjectStore::FieldType::Unit"  ;
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here
   Q_ASSERT(false);
}

ObjectStore::ObjectStore(char const *             const   className,
                         TypeLookup               const & typeLookup,
                         TableDefinition          const & primaryTable,
                         JunctionTableDefinitions const & junctionTables) :
   pimpl{ std::make_unique<impl>(className, typeLookup, primaryTable, junctionTables) } {
   qDebug() << Q_FUNC_INFO << "Construct of object store for primary table" << this->pimpl->primaryTable.tableName;
   // We have seen a circumstance where primaryTable.tableName is null, which shouldn't be possible.  This is some
   // diagnostic to try to find out why.
   if (this->pimpl->primaryTable.tableName.isNull()) {
      qCritical().noquote() << Q_FUNC_INFO << "Primary table without name.  Call stack is:" << Logging::getStackTrace();
   }
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
ObjectStore::~ObjectStore() {
   // Normally we try to avoid logging things here, as it's possible that the objects used in Logging.cpp have already
   // been destroyed, but it can be useful to turn this on when debugging ObjectStore problems.
   //qDebug() <<
   //   Q_FUNC_INFO << "Destruct of object store for primary table" << this->pimpl->primaryTable.tableName <<
   //   "(containing" << this->pimpl->allObjects.size() << "objects)";
   return;
}

QString ObjectStore::name() const {
   return this->pimpl->m_className;
}

ObjectStore::State ObjectStore::state() const {
   return this->pimpl->m_state;
}

void ObjectStore::logDiagnostics() const {
   for (int key : this->pimpl->allObjects.keys()) {
      std::shared_ptr<QObject> object = this->pimpl->allObjects.value(key);
      qDebug() <<
         Q_FUNC_INFO << "Object @" << static_cast<void *>(object.get()) << "stored as #" << key << "has key" <<
         this->pimpl->getPrimaryKey(*object) << "and shared pointer use count" << object.use_count();
   }
   return;
}

// Note that we have to pass Database in as a parameter because, ultimately, we're being called from Database::load()
// which is called from Database::getInstance(), so we don't want to get in an endless loop.
bool ObjectStore::createTables(Database & database, QSqlDatabase & connection) const {
   //
   // Note that we are not putting in foreign key constraints here, as we don't want to have to care about the order in
   // which tables are created.  The constraints are added subsequently by calls to addTableConstraints();
   //
   // Note too, that we don't care about default values as we assume we will always provide values for all columns when
   // we do an insert.  (Suitable default values for object fields are set in the object's constructor.)
   //
   if (!createTableWithoutForeignKeys(database, connection, this->pimpl->primaryTable)) {
      return false;
   }

   //
   // Now create the junction tables
   //
   for (auto const & junctionTable : this->pimpl->junctionTables) {
      if (!createTableWithoutForeignKeys(database, connection, junctionTable)) {
         return false;
      }
   }

   return true;
}

// Note that we have to pass Database in as a parameter because, ultimately, we're being called from Database::load()
// which is called from Database::getInstance(), so we don't want to get in an endless loop.
bool ObjectStore::addTableConstraints(Database & database, QSqlDatabase & connection) const {
   // This is all pretty much the same structure as createTables(), so I won't repeat all the comments here

   if (!addForeignKeysToTable(database, connection, this->pimpl->primaryTable)) {
      return false;
   }

   for (auto const & junctionTable : this->pimpl->junctionTables) {
      if (!addForeignKeysToTable(database, connection, junctionTable)) {
         return false;
      }
   }

   return true;
}

void ObjectStore::loadAll(Database * database) {
   // Assume we failed until we succeed!  (This saves us having to remember to set the error state in every error
   // branch.  Instead, we just have to set the all OK state at the end of this function.)
   this->pimpl->m_state = ObjectStore::State::ErrorInitialising;

   if (database) {
      this->pimpl->database = database;
   } else {
      this->pimpl->database = &Database::instance();
   }

   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   //
   // .:TBD:. In theory we don't need a transaction if we're _only_ reading data...
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database,
                               connection,
                               QString("Load All %1").arg(*this->pimpl->primaryTable.tableName)};

   //
   // Using QSqlTableModel would save us having to write a SELECT statement, however it is a bit hard to use it to
   // reliably get the number of rows in a table.  Eg, QSqlTableModel::rowCount() is not implemented for all databases,
   // and there is no documented way to detect the index supplied to QSqlTableModel::record(int row) is valid.  (In
   // testing with SQLite, the returned QSqlRecord object for an index one beyond the end of he table still gave a
   // false return to QSqlRecord::isEmpty() but then returned invalid record values.)
   //
   // So, instead, we create the appropriate SELECT query from scratch.  We specify the column names rather than just
   // do SELECT * because it's small extra effort and will give us an early error if an invalid column is specified.
   //
   QString queryString{"SELECT "};
   QTextStream queryStringAsStream{&queryString};
   this->pimpl->appendColumNames(queryStringAsStream, true, false);
   queryStringAsStream << "\n FROM " << this->pimpl->primaryTable.tableName << ";";
   BtSqlQuery sqlQuery{connection};
   sqlQuery.prepare(queryString);
   if (!sqlQuery.exec()) {
      qCritical() <<
         Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
      return;
   }

   qDebug() <<
      Q_FUNC_INFO << "Reading main table rows from" << this->pimpl->primaryTable.tableName <<
      "database table using query " << queryString;

   while (sqlQuery.next()) {
      //
      // We want to pull all the fields for the current row from the database and use them to construct a new
      // object.
      //
      // Two approaches suggest themselves:
      //
      //    (i)  Create a blank object and, using Qt Properties, fill in each field using the QObject setProperty()
      //         call (as we currently do when reading in an XML file).
      //    (ii) Read all the fields for this row from the database and then use them as parameters to call a
      //         suitable constructor to get a new object.
      //
      // The problem with approach (i) is that lots of the setters called via setProperty have side-effects
      // including emitting signals and trying to update the database.  We can sort of get away with ignoring this
      // while reading an XML file, but we risk going round in circles (including being deadlocked) if we let such
      // things happen while we're still reading everything out of the DB at start-up.  A solution would be to have
      // an "initialising" flag on the object that turns off setter side-effects.  This is a small change but one
      // that needs to be made in a lot of places, including almost every setter function.
      //
      // The problem with approach (ii) is that we don't want a constructor that takes a long list of parameters as
      // it's too easy to get bugs where a call is made with the parameters in the wrong order.  We can't easily use
      // Boost Parameter to solve this because it would be hard to have parameter names as pure data (one of the
      // advantages of the Qt Property system), plus it would apparently make compile times very long.  So we would
      // have to roll our own way of passing, say, a QHash (of propertyName -> QVariant) to a constructor.  This is
      // a chunkier change but only needs to be made in a small number of places (new constructors).
      //
      // Although (i) has the further advantage of not requiring a constructor update when a new property is added
      // to a class, it feels a bit wrong to construct an object in "invalid" state and then set a "now valid" flag
      // later after calling lots of setters.  In particular, it is hard (without adding lots of complexity) for the
      // object class to enforce mandatory construction parameters with this approach.
      //
      // Method (ii) is therefore our preferred approach.  We use NamedParameterBundle, which is a simple extension of
      // QHash.
      //
      NamedParameterBundle namedParameterBundle;
      int primaryKey = -1;

      //
      // Populate all the fields
      // By convention, the primary key should be listed as the first field
      //
      // NB: For now we're assuming that the primary key is always an integer, but it would not be enormous work to
      //     allow a wider range of types.
      //
      bool readPrimaryKey = false;
      for (auto const & fieldDefn : this->pimpl->primaryTable.tableFields) {
         QVariant fieldValue = sqlQuery.value(*fieldDefn.columnName);
         //qDebug() <<
         //   Q_FUNC_INFO << "Reading col" << fieldDefn.columnName << "(=" << fieldValue << ") into property" <<
         //   fieldDefn.propertyName;
         if (!fieldValue.isValid()) {
            qCritical() <<
               Q_FUNC_INFO << "Error reading column " << fieldDefn.columnName << " (" << fieldValue.toString() <<
               ") from database table " << this->pimpl->primaryTable.tableName << ". SQL error message: " <<
               sqlQuery.lastError().text();
            break;
         }

         // Fix-up the QVariant if needed, including converting enum string representation to int
         this->pimpl->wrapAndUnmapAsNeeded(this->pimpl->primaryTable, fieldDefn, fieldValue);

         // It's a coding error if we got the same parameter twice
         Q_ASSERT(!namedParameterBundle.contains(fieldDefn.propertyName));

         namedParameterBundle.insert(fieldDefn.propertyName, fieldValue);

         // We assert that the insert always works!
         Q_ASSERT(namedParameterBundle.contains(fieldDefn.propertyName));

         if (!readPrimaryKey) {
            readPrimaryKey = true;
            primaryKey = fieldValue.toInt();
         }
      }

      // Get a new object...
      auto object = this->createNewObject(namedParameterBundle);

      // ...and store it
      // It's a coding error if we have two objects with the same primary key
      Q_ASSERT(!this->pimpl->allObjects.contains(primaryKey));
      this->pimpl->allObjects.insert(primaryKey, object);
      // Normally leave this debug output commented, as it generates a lot of logging at start-up, but can be useful to
      // enable for debugging.
//      qDebug() <<
//         Q_FUNC_INFO << "Cached" << object->metaObject()->className() << "#" << primaryKey << "in" <<
//         this->metaObject()->className();
   }

   qDebug() <<
      Q_FUNC_INFO << "Read" << this->pimpl->allObjects.size() << "entries from primary table" <<
      this->pimpl->primaryTable.tableName;

   //
   // Now we load the data from the junction tables.  This, pretty much by definition, isn't needed for the object's
   // constructor, so we're OK to pull it out separately.  Otherwise we'd have to do a LEFT JOIN for each junction
   // table in the query above.  Since we're caching everything in memory, and we're not overly worried about
   // optimising every single SQL query (because the amount of data in the DB is not enormous), we prefer the
   // simplicity of separate queries.
   //
   for (auto const & junctionTable : this->pimpl->junctionTables) {
      qDebug() <<
         Q_FUNC_INFO << "Reading junction table " << junctionTable.tableName << " into " <<
         GetJunctionTableDefinitionPropertyName(junctionTable);

      //
      // Order first by the object we're adding the other IDs to, then order either by the other IDs or by another
      // column if one is specified.
      //
      queryString = "SELECT ";
      queryStringAsStream <<
         GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) << ", " <<
         GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable) <<
         " FROM " << junctionTable.tableName <<
         " ORDER BY " << GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) << ", ";
      if (!GetJunctionTableDefinitionOrderByColumn(junctionTable).isNull()) {
         queryStringAsStream << GetJunctionTableDefinitionOrderByColumn(junctionTable);
      } else {
         queryStringAsStream << GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable);
      }
      queryStringAsStream << ";";

///      sqlQuery = BtSqlQuery{connection};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return;
      }

      qDebug() << Q_FUNC_INFO << "Reading junction table rows from database query " << queryString;

      //
      // The simplest way to process the data is first to build the ID-to-ordered-list-of-IDs map in memory, then loop
      // through this to pass the data to the relevant objects.
      //
      int previousPrimaryKey = -1;
      QMap< int, QVector<int> > thisToOtherKeys;
      while (sqlQuery.next()) {
         int thisPrimaryKey = sqlQuery.value(*GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable)).toInt();
         int otherPrimaryKey = sqlQuery.value(*GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable)).toInt();
         // Usually keep the next line commented out otherwise it generates a lot of lines in the logs
//         qDebug() << Q_FUNC_INFO << "Interim store of" << thisPrimaryKey << "<->" << otherPrimaryKey;

         if (thisPrimaryKey != previousPrimaryKey) {
            thisToOtherKeys.insert(thisPrimaryKey, QVector<int>{});
            previousPrimaryKey = thisPrimaryKey;
         }
         Q_ASSERT(thisToOtherKeys.contains(thisPrimaryKey));
         thisToOtherKeys[thisPrimaryKey].append(otherPrimaryKey);
      }

      for (auto currentMapping = thisToOtherKeys.cbegin();
           currentMapping != thisToOtherKeys.cend();
           ++currentMapping) {
         //
         // It's probably a coding error somewhere if there's an associative entry for an object that doesn't exist,
         // but we can recover by ignoring the associative entry
         //
         if (!this->contains(currentMapping.key())) {
            qCritical() <<
               Q_FUNC_INFO << "Ignoring record in table " << junctionTable.tableName <<
               " for non-existent object with primary key " << currentMapping.key();
            continue;
         }

         auto currentObject = this->getById(currentMapping.key());

         // We assert that we could not have created a mapping without at least one entry
         Q_ASSERT(currentMapping.value().size() > 0);

         //
         // Normally we'd pass a list of all the "other" keys for each "this" object, but if we've been told to assume
         // there is at most one "other" per "this", then we'll pass just the first one we get back for each "this".
         //
         bool success = false;
         if (junctionTable.assumedNumEntries == ObjectStore::MAX_ONE_ENTRY) {
            qDebug() <<
               Q_FUNC_INFO << currentObject->metaObject()->className() << " #" << currentMapping.key() << ", " <<
               GetJunctionTableDefinitionPropertyName(junctionTable) << "=" << currentMapping.value().first();
            success = currentObject->setProperty(*GetJunctionTableDefinitionPropertyName(junctionTable),
                                                 currentMapping.value().first());
         } else {
            //
            // The setProperty function always takes a QVariant, so we need to create one from the QList<QVariant> we
            // have.  However, we need to be careful here.  There are several ways to get the call to setProperty wrong
            // at runtime, which gives you a "false" return code but no diagnostics or log of why the call failed.
            //
            // In particular, we can't just shove a QList<QVariant> (ie otherKeys) inside a QVariant, because passing
            // this to setProperty() (or equivalent calls via the metaObject) will cause Qt to attempt (and fail) to
            // access a setter that takes QList<QVariant>.  We need a QVector<int> (ie what the setter expects) wrapped
            // in a QVariant.
            //
            // To add to the challenge, despite QVariant having a huge number of constructors, none of them will accept
            // QVector<int>, so, instead, you have to use the static function QVariant::fromValue to create a QVariant
            // wrapper around QVector<int>.
            //
            QVariant wrappedConvertedOtherKeys = QVariant::fromValue(currentMapping.value());
            qDebug() <<
               Q_FUNC_INFO << currentObject->metaObject()->className() << " #" << currentMapping.key() << ", " <<
               GetJunctionTableDefinitionPropertyName(junctionTable) << "=" << currentMapping.value() << "(" <<
               wrappedConvertedOtherKeys << ")";
            success = currentObject->setProperty(*GetJunctionTableDefinitionPropertyName(junctionTable),
                                                 wrappedConvertedOtherKeys);
         }
         if (!success) {
            // This is a coding error - eg the property doesn't have a WRITE member function or it doesn't take the
            // type of argument we supplied inside a QVariant.
            qCritical() <<
               Q_FUNC_INFO << "Unable to set property" << GetJunctionTableDefinitionPropertyName(junctionTable) <<
               "on" << currentObject->metaObject()->className();
            Q_ASSERT(false); // Stop here on a debug build
            return;          // Continue but abort the transaction on a non-debug build
         }

         // This is useful for debugging but I usually leave it commented out as it generates a lot of logging at
         // start-up
//         qDebug() <<
//            Q_FUNC_INFO << "Set" <<
//            (junctionTable.assumedNumEntries == ObjectStore::MAX_ONE_ENTRY ? 1 : otherKeys.size()) <<
//            GetJunctionTableDefinitionPropertyName(junctionTable).c_str() << "property for" <<
//            currentObject->metaObject()->className() << "#" << currentKey;

      }
   }

   dbTransaction.commit();

   qInfo() << Q_FUNC_INFO << "Read" << this->size() << "objects from DB table" << this->pimpl->primaryTable.tableName;

   // If we made it this far, everything must have loaded in OK (otherwise we'd have bailed out above).
   this->pimpl->m_state = ObjectStore::State::InitialisedOk;

   return;
}

size_t ObjectStore::size() const {
   return this->pimpl->allObjects.size();
}

bool ObjectStore::contains(int id) const {
   return this->pimpl->allObjects.contains(id);
}

std::shared_ptr<QObject> ObjectStore::getById(int id) const {
   // Callers should always check that the object they are requesting exists.  However, if a caller does request
   // something invalid, then we at least want to log that for debugging.
   if (!this->pimpl->allObjects.contains(id)) {
      qCritical() <<
         Q_FUNC_INFO << "Unable to find cached object with ID" << id << "(which should be stored in DB table" <<
         this->pimpl->primaryTable.tableName << ")";
   }
   return this->pimpl->allObjects.value(id);
}

QList<std::shared_ptr<QObject> > ObjectStore::getByIds(QVector<int> const & listOfIds) const {
   QList<std::shared_ptr<QObject> > listToReturn;
   for (auto id : listOfIds) {
      if (this->pimpl->allObjects.contains(id)) {
         listToReturn.append(this->pimpl->allObjects.value(id));
      } else {
         qWarning() <<
            Q_FUNC_INFO << "Unable to find object with ID" << id << "(DB table" <<
            this->pimpl->primaryTable.tableName << ")";
      }
   }
   return listToReturn;
}

int ObjectStore::insert(std::shared_ptr<QObject> object) {
   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database,
                               connection,
                               QString("Insert %1").arg(*this->pimpl->primaryTable.tableName)};

   int primaryKey = this->pimpl->insertObjectInDb(connection, *object, false);

   //
   // Add the object to our list of all objects of this type (asserting that it should be impossible for an object with
   // this ID to already exist in that list).
   //
   Q_ASSERT(!this->pimpl->allObjects.contains(primaryKey));
   this->pimpl->allObjects.insert(primaryKey, object);

   // Everything succeeded if we got this far so we can wrap up the transaction
   dbTransaction.commit();

   //
   // Now we tell the object what its primary key is.  Note that we must do this _after_ the database transaction is
   // finished as there are some circumstances where this call will trigger the start of another database transaction.
   //
   BtStringConst const & primaryKeyProperty = this->pimpl->getPrimaryKeyProperty();
   bool setPrimaryKeyOk = object->setProperty(*primaryKeyProperty, primaryKey);
   if (!setPrimaryKeyOk) {
      // This is a coding error - eg the property doesn't have a WRITE member function or it doesn't take the type of
      // argument we supplied inside a QVariant.
      qCritical() <<
         Q_FUNC_INFO << "Unable to set property" << primaryKeyProperty << "on" << object->metaObject()->className();
      Q_ASSERT(false);
   }

   //
   // Tell any bits of the UI that need to know that there's a new object
   //
   emit this->signalObjectInserted(primaryKey);
   return primaryKey;
}

void ObjectStore::update(std::shared_ptr<QObject> object) {
   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database,
                               connection,
                               QString("Update %1").arg(*this->pimpl->primaryTable.tableName)};

   //
   // Construct the SQL, which will be of the form
   //
   //    UPDATE tablename
   //    SET firstColumn = :firstColumn, secondColumn = :secondColumn, ...
   //    WHERE primaryKeyColumn = :primaryKeyColumn;
   //
   // .:TBD:. A small optimisation might be to construct this just once rather than every time this function is called
   //
   QString queryString{"UPDATE "};
   QTextStream queryStringAsStream{&queryString};
   queryStringAsStream << this->pimpl->primaryTable.tableName << " SET ";

   QString  const primaryKeyColumn {*this->pimpl->getPrimaryKeyColumn()};
   QVariant const primaryKey       {this->pimpl->getPrimaryKey(*object)};

   bool skippedPrimaryKey = false;
   bool firstFieldOutput = false;
   for (auto const & fieldDefn: this->pimpl->primaryTable.tableFields) {
      if (!skippedPrimaryKey) {
         skippedPrimaryKey = true;
      } else {
         if (!firstFieldOutput) {
            firstFieldOutput = true;
         } else {
            queryStringAsStream << ", ";
         }
         queryStringAsStream << " " << fieldDefn.columnName << " = :" << fieldDefn.columnName;
      }
   }

   queryStringAsStream << " WHERE " << primaryKeyColumn << " = :" << primaryKeyColumn << ";";

   //
   // Bind the values.  Note that, because we're using bind names, it doesn't matter that the order in which we do the
   // binds is different than the order in which the fields appear in the query.
   //
   BtSqlQuery sqlQuery{connection};
   sqlQuery.prepare(queryString);
   for (auto const & fieldDefn: this->pimpl->primaryTable.tableFields) {
      QVariant bindValue{object->property(*fieldDefn.propertyName)};

      // Fix-up the QVariant if needed, including converting enums to strings
      this->pimpl->unwrapAndMapAsNeeded(this->pimpl->primaryTable, fieldDefn, bindValue);

      sqlQuery.bindValue(QString{":"} + *fieldDefn.columnName, bindValue);
   }

   //
   // Run the query
   //
   if (!sqlQuery.exec()) {
      qCritical() <<
         Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
      return;
   }

   //
   // Now update data in the junction tables
   //
   for (auto const & junctionTable : this->pimpl->junctionTables) {
      qDebug() <<
         Q_FUNC_INFO << "Updating property " << GetJunctionTableDefinitionPropertyName(junctionTable) <<
         " in junction table " << junctionTable.tableName;

      //
      // The simplest thing to do with each junction table is to blat any rows relating to the current object and then
      // write out data based on the current property values.  This may often mean we're deleting rows and rewriting
      // them but, for the small quantity of data we're talking about, it doesn't seem worth the complexity of
      // optimising (eg read what's in the DB, compare with what's in the object property, work out what deletes,
      // inserts and updates are needed to sync them, etc.
      //
      if (!deleteFromJunctionTableDefinition(junctionTable, primaryKey, connection)) {
         return;
      }
      if (!insertIntoJunctionTableDefinition(junctionTable, *object, primaryKey, connection)) {
         return;
      }
   }

   dbTransaction.commit();
   return;
}

void ObjectStore::update(QObject & object) {
   // It's a coding error to call this function for something that's not already stored in the DB
   int const primaryKey = this->pimpl->getPrimaryKey(object).toInt();
   Q_ASSERT(primaryKey > 0);

   // Since the object is already stored, we want a copy of the shared_ptr that we already have for it
   auto sharedPointer = this->getById(primaryKey);
   this->update(sharedPointer);
   return;
}

std::shared_ptr<QObject> ObjectStore::insertOrUpdate(std::shared_ptr<QObject> object) {
   QVariant const primaryKey = this->pimpl->getPrimaryKey(*object);
   if (primaryKey.toInt() > 0) {
      this->update(object);
   } else {
      this->insert(object);
   }
   return object;
}

int ObjectStore::insertOrUpdate(QObject & object) {
   QVariant const primaryKey = this->pimpl->getPrimaryKey(object);
   if (primaryKey.toInt() > 0) {
      // If the object is already stored, then we want a copy of the shared_ptr that we already have for it
      auto sharedPointer = this->getById(primaryKey.toInt());
      this->update(sharedPointer);
   } else {
      // If the object is NOT already stored, then we are assuming (because calling this member function rather than
      // the one with the shared_ptr parameter) that no-one has yet made a shared_ptr for it and we are safe to do so.
      std::shared_ptr<QObject> sharedPointer{&object};
      this->insert(sharedPointer);
   }
   return this->pimpl->getPrimaryKey(object).toInt();
}

void ObjectStore::updateProperty(QObject const & object, BtStringConst const & propertyName) {
   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{
      *this->pimpl->database,
      connection,
      QString("Update property %1 on %2").arg(*propertyName).arg(*this->pimpl->primaryTable.tableName)
   };

   if (!this->pimpl->updatePropertyInDb(connection, object, propertyName)) {
      // Something went wrong.  Bailing out here will abort the transaction and avoid sending the signal.
      return;
   }

   // Everything went fine so we can commit the transaction
   dbTransaction.commit();

   // Tell any bits of the UI that need to know that the property was updated
   emit this->signalPropertyChanged(this->pimpl->getPrimaryKey(object).toInt(), propertyName);

   return;
}

std::shared_ptr<QObject> ObjectStore::defaultSoftDelete(int id) {
   //
   // We assume on soft-delete that there is nothing to do on related objects - eg if a Mash is soft deleted (ie marked
   // deleted but remains in the DB) then there isn't actually anything we need to do with its MashSteps.
   //
   qDebug() << Q_FUNC_INFO << "Soft delete" << this->pimpl->m_className << "#" << id;
   auto object = this->pimpl->allObjects.value(id);
   if (this->pimpl->allObjects.contains(id)) {
      this->pimpl->allObjects.remove(id);

      // Tell any bits of the UI that need to know that an object was deleted
      emit this->signalObjectDeleted(id, object);
   }

   return object;
}

std::shared_ptr<QObject> ObjectStore::defaultHardDelete(int id) {
   //
   // We assume on hard-delete that the subclass ObjectStore (specifically ObjectStoreTyped) will override this member
   // function to interact with the object to delete any "owned" objects.  It is better to have the rules for that in
   // the object model than here in the object store as they can be subtle, and it would be cumbersome to model them
   // generically.
   //
   qDebug() << Q_FUNC_INFO << "Hard delete" << this->pimpl->m_className << "#" << id;
   auto object = this->pimpl->allObjects.value(id);
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database,
                               connection,
                               QString("Hard delete %1").arg(*this->pimpl->primaryTable.tableName)};

   // We'll use this in a couple of places below
   QVariant primaryKey{id};

   //
   // We need to delete from the junction tables before we delete from the main table, otherwise we'll get a foreign key
   // constraint violation because entries in the junction tables are referencing the row in the primary table that we
   // want to remove.  (In SQLite we'll just get a generic "FOREIGN KEY constraint failed" error, which is fun to debug
   // because the way SQLite works means it cannot tell you which FK is the problem at the point that it detects and
   // returns the error.)
   //
   // So, first remove data in the junction tables
   //
   for (auto const & junctionTable : this->pimpl->junctionTables) {
      if (!deleteFromJunctionTableDefinition(junctionTable, primaryKey, connection)) {
         // We'll have already logged errors in deleteFromJunctionTableDefinition().  Not much more we can do other than
         // bail here.
         return object;
      }
   }

   //
   // Now the main table row we want to remove is no longer referenced in the junction tables, we can delete it from the
   // primary table.
   //
   // Construct the SQL, which will be of the form
   //
   //    DELETE FROM tablename
   //    WHERE primaryKeyColumn = :primaryKeyColumn;
   //
   // .:TBD:. A small optimisation might be to construct this just once rather than every time this function is called
   //
   QString queryString{"DELETE FROM "};
   QTextStream queryStringAsStream{&queryString};
   queryStringAsStream << this->pimpl->primaryTable.tableName;
   BtStringConst const & primaryKeyColumn = this->pimpl->getPrimaryKeyColumn();
   queryStringAsStream << " WHERE " << primaryKeyColumn << " = :" << primaryKeyColumn << ";";
   qDebug() <<
      Q_FUNC_INFO << "Deleting main table row #" << id << "with database query " << queryString;

   //
   // Bind the value
   //
   BtSqlQuery sqlQuery{connection};
   sqlQuery.prepare(queryString);
   sqlQuery.bindValue(QString{":"} + *primaryKeyColumn, primaryKey);
   qDebug().noquote() << Q_FUNC_INFO << "Bind values:" << BoundValuesToString(sqlQuery);

   //
   // Run the query
   //
   if (!sqlQuery.exec()) {
      qCritical() <<
         Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
      return object;
   }

   dbTransaction.commit();

   //
   // Remove the object from the cache
   //
   this->pimpl->allObjects.remove(id);

   // Tell any bits of the UI that need to know that an object was deleted
   emit this->signalObjectDeleted(id, object);

   return object;
}

std::shared_ptr<QObject> ObjectStore::findFirstMatching(
   std::function<bool(std::shared_ptr<QObject>)> const & matchFunction
) const {
   auto result = std::find_if(this->pimpl->allObjects.cbegin(), this->pimpl->allObjects.cend(), matchFunction);
   if (result == this->pimpl->allObjects.cend()) {
      return nullptr;
   }
   return *result;
}

std::optional< QObject * > ObjectStore::findFirstMatching(std::function<bool(QObject *)> const & matchFunction) const {
   // std::find_if on this->pimpl->allObjects is going to need a lambda that takes shared pointer to QObject
   // We create a wrapper lambda with this profile that just extracts the raw pointer and passes it through to the
   // caller's lambda
   auto wrapperMatchFunction {
      [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(obj.get());}
   };
   auto result = std::find_if(this->pimpl->allObjects.cbegin(), this->pimpl->allObjects.cend(), wrapperMatchFunction);
   if (result == this->pimpl->allObjects.cend()) {
      return std::nullopt;
   }
   return result->get();
}

QList<std::shared_ptr<QObject> > ObjectStore::findAllMatching(
   std::function<bool(std::shared_ptr<QObject>)> const & matchFunction
) const {
   // Before Qt 6, it would be more efficient to use QVector than QList.  However, we use QList because (a) lots of the
   // rest of the code expects it and (b) from Qt 6, QList will become the same as QVector (see
   // https://www.qt.io/blog/qlist-changes-in-qt-6)
   QList<std::shared_ptr<QObject> > results;
   std::copy_if(this->pimpl->allObjects.cbegin(),
                this->pimpl->allObjects.cend(),
                std::back_inserter(results), matchFunction);
   return results;
}

QList<QObject *> ObjectStore::findAllMatching(std::function<bool(QObject *)> const & matchFunction) const {
   // Call the shared pointer overload of this function, with a suitable wrapper round the supplied lambda
   QList<std::shared_ptr<QObject> > results = this->findAllMatching(
      [matchFunction](std::shared_ptr<QObject> obj) {return matchFunction(obj.get());}
   );

   // Now convert the list of shared pointers to a list of raw pointers
   QList<QObject *> convertedResults;
   convertedResults.reserve(results.size());
   std::transform(results.cbegin(),
                  results.cend(),
                  std::back_inserter(convertedResults),
                  [](auto & sharedPointer) { return sharedPointer.get(); });
   return convertedResults;
}

QVector<int> ObjectStore::idsOfAllMatching(
   std::function<bool(QObject const *)> const & matchFunction
) const {
   qDebug() << Q_FUNC_INFO << this->pimpl->m_className;
   // It would be nice to use C++20 ranges here, but I couldn't find a way to use them with QHash in such a way that the
   // keys of the hash would be accessible in the range.  So, for now, we do it the old way.
   QVector<int> results;
   for (auto hashEntry = this->pimpl->allObjects.cbegin(); hashEntry != this->pimpl->allObjects.cend(); ++hashEntry) {
      if (matchFunction(hashEntry.value().get())) {
         results.append(hashEntry.key());
      }
   }
   return results;
}

QList<std::shared_ptr<QObject> > ObjectStore::getAll() const {
   // QHash already knows how to return a QList of its values
   return this->pimpl->allObjects.values();
}

QList<QObject *> ObjectStore::getAllRaw() const {
   QList<QObject *> listToReturn;
   listToReturn.reserve(this->pimpl->allObjects.size());
   std::transform(this->pimpl->allObjects.cbegin(),
                  this->pimpl->allObjects.cend(),
                  std::back_inserter(listToReturn),
                  [](auto & sharedPointer) { return sharedPointer.get(); });
   return listToReturn;
}

bool ObjectStore::writeAllToNewDb(Database & databaseNew, QSqlDatabase & connectionNew) const {
   //
   // This is primarily used when someone is migrating data from, say, SQLite to PostgreSQL.
   //
   // We've got all the data cached in memory, so we just need to write it to the new database ... with a couple of
   // twists.  The assumption here is that we're already inside a transaction and that foreign key constraints are
   // turned off.  So we just need to tell our insert member function to write to a different DB than normal and not to
   // try to do anything with transactions ... AND we want to keep all the existing primary key values the same, rather
   // than let the DB generate new ones when we do the inserts, so the third parameter to this->pimpl->insertObjectInDb
   // is true.
   //
   for (auto object : this->pimpl->allObjects) {
      if (this->pimpl->insertObjectInDb(connectionNew, *object, true) <= 0) {
         return false;
      }
   }

   //
   // Some databases (eg PostgreSQL) get confused if you manually insert values into a primary key column that is
   // normally automatically populated.  Database class knows how to put things back in order for such databases.
   //
   // Note that we only need to do this for the primary key on primaryTable.  We make no use of the primary key IDs on
   // junction tables and we always let the DB auto-generate them, even when writing all data to a new DB.
   //
   // We _could_ call databaseNew.updatePrimaryKeySequenceIfNecessary() in this->pimpl->insertObjectInDb() every time we
   // explicitly insert an ID in primaryTable, but it's currently not necessary as this is the only place we ask
   // this->pimpl->insertObjectInDb() to do that.
   //
   databaseNew.updatePrimaryKeySequenceIfNecessary(connectionNew,
                                                   this->pimpl->primaryTable.tableName,
                                                   this->pimpl->getPrimaryKeyColumn());

   return true;
}
