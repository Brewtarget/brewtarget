/*======================================================================================================================
 * model/RecipeUtils.h is part of Brewtarget, and is copyright the following authors 2021-2026:
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
#ifndef MODEL_RECIPEUTILS_H
#define MODEL_RECIPEUTILS_H
#pragma once

#include <QList>

#include "model/BrewLog.h"
#include "model/Recipe.h"

/**
 * \brief Non-member functions for \c Recipe
 */
namespace RecipeUtils {
   /**
    * \brief Gets the BrewLogs for a Recipe and all its ancestors
    */
   QList<std::shared_ptr<BrewLog>> brewLogsForRecipeAndAncestors(Recipe const & recipe);

   /**
    * \brief Turn automatic versioning on or off
    */
   void setAutomaticVersioningEnabled(bool enabled);

   /**
    * \brief Returns \c true if automatic versioning is enabled, \c false otherwise
    */
   bool getAutomaticVersioningEnabled();

   /**
    * \brief Checks whether an about-to-be-made property change require us to create a new version of a Recipe - eg
    *        because we are modifying some ingredient or other attribute of the Recipe and automatic versioning is
    *        enabled.
    */
   void prepareForPropertyChange(NamedEntity & ne, BtStringConst const & propertyName);

   /**
    * \brief Mini RAII class that allows automatic Recipe versioning to be suspended for the time that it's in scope
    */
   class SuspendRecipeVersioning {
   public:
      SuspendRecipeVersioning();
      ~SuspendRecipeVersioning();
   private:
      bool savedVersioningValue;
      // RAII class shouldn't be getting copied or moved
      SuspendRecipeVersioning(SuspendRecipeVersioning const &) = delete;
      SuspendRecipeVersioning & operator=(SuspendRecipeVersioning const &) = delete;
      SuspendRecipeVersioning(SuspendRecipeVersioning &&) = delete;
      SuspendRecipeVersioning & operator=(SuspendRecipeVersioning &&) = delete;
   };

   /**
    * \brief Mini RAII class that turns off a Recipe's calculations for the time that it's in scope.
    *
    *        This is used when reading a Recipe in from a file (BeerXML, BeerJSON, dotBeer): the calculations are not
    *        meaningful until all the Recipe's child objects (ingredient additions, mash, boil, etc) have been stored
    *        and linked to it.  Worse, if they run before then (eg because storing the Recipe in the database reads its
    *        calculated properties), the results get cached as if they were valid, so the Recipe would show (and
    *        export) the OG, IBU etc of a recipe with no ingredients.  Whilst calculations are disabled,
    *        \c Recipe::recalcAll does nothing, so the calculated values stay marked as uninitialised and are worked out
    *        properly the first time they are needed.
    */
   class SuspendRecipeCalcs {
   public:
      SuspendRecipeCalcs(std::shared_ptr<Recipe> recipe);
      ~SuspendRecipeCalcs();
   private:
      std::shared_ptr<Recipe> m_recipe;
      bool const m_savedCalcsEnabled;
      // RAII class shouldn't be getting copied or moved
      SuspendRecipeCalcs(SuspendRecipeCalcs const &) = delete;
      SuspendRecipeCalcs & operator=(SuspendRecipeCalcs const &) = delete;
      SuspendRecipeCalcs(SuspendRecipeCalcs &&) = delete;
      SuspendRecipeCalcs & operator=(SuspendRecipeCalcs &&) = delete;
   };

   //===================================================================================================================
   // The following functions are for things that can be shared between Recipes (\c Mash, \c Boil, \c Fermentation),
   // to help the UI make clear when changing something will affect more than one Recipe, and to protect things that
   // should not be changed (see model/LockablePropertyBase.h).
   //
   // Implementations are in the .cpp file, with explicit instantiations for Mash, Boil and Fermentation.
   //===================================================================================================================

   /**
    * \return The (non-deleted) Recipes that use \c item.  Order is not significant.
    */
   template<class NE> QList<Recipe *> recipesUsing(NE const & item);

   /**
    * \brief Why, if at all, the user should not be able to change something
    */
   enum class LockReason {
      //! The item can be changed (although, if it is shared, the user should be told that the change will affect all
      //! the recipes that use it)
      NotLocked,
      //! The item's own "locked" flag is set
      ItemLocked,
      //! The item is not itself locked, but is used by at least one locked Recipe -- eg a Recipe that has been brewed
      //! and then had a new version made.  Changing the item would change what the locked Recipe records.
      UsedByLockedRecipe,
   };

   template<class NE> LockReason lockReason(NE const & item);

   /**
    * \brief Make a copy of \c recipe's \c NE (ie \c Mash, \c Boil or \c Fermentation), and set \c recipe to use the
    *        copy rather than the original.  The copy is not locked (see \c LockablePropertyBase copy constructor), and
    *        its name says what it is a copy for.
    *
    *        It is a coding error to call this if \c recipe does not have an \c NE or if \c recipe is locked.
    *
    * \return The copy
    */
   template<class NE> std::shared_ptr<NE> useCopyFor(Recipe & recipe);

   /**
    * \return Text describing which recipes use \c item and whether it can be changed, suitable for showing to the user
    *         (eg in an editor).
    *
    * \param currentRecipe The Recipe from which the user is looking at \c item, if any.  Used to say "this recipe and
    *                      2 others" rather than "3 recipes", and to omit this recipe from the list of names.
    */
   template<class NE> QString describeSharingAndLocking(NE const & item, Recipe const * currentRecipe = nullptr);

}

#endif