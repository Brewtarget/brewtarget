/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/EnumeratedBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_ENUMERATEDBASE_H
#define MODEL_ENUMERATEDBASE_H
#pragma once

#include <type_traits>

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::EnumeratedBase { inline BtStringConst const property{#property}; }
AddPropertyName(ownerId       )
AddPropertyName(sequenceNumber)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief This CRTP base class provides "step-like" behaviour to classes that inherit from it.
 *
 *        For various reasons, including because it's the term used in BeerXML and BeerJSON, when we talk about a
 *        "step", we usually mean a \c MashStep, \c BoilStep or \c FermentationStep.  We use \c Step and \c StepBase to
 *        model the common behaviours and attributes of these three concrete classes.  However, \c Instruction also has
 *        some step-like behaviours and attributes that it shares with \c Step subclasses:
 *          - Each \c Instruction has an owner (a \c Recipe)
 *          - An \c Instruction has no independent existence of its owner (deleting a \c Recipe implies deleting all the
 *            corresponding \c Instruction objects).
 *          - The \c Instruction objects belonging to a given \c Recipe have an ordering
 *
 *        So we pull out these more fundamental properties into \c EnumeratedBase, as the following \b partial
 *        inheritance diagram shows:
 *
 *                          EnumeratedBase
 *                          /         \
 *                    StepBase       Instruction
 *                    /   |   \
 *                   /    |    \
 *                  /     |     \
 *          MashStep  BoilStep  FermentationStep
 *
 *        This gives enough of what we need, for now.  We would need to rethink a bit if we had one type of object
 *        owning more than one type of "steps".
 *
 *        Directly derived classes need to include ENUMERATED_COMMON_DECL and ENUMERATED_COMMON_CODE in the obvious
 *        places (either their own macros for \c StepBase or in the header and implementation file respectively for
 *        \c Instruction).
 */
template<class Derived> class EnumeratedPhantom;
template<class Derived, class Owner>
class EnumeratedBase : public CuriouslyRecurringTemplateBase<EnumeratedPhantom, Derived> {
protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! Non-virtual equivalent of compareWith
   bool doCompareWith(EnumeratedBase const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
      return (
         //
         // Note that we do _not_ compare m_ownerId.  We need to be able to compare classes with different owners.  Eg,
         // to know whether two different Mash objects are equal, we need, amongst other things, to check whether their
         // owned MashStep objects are equal.
         //
         AUTO_PROPERTY_COMPARE(this, other, m_sequenceNumber, PropertyNames::EnumeratedBase::sequenceNumber, propertiesThatDiffer)
      );
   }

   bool doIsLessThan(EnumeratedBase const & other) const {
      // It is a coding error if we are trying to order items that don't have the same owner
      Q_ASSERT(this->m_ownerId == other.m_ownerId);
      return this->m_sequenceNumber < other.m_sequenceNumber;
   }

   // Normally we'd make the constructors private and allow access to Derived as a friend.  However, we want StepBase
   // to inherit from EnumeratedBase, so we need the former's constructor to be able to call the latter's.
   EnumeratedBase() {
      return;
   }

   EnumeratedBase(NamedParameterBundle const & namedParameterBundle) :
      SET_REGULAR_FROM_NPB (m_ownerId       , namedParameterBundle, PropertyNames::EnumeratedBase::ownerId       , -1),
      SET_REGULAR_FROM_NPB (m_sequenceNumber, namedParameterBundle, PropertyNames::EnumeratedBase::sequenceNumber, -1) {
      return;
   }

   explicit EnumeratedBase(Derived const & other) :
   m_ownerId       {other.m_ownerId       },
   m_sequenceNumber{other.m_sequenceNumber} {
      return;
   }

   ~EnumeratedBase() = default;

public:
   static QString localisedName_ownerId       () { return Derived::tr("Owner ID"); }
   static QString localisedName_sequenceNumber() { return Derived::tr("Number"  ); }

   int ownerId   () const { return this->m_ownerId   ; }

   int sequenceNumber() const { return this->m_sequenceNumber; }

   void setOwnerId   (int const val) {
      this->m_ownerId = val;
      this->derived().propagatePropertyChange(PropertyNames::EnumeratedBase::ownerId, false);
      return;
   }

   /**
    * \brief Does what it says on the tin
    *
    * \param notify Needs to be set to \c false when part-way through swapping two steps, because we don't want
    *               observers to re-read all the steps until we've finished.
    */
   void setSequenceNumber(int const val, bool const notify = true) {
      if (notify) {
         this->derived().setAndNotify(PropertyNames::EnumeratedBase::sequenceNumber, this->m_sequenceNumber, val);
      } else {
         this->m_sequenceNumber = val;
         this->derived().propagatePropertyChange(PropertyNames::EnumeratedBase::sequenceNumber, false);
      }
      return;
   }

   /**
    * \brief This is, amongst other things, needed by \c TreeModelBase
    */
   std::shared_ptr<Owner> owner() const {
      auto const ownerId {this->derived().m_ownerId};
      if (ObjectStoreWrapper::contains<Owner>(ownerId)) {
         return ObjectStoreWrapper::getById<Owner>(ownerId);
      }
      return nullptr;
   }

   /**
    * \brief This is similarly needed by \c TreeModelBase
    */
   static QList<std::shared_ptr<Derived>> ownedBy(Owner const & owner) {
      // We already wrote all the logic in StepOwnerBase and StockPurchaseBase.
      return owner.ownedItems();
   }

   ObjectStore & doGetObjectStoreTypedInstance() const {
      return ObjectStoreTyped<Derived>::getInstance();
   }

   //! \brief Convenience function for logging
   virtual QString toString() const {
      return QString{"EnumeratedBase (m_ownerId: %1; m_sequenceNumber: %2)"}.arg(this->m_ownerId).arg(this->m_sequenceNumber);
   }

protected:
   //================================================ MEMBER VARIABLES =================================================
   int m_ownerId    = -1;
   // For historical reasons, step (aka sequence) numbers always start from 1, so 0 would also be invalid, but -1 is
   // more obvious
   int m_sequenceNumber = -1;
};

//
// Note that, where child classes also inherit from, eg, NamedEntity, we need to provide a more specific overload of
// operator<< specifically for the child class (otherwise the compiler won't know whether to use this one or the
// NamedEntity one).  See model/Instruction.h for an example.
//
template<class S, class Derived, class Owner>
S & operator<<(S & stream, EnumeratedBase<Derived, Owner> const & enumeratedBase) {
   stream << enumeratedBase.toString();
   return stream;
}

template<class Derived, class Owner>
TypeLookup const EnumeratedBase<Derived, Owner>::typeLookup {
   "EnumeratedBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::EnumeratedBase::ownerId,
       TypeInfo::construct<decltype(EnumeratedBase<Derived, Owner>::m_ownerId)>(
          PropertyNames::EnumeratedBase::ownerId,
          EnumeratedBase::localisedName_ownerId,
          TypeLookupOf<decltype(EnumeratedBase<Derived, Owner>::m_ownerId)>::value
       )},
      {&PropertyNames::EnumeratedBase::sequenceNumber,
       TypeInfo::construct<decltype(EnumeratedBase<Derived, Owner>::m_sequenceNumber)>(
          PropertyNames::EnumeratedBase::sequenceNumber,
          EnumeratedBase::localisedName_sequenceNumber,
          TypeLookupOf<decltype(EnumeratedBase<Derived, Owner>::m_sequenceNumber)>::value,
          NonPhysicalQuantity::OrdinalNumeral
       )}
   },
   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Concrete derived classes should (either directly or via inclusion in an intermediate class's equivalent macro)
 *        include this in their header file, right after Q_OBJECT.  Concrete derived classes also need to include the
 *        following block (see comment in model/StepBase.h for why):
 *
 *           // See model/EnumeratedBase.h for info, getters and setters for these properties
 *           Q_PROPERTY(int ownerId          READ ownerId          WRITE setOwnerId       )
 *           Q_PROPERTY(int sequenceNumber   READ sequenceNumber   WRITE setSequenceNumber)
 *
 *        Comments for these properties:
 *
 *           \c ownerId : ID of the owning object - eg \c Mash, \c Boil, \c Fermentation, \c Recipe
 *
 *           \c sequenceNumber : The number (starting from 1) in a sequence of, eg, steps.
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define ENUMERATED_COMMON_DECL(Derived, Owner) \
   /* This allows StepBase to call protected and private members of Derived */                \
   friend class EnumeratedBase<Derived, Owner>;                                               \
   public:                                                                                    \
      /* This alias makes it easier to template a number of functions */                      \
      /* that are essentially the same for all "stepped" classes.     */                      \
      using EnumeratedItemOwnerClass = Owner;                                                           \
      bool operator<(Derived const & other) const;                                            \
                                                                                              \
   protected:                                                                                 \
      /** Override NamedEntity::getObjectStoreTypedInstance */                                \
      virtual ObjectStore & getObjectStoreTypedInstance() const override;                     \

/**
 * \brief Derived classes should (either directly or via inclusion in an intermediate class's equivalent macro) include
 *        this in their implementation file.
 */
#define ENUMERATED_COMMON_CODE(Derived) \
   bool Derived::operator<(Derived const & other) const { return this->doIsLessThan(other); }                   \
   ObjectStore & Derived::getObjectStoreTypedInstance() const { return this->doGetObjectStoreTypedInstance(); } \


#endif
