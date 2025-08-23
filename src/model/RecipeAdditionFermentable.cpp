/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionFermentable.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "model/RecipeAdditionFermentable.h"

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Boil.h"
#include "model/BoilStep.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdditionFermentable.cpp"
#endif

QString RecipeAdditionFermentable::localisedName() { return tr("Fermentable Addition"); }
QString RecipeAdditionFermentable::localisedName_fermentable() { return tr("Fermentable"); }

QString RecipeAdditionFermentable::instanceNameTemplate() { return tr("Addition of %1 fermentable"); }

ObjectStore & RecipeAdditionFermentable::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeAdditionFermentable>::getInstance();
}

TypeLookup const RecipeAdditionFermentable::typeLookup {
   "RecipeAdditionFermentable",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdditionFermentable, fermentable, fermentable),
   },
   // Parent classes lookup.  NB: RecipeAddition not NamedEntity!
   {&RecipeAddition::typeLookup,
    std::addressof(IngredientAmount<RecipeAdditionFermentable, Fermentable>::typeLookup)}
};
static_assert(std::is_base_of<RecipeAddition, RecipeAdditionFermentable>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdditionFermentable, Fermentable>, RecipeAdditionFermentable>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Fermentable>);
static_assert(!HasTypeLookup<QString>);

RecipeAdditionFermentable::RecipeAdditionFermentable(QString name, int const recipeId, int const ingredientId) :
   RecipeAddition{name, recipeId, ingredientId},
   RecipeAdditionBase<RecipeAdditionFermentable, Fermentable>{},
   IngredientAmount<RecipeAdditionFermentable, Fermentable>{} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionFermentable::RecipeAdditionFermentable(NamedParameterBundle const & namedParameterBundle) :
   RecipeAddition{namedParameterBundle},
   RecipeAdditionBase<RecipeAdditionFermentable, Fermentable>{},
   IngredientAmount<RecipeAdditionFermentable, Fermentable>{namedParameterBundle} {
   //
   // If the addition stage is not specified then we assume it is boil, as this is the first stage at which it is usual
   // to add hops.
   //
   m_stage = namedParameterBundle.val<RecipeAddition::Stage>(PropertyNames::RecipeAddition::stage,
                                                             RecipeAddition::Stage::Boil);

   CONSTRUCTOR_END
   return;
}

RecipeAdditionFermentable::RecipeAdditionFermentable(RecipeAdditionFermentable const & other) :
   RecipeAddition{other},
   RecipeAdditionBase<RecipeAdditionFermentable, Fermentable>{},
   IngredientAmount<RecipeAdditionFermentable, Fermentable>{other} {

   CONSTRUCTOR_END
   return;
}

RecipeAdditionFermentable::~RecipeAdditionFermentable() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
NamedEntity * RecipeAdditionFermentable::ensureExists(BtStringConst const & property) {
   if (property == PropertyNames::RecipeAdditionFermentable::fermentable) {
      // It's a coding error if a RecipeAdditionFermentable doesn't have a Fermentable by the time we're accessing it via the property
      // system.
      Fermentable * fermentable = this->fermentable();
      if (!fermentable) {
         qCritical() << Q_FUNC_INFO << "No Fermentable set on RecipeAdditionFermentable #" << this->key();
         // Stop here on debug builds
         Q_ASSERT(false);
      }
      return fermentable;
   }
   // It's a coding error if we're asked to "create" a relational property we don't know about
   qCritical() << Q_FUNC_INFO << "Don't know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
bool RecipeAdditionFermentable::addAfterBoil() const {
   if (this->stage() != RecipeAddition::Stage::Boil) {
      return false;
   }
   //
   // For the moment, we assume a boil has minimum 3 stages (ramp-up, boil proper and one or more cool-down phases).
   //
   if (this->step() <= 2) {
      return false;
   }
   return true;
}

double RecipeAdditionFermentable::equivSucrose_kg() const {
   // .:TBD:. Not clear what we should return (or whether we should even be called) if amount is a volume
   auto const & amount = this->amount();
   if (amount.unit != &Measurement::Units::kilograms) {
      qWarning() << Q_FUNC_INFO << "Trying to calculate equivSucrose_kg for Fermentable amount of" << amount;
   }
   auto const & fermentable = this->fermentable();
   double const ret =
      amount.quantity * fermentable->fineGrindYield_pct().value_or(0.0) *
      (1.0 - fermentable->moisture_pct().value_or(0.0) / 100.0) / 100.0;

   // If this is a steeped grain...
   if (fermentable->type() == Fermentable::Type::Grain && this->stage() != RecipeAddition::Stage::Mash) {
      return 0.60 * ret; // Reduce the yield by 60%.
   }
   return ret;
}

// We rely on this being true in serialization/NamedEntityRecordBase.h, so let's check it at compile-time here
static_assert(std::is_base_of<OwnedByRecipe, RecipeAdditionFermentable>::value);

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdditionFermentable, Fermentable)
RECIPE_ADDITION_CODE(RecipeAdditionFermentable, Fermentable, fermentable)
