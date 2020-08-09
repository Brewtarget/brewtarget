/*
 * EquipmentButton.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _EQUIPMENTBUTTON_H
#define _EQUIPMENTBUTTON_H

#include <QPushButton>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class Equipment;
class Recipe;
class QWidget;

/*!
 * \class EquipmentButton
 * \author Philip G. Lee (rocketman768@gmail.com)
 *
 * \brief This is a view class that displays the name of an equipment.
 */
class EquipmentButton : public QPushButton
{
   Q_OBJECT
public:
   EquipmentButton(QWidget* parent = nullptr);
   virtual ~EquipmentButton(){}

   //! Observe a recipe's equipment.
   void setRecipe(Recipe* recipe);
   //! Observe a particular equipment.
   void setEquipment(Equipment* equip);

private slots:
   void recChanged(QMetaProperty,QVariant);
   void equipChanged(QMetaProperty,QVariant);

private:
   Recipe* _rec;
   Equipment* _equip;
};

#endif
