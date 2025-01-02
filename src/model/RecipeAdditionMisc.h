/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionMisc.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_RECIPEADDITIONMISC_H
#define MODEL_RECIPEADDITIONMISC_H
#pragma once

#include <memory>

#include "model/IngredientAmount.h"
#include "model/Misc.h"
#include "model/RecipeAddition.h"
#include "model/RecipeAdditionBase.h"
#include "model/Recipe.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeAdditionMisc { inline BtStringConst const property{#property}; }
AddPropertyName(misc)
AddPropertyName(use) // Deprecated - retained only for BeerXML

#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/**
 * \brief Represents the addition of a \c Misc to a \c Recipe
 */
class RecipeAdditionMisc : public RecipeAddition,
                           public RecipeAdditionBase<RecipeAdditionMisc, Misc>,
                           public IngredientAmount<RecipeAdditionMisc, Misc> {
   Q_OBJECT

   RECIPE_ADDITION_DECL(RecipeAdditionMisc, Misc)

   INGREDIENT_AMOUNT_DECL(RecipeAdditionMisc, Misc)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   /*!
    * \brief This is the old (BeerXML) way of specifying the stage at which the misc addition happens.  It is retained
    *        for BeerXML but otherwise deprecated.  With the arrival of BeerJSON, we now use \c RecipeAddition::Stage
    *        and \c RecipeAddition::Step to hold this information with more consistency and precision.
    *
    *        See also \c RecipeAdditionHop::Use.
    */
   enum class Use {Mash     ,
                   Boil     ,
                   Primary  ,
                   Secondary,
                   Bottling ,};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Use)

   /*!
    * \brief Mapping between \c RecipeAdditionMisc::Use and string values suitable for serialisation in DB, BeerXML, etc
    *        (but \b not used in BeerJSON)
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   static EnumStringMapping const useStringMapping;

   /*!
    * \brief Localised names of \c RecipeAdditionMisc::Use values suitable for displaying to the end user
    */
   static EnumStringMapping const useDisplayNames;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   RecipeAdditionMisc(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   RecipeAdditionMisc(NamedParameterBundle const & namedParameterBundle);
   RecipeAdditionMisc(RecipeAdditionMisc const & other);

   virtual ~RecipeAdditionMisc();

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief The \c Use.  This is moved from \c Misc with the introduction of BeerJSON.  It is required for BeerXML, but
    *        \b deprecated for other use.
    */
   Q_PROPERTY(Use   use   READ use   WRITE setUse STORED false)
   Q_PROPERTY(Misc * misc   READ misc   WRITE setMisc             )

   // See model/IngredientAmount.h
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   // Ideally this too would be marked [[deprecated]], but we do need to refer to it in RecipeAdditionMisc::typeLookup
   Use use() const;
   Misc * misc () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   [[deprecated]] void setUse(Use const val);
   void setMisc(Misc * const val);

   virtual NamedEntity * ensureExists(BtStringConst const & property) override;

protected:
   // Note that we don't override isEqualTo, as we don't have any non-inherited member variables
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

};

BT_DECLARE_METATYPES(RecipeAdditionMisc)

#endif
