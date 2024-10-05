/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepBase.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_STEPBASE_H
#define MODEL_STEPBASE_H
#pragma once

#include <optional>

#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"
#include "model/Step.h"
#include "PhysicalConstants.h"
#include "utils/CuriouslyRecurringTemplateBase.h"
#include "utils/TypeTraits.h"

namespace {
   // Where step time is required to have a value, this is the default
   constexpr double defaultStepTime_mins {0.0};
   // Where start temperature is required to have a value, this is the default
   constexpr double defaultStartTemp_c {22.2};

   // Everything has to be double because our underlying measure (minutes) is allowed to be measured in fractions.
   constexpr double minutesInADay = 24.0 * 60.0;
   std::optional<double> daysToMinutes(std::optional<double> val) {
      if (val) {
         return *val * minutesInADay;
      }
      return std::nullopt;
   }
}

/**
 * \brief Per the comments in model/Step.h on individual properties, there are a couple of cases where we want to be
 *        able to make minor changes between different subclasses -- eg \c stepTime_mins is required for \c MashStep but
 *        optional for \c BoilStep and \c FermentationStep.  \c Step itself cannot be a templated class because the Qt
 *        Meta Object Compiler (moc) will barf out "Template classes not supported by Q_OBJECT", but we can still do
 *        most of the work at compile-time here.
 */
struct StepBaseOptions {
   //! By default stepTime_mins is optional
   bool stepTimeRequired = false;
   //! By default startTemp_c is optional
   bool startTempRequired = false;
   //! By default rampTime_mins is not supported
   bool rampTimeSupported = false;
};
template <StepBaseOptions sbo> struct has_stepTimeRequired : public std::integral_constant<bool, sbo.stepTimeRequired >{};
template <StepBaseOptions sbo> struct has_startTempRequired: public std::integral_constant<bool, sbo.startTempRequired>{};
template <StepBaseOptions sbo> struct has_rampTimeSupported: public std::integral_constant<bool, sbo.rampTimeSupported>{};
// See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
template <StepBaseOptions ebo> concept CONCEPT_FIX_UP StepTimeRequired  = has_stepTimeRequired <ebo>::value;
template <StepBaseOptions ebo> concept CONCEPT_FIX_UP StartTempRequired = has_startTempRequired<ebo>::value;
template <StepBaseOptions ebo> concept CONCEPT_FIX_UP RampTimeSupported = has_rampTimeSupported<ebo>::value;

/**
 * \brief  Additional base class for \c MashStep, \c BoilStep, \c FermentationStep to provide strongly-typed functions
 *         using CRTP.
 */
template<class Derived> class StepPhantom;
template<class Derived, class Owner, StepBaseOptions stepBaseOptions>
class StepBase : public CuriouslyRecurringTemplateBase<StepPhantom, Derived> {
protected:
   StepBase() {
      return;
   }

   StepBase([[maybe_unused]] NamedParameterBundle const & namedParameterBundle) {
      // We intend that Derived should always inherit from Step before it inherits from StepBase.  So, at this point,
      // the Step bits of Derived() will be constructed and initialised from namedParameterBundle.

      //
      // If we're being constructed from a BeerXML file, we use the property stepTime_days for RECIPE > PRIMARY_AGE etc
      // Otherwise we use the stepTime_mins property.
      //
      // See comment in Step.h for why we cannot do this in the Step constructor.
      //
      if (!SET_IF_PRESENT_FROM_NPB_NO_MV(StepBase::doSetStepTime_mins, namedParameterBundle, PropertyNames::Step::stepTime_mins) &&
          !SET_IF_PRESENT_FROM_NPB_NO_MV(StepBase::doSetStepTime_days, namedParameterBundle, PropertyNames::Step::stepTime_days)) {
         this->derived().m_stepTime_mins = std::nullopt;
      }


      // Override the std::nullopt default for step time and/or start temp if subclass so requires
      this->derived().m_stepTime_mins = this->correctStepTime_mins(this->derived().m_stepTime_mins);
      this->derived().m_startTemp_c   = this->correctStartTemp_c  (this->derived().m_startTemp_c  );

      if (this->derived().m_rampTime_mins) {
         this->checkRampTimeSupported();
      }
      return;
   }

   StepBase([[maybe_unused]] Derived const & other) {
      return;
   }

   /**
    * \brief Steps do not directly belong to a \c Recipe, but a step's owner (ie its \c Mash, \c Boil, \c Fermentation)
    *        does.
    */
   Recipe * doGetOwningRecipe() const {
      Owner const * owner = ObjectStoreWrapper::getByIdRaw<Owner>(this->derived().m_ownerId);
      if (!owner) {
         return nullptr;
      }
      return owner->getOwningRecipe();
   }

   //! No-op version
   std::optional<double> correctStepTime_mins(std::optional<double> const val) const
   requires (!StepTimeRequired<stepBaseOptions>) {
      return val;
   }
   //! Substantive version
   std::optional<double> correctStepTime_mins(std::optional<double> const val) const
   requires (StepTimeRequired<stepBaseOptions>) {
      // You might think we could assert here that either step time is optional or val is not std::nullopt.  But it's
      // not that simple, as generic code will have read that stepTime_mins is an optional field.  So, the best we can
      // do here is force std::nullopt to some default value;
      return val.value_or(defaultStepTime_mins);
   }

   std::optional<double> doStepTime_mins() const {
      // If subclass needs step time to be non-optional then we ensure std::nullopt cannot be returned
      return this->correctStepTime_mins(this->derived().m_stepTime_mins);
   }
   void doSetStepTime_mins(std::optional<double> const val) {
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::Step::stepTime_mins,
                                   this->derived().m_stepTime_mins,
                                   this->correctStepTime_mins(val));
      return;
   }
   std::optional<double> doStepTime_days() const {
      auto const val = this->doStepTime_mins();
      // Convert minutes to days
      if (val) {
         return *val / minutesInADay;
      }
      return std::nullopt;
   }
   void doSetStepTime_days(std::optional<double> const val) {
      this->doSetStepTime_mins(daysToMinutes(val));
      return;
   }

   //! No-op version
   std::optional<double> correctStartTemp_c(std::optional<double> const val) const
   requires (!StartTempRequired<stepBaseOptions>) {
      return val;
   }
   //! Substantive version
   std::optional<double> correctStartTemp_c(std::optional<double> const val) const
   requires (StartTempRequired<stepBaseOptions>) {
      return val.value_or(defaultStartTemp_c);
   }

   std::optional<double> doStartTemp_c  () const {
      return this->correctStartTemp_c(this->derived().m_startTemp_c);
   }
   void doSetStartTemp_c(std::optional<double> const val) {
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::Step::startTemp_c,
                                   this->derived().m_startTemp_c,
                                   this->derived().enforceMin(this->correctStartTemp_c(val),
                                                              "start temp",
                                                              PhysicalConstants::absoluteZero));
      return;
   }

   //! No-op version
   void checkRampTimeSupported() const requires (RampTimeSupported<stepBaseOptions>) {
      return;
   }
   //! Substantive version
   void checkRampTimeSupported() const requires (!RampTimeSupported<stepBaseOptions>) {
      // It would probably be a coding error if we got here, but I _think_ it would also be harmless to carry on rather
      // than assert.
      qCritical().noquote() <<
         Q_FUNC_INFO << "Ramp time not supported for" << this->derived().metaObject()->className() << ":" <<
         Logging::getStackTrace();
      return;
   }

   std::optional<double> doRampTime_mins() const {
      this->checkRampTimeSupported();
      return this->derived().m_rampTime_mins;
   }

   void doSetRampTime_mins(std::optional<double> const val) {
      this->checkRampTimeSupported();
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::Step::rampTime_mins, this->derived().m_rampTime_mins, val);
      return;
   }

   ObjectStore & doGetObjectStoreTypedInstance() const {
      return ObjectStoreTyped<Derived>::getInstance();
   }

};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions
 */
#define STEP_COMMON_DECL(NeName, Options) \
   /* This allows StepBase to call protected and private members of Derived */  \
   friend class StepBase<NeName##Step,                                          \
                         NeName,                                                \
                         Options>;                                              \
                                                                                \
   public:                                                                      \
      /* This alias makes it easier to template a number of functions */        \
      /* that are essentially the same for all "Step" classes.        */        \
      using StepOwnerClass = NeName;                                            \
                                                                                \
      virtual Recipe * getOwningRecipe() const;                                 \
                                                                                \
      virtual std::optional<double> stepTime_mins() const override;             \
      virtual std::optional<double> stepTime_days() const override;             \
      virtual std::optional<double> startTemp_c  () const override;             \
      virtual std::optional<double> rampTime_mins() const override;             \
      virtual void setStepTime_mins(std::optional<double> const val) override;  \
      virtual void setStepTime_days(std::optional<double> const val) override;  \
      virtual void setStartTemp_c  (std::optional<double> const val) override;  \
      virtual void setRampTime_mins(std::optional<double> const val) override;  \
                                                                                \
   protected:                                                                   \
      /** Override NamedEntity::getObjectStoreTypedInstance */                  \
      virtual ObjectStore & getObjectStoreTypedInstance() const override;       \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_COMMON_CODE(NeName) \
   Recipe *              NeName##Step::getOwningRecipe() const { return this->doGetOwningRecipe(); }                 \
   std::optional<double> NeName##Step::stepTime_mins  () const { return this->doStepTime_mins  (); }                 \
   std::optional<double> NeName##Step::stepTime_days  () const { return this->doStepTime_days  (); }                 \
   std::optional<double> NeName##Step::startTemp_c    () const { return this->doStartTemp_c    (); }                 \
   std::optional<double> NeName##Step::rampTime_mins  () const { return this->doRampTime_mins  (); }                 \
   void NeName##Step::setStepTime_mins(std::optional<double> const val) { this->doSetStepTime_mins(val); return; }   \
   void NeName##Step::setStepTime_days(std::optional<double> const val) { this->doSetStepTime_days(val); return; }   \
   void NeName##Step::setStartTemp_c  (std::optional<double> const val) { this->doSetStartTemp_c  (val); return; }   \
   void NeName##Step::setRampTime_mins(std::optional<double> const val) { this->doSetRampTime_mins(val); return; }   \
   ObjectStore & NeName##Step::getObjectStoreTypedInstance() const { return this->doGetObjectStoreTypedInstance(); } \

#endif
