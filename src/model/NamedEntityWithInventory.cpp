/*
 * model/NamedEntityWithInventory.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/NamedEntityWithInventory.h"

#include <QDebug>

#include "model/Inventory.h"
#include "model/NamedParameterBundle.h"

NamedEntityWithInventory::NamedEntityWithInventory(QString t_name,
                                                   bool t_display,
                                                   QString folder) :
   NamedEntity   {t_name, t_display, folder},
   m_inventory_id{-1} {
   return;
}

NamedEntityWithInventory::NamedEntityWithInventory(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity   {namedParameterBundle},
   // If we're reading in from a BeerXML file, there won't be an inventory ID
   m_inventory_id{namedParameterBundle(PropertyNames::NamedEntityWithInventory::inventoryId, -1)} {
   return;
}

NamedEntityWithInventory::NamedEntityWithInventory(NamedEntityWithInventory const & other) :
   NamedEntity{other},
   // Don't copy Inventory ID as new Fermentable/Hop/etc should have its own inventory - unless it's a child, but that
   // case is handled in makeChild() below
   m_inventory_id {-1} {
   return;
}

void NamedEntityWithInventory::makeChild(NamedEntity const & copiedFrom) {
   // First do the base class work
   this->NamedEntity::makeChild(copiedFrom);

   // Now we want the child to share the same inventory item as its parent
   this->m_inventory_id = static_cast<NamedEntityWithInventory const &>(copiedFrom).m_inventory_id;
   return;
}

void NamedEntityWithInventory::setInventoryId(int key) {
   if (key < 1) {
      // This really shouldn't happen
      qCritical() << Q_FUNC_INFO << this->metaObject()->className() << "Bad inventory id:" << key;
      Q_ASSERT(false); // Bail on debug build
      return;          // Continue (without setting invalid ID) otherwise
   }
   this->setAndNotify(PropertyNames::NamedEntityWithInventory::inventoryId, this->m_inventory_id, key);
   return;
}

int NamedEntityWithInventory::inventoryId() const {
   return m_inventory_id;
}
