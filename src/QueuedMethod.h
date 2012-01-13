/*
* QueuedMethod.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2012.
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

#ifndef _QUEUEDMETHOD_H
#define _QUEUEDMETHOD_H

#include <QString>
#include <QGenericArgument>
#include <QGenericReturnArgument>
#include <QList>
#include <QThread>

/*!
 * \class QueuedMethod
 * \author Philip G. Lee (rocketman768@gmail.com)
 *
 * This class allows you to queue any \em invokable function call that would
 * normally block so that it executes in the background. Have I duplicated the
 * functionality of QtConcurrent::run()?
 */
class QueuedMethod : public QThread
{
   Q_OBJECT
public:
   /*!
    * Note: may add more available arguments in future.
    *
    * \param startImmediately true if you want to immediately execute.
    *        Otherwise, call \b start() manually to begin.
    * \param arg0 is the first argument to the method.
    */
   QueuedMethod(QObject* obj, QString const& methodName,
                //QGenericReturnArgument ret,
                bool startImmediately = true,
                QGenericArgument arg0 = QGenericArgument(0) );
   virtual ~QueuedMethod();

   /*!
    * Chain the method call with \b other. I.e. when \b this finishes,
    * \b other will be started.
    * \returns \b other so you can do a->chainWith(b)->chainWith(c) which
    * executes a, then b, then c.
    */
   QueuedMethod* chainWith( QueuedMethod* other );

   /*!
    * Push a method onto the queue. When \b qm->done() is emitted, \b qm
    * will be destructed and dequeued. Only use this when qm is allocated
    * via the \b new operator. Maybe it's a bad name, but please note that
    * the order of enqueuing is not necessarily the order of execution. For
    * order control, see \b chainWith().
    */
   static void enqueue( QueuedMethod* qm );

protected:
   //! Reimplemented from QThread.
   void run();
   
signals:
   /*!
    * Emitted when the encapsulated function has completed.
    * \param success is return value of QMetaObject::invokeMethod().
    */
   void done(bool success);

public slots:
   
private slots:
   void executeFunction();
   void dequeueMyself();
   void startChained();
   
private:
   QueuedMethod* _chainedMethod;
   QObject* _obj;
   QString _methodName;
   //const char* _retName;
   //void* _retData;
   const char* _arg0Name;
   void* _arg0Data;
   bool success;
   
   static QList<QueuedMethod*> _queue;
   static void dequeue( QueuedMethod* qm );
};


#endif /*_QUEUEDMETHOD_H*/
