/*
 * observable.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include <QVector>
#include "observable.h"

void Observable::setDefaults()
{
   observers = QVector<Observer*>();
   doNotify = true;
}

Observable::Observable()
{
   setDefaults();
}

Observable::Observable(Observer* obs)
{
   setDefaults();
   observers.push_back(obs);
}

void Observable::addObserver(Observer* obs)
{
   observers.push_back(obs);
}

bool Observable::removeObserver(Observer* obs)
{
   QVector<Observer*>::iterator iter;
   
   for( iter = observers.begin(); iter != observers.end(); ++iter )
      if( *iter == obs )
      {
         observers.erase(iter);
         return true;
      }
   
   return false;
}

void Observable::hasChanged(QVariant info)
{
   notifyObservers(info);
}

void Observable::notifyObservers(QVariant info)
{
   unsigned int i, size=observers.size();

   if( ! doNotify )
      return;

   for( i = 0; i < size; ++i )
      observers[i]->notify(this, info);
}

void Observable::forceNotify()
{
   notifyObservers(QVariant());
}

void Observable::disableNotification()
{
   doNotify = false;
}

void Observable::reenableNotification()
{
   doNotify = true;
}
