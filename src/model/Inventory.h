/*
 * model/Inventory.h is part of Brewtarget, and is copyright the following
 * authors 2021:
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef MODEL_INVENTORY_H
#define MODEL_INVENTORY_H
#pragma once

#include <memory>

#include <QObject>

#include "model/NamedParameterBundle.h"

class ObjectStore;
class TypeLookup;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Inventory { BtStringConst const property{#property}; }
AddPropertyName(id)
AddPropertyName(amount)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief Class representing an inventory entry for Hop/Fermentable/Yeast/Misc
 *
 *        Initial version of this class holds rather minimal data, but we envisage expanding it in future
 *
 *        NB: When we add, eg, a Hop to a Recipe, we make a copy for various reasons (including that the amount of Hop
 *            used in the Recipe is stored in the Hop, not the Recipe).  Each such copy _shares_ its Inventory with the
 *            Hop from which it was copied (aka its parent).  Thus all the Hops with the same parent will have the
 *            same Inventory object as that parent (because they are not really different Hops, merely different usages
 *            of that parent hop).
 *
 *            We want each type of inventory to be a different class so that it works with \c ObjectStoreTyped
 *
 *            It would be tempting to make Inventory a templated class (for \c Inventory<Hop>,
 *            \c Inventory<Fermentable>, etc), however we need Inventory to inherit from QObject so we can use Qt
 *            Properties in the \c ObjectStore layer, and this precludes the use of templates.  (The Qt meta-object
 *            compiler, aka moc, does not understand C++ templates.)
 *
 *            Instead we use inheritance to get \c InventoryHop, \c InventoryFermentable, etc, which is a bit more
 *            clunky, but not a lot.  And there are a few tricks in the cpp file that still allow us to do some
 *            templating.
 */
class Inventory : public QObject {
   Q_OBJECT
public:
   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Inventory();
   Inventory(NamedParameterBundle const & namedParameterBundle);
   Inventory(Inventory const & other);

   ~Inventory();

   Q_PROPERTY(int    id     READ getId     WRITE setId    )
   Q_PROPERTY(double amount READ getAmount WRITE setAmount)

   /**
    * \brief Returns the ID of the Inventory object, which is unique for a given subclass of Inventory (eg InventoryHop)
    */
   int getId() const;

   /**
    * \brief Returns the amount of the ingredient in the inventory.  Note that the interpretation of this amount (eg,
    *        whether it's kilograms, liters, etc) is the responsibility of the ingredient class.
    */
   double getAmount() const;

   void setId(int id);
   void setAmount(double amount);

   /**
    * \brief Synonym for \c setId(), as it's needed for \c ObjectStoreTyped::hardDelete()
    */
   void setKey(int id);

   /**
    * \brief This doesn't actually do anything, but using ObjectStoreTyped means we have to provide an implementation,
    *        as it's needed for \c ObjectStoreTyped::softDelete().
    */
   void setDeleted(bool var);

   /**
    * \brief This doesn't actually do anything, but using ObjectStoreTyped means we have to provide an implementation,
    *        as it's needed for \c ObjectStoreTyped::softDelete().
    */
   void setDisplay(bool var);

   /**
    * \brief Returns the name of the ingredient class (eg Hop, Fermentable, Misc, Yeast) to which this Inventory class
    *        relates.  Subclasses need to provide the (trivial) implementation of this.  Primarily useful for logging
    *        and debugging.
    */
   virtual char const * getIngredientClass() const = 0;

   /**
    * \brief We need this for ObjectStoreTyped to call
    */
   void hardDeleteOwnedEntities();

protected:
   /**
    * \brief Subclasses need to override this function to return the appropriate instance of \c ObjectStoreTyped.
    */
   virtual ObjectStore & getObjectStoreTypedInstance() const = 0;

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;
};
// Thankfully C++11 allows us to inherit constructors using "using"
class InventoryHop         : public Inventory { using Inventory::Inventory; public: virtual char const * getIngredientClass() const; protected: virtual ObjectStore & getObjectStoreTypedInstance() const; };
class InventoryFermentable : public Inventory { using Inventory::Inventory; public: virtual char const * getIngredientClass() const; protected: virtual ObjectStore & getObjectStoreTypedInstance() const; };
class InventoryMisc        : public Inventory { using Inventory::Inventory; public: virtual char const * getIngredientClass() const; protected: virtual ObjectStore & getObjectStoreTypedInstance() const; };
class InventoryYeast       : public Inventory { using Inventory::Inventory; public: virtual char const * getIngredientClass() const; protected: virtual ObjectStore & getObjectStoreTypedInstance() const; };

namespace InventoryUtils {
   /**
    * \brief Helper function to set inventory amount for a given object
    */
   template<class Ing>
   void setAmount(Ing & ing, double amount);

   template<class Ing>
   double getAmount(Ing const & ing);
}

#endif
