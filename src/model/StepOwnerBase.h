/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepOwnerBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_STEPOWNERBASE_H
#define MODEL_STEPOWNERBASE_H
#pragma once

#include <algorithm>
#include <memory>
#include <optional>

#include <QDebug>
#include <QList>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "model/OwnedSet.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StepOwnerBase { inline BtStringConst const property{#property}; }
AddPropertyName(numSteps)
AddPropertyName(steps   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief This is a nothing class that exists to allow us to determine whether something inherits from the
 *        \c StepOwnerBase CRTP class.
 */
class StepOwner{};

#define StepOwnerBaseOptions OwnedSetOptions{ .enumerated = true }
/**
 * \brief Templated base class for \c Mash, \c Boil and \c Fermentation to handle manipulation of their component steps
 *        (\c MashStep, \c BoilStep and \c FermentationStep respectively).
 *
 *        In BeerJSON, the step owner types have overlapping sets of fields, which correspond to our properties as
 *        follows (where ‡ means a field is required and * means Mash/Boil/Fermentation as appropriate):
 *
 *           MashProcedureType     BoilProcedureType     FermentationProcedureType  |  Property
 *           -----------------     -----------------     -------------------------  |  --------
 *         ‡ name                  name                ‡ name                       |   NamedEntity::name
 *           notes                 notes                 notes                      |             *::notes
 *                                 description           description                |             *::description
 *         ‡ grain_temperature                                                      |          Mash::grainTemp_c
 *                                 pre_boil_size                                    |          Boil::preBoilSize_l
 *                               ‡ boil_time                                        |          Boil::boilTime_mins
 *         ‡ mash_steps                                                             |          Mash::mashSteps
 *                                 boil_steps                                       |          Boil::boilSteps
 *                                                     ‡ fermentation_steps         |  Fermentation::fermentationSteps
 *
 *        We don't do this as a subclass of \c NamedEntity, because the only common property in \c Mash, \c Boil and
 *        \c Fermentation (apart from the ones they get from \c NamedEntity is the \c notes field.  What we want is
 *        something that will handle all the logic of step addition, removal, reordering etc, and ideally do so in a
 *        strongly-typed way (eg so that it is never possible to add a \c BoilStep to a \c Mash!)  Hence this templated
 *        class.
 *
 *        As noted elsewhere in the code base, the Qt meta object compiler (moc) can't handle templated classes, but it
 *        can ignore them.  So we need to use multiple inheritance to use a templated base class, and to ensure that the
 *        templated base class is not the first thing inherited from.  (In fact, the requirement is stronger: the base
 *        class inheriting from \c QObject must be the first in the inheritance list.)
 *
 *        We also use the Curiously Recurring Template Pattern (CRTP) to allow this base class to easily access members
 *        of the derived class.
 *
 *        We assume/require that \c DerivedStep inherits from \c Step.
 *
 *        TBD: For the moment, we have \c OwnedSet as a member, with wrapper functions.  We could, instead, inherit from
 *             \c OwnedSet, and use its interface directly, at the cost of a bit of renaming.
 */
template<class Derived> class StepOwnerPhantom;
template<class Derived, class DerivedStep>
class StepOwnerBase : public CuriouslyRecurringTemplateBase<StepOwnerPhantom, Derived>, public StepOwner {
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   // This allows Derived to call our protected and private members
   friend Derived;

   static QString localisedName_numSteps() { return Derived::tr("№ Steps"); }
   static QString localisedName_steps   () { return Derived::tr("Steps"  ); }

private:
   //! Non-virtual equivalent of compareWith
   bool doCompareWith(StepOwnerBase const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
      return this->m_stepSet.doCompareWith(other.m_stepSet, propertiesThatDiffer);
   }

   StepOwnerBase() :
      m_stepSet{this->derived()} {
      return;
   }

   StepOwnerBase([[maybe_unused]] NamedParameterBundle const & namedParameterBundle) :
      // See comment in OwnedSet for why it never needs the NamedParameterBundle
      m_stepSet{this->derived()} {
      return;
   }

   StepOwnerBase(StepOwnerBase const & other) :
      m_stepSet{this->derived(), other.m_stepSet} {
      return;
   }

   /**
    * \brief We don't want copy assignment happening.
    */
   StepOwnerBase & operator=(StepOwnerBase const & other) = delete;

   ~StepOwnerBase() = default;

public:
   OwnedSet<Derived,
            DerivedStep,
            PropertyNames::StepOwnerBase::steps,
            nullptr,
            StepOwnerBaseOptions> & ownedSet() {
      return this->m_stepSet;
   }


   // TBD: This public block of member functions is just a wrapper around the OwnedSet interface.  We could get rid of
   // it if we inherited from OwnedSet.
   QList<std::shared_ptr<DerivedStep>> steps() const {
      return this->m_stepSet.items();
   }
   /**
    * \brief This "function alias" is used by \c EnumeratedBase and \c TreeModelBase.
    *
    *        True member function aliases do not currently exist in C++, and nor do references to member functions --
    *        see https://stackoverflow.com/questions/21952386/why-doesnt-reference-to-member-exist-in-c.  So this
    *        wrapper is the best we can do.
    */
   QList<std::shared_ptr<DerivedStep>> ownedItems() const { return this->steps(); }

   /**
    * \brief Returns the step at the specified position, if it exists, or \c nullptr if not
    *
    * \param seqNum counted from 1
    */
   std::shared_ptr<DerivedStep> stepAt(int const seqNum) const {
      return this->m_stepSet.itemAt(seqNum);
   }

   /**
    * \brief Inserts a new step at the specified position.  If there is already a step in that position, it (and all
    *        subsequent ones) will be bumped one place down the list.
    *
    * \param step
    * \param seqNum counted from 1
    */
   std::shared_ptr<DerivedStep> insert(std::shared_ptr<DerivedStep> step, int const seqNum) {
      return this->m_stepSet.insert(step, seqNum);
   }

   /**
    * \brief Adds a new step at the end of the current list
    */
   std::shared_ptr<DerivedStep> add(std::shared_ptr<DerivedStep> step) {
      return this->m_stepSet.add(step);
   }

   std::shared_ptr<DerivedStep> remove(std::shared_ptr<DerivedStep> step) {
      return this->m_stepSet.remove(step);
   }

   /**
    * \brief Sets (or unsets) the step at the specified position.
    *
    *        Note this is different from insertStep(), as:
    *          - If there is a step in the specified position it will be overwritten rather than bumped down the list
    *          - Calling this with non-null value (ie not std::nullopt) for second and later steps will ensure prior
    *            step(s) exist by creating default ones if necessary.
    *          - Calling this with null value (ie std::nullopt) delete any subsequent steps.  (Doesn't make sense for
    *            third step to become second in the context of this function.)
    *
    * \param step The step to set, or \c nullptr to unset it
    * \param seqNum
    */
   void setStepAt(std::shared_ptr<DerivedStep> step, int const seqNum) {
      this->m_stepSet.setAt(step, seqNum);
      return;
   }

   void setSteps(QList<std::shared_ptr<DerivedStep>> const & steps) {
      this->m_stepSet.setAll(steps);
      return;
   }

   unsigned int numSteps() const {
      return this->m_stepSet.size();
   }

   /*!
    * \brief Swap Steps \c step1 and \c step2
    */
   void swapOrder(DerivedStep & lhs, DerivedStep & rhs) {
      this->m_stepSet.swap(lhs, rhs);
      return;
   }

   void removeAllSteps() {
      this->m_stepSet.removeAll();
      return;
   }

private:
   void doSetKey(int key) {
      // First call the base class function
      this->derived().NamedEntity::setKey(key);
      // Now tell the OwnedSet about the new key
      this->m_stepSet.doSetKey(key);
      return;
   }

   /**
    * \brief Called from \c ObjectStoreTyped::postLoadInit
    */
   void connectSignals() {
      this->m_stepSet.connectAllItemChangedSignals();
      return;
   }

   /**
    * \brief Intended to be called from \c Derived::acceptSetMemberChange
    *
    * \param sender - Result of caller calling \c this->sender() (which is protected, so we can't call it here)
    * \param prop - As received by Derived::acceptSetMemberChange
    * \param val  - As received by Derived::acceptSetMemberChange
    */
   void doAcceptSetMemberChange(QObject * sender,
                           QMetaProperty prop,
                           QVariant      val) {
      DerivedStep * stepSender = qobject_cast<DerivedStep*>(sender);
      if (!stepSender) {
         return;
      }

      this->m_stepSet.acceptItemChange(*stepSender, prop, val);
      return;
   }

   //================================================ Member variables =================================================
   OwnedSet<Derived,
            DerivedStep,
            PropertyNames::StepOwnerBase::steps,
            nullptr,
            StepOwnerBaseOptions> m_stepSet;
};

template<class Derived, class DerivedStep>
TypeLookup const StepOwnerBase<Derived, DerivedStep>::typeLookup {
   "StepOwnerBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::StepOwnerBase::numSteps,
       TypeInfo::construct<MemberFunctionReturnType_t<&StepOwnerBase::numSteps>>(
          PropertyNames::StepOwnerBase::numSteps,
          StepOwnerBase::localisedName_numSteps,
          TypeLookupOf<MemberFunctionReturnType_t<&StepOwnerBase::numSteps>>::value,
          NonPhysicalQuantity::OrdinalNumeral
       )},
   },
   // Parent class lookup: Although we are at the top of this branch of the inheritance tree, OwnedSet has a similar
   // role to a base class for us.
   {std::addressof(OwnedSet<Derived,
                   DerivedStep,
                   PropertyNames::StepOwnerBase::steps,
                   nullptr,
                   StepOwnerBaseOptions>::typeLookup)}
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STEP_OWNER_COMMON_DECL(NeName, LcNeName) \
   /* This allows StepOwnerBase to call protected and private members of Derived */           \
   friend class StepOwnerBase<NeName,                                                         \
                              NeName##Step>;                                                  \
                                                                                              \
   public:                                                                                    \
      /* This alias makes it easier to template a number of functions that are */             \
      /* essentially the same for all "Step Owner" classes.                    */             \
      using StepClass = NeName##Step;                                                         \
                                                                                              \
      /* Relational getters and setters */                                                    \
      QList<std::shared_ptr<NeName##Step>> LcNeName##Steps        () const;                   \
      void set##NeName##Steps        (QList<std::shared_ptr<NeName##Step>> const & val);      \
                                                                                              \
      /* Return number of steps matching the supplied lambda */                               \
      int num##NeName##StepsMatching(std::function<bool(NeName##Step const &)> const & matchFunction) const; \
                                                                                              \
      virtual void setKey(int key) override;                                                  \
                                                                                              \
      /** \brief NeName owns its NeName##Steps so needs to delete them if it */               \
      /*         itself is being deleted                                     */               \
      virtual void hardDeleteOwnedEntities() override;                                        \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_OWNER_COMMON_CODE(NeName, LcNeName) \
   QList<std::shared_ptr<NeName##Step>> NeName::LcNeName##Steps() const {                           \
      return this->m_stepSet.items();                                                               \
   }                                                                                                \
   void NeName::set##NeName##Steps(QList<std::shared_ptr<NeName##Step>> const & val) {              \
      this->m_stepSet.setAll(val); return;                                                          \
   }                                                                                                \
                                                                                                    \
   int NeName::num##NeName##StepsMatching(std::function<bool(NeName##Step const &)> const & matchFunction) const { \
      return this->m_stepSet.numMatching(matchFunction);                                            \
   }                                                                                                \
                                                                                                    \
   void NeName::setKey(int key) { this->doSetKey(key); return; }                                    \
                                                                                                    \
   void NeName::hardDeleteOwnedEntities() { this->m_stepSet.doHardDeleteOwnedEntities(); return; }  \

#endif
