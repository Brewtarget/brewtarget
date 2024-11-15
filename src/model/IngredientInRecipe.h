/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/IngredientInRecipe.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef MODEL_INGREDIENTINRECIPE_H
#define MODEL_INGREDIENTINRECIPE_H
#pragma once

#include "model/OwnedByRecipe.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::IngredientInRecipe { inline BtStringConst const property{#property}; }
AddPropertyName(ingredientId   )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief This is a "lite" version of \c RecipeAddition that serves as a common base to \c RecipeAddition,
 *        \c RecipeAdjustmentSalt and \c RecipeUseOfWater
 */
class IngredientInRecipe : public OwnedByRecipe {
   Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   IngredientInRecipe(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   IngredientInRecipe(NamedParameterBundle const & namedParameterBundle);
   IngredientInRecipe(IngredientInRecipe const & other);

   virtual ~IngredientInRecipe();

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The ID of the ingredient (ie \c Hop, \c Fermentable, \c Misc or \c Yeast) or \c Salt or \c Water being
    *        added.
    *
    *        Strictly, water isn't quite the same as other ingredients, but calling it an ingredient here allows us to
    *        minimise code duplication in \c Recipe
    */
   Q_PROPERTY(int ingredientId READ ingredientId WRITE setIngredientId)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   int ingredientId() const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setIngredientId(int const val);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;

protected:
   int m_ingredientId;

};

#endif
