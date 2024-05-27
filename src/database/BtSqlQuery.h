/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * database/BtSqlQuery.h is part of Brewtarget, and is copyright the following authors 2021:
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
#ifndef DATABASE_BTSQLQUERY_H
#define DATABASE_BTSQLQUERY_H
#pragma once

#include <QString>
#include <QSqlQuery>

/**
 * \class BtSqlQuery is an extension of \c QSqlQuery with more helpful behaviour around prepared statements
 *
 *        It's not clear from the Qt documentation, but it turns out that, if you call QSqlQuery::prepare() on a SQL
 *        statement that doesn't have any placeholders for binding values, then you get a syntax error when you're
 *        using PostgreSQL; you have to call exec() directly on the SQL.
 *
 *        At one level, this is "correct" because a query without bind value placeholders is not a prepared statement
 *        (https://en.wikipedia.org/wiki/Prepared_statement) and so doesn't need preparing.  (Qt is just reporting the
 *        error it gets back from PostgreSQL when it asks it to prepare the statement.)
 *
 *        On the other hand, it's annoying because we have to have tedious special-case handling in code that is not
 *        using local hard-coded SQL.
 *
 *        Since, per discussion in https://bugreports.qt.io/browse/QTBUG-48471, QSqlQuery is unlikely to be fixed or
 *        enhanced to deal with this, we create our own wrapper class that does the right thing.
 *
 *        USAGE:
 *           - Create a BtSqlQuery object
 *           - Call its \c prepare() member function to tell it what SQL you want to execute (regardless of whether it
 *             has bind parameters)
 *           - Call \c bindValue() as necessary for any bind parameters
 *           - Call \c exec() to execute
 *
 *        Note that a syntax error in a prepared statement will not get reported until the first call to \c bindValue()
 *        (and will be reported via logging + run-time exception rather than return value), but otherwise behaviour
 *        should be similar to the way you would want \c QSqlQuery to work.
 */
class BtSqlQuery : public QSqlQuery {
public:
   // Use the same constructors as QSqlQuery
   using QSqlQuery::QSqlQuery;

   /**
    * \brief As \c QSqlQuery::prepare() except we don't actually call QSqlQuery::prepare() unless and until a value is
    *        bound to the query (via \c bindValue)
    */
   bool prepare(const QString & query);

   void addBindValue(const QVariant &val, QSql::ParamType paramType = QSql::In);
   void bindValue(const QString &placeholder, const QVariant &val, QSql::ParamType paramType = QSql::In);
   void bindValue(int pos, const QVariant &val, QSql::ParamType paramType = QSql::In);

   // Allow access to the version of QSqlQuery::exec() that we don't override
   using QSqlQuery::exec;

   /**
    * \brief As \c QSqlQuery::exec() except that if no values were bound to the query, we pass the SQL from \c prepare()
    *        as a parameter
    */
   bool exec();

private:
   // We need to be careful about names to avoid clashes with anything in the base class
   QString bt_query;
   bool bt_boundValues = false;

   void reallyPrepare();


};

#endif
