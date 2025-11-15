/*======================================================================================================================
 * model/StockUseBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef MODEL_STOCKUSEBASE_H
#define MODEL_STOCKUSEBASE_H
#pragma once

#include "database/ObjectStoreWrapper.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockUseBase { inline BtStringConst const property{#property}; }
AddPropertyName(amountRemaining  )
AddPropertyName(amountUsed       )
AddPropertyName(quantityRemaining)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Small CRTP base class to provide templated code for inventory classes: \c StockUseHop, \c StockUseFermentable,
 *        \c StockUseMisc, \c StockUseSalt, \c StockUseYeast.
 *
 * \param Derived            = the derived class, eg \c StockUseHop
 * \param StockPurchaseClass = the inventoryclass, eg \c StockPurchaseHop
 */
template<class Derived> class StockUsePhantom;
template<class Derived, class StockPurchaseClass>
class StockUseBase : public CuriouslyRecurringTemplateBase<StockUsePhantom, Derived> {
protected:
   StockUseBase() { return; }
   StockUseBase([[maybe_unused]] NamedParameterBundle const & namedParameterBundle) { return; }
   StockUseBase([[maybe_unused]] StockUseBase const & other) { return; }

   ~StockUseBase() = default;

public:
   static QString localisedName_amountUsed       () { return Derived::tr("Amount Used"     ); }
   static QString localisedName_amountRemaining  () { return Derived::tr("Amount Remaining"); }
   static QString localisedName_quantityRemaining() { return Derived::tr("Quantity Remaining"); }

   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //
   // Note that, since StockUseBase and StockPurchaseBase both refer to each other, they can't both call each other's
   // member functions.  It's more important for StockPurchaseBase to access StockUseBase member functions (so it can
   // connect signals), so we allow dependencies in that direction.  This means here we can't call
   // this->derived().owner()->measure().  We must push that down into the .cpp file of Derived.
   //
   Measurement::Amount amountUsed() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityUsed()};
   }

   void updateCachedQuantityRemaining(double const val) const {
      this->m_cachedQuantityRemaining = val;
      return;
   }

   double quantityRemaining() const {
      if (!this->m_cachedQuantityRemaining) {
         // If the cached value hasn't been set then we need to ask the derived class to ask its owning StockPurchase
         // item (which we can't access directly) to set it.
         this->derived().recalculateCachedQuantityRemaining();
      }
      Q_ASSERT(this->m_cachedQuantityRemaining);
      return *this->m_cachedQuantityRemaining;
   }

   Measurement::Amount amountRemaining() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityRemaining()};
   }

   /**
    * \brief Convenience function to set \c quantityUsed with units.  (NOTE that it's the caller's responsibility to
    *        ensure that they are the same as for the parent \c StockPurchase item's \c amountReceived.)
    */
   void setAmountUsed(Measurement::Amount const & val) {
      //
      // We can only check against the parent object if it's been set.  (Although EnumeratedBase gives Derived
      // the member function `owner()`, we can't call that from this base class as Derived's inheritance from
      // EnumeratedBase hasn't yet happened.)
      //
      if (this->derived().ownerId() > 0) {
         Measurement::Unit const * inventoryUnit = this->derived().unit();
         if (val.unit != inventoryUnit) {
            //
            // This probably isn't OK, but it's not catastrophic to solidier on
            //
            qWarning() <<
               Q_FUNC_INFO << "Setting" << *val.unit << "on amount used, but parent StockPurchase is measured in" <<
               *inventoryUnit;
         }
      }
      this->derived().setQuantityUsed(val.quantity);

      return;
   }

   //================================================ Member variables =================================================

   //! \brief This gets written to by \c StockPurchaseBase via subclasses of \c StockPurchase.
   mutable std::optional<double> m_cachedQuantityRemaining = std::nullopt;

};

template<class Derived, class IngredientClass>
TypeLookup const StockUseBase<Derived, IngredientClass>::typeLookup {
   "StockUseBase",
   {
      //
      // See comments in other CRTP base classes for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::StockUseBase::amountUsed,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockUseBase::amountUsed>>(
          PropertyNames::StockUseBase::amountUsed,
          StockUseBase::localisedName_amountUsed,
          TypeLookupOf<MemberFunctionReturnType_t<&StockUseBase::amountUsed>>::value,
          Derived::IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::StockUseBase::quantityRemaining,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockUseBase::quantityRemaining>>(
          PropertyNames::StockUseBase::quantityRemaining,
          StockUseBase::localisedName_quantityRemaining,
          TypeLookupOf<MemberFunctionReturnType_t<&StockUseBase::quantityRemaining>>::value,
          TypeInfo::Access::ReadOnly,
          Derived::IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::StockUseBase::amountRemaining,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockUseBase::amountRemaining>>(
          PropertyNames::StockUseBase::amountRemaining,
          StockUseBase::localisedName_amountRemaining,
          TypeLookupOf<MemberFunctionReturnType_t<&StockUseBase::amountRemaining>>::value,
          TypeInfo::Access::ReadOnly,
          Derived::IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Concrete derived classes should (either directly or via inclusion in an intermediate class's equivalent macro)
 *        include this in their header file, right after Q_OBJECT.  Concrete derived classes also need to include the
 *        following block (see comment in model/StepBase.h for why):
 *
 *           //! See \c StockUseBase for getters and setters
 *           Q_PROPERTY(double              quantityRemaining   READ quantityRemaining   STORED false)
 *           Q_PROPERTY(Measurement::Amount amountUsed          READ amountUsed          STORED false)
 *           Q_PROPERTY(Measurement::Amount amountRemaining     READ amountRemaining     STORED false)
 *
 *        Additionally, the corresponding ItemDelegate class needs to be declared (but not defined) before the class
 *        definition.
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define STOCK_USE_COMMON_DECL(IngredientName) \
friend class StockUseBase;     \
friend class StockUseBase<StockUse##IngredientName, IngredientName>;     \
/* Owner class is also our friend as it calls updateCachedQuantityRemaining */         \
friend class StockPurchase##IngredientName;                                                \
                                                                                       \
public:                                                                                \
   static TypeLookup const typeLookup;                                                 \
   TYPE_LOOKUP_GETTER                                                                  \
                                                                                       \
   using EditorClass       = StockUse##IngredientName##Editor;                  \
   using IngredientClass   = IngredientName;                                           \
   using OwnerClass        = StockPurchase##IngredientName;                                \
   using ItemDelegateClass = StockUse##IngredientName##ItemDelegate;            \
   using TableModelClass   = StockUse##IngredientName##TableModel;              \
                                                                                       \
   StockUse##IngredientName(QString const & name = "");                         \
   StockUse##IngredientName(NamedParameterBundle const & namedParameterBundle); \
   StockUse##IngredientName(StockUse##IngredientName const & other);     \
   ~StockUse##IngredientName();                                                 \
                                                                                       \
   Measurement::PhysicalQuantity measure() const;                                      \
   Measurement::Unit const * unit() const;                                             \
                                                                                       \
private:                                                                               \
   void recalculateCachedQuantityRemaining() const;                                    \

/**
 * \brief Concrete derived classes should include this in their header but \b outside the class definition.
 */
#define STOCK_USE_COMMON_TMPL(IngredientName) \
   /* Because StockUse subclasses inherit from multiple bases, more than one of which has a match for */ \
   /* operator<<, we need to provide an overload of operator<< that is a better match than any of the others */ \
   /* (and that combines the output of those for all the base classes).                                      */ \
   template<class S>                                                                                            \
   S & operator<<(S & stream, StockUse##IngredientName const & val) {                                    \
      stream <<                                                                                                 \
         static_cast<NamedEntity const &>(val) << " " <<                                                        \
         static_cast<EnumeratedBase<StockUse##IngredientName, StockPurchase##IngredientName> const &>(val);  \
      return stream;                                                                                            \
   }                                                                                                            \


/**
 * \brief Derived classes should (either directly or via inclusion in an intermediate class's equivalent macro) include
 *        this in their implementation file.
 */
#define STOCK_USE_COMMON_CODE(IngredientName) \
TypeLookup const StockUse##IngredientName::typeLookup {                                              \
   "StockUse" #IngredientName,                                                                       \
   {                                                                                                        \
   },                                                                                                       \
   /* Parent class lookup. */                                                                               \
   {&StockUse::typeLookup,                                                                           \
    std::addressof(StockUseBase<StockUse##IngredientName, IngredientName>::typeLookup),       \
    std::addressof(EnumeratedBase<StockUse##IngredientName, StockPurchase##IngredientName>::typeLookup)} \
};                                                                                                       \
StockUse##IngredientName::StockUse##IngredientName(QString const & name) :                               \
   StockUse(name),                                                                                       \
   StockUseBase<StockUse##IngredientName, StockPurchase##IngredientName>(),                              \
   EnumeratedBase     <StockUse##IngredientName, StockPurchase##IngredientName>() {                      \
   CONSTRUCTOR_END                                                                                       \
   return;                                                                                               \
}                                                                                                        \
StockUse##IngredientName::StockUse##IngredientName(NamedParameterBundle const & namedParameterBundle) :  \
   StockUse(namedParameterBundle),                                                                       \
   StockUseBase  <StockUse##IngredientName, StockPurchase##IngredientName>(namedParameterBundle),        \
   EnumeratedBase<StockUse##IngredientName, StockPurchase##IngredientName>(namedParameterBundle) {       \
   CONSTRUCTOR_END                                                                                       \
   return;                                                                                               \
}                                                                                                        \
StockUse##IngredientName::StockUse##IngredientName(StockUse##IngredientName const & other) :             \
   StockUse(other),                                                                                      \
   StockUseBase<StockUse##IngredientName, StockPurchase##IngredientName>(other),                         \
   EnumeratedBase     <StockUse##IngredientName, StockPurchase##IngredientName>(other) {                 \
   /* NB: We do not copy m_cachedQuantityRemaining! */                                                   \
   CONSTRUCTOR_END                                                                                       \
   return;                                                                                               \
}                                                                                                        \
StockUse##IngredientName::~StockUse##IngredientName() = default;                                         \
                                                                                                         \
Measurement::PhysicalQuantity StockUse##IngredientName::measure() const {                     \
   /* If we don't yet have an owner, we need a default response */                            \
   return this->ownerId() <= 0 ? IngredientName::defaultMeasure : this->owner()->measure();   \
}                                                                                             \
                                                                                              \
Measurement::Unit const * StockUse##IngredientName::unit() const {                            \
   /* If we don't yet have an owner, we need a default response */                            \
   return this->ownerId() <= 0 ?                                                              \
      &Measurement::Unit::getCanonicalUnit(IngredientName::defaultMeasure) :                  \
      this->owner()->unit();                                                                  \
}                                                                                             \
                                                                                              \
void StockUse##IngredientName::recalculateCachedQuantityRemaining() const {                   \
   this->owner()->doUpdateChangeItems();                                                      \
   return;                                                                                    \
}                                                                                             \

#endif
