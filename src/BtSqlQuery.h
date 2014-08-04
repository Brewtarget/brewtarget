/*
 * BtSqlQuery.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#ifndef _BTSQLQUERY_H
#define _BTSQLQUERY_H

#include <QObject>
#include <QMutex>
#include <QWaitCondition>
#include <QList>
#include <QSqlRecord>

class QSqlQuery;

/*!
 * \brief An sql query that happens asynchronously in the Database thread only.
 *
 * Since sql queries to a database connection made in one thread can only be
 * made in the same thread, this class ensures that that happens.
 */
class BtSqlQuery : QObject
{
   Q_OBJECT
   
public:
   //! Prepares the query, but does not begin to execute it.
   BtSqlQuery( QString const& query = QString() );
   virtual ~BtSqlQuery();
   
   //! Execute the query. Returns immediately. Use \em resultsReady() to get results.
   void exec();
   
   //! Executes the query, and blocks until the results come back. Doesn't work.
   void exec( QList<QSqlRecord>& results );
   
   //! Bind a value to the query, as in \em QSqlQuery::bindValue().
   void bindValue( QString const& placeHolder, QVariant const& val );
   
signals:
   //! Emitted when the results of the query are ready.
   void resultsReady( QList<QSqlRecord> results );

private slots:
   void doQuery();
   
   void finishConstructor();
private:
   QSqlQuery* _query;
   QMutex _queryMutex;
   QString _queryString;
   QList<QSqlRecord> _results;
   QMutex _waitMutex;
   QWaitCondition _waitCondition;
};

#endif