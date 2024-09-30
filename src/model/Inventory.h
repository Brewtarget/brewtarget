/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Inventory.h is part of Brewtarget, and is copyright the following authors 2021-2024:
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
#ifndef MODEL_INVENTORY_H
#define MODEL_INVENTORY_H
#pragma once

#include <memory>

#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Ingredient.h"
#include "model/NamedEntity.h"
#include "utils/MetaTypes.h"

class ObjectStore;
class TypeLookup;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Inventory { BtStringConst const property{#property}; }
AddPropertyName(ingredientId)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

//
// Only classes that derive from Ingredient have inventory.
//
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <typename NE> concept CONCEPT_FIX_UP    CanHaveInventory = std::is_base_of_v<Ingredient, NE>;
template <typename NE> concept CONCEPT_FIX_UP CannotHaveInventory = std::negation_v<std::is_base_of<Ingredient, NE>>;

/**
 * \brief Class representing an inventory entry for Hop/Fermentable/Yeast/Misc
 *
 *        Initial version of this class holds rather minimal data, but we envisage expanding it in future.  In
 *        particular, we would like to be able to hold multiple Inventory objects for a given Ingredient object,
 *        representing multiple purchases of that ingredient (potentially with different prices, expiry dates, etc).
 *
 *        Subclasses need to supply a `using IngredientClass` alias analogous to the `using InventoryClass` one in the
 *        Ingredient classes.  This is handled automatically by the INVENTORY_DECL macro below.
 *
 *        NB: We want each type of inventory to be a different class so that it works with \c ObjectStoreTyped
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
class Inventory : public NamedEntity {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Inventory();
   Inventory(NamedParameterBundle const & namedParameterBundle);
   Inventory(Inventory const & other);

   virtual ~Inventory();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(int    ingredientId     READ ingredientId     WRITE setIngredientId    )
   /**
    * These properties are defined here with virtual accessors.  Child classes actually get the implementations by
    * inheriting from \c IngredientAmount.
    */
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   int ingredientId() const;

   virtual Measurement::Amount           amount  () const = 0;
   virtual double                        quantity() const = 0;
   virtual Measurement::Unit const *     unit    () const = 0;
   virtual Measurement::PhysicalQuantity measure () const = 0;
   virtual bool                          isWeight() const = 0;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setIngredientId(int const val);

   virtual void setAmount  (Measurement::Amount           const & val) = 0;
   virtual void setQuantity(double                        const   val) = 0;
   virtual void setUnit    (Measurement::Unit const *     const   val) = 0;
   virtual void setMeasure (Measurement::PhysicalQuantity const   val) = 0;
   virtual void setIsWeight(bool                          const   val) = 0;

   //============================================= OTHER MEMBER FUNCTIONS ==============================================

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
    * \brief We need this for ObjectStoreTyped to call
    */
   void hardDeleteOwnedEntities();

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

   int m_ingredientId;
};

/**
 * \brief For templates that require a parameter to be a subclass of \c Inventory, this makes the concept requirement
 *        slightly more concise.
 *
 *        See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it).
 */
template <typename T> concept CONCEPT_FIX_UP IsInventory = std::is_base_of_v<Inventory, T>;

namespace InventoryTools {
   /**
   * \return First found \c Inventory subclass object exists for the supplied \c Ingredient subclass object.  Or
   *         \c nullptr if none is found.
   */
   template<IsInventory Inv, IsIngredient Ing>
   std::shared_ptr<Inv> firstInventory(Ing const & ing) {
      auto ingredientId = ing.key();
      auto result = ObjectStoreWrapper::findFirstMatching<Inv>(
         [ingredientId](std::shared_ptr<Inv> inventory) {
            return inventory->ingredientId() == ingredientId;
         }
      );
      return result;
   }

   /**
   * \return \c true if at least one \c Inventory subclass object exists for the supplied \c Ingredient subclass object;
   *         \c false otherwise.
   */
   template<IsIngredient Ing>
   bool hasInventory(Ing const & ing) {
      auto result = InventoryTools::firstInventory<typename Ing::InventoryClass, Ing>(ing);
      // Although smart pointers can be treated as booleans inside if statements (eg `if (result)` etc) they are not
      // implicitly convertible to bool in other circumstances.  The double negation here is a trick to get around this
      // which avoids a cast or something painful such as `result ? true : false`.
      return !!result;
   }

   /**
   * \return A suitable \c Inventory subclass object for the supplied \c Ingredient subclass object.  If the former does
   *         not exist, it will be created.
   */
   template<IsIngredient Ing>
   std::shared_ptr<typename Ing::InventoryClass> getInventory(Ing const & ing) {
      //
      // At the moment, we assume there is at most one Inventory object per ingredient object.  In time we would like to
      // extend this to manage, eg, different purchases/batches as separate Inventory items, but that's for another day.
      //
      auto result = firstInventory<typename Ing::InventoryClass, Ing>(ing);
      if (result) {
         return result;
      }

      auto newInventory = std::make_shared<typename Ing::InventoryClass>();
      newInventory->setIngredientId(ing.key());
      // Even though the Inventory base class does not have a setQuantity member function, we know that all its
      // subclasses will, so this line will be fine when this template function is instantiated.
      newInventory->setQuantity(0.0);
      ObjectStoreWrapper::insert<typename Ing::InventoryClass>(newInventory);
      return newInventory;
   }
}

/**
 * \brief Subclasses of \c Inventory should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define INVENTORY_DECL(IngredientName, LcIngredientName) \
public:                                                                            \
   /** \brief See comment in model/NamedEntity.h */                                \
   static QString localisedName();                                                 \
                                                                                   \
   /** \brief See \c NamedEntity::typeLookup. */                                   \
   static TypeLookup const typeLookup;                                             \
   TYPE_LOOKUP_GETTER                                                              \
                                                                                   \
   using IngredientClass = IngredientName;                                         \
                                                                                   \
   Inventory##IngredientName();                                                    \
   Inventory##IngredientName(NamedParameterBundle const & namedParameterBundle);   \
   Inventory##IngredientName(Inventory##IngredientName const & other);             \
                                                                                   \
   virtual ~Inventory##IngredientName();                                           \
                                                                                   \
public:                                                                            \
   IngredientName * LcIngredientName() const ;                                     \
                                                                                   \
protected:                                                                         \
   virtual bool isEqualTo(NamedEntity const & other) const;                        \
   virtual ObjectStore & getObjectStoreTypedInstance() const;                      \

/**
 * \brief Subclasses of \c Inventory should include this in their implementation file.
 *        (Currently, that's Inventory.cpp for everything.)
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
ObjectStore & Inventory##IngredientName::getObjectStoreTypedInstance() const {                                       \
   return ObjectStoreTyped<Inventory##IngredientName>::getInstance();                                                \
}                                                                                                                    \
bool Inventory##IngredientName::isEqualTo(NamedEntity const & other) const {                                         \
   Inventory##IngredientName const & rhs = static_cast<Inventory##IngredientName const &>(other);                    \
   return (this->m_amount == rhs.m_amount && this->Inventory::isEqualTo(other));                                     \
}                                                                                                                    \
/* All properties are defined in base classes */                                                                     \
TypeLookup const Inventory##IngredientName::typeLookup {                                                             \
   "Inventory"#IngredientName, { },                                                                                  \
   {&Inventory::typeLookup, std::addressof(IngredientAmount<Inventory##IngredientName, IngredientName>::typeLookup)} \
};                                                                                                                   \
static_assert(std::is_base_of<Inventory, Inventory##IngredientName>::value);                                         \
Inventory##IngredientName::Inventory##IngredientName() :                                                             \
   Inventory{},                                                                                                      \
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
   return ObjectStoreWrapper::getByIdRaw<IngredientName>(this->m_ingredientId);                                      \
}                                                                                                                    \

#endif
