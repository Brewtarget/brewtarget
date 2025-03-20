/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/DatabaseSchemaHelper.h is part of Brewtarget, and is copyright the following authors 2009-2021:
 *   • Jonatan Pålsson <jonatan.p@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef DATABASE_DATABASESCHEMAHELPER_H
#define DATABASE_DATABASESCHEMAHELPER_H
#pragma once

#include <QSqlDatabase>

#include "Database.h"

class QTextStream;

/*!
 * \brief Helper functions to manage Database schema upgrades etc
 */
namespace DatabaseSchemaHelper {

   //! \brief Database version. Increment on any schema change.
   extern int const latestVersion;

   //! \brief Get where we are up to with default content files
   int getDefaultContentVersionFromDb(QSqlDatabase & db);

   //! \brief Set where we are up to with default content files
   bool setDefaultContentVersionFromDb(QSqlDatabase & db, int val);

   /*!
    * \brief Create a blank database whose schema version is \c dbVersion
    */
   bool create(Database & database, QSqlDatabase db);

   /*!
    * \brief Migrate schema from \c oldVersion to \c newVersion
    */
   bool migrate(Database & database, int oldVersion, int newVersion, QSqlDatabase connection);

   //! \brief Current schema version of the given database
   int schemaVersion(QSqlDatabase & db);

   //! \brief does the heavy lifting to copy the contents from one db to the next
   bool copyToNewDatabase(Database & newDatabase, QSqlDatabase & connectionNew);
}

#endif
