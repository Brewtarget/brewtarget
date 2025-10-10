/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Inventory.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#include "measurement/CurrencyAmount.h"
#include "model/NamedParameterBundle.h"
#include "model/Ingredient.h"
#include "model/NamedEntity.h"
#include "utils/MetaTypes.h"
#include "utils/AutoCompare.h"

class ObjectStore;
class TypeLookup;


//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Inventory { inline BtStringConst const property{#property}; }
AddPropertyName(dateBestBefore   )
AddPropertyName(dateOrdered      )
AddPropertyName(dateReceived     )
AddPropertyName(purchasePrice    )
AddPropertyName(purchaseTax      )
AddPropertyName(quantityOrdered  )
AddPropertyName(quantityRemaining)
AddPropertyName(shippingCost     )
AddPropertyName(supplier         )
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
 *        An inventory entry (eg \c InventoryHop, \c InventoryFermentable) represents the acquisition (eg purchase) of
 *        an ingredient.  Each use of that ingredient from that inventory is tracked in an inventory use record (eg
 *        \c InventoryUseHop, \c InventoryUseFermentable).
 *
 *        The intent is to allow the user not just to say "How much of ingredient X do I have" but also to track
 *        separate purchases of the same ingredient so s/he can see price changes over time, do stock rotation (ie
 *        ensure older purchases used first) and track which ingredients are approaching their best-before date.
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
   static QString localisedName_dateOrdered      ();
   static QString localisedName_dateReceived     ();
   static QString localisedName_dateBestBefore   ();
   static QString localisedName_supplier         ();
   static QString localisedName_quantityOrdered  ();
   static QString localisedName_purchasePrice    ();
   static QString localisedName_purchaseTax      ();
   static QString localisedName_shippingCost     ();
   static QString localisedName_quantityRemaining();
   static QString localisedName_ingredient       ();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Inventory(QString const & name = "");
   Inventory(NamedParameterBundle const & namedParameterBundle);
   Inventory(Inventory const & other);

   virtual ~Inventory();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(std::optional<QDate> dateOrdered      READ dateOrdered      WRITE setDateOrdered   )
   Q_PROPERTY(std::optional<QDate> dateReceived     READ dateReceived     WRITE setDateReceived  )
   Q_PROPERTY(std::optional<QDate> dateBestBefore   READ dateBestBefore   WRITE setDateBestBefore)

   Q_PROPERTY(QString supplier   READ supplier   WRITE setSupplier)

   /**
    * \brief This is how much of the ingredient was ordered, whereas \c IngredientAmount::amount is the actual amount
    *        that arrived.  Often there is a small difference -- eg 1000g ordered, 1039g arrived.
    *
    *        Note that units for \c quantityOrdered are the same as \c IngredientAmount::amount, hence why we only store
    *        quantity for this field.
    */
   Q_PROPERTY(std::optional<double> quantityOrdered      READ quantityOrdered      WRITE setQuantityOrdered)


   /**
    * \brief Normally this will be in the same currency as \c QLocale::currencySymbol, but we allow the user to override
    *        currency, eg for cross-border purchases.
    */
   Q_PROPERTY(std::optional<CurrencyAmount> purchasePrice   READ purchasePrice   WRITE setPurchasePrice)
   Q_PROPERTY(std::optional<CurrencyAmount> purchaseTax     READ purchaseTax     WRITE setPurchaseTax  )
   Q_PROPERTY(std::optional<CurrencyAmount> shippingCost    READ shippingCost    WRITE setShippingCost )

   //! \brief Calculated quantity remaining
   Q_PROPERTY(double quantityRemaining   READ quantityRemaining   STORED false)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   std::optional<QDate>               dateOrdered    () const;
   std::optional<QDate>               dateReceived   () const;
   std::optional<QDate>               dateBestBefore () const;
   QString                            supplier       () const;
   std::optional<double>              quantityOrdered() const;
   std::optional<CurrencyAmount>      purchasePrice  () const;
   std::optional<CurrencyAmount>      purchaseTax    () const;
   std::optional<CurrencyAmount>      shippingCost   () const;
   // Getters for calculated values
   double              quantityRemaining() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setDateOrdered    (std::optional<QDate>          const & val);
   void setDateReceived   (std::optional<QDate>          const & val);
   void setDateBestBefore (std::optional<QDate>          const & val);
   void setSupplier       (QString                       const & val);
   void setQuantityOrdered(std::optional<double>         const   val);
   void setPurchasePrice  (std::optional<CurrencyAmount> const & val);
   void setPurchaseTax    (std::optional<CurrencyAmount> const & val);
   void setShippingCost   (std::optional<CurrencyAmount> const & val);

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
   virtual void hardDeleteOwnedEntities() override;

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

   std::optional<QDate>          m_dateOrdered    ;
   std::optional<QDate>          m_dateReceived   ;
   std::optional<QDate>          m_dateBestBefore ;
   QString                       m_supplier       ;
   std::optional<double>         m_quantityOrdered;
   std::optional<CurrencyAmount> m_purchasePrice  ;
   std::optional<CurrencyAmount> m_purchaseTax    ;
   std::optional<CurrencyAmount> m_shippingCost   ;
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


#endif
