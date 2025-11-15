/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OwnedByRecipe.h is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#ifndef MODEL_OWNEDBYRECIPE_H
#define MODEL_OWNEDBYRECIPE_H
#pragma once

#include <QString>

#include "model/NamedEntity.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
#define AddPropertyName(property) namespace PropertyNames::OwnedByRecipe { inline BtStringConst const property{#property}; }
AddPropertyName(recipe  )
AddPropertyName(recipeId)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

// Forward declarations;
class Recipe;

class OwnedByRecipe : public NamedEntity {
    Q_OBJECT

public:
   /**
    * \brief See comment in model/NamedEntity.h
    */
   static QString localisedName();
   static QString localisedName_recipe  ();
   static QString localisedName_recipeId();

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   OwnedByRecipe(QString name = "", int const recipeId = -1);
   OwnedByRecipe(NamedParameterBundle const & namedParameterBundle);
   OwnedByRecipe(OwnedByRecipe const & other);

   virtual ~OwnedByRecipe();

   //=================================================== PROPERTIES ====================================================
   Q_PROPERTY(int                     recipeId   READ recipeId   WRITE setRecipeId)
   Q_PROPERTY(std::shared_ptr<Recipe> recipe     READ recipe     /*WRITE setRecipe*/  )

   void setRecipeId(int const val);
   void setRecipe(Recipe * recipe);

   /**
    * \brief This is, amongst other things, needed by \c TreeModelBase
    */
   std::shared_ptr<Recipe> owner() const;

   virtual std::shared_ptr<Recipe> owningRecipe() const override;
   int recipeId() const;
   std::shared_ptr<Recipe> recipe() const;

   // TODO: ownerId / setOwnerId are needed by OwnedSet.  Should ideally merge them with recipeId / setRecipeId
   inline void setOwnerId(int const val) { setRecipeId(val); return; }
   inline int ownerId() const { return recipeId(); }

protected:
   virtual bool compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const override;

protected:
   int m_recipeId;
};

#endif
