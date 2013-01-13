/*
* SetterCommandStack.cpp is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2013.
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

#include "SetterCommandStack.h"
#include "SetterCommand.h"
#include <QThread>
#include <QTimer>
#include <QDebug>

SetterCommandStack::SetterCommandStack(QThread* thread, int interval_ms)
   : QObject(),
     _commandLimit(100),
     _numCommands(0),
     _executionInterval_ms(interval_ms),
     _nextCommand(0),
     _nextCommandTmp(0),
     _timer(0)
{
   _timer = new QTimer(this);
   connect( _timer, SIGNAL(timeout()), this, SLOT(executeNext()) );
   _timer->setSingleShot(true);
   _timer->setInterval(_executionInterval_ms);
   _timer->start();
 
   // Save this til the end, because we need to wait until _timer is added
   // as a child. Otherwise, this object will be in a new thread, but the
   // timer will be attached in the calling thread, which is apparently an
   // issue.
   moveToThread(thread);
}

SetterCommandStack::~SetterCommandStack()
{
   // Keep other people from going through push(). We may miss a few if someone
   // is still in executeNext(), adding things to _commands.
   _commandPtrSwitch.lock();

   qDeleteAll(_commands);
   _commands.clear();
   delete _nextCommand;
   _nextCommand = 0;
   
   _commandPtrSwitch.unlock();
}

void SetterCommandStack::push( SetterCommand* command )
{
   // Yes, I know I said non-blocking, but since pointer swapping takes
   // almost no time, this lock should not introduce much locking time.
   _commandPtrSwitch.lock();
   
   if( !_nextCommand )
      _nextCommand = command;
   else
   {
      _nextCommand->mergeWith(command);
      //command->deleteLater();
      delete command;
   }
   
   _commandPtrSwitch.unlock();
   
   // If the timer is out, start it. We will aggregate commands together
   // until it times out.
   if( !_timer->isActive() )
      _timer->start();
}

void SetterCommandStack::flush()
{
   bool restart_timer = false;
   // Don't want the normal process to fire in the middle of a forced flush
   if ( _timer->isActive() )
   {
      _timer->stop();
      restart_timer = true;
   }
   executeNext();

   // Restart the timer.
   if ( restart_timer )
      _timer->start();
}

void SetterCommandStack::executeNext()
{
  // Prevent timers from stepping on each other.
  //_timer->stop();

   // Check to make sure there is actually something to run.
   _commandPtrSwitch.lock();
   if( _nextCommand )
   {
      // First, exchange pointers.
      //SetterCommand* tmp = _nextCommand;
      //_nextCommand = _nextCommandTmp;
      //_nextCommandTmp = tmp;
      
      _nextCommandTmp = _nextCommand;
      _nextCommand = 0;
      // Push _nextCommandTmp onto stack.
      _commands.append( _nextCommandTmp );
      _numCommands++;
      _commandPtrSwitch.unlock();
   
      // Now, people can keep calling push() without blocking as we execute,
      // creating a new _nextCommand while we work with the old one _nextCommandTmp.
      // Since the timer is stopped, we are guaranteed no-one else is in this
      // function.
      if( _numCommands > _commandLimit )
      {
         delete _commands.takeFirst();;
         _numCommands--;
      }
   
      // Now, execute _nextCommandTmp.
      _nextCommandTmp->redo();
      _nextCommandTmp = 0;
   }
   else
      _commandPtrSwitch.unlock();
   
   // Reset the timer.
   //_timer->start();
}
