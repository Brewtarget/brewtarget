/*
 * EquipmentComboBox.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _EQUIPMENTCOMBOBOX_H
#define	_EQUIPMENTCOMBOBOX_H

class EquipmentComboBox;

#include <QComboBox>
#include <QWidget>
#include <vector>
#include <string>
#include "observable.h"
#include "equipment.h"
#include "database.h"
#include "recipe.h"

class EquipmentComboBox : public QComboBox, public MultipleObserver
{
   Q_OBJECT

public:
   EquipmentComboBox(QWidget* parent=0);
   void startObservingDB();
   void observeRecipe(Recipe* rec);
   void addEquipment(Equipment* equipment);
   void setIndexByEquipmentName(std::string name);
   void removeAllEquipments();
   void repopulateList();

   Equipment* getSelected();

   virtual void notify(Observable *notifier); // This will get called by observed whenever it changes.

private:
   std::vector<Equipment*> equipmentObs;
   Recipe* recipeObs;
   Database* dbObs;
};

#endif	/* _EQUIPMENTCOMBOBOX_H */

