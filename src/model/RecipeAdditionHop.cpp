/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionHop.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "model/RecipeAdditionHop.h"

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Boil.h"
#include "model/BoilStep.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionHop.cpp"
#endif

QString RecipeAdditionHop::localisedName() { return tr("Hop Addition"); }

EnumStringMapping const RecipeAdditionHop::useStringMapping {
   {RecipeAdditionHop::Use::Mash      , "Mash"      },
   {RecipeAdditionHop::Use::First_Wort, "First Wort"},
   {RecipeAdditionHop::Use::Boil      , "Boil"      },
   {RecipeAdditionHop::Use::Aroma     , "Aroma"     },
   {RecipeAdditionHop::Use::Dry_Hop   , "Dry Hop"   },
};

EnumStringMapping const RecipeAdditionHop::useDisplayNames {
   {RecipeAdditionHop::Use::Mash      , tr("Mash"      )},
   {RecipeAdditionHop::Use::First_Wort, tr("First Wort")},
   {RecipeAdditionHop::Use::Boil      , tr("Boil"      )},
   {RecipeAdditionHop::Use::Aroma     , tr("Post-Boil" )},
   {RecipeAdditionHop::Use::Dry_Hop   , tr("Dry Hop"   )},
};

ObjectStore & RecipeAdditionHop::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeAdditionHop>::getInstance();
}

TypeLookup const RecipeAdditionHop::typeLookup {
   "RecipeAdditionHop",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::RecipeAdditionHop::hop, RecipeAdditionHop::hop),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::RecipeAdditionHop::use, RecipeAdditionHop::use),
   },
   // Parent classes lookup.  NB: RecipeAddition not NamedEntity!
   {&RecipeAddition::typeLookup,
    std::addressof(IngredientAmount<RecipeAdditionHop, Hop>::typeLookup)}
};
static_assert(std::is_base_of<RecipeAddition, RecipeAdditionHop>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdditionHop, Hop>, RecipeAdditionHop>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Hop>);
static_assert(!HasTypeLookup<QString>);


RecipeAdditionHop::RecipeAdditionHop(QString name, int const recipeId, int const ingredientId) :
   RecipeAddition{name, recipeId, ingredientId},
   RecipeAdditionBase<RecipeAdditionHop, Hop>{},
   IngredientAmount<RecipeAdditionHop, Hop>{} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionHop::RecipeAdditionHop(NamedParameterBundle const & namedParameterBundle) :
   RecipeAddition{namedParameterBundle},
   RecipeAdditionBase<RecipeAdditionHop, Hop>{},
   IngredientAmount<RecipeAdditionHop, Hop>{namedParameterBundle} {
   //
   // If the addition stage is not specified then we assume it is boil, as this is the first stage at which it is usual
   // to add hops.
   //
   m_stage = namedParameterBundle.val<RecipeAddition::Stage>(PropertyNames::RecipeAddition::stage,
                                                             RecipeAddition::Stage::Boil);
///   qDebug() << Q_FUNC_INFO << "RecipeAdditionHop #" << this->key() << ": Recipe #" << this->m_recipeId << ", Hop #" << this->m_ingredientId;

   CONSTRUCTOR_END
   return;
}

RecipeAdditionHop::RecipeAdditionHop(RecipeAdditionHop const & other) :
   RecipeAddition{other},
   RecipeAdditionBase<RecipeAdditionHop, Hop>{},
   IngredientAmount<RecipeAdditionHop, Hop>{other} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionHop::~RecipeAdditionHop() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
RecipeAdditionHop::Use RecipeAdditionHop::use() const {
   switch (this->stage()) {
      case RecipeAddition::Stage::Mash:
         return RecipeAdditionHop::Use::Mash;

      case RecipeAddition::Stage::Boil:
         if (this->isFirstWort()) {
            return RecipeAdditionHop::Use::First_Wort;
         }
         if (this->isAroma()) {
            return RecipeAdditionHop::Use::Aroma;
         }
         return RecipeAdditionHop::Use::Boil;

      case RecipeAddition::Stage::Fermentation:
      case RecipeAddition::Stage::Packaging:
         return RecipeAdditionHop::Use::Dry_Hop;

      // No default case as we want the compiler to warn us if we missed a case above
   }

   // This should be unreachable, but putting a return statement here prevents compiler warnings
   return RecipeAdditionHop::Use::Boil;
}

Hop * RecipeAdditionHop::hop() const {
   // Normally there should always be a valid Hop in a RecipeAdditionHop.  (The Recipe ID may be -1 if the addition is
   // only just about to be added to the Recipe or has just been removed from it, but there's no great reason for the
   // Hop ID not to be valid).
   if (this->m_ingredientId <= 0) {
      qWarning() << Q_FUNC_INFO << "No Hop set on RecipeAdditionHop #" << this->key();
      return nullptr;
   }

///   qDebug() << Q_FUNC_INFO << "RecipeAdditionHop #" << this->key() << ": Recipe #" << this->m_recipeId << ", Hop #" << this->m_ingredientId << "@" << ObjectStoreWrapper::getByIdRaw<Hop>(this->m_ingredientId);
   return ObjectStoreWrapper::getByIdRaw<Hop>(this->m_ingredientId);
}

bool RecipeAdditionHop::isFirstWort() const {
   //
   // In switching from Hop::use to RecipeAddition::stage, there is no longer an explicit flag for First Wort Hops.
   // Instead, a first wort addition is simply(!) one that occurs at the beginning of step 1 of the boil if that step
   // ramps from mash end temperature to boil temperature.
   //
   // We could work this out in a single if statement, but it would be too horrible to look at, so we simply go through
   // all the conditions that have to be satisfied.
   //
   if (this->stage() != RecipeAddition::Stage::Boil) {
      return false;
   }

   // First Wort must be the first step of the boil, during ramp-up from mashout and before the boil proper
   if (!this->step() || *this->step() != 1) {
      return false;
   }

   Recipe const * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);
   auto boil = recipe->boil();
   if (!boil || boil->boilSteps().empty()) {
      return false;
   }

   auto boilStep = boil->boilSteps().first();
   if (!boilStep->startTemp_c() || *boilStep->startTemp_c() > Boil::minimumBoilTemperature_c) {
      return false;
   }

   return true;
}

bool RecipeAdditionHop::isAroma() const {
   //
   // In switching from Hop::use to RecipeAddition::stage, there is no longer an explicit flag for Aroma Hops, ie those
   // added after the boil (aka zero minute hops).
   //
   if (this->stage() != RecipeAddition::Stage::Boil) { return false; }

   // Aroma must be after the first step of the boil
   if (!this->step() || *this->step() == 1) { return false; }

   Recipe const * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);


   auto boil = recipe->boil();
   if (!boil) { return false; }
   if (boil->boilSteps().empty()) { return false; }

   int const numBoilSteps = boil->boilSteps().size();
   if (*this->step() > numBoilSteps) {
      qCritical() <<
         Q_FUNC_INFO << "RecipeAdditionHop #" << this->key() << "in Recipe #" << this->m_recipeId <<
         "has boil step #" << *this->step() << "but boil only has" << numBoilSteps << "steps.  This is probably a bug!";
      return false;
   }

   // Remember RecipeAddition steps are numbered from 1, but vectors are indexed from 0
   auto boilStep = boil->boilSteps()[*this->step() - 1];
   if (!boilStep->endTemp_c() || *boilStep->endTemp_c() > Boil::minimumBoilTemperature_c) { return false; }

   return true;
}

NamedEntity * RecipeAdditionHop::ensureExists(BtStringConst const & property) {
   if (property == PropertyNames::RecipeAdditionHop::hop) {
      // It's a coding error if a RecipeAdditionHop doesn't have a Hop by the time we're accessing it via the property
      // system.
      Hop * hop = this->hop();
      if (!hop) {
         qCritical() << Q_FUNC_INFO << "No Hop set on RecipeAdditionHop #" << this->key();
         // Stop here on debug builds
         Q_ASSERT(false);
      }
      return hop;
   }
   // It's a coding error if we're asked to "create" a relational property we don't know about
   qCritical() << Q_FUNC_INFO << "Don't know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeAdditionHop::setUse(RecipeAdditionHop::Use const val) {
   switch (val) {
      case RecipeAdditionHop::Use::Mash:
         this->setStage(RecipeAddition::Stage::Mash);
         break;

      case RecipeAdditionHop::Use::First_Wort:
         // A first wort hop is in the ramp-up stage of the boil
         this->setStage(RecipeAddition::Stage::Boil);
         this->recipe()->nonOptBoil()->ensureStandardProfile();
         this->setStep(1);
         break;

      case RecipeAdditionHop::Use::Boil:
         this->setStage(RecipeAddition::Stage::Boil);
         this->recipe()->nonOptBoil()->ensureStandardProfile();
         this->setStep(2);
         break;

      case RecipeAdditionHop::Use::Aroma:
         // An aroma hop is added during the post-boil
         this->setStage(RecipeAddition::Stage::Boil);
         this->recipe()->nonOptBoil()->ensureStandardProfile();
         this->setStep(3);
         break;

      case RecipeAdditionHop::Use::Dry_Hop:
         this->setStage(RecipeAddition::Stage::Fermentation);
         break;

      // No default case as we want the compiler to warn us if we missed a case above
   }
   return;
}

void RecipeAdditionHop::setHop(Hop * const val) {
   if (val) {
      this->setIngredientId(val->key());
      this->setName(tr("Addition of %1").arg(val->name()));
   } else {
      // Normally we don't want to invalidate the Hop on a RecipeAdditionHop, because it doesn't buy us anything.
      qWarning() << Q_FUNC_INFO << "Null Hop set on RecipeAdditionHop #" << this->key();
      this->setIngredientId(-1);
      this->setName(tr("Invalid!"));
   }
   return;
}

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdditionHop, Hop)
RECIPE_ADDITION_CODE(RecipeAdditionHop, Hop)
