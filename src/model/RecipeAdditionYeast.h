/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionYeast.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#ifndef MODEL_RECIPEADDITIONYEAST_H
#define MODEL_RECIPEADDITIONYEAST_H
#pragma once

#include <memory>

#include "model/IngredientAmount.h"
#include "model/RecipeAddition.h"
#include "model/RecipeAdditionBase.h"
#include "model/Recipe.h"
#include "model/Yeast.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::RecipeAdditionYeast { inline BtStringConst const property{#property}; }
AddPropertyName(addToSecondary   )  // Deprecated - retained only for BeerXML
AddPropertyName(attenuation_pct  )
AddPropertyName(yeast            )
AddPropertyName(timesCultured    )
AddPropertyName(cellCountBillions)

#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

class RecipeAdditionYeastTableModel;
class RecipeAdditionYeastSortFilterProxyModel;
class RecipeAdditionYeastItemDelegate;

/**
 * \brief Represents the addition of a \c Yeast to a \c Recipe
 */
class RecipeAdditionYeast : public RecipeAddition,
                            public RecipeAdditionBase<RecipeAdditionYeast, Yeast>,
                            public IngredientAmount<RecipeAdditionYeast, Yeast> {
   Q_OBJECT

   RECIPE_ADDITION_DECL(RecipeAdditionYeast, Yeast)

   INGREDIENT_AMOUNT_DECL(RecipeAdditionYeast, Yeast)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();

   //
   // These aliases make it easier to template a number of functions that are essentially the same for a number of
   // different RecipeAddition etc subclasses.
   //
   using TableModelClass           = RecipeAdditionYeastTableModel;
   using SortFilterProxyModelClass = RecipeAdditionYeastSortFilterProxyModel;
   using ItemDelegateClass         = RecipeAdditionYeastItemDelegate;

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;
   TYPE_LOOKUP_GETTER

   RecipeAdditionYeast(QString name = "", int const recipeId = -1, int const ingredientId = -1);
   RecipeAdditionYeast(NamedParameterBundle const & namedParameterBundle);
   RecipeAdditionYeast(RecipeAdditionYeast const & other);

   virtual ~RecipeAdditionYeast();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(Yeast * yeast   READ yeast   WRITE setYeast             )

   // See model/IngredientAmount.h
   Q_PROPERTY(Measurement::Amount           amount    READ amount     WRITE setAmount  )
   Q_PROPERTY(double                        quantity  READ quantity   WRITE setQuantity)
   Q_PROPERTY(Measurement::Unit const *     unit      READ unit       WRITE setUnit    )
   Q_PROPERTY(Measurement::PhysicalQuantity measure   READ measure    WRITE setMeasure )
   Q_PROPERTY(bool                          isWeight  READ isWeight   WRITE setIsWeight)

   //! \brief The apparent attenuation in percent (moved from \c Yeast).  ⮜⮜⮜ Optional in BeerJSON and BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<double>  attenuation_pct           READ attenuation_pct           WRITE setAttenuation_pct  )
   //! \brief The number of times recultured (moved from \c Yeast).       ⮜⮜⮜ Optional in BeerXML and BeerJSON ⮞⮞⮞
   Q_PROPERTY(std::optional<int>     timesCultured             READ timesCultured             WRITE setTimesCultured    )
   //! \brief A more fundamental way of measuring how much yeast is being added.  ⮜⮜⮜ Optional in BeerJSON, not part of BeerXML ⮞⮞⮞
   Q_PROPERTY(std::optional<int>     cellCountBillions         READ cellCountBillions         WRITE setCellCountBillions)

   //! \brief Whether the yeast is added to secondary or primary.  ⮜⮜⮜ Optional in BeerXML, not part of BeerJSON ⮞⮞⮞
   Q_PROPERTY(std::optional<bool>    addToSecondary            READ addToSecondary            WRITE setAddToSecondary   )

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   Yeast *               yeast            () const;
   std::optional<double> attenuation_pct  () const;
   std::optional<int   > timesCultured    () const;
   std::optional<int   > cellCountBillions() const;
   std::optional<bool  > addToSecondary   () const;

   //============================================ "SETTER" MEMBER FUNCTIONS ============================================
   void setYeast            (Yeast *               const val);
   void setAttenuation_pct  (std::optional<double> const val);
   void setTimesCultured    (std::optional<int   > const val);
   void setCellCountBillions(std::optional<int   > const val);
   void setAddToSecondary   (std::optional<bool  > const val);

   virtual NamedEntity * ensureExists(BtStringConst const & property) override;

protected:
   // Note that we don't override isEqualTo, as we don't have any non-inherited member variables
   virtual ObjectStore & getObjectStoreTypedInstance() const override;

private:
   std::optional<double> m_attenuation_pct  ;
   std::optional<int   > m_timesCultured    ;
   std::optional<int   > m_cellCountBillions;
};

BT_DECLARE_METATYPES(RecipeAdditionYeast)

#endif
