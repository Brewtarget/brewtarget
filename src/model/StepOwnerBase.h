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
#include "model/SteppedOwnerBase.h"

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
 *        Note that we do \b not inherit from \c CuriouslyRecurringTemplateBase because \c SteppedOwnerBase already does
 *        this.  If we inherited again, we'd end up with two (identical) implementations of this->derived() that the
 *        compiler can't disambiguate between.
 */
template<class Derived> class StepOwnerPhantom;
template<class Derived, class DerivedStep>
class StepOwnerBase : public SteppedOwnerBase<Derived, DerivedStep> {
public:
   // Note that, because this is static, it cannot be initialised inside the class definition
   static TypeLookup const typeLookup;

   StepOwnerBase() :
      SteppedOwnerBase<Derived, DerivedStep>{} {
      return;
   }

   StepOwnerBase(NamedParameterBundle const & namedParameterBundle) :
      SteppedOwnerBase<Derived, DerivedStep>{namedParameterBundle} {
      return;
   }

   StepOwnerBase(Derived const & other) :
      SteppedOwnerBase<Derived, DerivedStep>{other} {
      return;
   }

   /**
    * \brief We have to delete the default copy constructor because we want the constructor above (that takes \c Derived
    *        rather than \c SteppedOwnerBase) to be used instead of a compiler-generated copy constructor which wouldn't
    *        do the deep copy we need.
    */
   StepOwnerBase(StepOwnerBase const & other) = delete;

   /**
    * \brief Similarly, we don't want copy assignment happening.
    */
   StepOwnerBase & operator=(StepOwnerBase const & other) = delete;

   ~StepOwnerBase() = default;

};

template<class Derived, class DerivedStep>
TypeLookup const StepOwnerBase<Derived, DerivedStep>::typeLookup {
   "StepOwnerBase",
   {
      // We don't have any properties
   },
   // Parent class lookup
   {&SteppedOwnerBase<Derived, DerivedStep>::typeLookup}
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
   STEPPED_OWNER_COMMON_DECL(NeName, NeName##Step) \
   /* This allows StepOwnerBase to call protected and private members of Derived */ \
   friend class StepOwnerBase<NeName,                                               \
                              NeName##Step>;                                        \
                                                                                    \
   public:                                                                          \
                                                                                    \
      /** \brief Overrides \c NamedEntity::setKey()  */                             \
      virtual void setKey(int key) override;                                        \
                                                                                    \
      /** \brief Overrides \c NamedEntity::hardDeleteOwnedEntities()         */     \
      /*         Derived owns its DerivedSteps so needs to delete them if it */     \
      /*         itself is being deleted                                     */     \
      virtual void hardDeleteOwnedEntities() override;                              \
                                                                                    \
      /* TODO These are aliases we should probably get rid of */                    \
      QList<std::shared_ptr<NeName##Step>> LcNeName##Steps() const;                 \
      void set##NeName##Steps(QList<std::shared_ptr<NeName##Step>> const & val);    \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_OWNER_COMMON_CODE(NeName, LcNeName) \
   STEPPED_OWNER_COMMON_CODE(NeName, NeName##Step) \
   void NeName::setKey(int key) { this->doSetKey(key); return; }                                  \
                                                                                                  \
   void NeName::hardDeleteOwnedEntities() { this->doHardDeleteOwnedEntities(); return; }          \
                                                                                                  \
   QList<std::shared_ptr<NeName##Step>> NeName::LcNeName##Steps() const { return this->steps(); } \
   void NeName::set##NeName##Steps(QList<std::shared_ptr<NeName##Step>> const & val) {            \
      this->setSteps(val); return;                                                                \
   }                                                                                              \

#endif
