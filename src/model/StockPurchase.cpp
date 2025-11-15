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
 =====================================================================================================================*/
#include "model/StockPurchase.h"

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"
#include "utils/TypeLookup.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StockPurchase.cpp"
#endif

QString StockPurchase::localisedName() { return tr("StockPurchase"); }
QString StockPurchase::localisedName_dateOrdered      () { return tr("Date Ordered"      ); }
QString StockPurchase::localisedName_dateReceived     () { return tr("Date Received"     ); }
QString StockPurchase::localisedName_dateBestBefore   () { return tr("Best Before Date"  ); }
QString StockPurchase::localisedName_supplier         () { return tr("Supplier"          ); }
QString StockPurchase::localisedName_note             () { return tr("Note"              ); }
QString StockPurchase::localisedName_quantityOrdered  () { return tr("Quantity Ordered"  ); }
QString StockPurchase::localisedName_purchasePrice    () { return tr("Purchase Price"    ); }
QString StockPurchase::localisedName_purchaseTax      () { return tr("Purchase Tax"      ); }
QString StockPurchase::localisedName_shippingCost     () { return tr("Shipping Cost"     ); }
QString StockPurchase::localisedName_ingredient       () { return tr("Ingredient"        ); }

bool StockPurchase::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StockPurchase const & rhs = static_cast<StockPurchase const &>(other);
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateOrdered    , PropertyNames::StockPurchase::dateOrdered    , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateReceived   , PropertyNames::StockPurchase::dateReceived   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_dateBestBefore , PropertyNames::StockPurchase::dateBestBefore , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_supplier       , PropertyNames::StockPurchase::supplier       , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_note           , PropertyNames::StockPurchase::note           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_quantityOrdered, PropertyNames::StockPurchase::quantityOrdered, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_purchasePrice  , PropertyNames::StockPurchase::purchasePrice  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_purchaseTax    , PropertyNames::StockPurchase::purchaseTax    , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_shippingCost   , PropertyNames::StockPurchase::shippingCost   , propertiesThatDiffer)
   );
}

TypeLookup const StockPurchase::typeLookup {
   "StockPurchase",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, dateOrdered      , m_dateOrdered    , NonPhysicalQuantity::Date),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, dateReceived     , m_dateReceived   , NonPhysicalQuantity::Date),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, dateBestBefore   , m_dateBestBefore , NonPhysicalQuantity::Date),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, supplier         , m_supplier       , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, note             , m_note           , NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, quantityOrdered  , m_quantityOrdered, NonPhysicalQuantity::Dimensionless),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, purchasePrice    , m_purchasePrice  , NonPhysicalQuantity::Currency),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, purchaseTax      , m_purchaseTax    , NonPhysicalQuantity::Currency),
      PROPERTY_TYPE_LOOKUP_ENTRY(StockPurchase, shippingCost     , m_shippingCost   , NonPhysicalQuantity::Currency),
   },
   {&NamedEntity::typeLookup}
};

StockPurchase::StockPurchase(QString const & name) :
   NamedEntity{name},
   m_dateOrdered    {},
   m_dateReceived   {},
   m_dateBestBefore {},
   m_supplier       {},
   m_note           {},
   m_quantityOrdered{},
   m_purchasePrice  {},
   m_purchaseTax    {},
   m_shippingCost   {} {

   CONSTRUCTOR_END
   return;
}

StockPurchase::StockPurchase(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_dateOrdered    , namedParameterBundle, PropertyNames::StockPurchase::dateOrdered    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_dateReceived   , namedParameterBundle, PropertyNames::StockPurchase::dateReceived   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_dateBestBefore , namedParameterBundle, PropertyNames::StockPurchase::dateBestBefore , std::nullopt),
   SET_REGULAR_FROM_NPB (m_supplier       , namedParameterBundle, PropertyNames::StockPurchase::supplier       , ""          ),
   SET_REGULAR_FROM_NPB (m_note           , namedParameterBundle, PropertyNames::StockPurchase::note           , ""          ),
   SET_REGULAR_FROM_NPB (m_quantityOrdered, namedParameterBundle, PropertyNames::StockPurchase::quantityOrdered, std::nullopt),
   SET_REGULAR_FROM_NPB (m_purchasePrice  , namedParameterBundle, PropertyNames::StockPurchase::purchasePrice  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_purchaseTax    , namedParameterBundle, PropertyNames::StockPurchase::purchaseTax    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_shippingCost   , namedParameterBundle, PropertyNames::StockPurchase::shippingCost   , std::nullopt) {

   CONSTRUCTOR_END
   return;
}

StockPurchase::StockPurchase(StockPurchase const & other) :
   NamedEntity      {other},
   m_dateOrdered    {other.m_dateOrdered    },
   m_dateReceived   {other.m_dateReceived   },
   m_dateBestBefore {other.m_dateBestBefore },
   m_supplier       {other.m_supplier       },
   m_note           {other.m_note           },
   m_quantityOrdered{other.m_quantityOrdered},
   m_purchasePrice  {other.m_purchasePrice  },
   m_purchaseTax    {other.m_purchaseTax    },
   m_shippingCost   {other.m_shippingCost   } {

   CONSTRUCTOR_END
   return;
}

StockPurchase::~StockPurchase() = default;

//============================================ "GETTER" MEMBER FUNCTIONS ============================================
std::optional<QDate>          StockPurchase::dateOrdered    () const { return this->m_dateOrdered    ; }
std::optional<QDate>          StockPurchase::dateReceived   () const { return this->m_dateReceived   ; }
std::optional<QDate>          StockPurchase::dateBestBefore () const { return this->m_dateBestBefore ; }
QString                       StockPurchase::supplier       () const { return this->m_supplier       ; }
QString                       StockPurchase::note           () const { return this->m_note           ; }
std::optional<double>         StockPurchase::quantityOrdered() const { return this->m_quantityOrdered; }
std::optional<CurrencyAmount> StockPurchase::purchasePrice  () const { return this->m_purchasePrice  ; }
std::optional<CurrencyAmount> StockPurchase::purchaseTax    () const { return this->m_purchaseTax    ; }
std::optional<CurrencyAmount> StockPurchase::shippingCost   () const { return this->m_shippingCost   ; }

//============================================ "SETTER" MEMBER FUNCTIONS ============================================
void StockPurchase::setDateOrdered    (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::dateOrdered    , this->m_dateOrdered    , val); return;}
void StockPurchase::setDateReceived   (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::dateReceived   , this->m_dateReceived   , val); return;}
void StockPurchase::setDateBestBefore (std::optional<QDate>          const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::dateBestBefore , this->m_dateBestBefore , val); return;}
void StockPurchase::setSupplier       (QString                       const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::supplier       , this->m_supplier       , val); return;}
void StockPurchase::setNote           (QString                       const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::note           , this->m_note           , val); return;}
void StockPurchase::setQuantityOrdered(std::optional<double>         const   val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::quantityOrdered, this->m_quantityOrdered, val); return;}
void StockPurchase::setPurchasePrice  (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::purchasePrice  , this->m_purchasePrice  , val); return;}
void StockPurchase::setPurchaseTax    (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::purchaseTax    , this->m_purchaseTax    , val); return;}
void StockPurchase::setShippingCost   (std::optional<CurrencyAmount> const & val) { SET_AND_NOTIFY(PropertyNames::StockPurchase::shippingCost   , this->m_shippingCost   , val); return;}
