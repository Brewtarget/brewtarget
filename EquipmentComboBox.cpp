/*
 * EquipmentComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "EquipmentComboBox.h"
#include <list>

EquipmentComboBox::EquipmentComboBox(QWidget* parent)
        : QComboBox(parent)
{
}

void EquipmentComboBox::startObservingDB()
{
   if( Database::isInitialized() )
   {
      dbObs = Database::getDatabase();
      addObserved(dbObs);

      std::list<Equipment*>::iterator it, end;

      end = dbObs->getEquipmentEnd();

      for( it = dbObs->getEquipmentBegin(); it != end; ++it )
         addEquipment(*it);
      repopulateList();
   }

   setCurrentIndex(-1);
}

void EquipmentComboBox::addEquipment(Equipment* equipment)
{
   equipmentObs.push_back(equipment);
   addObserved(equipment);

   addItem( tr(equipment->getName().c_str()) );
}

void EquipmentComboBox::removeAllEquipments()
{
   /*
   removeAllObserved(); // Don't want to observe anything.
   equipmentObs.clear(); // Delete internal list.
    */

   int i;
   for( i=0; i < equipmentObs.size(); ++i )
      removeObserved(equipmentObs[i]);
   equipmentObs.clear(); // Clear internal list.
   clear(); // Clear the combo box's visible list.
}

void EquipmentComboBox::notify(Observable *notifier)
{
   unsigned int i, size;

   // Notifier could be the database.
   if( notifier == dbObs )
   {
      removeAllEquipments();
      std::list<Equipment*>::iterator it, end;

      end = dbObs->getEquipmentEnd();

      for( it = dbObs->getEquipmentBegin(); it != end; ++it )
         addEquipment(*it);
      repopulateList();
   }
   else // Otherwise, we know that one of the equipments changed.
   {
      size = equipmentObs.size();
      for( i = 0; i < size; ++i )
         if( notifier == equipmentObs[i] )
         {
            // Notice we assume 'i' is an index into both 'equipmentObs' and also
            // to the text list in this combo box...
            setItemText(i, tr(equipmentObs[i]->getName().c_str()));
         }
   }
}

void EquipmentComboBox::setIndexByEquipmentName(std::string name)
{
   int ndx;

   ndx = findText( tr(name.c_str()), Qt::MatchExactly );
   if( ndx == -1 )
      return;

   setCurrentIndex(ndx);
}

void EquipmentComboBox::repopulateList()
{
   unsigned int i, size;
   clear();

   size = equipmentObs.size();
   for( i = 0; i < size; ++i )
      addItem( tr(equipmentObs[i]->getName().c_str()) );
}

Equipment* EquipmentComboBox::getSelected()
{
   if( currentIndex() >= 0 )
      return equipmentObs[currentIndex()];
   else
      return 0;
}
