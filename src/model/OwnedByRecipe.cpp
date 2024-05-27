/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OwnedByRecipe.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "model/OwnedByRecipe.h"

#include "model/Recipe.h"

QString OwnedByRecipe::localisedName() { return tr("Owned By Recipe"); }

bool OwnedByRecipe::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   OwnedByRecipe const & rhs = static_cast<OwnedByRecipe const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_recipeId == rhs.m_recipeId
   );
}

TypeLookup const OwnedByRecipe::typeLookup {
   "OwnedByRecipe",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::OwnedByRecipe::recipeId, OwnedByRecipe::m_recipeId),
   },
   // Parent class lookup
   {&NamedEntity::typeLookup}
};

OwnedByRecipe::OwnedByRecipe(QString name, int const recipeId) :
   NamedEntity{name, true},
   m_recipeId{recipeId} {
   return;
}

OwnedByRecipe::OwnedByRecipe(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   SET_REGULAR_FROM_NPB(m_recipeId, namedParameterBundle, PropertyNames::OwnedByRecipe::recipeId) {
   return;
}

OwnedByRecipe::OwnedByRecipe(OwnedByRecipe const & other) :
   NamedEntity{other},
   m_recipeId{other.m_recipeId} {
   return;
}

OwnedByRecipe::~OwnedByRecipe() = default;

int OwnedByRecipe::recipeId() const { return this->m_recipeId; }
std::shared_ptr<Recipe> OwnedByRecipe::recipe() const {
   if (!ObjectStoreWrapper::contains<Recipe>(this->m_recipeId)) {
      qCritical() << Q_FUNC_INFO << "No recipe for ID" << this->m_recipeId;
      Q_ASSERT(false);
   }
   return ObjectStoreWrapper::getById<Recipe>(this->m_recipeId);
}

std::shared_ptr<Recipe> OwnedByRecipe::owningRecipe() const {
   if (ObjectStoreWrapper::contains<Recipe>(this->m_recipeId)) {
      return ObjectStoreWrapper::getById<Recipe>(this->m_recipeId);
   }
   return nullptr;
}

void OwnedByRecipe::setRecipeId(int const val) { SET_AND_NOTIFY(PropertyNames::OwnedByRecipe::recipeId, this->m_recipeId, val); return; }

void OwnedByRecipe::setRecipe(Recipe * recipe) {
   Q_ASSERT(nullptr != recipe);
   this->setRecipeId(recipe->key());
   return;
}
