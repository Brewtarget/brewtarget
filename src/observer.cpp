/*
 * observer.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QVector>
#include "observable.h"
#include "observable.h"

bool Observer::dirty = false;

Observer::Observer()
{
   observed = 0;
}

Observer::Observer(Observable* obs)
{
   if( obs )
      observed = obs;
   
   observed->addObserver(this);
}

void Observer::setObserved(Observable* obs)
{
   if( observed )
      observed->removeObserver(this);
   
   observed = obs;
   if( observed )
      observed->addObserver(this);
}

bool Observer::isDirty()
{
   return Observer::dirty;
}

void Observer::setDirty(bool flag)
{
   Observer::dirty = flag;
}

MultipleObserver::MultipleObserver()
{
   obsVec.clear();
}

MultipleObserver::MultipleObserver(Observable* obs)
{
   obsVec.clear();
   if( obs )
   {
      obs->addObserver(this);
      obsVec.push_back(obs);
   }
   
}

void MultipleObserver::addObserved(Observable* obs)
{
   if( obs )
   {
      obs->addObserver(this);
      obsVec.push_back(obs);
   }
}

void MultipleObserver::removeObserved(Observable* obs)
{
   QVector<Observable*>::iterator iter;
   
   if( !obs )
      return;
   
   obs->removeObserver(this);
   for( iter = obsVec.begin(); iter != obsVec.end(); iter++ )
      if( *iter == obs )
      {
         obsVec.erase(iter);
         return;
      }
}

void MultipleObserver::removeAllObserved()
{
   unsigned int i, size;

   size = obsVec.size();
   for( i = 0; i < size; ++i )
      obsVec[i]->removeObserver(this);

   obsVec.clear();
}
