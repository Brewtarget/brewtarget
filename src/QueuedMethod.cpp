/*
* QueuedMethod.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2011.
*
* Brewtarget is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Brewtarget is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "QueuedMethod.h"
#include <QTimer>
#include <QMetaObject>

QList<QueuedMethod*> QueuedMethod::_queue;

QueuedMethod::QueuedMethod(
   QObject* obj, QString const& methodName,
   //QGenericReturnArgument ret,
   bool startImmediately,
   QGenericArgument arg0
)
   : QThread(),
     _obj(obj),
     _methodName(methodName),
     //_retName(ret.name()),
     //_retData(ret.data()),
     _arg0Name(arg0.name()),
     _arg0Data(arg0.data())
{
   // Sets the right affinity.
   moveToThread(this);
   
   if( startImmediately )
      start();
}

void QueuedMethod::run()
{
   // Schedule the function execution to start when our event loop gets created.
   QTimer::singleShot( 0, this, SLOT(executeFunction()) );
   
   // Exec blocks until the event loop stops.
   exec();
}

void QueuedMethod::executeFunction()
{
   // Do the function call.
   bool success = QMetaObject::invokeMethod(
                     _obj, 
                     _methodName.toStdString().c_str(),
                     Qt::QueuedConnection,
                     //QGenericReturnArgument(_retName, _retData),
                     QGenericArgument(_arg0Name, _arg0Data)
                  );
   emit done(success);
   dequeue( this );
}

void QueuedMethod::enqueue( QueuedMethod* qm )
{
   if( !_queue.contains(qm) )
      _queue.append(qm);
}

void QueuedMethod::dequeue( QueuedMethod* qm )
{
   if( _queue.contains(qm) )
   {
      _queue.removeAll(qm);
      delete qm;
   }
}