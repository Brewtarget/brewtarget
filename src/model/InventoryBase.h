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

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::InventoryBase { inline BtStringConst const property{#property}; }
AddPropertyName(amountOrdered  )
AddPropertyName(amountReceived )
AddPropertyName(amountRemaining)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

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
   static QString localisedName_amountOrdered  () { return Derived::tr("Amount Ordered"  ); }
   static QString localisedName_amountReceived () { return Derived::tr("Amount Received" ); }
   static QString localisedName_amountRemaining() { return Derived::tr("Amount Remaining"); }

   //
   // This alias makes it easier to template a number of functions that are essentially the same for all subclasses of
   // Inventory.  Eg InventoryHop::IngredientClass is Hop; InventoryYeast::IngredientClass is Yeast; etc.
   // Note that the alias and the template parameter cannot have the same name, hence why we use Ingr for the latter.
   //
   using IngredientClass = Ingr;

   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! \brief Convenience function to give \c quantityOrdered with units
   std::optional<Measurement::Amount> amountOrdered() const {
      auto const quantity{this->derived().quantityOrdered()};
      if (quantity) {
         return Measurement::Amount{this->derived().measure(), *quantity};
      }
      return std::nullopt;
   }

   //! Convenience function so we don't have to remember to use IngredientAmount::amount() for amountReceived
   Measurement::Amount amountReceived() const {
      return this->derived().amount();
   }

   //! \brief Convenience function to give \c quantityRemaining with units
   Measurement::Amount amountRemaining() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityRemaining()};
   }
};

template<class Derived, class IngredientClass>
TypeLookup const InventoryBase<Derived, IngredientClass>::typeLookup {
   "InventoryBase",
   {
      //
      // We can't use the PROPERTY_TYPE_LOOKUP_ENTRY or PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here, because macros
      // don't know how to handle templated references such as `InventoryBase<Derived, IngredientClass>::m_quantity`
      // and try to treat them as two macro parameters).  So we have to put the raw code instead.
      //
      // (We can't get around this by writing `using IngredientAmountType = InventoryBase<Derived, IngredientClass>`
      // because Derived and IngredientClass aren't defined until the template is instantiated.)
      //
      // This does show the advantage of being able to use the macros elsewhere! :)
      //
      {&PropertyNames::InventoryBase::amountOrdered,
       TypeInfo::construct<MemberFunctionReturnType_t<&InventoryBase::amountOrdered>>(
          PropertyNames::InventoryBase::amountOrdered,
          InventoryBase::localisedName_amountOrdered,
          TypeLookupOf<MemberFunctionReturnType_t<&InventoryBase::amountOrdered>>::value,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::InventoryBase::amountReceived,
       TypeInfo::construct<MemberFunctionReturnType_t<&InventoryBase::amountReceived>>(
          PropertyNames::InventoryBase::amountReceived,
          InventoryBase::localisedName_amountReceived,
          TypeLookupOf<MemberFunctionReturnType_t<&InventoryBase::amountReceived>>::value,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::InventoryBase::amountRemaining,
       TypeInfo::construct<MemberFunctionReturnType_t<&InventoryBase::amountRemaining>>(
          PropertyNames::InventoryBase::amountRemaining,
          InventoryBase::localisedName_amountRemaining,
          TypeLookupOf<MemberFunctionReturnType_t<&InventoryBase::amountRemaining>>::value,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Subclasses of \c Inventory should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define INVENTORY_DECL(IngredientName, LcIngredientName) \
public:                                                                            \
   /** \brief See comment in model/NamedEntity.h */                                \
   static QString localisedName();                                                 \
   static QString localisedName_##LcIngredientName();                              \
                                                                                   \
   /** \brief See \c NamedEntity::typeLookup. */                                   \
   static TypeLookup const typeLookup;                                             \
   TYPE_LOOKUP_GETTER                                                              \
                                                                                   \
   using IngredientClass = IngredientName;                                         \
                                                                                   \
   Inventory##IngredientName(QString const & name = "");                           \
   Inventory##IngredientName(NamedParameterBundle const & namedParameterBundle);   \
   Inventory##IngredientName(Inventory##IngredientName const & other);             \
                                                                                   \
   virtual ~Inventory##IngredientName();                                           \
                                                                                   \
public:                                                                            \
   IngredientName * LcIngredientName() const ;                                     \
                                                                                   \
protected:                                                                         \
   virtual bool compareWith(NamedEntity const & other,                             \
                            QList<BtStringConst const *> * propertiesThatDiffer) const override; \
   virtual ObjectStore & getObjectStoreTypedInstance() const override;             \

/**
 * \brief Subclasses of \c InventoryBase should include this in their implementation file.
 *
 *        Note that #IngredientName will expand to "Fermentable"/"Hop"/etc and that
 *           "Fermentable" " Inventory"
 *        is treated by the compiler exactly the same as
 *           "Fermentable Inventory"
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define INVENTORY_COMMON_CODE(IngredientName, LcIngredientName) \
QString Inventory##IngredientName::localisedName() { return tr(#IngredientName " Inventory"); }                      \
QString Inventory##IngredientName::localisedName_##LcIngredientName() { return IngredientName::localisedName(); }    \
ObjectStore & Inventory##IngredientName::getObjectStoreTypedInstance() const {                                       \
   return ObjectStoreTyped<Inventory##IngredientName>::getInstance();                                                \
}                                                                                                                    \
/* Most properties are defined in base classes */                                                                    \
TypeLookup const Inventory##IngredientName::typeLookup {                                                             \
   "Inventory"#IngredientName,                                                                                       \
   {                                                                                                                 \
      PROPERTY_TYPE_LOOKUP_NO_MV(Inventory##IngredientName, LcIngredientName, LcIngredientName),                     \
   },                                                                                                                \
   {&Inventory::typeLookup,                                                                                          \
    std::addressof(IngredientAmount<Inventory##IngredientName, IngredientName>::typeLookup),                         \
    std::addressof(InventoryBase   <Inventory##IngredientName, IngredientName>::typeLookup)                          \
   }                                                                                                                 \
};                                                                                                                   \
static_assert(std::is_base_of<Inventory, Inventory##IngredientName>::value);                                         \
Inventory##IngredientName::Inventory##IngredientName(QString const & name) :                                         \
   Inventory{name},                                                                                                  \
   IngredientAmount<Inventory##IngredientName, IngredientName>{} {                                                   \
   CONSTRUCTOR_END                                                                                                   \
   return;                                                                                                           \
}                                                                                                                    \
Inventory##IngredientName::Inventory##IngredientName(NamedParameterBundle const & npb) :                             \
   Inventory {npb},                                                                                                  \
   IngredientAmount<Inventory##IngredientName, IngredientName>{npb} {                                                \
   CONSTRUCTOR_END                                                                                                   \
   return;                                                                                                           \
}                                                                                                                    \
Inventory##IngredientName::Inventory##IngredientName(Inventory##IngredientName const & other) :                      \
   Inventory {other},                                                                                                \
   IngredientAmount<Inventory##IngredientName, IngredientName>{other} {                                              \
   CONSTRUCTOR_END                                                                                                   \
   return;                                                                                                           \
}                                                                                                                    \
Inventory##IngredientName::~Inventory##IngredientName() = default;                                                   \
IngredientName * Inventory##IngredientName::LcIngredientName() const {                                               \
   return ObjectStoreWrapper::getByIdRaw<IngredientName>(this->ingredientId());                                      \
}                                                                                                                    \

#endif
