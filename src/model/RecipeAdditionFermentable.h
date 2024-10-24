/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionFermentable.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_RECIPEADDITIONFERMENTABLE_H
#define MODEL_RECIPEADDITIONFERMENTABLE_H
#pragma once

#include <memory>

#include "model/Fermentable.h"
#include "model/IngredientAmount.h"
#include "model/RecipeAddition.h"
#include "model/RecipeAdditionBase.h"
#include "model/Recipe.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeAdditionFermentable { BtStringConst const property{#property}; }
AddPropertyName(fermentable)
AddPropertyName(use) // Deprecated - retained only for BeerXML

#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Represents the addition of a \c Fermentable to a \c Recipe
 */
class RecipeAdditionFermentable : public RecipeAddition,
                                  public RecipeAdditionBase<RecipeAdditionFermentable, Fermentable>,
                                  public IngredientAmount<RecipeAdditionFermentable, Fermentable> {
   Q_OBJECT

   RECIPE_ADDITION_DECL(RecipeAdditionFermentable, Fermentable)

   INGREDIENT_AMOUNT_DECL(RecipeAdditionFermentable, Fermentable)

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
   TYPE_LOOKUP_GETTER

   RecipeAdditionFermentable(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   RecipeAdditionFermentable(NamedParameterBundle const & namedParameterBundle);
   RecipeAdditionFermentable(RecipeAdditionFermentable const & other);

   virtual ~RecipeAdditionFermentable();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(Fermentable * fermentable   READ fermentable   WRITE setFermentable             )

   // See model/IngredientAmount.h
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Fermentable * fermentable () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setFermentable(Fermentable * const val);

   //! If something is added at the end of the boil, this will return \c true
   bool addAfterBoil() const;

   // We don't provide isMashed() as the replacement is simply `this->stage() == RecipeAddition::Stage::Mash`

   //! \brief The maximum kg of equivalent glucose that will come from this Fermentable addition.
   double equivSucrose_kg() const;

   virtual NamedEntity * ensureExists(BtStringConst const & property) override;

protected:
   // Note that we don't override isEqualTo, as we don't have any non-inherited member variables
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

};

Q_DECLARE_METATYPE(Fermentable)
Q_DECLARE_METATYPE(Fermentable *)
BT_DECLARE_METATYPES(RecipeAdditionFermentable)

#endif
