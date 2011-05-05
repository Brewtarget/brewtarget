/*
 * observable.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _OBSERVABLE_H
#define   _OBSERVABLE_H
#include <QVector>
#include <QVariant>

class Observable;
class Observer;
class MultipleObserver;

class Observable
{
public:
   Observable();
   Observable(Observer* obs);
   void addObserver(Observer* obs);
   bool removeObserver(Observer* obs); // Returns true if successful.
   void hasChanged(QVariant info = QVariant());

   // In case notification causes weird problems.
   void disableNotification();
   void reenableNotification();

   // In case you need it. Forces a notification of observers.
   void forceNotify();
   
private:
   QVector<Observer*> observers;
   bool doNotify;
   
   void notifyObservers(QVariant info);
   void setDefaults();
};

class Observer
{
public:
   Observer();
   Observer(Observable* obs);
   void setObserved(Observable* obs);
   
   virtual void notify(Observable *notifier, QVariant info) = 0; // This will get called by observed whenever it changes.
   
private:
   Observable* observed;
};

class MultipleObserver : public Observer
{
public:
   MultipleObserver();
   MultipleObserver(Observable* obs);
   void addObserved(Observable* obs);
   void removeObserved(Observable* obs);
   void removeAllObserved();
   
   // virtual void notify(Observable* notifier, QVariant info); // Don't forget to overload from class Observer.
   
private:
   QVector<Observable*> obsVec;
};

#endif   /* _OBSERVABLE_H */

