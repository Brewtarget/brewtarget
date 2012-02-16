/*
* SetterCommandStack.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "SetterCommandStack.h"
#include "SetterCommand.h"
#include <QThread>
#include <QTimer>

SetterCommandStack::SetterCommandStack(QThread* thread, int interval_ms)
   : QObject(),
     _commandLimit(100),
     _numCommands(0),
     _executionInterval_ms(interval_ms),
     _nextCommand(0),
     _nextCommandTmp(0)
{
   // NOTE: Is moveToThread() correct? I want many threads to call
   // push() without executeCommand() blocking it.
   moveToThread(thread);
   
   connect( &_timer, SIGNAL(timeout()), this, SLOT(executeNext()) );
   _timer.setSingleShot(true);
   _timer.setInterval(_executionInterval_ms);
   _timer.start();
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
}

void SetterCommandStack::flush()
{
   bool restart_timer = false;
   // Don't want the normal process to fire in the middle of a forced flush
   if ( _timer.isActive() )
   {
      _timer.stop();
      restart_timer = true;
   }
   executeNext();

   // Restart the timer.
   if ( restart_timer )
      _timer.start();
}

void SetterCommandStack::executeNext()
{
   // Check to make sure there is actually something to run.
   if( _nextCommand )
   {
      // First, exchange pointers.
      _commandPtrSwitch.lock();
      SetterCommand* tmp = _nextCommand;
      _nextCommand = _nextCommandTmp;
      _nextCommandTmp = tmp;
      _commandPtrSwitch.unlock();
   
      // Now, people can keep calling push() without blocking as we execute.
   
      // Push _nextCommandTmp onto stack.
      _commands.append( _nextCommandTmp );
      _numCommands++;
      if( _numCommands > _commandLimit )
      {
         tmp = _commands.takeFirst();
         //tmp->deleteLater();
         delete tmp;
         _numCommands--;
      }
   
      // Now, execute _nextCommandTmp.
      _nextCommandTmp->redo();
      _nextCommandTmp = 0;
   }
   
   // Reset the timer.
   _timer.start();
}

