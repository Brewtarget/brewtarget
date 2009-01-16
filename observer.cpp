/*
 * observer.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <vector>
#include "observable.h"
#include "observable.h"

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
   observed->addObserver(this);
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
   std::vector<Observable*>::iterator iter;
   
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
