/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/SteppedBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_STEPPEDBASE_H
#define MODEL_STEPPEDBASE_H
#pragma once

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::SteppedBase { BtStringConst const property{#property}; }
AddPropertyName(ownerId   )
AddPropertyName(stepNumber)
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
 *        So we pull out these more fundamental properties into \c SteppedBase and \c SteppedOwnerBase, as the following
 *        \b partial inheritance diagram shows:
 *
 *                          SteppedBase                                      SteppedOwnerBase
 *                          /         \                                       /           \
 *                    StepBase       Instruction                       StepOwnerBase     Recipe
 *                    /   |   \                                          /   |   \
 *                   /    |    \                                        /    |    \
 *                  /     |     \                                      /     |     \
 *          MashStep  BoilStep  FermentationStep                    Mash   Boil    Fermentation
 *
 *        This gives enough of what we need, for now.  We would need to rethink a bit if we had one type of object
 *        owning more than one type of "steps".
 *
 *        NOTE that the "stepped" concept does not apply to other things "owned" by Recipe, such as \c RecipeAdditionHop
 *        or \c BrewNote, because they do not have the same need of pure ordering.  There is an implicit ordering by
 *        addition time or date, but it is not necessary or desirable to be more precise than this.  You will often have
 *        two hop additions that happen at the same time, for instance.  See \c OwnedSet for the logic for dealing with
 *        these.
 *
 *        Directly derived classes need to include STEPPED_COMMON_DECL and STEPPED_COMMON_CODE in the obvious places
 *        (either their own macros for \c StepBase or in the header and implementation file respectively for
 *        \c Instruction).
 */
template<class Derived> class SteppedPhantom;
template<class Derived, class Owner>
class SteppedBase : public CuriouslyRecurringTemplateBase<SteppedPhantom, Derived> {
protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! Non-virtual equivalent of isEqualTo
   bool doIsEqualTo(SteppedBase const & other) const {
      return (
         Utils::AutoCompare(this->m_ownerId   , other.m_ownerId   ) &&
         Utils::AutoCompare(this->m_stepNumber, other.m_stepNumber)
      );
   }

   // Normally we'd make the constructors private and allow access to Derived as a friend.  However, we want StepBase
   // to inherit from SteppedBase, so we need the former's constructor to be able to call the latter's.
   SteppedBase() {
      return;
   }

   SteppedBase(NamedParameterBundle const & namedParameterBundle) :
      SET_REGULAR_FROM_NPB (m_ownerId   , namedParameterBundle, PropertyNames::SteppedBase::ownerId   , -1),
      SET_REGULAR_FROM_NPB (m_stepNumber, namedParameterBundle, PropertyNames::SteppedBase::stepNumber, -1) {
      return;
   }

   SteppedBase(Derived const & other) :
   m_ownerId   {other.m_ownerId   },
   m_stepNumber{other.m_stepNumber} {
      return;
   }

   ~SteppedBase() = default;

public:
   int ownerId   () const { return this->m_ownerId   ; }
   int stepNumber() const { return this->m_stepNumber; }

   void setOwnerId   (int const val) {
      this->m_ownerId = val;
      this->derived().propagatePropertyChange(PropertyNames::SteppedBase::ownerId, false);
      return;
   }

   /**
    * \brief Does what it says on the tin
    *
    * \param notify Needs to be set to \c false when part-way through swapping two steps, because we don't want
    *               observers to re-read all the steps until we've finished.
    */
   void setStepNumber(int const val, bool const notify = true) {
      if (notify) {
         this->derived().setAndNotify(PropertyNames::SteppedBase::stepNumber, this->m_stepNumber, val);
      } else {
         this->m_stepNumber = val;
         this->derived().propagatePropertyChange(PropertyNames::SteppedBase::stepNumber, false);
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
   static QList<std::shared_ptr<Derived>> ownedBy(std::shared_ptr<Owner> owner) {
      // We already wrote all the logic in SteppedOwnerBase.
      return owner->steps();
   }

   ObjectStore & doGetObjectStoreTypedInstance() const {
      return ObjectStoreTyped<Derived>::getInstance();
   }



protected:
   //================================================ MEMBER VARIABLES =================================================
   int m_ownerId    = -1;
   // For historical reasons, step numbers always start from 1, so 0 would also be invalid, but -1 is more obvious
   int m_stepNumber = -1;
};

template<class Derived, class Owner>
TypeLookup const SteppedBase<Derived, Owner>::typeLookup {
   "SteppedBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::SteppedBase::ownerId,
       TypeInfo::construct<decltype(SteppedBase<Derived, Owner>::m_ownerId)>(
          PropertyNames::SteppedBase::ownerId,
          TypeLookupOf<decltype(SteppedBase<Derived, Owner>::m_ownerId)>::value
       )},
      {&PropertyNames::SteppedBase::stepNumber,
       TypeInfo::construct<decltype(SteppedBase<Derived, Owner>::m_stepNumber)>(
          PropertyNames::SteppedBase::stepNumber,
          TypeLookupOf<decltype(SteppedBase<Derived, Owner>::m_stepNumber)>::value,
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
 *           // See model/SteppedBase.h for info, getters and setters for these properties
 *           Q_PROPERTY(int ownerId      READ ownerId      WRITE setOwnerId   )
 *           Q_PROPERTY(int stepNumber   READ stepNumber   WRITE setStepNumber)
 *
 *        Comments for these properties:
 *
 *           \c ownerId : ID of the owning object - eg \c Mash, \c Boil, \c Fermentation, \c Recipe
 *
 *           \c stepNumber : The step number (starting from 1) in a sequence of other steps.
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions.
 *
 *        In older versions of Qt, we didn't used to be able to put Q_PROPERTY inside our own macro, but, thankfully,
 *        the Qt MOC (meta object compiler) now expands "normal" macros before processing its own "special" ones.
 */
#define STEPPED_COMMON_DECL(Derived, Owner) \
   /* This allows StepBase to call protected and private members of Derived */                \
   friend class SteppedBase<Derived, Owner>;                                                  \
   public:                                                                                    \
      /* This alias makes it easier to template a number of functions */                      \
      /* that are essentially the same for all "stepped" classes.     */                      \
      using StepOwnerClass = Owner;                                                           \
                                                                                              \
   protected:                                                                                 \
      /** Override NamedEntity::getObjectStoreTypedInstance */                                \
      virtual ObjectStore & getObjectStoreTypedInstance() const override;                     \

/**
 * \brief Derived classes should (either directly or via inclusion in an intermediate class's equivalent macro) include
 *        this in their implementation file.
 */
#define STEPPED_COMMON_CODE(Derived) \
   ObjectStore & Derived::getObjectStoreTypedInstance() const { return this->doGetObjectStoreTypedInstance(); } \


#endif
