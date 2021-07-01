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

NamedEntityWithInventory::NamedEntityWithInventory(Brewtarget::DBTable table, bool cache, QString t_name, bool t_display, QString folder) :
   NamedEntity   {table, cache, t_name, t_display},
   m_inventory{0},
   m_inventory_id{-1} {
   return;
}

NamedEntityWithInventory::NamedEntityWithInventory(TableSchema* table, QSqlRecord rec, int t_key) :
   NamedEntity(table, rec, t_key),
   m_inventory{0} {

   // keys need special handling
   m_inventory_id = rec.value( table->foreignKeyToColumn(PropertyNames::NamedEntityWithInventory::inventoryId)).toInt();

   return;
}

NamedEntityWithInventory::NamedEntityWithInventory(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity   {namedParameterBundle},
   m_inventory_id{namedParameterBundle(PropertyNames::NamedEntityWithInventory::inventoryId).toInt()} {
   return;
}

NamedEntityWithInventory::NamedEntityWithInventory(NamedEntityWithInventory const & other) :
   NamedEntity     {other                 },
   // Don't copy Inventory ID as new object should have its own inventory - unless it's a child
   m_inventory{0},
   m_inventory_id {-1} {
   return;
}

void NamedEntityWithInventory::setInventoryId(int key) {
   if( key < 1 ) {
      qWarning() << Q_FUNC_INFO << this->metaObject()->className() << ": bad inventory id:" << key;
      return;
   }
   m_inventory_id = key;
   if ( ! m_cacheOnly ) {
      setEasy(PropertyNames::NamedEntityWithInventory::inventoryId, key);
   }
   return;
}

int NamedEntityWithInventory::inventoryId() const {
   return m_inventory_id;
}


// changes to inventory amounts do NOT create a version
void NamedEntityWithInventory::setInventoryAmount(double num) {
   if( num < 0.0 ) {
      qWarning() << Q_FUNC_INFO << this->metaObject()->className() << ": negative inventory:" << num;
      return;
   }

   m_inventory = num;
   if ( ! m_cacheOnly ) {
      if( m_inventory_id < 1 ) {
         qWarning() << Q_FUNC_INFO << this->metaObject()->className() << ": bad inventory id:" << m_inventory_id;
         return;
      }
      setInventory(num,m_inventory_id);
   }
}

double NamedEntityWithInventory::inventory() const {
   if ( m_inventory < 0 ) {
      m_inventory = getInventory().toDouble();
   }
   return m_inventory;
}
