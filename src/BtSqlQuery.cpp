/*
 * BtSqlQuery.cpp is part of Brewtarget, and is Copyright the following
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

#include "BtSqlQuery.h"

#include <QSqlQuery>
#include <QTimer>
#include <QThread>
#include "database.h"

BtSqlQuery::BtSqlQuery( QString const& query )
   : //QObject( &(Database::instance()) ),
     QObject(),
     _query(0),
     _queryString(query)
{
   _queryMutex.lock(); // finishConstructor() will unlock.
   
   moveToThread( Database::instance()._thread );
   QTimer::singleShot( 0, this, SLOT(finishConstructor()) );
   // Have to spin the event loop to get the previous connection to fire
   // because someone is probably going to construct this object, then
   // immediately call exec(), so we would otherwise never call finishConstructor().
   QCoreApplication::processEvents();
}

BtSqlQuery::~BtSqlQuery()
{
   _queryMutex.lock();
   delete _query;
   _queryMutex.unlock();
}

void BtSqlQuery::bindValue( QString const& placeHolder, QVariant const& val )
{
   // NOTE: is it ok that this happens in the calling thread?
   // If not, cause QTimer::singleShot() to call a private slot
   // to finish the binding.
   
   _queryMutex.lock();
   
   _query->bindValue( placeHolder, val );
   
   _queryMutex.unlock();
}

void BtSqlQuery::finishConstructor()
{
   setParent( &(Database::instance()) );
   _query = new QSqlQuery( Database::sqlDatabase() );
   _query->prepare(_queryString);
   _queryMutex.unlock();
}

void BtSqlQuery::exec()
{
   // This causes doQuery() to be executed not in the calling thread, but our own.
   QTimer::singleShot(0, this, SLOT(doQuery()));
}

void BtSqlQuery::exec( QList<QSqlRecord>& results )
{
   // These two just make sure that finishConstructor() has run through.
   _queryMutex.lock();
   _queryMutex.unlock();
   
   _waitMutex.lock();
   
   // Cause doQuery() to be executed in our own thread.
   QTimer::singleShot(0, this, SLOT(doQuery()));
   // Supposed to block until we are set free by doQuery().
   // I think this doesn't work, because it blocks the whole
   // damn thread, including the event loop that is supposed
   // to send the QTimer signal to doQuery(), so we lock up
   // forever right here.
   _waitCondition.wait(&_waitMutex);
   
   _waitMutex.unlock();
   // Return the results.
   results = _results;
}

void BtSqlQuery::doQuery()
{
   _queryMutex.lock();
   
   _query->exec();
   while( _query->next() )
      _results.append(_query->record());
   
   _queryMutex.unlock();
   
   _waitCondition.wakeAll(); // Wake up someone who is waiting on the results.
   emit resultsReady( _results );
}