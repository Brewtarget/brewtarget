/*
 * EquipmentButton.cpp is part of Brewtarget, and is copyright the following
 * authors 2009-2021:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mat Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "EquipmentButton.h"

#include <QWidget>

#include "model/Equipment.h"
#include "model/Recipe.h"

EquipmentButton::EquipmentButton(QWidget* parent) :
   QPushButton(parent),
   m_rec(nullptr),
   m_equip(nullptr) {
   return;
}

void EquipmentButton::setRecipe(Recipe* rec) {
   if (this->m_rec) {
      disconnect(this->m_rec, nullptr, this, nullptr );
   }

   this->m_rec = rec;
   if (this->m_rec) {
      connect(this->m_rec, &NamedEntity::changed, this, &EquipmentButton::recChanged );
      this->setEquipment(this->m_rec->equipment() );
   } else {
      this->setEquipment(nullptr);
   }
   return;
}

void EquipmentButton::setEquipment(Equipment* equip) {
   if (this->m_equip) {
      disconnect(this->m_equip, nullptr, this, nullptr);
   }

   this->m_equip = equip;
   if (this->m_equip) {
      connect(this->m_equip, &NamedEntity::changed, this, &EquipmentButton::equipChanged );
      setText(this->m_equip->name() );
   } else {
      setText("");
   }
   return;
}

void EquipmentButton::equipChanged(QMetaProperty prop, QVariant val) {
   if (prop.name() == PropertyNames::NamedEntity::name) {
      this->setText( val.toString() );
   }
   return;
}

void EquipmentButton::recChanged(QMetaProperty prop, QVariant val) {
   if (prop.name() == PropertyNames::Recipe::equipment) {
      this->setEquipment(val.value<Equipment *>());
   }
   return;
}
