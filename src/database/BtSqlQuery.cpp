/*
 * database/BtSqlQuery.cpp is part of Brewtarget, and is copyright the following
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
#include "database/BtSqlQuery.h"

#include <stdexcept>

#include <QDebug>
#include <QSqlError>

bool BtSqlQuery::prepare(const QString & query) {
   //
   // We don't want to call QSqlQuery::prepare() because if there are no bind values and the DB is PostgreSQL then we'll
   // get an error.
   //
   this->bt_query = query;
   this->bt_boundValues = false;

   // Since we didn't actually call QSqlQuery::prepare() (yet), there's no possibility of an error to return
   return true;
}

void BtSqlQuery::reallyPrepare() {
   // Once the caller is trying to bind values, we can assume this really is a prepared statement.  So, if we didn't
   // already, call QSqlQuery::prepare()
   if (!this->bt_boundValues) {
      this->bt_boundValues = true;
      if (!this->QSqlQuery::prepare(this->bt_query)) {
         qCritical() << Q_FUNC_INFO << "Call to QSqlQuery::prepare() failed: " << this->lastError().text();
         throw std::runtime_error(this->lastError().text().toStdString());
      }
   }
   return;
}

void BtSqlQuery::addBindValue(const QVariant &val, QSql::ParamType paramType) {
   this->reallyPrepare();
   this->QSqlQuery::addBindValue(val, paramType);
   return;
}

void BtSqlQuery::bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType) {
   this->reallyPrepare();
   this->QSqlQuery::bindValue(placeholder, val, paramType);
   return;
}

void BtSqlQuery::bindValue(int pos, const QVariant &val, QSql::ParamType paramType) {
   this->reallyPrepare();
   this->QSqlQuery::bindValue(pos, val, paramType);
   return;
}

/**
   * \brief As \c QSqlQuery::exec() except that if no values were bound to the query, we pass the SQL from \c prepare()
   *        as a parameter
   */
bool BtSqlQuery::exec() {
   bool result;
   if (this->bt_boundValues) {
      result = this->QSqlQuery::exec();
   } else {
      // If we never bound any values then the SQL was not a prepared statement and this is the point we can safely
      // pass it to QSqlQuery for execution
      result = this->QSqlQuery::exec(this->bt_query);
   }

   // If someone wants to reuse the object, eg to insert multiple rows with the same query, it's already in the correct
   // state (whether or not there were bound variables, so we're done here.

   return result;
}
