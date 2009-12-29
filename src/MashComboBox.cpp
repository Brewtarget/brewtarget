/*
* MashComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "MashComboBox.h"

MashComboBox::MashComboBox(QWidget* parent)
: QComboBox(parent)
{
   setCurrentIndex(-1);
}

MashComboBox::~MashComboBox()
{
}

void MashComboBox::startObservingDB()
{
   if( Database::isInitialized() )
   {
      dbObs = Database::getDatabase();
      addObserved(dbObs);
      
      std::list<Mash*>::iterator it, end;
      
      end = dbObs->getMashEnd();
      
      for( it = dbObs->getMashBegin(); it != end; ++it )
         addMash(*it);
      repopulateList();
   }
}

void MashComboBox::addMash(Mash* m)
{
   mashObs.push_back(m);
   addObserved(m);
   
   addItem( m->getName().c_str() );
}

void MashComboBox::removeAllMashs()
{
   unsigned int i, size;
   size = mashObs.size();
   for( i = 0; i < size; ++i )
      removeObserved(mashObs[i]);
   mashObs.clear(); // Clear internal list.
   clear(); // Clear the combo box's visible list.
}

void MashComboBox::notify(Observable *notifier, QVariant info)
{
   unsigned int i, size;
   
   // Notifier could be the database. Only pay attention if the number of
   // mashs has changed.
   if( notifier == dbObs && (info.toInt() == DBMASH || info.toInt() == DBALL) )
   {
      removeAllMashs();
      std::list<Mash*>::iterator it, end;
      
      end = dbObs->getMashEnd();
      
      for( it = dbObs->getMashBegin(); it != end; ++it )
         addMash(*it);
      repopulateList();
   }
   else // Otherwise, we know that one of the mashs changed.
   {
      size = mashObs.size();
      for( i = 0; i < size; ++i )
         if( notifier == mashObs[i] )
         {
            // Notice we assume 'i' is an index into both 'mashObs' and also
            // to the text list in this combo box...
            setItemText( i, mashObs[i]->getName().c_str() );
         }
   }
}

void MashComboBox::setIndexByMashName(std::string name)
{
   int ndx;
   
   ndx = findText( name.c_str(), Qt::MatchExactly );
   
   setCurrentIndex(ndx);
}

void MashComboBox::setIndex(int ndx)
{
   setCurrentIndex(ndx);
}

void MashComboBox::repopulateList()
{
   unsigned int i, size;
   clear();
   
   size = mashObs.size();
   for( i = 0; i < size; ++i )
      addItem( mashObs[i]->getName().c_str() );
   
   setCurrentIndex(-1);
}

Mash* MashComboBox::getSelectedMash()
{
   if( currentIndex() >= 0 )
      return mashObs[currentIndex()];
   else
      return 0;
}
