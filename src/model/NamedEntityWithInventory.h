/*
 * model/NamedEntityWithInventory.h is part of Brewtarget, and is Copyright the following
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
#ifndef MODEL_NAMEDENTITYWITHINVENTORY_H
#define MODEL_NAMEDENTITYWITHINVENTORY_H
#pragma once

#include "model/NamedEntity.h"

namespace PropertyNames::NamedEntityWithInventory { static char const * const inventory = "inventory"; /* previously kpropInventory */ }
namespace PropertyNames::NamedEntityWithInventory { static char const * const inventoryId = "inventoryId"; /* previously kpropInventoryId */ }

/**
 * \class NamedEntityWithInventory
 *
 * \brief Extends \c NamedEntity to provide functionality for storing in Inventory
 */
class NamedEntityWithInventory : public NamedEntity {
   Q_OBJECT
public:
   NamedEntityWithInventory(Brewtarget::DBTable table, bool cache = true, QString t_name = QString(), bool t_display = false, QString folder = QString());
   NamedEntityWithInventory(TableSchema* table, QSqlRecord rec, int t_key = -1);
   NamedEntityWithInventory(NamedEntityWithInventory const & other);
   NamedEntityWithInventory(NamedParameterBundle const & namedParameterBundle);

   virtual ~NamedEntityWithInventory() = default;

   //! \brief The amount in inventory (usually in kg)
   Q_PROPERTY( double inventory              READ inventory              WRITE setInventoryAmount        /*NOTIFY changed*/ /*changedInventory*/ )
   //! \brief The inventory table id, needed for signals
   Q_PROPERTY( double inventoryId            READ inventoryId            WRITE setInventoryId            /*NOTIFY changed*/ /*changedInventoryId*/ )

   virtual double inventory() const;
   int inventoryId() const;

   virtual void setInventoryAmount(double amount);
   void setInventoryId(int key);

protected:
   mutable double m_inventory;
   int m_inventory_id;
};

#endif
