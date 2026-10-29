/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionHop.cpp is part of Brewtarget, and is copyright the following authors 2023-2026:
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

#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionHop.cpp"
#endif

namespace {
   /**
    * \brief Maps the (legacy, BeerXML) \c RecipeAdditionHop::Use onto \c RecipeAddition::Stage and step number.  Used
    *        both by \c RecipeAdditionHop::setUse and by the \c NamedParameterBundle constructor, where we cannot call
    *        \c setUse because the addition is not yet attached to a \c Recipe.
    *
    *        Step numbers assume the standard three-step boil profile (see \c Boil::ensureStandardProfile).
    */
   struct StageAndStep {
      RecipeAddition::Stage stage;
      std::optional<int>    step;
   };
   StageAndStep stageAndStepFor(RecipeAdditionHop::Use const use) {
      switch (use) {
         case RecipeAdditionHop::Use::Mash      : return {RecipeAddition::Stage::Mash        , std::nullopt};
         // A first wort hop is in the ramp-up stage of the boil
         case RecipeAdditionHop::Use::First_Wort: return {RecipeAddition::Stage::Boil        , 1           };
         case RecipeAdditionHop::Use::Boil      : return {RecipeAddition::Stage::Boil        , 2           };
         // An aroma hop is added during the post-boil
         case RecipeAdditionHop::Use::Aroma     : return {RecipeAddition::Stage::Boil        , 3           };
         case RecipeAdditionHop::Use::Dry_Hop   : return {RecipeAddition::Stage::Fermentation, std::nullopt};
         // No default case as we want the compiler to warn us if we missed a case above
      }
      Q_UNREACHABLE(); // We should never get here
   }
}

QString RecipeAdditionHop::localisedName() { return tr("Hop Addition"); }
QString RecipeAdditionHop::localisedName_hop() { return tr("Hop"); }
QString RecipeAdditionHop::localisedName_use() { return tr("Use"); }
QString RecipeAdditionHop::localisedName_beerXmlTime_mins() { return tr("Time"); }

QString RecipeAdditionHop::instanceNameTemplate() { return tr("Addition of %1 hop"); }

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

bool RecipeAdditionHop::compareWith(NamedEntity const & other,
                                    QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeAdditionHop const & rhs = static_cast<RecipeAdditionHop const &>(other);
   return (
      // Parent classes have to be equal
      this->RecipeAddition    ::compareWith  (rhs, propertiesThatDiffer) &&
      this->RecipeAdditionBase::doCompareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount  ::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const RecipeAdditionHop::typeLookup {
   "RecipeAdditionHop",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionHop, hop, hop),
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionHop, use, use),
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionHop, beerXmlTime_mins, beerXmlTime_mins, Measurement::PhysicalQuantity::Time, DisplayInfo::Precision{0}),
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
   RecipeAddition{name, recipeId},
   RecipeAdditionBase<RecipeAdditionHop, Hop>{},
   IngredientAmount<RecipeAdditionHop, Hop>{ingredientId} {

   //
   // RecipeAddition constructor will have set default stage to RecipeAddition::Stage::Mash, but, per comment below, for
   // Hop additions, it makes more sense to default it to RecipeAddition::Stage::Boil
   //
   this->m_stage = RecipeAddition::Stage::Boil;

   CONSTRUCTOR_END
   return;
}

RecipeAdditionHop::RecipeAdditionHop(NamedParameterBundle const & namedParameterBundle) :
   RecipeAddition{namedParameterBundle},
   RecipeAdditionBase<RecipeAdditionHop, Hop>{},
   IngredientAmount<RecipeAdditionHop, Hop>{namedParameterBundle} {
   //
   // Where the addition stage is specified (eg BeerJSON, dotBeer, our DB), it takes precedence.  Otherwise (eg BeerXML)
   // we may have the legacy "use" field (Boil, Dry Hop, Aroma, etc), which tells us the stage and step.  NOTE that we
   // can't just call setUse() here, as it needs this->recipe(), which isn't yet available.  If neither is present, we
   // assume boil, as this is the first stage at which it is usual to add hops.
   //
   if (namedParameterBundle.contains(PropertyNames::RecipeAddition::stage)) {
      this->m_stage = namedParameterBundle.val<RecipeAddition::Stage>(PropertyNames::RecipeAddition::stage);
   } else if (namedParameterBundle.contains(PropertyNames::RecipeAdditionHop::use)) {
      auto const stageAndStep = stageAndStepFor(
         namedParameterBundle.val<RecipeAdditionHop::Use>(PropertyNames::RecipeAdditionHop::use)
      );
      this->m_stage = stageAndStep.stage;
      // An explicit step (eg from the BeerXML STEP extension tag) wins
      if (!this->m_step) {
         this->m_step = stageAndStep.step;
      }
   } else {
      this->m_stage = RecipeAddition::Stage::Boil;
   }

   //
   // The BeerXML TIME field means different things depending on USE (see comment on the beerXmlTime_mins property in
   // the header), so we can only interpret it here, where we know both.  (This only happens for BeerXML: BeerJSON and
   // dotBeer give us addAtTime_mins and duration_mins directly.)  Note that we look at USE even if STAGE is also
   // present, because, in a BeerXML file we wrote, TIME was written according to USE.
   //
   if (namedParameterBundle.contains(PropertyNames::RecipeAdditionHop::beerXmlTime_mins)) {
      std::optional<double> const time {
         namedParameterBundle.val<std::optional<double>>(PropertyNames::RecipeAdditionHop::beerXmlTime_mins)
      };
      RecipeAdditionHop::Use const use {
         namedParameterBundle.contains(PropertyNames::RecipeAdditionHop::use) ?
            namedParameterBundle.val<RecipeAdditionHop::Use>(PropertyNames::RecipeAdditionHop::use) :
            RecipeAdditionHop::Use::Boil
      };
      switch (use) {
         case RecipeAdditionHop::Use::First_Wort:
            // A first wort hop is added at the start of the pre-boil step, so has no "add at" time.  BeerXML's TIME
            // for it is the boil time, which we get from the Boil.
            break;
         case RecipeAdditionHop::Use::Aroma:
            // BeerXML's TIME for an aroma (eg whirlpool) hop is the steeping time.  If we wrote the file, the
            // DURATION_MINS extension tag will also have given us this, so we don't overwrite it.
            [[fallthrough]];
         case RecipeAdditionHop::Use::Dry_Hop:
            // Similarly, BeerXML's TIME for a dry hop is how long the hops stay in, ie the duration, not the time
            // after the start of fermentation at which they are added.
            if (!this->m_duration_mins) {
               this->m_duration_mins = time;
            }
            break;
         case RecipeAdditionHop::Use::Mash:
         case RecipeAdditionHop::Use::Boil:
            this->m_addAtTime_mins = time;
            break;
         // No default case as we want the compiler to warn us if we missed a case above
      }
   }

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
   Q_UNREACHABLE(); // We should never get here
}

std::optional<double> RecipeAdditionHop::beerXmlTime_mins() const {
   // See comment in header for what's going on here.  Working out the Use requires the Recipe (which we always have when
   // writing a BeerXML recipe), so, without one, the best we can do is the "add at" time.
   auto const recipe = this->recipe();
   if (!recipe) {
      return this->m_addAtTime_mins;
   }
   switch (this->use()) {
      case RecipeAdditionHop::Use::First_Wort:
         {
            auto const boil = recipe->boil();
            return boil ? boil->boilTime_mins() : Boil::default_boilTime_mins;
         }
      case RecipeAdditionHop::Use::Aroma:
      case RecipeAdditionHop::Use::Dry_Hop:
         // Older data may have the steeping or dry hop time in addAtTime_mins
         return this->m_duration_mins ? this->m_duration_mins : this->m_addAtTime_mins;
      case RecipeAdditionHop::Use::Mash:
      case RecipeAdditionHop::Use::Boil:
         return this->m_addAtTime_mins;
      // No default case as we want the compiler to warn us if we missed a case above
   }
   Q_UNREACHABLE(); // We should never get here
}

bool RecipeAdditionHop::isFirstWort() const {
   //
   // In switching from Hop::use to RecipeAddition::stage, there is no longer an explicit flag for First Wort Hops.
   // Instead, per dotBeer (see AdditionSchedule in Recipe.beer.schema), a first wort addition is one made in a boil
   // step that ramps up from mash temperature -- ie one that starts at or below Boil::minimumBoilTemperature_c and
   // comes before the boil proper.  We identify the step by its temperatures rather than its number (which is usually,
   // but not necessarily, 1).
   //
   // NB: dotBeer says a first wort addition has no `time` (as it is added at the start of the step), but we do not
   //     insist on that here, because additions migrated from older versions of the database kept their old Hop time
   //     (typically the boil length).
   //
   // We could work this out in a single if statement, but it would be too horrible to look at, so we simply go through
   // all the conditions that have to be satisfied.
   //
   if (this->stage() != RecipeAddition::Stage::Boil) {
      return false;
   }

   if (!this->step()) {
      return false;
   }

   Recipe const * recipe = ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);
   auto boil = recipe->boil();
   if (!boil || boil->boilSteps().empty()) {
      return false;
   }

   auto const boilSteps = boil->boilSteps();
   // Remember RecipeAddition steps are numbered from 1, but lists are indexed from 0
   int const stepIndex = *this->step() - 1;
   if (stepIndex < 0 || stepIndex >= static_cast<int>(boilSteps.size())) {
      return false;
   }

   // The step must start below boiling (with an unset start temperature meaning boiling -- see Boil::startsAtBoil)...
   if (Boil::startsAtBoil(*boilSteps[stepIndex])) {
      return false;
   }

   // ...and must come before the boil proper (otherwise it's a post-boil step that has somehow cooled and been reheated)
   for (int ii = 0; ii < stepIndex; ++ii) {
      if (Boil::isBoilProper(*boilSteps[ii])) {
         return false;
      }
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
   auto const boilSteps = boil->boilSteps();
   auto boilStep = boilSteps[*this->step() - 1];
   // Unset end temperature means boiling -- see Boil::endsAtBoil
   if (Boil::endsAtBoil(*boilStep)) { return false; }

   // Same as for isFirstWort, we go by temperatures: an aroma addition is in a step that comes after the boil proper
   for (int ii = 0; ii < *this->step() - 1; ++ii) {
      if (Boil::isBoilProper(*boilSteps[ii])) {
         return true;
      }
   }
   return false;
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
   auto const stageAndStep = stageAndStepFor(val);
   this->setStage(stageAndStep.stage);
   if (stageAndStep.stage == RecipeAddition::Stage::Boil) {
      this->recipe()->nonOptBoil()->ensureStandardProfile();
   }
   if (stageAndStep.step) {
      this->setStep(stageAndStep.step);
   }
   return;
}

void RecipeAdditionHop::setBeerXmlTime_mins(std::optional<double> const val) {
   //
   // Normally, BeerXML TIME is handled in the NamedParameterBundle constructor (see comment there), so this is here
   // mostly for completeness.  Working out the Use requires the Recipe, so, without one, we can only treat the value as
   // an ordinary "add at" time.
   //
   if (!this->recipe()) {
      this->setAddAtTime_mins(val);
      return;
   }
   switch (this->use()) {
      case RecipeAdditionHop::Use::First_Wort:
         // Boil time is a property of the Boil, not of the addition -- see comment in header
         break;
      case RecipeAdditionHop::Use::Aroma:
      case RecipeAdditionHop::Use::Dry_Hop:
         this->setDuration_mins(val);
         break;
      case RecipeAdditionHop::Use::Mash:
      case RecipeAdditionHop::Use::Boil:
         this->setAddAtTime_mins(val);
         break;
      // No default case as we want the compiler to warn us if we missed a case above
   }
   return;
}

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdditionHop, Hop)
RECIPE_ADDITION_CODE(RecipeAdditionHop, Hop, hop)