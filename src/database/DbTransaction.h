/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/DbTransaction.h is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#ifndef DATABASE_DBTRANSACTION_H
#define DATABASE_DBTRANSACTION_H
#pragma once

#include <QSqlDatabase>

class Database;

/**
 * \brief RAII wrapper for transaction(), commit(), rollback() member functions of QSqlDatabase
 */
class DbTransaction {
public:
   enum SpecialBehaviours {
      NONE = 0,
      DISABLE_FOREIGN_KEYS = 1 // For the duration of this transaction
   };

   /**
    * \brief Constructing a \c DbTransaction will start a DB transaction
    */
   DbTransaction(Database & database,
                 QSqlDatabase & connection,
                 QString const nameForLogging = "???",
                 SpecialBehaviours specialBehaviours = NONE);

   /**
    * \brief When a \c DbTransaction goes out of scope and its destructor is called, the transaction started in the
    *        constructor will be rolled back -- unless the caller already successfully called \c commit()
    */
   ~DbTransaction();

   /**
    * \brief Commits the transaction started in the constructor
    *
    * \returns \c true if the commit succeeded, \c false otherwise
    */
   bool commit();

private:
   Database & database;
   // This is intended to be a short-lived object, so it's OK to store a reference to a QSqlDatabase object
   QSqlDatabase & connection;
   // This is useful for diagnosing problems such as
   // 'Unable to start database transaction: "cannot start a transaction within a transaction Unable to begin transaction"'
   QString const nameForLogging;
   bool committed;
   int specialBehaviours;

   // RAII class shouldn't be getting copied or moved
   DbTransaction(DbTransaction const &) = delete;
   DbTransaction & operator=(DbTransaction const &) = delete;
   DbTransaction(DbTransaction &&) = delete;
   DbTransaction & operator=(DbTransaction &&) = delete;

};


#endif
