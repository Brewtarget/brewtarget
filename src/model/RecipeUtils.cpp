/*======================================================================================================================
 * model/RecipeUtils.cpp is part of Brewtarget, and is copyright the following authors 2021-2026:
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
 =====================================================================================================================*/
#include "RecipeUtils.h"

#include <type_traits>

#include "PersistentSettings.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentation.h"
#include "model/Mash.h"

QList<std::shared_ptr<BrewLog>> RecipeUtils::brewLogsForRecipeAndAncestors(Recipe const & recipe) {
   QList<std::shared_ptr<BrewLog>> brewLogs = recipe.brewLogs();
   for (auto ancestor : recipe.ancestors()) {
      brewLogs.append(ancestor->brewLogs());
   }
   return brewLogs;
}

void RecipeUtils::prepareForPropertyChange(NamedEntity & ne, BtStringConst const & propertyName) {

   //
   // If the user has said they don't want versioning, just return
   //
   if (!RecipeUtils::getAutomaticVersioningEnabled()) {
      return;
   }

   qDebug() <<
      Q_FUNC_INFO << "Modifying: " << ne.metaObject()->className() << "#" << ne.key() << "property" << propertyName;

   //
   // A BrewLog belongs to a Recipe, but it is a record of brewing the Recipe rather than part of the Recipe itself.  So
   // filling in a BrewLog (eg with measurements on brew day) is not a change to the Recipe and must not create a new
   // version of it.  (Amongst other things, this used to cause https://github.com/Brewtarget/brewtarget/issues/730.)
   //
   if (qobject_cast<BrewLog const *>(&ne)) {
      return;
   }

   //
   // If the object we're about to change a property on is a Recipe or is used in a Recipe, then it might need a new
   // version -- unless it's already being versioned.
   //
   auto owningRecipe = ne.owningRecipe();
   if (!owningRecipe) {
      return;
   }
   if (owningRecipe->isBeingModified()) {
      // Change is not related to a recipe or the recipe is already being modified
      return;
   }

   //
   // Automatic versioning means that, once a recipe is brewed, it is "soft locked" and the first change should spawn a
   // new version.  Any subsequent change should not spawn a new version until it is brewed again.
   //
   // Because we move the BrewLogs to the snapshot when we version (see below), "has BrewLogs" means "has been brewed
   // since the last time it was versioned".
   //
   if (owningRecipe->brewLogs().empty()) {
      // Recipe hasn't been brewed
      return;
   }

   // If the object we're about to change already has descendants, then we don't want to create new ones.
   if (owningRecipe->hasDescendants()) {
      qDebug() << Q_FUNC_INFO << "Recipe #" << owningRecipe->key() << "already has descendants, so not creating any more";
      return;
   }

   //
   // Once we've started doing versioning, we don't want to trigger it again on the same Recipe until we've finished
   //
   NamedEntityModifyingMarker ownerModifyingMarker(*owningRecipe);

   //
   // Versioning when modifying something in a recipe is *hard*.  If we copy the recipe, there is no easy way to say
   // "this ingredient in the old recipe is that ingredient in the new".  One approach would be to use the delete idea,
   // ie copy everything but what's being modified, clone what's being modified and add the clone to the copy.  Another
   // is to take a deep copy of the Recipe and make that the "prior version".
   //

   // Create a deep copy of the Recipe, and put it in the DB, so it has an ID.
   // (This will also emit signalObjectInserted for the new Recipe from ObjectStoreTyped<Recipe>.)
   qDebug() << Q_FUNC_INFO << "Copying Recipe" << owningRecipe->key();

   // We also don't want to trigger versioning on the newly spawned Recipe until we're completely done here!
   std::shared_ptr<Recipe> spawn = std::make_shared<Recipe>(*owningRecipe);
   NamedEntityModifyingMarker spawnModifyingMarker(*spawn);
   ObjectStoreWrapper::insert(spawn);

   qDebug() << Q_FUNC_INFO << "Copied Recipe #" << owningRecipe->key() << "to new Recipe #" << spawn->key();

   // We assert that the newly created version of the recipe has not yet been brewed (and therefore will not get
   // automatically versioned on subsequent changes before it is brewed).
   Q_ASSERT(spawn->brewLogs().empty());

   //
   // The BrewLogs are records of brewing the Recipe as it was before the change we're about to make, so they belong
   // with the snapshot (which is what was actually brewed) rather than with the Recipe that's about to change.  This
   // also means the Recipe now has no BrewLogs, so the check above stops further changes spawning more versions until
   // the Recipe is brewed again.
   //
   // (Changing the BrewLog's Recipe ID won't trigger versioning, because of the check for BrewLog above, and anyway
   // owningRecipe is marked as being modified.)
   //
   for (auto const & brewLog : owningRecipe->brewLogs()) {
      qDebug() << Q_FUNC_INFO << "Moving" << *brewLog << "to Recipe #" << spawn->key();
      brewLog->setRecipeId(spawn->key());
   }
   Q_ASSERT(owningRecipe->brewLogs().empty());

   //
   // By default, copying a Recipe does not copy all its ancestry.  Here, we want the copy to become our ancestor (ie
   // previous version).  This will also emit a signalPropertyChanged from ObjectStoreTyped<Recipe>, which the UI can
   // pick up to update tree display of Recipes etc.
   //
   owningRecipe->setAncestor(*spawn);

   return;
}

/**
 * \brief Turn automatic versioning on or off
 */
void RecipeUtils::setAutomaticVersioningEnabled(bool enabled) {
   PersistentSettings::insert_ck(PersistentSettings::Names::versioning, enabled);
   return;
}

/**
 * \brief Returns \c true if automatic versioning is enabled, \c false otherwise
 */
bool RecipeUtils::getAutomaticVersioningEnabled() {
   return PersistentSettings::value_ck(PersistentSettings::Names::versioning, false).toBool();
}

RecipeUtils::SuspendRecipeVersioning::SuspendRecipeVersioning() {
   this->savedVersioningValue = RecipeUtils::getAutomaticVersioningEnabled();
   if (this->savedVersioningValue) {
      qDebug() << Q_FUNC_INFO << "Temporarily suspending automatic Recipe versioning";
      RecipeUtils::setAutomaticVersioningEnabled(false);
   }
   return;
}
RecipeUtils::SuspendRecipeVersioning::~SuspendRecipeVersioning() {
   if (this->savedVersioningValue) {
      qDebug() << Q_FUNC_INFO << "Re-enabling automatic Recipe versioning";
      RecipeUtils::setAutomaticVersioningEnabled(true);
   }
   return;
}

RecipeUtils::SuspendRecipeCalcs::SuspendRecipeCalcs(std::shared_ptr<Recipe> recipe) :
   m_recipe{recipe},
   m_savedCalcsEnabled{recipe->calcsEnabled()} {
   if (this->m_savedCalcsEnabled) {
      this->m_recipe->setCalcsEnabled(false);
   }
   return;
}
RecipeUtils::SuspendRecipeCalcs::~SuspendRecipeCalcs() {
   this->m_recipe->setCalcsEnabled(this->m_savedCalcsEnabled);
   return;
}

template<class NE> QList<Recipe *> RecipeUtils::recipesUsing(NE const & item) {
   return ObjectStoreWrapper::findAllMatching<Recipe>(
      [&item](Recipe const * recipe) { return !recipe->deleted() && recipe->uses(item); }
   );
}

template<class NE> RecipeUtils::LockReason RecipeUtils::lockReason(NE const & item) {
   if (item.locked()) {
      return RecipeUtils::LockReason::ItemLocked;
   }
   for (Recipe const * recipe : RecipeUtils::recipesUsing(item)) {
      if (recipe->locked()) {
         return RecipeUtils::LockReason::UsedByLockedRecipe;
      }
   }
   return RecipeUtils::LockReason::NotLocked;
}

template<class NE> std::shared_ptr<NE> RecipeUtils::useCopyFor(Recipe & recipe) {
   std::shared_ptr<NE> const original = recipe.get<NE>();
   // It's a coding error to call this function if the recipe has nothing to copy or cannot be changed
   Q_ASSERT(original);
   Q_ASSERT(!recipe.locked());

   std::shared_ptr<NE> copy = ObjectStoreWrapper::insertCopyOf(*original);
   copy->setName(NE::tr("%1 (copy for %2)").arg(original->name(), recipe.name()));
   qInfo() << Q_FUNC_INFO << "Recipe #" << recipe.key() << "now using" << *copy << "instead of" << *original;
   recipe.set<NE>(copy);
   return copy;
}

template<class NE> QString RecipeUtils::describeSharingAndLocking(NE const & item, Recipe const * currentRecipe) {
   QList<Recipe *> const recipes = RecipeUtils::recipesUsing(item);

   QStringList otherRecipeNames;
   QStringList otherLockedRecipeNames;
   bool usedByCurrentRecipe = false;
   for (Recipe const * recipe : recipes) {
      if (recipe == currentRecipe) {
         usedByCurrentRecipe = true;
      } else {
         otherRecipeNames.append(recipe->name());
         if (recipe->locked()) {
            otherLockedRecipeNames.append(recipe->name());
         }
      }
   }
   otherRecipeNames.sort(Qt::CaseInsensitive);
   otherLockedRecipeNames.sort(Qt::CaseInsensitive);
   bool const currentRecipeLocked = usedByCurrentRecipe && currentRecipe->locked();

   //
   // First say who uses the item
   //
   QString const itemType = NE::localisedName().toLower();
   QStringList sentences;
   if (recipes.isEmpty()) {
      sentences.append(NE::tr("This %1 is not used by any recipe.").arg(itemType));
   } else if (recipes.size() == 1) {
      sentences.append(NE::tr("This %1 is used only by one recipe (%2).").arg(itemType, recipes.first()->name()));
      if constexpr (std::is_same_v<NE, Mash>) {
         // See MashStepTableModel::validate
         sentences.append(NE::tr("Step validation highlighting is therefore enabled."));
      }
   } else if (usedByCurrentRecipe) {
      sentences.append(
         NE::tr("This %1 is also used by %n other recipe(s): %2.", "", otherRecipeNames.size()).arg(
            itemType, otherRecipeNames.join(", ")
         )
      );
   } else {
      sentences.append(
         NE::tr("This %1 is used by %n recipe(s): %2.", "", otherRecipeNames.size()).arg(
            itemType, otherRecipeNames.join(", ")
         )
      );
   }

   //
   // Then say whether it can be changed
   //
   if (item.locked()) {
      sentences.append(NE::tr("It is locked, so it cannot be changed."));
   } else if (currentRecipeLocked) {
      sentences.append(NE::tr("It cannot be changed because the recipe \"%1\" is locked.").arg(currentRecipe->name()));
   } else if (!otherLockedRecipeNames.isEmpty()) {
      sentences.append(
         NE::tr("It cannot be changed because it is used by %n locked recipe(s): %1.", "", otherLockedRecipeNames.size()).arg(
            otherLockedRecipeNames.join(", ")
         )
      );
   } else if (recipes.size() > 1) {
      sentences.append(NE::tr("Any changes you make here will affect all of them."));
   }

   return sentences.join(" ");
}

//
// Instantiate the above templates for the types that are going to use them
//
template QList<Recipe *> RecipeUtils::recipesUsing<Mash        >(Mash         const & item);
template QList<Recipe *> RecipeUtils::recipesUsing<Boil        >(Boil         const & item);
template QList<Recipe *> RecipeUtils::recipesUsing<Fermentation>(Fermentation const & item);
template QList<Recipe *> RecipeUtils::recipesUsing<Equipment   >(Equipment    const & item);
template RecipeUtils::LockReason RecipeUtils::lockReason<Mash        >(Mash         const & item);
template RecipeUtils::LockReason RecipeUtils::lockReason<Boil        >(Boil         const & item);
template RecipeUtils::LockReason RecipeUtils::lockReason<Fermentation>(Fermentation const & item);
template std::shared_ptr<Mash        > RecipeUtils::useCopyFor<Mash        >(Recipe & recipe);
template std::shared_ptr<Boil        > RecipeUtils::useCopyFor<Boil        >(Recipe & recipe);
template std::shared_ptr<Fermentation> RecipeUtils::useCopyFor<Fermentation>(Recipe & recipe);
template QString RecipeUtils::describeSharingAndLocking<Mash        >(Mash         const & item, Recipe const * currentRecipe);
template QString RecipeUtils::describeSharingAndLocking<Boil        >(Boil         const & item, Recipe const * currentRecipe);
template QString RecipeUtils::describeSharingAndLocking<Fermentation>(Fermentation const & item, Recipe const * currentRecipe);
