/*
 * DatabaseSchemaHelper.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip G. Lee <rocketman768@gmail.com>
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

#ifndef _DATABASESCHEMAHELPER_H
#define _DATABASESCHEMAHELPER_H

#include "brewtarget.h"
#include <QString>
#include <QSqlDatabase>

#include "DatabaseSchema.h"


/*!
 * \brief Helper to Database that manages schema stuff
 * \author Philip G. Lee
 *
 * This helper has static methods available only to Database that help it
 * manage the schema.
 */
class DatabaseSchemaHelper
{
   friend class BeerXML;
   friend class Database;

public:

   // No public methods. Database is the only class able to access
   // DatabaseSchemaHelper methods.

private:

   //! \brief Database version. Increment on any schema change.
   static const int dbVersion;

   // Commands and keywords
   static QString CREATETABLE;
   static QString ALTERTABLE;
   static QString DROPTABLE;
   static QString ADDCOLUMN;
   static QString DROPCOLUMN;
   static QString UPDATE;
   static QString SET;
   static QString INSERTINTO;
   static QString DEFAULT;
   static QString SELECT;
   static QString SEP;
   static QString UNIQUE;
   static QString COMMA;
   static QString OPENPAREN;
   static QString CLOSEPAREN;
   static QString END;

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   static bool upgrade;
   /*!
    * \brief Create a blank database whose schema version is \c dbVersion
    */
   static bool create(QSqlDatabase db, DatabaseSchema* defn, Brewtarget::DBTypes dbType = Brewtarget::NODB);

   /*!
    * \brief Migrate from version \c oldVersion to \c oldVersion+1
    */
   static bool migrateNext(int oldVersion, QSqlDatabase db = QSqlDatabase());

   /*!
    * \brief Migrate schema from \c oldVersion to \c newVersion
    */
   static bool migrate(int oldVersion, int newVersion, QSqlDatabase db = QSqlDatabase());

   //! \brief Current schema version of the given database
   static int currentVersion(QSqlDatabase db = QSqlDatabase());

   static bool drop_columns(QSqlQuery q, TableSchema* tbl);
   static bool drop_columns(QSqlQuery q, TableSchema* tbl, QStringList colNames);

   static bool migrate_to_202(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_210(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_4(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_5(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_6(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_7(QSqlQuery q, DatabaseSchema *defn);
   static bool migration_aide_8(QSqlQuery q, DatabaseSchema* defn, Brewtarget::DBTable table );
   static bool migrate_to_8(QSqlQuery q, DatabaseSchema *defn);
   static bool migrate_to_9(QSqlQuery q, DatabaseSchema *defn);
};

#endif
