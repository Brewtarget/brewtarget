/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Ingredient.h is part of Brewtarget, and is copyright the following authors 2023-2026:
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

#include "model/FolderPropertyBase.h"
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
class Ingredient : public OutlineableNamedEntity {
   Q_OBJECT

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

   explicit Ingredient(QString const & name = "");
   explicit Ingredient(NamedParameterBundle const & namedParameterBundle);
   Ingredient(Ingredient const & other);

   ~Ingredient() override;

   //=================================================== PROPERTIES ====================================================
   /**
    * \brief It's convenient to have a property that gives us current total inventory for a given \c Ingredient instance
    *        (eg \c Hop etc instance).  Implementation in \c IngredientBase calls \c getTotalInventory on the relevant
    *        subclass of \c StockPurchase to do the real work (by summation across \c StockPurchase subclass objects).
    */
   Q_PROPERTY(Measurement::Amount totalInventory   READ totalInventory   STORED false)

   //============================================ "GETTER" MEMBER FUNCTIONS ============================================
   [[nodiscard]] virtual Measurement::Amount totalInventory() const = 0;
};

#endif