/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionMisc.cpp is part of Brewtarget, and is copyright the following authors 2023-2026:
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
#include "model/RecipeAdditionMisc.h"

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Boil.h"
#include "model/BoilStep.h"

#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionMisc.cpp"
#endif

namespace {
   /**
    * \brief Maps the (legacy, BeerXML) \c RecipeAdditionMisc::Use onto \c RecipeAddition::Stage and step number.  Used
    *        both by \c RecipeAdditionMisc::setUse and by the \c NamedParameterBundle constructor, where we cannot call
    *        \c setUse because the addition is not yet attached to a \c Recipe.
    */
   struct StageAndStep {
      RecipeAddition::Stage stage;
      std::optional<int>    step;
   };
   StageAndStep stageAndStepFor(RecipeAdditionMisc::Use const use) {
      switch (use) {
         case RecipeAdditionMisc::Use::Mash     : return {RecipeAddition::Stage::Mash        , std::nullopt};
         case RecipeAdditionMisc::Use::Boil     : return {RecipeAddition::Stage::Boil        , 2           };
         case RecipeAdditionMisc::Use::Primary  : return {RecipeAddition::Stage::Fermentation, 1           };
         case RecipeAdditionMisc::Use::Secondary: return {RecipeAddition::Stage::Fermentation, 2           };
         case RecipeAdditionMisc::Use::Bottling : return {RecipeAddition::Stage::Packaging   , std::nullopt};
         // No default case as we want the compiler to warn us if we missed a case above
      }
      Q_UNREACHABLE(); // We should never get here
   }
}

QString RecipeAdditionMisc::localisedName() { return tr("Misc Addition"); }
QString RecipeAdditionMisc::localisedName_misc() { return tr("Misc"); }
QString RecipeAdditionMisc::localisedName_use () { return tr("Use "); }

// Unlike with hop, fermentable, yeast, etc, I don't think including the word "misc" or "miscellaneous" in the instance
// name is helpful.
QString RecipeAdditionMisc::instanceNameTemplate() { return tr("Addition of %1"); }

EnumStringMapping const RecipeAdditionMisc::useStringMapping {
   {RecipeAdditionMisc::Use::Mash     , "Mash"     },
   {RecipeAdditionMisc::Use::Boil     , "Boil"     },
   {RecipeAdditionMisc::Use::Primary  , "Primary"  },
   {RecipeAdditionMisc::Use::Secondary, "Secondary"},
   {RecipeAdditionMisc::Use::Bottling , "Bottling" }
};

EnumStringMapping const RecipeAdditionMisc::useDisplayNames {
   {RecipeAdditionMisc::Use::Mash     , tr("Mash"     )},
   {RecipeAdditionMisc::Use::Boil     , tr("Boil"     )},
   {RecipeAdditionMisc::Use::Primary  , tr("Primary"  )},
   {RecipeAdditionMisc::Use::Secondary, tr("Secondary")},
   {RecipeAdditionMisc::Use::Bottling , tr("Bottling" )}
};

ObjectStore & RecipeAdditionMisc::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeAdditionMisc>::getInstance();
}

bool RecipeAdditionMisc::compareWith(NamedEntity const & other,
                                     QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeAdditionMisc const & rhs = static_cast<RecipeAdditionMisc const &>(other);
   return (
      // Parent classes have to be equal
      this->RecipeAddition    ::compareWith  (rhs, propertiesThatDiffer) &&
      this->RecipeAdditionBase::doCompareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount  ::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const RecipeAdditionMisc::typeLookup {
   "RecipeAdditionMisc",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionMisc, misc, misc),
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionMisc, use , use ),
   },
   // Parent classes lookup.  NB: RecipeAddition not NamedEntity!
   {&RecipeAddition::typeLookup,
    std::addressof(IngredientAmount<RecipeAdditionMisc, Misc>::typeLookup)}
};
static_assert(std::is_base_of<RecipeAddition, RecipeAdditionMisc>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdditionMisc, Misc>, RecipeAdditionMisc>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Misc>);
static_assert(!HasTypeLookup<QString>);


RecipeAdditionMisc::RecipeAdditionMisc(QString name, int const recipeId, int const ingredientId) :
   RecipeAddition{name, recipeId},
   RecipeAdditionBase<RecipeAdditionMisc, Misc>{},
   IngredientAmount<RecipeAdditionMisc, Misc>{ingredientId} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionMisc::RecipeAdditionMisc(NamedParameterBundle const & namedParameterBundle) :
   RecipeAddition{namedParameterBundle},
   RecipeAdditionBase<RecipeAdditionMisc, Misc>{},
   IngredientAmount<RecipeAdditionMisc, Misc>{namedParameterBundle} {
   //
   // As for RecipeAdditionHop: an explicit stage takes precedence; otherwise (eg BeerXML) use the legacy "use" field
   // (Mash, Boil, Primary, Secondary, Bottling) if present; otherwise assume boil, as this is the first stage at which it
   // is usual to add miscs.
   //
   if (namedParameterBundle.contains(PropertyNames::RecipeAddition::stage)) {
      this->m_stage = namedParameterBundle.val<RecipeAddition::Stage>(PropertyNames::RecipeAddition::stage);
   } else if (namedParameterBundle.contains(PropertyNames::RecipeAdditionMisc::use)) {
      auto const stageAndStep = stageAndStepFor(
         namedParameterBundle.val<RecipeAdditionMisc::Use>(PropertyNames::RecipeAdditionMisc::use)
      );
      this->m_stage = stageAndStep.stage;
      if (!this->m_step) {
         this->m_step = stageAndStep.step;
      }
   } else {
      this->m_stage = RecipeAddition::Stage::Boil;
   }

   CONSTRUCTOR_END
   return;
}

RecipeAdditionMisc::RecipeAdditionMisc(RecipeAdditionMisc const & other) :
   RecipeAddition{other},
   RecipeAdditionBase<RecipeAdditionMisc, Misc>{},
   IngredientAmount<RecipeAdditionMisc, Misc>{other} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionMisc::~RecipeAdditionMisc() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
RecipeAdditionMisc::Use  RecipeAdditionMisc::use() const {
   switch (this->stage()) {
      case RecipeAddition::Stage::Mash:
         return RecipeAdditionMisc::Use::Mash;

      case RecipeAddition::Stage::Boil:
         return RecipeAdditionMisc::Use::Boil;

      case RecipeAddition::Stage::Fermentation:
         if (1 == this->step()) {
            return RecipeAdditionMisc::Use::Primary;
         }
         return RecipeAdditionMisc::Use::Secondary;

      case RecipeAddition::Stage::Packaging:
         return RecipeAdditionMisc::Use::Bottling;

      // No default case as we want the compiler to warn us if we missed a case above
   }
   Q_UNREACHABLE(); // We should never get here
}

NamedEntity * RecipeAdditionMisc::ensureExists(BtStringConst const & property) {
   if (property == PropertyNames::RecipeAdditionMisc::misc) {
      // It's a coding error if a RecipeAdditionMisc doesn't have a Misc by the time we're accessing it via the property
      // system.
      Misc * misc = this->misc();
      if (!misc) {
         qCritical() << Q_FUNC_INFO << "No Misc set on RecipeAdditionMisc #" << this->key();
         // Stop here on debug builds
         Q_ASSERT(false);
      }
      return misc;
   }
   // It's a coding error if we're asked to "create" a relational property we don't know about
   qCritical() << Q_FUNC_INFO << "Don't know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeAdditionMisc::setUse(RecipeAdditionMisc::Use const val) {
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

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdditionMisc, Misc)
RECIPE_ADDITION_CODE(RecipeAdditionMisc, Misc, misc)