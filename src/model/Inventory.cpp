/**
 * model/Inventory.cpp is part of Brewtarget, and is copyright the following
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
#include "model/Inventory.h"

#include "database/ObjectStoreWrapper.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/Yeast.h"

namespace {

   //
   // A set of template functions and specialisations that sort of allow us to treat InventoryHop as Inventory<Hop>, etc
   //

   template<class Ing> ObjectStore & getInventoryObjectStore();
   template<> ObjectStore & getInventoryObjectStore<Fermentable>() { return ObjectStoreTyped<InventoryFermentable>::getInstance(); }
   template<> ObjectStore & getInventoryObjectStore<Hop>()         { return ObjectStoreTyped<InventoryHop>::getInstance();         }
   template<> ObjectStore & getInventoryObjectStore<Misc>()        { return ObjectStoreTyped<InventoryMisc>::getInstance();        }
   template<> ObjectStore & getInventoryObjectStore<Yeast>()       { return ObjectStoreTyped<InventoryYeast>::getInstance();       }

   template<class Ing> std::shared_ptr<Inventory> newInventory();
   template<> std::shared_ptr<Inventory> newInventory<Fermentable>() { return std::static_pointer_cast<Inventory>(std::make_shared<InventoryFermentable>()); }
   template<> std::shared_ptr<Inventory> newInventory<Hop>()         { return std::static_pointer_cast<Inventory>(std::make_shared<InventoryHop>());         }
   template<> std::shared_ptr<Inventory> newInventory<Misc>()        { return std::static_pointer_cast<Inventory>(std::make_shared<InventoryMisc>());        }
   template<> std::shared_ptr<Inventory> newInventory<Yeast>()       { return std::static_pointer_cast<Inventory>(std::make_shared<InventoryYeast>());       }

}

//
// This private implementation class holds all private non-virtual members of Inventory
//
class Inventory::impl {
public:

   //
   // Constructors
   //
   impl() : id    {-1},
            amount{0.0} {
      return;
   }

   impl(NamedParameterBundle const & namedParameterBundle) :
      id    {namedParameterBundle(PropertyNames::Inventory::id).toInt()       },
      amount{namedParameterBundle(PropertyNames::Inventory::amount).toDouble()} {
      return;
   }

   impl(impl const & other) :
      id    {other.id    },
      amount{other.amount} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   int id;
   double amount;
};


Inventory::Inventory() : pimpl{ new impl{} } {
   return;
}

Inventory::Inventory(NamedParameterBundle const & namedParameterBundle) :
   pimpl{ new impl{namedParameterBundle} } {
   return;
}

Inventory::Inventory(Inventory const & other) :
   pimpl{ new impl{*other.pimpl} } {
   return;
}


Inventory::~Inventory() = default;


int    Inventory::getId() const      { return this->pimpl->id; }
double Inventory::getAmount() const  { return this->pimpl->amount; }

void Inventory::setId(int id) {
   this->pimpl->id = id;
   return;
}

void Inventory::setAmount(double amount) {
   this->pimpl->amount = amount;
   this->getObjectStoreTypedInstance().updateProperty(*this, PropertyNames::Inventory::amount);
   // .:TBD:. Do we need to send any signals here?  Or should we do that in updateProperty?
   return;
}

void Inventory::setDeleted(bool var) {
   // See comment in header.  This is not currently implemented and it's therefore a coding error if it gets called
   Q_ASSERT(false);
   return;
}

void Inventory::setDisplay(bool var) {
   // See comment in header.  This is not currently implemented and it's therefore a coding error if it gets called
   Q_ASSERT(false);
   return;
}

void Inventory::hardDeleteOwnedEntities() {
   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << "owns no other entities";
   return;
}


char const * InventoryFermentable::getIngredientClass() const { return "Fermentable"; }
char const * InventoryHop::getIngredientClass() const         { return "Hop"; }
char const * InventoryMisc::getIngredientClass() const        { return "Misc"; }
char const * InventoryYeast::getIngredientClass() const       { return "Yeast"; }

ObjectStore & InventoryFermentable::getObjectStoreTypedInstance() const { return ObjectStoreTyped<InventoryFermentable>::getInstance(); }
ObjectStore & InventoryHop::getObjectStoreTypedInstance() const         { return ObjectStoreTyped<InventoryHop>::getInstance(); }
ObjectStore & InventoryMisc::getObjectStoreTypedInstance() const        { return ObjectStoreTyped<InventoryMisc>::getInstance(); }
ObjectStore & InventoryYeast::getObjectStoreTypedInstance() const       { return ObjectStoreTyped<InventoryYeast>::getInstance(); }

template<class Ing>
void InventoryUtils::setAmount(Ing & ing, double amount) {
   // Callers shouldn't try to set negative amounts, but filter here just in case
   if (amount < 0.0) {
      qWarning() << Q_FUNC_INFO << ing.metaObject()->className() << ": negative inventory: " << amount;
      return;
   }

   ObjectStore & inventoryObjectStore = getInventoryObjectStore<Ing>();

   int invId = ing.inventoryId();
   if (invId > 0) {
      // The easy case: set an amount in an existing inventory entry
      auto inventory = std::static_pointer_cast<Inventory>(inventoryObjectStore.getById(invId));
      inventory->setAmount(amount);
      return;
   }

   // There isn't an inventory entry so (a) create a new one and set the amount...
   auto inventory = newInventory<Ing>();
   inventory->setAmount(amount);
   // ...(b) store it...
   inventoryObjectStore.insert(std::static_pointer_cast<QObject>(inventory));
   // ...(c) tell the ingredient (and its parent, children, siblings) that it now has an inventory
   QVector<int> idsOfParentIngredientAndItsChildren = ing.getParentAndChildrenIds();
   auto parentIngredientAndItsChildren = inventoryObjectStore.getByIds(idsOfParentIngredientAndItsChildren);
   for (auto ii : parentIngredientAndItsChildren) {
      std::static_pointer_cast<Ing>(ii)->setInventoryId(inventory->getId());
   }
   return;
}
// Instantiate the above template for all the classes we care about.
// This is just a trick to avoid having the template definition in the header file.
template void InventoryUtils::setAmount(Fermentable & ing, double amount);
template void InventoryUtils::setAmount(Hop & ing,         double amount);
template void InventoryUtils::setAmount(Misc & ing,        double amount);
template void InventoryUtils::setAmount(Yeast & ing,       double amount);

template<class Ing>
double InventoryUtils::getAmount(Ing const & ing) {
   ObjectStore & inventoryObjectStore = getInventoryObjectStore<Ing>();
   int invId = ing.inventoryId();
   if (invId > 0) {
      auto inventory = std::static_pointer_cast<Inventory>(inventoryObjectStore.getById(invId));
      return inventory->getAmount();
   }

   // There isn't an inventory for this object, so we don't have any stock of it
   return 0.0;
}
template double InventoryUtils::getAmount(Fermentable const & ing);
template double InventoryUtils::getAmount(Hop const & ing        );
template double InventoryUtils::getAmount(Misc const & ing       );
template double InventoryUtils::getAmount(Yeast const & ing      );
