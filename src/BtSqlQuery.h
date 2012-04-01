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