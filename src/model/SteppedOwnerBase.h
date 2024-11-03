/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/SteppedOwnerBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_STEPPEDOWNERBASE_H
#define MODEL_STEPPEDOWNERBASE_H
#pragma once

#include <algorithm>

#include <QList>

#include "model/NamedParameterBundle.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::SteppedOwnerBase { BtStringConst const property{#property}; }
AddPropertyName(numSteps)
AddPropertyName(steps   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief See comment in \c model/SteppedBase.h for explanation of how this relates to \c StepOwnerBase and \c Recipe
 *        etc.
 *
 *        Concrete classes deriving from this one need to declare a `void stepsChanged()` Qt signal in their header.
 *
 *        TODO: We could probably find a way to share more code between this class and \c OwnedSet.  Plus, I also now
 *              think we'd be better using \c OwnedSet for Recipe's ownership of \c Instructions, so we can merge this
 *              back into \c StepOwnerBase etc.
 */
template<class Derived> class SteppedOwnerPhantom;
template<class Derived, class DerivedStep>
class SteppedOwnerBase : public CuriouslyRecurringTemplateBase<SteppedOwnerPhantom, Derived> {
protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! Non-virtual equivalent of isEqualTo
   bool doIsEqualTo(SteppedOwnerBase const & other) const {
      //
      // Check each object has the same number of steps and they're all the same
      //
      auto const mySteps = this->steps();
      auto const otherSteps = other.steps();
      return std::equal(   mySteps.begin(),    mySteps.end(),
                        otherSteps.begin(), otherSteps.end(),
                        [](std::shared_ptr<DerivedStep> const & lhs,
                           std::shared_ptr<DerivedStep> const & rhs) {return *lhs == *rhs;});
   }

   // Normally we'd make the constructors private and allow access to Derived as a friend.  However, we want StepBase
   // to inherit from SteppedBase, so we need the former's constructor to be able to call the latter's.
   SteppedOwnerBase() :
      m_stepIds{} {
      return;
   }

   SteppedOwnerBase([[maybe_unused]] NamedParameterBundle const & namedParameterBundle) {
      return;
   }

   SteppedOwnerBase(Derived const & other) {
      // Deep copy of Steps
      for (auto step : other.steps()) {
         // Make a copy of the current DerivedStep object we're looking at in the other Mash
         auto stepToAdd = std::make_shared<DerivedStep>(*step);

         // This is where things get a bit tricky.
         // We don't have an ID yet, so we can't give it to the new DerivedStep
         stepToAdd->setOwnerId(-1);

         // However, if we insert the new DerivedStep in the object store, that will give it its own ID
         ObjectStoreWrapper::insert(stepToAdd);

         // Store the ID of the copy DerivedStep
         // If and when we get our ID then we can give it to our MashSteps
         // .:TBD:. It would be nice to find a more automated way of doing this
         this->m_stepIds.append(stepToAdd->key());

         // Connect signals so that we are notified when there are changes to the DerivedStep we just added to our
         // StepOwner.
         this->derived().connect(stepToAdd.get(), &NamedEntity::changed, &this->derived(), &Derived::acceptStepChange);
      }
      return;
   }

   /**
    * \brief We have to delete the default copy constructor because we want the constructor above (that takes \c Derived
    *        rather than \c SteppedOwnerBase) to be used instead of a compiler-generated copy constructor which wouldn't
    *        do the deep copy we need.
    */
   SteppedOwnerBase(SteppedOwnerBase const & other) = delete;

   /**
    * \brief Similarly, we don't want copy assignment happening.
    */
   SteppedOwnerBase & operator=(SteppedOwnerBase const & other) = delete;

   ~SteppedOwnerBase() = default;

public:
   QList<std::shared_ptr<DerivedStep>> steps() const {
      //
      // The StepOwnerBase (Mash, Boil, etc) owns its Steps (MashSteps, BoilSteps, etc).  But, for the moment at least,
      // it's the DerivedStep that knows which StepOwnerBase it's in (and in what order) rather than the StepOwnerBase
      // which knows which Steps it has, so we have to ask.  The only exception to this is if the StepOwnerBase is not
      // yet stored in the DB, in which case there is not yet any StepOwnerBase ID to give the Steps, so we store an
      // internal list of them.
      //
      int const myId = this->derived().key();

      QList<std::shared_ptr<DerivedStep>> steps;
      if (myId < 0) {
         for (int ii : this->m_stepIds) {
            steps.append(ObjectStoreWrapper::getById<DerivedStep>(ii));
         }
      } else {
         steps = ObjectStoreWrapper::findAllMatching<DerivedStep>(
            [myId](std::shared_ptr<DerivedStep> const step) {return step->ownerId() == myId && !step->deleted();}
         );

         // Now we've got the Steps, we need to make sure they're in the right order
         std::sort(steps.begin(),
                   steps.end(),
                   [](std::shared_ptr<DerivedStep> const lhs, std::shared_ptr<DerivedStep> const rhs) {
                      return lhs->stepNumber() < rhs->stepNumber();
                   });
      }

      //
      // It can be that, although they are in the right order, the steps are not canonically numbered.  If this happens,
      // it looks a bit odd in the UI -- eg because you have Instructions in a Recipe starting with Instruction #2 as
      // the first one.  We _could_ fix this in the UI layer, but it's easier to do it here -- and, since we're never
      // talking about more than a handful of steps (often less than 10, usually less than 20, pretty much always less
      // than 30), the absolute overhead of doing so should be pretty small.
      //
      for (int ii = 0; ii < steps.size(); ++ii) {
         //
         // Canonical step numbering starts from 1, which is +1 on canonical indexing!
         //
         int const canonicalStepNumber = ii + 1;
         if (steps[ii]->stepNumber() != canonicalStepNumber) {
            steps[ii]->setStepNumber(canonicalStepNumber);
         }
      }

      return steps;
   }


   /**
    * \brief Inserts a new step at the specified position.  If there is already a step in that position, it (and all
    *        subsequent ones) will be bumped one place down the list.
    *
    * \param step
    * \param number counted from 1
    */
   std::shared_ptr<DerivedStep> insertStep(std::shared_ptr<DerivedStep> step, int const stepNumber) {
      if (this->derived().key() > 0) {
         qDebug() <<
            Q_FUNC_INFO << "Add" << DerivedStep::staticMetaObject.className() << "#" << step->key() << "to" <<
            Derived::staticMetaObject.className() << "#" << this->derived().key();
         step->setOwnerId(this->derived().key());
      }

      // We could skip over prior steps, but the lists are so short it's not worth the extra code IMHO
      for (auto existingStep : this->steps()) {
         int const existingStepNumber = existingStep->stepNumber();
         if (existingStepNumber >= stepNumber) {
            existingStep->setStepNumber(existingStepNumber + 1);
         }
      }

      step->setStepNumber(stepNumber);

      // DerivedStep needs to be in the DB for us to add it to the Derived
      if (step->key() < 0) {
         qDebug() <<
            Q_FUNC_INFO << "Inserting" << DerivedStep::staticMetaObject.className() << "in DB for" <<
            Derived::staticMetaObject.className() << "#" << this->derived().key() << "(" << step->ownerId() << ")";
         ObjectStoreWrapper::insert(step);
      }

      Q_ASSERT(step->key() > 0);

      //
      // If the Derived itself is not yet stored in the DB then it needs to hang on to its list of DerivedSteps so that,
      // when the Derived does get stored, it can tell all the DerivedSteps what their Derived ID is (see doSetKey()).
      //
      // (Conversely, if the Derived is in the DB, then we don't need to do anything further.  We can get all our
      // DerivedSteps any time by just asking the relevant ObjectStore for all DerivedSteps with Derived ID the same as
      // ours.)
      //
      if (this->derived().key() < 0) {
         qDebug() <<
            Q_FUNC_INFO << "Adding" << DerivedStep::staticMetaObject.className() << "#" << step->key() << "to" <<
            Derived::staticMetaObject.className() << "#" << this->derived().key();
         this->m_stepIds.insert(stepNumber - 1, step->key());
      }

      emit this->derived().stepsChanged();

      return step;
   }

   /**
    * \brief Adds a new step at the end of the current list
    */
   std::shared_ptr<DerivedStep> addStep(std::shared_ptr<DerivedStep> step) {
      return this->insertStep(step, this->steps().size() + 1);
   }

   std::shared_ptr<DerivedStep> removeStep(std::shared_ptr<DerivedStep> step) {
      // Disassociate the DerivedStep from this Derived
      step->setOwnerId(-1);

      // As per addStep(), if we're not yet stored in the database, then we also need to update our list of
      // DerivedSteps.
      if (this->derived().key() < 0) {
         int indexOfStep = this->m_stepIds.indexOf(step->key());
         if (indexOfStep < 0 ) {
            // This shouldn't happen, but it doesn't inherently break anything, so just log a warning and carry on
            qWarning() <<
               Q_FUNC_INFO << "Tried to remove" << DerivedStep::staticMetaObject.className() << "#" << step->key() <<
               " (from unsaved" << Derived::staticMetaObject.className() << "#" << this->derived().key() <<
               ") but couldn't find it";
         } else {
            this->m_stepIds.removeAt(indexOfStep);
         }
      }

      //
      // Since a Derived owns its DerivedSteps, we need to remove the DerivedStep from the DB when we remove it from the
      // Derived.  It then makes sense (in the context of undo/redo) to put the DerivedStep object back into "new"
      // state, which ObjectStoreTyped will do for us.
      //
      ObjectStoreWrapper::hardDelete(step);

      this->setCanonicalStepNumbers();

      emit this->derived().stepsChanged();

      return step;
   }

   void setSteps(QList<std::shared_ptr<DerivedStep>> const & val) {
      this->removeAllSteps();
      for (auto step : val) {
         this->addStep(step);
      }
      return;
   }

   unsigned int numSteps() const {
      return this->steps().size();
   }

   /*!
    * \brief Swap Steps \c step1 and \c step2
    */
   void swapSteps(DerivedStep & step1, DerivedStep & step2) {
      // It's a coding error if either of the steps does not belong to this mash / boil
      Q_ASSERT(step1.ownerId() == this->derived().key());
      Q_ASSERT(step2.ownerId() == this->derived().key());

      // It's also a coding error if we're trying to swap a step with itself
      Q_ASSERT(step1.key() != step2.key());

      this->setCanonicalStepNumbers();

      qDebug() <<
         Q_FUNC_INFO << "Swapping steps" << step1.stepNumber() << "(#" << step1.key() << ") and " <<
         step2.stepNumber() << " (#" << step2.key() << ")";

      // Make sure we don't send notifications until the end (hence the false parameter on setStepNumber).
      int temp = step1.stepNumber();
      step1.setStepNumber(step2.stepNumber(), false);
      step2.setStepNumber(temp, false);

      int indexOf1 = this->m_stepIds.indexOf(step1.key());
      int indexOf2 = this->m_stepIds.indexOf(step2.key());

      // We can't swap them if we can't find both of them
      // There's no point swapping them if they're the same
      if (-1 != indexOf1 && -1 != indexOf2 && indexOf1 != indexOf2) {
         this->m_stepIds.swapItemsAt(indexOf1, indexOf2);
      }

      emit this->derived().stepsChanged();
      return;
   }

   void removeAllSteps() {
      auto steps = this->steps();
      qDebug() << Q_FUNC_INFO << "Removing" << steps.size() << "steps from" << this->derived();
      for (auto step : steps) {
         ObjectStoreWrapper::hardDelete(*step);
      }
      this->m_stepIds.clear();
      emit this->derived().stepsChanged();
      return;
   }

   /**
    * \brief Connect DerivedStep changed signals to their parent Mashes.
    *
    *        Needs to be called \b after all the calls to ObjectStoreTyped<FooBar>::getInstance().loadAll()
    */
   static void connectSignals() {
      for (auto dd : ObjectStoreTyped<Derived>::getInstance().getAllRaw()) {
         for (auto step : dd->steps()) {
            Derived::connect(step.get(), &NamedEntity::changed, dd, &Derived::acceptStepChange);
         }
      }
      return;
   }

   /**
    * \brief Needs to be called from Derived::setKey
    */
   void doSetKey(int key) {
      // First call the base class function
      this->derived().NamedEntity::setKey(key);
      // Now give our ID (key) to our DerivedSteps
      for (auto stepId : this->m_stepIds) {
         if (!ObjectStoreWrapper::contains<DerivedStep>(stepId)) {
            // This is almost certainly a coding error, as each DerivedStep is owned by one Derived, but we can
            // (probably) recover by ignoring the missing DerivedStep.
            qCritical() <<
               Q_FUNC_INFO << "Unable to retrieve" << DerivedStep::staticMetaObject.className() << "#" << stepId <<
               "for" << Derived::staticMetaObject.className() << "#" << this->derived().key();
         } else {
            ObjectStoreWrapper::getById<DerivedStep>(stepId)->setOwnerId(key);
         }
      }
      return;
   }

   /**
    * \brief Needs to be called from Derived::hardDeleteOwnedEntities (which is virtual)
    */
   void doHardDeleteOwnedEntities() {
      // It's the DerivedStep that stores its Derived ID, so all we need to do is delete our DerivedSteps then the
      // subsequent database delete of this Derived won't hit any foreign key problems.
      auto steps = this->steps();
      for (auto step : steps) {
         ObjectStoreWrapper::hardDelete<DerivedStep>(*step);
      }
      return;
   }

   /**
    * \brief Returns the step at the specified position, if it exists, or \c nullptr if not
    *
    * \param stepNumber counted from 1
    */
   std::shared_ptr<DerivedStep> stepAt(int const stepNumber) const {
      Q_ASSERT(stepNumber > 0);
      auto mySteps = this->steps();

      if (mySteps.size() >= stepNumber) {
         return mySteps[stepNumber - 1];
      }
      return nullptr;
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
    * \param stepNumber
    */
   void setStepAt(std::shared_ptr<DerivedStep> step, int const stepNumber) {
      Q_ASSERT(stepNumber > 0);
      auto mySteps = this->steps();
      if (mySteps.size() >= stepNumber) {
         // We already have a step of the number supplied, and possibly some subsequent ones

         if (step) {
            // This is an easy case: we're replacing an existing step
            this->removeStep(mySteps[stepNumber - 1]);
            this->insertStep(step, stepNumber);
            return;
         }

         // Caller supplied nullptr, so we're deleting this step and all the ones after it
         for (int stepNumberToDelete = mySteps.size(); stepNumberToDelete >= stepNumber; --stepNumberToDelete) {
            this->removeStep(mySteps[stepNumberToDelete]);
         }
         return;
      }

      // There isn't a step of the number supplied
      if (!step) {
         // Nothing to do if caller supplied std::nullopt
         return;
      }

      // We have to ensure any prior steps exist
      for (int stepNumbertoCreate = mySteps.size(); stepNumbertoCreate < stepNumber; ++stepNumbertoCreate) {
         this->insertStep(std::make_shared<DerivedStep>(), stepNumbertoCreate);
      }
      this->insertStep(step, stepNumber);

      return;
   }

   /**
    * \brief Intended to be called from \c Derived::acceptStepChange
    *
    * \param sender - Result of caller calling \c this->sender() (which is protected, so we can't call it here)
    * \param prop - As received by Derived::acceptStepChange
    * \param val  - As received by Derived::acceptStepChange
    * \param additionalProperties - Additional properties for which to emit \c changed signal if the change we are
    *                               receiving comes from one of our steps.  TODO: Should move this to a template parameter
    */
   void doAcceptStepChange(QObject * sender,
                           [[maybe_unused]] QMetaProperty prop,
                           [[maybe_unused]] QVariant      val,
                           QList<BtStringConst const *> const additionalProperties = {}) {
      DerivedStep * stepSender = qobject_cast<DerivedStep*>(sender);
      if (!stepSender) {
         return;
      }

      // If one of our steps changed, our pseudo properties may also change, so we need to emit some signals
      if (stepSender->ownerId() == this->derived().key()) {
///         emit this->derived().changed(this->derived().metaProperty(*PropertyNames::SteppedOwnerBase::numSteps), QVariant());
         emit this->derived().changed(this->derived().metaProperty(*PropertyNames::SteppedOwnerBase::steps   ), QVariant());
         for (auto property : additionalProperties) {
            emit this->derived().changed(this->derived().metaProperty(**property), QVariant());
         }
      }

      return;
   }

private:
   // The ordering of DerivedSteps within a Derived is stored in the DerivedSteps.  If we remove a DerivedStep from the
   // list, it doesn't break the ordering, but debugging is easier if the step numbers are always sequential starting
   // from 1.
   void setCanonicalStepNumbers() {
      int stepNumber = 1;
      for (auto step : this->derived().steps()) {
         step->setStepNumber(stepNumber++);
      }
      return;
   }

protected:
   //================================================ MEMBER VARIABLES =================================================
   QVector<int> m_stepIds;
};

template<class Derived, class DerivedStep>
TypeLookup const SteppedOwnerBase<Derived, DerivedStep>::typeLookup {
   "SteppedOwnerBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::SteppedOwnerBase::numSteps,
       TypeInfo::construct<MemberFunctionReturnType_t<&SteppedOwnerBase::numSteps>>(
          PropertyNames::SteppedOwnerBase::numSteps,
          TypeLookupOf<MemberFunctionReturnType_t<&SteppedOwnerBase::numSteps>>::value,
          NonPhysicalQuantity::OrdinalNumeral
       )},
      {&PropertyNames::SteppedOwnerBase::steps,
       TypeInfo::construct<MemberFunctionReturnType_t<&SteppedOwnerBase::steps>>(
          PropertyNames::SteppedOwnerBase::steps,
          TypeLookupOf<MemberFunctionReturnType_t<&SteppedOwnerBase::steps>>::value
      )}
   },

   // Parent class lookup: none as we are at the top of this branch of the inheritance tree
   {}
};

/**
 * \brief Concrete derived classes should (either directly or via inclusion in an intermediate class's equivalent macro)
 *        include this in their header file, right after Q_OBJECT.  Concrete derived classes also need to include the
 *        following block ⭐ with appropriate substitution for \b DerivedStep ⭐ (see comment in model/StepBase.h for
 *        why):
 *
 *           // See model/SteppedOwnerBase.h for info, getters and setters for these properties
 *           Q_PROPERTY(QList<std::shared_ptr<DerivedStep>> steps   READ steps   WRITE setSteps   STORED false)
 *           Q_PROPERTY(unsigned int numSteps   READ numSteps   STORED false)
 *
 *        Comments for these properties:
 *
 *           \c steps : The individual steps.
 *
 *           \c numSteps : Number of steps -- mostly for BeerXML.  NB: Read-only.
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions.
 *
 *        In older versions of Qt, we didn't used to be able to put Q_PROPERTY inside our own macro, but, thankfully,
 *        the Qt MOC (meta object compiler) now expands "normal" macros before processing its own "special" ones.
 */
#define STEPPED_OWNER_COMMON_DECL(Derived, DerivedStep) \
   /* This allows SteppedOwnerBase to call protected and private members of Derived */     \
   friend class SteppedOwnerBase<Derived,                                                  \
                                 DerivedStep>;                                             \
                                                                                           \
   public:                                                                                 \
      /* This alias makes it easier to template a number of functions that are */          \
      /* essentially the same for all "Step Owner" classes.                    */          \
      using StepClass = DerivedStep;                                                       \

/**
 * \brief Derived classes should (either directly or via inclusion in an intermediate class's equivalent macro) include
 *        this in their implementation file.
 */
#define STEPPED_OWNER_COMMON_CODE(Derived, DerivedStep) \
   /* Nothing to add for the moment at least! */ \

#endif
