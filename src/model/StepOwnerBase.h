/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepOwnerBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_STEPSEQUENCE_H
#define MODEL_STEPSEQUENCE_H
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
 */
template<class Derived> class StepOwnerPhantom;
template<class Derived, class DerivedStep>
class StepOwnerBase : public CuriouslyRecurringTemplateBase<StepOwnerPhantom, Derived> {
public:

   StepOwnerBase() : m_stepIds{} {
      return;
   }

   StepOwnerBase(StepOwnerBase const & other) {
      // Deep copy of MashSteps
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

         // Connect signals so that we are notified when there are changes to the DerivedStep we just added to
         // our Mash.
         this->derived().connect(stepToAdd.get(), &NamedEntity::changed, &this->derived(), &Derived::acceptStepChange);
      }
      return;
   }

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
            Derived::staticMetaObject.className() << "#" << this->derived().key();
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

      int temp = step1.stepNumber();
      step1.setStepNumber(step2.stepNumber());
      step2.setStepNumber(temp);

      int indexOf1 = this->m_stepIds.indexOf(step1.key());
      int indexOf2 = this->m_stepIds.indexOf(step2.key());

      // We can't swap them if we can't find both of them
      // There's no point swapping them if they're the same
      if (-1 == indexOf1 || -1 == indexOf2 || indexOf1 == indexOf2) {
         return;
      }

      // As of Qt 5.14 we could write:
      //    this->m_stepIds.swapItemsAt(indexOf1, indexOf2);
      // However, we still need to support slightly older versions of Qt (5.12 in particular), hence the more cumbersome
      // way here.
      std::swap(this->m_stepIds[indexOf1], this->m_stepIds[indexOf2]);

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
   static void doConnectSignals() {
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
            // This is almost certainly a coding error, as each DerivedStep is owned by one Mash, but we can (probably)
            // recover by ignoring the missing DerivedStep.
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
    * \brief Needs to be called from Derived::getOwningRecipe (which is virtual)
    */
   Recipe * doGetOwningRecipe() const {
      return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(this->derived());} );
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

   //
   // A set of convenience functions for accessing the first, second and third steps.  Note that calling setSecondary or
   // setTertiary with something other than std::nullopt needs to ensure the right number of prior step(s) exist, if
   // necessary by creating default ones.
   //
   std::shared_ptr<DerivedStep> doPrimary  () const { return this->stepAt(1); }
   std::shared_ptr<DerivedStep> doSecondary() const { return this->stepAt(2); }
   std::shared_ptr<DerivedStep> doTertiary () const { return this->stepAt(3); }
   void doSetPrimary  (std::shared_ptr<DerivedStep> val) { this->setStepAt(val, 1); return; }
   void doSetSecondary(std::shared_ptr<DerivedStep> val) { this->setStepAt(val, 2); return; }
   void doSetTertiary (std::shared_ptr<DerivedStep> val) { this->setStepAt(val, 3); return; }

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
   QVector<int> m_stepIds;
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
   /* This allows StepOwnerBase to call protected and private members of Derived */      \
   friend class StepOwnerBase<NeName,                                                    \
                              NeName##Step>;                                             \
                                                                                         \
   public:                                                                               \
      /* This alias makes it easier to template a number of functions that are */        \
      /* essentially the same for all "Step Owner" classes.                    */        \
      using StepClass = NeName##Step;                                                    \
                                                                                         \
      /* Relational getters and setters */                                               \
      QList<std::shared_ptr<NeName##Step>> LcNeName##Steps        () const;              \
      void set##NeName##Steps        (QList<std::shared_ptr<NeName##Step>> const & val); \
                                                                                         \
      /** \brief Connect DerivedStep changed signals to their parent Mashes. */          \
      /*         Needs to be called \b after all the calls to                */          \
      /*         ObjectStoreTyped<FooBar>::getInstance().loadAll()           */          \
      static void connectSignals();                                                      \
                                                                                         \
      virtual void setKey(int key);                                                      \
                                                                                         \
      virtual Recipe * getOwningRecipe() const;                                          \
      /** \brief NeName owns its NeName##Steps so needs to delete them if it */          \
      /*         itself is being deleted                                     */          \
      virtual void hardDeleteOwnedEntities();                                            \
                                                                                         \
      /* We don't put the step name in these getters/setters as it would become */       \
      /* unwieldy - eg setSecondaryFermentationStep()                           */       \
      std::shared_ptr<NeName##Step> primary  () const;                                   \
      std::shared_ptr<NeName##Step> secondary() const;                                   \
      std::shared_ptr<NeName##Step> tertiary () const;                                   \
      void setPrimary  (std::shared_ptr<NeName##Step> val);                              \
      void setSecondary(std::shared_ptr<NeName##Step> val);                              \
      void setTertiary (std::shared_ptr<NeName##Step> val);                              \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_OWNER_COMMON_CODE(NeName, LcNeName) \
   QList<std::shared_ptr<NeName##Step>> NeName::LcNeName##Steps        () const { return this->steps(); } \
   void NeName::set##NeName##Steps(QList<std::shared_ptr<NeName##Step>> const & val) {                    \
      this->setSteps(val); return;                                                                        \
   }                                                                                                      \
                                                                                                          \
   void NeName::connectSignals() { StepOwnerBase<NeName, NeName##Step>::doConnectSignals(); return; }     \
                                                                                                          \
   void NeName::setKey(int key) { this->doSetKey(key); return; }                                          \
                                                                                                          \
   Recipe * NeName::getOwningRecipe() const { return this->doGetOwningRecipe(); }                         \
   void NeName::hardDeleteOwnedEntities() { this->doHardDeleteOwnedEntities(); return; }                  \
                                                                                                          \
   std::shared_ptr<NeName##Step> NeName::primary  () const { return this->doPrimary  (); }                \
   std::shared_ptr<NeName##Step> NeName::secondary() const { return this->doSecondary(); }                \
   std::shared_ptr<NeName##Step> NeName::tertiary () const { return this->doTertiary (); }                \
   void NeName::setPrimary  (std::shared_ptr<NeName##Step> val) { this->doSetPrimary  (val); return; }    \
   void NeName::setSecondary(std::shared_ptr<NeName##Step> val) { this->doSetSecondary(val); return; }    \
   void NeName::setTertiary (std::shared_ptr<NeName##Step> val) { this->doSetTertiary (val); return; }    \

#endif
