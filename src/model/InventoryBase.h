/*======================================================================================================================
 * model/InventoryBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef MODEL_INVENTORYBASE_H
#define MODEL_INVENTORYBASE_H
#pragma once

#include "utils/CuriouslyRecurringTemplateBase.h"

///template<class Derived> class IngredientUserPhantom;
///template<class Derived, class Ingr>
///class IngredientUserBase : public CuriouslyRecurringTemplateBase<IngredientUserPhantom, Derived> {
///};


/**
 * \brief Small CRTP base class to provide templated code for inventory classes: \c InventoryHop,
 *        \c InventoryFermentable, \c InventoryMisc, \c InventoryYeast.
 *
 * \param Derived = the derived class, eg \c InventoryHop
 * \param Ingr    = the ingredient class, eg \c Hop
 *
 * NOTE: Derived classes need to have a static member function \c instanceNameTemplate for the translated name of class
 *       instances (eg `tr("Addition of %1 hops")` or `tr("Use of %1 water")`).  The declaration of this is handled by
 *       the \c RECIPE_ADDITION_DECL macro, but the trivial class-specific definition needs to done by hand.  Note that
 *       \c instanceNameTemplate is a member function rather than a member variable for the same reasons as
 *       \c localisedName.  See comment in \c model/NamedEntity.h.
 */
template<class Derived> class InventoryPhantom;
template<class Derived, class Ingr>
class InventoryBase : public CuriouslyRecurringTemplateBase<InventoryPhantom, Derived> {
protected:
   InventoryBase() {
      return;
   }

   ~InventoryBase() = default;

public:

   //
   // This alias makes it easier to template a number of functions that are essentially the same for all subclasses of
   // Inventory.  Eg InventoryHop::IngredientClass is Hop; InventoryYeast::IngredientClass is Yeast; etc.
   // Note that the alias and the template parameter cannot have the same name, hence why we use Ingr for the latter.
   //
   using IngredientClass = Ingr;

   Measurement::Amount amountOrdered() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityOrdered()};
   }

   //! Convenience function so we don't have to remember to use IngredientAmount::amount() for amountReceived
   Measurement::Amount amountReceived() const {
      return this->derived().amount();
   }

   Measurement::Amount amountRemaining() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityRemaining()};
   }
};

#endif
