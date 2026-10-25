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

}

#endif