/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Inventory.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "model/Inventory.h"

#include "database/ObjectStoreWrapper.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/InventoryFermentable.h"
#include "model/InventoryHop.h"
#include "model/Misc.h"
#include "model/NamedParameterBundle.h"
#include "model/Yeast.h"
#include "utils/TypeLookup.h"

// If we're compiling with CMake, the AUTOMOC property will run the Qt meta-object compiler (MOC) on InventoryHop.h (to
// produce moc_InventoryHop.cpp) etc but will not link the resulting code (because there is not a corresponding
// InventoryHop.cpp.  If we include the results of the MOC here, it guarantees they get linked into the final
// executable.
   #include "moc_InventoryFermentable.cpp"
   #include "moc_InventoryHop.cpp"
   #include "moc_InventoryMisc.cpp"
   #include "moc_InventorySalt.cpp"
   #include "moc_InventoryYeast.cpp"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Inventory.cpp"
#endif

QString Inventory::localisedName() { return tr("Inventory"); }
///QString Inventory::localisedName_ingredientId     () { return tr("Ingredient ID"     ); }
QString Inventory::localisedName_dateOrdered      () { return tr("Date Ordered"      ); }
QString Inventory::localisedName_dateReceived     () { return tr("Date Received"     ); }
QString Inventory::localisedName_dateBestBefore   () { return tr("Best Before Date"  ); }
QString Inventory::localisedName_supplier         () { return tr("Supplier"          ); }
QString Inventory::localisedName_quantityOrdered  () { return tr("Quantity Ordered"  ); }
QString Inventory::localisedName_purchasePrice    () { return tr("Purchase Price"    ); }
QString Inventory::localisedName_purchaseTax      () { return tr("Purchase Tax"      ); }
QString Inventory::localisedName_shippingCost     () { return tr("Shipping Cost"     ); }
QString Inventory::localisedName_quantityRemaining() { return tr("Quantity Remaining"); }
QString Inventory::localisedName_ingredient       () { return tr("Ingredient"        ); }

bool Inventory::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Inventory const & rhs = static_cast<Inventory const &>(other);
   return (
///      AUTO_PROPERTY_COMPARE(this, rhs, m_ingredientId   , PropertyNames::Inventory::ingredientId   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateOrdered    , PropertyNames::Inventory::dateOrdered    , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateReceived   , PropertyNames::Inventory::dateReceived   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateBestBefore , PropertyNames::Inventory::dateBestBefore , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_supplier       , PropertyNames::Inventory::supplier       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_quantityOrdered, PropertyNames::Inventory::quantityOrdered, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_purchasePrice  , PropertyNames::Inventory::purchasePrice  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_purchaseTax    , PropertyNames::Inventory::purchaseTax    , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_shippingCost   , PropertyNames::Inventory::shippingCost   , propertiesThatDiffer)
   );
}

TypeLookup const Inventory::typeLookup {
   "Inventory",
   {
///      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, ingredientId     , m_ingredientId   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, dateOrdered      , m_dateOrdered    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, dateReceived     , m_dateReceived   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, dateBestBefore   , m_dateBestBefore ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, supplier         , m_supplier       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, quantityOrdered  , m_quantityOrdered),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, purchasePrice    , m_purchasePrice  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, purchaseTax      , m_purchaseTax    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Inventory, shippingCost     , m_shippingCost   ),
      PROPERTY_TYPE_LOOKUP_NO_MV(Inventory, quantityRemaining, quantityRemaining),
///      PROPERTY_TYPE_LOOKUP_NO_MV(Inventory, ingredient       , ingredient       ),
   },
   {&NamedEntity::typeLookup}
};

Inventory::Inventory(QString const & name) :
   NamedEntity{name},
///   IngredientAmount{},
///   m_ingredientId   {-1},
   m_dateOrdered    {},
   m_dateReceived   {},
   m_dateBestBefore {},
   m_supplier       {},
   m_quantityOrdered{},
   m_purchasePrice  {},
   m_purchaseTax    {},
   m_shippingCost   {} {

   CONSTRUCTOR_END
   return;
}

Inventory::Inventory(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
///   IngredientAmount{namedParameterBundle},
///   SET_REGULAR_FROM_NPB (m_ingredientId   , namedParameterBundle, PropertyNames::Inventory::ingredientId   ),
   SET_REGULAR_FROM_NPB (m_dateOrdered    , namedParameterBundle, PropertyNames::Inventory::dateOrdered    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_dateReceived   , namedParameterBundle, PropertyNames::Inventory::dateReceived   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_dateBestBefore , namedParameterBundle, PropertyNames::Inventory::dateBestBefore , std::nullopt),
   SET_REGULAR_FROM_NPB (m_supplier       , namedParameterBundle, PropertyNames::Inventory::supplier       , ""          ),
   SET_REGULAR_FROM_NPB (m_quantityOrdered, namedParameterBundle, PropertyNames::Inventory::quantityOrdered, std::nullopt),
   SET_REGULAR_FROM_NPB (m_purchasePrice  , namedParameterBundle, PropertyNames::Inventory::purchasePrice  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_purchaseTax    , namedParameterBundle, PropertyNames::Inventory::purchaseTax    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_shippingCost   , namedParameterBundle, PropertyNames::Inventory::shippingCost   , std::nullopt) {

   CONSTRUCTOR_END
   return;
}

Inventory::Inventory(Inventory const & other) :
   NamedEntity      {other},
///   IngredientAmount {other},
///   m_ingredientId   {other.m_ingredientId   },
   m_dateOrdered    {other.m_dateOrdered    },
   m_dateReceived   {other.m_dateReceived   },
   m_dateBestBefore {other.m_dateBestBefore },
   m_supplier       {other.m_supplier       },
   m_quantityOrdered{other.m_quantityOrdered},
   m_purchasePrice  {other.m_purchasePrice  },
   m_purchaseTax    {other.m_purchaseTax    },
   m_shippingCost   {other.m_shippingCost   } {

   CONSTRUCTOR_END
   return;
}

Inventory::~Inventory() = default;

//============================================ "GETTER" MEMBER FUNCTIONS ============================================
///int                           Inventory::ingredientId   () const { return this->m_ingredientId   ; }
std::optional<QDate>          Inventory::dateOrdered    () const { return this->m_dateOrdered    ; }
std::optional<QDate>          Inventory::dateReceived   () const { return this->m_dateReceived   ; }
std::optional<QDate>          Inventory::dateBestBefore () const { return this->m_dateBestBefore ; }
QString                       Inventory::supplier       () const { return this->m_supplier       ; }
std::optional<double>         Inventory::quantityOrdered() const { return this->m_quantityOrdered; }
std::optional<CurrencyAmount> Inventory::purchasePrice  () const { return this->m_purchasePrice  ; }
std::optional<CurrencyAmount> Inventory::purchaseTax    () const { return this->m_purchaseTax    ; }
std::optional<CurrencyAmount> Inventory::shippingCost   () const { return this->m_shippingCost   ; }

// Calculated getters
double Inventory::quantityRemaining() const {
   // TODO Not yet implemented! ********************************************************8
//   Q_ASSERT(false);
   return 0.0;
}

//============================================ "SETTER" MEMBER FUNCTIONS ============================================
///void Inventory::setIngredientId   (int                           const   val) { SET_AND_NOTIFY(PropertyNames::Inventory::ingredientId   , this->m_ingredientId   , val); return;}
void Inventory::setDateOrdered    (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::dateOrdered    , this->m_dateOrdered    , val); return;}
void Inventory::setDateReceived   (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::dateReceived   , this->m_dateReceived   , val); return;}
void Inventory::setDateBestBefore (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::dateBestBefore , this->m_dateBestBefore , val); return;}
void Inventory::setSupplier       (QString                       const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::supplier       , this->m_supplier       , val); return;}
void Inventory::setQuantityOrdered(std::optional<double>         const   val) { SET_AND_NOTIFY(PropertyNames::Inventory::quantityOrdered, this->m_quantityOrdered, val); return;}
void Inventory::setPurchasePrice  (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::purchasePrice  , this->m_purchasePrice  , val); return;}
void Inventory::setPurchaseTax    (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::purchaseTax    , this->m_purchaseTax    , val); return;}
void Inventory::setShippingCost   (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::Inventory::shippingCost   , this->m_shippingCost   , val); return;}


void Inventory::setDeleted([[maybe_unused]] bool var) {
   // See comment in header.  This is currently a no-op.
   return;
}

void Inventory::setDisplay([[maybe_unused]] bool var) {
   // See comment in header.  This is currently a no-op.
   return;
}

void Inventory::hardDeleteOwnedEntities() {
   qDebug() << Q_FUNC_INFO << this->metaObject()->className() << "owns no other entities";
   return;
}
