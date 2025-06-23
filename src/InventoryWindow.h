/*======================================================================================================================
 * InventoryWindow.h is part of Brewtarget, and is copyright the following authors 2025:
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
 =====================================================================================================================*/
#ifndef INVENTORYWINDOW_H
#define INVENTORYWINDOW_H
#pragma once

#include <QDialog>

#include "ui_inventoryWindow.h"

/**
 * \brief A window showing inventory of Fermentables, Hops, Miscs, Salts, Yeasts
 *
 *        We track purchases:
 *           Type
 *           ID
 *           Ingredient ID
 *           Date Acquired (Purchased)
 *           Supplier (Retailer)
 *           Amount Type (mass, volume, etc)
 *           Nominal Quantity acquired
 *           Actual Quantity acquired = Nominal Quantity + sum of all corrections
 *           Quantity remaining = Actual Quantity acquired - sum of all usages
 *           Currency
 *           Price Paid
 *           Shipping Cost
 *           Best Before Date
 *
 *        And usages against each purchase:
 *           ID
 *           Inventory (Purchase) ID
 *           Date
 *           Quantity used
 *           Use Type (Used in recipe, Other use, correction)
 *           BrewNode ID (for used in recipe)
 *
 */
class InventoryWindow : public QDialog, public Ui::inventoryWindow {
   Q_OBJECT

public:
   //! \brief Default constructor.
   InventoryWindow(QWidget * parent = 0);
   ~InventoryWindow();

};

#endif
