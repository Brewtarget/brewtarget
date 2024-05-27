/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdjustmentSalt.h is part of Brewtarget, and is copyright the following authors 2024:
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
#ifndef MODEL_RECIPEADJUSTMENTSALT_H
#define MODEL_RECIPEADJUSTMENTSALT_H
#pragma once

#include <memory>

#include "model/IngredientAmount.h"
#include "model/IngredientInRecipe.h"
#include "model/RecipeAdditionBase.h"
#include "model/Recipe.h"
#include "model/Salt.h"

//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeAdjustmentSalt { BtStringConst const property{#property}; }
AddPropertyName(salt     )
AddPropertyName(whenToAdd)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌

/**
 * \brief Represents the addition of a \c Salt to a \c Recipe to adjust a water profile.
 *
 *        Salts, and the timing of their additions, are different from other ingredients, so we do not inherit from
 *        \c RecipeAddition, though, as with \c RecipeUseOfWater, we do try to align with it where that makes sense and
 *        reduces code duplication.
 *
 *        In BeerXML and BeerJSON, salt additions are not part of the recipe.  Arguably they are an adaptation to allow
 *        a recipe to be brewed from a different water profile, thus two different brewers brewing the same beer in
 *        different locations might need different salt additions.
 */
class RecipeAdjustmentSalt : public IngredientInRecipe,
                             public RecipeAdditionBase<RecipeAdjustmentSalt, Salt>,
                             public IngredientAmount<RecipeAdjustmentSalt, Salt> {
   Q_OBJECT

   // This allows RecipeAdditionBase to call protected and private members of RecipeAdjustmentSalt
   friend class RecipeAdditionBase<RecipeAdjustmentSalt, Salt>;

   INGREDIENT_AMOUNT_DECL(RecipeAdjustmentSalt, Salt)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   enum class WhenToAdd {
      Mash  ,
      Sparge,
      Ratio ,
      Equal ,
   };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(WhenToAdd)

   static EnumStringMapping const whenToAddStringMapping;
   static EnumStringMapping const whenToAddDisplayNames;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   RecipeAdjustmentSalt(QString name = "", int const recipeId = -1, int const saltId = -1);
   RecipeAdjustmentSalt(NamedParameterBundle const & namedParameterBundle);
   RecipeAdjustmentSalt(RecipeAdjustmentSalt const & other);

   virtual ~RecipeAdjustmentSalt();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(Salt * salt   READ salt   WRITE setSalt             )

   // See model/IngredientAmount.h
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)

   /**
    * \brief The \c Use.  This is moved from \c Salt.  It is retained for our non-standard BeerXML extensions, but
    *        \b deprecated for other use.
    */
   //! \brief When to add the salt (mash or sparge)
   Q_PROPERTY(WhenToAdd whenToAdd      READ whenToAdd      WRITE setWhenToAdd       )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Salt *               salt          () const;
   RecipeAdjustmentSalt::WhenToAdd whenToAdd()      const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setSalt          (Salt *               const val);
   void setWhenToAdd     (RecipeAdjustmentSalt::WhenToAdd val);

   virtual NamedEntity * ensureExists(BtStringConst const & property);

protected:
   // Note that we don't override isEqualTo, as we don't have any non-inherited member variables
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   WhenToAdd m_whenToAdd;
};

Q_DECLARE_METATYPE(Salt)
Q_DECLARE_METATYPE(Salt *)
BT_DECLARE_METATYPES(RecipeAdjustmentSalt)

#endif
