/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/IngredientAmount.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_INGREDIENTAMOUNT_H
#define MODEL_INGREDIENTAMOUNT_H
#pragma once

#include <type_traits>

#include "measurement/Unit.h"
#include "model/NamedParameterBundle.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/EnumStringMapping.h"
#include "utils/TypeLookup.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::IngredientAmount { inline BtStringConst const property{#property}; }
AddPropertyName(amount  )
AddPropertyName(isWeight) // Deprecated.  Used only for BeerXML support
AddPropertyName(measure )
AddPropertyName(quantity) // Only quantity and unit should be used in database mappings
AddPropertyName(unit    ) // Only quantity and unit should be used in database mappings
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Represents an amount of an ingredient.  These amounts are used in two places: in the \c RecipeAddition
 *        subclasses for the amount of an ingredient being added to a \c Recipe; and in the \c Inventory subclasses for
 *        the amount of an ingredient held in stock.
 *
 *        In our model, different types of ingredients are allowed to be measured in different ways:
 *
 *           \c Salt can be measured only by mass
 *           \c Water can be measured only by volume
 *           \c Fermentable and \c Hop can be measured either by mass or by volume
 *           \c Misc and \c Yeast can be measured by mass, by volume or by count
 *
 *        Typically, for things that can be measured more than one way, it is the individual instance of a class that
 *        determines the measurement.  Eg, a \c Hop would be measured by mass if it is leaves, pellets or powder, but by
 *        volume if it is an extract.  For other things, there isn't a rule, and it's a case-by-case decision for the
 *        brewer.   Eg, for dry \c Yeast it's the brewer's choice to measure by packets or mass.  For \c Misc, it might
 *        even vary by recipe as to whether you're adding, say, half an apple or 500 grams of apple.
 *
 *        So, for, ingredient types where we allow choice of how-to-measure, we pick a sensible default and let the user
 *        change it where needed.
 *
 *        Classes inheriting from this one, should place the \c INGREDIENT_AMOUNT_DECL macro in their header class
 *        declaration (eg immediately (typically on the line after after the \c Q_OBJECT macro), and the
 *        \c INGREDIENT_AMOUNT_COMMON_CODE macro in their .cpp file.  They also need the following lines in their class
 *        declaration:
 *
 *              Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
 *              Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
 *              Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
 *              Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
 *              Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)
 *
 *        Note that we cannot insert these lines with the \c INGREDIENT_AMOUNT_DECL (or another macro) because the Qt
 *        Meta Object Compiler aka MOC does not expand non-Qt macros (or at least does not do so before processing
 *        Q_PROPERTY).  The end-result of trying to insert these lines with our own macro would be a run-time error that
 *        the property does not exist.
 *
 *        Note that, because the Qt MOC doesn't understand templating or type aliases, it is simpler to keep the type of
 *        the \c amount property as \c Measurement::Amount.  The casting to and from the templated
 *        \c Measurement::ConstrainedAmount subclass is safe because \c Measurement::ConstrainedAmount does not add any
 *        data members to \c Measurement::Amount, and so has the same layout in memory.
 *
 *        ADDITIONALLY, it is necessary in the class declaration of the corresponding ingredient (eg in Hop.h for hops)
 *        to add publicly accessible:
 *           static constexpr Measurement::VariantPhysicalQuantity validMeasures = ...
 *           static constexpr Measurement::PhysicalQuantity defaultMeasure = ...
 *
 *        TBD: Using "measure" for the \c PhysicalQuantity property is a bit of a compromise.  I didn't want to call it
 *             "physicalQuantity" because that would look odd next to "quantity".
 *
 *        ---
 *
 *        Because this class is essentially just adding a couple of fields to its "owner" (eg \c RecipeAdditionHop,
 *        \c InventoryHop), it doesn't merit being a full-fledged \c NamedEntity with its own separate database table.
 *        Nonetheless, we do want the fields of this class to be stored in the database(!) but just as extra columns on
 *        the tables used by the "owner" classes.  And we want to be able to take advantage of utility functions such as
 *        \c NamedEntity::setAndNotify().  Using the Curiously Recurring Template (CRTP) pattern allows us to piggy-back
 *        the fields of this class onto the "owner" class at the cost of some slight ugliness/complexity in the code,
 *        which we mostly hide from the "owner" class with macros.
 *
 *        ---
 *
 *        We could perhaps have aligned this class more closely with \c Measurement::Amount, and used
 *        \c Measurement::PhysicalQuantity and \c Measurement::MixedPhysicalQuantities instead of \c defaultMeasure and
 *        \c validMeasures etc.  This would simplify some things, but be at the cost of making the class more generic
 *        than we want it to be.  Eg, we never want to measure ingredients by temperature!
 */
template<class Derived> class IngredientAmountPhantom;
template<class Derived, class IngredientClass>
class IngredientAmount : public CuriouslyRecurringTemplateBase<IngredientAmountPhantom, Derived> {

protected:

   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   IngredientAmount() :
      m_amount{0.0, Measurement::Unit::getCanonicalUnit(IngredientClass::defaultMeasure)} {
      return;
   }

   IngredientAmount(NamedParameterBundle const & namedParameterBundle) {
      if (namedParameterBundle.contains(PropertyNames::IngredientAmount::quantity)) {
         this->m_amount.quantity = namedParameterBundle.val<double>(PropertyNames::IngredientAmount::quantity);
         this->m_amount.unit     = namedParameterBundle.val<Measurement::Unit const *>(
            PropertyNames::IngredientAmount::unit,
            &Measurement::Unit::getCanonicalUnit(IngredientClass::defaultMeasure)
         );
         return;
      }

      this->m_amount = namedParameterBundle.val<Measurement::Amount>(PropertyNames::IngredientAmount::amount);
      return;
   }

   IngredientAmount(IngredientAmount const & other) :
      m_amount{other.m_amount} {
      return;
   }

   ~IngredientAmount() = default;

public:

   using ValidMeasuresType = std::remove_const_t<decltype(IngredientClass::validMeasures)>;
   using AmountType = Measurement::ConstrainedAmount<ValidMeasuresType, IngredientClass::validMeasures>;

   AmountType getAmount() const {
      return this->m_amount;
   }

   double getQuantity() const {
      return this->m_amount.quantity;
   }

   Measurement::Unit const * getUnit() const {
      return this->m_amount.unit;
   }

   Measurement::PhysicalQuantity getMeasure () const {
      if (this->m_amount.unit) {
         return this->m_amount.unit->getPhysicalQuantity();
      }
      return IngredientClass::defaultMeasure;
   }

   bool amountIsWeight() const {
      return this->m_amount.unit->getPhysicalQuantity() == Measurement::PhysicalQuantity::Mass;
   }

   /**
    * \brief This function is used (as a parameter to std::sort) for sorting in the recipe formatter
    */
   [[nodiscard]] static bool lessThanByWeight(IngredientAmount const & lhs, IngredientAmount const & rhs) {

      // Sort by name if the two fermentables are of equal weight or volume
      if (lhs.getMeasure() == Measurement::PhysicalQuantity::Mass &&
          rhs.getMeasure() == Measurement::PhysicalQuantity::Mass &&
          qFuzzyCompare(lhs.getQuantity(), rhs.getQuantity())) {
         return lhs.derived().name() < rhs.derived().name();
      }

      // .:TBD:. Do we want to separate out liquids and solids?

      // Yes. I know. This seems silly, but I want the returned list in descending not ascending order.
      return lhs.getQuantity() > rhs.getQuantity();
   }

   template<typename RA>
   [[nodiscard]] static bool lessThanByWeight(std::shared_ptr<RA> const lhs, std::shared_ptr<RA> const rhs) {
      return lessThanByWeight(*lhs, *rhs);
   }

   //! This is only for BeerXML support
   bool getIsWeight() const {
      return (this->getMeasure() == Measurement::PhysicalQuantity::Mass);
   }

   void doSetAmount(AmountType const & val) {
      //
      // For the moment, we keep the database layer and update one column from one property, hence the split into two
      // separate calls here.  If we ended up doing this sort of stuff in a lot of places, we could expand the
      // capabilities of ObjectStore etc to handle compound types such as Measurement::Amount.  Note, however, that very
      // often, the unit is not changing, so a lot of the time this is still only one DB call.
      //
      this->doSetQuantity(val.quantity);
      this->doSetUnit    (val.unit    );
      return;
   }

   void doSetQuantity(double const val) {
      this->derived().setAndNotify(PropertyNames::IngredientAmount::quantity,
                                   this->m_amount.quantity, val);
      return;
   }

   void doSetUnit(Measurement::Unit const * val) {
      // It's a coding error to provide an amount that is not in canonical units
      Q_ASSERT(val->isCanonical());
      this->derived().setAndNotify(PropertyNames::IngredientAmount::unit,
                                   this->m_amount.unit, val);
      return;
   }

   void doSetMeasure (Measurement::PhysicalQuantity const val) {
      // Since Q_ASSERT is a macro, it gets confused by some templated expressions, so we have to have this separate
      // variable
      bool const measureIsValid = Measurement::isValid<ValidMeasuresType, IngredientClass::validMeasures>(val);
      Q_ASSERT(measureIsValid);
      this->doSetUnit(&Measurement::Unit::getCanonicalUnit(val));
      return;
   }

   //! This is only for BeerXML support
   void doSetIsWeight(bool const val) {
      // In BeerXML, amount not being weight implies that it's volume.  Obviously that's not more generally true, hence
      // this property / function only being for BeerXML support.
      this->doSetMeasure(val ? Measurement::PhysicalQuantity::Mass : Measurement::PhysicalQuantity::Volume);
      return;
   }

protected:
   AmountType m_amount;
};

template<class Derived, class IngredientClass>
TypeLookup const IngredientAmount<Derived, IngredientClass>::typeLookup {
   "IngredientAmount",
   {
      //
      // We can't use the PROPERTY_TYPE_LOOKUP_ENTRY or PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here, because macros
      // don't know how to handle templated references such as `IngredientAmount<Derived, IngredientClass>::m_quantity`
      // and try to treat them as two macro parameters).  So we have to put the raw code instead.
      //
      // (We can't get around this by writing `using IngredientAmountType = IngredientAmount<Derived, IngredientClass>`
      // because Derived and IngredientClass aren't defined until the template is instantiated.)
      //
      // This does show the advantage of being able to use the macros elsewhere! :)
      //
      {&PropertyNames::IngredientAmount::amount,
       TypeInfo::construct<decltype(IngredientAmount<Derived, IngredientClass>::m_amount)>(
          PropertyNames::IngredientAmount::amount,
          TypeLookupOf<decltype(IngredientAmount<Derived, IngredientClass>::m_amount)>::value,
          IngredientClass::validMeasures
       )},
      {&PropertyNames::IngredientAmount::quantity,
       TypeInfo::construct<decltype(IngredientAmount<Derived, IngredientClass>::m_amount.quantity)>(
          PropertyNames::IngredientAmount::quantity,
          TypeLookupOf<decltype(IngredientAmount<Derived, IngredientClass>::m_amount.quantity)>::value,
          IngredientClass::validMeasures
       )},
      {&PropertyNames::IngredientAmount::unit,
       TypeInfo::construct<decltype(IngredientAmount<Derived, IngredientClass>::m_amount.unit)>(
          PropertyNames::IngredientAmount::unit,
          TypeLookupOf<decltype(IngredientAmount<Derived, IngredientClass>::m_amount.unit)>::value
       )},
      {&PropertyNames::IngredientAmount::measure,
       TypeInfo::construct<MemberFunctionReturnType_t<&IngredientAmount::getMeasure>>(
          PropertyNames::IngredientAmount::measure,
          TypeLookupOf<MemberFunctionReturnType_t<&IngredientAmount::getMeasure>>::value,
          NonPhysicalQuantity::Enum
       )},
      {&PropertyNames::IngredientAmount::isWeight,
       TypeInfo::construct<MemberFunctionReturnType_t<&IngredientAmount::amountIsWeight>>(
          PropertyNames::IngredientAmount::isWeight,
          TypeLookupOf<MemberFunctionReturnType_t<&IngredientAmount::amountIsWeight>>::value,
          NonPhysicalQuantity::Bool
       )},
   },
   // Parent class lookup: none as we are at the top of this arm of the inheritance tree
   {}
};


/**
 * \brief Concrete derived classes should (either directly or via inclusion in an intermediate class's equivalent macro)
 *        include this in their header file, right after Q_OBJECT.  Concrete derived classes also need to include the
 *        following block (see comment in model/StepBase.h for why):
 *
 *           // See model/IngredientAmount.h for info, getters and setters for these properties
 *           Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
 *           Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
 *           Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
 *           Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
 *           Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)
 *
 *        Comments for these properties:
 *
 *           \c amount : Convenience to access \c quantity and \c unit simultaneously
 *
 *           \c quantity :
 *
 *           \c unit :
 *
 *           \c measure :
 *
 *           \c isWeight : Mostly for BeerXML support
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define INGREDIENT_AMOUNT_DECL(Derived, IngredientClass) \
   /* This allows IngredientAmount to call protected and private members of Derived. */ \
   /* Note that a friend statement can either apply to all instances of              */ \
   /* IngredientAmount or to one specialisation.  It cannot apply to a partial       */ \
   /* specialisation.  Hence why we need to specify IngredientClass here.            */ \
   friend class IngredientAmount<Derived, IngredientClass>;                             \
                                                                                            \
   public:                                                                                  \
   /*=========================== IA "GETTER" MEMBER FUNCTIONS ===========================*/ \
   Measurement::Amount           amount  () const;                                          \
   double                        quantity() const;                                          \
   Measurement::Unit const *     unit    () const;                                          \
   Measurement::PhysicalQuantity measure () const;                                          \
   bool                          isWeight() const;                                          \
   /*=========================== IA "SETTER" MEMBER FUNCTIONS ===========================*/ \
   void setAmount  (Measurement::Amount           const & val);                             \
   void setQuantity(double                        const   val);                             \
   void setUnit    (Measurement::Unit const *     const   val);                             \
   void setMeasure (Measurement::PhysicalQuantity const   val);                             \
   void setIsWeight(bool                          const   val);                             \

/**
 * \brief Derived classes should include this in their .cpp file
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define INGREDIENT_AMOUNT_COMMON_CODE(Derived, IngredientClass) \
   /*============================ "GETTER" MEMBER FUNCTIONS ============================*/ \
   Measurement::Amount           Derived::amount  () const { return this->getAmount  (); } \
   double                        Derived::quantity() const { return this->getQuantity(); } \
   Measurement::Unit const *     Derived::unit    () const { return this->getUnit    (); } \
   Measurement::PhysicalQuantity Derived::measure () const { return this->getMeasure (); } \
   bool                          Derived::isWeight() const { return this->getIsWeight(); } \
   /*============================ "SETTER" MEMBER FUNCTIONS ============================*/ \
   void Derived::setAmount  (Measurement::Amount           const & val) { this->doSetAmount  (val); return; } \
   void Derived::setQuantity(double                        const   val) { this->doSetQuantity(val); return; } \
   void Derived::setUnit    (Measurement::Unit const *     const   val) { this->doSetUnit    (val); return; } \
   void Derived::setMeasure (Measurement::PhysicalQuantity const   val) { this->doSetMeasure (val); return; } \
   void Derived::setIsWeight(bool                          const   val) { this->doSetIsWeight(val); return; } \

#endif
