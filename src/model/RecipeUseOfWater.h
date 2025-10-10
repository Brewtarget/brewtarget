/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeUseOfWater.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef MODEL_RECIPEUSEOFWATER_H
#define MODEL_RECIPEUSEOFWATER_H
#pragma once

#include <QString>

#include "model/IngredientAmount.h"
#include "model/OwnedByRecipe.h"
#include "model/RecipeAdditionBase.h"
#include "model/Water.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeUseOfWater { inline BtStringConst const property{#property}; }
AddPropertyName(recipeId)
AddPropertyName(water   )
AddPropertyName(volume_l)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Records the amount of \c Water used in a \c Recipe
 *
 *        This has some similarities with \c RecipeAddition and its derived classes, but rather less information is
 *        stored for water additions, so it's a separate class.  Also, \c Water is \b not an \c Ingredient (because we
 *        do not hold inventory of it).
 *
 *        We could almost have done without this class and just had a \c Recipe directly refer to the \c Water it uses.
 *        However, \b technically, both BeerJSON and BeerXML allow for multiple different waters to be added to a
 *        recipe, so we align with that.
 */
class RecipeUseOfWater : public OwnedByRecipe,
                         public RecipeAdditionBase<RecipeUseOfWater, Water>,
                         public IngredientAmount<RecipeUseOfWater, Water> {
   Q_OBJECT

   RECIPE_ADDITION_DECL(RecipeUseOfWater, Water, water)

   INGREDIENT_AMOUNT_DECL(RecipeUseOfWater, Water)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_recipeId();
   static QString localisedName_water   ();
   static QString localisedName_volume_l();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   RecipeUseOfWater(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   RecipeUseOfWater(NamedParameterBundle const & namedParameterBundle);
   RecipeUseOfWater(RecipeUseOfWater const & other);

   virtual ~RecipeUseOfWater();

   //=================================================== PROPERTIES ====================================================
   //! See \c RecipeAdditionBase for getter and setter
   Q_PROPERTY(Water * water   READ water   WRITE setWater)

   // See model/IngredientAmount.h
   // Yes, it's a bit overkill to use IngredientAmount when we know water will always be volume, but it makes other
   // things easier to align with the other subclasses of RecipeAdditionBase
   Q_PROPERTY(int                           ingredientId READ ingredientId WRITE setIngredientId)
   Q_PROPERTY(Measurement::Amount           amount       READ amount       WRITE setAmount      )
   Q_PROPERTY(double                        quantity     READ quantity     WRITE setQuantity    )
   Q_PROPERTY(Measurement::Unit const *     unit         READ unit         WRITE setUnit        )
   Q_PROPERTY(Measurement::PhysicalQuantity measure      READ measure      WRITE setMeasure     )
   Q_PROPERTY(bool                          isWeight     READ isWeight     WRITE setIsWeight    )

   /**
    * \brief The volume of water being used, in liters.
    */
   Q_PROPERTY(int volume_l READ volume_l WRITE setVolume_l)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   double  volume_l    () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setVolume_l    (double  const val);

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

};

BT_DECLARE_METATYPES(RecipeUseOfWater)

#endif
