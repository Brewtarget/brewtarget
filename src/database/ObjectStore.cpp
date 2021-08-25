/*
 * database/ObjectStore.cpp is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "database/ObjectStore.h"

#include <cstring>

#include <QDebug>
#include <QHash>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlField>
#include <QSqlRecord>

#include "database/BtSqlQuery.h"
#include "database/Database.h"
#include "database/DbTransaction.h"
#include "model/NamedParameterBundle.h"

// Private implementation details that don't need access to class member variables
namespace {

   /**
    * For a given field type, get the native database typename
    */
   char const * getDatabaseNativeTypeName(Database const & database, ObjectStore::FieldType const fieldType) {
      switch (fieldType) {
         case ObjectStore::FieldType::Bool:   return database.getDbNativeTypeName<bool>();
         case ObjectStore::FieldType::Int:    return database.getDbNativeTypeName<int>();
         case ObjectStore::FieldType::UInt:   return database.getDbNativeTypeName<unsigned int>();
         case ObjectStore::FieldType::Double: return database.getDbNativeTypeName<double>();
         case ObjectStore::FieldType::String: return database.getDbNativeTypeName<QString>();
         case ObjectStore::FieldType::Date:   return database.getDbNativeTypeName<QDate>();
         case ObjectStore::FieldType::Enum:   return database.getDbNativeTypeName<QString>();
         default:
            // It's a coding error if we get here!
            Q_ASSERT(false);
            break;
      }
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
      // .:TBD:. At some future point we might extend our model to allow marking some columns as NOT NULL (eg via some
      //         extra optional flag on ObjectStore::TableField), but it doesn't seem pressing at the moment.
      //
      QString queryString{"CREATE TABLE "};
      QTextStream queryStringAsStream{&queryString};
      queryStringAsStream << tableDefinition.tableName << " (\n";
      bool firstFieldOutput = false;
      for (auto const & fieldDefn: tableDefinition.tableFields) {
         if (fieldDefn.foreignKeyTo != nullptr) {
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
         if (fieldDefn.fieldType == ObjectStore::FieldType::Int && fieldDefn.foreignKeyTo != nullptr) {
            // It's obviously a programming error if the foreignKeyTo table doesn't have any fields.  (We only care
            // here about the first of those fields as, by convention, that's always the primary key on the table.)
            Q_ASSERT(fieldDefn.foreignKeyTo->tableFields.size() > 0);

            QString queryString = QString(
               database.getSqlToAddColumnAsForeignKey()
            ).arg(
               *tableDefinition.tableName
            ).arg(
               *fieldDefn.columnName
            ).arg(
               *fieldDefn.foreignKeyTo->tableName
            ).arg(
               *fieldDefn.foreignKeyTo->tableFields[0].columnName
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
    * Return a string containing all the bound values on a query.   This is quite a useful thing to have logged when
    * you get an error!
    *
    * NB: This can be a long string.  It includes newlines, and is intended to be logged with qDebug().noquote() or
    *     similar.
    */
   QString BoundValuesToString(BtSqlQuery const & sqlQuery) {
      QString result;
      QTextStream resultAsStream{&result};

      QMap<QString, QVariant> boundValueMap = sqlQuery.boundValues();
      for (auto bv = boundValueMap.begin(); bv != boundValueMap.end(); ++bv) {
         resultAsStream << bv.key() << ": " << bv.value().toString() << "\n";
      }

      return result;
   }

   /**
    * Given a (QVariant-wrapped) string value pulled out of the DB for an enum, look up and return its internal
    * numerical enum equivalent
    */
   int stringToEnum(ObjectStore::TableField const & fieldDefn, QVariant const & valueFromDb) {
      // It's a coding error if we called this function for a non-enum field
      Q_ASSERT(fieldDefn.fieldType == ObjectStore::Enum);
      Q_ASSERT(fieldDefn.enumMapping != nullptr);

      if (valueFromDb.isNull()) {
         qCritical() <<
            Q_FUNC_INFO << "Found null value for enum when mapping column " << fieldDefn.columnName <<
            " to property " << fieldDefn.propertyName << " so using 0";
         return 0;
      }

      QString stringValue = valueFromDb.toString();
      auto match = std::find_if(
         fieldDefn.enumMapping->begin(),
         fieldDefn.enumMapping->end(),
         [stringValue](ObjectStore::EnumAndItsDbString const & ii){return stringValue == ii.string;}
      );

      // If we didn't find a match, its either a coding error or someone messed with the DB data
      if (match == fieldDefn.enumMapping->end()) {
         qCritical() <<
            Q_FUNC_INFO << "Could not decode " << stringValue << " to enum when mapping column " <<
            fieldDefn.columnName << " to property " << fieldDefn.propertyName << " so using 0";
         return 0;
      }
      return match->native;
   }

   /**
    * Given a (QVariant-wrapped) int value of a native enum, look up and return the corresponding string we use to
    * store it in the DB
    */
   QString enumToString(ObjectStore::TableField const & fieldDefn, QVariant const & propertyValue) {
      // It's a coding error if we called this function for a non-enum field
      Q_ASSERT(fieldDefn.fieldType == ObjectStore::Enum);
      Q_ASSERT(fieldDefn.enumMapping != nullptr);

      int nativeValue = propertyValue.toInt();
      auto match = std::find_if(
         fieldDefn.enumMapping->begin(),
         fieldDefn.enumMapping->end(),
         [nativeValue](ObjectStore::EnumAndItsDbString const & ii){return nativeValue == ii.native;}
      );

      // It's a coding error if we couldn't find a match
      Q_ASSERT(match != fieldDefn.enumMapping->end());

      return match->string;
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
      if (QVariant::Type::Int != primaryKey.type()) {
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

      //
      // Note that, when we are using bind values, we do NOT want to call the
      // BtSqlQuery::BtSqlQuery(const QString &, QSqlDatabase db) version of the BtSqlQuery constructor because that would
      // result in the supplied query being executed immediately (ie before we've had a chance to bind parameters).
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
         // The propertyValuesWrapper QVariant should hold QVector<int>.  If it doesn't it's a coding error (because we have a
         // property getter that's returning something else).
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
         Q_FUNC_INFO << propertyValues.size() << "value(s) (in" << propertyValuesWrapper.typeName() << ") for property" <<
         GetJunctionTableDefinitionPropertyName(junctionTable) << "of" << object.metaObject()->className() <<
         "#" << primaryKey.toInt();
      for (int curValue : propertyValues) {
         sqlQuery.bindValue(thisPrimaryKeyBindName, primaryKey);
         sqlQuery.bindValue(otherPrimaryKeyBindName, curValue);
         if (!GetJunctionTableDefinitionOrderByColumn(junctionTable).isNull()) {
            sqlQuery.bindValue(orderByBindName, itemNumber);
         }
         qDebug() <<
            Q_FUNC_INFO << itemNumber << ": " <<
            GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable) << " #" << primaryKey.toInt() << " <-> " <<
            GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable) << " #" << curValue;

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

      QString const thisPrimaryKeyBindName = QString{":"} + *GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable);

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

}

// This private implementation class holds all private non-virtual members of ObjectStore
class ObjectStore::impl {
public:

   /**
    * Constructor
    */
   impl(TableDefinition const &           primaryTable,
        JunctionTableDefinitions const & junctionTables) : primaryTable{primaryTable},
                                                           junctionTables{junctionTables},
                                                           allObjects{},
                                                           database{nullptr} {
      return;
   }


   /**
    * Destructor
    */
   ~impl() = default;

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
      // First check whether this is a simple property.  (If not we look for it in the ones we store in junction tables.)
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

         //
         // Bind the values
         //
         BtSqlQuery sqlQuery{connection};
         sqlQuery.prepare(queryString);
         QVariant propertyBindValue{object.property(*propertyName)};
         // Enums need to be converted to strings first
         auto fieldDefn = std::find_if(
            this->primaryTable.tableFields.begin(),
            this->primaryTable.tableFields.end(),
            [propertyName](TableField const & fd){return propertyName == fd.propertyName;}
         );
         // It's a coding error if we're trying to update a property that's not in the field definitions
         Q_ASSERT(fieldDefn != this->primaryTable.tableFields.end());
         if (fieldDefn->fieldType == ObjectStore::Enum) {
            propertyBindValue = QVariant{enumToString(*fieldDefn, propertyBindValue)};
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
         // The property we've been given isn't a simple property, so look for it in the ones we store in junction tables
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
            Q_ASSERT(false);
         }

         //
         // As elsewhere, the simplest way to update a junction table is to blat any rows relating to the current object and then
         // write out data based on the current property values.
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

      //
      // Bind the values
      //
      BtSqlQuery sqlQuery{connection};
      sqlQuery.prepare(queryString);
      for (int ii = (writePrimaryKey ? 0 : 1); ii < this->primaryTable.tableFields.size(); ++ii) {
         auto const & fieldDefn = this->primaryTable.tableFields[ii];

         QVariant bindValue{object.property(*fieldDefn.propertyName)};
         if (fieldDefn.fieldType == ObjectStore::Enum) {
            // Enums need to be converted to strings first
            bindValue = QVariant{enumToString(fieldDefn, bindValue)};
         } else if (fieldDefn.foreignKeyTo && bindValue.toInt() <= 0) {
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

      //
      // Get the ID of the row we just inserted
      //
      // Assert that we are only using database drivers that support returning the last insert ID.  (It is frustratingly
      // hard to find documentation about this, as, eg, https://doc.qt.io/qt-5/sql-driver.html does not explicitly list
      // which supplied drivers support which features.  However, in reality, we know SQLite and PostgreSQL drivers both
      // support this, so it would likely only be a problem if a new type of DB were introduced.)
      //
      // Note too that we have to explicitly put the primary key into an int, because, by default it might come back as
      // long long int rather than int (ie 64-bits rather than 32-bits in the C++ implementations we care about).
      //
      Q_ASSERT(sqlQuery.driver()->hasFeature(QSqlDriver::LastInsertId));
      QVariant rawPrimaryKey = sqlQuery.lastInsertId();
      Q_ASSERT(rawPrimaryKey.canConvert(QMetaType::Int));
      int primaryKeyInDb = rawPrimaryKey.toInt();
      qDebug() <<
         Q_FUNC_INFO << object.metaObject()->className() << "#" << primaryKeyInDb << "inserted in database using" <<
         queryString;

      //
      // Where we're not explicitly writing the primary key it's because the object we are inserting should not already
      // have a valid primary key.
      //
      // Where we are writing the primary key, it shouldn't get changed by the write.
      //
      // .:TBD:. Maybe if we're doing undelete, this is the place to handle that case.
      //
      int currentPrimaryKey = object.property(*this->getPrimaryKeyProperty()).toInt();
      if (writePrimaryKey && primaryKeyInDb != currentPrimaryKey) {
         // Likely a coding error
         qCritical() <<
            Q_FUNC_INFO << "After writing" << object.metaObject()->className() << "#" << currentPrimaryKey <<
            "to database, primary key was changed to" << primaryKeyInDb;
         Q_ASSERT(false); // Stop here on debug build
      } else if (!writePrimaryKey && currentPrimaryKey > 0) {
         // This is almost certainly a coding error
         qCritical() <<
            Q_FUNC_INFO << "Wrote new" << object.metaObject()->className() << " to database (with primary key " <<
            primaryKeyInDb << ") but it already had primary key" << currentPrimaryKey;
         Q_ASSERT(false); // Stop here on debug build
      }

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

   TableDefinition const & primaryTable;
   JunctionTableDefinitions const & junctionTables;
   QHash<int, std::shared_ptr<QObject> > allObjects;
   Database * database;
};


ObjectStore::ObjectStore(TableDefinition const &           primaryTable,
                         JunctionTableDefinitions const & junctionTables) :
   pimpl{ std::make_unique<impl>(primaryTable, junctionTables) } {
   qDebug() << Q_FUNC_INFO << "Construct of object store for primary table" << this->pimpl->primaryTable.tableName;
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
   DbTransaction dbTransaction{*this->pimpl->database, connection};

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

         // Enums need to be converted from their string representation in the DB to a numeric value
         if (fieldDefn.fieldType == ObjectStore::Enum) {
            fieldValue = QVariant(stringToEnum(fieldDefn, fieldValue));
            //qDebug() <<
            //   Q_FUNC_INFO << "Value for property" << fieldDefn.propertyName << "after enum conversion: " <<
            //   fieldValue;
         }

         // It's a coding error if we got the same parameter twice
         Q_ASSERT(!namedParameterBundle.contains(*fieldDefn.propertyName));

         namedParameterBundle.insert(fieldDefn.propertyName, fieldValue);

         // We assert that the insert always works!
         Q_ASSERT(namedParameterBundle.contains(*fieldDefn.propertyName));

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

      sqlQuery = BtSqlQuery{connection};
      sqlQuery.prepare(queryString);
      if (!sqlQuery.exec()) {
         qCritical() <<
            Q_FUNC_INFO << "Error executing database query " << queryString << ": " << sqlQuery.lastError().text();
         return;
      }

      qDebug() << Q_FUNC_INFO << "Reading junction table rows from database query " << queryString;

      //
      // The simplest way to process the data is first to build the raw ID-to-ID map in memory...
      //
      QMultiHash<int, QVariant> thisToOtherKeys;
      while (sqlQuery.next()) {
         thisToOtherKeys.insert(sqlQuery.value(*GetJunctionTableDefinitionThisPrimaryKeyColumn(junctionTable)).toInt(),
                                sqlQuery.value(*GetJunctionTableDefinitionOtherPrimaryKeyColumn(junctionTable)));
      }

      //
      // ...then loop through the map to pass the data to the relevant objects
      //
      for (int const currentKey : thisToOtherKeys.uniqueKeys()) {
         //
         // It's probably a coding error somewhere if there's an associative entry for an object that doesn't exist,
         // but we can recover by ignoring the associative entry
         //
         if (!this->contains(currentKey)) {
            qCritical() <<
               Q_FUNC_INFO << "Ignoring record in table " << junctionTable.tableName <<
               " for non-existent object with primary key " << currentKey;
            continue;
         }

         auto currentObject = this->getById(currentKey);

         //
         // Normally we'd pass a list of all the "other" keys for each "this" object, but if we've been told to assume
         // there is at most one "other" per "this", then we'll pass just the first one we get back for each "this".
         //
         QList<QVariant> otherKeys = thisToOtherKeys.values(currentKey);
         bool success = false;
         if (junctionTable.assumedNumEntries == ObjectStore::MAX_ONE_ENTRY) {
            qDebug() <<
               Q_FUNC_INFO << currentObject->metaObject()->className() << " #" << currentKey << ", " <<
               GetJunctionTableDefinitionPropertyName(junctionTable) << "=" << otherKeys.first();
            success = currentObject->setProperty(*GetJunctionTableDefinitionPropertyName(junctionTable),
                                                 otherKeys.first());
         } else {
            //
            // The setProperty function always takes a QVariant, so we need to create one from the QList<QVariant> we
            // have.  However, we need to be careful here.  There are several ways to get the call to setProperty wrong
            // at runtime, which gives you a "false" return code but no diagnostics or log of why the call failed.
            //
            // In particular, we can't just shove a QList<QVariant> (ie otherKeys) inside a QVariant, because passing
            // this to setProperty() (or equivalent calls via the metaObject) will cause Qt to attempt (and fail) to
            // access a setter that takes QList<QVariant>.  We need to create QVector<int> (ie what the setter expects)
            // and then wrap that in a QVariant.
            //
            // To add to the challenge, despite QVariant having a huge number of constructors, none of them will accept
            // QVector<int>, so, instead, you have to use the static function QVariant::fromValue to create a QVariant
            // wrapper around QVector<int>.
            //
            QVector<int> convertedOtherKeys;
            for (auto ii : otherKeys) {
               convertedOtherKeys.append(ii.toInt());
            }
            QVariant wrappedConvertedOtherKeys = QVariant::fromValue(convertedOtherKeys);
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

         // This is useful for debugging but I usually leave it commented out as it generates a lot of logging at start-up
//         qDebug() <<
//            Q_FUNC_INFO << "Set" <<
//            (junctionTable.assumedNumEntries == ObjectStore::MAX_ONE_ENTRY ? 1 : otherKeys.size()) <<
//            GetJunctionTableDefinitionPropertyName(junctionTable).c_str() << "property for" <<
//            currentObject->metaObject()->className() << "#" << currentKey;

      }
   }

   dbTransaction.commit();
   return;
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
         qWarning() << Q_FUNC_INFO << "Unable to find object with ID " << id;
      }
   }
   return listToReturn;
}


int ObjectStore::insert(std::shared_ptr<QObject> object) {
   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database, connection};

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
   DbTransaction dbTransaction{*this->pimpl->database, connection};

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

      // Enums need to be converted to strings first
      if (fieldDefn.fieldType == ObjectStore::Enum) {
         bindValue = QVariant{enumToString(fieldDefn, bindValue)};
      }

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
   std::shared_ptr<QObject> sharedPointer{&object};
   this->insertOrUpdate(sharedPointer);
   return this->pimpl->getPrimaryKey(object).toInt();
}

void ObjectStore::updateProperty(QObject const & object, BtStringConst const & propertyName) {
   // Start transaction
   // (By the magic of RAII, this will abort if we return from this function without calling dbTransaction.commit()
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database, connection};

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


void ObjectStore::softDelete(int id) {
   //
   // We assume on soft-delete that there is nothing to do on related objects - eg if a Mash is soft deleted (ie marked
   // deleted but remains in the DB) then there isn't actually anything we need to do with its MashSteps.
   //
   qDebug() << Q_FUNC_INFO << "Soft delete item #" << id;
   auto object = this->pimpl->allObjects.value(id);
   if (this->pimpl->allObjects.contains(id)) {
      this->pimpl->allObjects.remove(id);

      // Tell any bits of the UI that need to know that an object was deleted
      emit this->signalObjectDeleted(id, object);
   }

   return;
}

//
void ObjectStore::hardDelete(int id) {
   //
   // We assume on hard-delete that the subclass ObjectStore (specifically ObjectStoreTyped will override this member
   // function to interact with the object to delete any "owned" objects.  It is better to have the rules for that in
   // the object model than here in the object store as they can be subtle, and it would be cumbersome to model them
   // generically.
   //
   qDebug() << Q_FUNC_INFO << "Hard delete item #" << id;
   QSqlDatabase connection = this->pimpl->database->sqlDatabase();
   DbTransaction dbTransaction{*this->pimpl->database, connection};

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
   QVariant primaryKey{id};
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
      return;
   }

   //
   // Now remove data in the junction tables
   //
   for (auto const & junctionTable : this->pimpl->junctionTables) {
      if (!deleteFromJunctionTableDefinition(junctionTable, primaryKey, connection)) {
         return;
      }
   }

   dbTransaction.commit();

   //
   // Remove the object from the cache
   //
   auto object = this->pimpl->allObjects.value(id);
   this->pimpl->allObjects.remove(id);

   // Tell any bits of the UI that need to know that an object was deleted
   emit this->signalObjectDeleted(id, object);

   return;
}


std::optional< std::shared_ptr<QObject> > ObjectStore::findFirstMatching(
   std::function<bool(std::shared_ptr<QObject>)> const & matchFunction
) const {
   auto result = std::find_if(this->pimpl->allObjects.cbegin(), this->pimpl->allObjects.cend(), matchFunction);
   if (result == this->pimpl->allObjects.end()) {
      return std::nullopt;
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
   if (result == this->pimpl->allObjects.end()) {
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
   std::copy_if(this->pimpl->allObjects.cbegin(), this->pimpl->allObjects.cend(), std::back_inserter(results), matchFunction);
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

bool ObjectStore::writeAllToNewDb(QSqlDatabase connectionNew) const {
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

   return true;
}
