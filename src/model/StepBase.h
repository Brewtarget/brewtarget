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

#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"
#include "utils/CuriouslyRecurringTemplateBase.h"

/**
 * \brief  Additional base class for \c MashStep, \c BoilStep, \c FermentationStep to provide strongly-typed functions
 *         using CRTP.
 */
template<class Derived> class StepPhantom;
template<class Derived, class Owner>
class StepBase : public CuriouslyRecurringTemplateBase<StepPhantom, Derived> {
protected:

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
#define STEP_COMMON_DECL(NeName) \
   /* This allows StepBase to call protected and private members of Derived */  \
   friend class StepBase<NeName##Step,                                          \
                         NeName>;                                               \
                                                                                \
   public:                                                                      \
      /* This alias makes it easier to template a number of functions */        \
      /* that are essentially the same for all "Step" classes.        */        \
      using StepOwnerClass = NeName;                                            \
                                                                                \
      virtual Recipe * getOwningRecipe() const;                                 \
                                                                                \
   protected:                                                                   \
      /** Override NamedEntity::getObjectStoreTypedInstance */                  \
      virtual ObjectStore & getObjectStoreTypedInstance() const;                \

/**
 * \brief Derived classes should include this in their implementation file
 */
#define STEP_COMMON_CODE(NeName) \
   Recipe *      NeName##Step::getOwningRecipe            () const { return this->doGetOwningRecipe            (); } \
   ObjectStore & NeName##Step::getObjectStoreTypedInstance() const { return this->doGetObjectStoreTypedInstance(); } \

#endif
