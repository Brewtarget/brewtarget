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

#include "model/BrewNote.h"
#include "model/Recipe.h"

/**
 * \brief Non-member functions for \c Recipe
 */
namespace RecipeUtils {
   /**
    * \brief Gets the BrewNotes for a Recipe and all its ancestors
    */
   QList<std::shared_ptr<BrewNote>> brewNotesForRecipeAndAncestors(Recipe const & recipe);

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

}

#endif
