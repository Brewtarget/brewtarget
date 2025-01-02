/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeUseOfWater.h is part of Brewtarget, and is copyright the following authors 2024:
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

#include "model/IngredientInRecipe.h"
#include "model/RecipeAdditionBase.h"
#include "model/Water.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeUseOfWater { inline BtStringConst const property{#property}; }
AddPropertyName(recipeId    )
AddPropertyName(water       )
AddPropertyName(volume_l    )
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
class RecipeUseOfWater : public IngredientInRecipe,
                         public RecipeAdditionBase<RecipeUseOfWater, Water> {
   Q_OBJECT

   // This allows RecipeAdditionBase to call protected and private members of RecipeUseOfWater
   friend class RecipeAdditionBase<RecipeUseOfWater, Water>;

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

   RecipeUseOfWater(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   RecipeUseOfWater(NamedParameterBundle const & namedParameterBundle);
   RecipeUseOfWater(RecipeUseOfWater const & other);

   virtual ~RecipeUseOfWater();

   //=================================================== PROPERTIES ====================================================

   Q_PROPERTY(Water * water   READ water   WRITE setWater)

   /**
    * \brief The volume of water being used, in liters.
    */
   Q_PROPERTY(int volume_l READ volume_l WRITE setVolume_l)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Water * water       () const;
   double  volume_l    () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setWater       (Water * const val);
   void setVolume_l    (double  const val);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const override;
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

protected:
   double m_volume_l;

};

BT_DECLARE_METATYPES(RecipeUseOfWater)

#endif
