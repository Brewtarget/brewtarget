/*
 * QueuedMethod.cpp is part of Brewtarget, and is Copyright the following
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

#include "QueuedMethod.h"
#include <QTimer>
#include <QMetaObject>
#include <QDebug>
#include <QThread>

QList< QSharedPointer<QueuedMethod> > QueuedMethod::_queue;

QueuedMethod::QueuedMethod(
   QObject* obj, QString const& methodName,
   //QGenericReturnArgument ret,
   bool startImmediately,
   QGenericArgument arg0
)
   : QThread(),
     _chainedMethod(),
     _obj(obj),
     _methodName(methodName),
     //_retName(ret.name()),
     //_retData(ret.data()),
     _arg0Name(arg0.name()),
     _arg0Data(arg0.data())
{
   if( startImmediately )
      start();
}

QueuedMethod::~QueuedMethod()
{
   qDebug() << "~QueuedMethod()";
   qDebug() << "   thread=" << QThread::currentThread();
}

void QueuedMethod::run()
{
   // This will call executeFunction() once the event loop starts.
   QTimer::singleShot(0, this, SLOT(executeFunction()));
   
   // This call starts the event loop and blocks until it is stopped.
   exec();
}

void QueuedMethod::executeFunction()
{
   // Do the function call.
   success = QMetaObject::invokeMethod(
                _obj, 
                _methodName.toStdString().c_str(),
                Qt::QueuedConnection,
                //QGenericReturnArgument(_retName, _retData),
                QGenericArgument(_arg0Name, _arg0Data)
             );
   //qDebug() << _methodName << ": " << success;
   
   emit done(success);
   
   // If there is no chained method, we are done.
   if( !_chainedMethod )
   {
      quit(); // Should cause event loop to stop, and run() to return.
      dequeueMyself();
   }
}

void QueuedMethod::enqueue( QSharedPointer<QueuedMethod> qm )
{
   if( !_queue.contains(qm) )
      _queue.append(qm);
}

void QueuedMethod::dequeueMyself()
{
   //qDebug() << "Dequeueing: " << this;
   
   // First, find a shared-pointer that has internal pointer equal to 'this'
   QList< QSharedPointer<QueuedMethod> >::iterator i = _queue.begin();
   while( i != _queue.end() && *i != this )
      i++;
   
   // If we found a matching shared pointer, remove all of them from the queue.
   if( *i == this )
      _queue.removeAll(*i);
}

QSharedPointer<QueuedMethod> QueuedMethod::chainWith( QSharedPointer<QueuedMethod> other )
{
   _chainedMethod = other;
   connect( this, &QueuedMethod::done, this, &QueuedMethod::startChained );
   return other;
}

void QueuedMethod::startChained()
{
   //qDebug() << "startChained(): " << this << _chainedMethod;
   if( _chainedMethod )
      _chainedMethod->start();
   
   // Since this is the last thing we should do, safe to exit the thread.
   quit(); // Stops the event loop.
   dequeueMyself();
}
