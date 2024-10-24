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

#include "model/Recipe.h"
#include "model/Step.h"
#include "model/SteppedBase.h"
#include "PhysicalConstants.h"
#include "utils/AutoCompare.h"
#include "utils/TypeTraits.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::StepBase { BtStringConst const property{#property}; }
AddPropertyName(rampTime_mins)
AddPropertyName(startTemp_c  )
AddPropertyName(stepTime_days) // Mostly needed for BeerXML
AddPropertyName(stepTime_mins)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

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
 * \brief Additional base class for \c MashStep, \c BoilStep, \c FermentationStep to provide strongly-typed functions
 *        using CRTP.
 *
 *        We also implement some properties where we want slightly different handling for the different derived classes.
 *        (The regular properties that are exactly the same for all derived classes are in \c Step and \c StepExtended.)
 *
 *        Note that we do \b not inherit from \c CuriouslyRecurringTemplateBase because \c SteppedBase already does
 *        this.  If we inherited again, we'd end up with two (identical) implementations of this->derived() that the
 *        compiler can't disambiguate between.
 */
template<class Derived> class StepPhantom;
template<class Derived, class Owner, StepBaseOptions stepBaseOptions>
class StepBase : public SteppedBase<Derived, Owner> {
protected:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   //! Non-virtual equivalent of isEqualTo
   bool doIsEqualTo(StepBase const & other) const {
      return (
         Utils::AutoCompare(this->m_stepTime_mins, other.m_stepTime_mins) &&
         Utils::AutoCompare(this->m_startTemp_c  , other.m_startTemp_c  ) &&
         Utils::AutoCompare(this->m_rampTime_mins, other.m_rampTime_mins) &&
         // Parent classes have to be equal too
         this->SteppedBase<Derived, Owner>::doIsEqualTo(other)
      );
   }

private:
   friend Derived;
   StepBase() :
      SteppedBase<Derived, Owner>{} {
      return;
   }

   StepBase(NamedParameterBundle const & namedParameterBundle) :
      SteppedBase<Derived, Owner>{namedParameterBundle},
      // See below for m_stepTime_mins
      SET_REGULAR_FROM_NPB (m_startTemp_c  , namedParameterBundle, PropertyNames::StepBase::startTemp_c  , std::nullopt),
      SET_REGULAR_FROM_NPB (m_rampTime_mins, namedParameterBundle, PropertyNames::StepBase::rampTime_mins, std::nullopt) {
      // We intend that Derived should always inherit from Step before it inherits from StepBase.  So, at this point,
      // the Step bits of Derived() will be constructed and initialised from namedParameterBundle.

      //
      // If we're being constructed from a BeerXML file, we use the property stepTime_days for RECIPE > PRIMARY_AGE etc
      // Otherwise we use the stepTime_mins property.
      //
      // See comment in Step.h for why we cannot do this in the Step constructor.
      //
      if (!SET_IF_PRESENT_FROM_NPB_NO_MV(StepBase::setStepTime_mins, namedParameterBundle, PropertyNames::StepBase::stepTime_mins) &&
          !SET_IF_PRESENT_FROM_NPB_NO_MV(StepBase::setStepTime_days, namedParameterBundle, PropertyNames::StepBase::stepTime_days)) {
         this->m_stepTime_mins = std::nullopt;
      }

      // Override the std::nullopt default for step time and/or start temp if subclass so requires
      this->m_stepTime_mins = this->correctStepTime_mins(this->m_stepTime_mins);
      this->m_startTemp_c   = this->correctStartTemp_c  (this->m_startTemp_c  );

      if (this->m_rampTime_mins) {
         this->checkRampTimeSupported();
      }
      return;
   }

   StepBase(Derived const & other) :
      SteppedBase<Derived, Owner>{other},
      m_stepTime_mins{other.m_stepTime_mins},
      m_startTemp_c  {other.m_startTemp_c  },
      m_rampTime_mins{other.m_rampTime_mins} {
      return;
   }

private:
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

public:
   std::optional<double> stepTime_mins() const {
      // If subclass needs step time to be non-optional then we ensure std::nullopt cannot be returned
      return this->correctStepTime_mins(this->m_stepTime_mins);
   }
   void setStepTime_mins(std::optional<double> const val) {
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::StepBase::stepTime_mins,
                                   this->m_stepTime_mins,
                                   this->correctStepTime_mins(val));
      return;
   }
   std::optional<double> stepTime_days() const {
      auto const val = this->stepTime_mins();
      // Convert minutes to days
      if (val) {
         return *val / minutesInADay;
      }
      return std::nullopt;
   }
   void setStepTime_days(std::optional<double> const val) {
      this->setStepTime_mins(daysToMinutes(val));
      return;
   }

private:
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

public:
   std::optional<double> startTemp_c  () const {
      return this->correctStartTemp_c(this->m_startTemp_c);
   }
   void setStartTemp_c(std::optional<double> const val) {
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::StepBase::startTemp_c,
                                   this->m_startTemp_c,
                                   this->derived().enforceMin(this->correctStartTemp_c(val),
                                                              "start temp",
                                                              PhysicalConstants::absoluteZero));
      return;
   }

private:
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

public:
   std::optional<double> rampTime_mins() const {
      this->checkRampTimeSupported();
      return this->derived().m_rampTime_mins;
   }

   void setRampTime_mins(std::optional<double> const val) {
      this->checkRampTimeSupported();
      // Can't use SET_AND_NOTIFY macro here, but fortunately it's trivial
      this->derived().setAndNotify(PropertyNames::StepBase::rampTime_mins, this->m_rampTime_mins, val);
      return;
   }

protected:
   //================================================ MEMBER VARIABLES =================================================
   std::optional<double> m_stepTime_mins = std::nullopt;
   std::optional<double> m_startTemp_c   = std::nullopt;
   std::optional<double> m_rampTime_mins = std::nullopt;

};

template<class Derived, class Owner, StepBaseOptions stepBaseOptions>
TypeLookup const StepBase<Derived, Owner, stepBaseOptions>::typeLookup {
   "StepBase",
   {
      //
      // See comment in model/IngredientAmount.h for why we can't use the PROPERTY_TYPE_LOOKUP_ENTRY or
      // PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV macros here.
      //
      {&PropertyNames::StepBase::stepTime_mins,
       TypeInfo::construct<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_stepTime_mins)>(
          PropertyNames::StepBase::stepTime_mins,
          TypeLookupOf<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_stepTime_mins)>::value,
          Measurement::PhysicalQuantity::Time
       )},
      {&PropertyNames::StepBase::stepTime_days,
       TypeInfo::construct<MemberFunctionReturnType_t<&StepBase<Derived, Owner, stepBaseOptions>::stepTime_days>>(
          PropertyNames::StepBase::stepTime_days,
          TypeLookupOf<MemberFunctionReturnType_t<&StepBase<Derived, Owner, stepBaseOptions>::stepTime_days>>::value,
          // Note that, because days is not our canonical unit of measurement for time, this has to be a
          // NonPhysicalQuantity, not Measurement::PhysicalQuantity::Time.
          NonPhysicalQuantity::OrdinalNumeral
       )},
      {&PropertyNames::StepBase::startTemp_c,
       TypeInfo::construct<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_startTemp_c)>(
          PropertyNames::StepBase::startTemp_c,
          TypeLookupOf<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_startTemp_c)>::value,
          Measurement::PhysicalQuantity::Temperature
       )},
      {&PropertyNames::StepBase::rampTime_mins,
       TypeInfo::construct<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_rampTime_mins)>(
          PropertyNames::StepBase::rampTime_mins,
          TypeLookupOf<decltype(StepBase<Derived, Owner, stepBaseOptions>::m_rampTime_mins)>::value,
          Measurement::PhysicalQuantity::Time
       )},
   },
   // Parent class lookup
   {&SteppedBase<Derived, Owner>::typeLookup}
};

/**
 * \brief Derived classes should include this in their header file, right after Q_OBJECT.  Concrete derived classes also
 *        need to include the following block (as well as the equivalent from \c model/SteppedBase.h):
 *
 *           // See model/StepBase.h for info, getters and setters for these properties
 *           Q_PROPERTY(std::optional<double> stepTime_mins   READ stepTime_mins   WRITE setStepTime_mins)
 *           Q_PROPERTY(std::optional<double> stepTime_days   READ stepTime_days   WRITE setStepTime_days)
 *           Q_PROPERTY(std::optional<double> startTemp_c     READ startTemp_c     WRITE setStartTemp_c  )
 *           Q_PROPERTY(std::optional<double> rampTime_mins   READ rampTime_mins   WRITE setRampTime_mins)
 *
 *        This is needed because, although recent versions of Qt MOC (Meta Object Compiler) now "fully expand macros"
 *        (at least according to https://woboq.com/blog/moc-myths.html), they do not appear to pick up Q_PROPERTY
 *        declarations inside a macro.  One day, maybe we could work out why by looking at
 *        https://github.com/qt/qtbase/blob/dev/src/tools/moc/preprocessor.cpp
 *
 *        Comments for these properties:
 *
 *        \c stepTime_mins : The time of the step in min.
 *                           NOTE: This is required for \c MashStep but optional for \c BoilStep and
 *                                 \c FermentationStep.  We make it optional here but classes that need it required
 *                                 should set \c StepBaseOptions.stepTimeIsRequired parameter on \c StepBase template.
 *
 *        \c stepTime_days : The time of the step in days - primarily for convenience on \c FermentationStep where
 *                           measuring in minutes is overly precise.  The underlying measure in the database remains
 *                           minutes however, for consistency.
 *
 *        \c startTemp_c : Per comment in \cmodel/Step.h, this is also referred to as step temperature when talking
 *                         about Mash Steps.  For a \c MashStep, this is the target temperature of this step in °C.
 *                         This is the main field to use when dealing with the mash step temperature.
 *
 *                         NOTE: This is required for MashStep but optional for BoilStep and FermentationStep.  We make
 *                               it optional here but classes that need it required should set
 *                               \c StepBaseOptions.startTempIsRequired parameter on \c StepBase template.
 *
 *        \c rampTime_mins : The time it takes to ramp the temp to the target temp in min - ie the amount of time that
 *                           passes before this step begins.                    ⮜⮜⮜ Optional in BeerXML & BeerJSON ⮞⮞⮞
 *
 *                           Eg for \c MashStep, moving from a mash step (step 1) of 148F, to a new temperature step of
 *                           156F (step 2) may take 8 minutes to heat the mash. Step 2 would have a ramp time of 8
 *                           minutes.
 *
 *                           Similarly, for a \c BoilStep, moving from a boiling step (step 1) to a whirlpool step
 *                           (step 2) may take 5 minutes.  Step 2 would have a ramp time of 5 minutes, hop isomerization
 *                           and bitterness calculations will need to account for this accordingly.
 *
 *                           NOTE: This property is \b not used by \c FermentationStep.  (It is the only property shared
 *                                 by \c MashStep and \c BoilStep that is not also needed in \c FermentationStep.  We
 *                                 can't really do mix-ins in Qt, so it's simplest just to not use it in
 *                                 \c FermentationStep.  We require the classes that use this property to set set
 *                                 \c StepBaseOptions.rampTimeIsSupported parameter on \c StepBase template, so we can
 *                                 at least get a run-time error if we accidentally try to use this property on a
 *                                 \c FermentationStep.)
 *
 *        Although we could do more in this macro, we limit it to member functions that are just wrappers around calls
 *        to this base class.
 *
 *        Note we have to be careful about comment formats in macro definitions.
 */
#define STEP_COMMON_DECL(NeName, Options) \
   STEPPED_COMMON_DECL(NeName##Step, NeName) \
   /* This allows StepBase to call protected and private members of Derived */  \
   friend class StepBase<NeName##Step,                                          \
                         NeName,                                                \
                         Options>;                                              \


/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_COMMON_CODE(NeName) \
   STEPPED_COMMON_CODE(NeName##Step)   \

#endif
