/*======================================================================================================================
 * model/StockPurchaseBase.h is part of Brewtarget, and is copyright the following authors 2025:
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
#ifndef MODEL_STOCKPURCHASEBASE_H
#define MODEL_STOCKPURCHASEBASE_H
#pragma once

#include <memory>

#include <QList>
#include <QtNumeric>

#include "utils/CuriouslyRecurringTemplateBase.h"
#include "model/BrewNote.h"
#include "model/OwnedSet.h"
#include "model/Recipe.h"
#include "model/StockUse.h"     // For StockUse::Reason
#include "model/StockUseBase.h" // For PropertyNames::StockUseBase::quantityRemaining

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StockPurchaseBase { inline BtStringConst const property{#property}; }
AddPropertyName(amountOrdered    )
AddPropertyName(amountReceived   )
AddPropertyName(amountRemaining  )
AddPropertyName(changes          )
AddPropertyName(quantityRemaining)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

#define StockPurchaseBaseOptions OwnedSetOptions{ .enumerated = true }

/**
 * \brief Small CRTP base class to provide templated code for stock purchase classes: \c StockPurchaseHop,
 *        \c StockPurchaseFermentable, \c StockPurchaseMisc, \c StockPurchaseSalt, \c StockPurchaseYeast.
 *
 * \param Derived = the derived class, eg \c StockPurchaseHop
 * \param IngredientClass = the ingredient class, eg \c Hop
 */
template<class Derived> class StockPurchasePhantom;
template<class Derived, class IngredientClass, class StockUseClass>
class StockPurchaseBase : public CuriouslyRecurringTemplateBase<StockPurchasePhantom, Derived> {
friend StockUseClass;
protected:
   StockPurchaseBase():
      m_changes{this->derived()} {
      this->connectSlots();
      return;
   }

   StockPurchaseBase([[maybe_unused]] NamedParameterBundle const & namedParameterBundle) :
      // See comment in OwnedSet for why it never needs the NamedParameterBundle
      m_changes{this->derived()} {
      this->connectSlots();
      return;
   }

   StockPurchaseBase(StockPurchaseBase const & other) :
      m_changes{this->derived(), other.m_changes} {
      this->connectSlots();
      return;
   }

   ~StockPurchaseBase() = default;

private:
   /**
    * Called from constructors
    */
   void connectSlots() {
      //
      // Yes, we really are listening to a signal from ourselves.  It's just a way to hook into things when one of the
      // StockUse items changes (or changes position or is added or deleted) so that we can update
      // quantityRemaining on each of them.
      //
      this->derived().connect(&this->derived(), &StockPurchase::ownedItemsChanged, &this->derived(), &Derived::updateChangeItems);
      return;
   }

protected:
   /**
    * \brief Updates \c quantityRemaining on each of our steps
    *
    *        NOTE that, although it's fine to call \c Derived::quantityRemaining() we must not call
    *        \c StockUseClass::quantityRemaining() as we risk getting in an endless loop.  (Fortunately there is
    *        no need for us to call this latter function.)
    */
   void doUpdateChangeItems() {
      QList<std::shared_ptr<StockUseClass>> changes = this->m_changes.items();
      double remaining = this->quantityReceived();
      for (auto const & change : changes) {
         remaining -= change->quantityUsed();
         if (remaining < 0.0) {
            remaining = 0.0;
         }
         //
         // What we want to do here is call change->updateCachedQuantityRemaining(quantityRemaining), but we can't
         // because StockPurchaseBase can't be a friend of StockUseClass (because the former is not yet defined when
         // the latter is defined, and we can't go via another class as friendship is neither inherited nor transitive).
         //
         // So we get the derived class to do the call for us.
         //
         this->derived().updateCachedQuantityRemaining(*change, remaining);
      }
      return;
   }

public:
   static QString localisedName_amountOrdered    () { return Derived::tr("Amount Ordered"    ); }
   static QString localisedName_amountReceived   () { return Derived::tr("Amount Received"   ); }
   static QString localisedName_amountRemaining  () { return Derived::tr("Amount Remaining"  ); }
   static QString localisedName_quantityRemaining() { return Derived::tr("Quantity Remaining"); }

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

   /**
    * \brief Convenience function to set \c quantityOrdered with units.  (NOTE that it's the caller's responsibility to
    *        ensure that they are the same as for \c amountReceived.)
    */
   void setAmountOrdered(std::optional<Measurement::Amount> const & val) {
      //
      // amountOrdered is optional, but amountReceived is not, so we know the latter will always have a unit
      //
      if (!val) {
         this->derived().setQuantityOrdered(std::nullopt);
         return;
      }

      Measurement::Unit const * amountReceivedUnit = this->derived().unit();
      if (val->unit != amountReceivedUnit) {
         //
         // This might be OK if amountReceived is also about to be updated, but let's log it just in case
         //
         qWarning() <<
            Q_FUNC_INFO << "Setting" << *val->unit << "on amount ordered, but amount received is measured in" <<
            *amountReceivedUnit;
      }
      this->derived().setQuantityOrdered(val->quantity);
      return;
   }


   //! Convenience function so we don't have to remember to use IngredientAmount::amount() for amountReceived
   Measurement::Amount amountReceived() const {
      return this->derived().amount();
   }

   //! Convenience function so we don't have to remember to use IngredientAmount::quantity() for quantityReceived
   double quantityReceived() const {
      return this->derived().quantity();
   }

   //! Convenience function so we don't have to remember to use IngredientAmount::setAmount() for setAmountReceived
   void setAmountReceived(Measurement::Amount const & val) {
      this->derived().setAmount(val);
      return;
   }

   double quantityRemaining() const {
      QList<std::shared_ptr<StockUseClass>> changes = this->m_changes.items();
      if (changes.isEmpty()) {
         return this->quantityReceived();
      }
      return changes.last()->quantityRemaining();
   }

   //! \brief Convenience function to give \c quantityRemaining with units
   Measurement::Amount amountRemaining() const {
      return Measurement::Amount{this->derived().measure(), this->derived().quantityRemaining()};
   }

   OwnedSet<Derived,
            StockUseClass,
            PropertyNames::StockPurchaseBase::changes,
            nullptr,
            StockPurchaseBaseOptions> & ownedSet() {
      return this->m_changes;
   }

   /**
    * Similar to \c StepOwnerBase::steps, this is a wrapper around the \c OwnedSet interface
    */
   QList<std::shared_ptr<StockUseClass>> changes() const {
      return this->m_changes.items();
   }
   /**
    * \brief This "function alias" is used by \c EnumeratedBase.
    *
    *        True member function aliases do not currently exist in C++, and nor do references to member functions --
    *        see https://stackoverflow.com/questions/21952386/why-doesnt-reference-to-member-exist-in-c.  So this
    *        wrapper is the best we can do.
    */
   QList<std::shared_ptr<StockUseClass>> ownedItems() const { return this->changes(); }

   /**
    * \brief Intended to be called from \c Derived::acceptSetMemberChange
    *
    * \param sender - Result of caller calling \c this->sender() (which is protected, so we can't call it here)
    * \param prop - As received by Derived::acceptSetMemberChange
    * \param val  - As received by Derived::acceptSetMemberChange
    */
   void doAcceptSetMemberChange(QObject * sender, QMetaProperty prop, QVariant val) {
      StockUseClass * invChangeSender = qobject_cast<StockUseClass*>(sender);
      if (!invChangeSender) {
         return;
      }

      //
      // If there was a change in amount of an StockUse, we need to update the cached quantity remaining values.
      //
      if (invChangeSender->ownerId() == this->derived().key() &&
          prop.name() == PropertyNames::StockUseBase::quantityRemaining) {
         this->doUpdateChangeItems();
      }

      //
      // Call here to OwnedSet sends the same signal but this time from the set owner rather than the set member
      //
      this->m_changes.acceptItemChange(*invChangeSender, prop, val);
      return;
   }

   ObjectStore & doGetObjectStoreTypedInstance() const {
      return ObjectStoreTyped<Derived>::getInstance();
   }

   /**
    * \brief Adds a new stock use at the end of the current list
    */
   std::shared_ptr<StockUseClass> add(std::shared_ptr<StockUseClass> change) {
      return this->m_changes.add(change);
   }

   std::shared_ptr<StockUseClass> remove(std::shared_ptr<StockUseClass> change) {
      return this->m_changes.remove(change);
   }

   /**
    * \brief For a given ingredient, get the total amount we have on hand -- ie total of all purchases minus total of
    *        all uses.
    */
   static Measurement::Amount getTotalInventory(IngredientClass const & ingredient) {
      //
      // Get all the non-deleted non-zero StockPurchase objects for the supplied ingredient
      //
      int const ingredientKey = ingredient.key();
      auto purchases = ObjectStoreWrapper::findAllMatching<Derived>(
         [ingredientKey](Derived const * sp) {
            return !sp->deleted() &&
                   (ingredientKey == sp->ingredientId()) &&
                   !qFuzzyIsNull(sp->amountRemaining().quantity);
         }
      );

      if (purchases.size() == 0) {
         return Measurement::Amount{Derived::defaultMeasure, 0.0};
      }

      //
      // We assume every purchase is stored in the same physical quantity as the first.  If that's not true (eg some
      // are Volume and some are Mass) then Measurement::Amount::operator+ will log a warning but otherwise carry on, so
      // we'll get a result but it might not be hugely meaningful.
      //
      return std::accumulate(purchases.cbegin(),
                             purchases.cend(),
                             Measurement::Amount{purchases[0]->measure(), 0.0},
                             [](Measurement::Amount sum, Derived const * sp) { return sum + sp->amountRemaining(); });
   }

   /**
    * \brief For a given ingredient, reduce the total amount we have on hand (as a consequence of it being used in a
    *        Recipe).
    *
    *        We assume that older purchases are used before newer ones.
    *
    * \param ingredient
    * \param amountUsed
    * \param brewNote
    */
   static void reduceTotalInventory(IngredientClass const & ingredient,
                                    Measurement::Amount amountUsed,
                                    BrewNote const & brewNote) {
      //
      // Get all the non-deleted StockPurchase objects for the supplied ingredient
      //
      int const ingredientKey = ingredient.key();
      auto purchases = ObjectStoreWrapper::findAllMatching<Derived>(
         [ingredientKey](Derived const * sp) { return !sp->deleted() && (ingredientKey == sp->ingredientId()); }
      );

      //
      // Put everything in date order
      //
      std::sort(
         purchases.begin(),
         purchases.end(),
         [](Derived const * lhs, Derived const * rhs) {
            // Best before date is our preferred sort, but it's an optional field
            if (lhs->dateBestBefore() && rhs->dateBestBefore()) {
               return *lhs->dateBestBefore() < *rhs->dateBestBefore();
            }
            // Date received is a second best
            if (lhs->dateReceived() && rhs->dateReceived()) {
               return *lhs->dateReceived() < *rhs->dateReceived();
            }
            // Date ordered is a third best
            if (lhs->dateOrdered() && rhs->dateOrdered()) {
               return *lhs->dateOrdered() < *rhs->dateOrdered();
            }
            // ID is the last resort
            return lhs->key() < rhs->key();
         }
      );


      Measurement::PhysicalQuantity const physicalQuantity = amountUsed.unit->getPhysicalQuantity();

      for (Derived * purchase : purchases) {
         Measurement::Amount remaining = purchase->amountRemaining();
         if (qFuzzyIsNull(remaining.quantity)) {
            // Skip ~0.0 amounts
            continue;
         }

         if (remaining.unit->getPhysicalQuantity() != physicalQuantity) {
            // Skip over this purchase if it's not the right physical quantity (eg is Volume when we have Mass)
            qInfo() << Q_FUNC_INFO << "Skipping" << purchase << "as not" << physicalQuantity;
            continue;
         }

         auto stockUse = std::make_shared<typename Derived::StockUseClass>();
         stockUse->setDate(QDate::currentDate());
         stockUse->setBrewNoteId(brewNote.key());
         stockUse->setReason(StockUse::Reason::Used);
         stockUse->setName(
            Derived::StockUseClass::tr(
               "Use of %1 in %2"
            ).arg(purchase->ingredientRaw()->localisedName()).arg(brewNote.recipe()->name())
         );

         if (remaining >= amountUsed) {
            //
            // This is the easy case: we can satisfy what's still needed from this purchase
            //
            stockUse->setQuantityUsed(amountUsed.quantity);
            purchase->add(stockUse);
            // No further processing required
            return;
         }

         //
         // We don't have enough from this StockPurchase for what's still needed, so use what we have and hope that
         // subsequent StockPurchase items can supply the remainder.
         //
         stockUse->setQuantityUsed(remaining.quantity);
         amountUsed.quantity -= remaining.quantity;
         purchase->add(stockUse);
      }

      if (!qFuzzyIsNull(amountUsed.quantity)) {
         //
         // We didn't have enough inventory to fully satisfy the requirement.
         //
         // TBD: For the moment, we just log an error, but ideally we should ask the user whether to create another
         //      StockPurchase item.
         //
         qWarning() << Q_FUNC_INFO << "Unable to satisfy" << amountUsed << "of" << ingredient;
      }

      return;
   }
   //================================================ Member variables =================================================

   /**
    * \brief These are the uses of inventory
    */
   OwnedSet<Derived,
            StockUseClass,
            PropertyNames::StockPurchaseBase::changes,
            nullptr, // Null pointer here means default to Derived::acceptSetMemberChange
            StockPurchaseBaseOptions> m_changes;

};

template<class Derived, class IngredientClass, class StockUseClass>
TypeLookup const StockPurchaseBase<Derived, IngredientClass, StockUseClass>::typeLookup {
   "StockPurchaseBase",
   {
      //
      // We can't use the PROPERTY_TYPE_LOOKUP_ENTRY or PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here, because macros
      // don't know how to handle templated references such as `StockPurchaseBase<Derived, IngredientClass>::m_quantity`
      // and try to treat them as two macro parameters).  So we have to put the raw code instead.
      //
      // (We can't get around this by writing `using IngredientAmountType = StockPurchaseBase<Derived, IngredientClass>`
      // because Derived and IngredientClass aren't defined until the template is instantiated.)
      //
      // This does show the advantage of being able to use the macros elsewhere! :)
      //
      {&PropertyNames::StockPurchaseBase::amountOrdered,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockPurchaseBase::amountOrdered>>(
          PropertyNames::StockPurchaseBase::amountOrdered,
          StockPurchaseBase::localisedName_amountOrdered,
          TypeLookupOf<MemberFunctionReturnType_t<&StockPurchaseBase::amountOrdered>>::value,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::StockPurchaseBase::amountReceived,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockPurchaseBase::amountReceived>>(
          PropertyNames::StockPurchaseBase::amountReceived,
          StockPurchaseBase::localisedName_amountReceived,
          TypeLookupOf<MemberFunctionReturnType_t<&StockPurchaseBase::amountReceived>>::value,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::StockPurchaseBase::amountRemaining,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockPurchaseBase::amountRemaining>>(
          PropertyNames::StockPurchaseBase::amountRemaining,
          StockPurchaseBase::localisedName_amountRemaining,
          TypeLookupOf<MemberFunctionReturnType_t<&StockPurchaseBase::amountRemaining>>::value,
          TypeInfo::Access::ReadOnly,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
      {&PropertyNames::StockPurchaseBase::quantityRemaining,
       TypeInfo::construct<MemberFunctionReturnType_t<&StockPurchaseBase::quantityRemaining>>(
          PropertyNames::StockPurchaseBase::quantityRemaining,
          StockPurchaseBase::localisedName_quantityRemaining,
          TypeLookupOf<MemberFunctionReturnType_t<&StockPurchaseBase::quantityRemaining>>::value,
          TypeInfo::Access::ReadOnly,
          IngredientClass::validMeasures,
          DisplayInfo::Precision{1}
       )},
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Subclasses of \c StockPurchase should include this in their header file, right after Q_OBJECT
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STOCK_PURCHASE_DECL(IngredientName, LcIngredientName) \
friend class StockPurchaseBase<StockPurchase##IngredientName, \
                               IngredientName,                \
                               StockUse##IngredientName>;     \
                                                              \
public:                                                       \
   /** \brief See comment in model/NamedEntity.h */                                  \
   static QString localisedName();                                                   \
   static QString localisedName_##LcIngredientName();                                \
                                                                                     \
   static QString instanceNameTemplate();                                            \
                                                                                     \
   /** \brief See \c NamedEntity::typeLookup. */                                     \
   static TypeLookup const typeLookup;                                               \
   TYPE_LOOKUP_GETTER                                                                \
                                                                                     \
   /* Valid measures must be the same as for the underlying ingredient */            \
   static constexpr auto  validMeasures = IngredientName:: validMeasures;            \
   static constexpr auto defaultMeasure = IngredientName::defaultMeasure;            \
                                                                                     \
   using IngredientClass = IngredientName;                                           \
   using EditorClass     = StockPurchase##IngredientName##Editor;                    \
   using StockUseClass   = StockUse##IngredientName;                                 \
                                                                                     \
   StockPurchase##IngredientName(QString const & name = "");                         \
   StockPurchase##IngredientName(NamedParameterBundle const & namedParameterBundle); \
   StockPurchase##IngredientName(StockPurchase##IngredientName const & other);       \
                                                                                     \
   virtual ~StockPurchase##IngredientName();                                         \
                                                                                     \
   IngredientName * LcIngredientName() const ;                                       \
   void set##IngredientName(IngredientName * val);                                   \
   /* Called from \c WindowDistributor::editorForNewStockPurchase */                 \
   void setIngredient(IngredientName const & val);                                   \
                                                                                     \
   /** \brief We need this for \c ObjectStoreTyped to call */                        \
   virtual void hardDeleteOwnedEntities() override;                                  \
                                                                                     \
public slots:                                                                        \
   void acceptSetMemberChange(QMetaProperty prop, QVariant val);                     \
   void updateChangeItems();                                                         \
                                                                                     \
protected:                                                                           \
   virtual bool compareWith(NamedEntity const & other,                               \
                            QList<BtStringConst const *> * propertiesThatDiffer) const override; \
   virtual ObjectStore & getObjectStoreTypedInstance() const override;             \
                                                                                   \
private:                                                                           \
   void updateCachedQuantityRemaining(StockUse##IngredientName & change,           \
                                      double const quantityRemaining) const;       \

/**
 * \brief Subclasses of \c StockPurchaseBase should include this in their implementation file.
 *
 *        Note that #IngredientName will expand to "Fermentable"/"Hop"/etc and that
 *           "Fermentable" " StockPurchase"
 *        is treated by the compiler exactly the same as
 *           "Fermentable StockPurchase"
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STOCK_PURCHASE_COMMON_CODE(IngredientName, LcIngredientName)               \
QString StockPurchase##IngredientName::localisedName() {                           \
   return tr(#IngredientName " Stock Purchase");                                   \
}                                                                                  \
QString StockPurchase##IngredientName::localisedName_##LcIngredientName() {        \
   return IngredientName::localisedName();                                         \
}                                                                                  \
                                                                                   \
QString StockPurchase##IngredientName::instanceNameTemplate() {                    \
   /* An acquisition is not necessarily a purchase, but purchase seems */          \
   /* a more natural word to use in this context.                      */          \
   return tr("Purchase of %1 " #LcIngredientName);                                 \
}                                                                                  \
                                                                                   \
IngredientName * StockPurchase##IngredientName::LcIngredientName() const {         \
   return this->ingredientRaw(); /* See IngredientAmount */                        \
}                                                                                  \
                                                                                   \
void StockPurchase##IngredientName::set##IngredientName(IngredientName * val) {    \
   this->setIngredientRaw(val); /* See IngredientAmount */                         \
   return;                                                                         \
}                                                                                  \
                                                                                   \
void StockPurchase##IngredientName::hardDeleteOwnedEntities() {                    \
   this->m_changes.doHardDeleteOwnedEntities();                                    \
   return;                                                                         \
}                                                                                  \
                                                                                   \
void StockPurchase##IngredientName::acceptSetMemberChange(QMetaProperty prop,      \
                                                          QVariant val) {          \
   this->doAcceptSetMemberChange(this->sender(), prop, val);                       \
}                                                                                  \
                                                                                   \
void StockPurchase##IngredientName::updateChangeItems() {                          \
   this->doUpdateChangeItems();                                                    \
   return;                                                                         \
}                                                                                  \
                                                                                   \
ObjectStore & StockPurchase##IngredientName::getObjectStoreTypedInstance() const { \
   return this->doGetObjectStoreTypedInstance();                                   \
}                                                                                  \
                                                                                   \
void StockPurchase##IngredientName::updateCachedQuantityRemaining(                 \
   StockUse##IngredientName & change,                                              \
   double const quantity                                                           \
) const {                                                                          \
   change.updateCachedQuantityRemaining(quantity);                                 \
   return;                                                                         \
}                                                                                  \


#endif
