/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/OwnedByRecipe.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_OwnedByRecipe.cpp"
#endif

QString OwnedByRecipe::localisedName() { return tr("Owned By Recipe"); }
QString OwnedByRecipe::localisedName_recipeId() { return tr("Recipe ID"); }

bool OwnedByRecipe::compareWith([[maybe_unused]] NamedEntity const & other,
                                [[maybe_unused]] QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
//   OwnedByRecipe const & rhs = static_cast<OwnedByRecipe const &>(other);
   // Base class will already have ensured names are equal
   return (
      //
      // Note that we do _not_ compare m_recipeId.  We need to be able to compare classes with different owners.  Eg,
      // as part of comparing whether two Recipe objects objects are equal, we need, amongst other things, to check
      // whether their owned objects are equal.
      //
      true
   );
}

TypeLookup const OwnedByRecipe::typeLookup {
   "OwnedByRecipe",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(OwnedByRecipe, recipeId, m_recipeId),
   },
   // Parent class lookup
   {&NamedEntity::typeLookup}
};

OwnedByRecipe::OwnedByRecipe(QString name, int const recipeId) :
   NamedEntity{name},
   m_recipeId{recipeId} {

   CONSTRUCTOR_END
   return;
}

OwnedByRecipe::OwnedByRecipe(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   // Although recipeId is required, we have to supply a default value for when we are reading from BeerXML or BeerJSON
   SET_REGULAR_FROM_NPB(m_recipeId, namedParameterBundle, PropertyNames::OwnedByRecipe::recipeId, -1) {

   CONSTRUCTOR_END
   return;
}

OwnedByRecipe::OwnedByRecipe(OwnedByRecipe const & other) :
   NamedEntity{other},
   m_recipeId{other.m_recipeId} {

   CONSTRUCTOR_END
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

std::shared_ptr<Recipe> OwnedByRecipe::owner() const {
   if (ObjectStoreWrapper::contains<Recipe>(this->m_recipeId)) {
      return ObjectStoreWrapper::getById<Recipe>(this->m_recipeId);
   }
   return nullptr;
}

std::shared_ptr<Recipe> OwnedByRecipe::owningRecipe() const {
   // See comment in model/NamedEntity.h.  This function is virtual (runtime polymorphic) but we implement it with by
   // calling the compile-time polymorphic function whose signature we share with BoilStep, MashStep, FermentationStep.
   return this->owner();
}

void OwnedByRecipe::setRecipeId(int const val) { SET_AND_NOTIFY(PropertyNames::OwnedByRecipe::recipeId, this->m_recipeId, val); return; }

void OwnedByRecipe::setRecipe(Recipe * recipe) {
   Q_ASSERT(nullptr != recipe);
   this->setRecipeId(recipe->key());
   return;
}
