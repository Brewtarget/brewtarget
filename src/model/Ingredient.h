/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Ingredient.h is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#ifndef MODEL_INGREDIENT_H
#define MODEL_INGREDIENT_H
#pragma once

#include "model/FolderBase.h"
#include "model/OutlineableNamedEntity.h"
#include "utils/EnumStringMapping.h"
#include "utils/TypeTraits.h"

class NamedParameterBundle;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::Ingredient { inline BtStringConst const property{#property}; }
AddPropertyName(totalInventory)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/**
 * \brief Subclasses of this class are actual ingredients in a recipe (eg \c Hop, \c Fermentable).
 *
 *        Ingredients are the objects for which we keep inventory.
 */
class Ingredient : public OutlineableNamedEntity,
                   public FolderBase<Ingredient> {
   Q_OBJECT
   FOLDER_BASE_DECL(Ingredient)
   // See model/FolderBase.h for info, getters and setters for these properties
   Q_PROPERTY(QString folderPath        READ folderPath        WRITE setFolderPath)

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_totalInventory();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Ingredient(QString name = "");
   Ingredient(NamedParameterBundle const & namedParameterBundle);
   Ingredient(Ingredient const & other);

   virtual ~Ingredient();

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief It's convenient to have a property that gives us current total inventory for a given \c Ingredient instance
    *        (eg \c Hop etc instance).  Implementation in \c IngredientBase calls \c getTotalInventory on the relevant
    *        subclass of \c StockPurchase to do the real work (by summation across \c StockPurchase subclass objects).
    */
   Q_PROPERTY(Measurement::Amount totalInventory   READ totalInventory   STORED false)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   virtual Measurement::Amount totalInventory() const = 0;

};

/**
 * \brief For templates that require a parameter to be a subclass of \c Ingredient, this makes the concept requirement
 *        slightly more concise.
 *
 *        See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it).
 */
template <typename T> concept CONCEPT_FIX_UP    IsIngredient = std::is_base_of_v<Ingredient, T>;
template <typename T> concept CONCEPT_FIX_UP IsNotIngredient = std::negation_v<std::is_base_of<Ingredient, T>>;

#endif
