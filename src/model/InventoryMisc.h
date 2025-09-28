/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/InventoryMisc.h is part of Brewtarget, and is copyright the following authors 2023-2025:
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef MODEL_INVENTORYMISC_H
#define MODEL_INVENTORYMISC_H
#pragma once

#include <QObject>
#include <QString>

#include "model/Misc.h"
#include "model/Inventory.h"
#include "model/IngredientAmount.h"

/**
 * \brief Inventory of \c Misc
 */
class InventoryMisc : public Inventory, public IngredientAmount<InventoryMisc, Misc> {
   Q_OBJECT

   INGREDIENT_AMOUNT_DECL(InventoryMisc, Misc)
   INVENTORY_DECL(Misc, misc)

   // See model/IngredientAmount.h for info, getters and setters for these properties
   Q_PROPERTY(int                           ingredientId READ ingredientId WRITE setIngredientId)
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)
};

#endif
