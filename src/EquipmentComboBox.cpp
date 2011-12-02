/*
 * EquipmentComboBox.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "EquipmentComboBox.h"
#include <QList>

EquipmentComboBox::EquipmentComboBox(QWidget* parent)
        : QComboBox(parent), recipe(0)
{
   setCurrentIndex(-1);
   connect( Database::instance(), SIGNAL(changed(QMetaProperty,QVariant)), this SLOT(changed(QMetaProperty,QVariant)) );
   repopulateList();
}

void EquipmentComboBox::addEquipment(Equipment* equipment)
{
   if( !equipments.contains(equipment) )
      equipments.append(equipment);
   connect( equipment, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

   addItem( equipment->getName() );
}

void EquipmentComboBox::removeEquipment(Equipment* equipment)
{
   disconnect( equipment, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   int ndx = equipments.indexOf(equipment);
   if( ndx > 0 )
   {
      equipments.removeAt(ndx);
      removeItem(ndx);
   }
   
}

//void EquipmentComboBox::notify(Observable *notifier, QVariant info)
void EquipmentComboBox::changed(QMetaProperty prop, QVariant val)
{
   unsigned int i, size;

   // Notifier could be the database.
   if( sender() == &(Database::instance()) &&
       prop.propertyIndex() == Database::instance().metaObject().indexOfProperty("equipments") )
   {
      Equipment* previousSelection = getSelected();
      
      repopulateList();

      // Need to reset the selected entry if we observe a recipe.
      if( recipeObs && recipe->getEquipment() )
         setIndexByEquipment( recipe->getEquipment() );
      // Or, try to select the same thing we had selected last.
      else if( previousSelection )
         setIndexByEquipment(previousSelection);
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else if( sender() == recipe )
   {
      // Only respond if the equipment changed.
      if( prop.propertyIndex() != recipe->metaObject().indexOfProperty("equipment") )
         return;
      
      // All we care about is the equipment in the recipe.
      if( recipe->getEquipment() )
         setIndexByEquipment( recipeObs->getEquipment() );
      else
         setCurrentIndex(-1); // Or just give up.
   }
   else // Otherwise, we know that one of the equipments changed.
   {
      Equipment* e = qobject_cast<Equipment*>(sender());
      i = equipments.indexOf(e);
      if( i > 0 )
         setItemText(i, equipments[i]->getName());
   }
}

void EquipmentComboBox::setIndexByEquipment(Equipment* e)
{
   int ndx;

   ndx = equipments.indexOf(e);
   setCurrentIndex(ndx);
}

void EquipmentComboBox::repopulateList()
{
   int i, size;
   clear();

   // Disconnect all current equipments.
   size = equipments.size();
   for( i = 0; i < size; ++i )
      removeEquipment(equipments[i]);
   
   // Get the new list of equipments.
   Database::instance().getEquipments( equipments );
   
   // Connect and add all new equipments.
   size = equipments.size();
   for( i = 0; i < size; ++i )
      addEquipment(equipments[i]);
}

Equipment* EquipmentComboBox::getSelected()
{
   if( currentIndex() >= 0 )
      return equipments[currentIndex()];
   else
      return 0;
}

void EquipmentComboBox::observeRecipe(Recipe* rec)
{
   if( rec )
   {
      if( recipe )
         disconnect( recipe, 0, 0, 0 );
      recipe = rec;
      connect( recipe, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

      if( recipe->getEquipment() )
         setIndexByEquipment( recipe->getEquipment() );
      else
         setCurrentIndex(-1);
   }
}
