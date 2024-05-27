/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeUseOfWater.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "model/RecipeUseOfWater.h"

#include "model/NamedParameterBundle.h"

QString RecipeUseOfWater::localisedName() { return tr("Recipe Use Of Water"); }

ObjectStore & RecipeUseOfWater::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeUseOfWater>::getInstance();
}

bool RecipeUseOfWater::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeUseOfWater const & rhs = static_cast<RecipeUseOfWater const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_volume_l == rhs.m_volume_l &&
      // Parent classes have to be equal too
      this->IngredientInRecipe::isEqualTo(other)
   );
}

TypeLookup const RecipeUseOfWater::typeLookup {
   "RecipeUseOfWater",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::RecipeUseOfWater::volume_l, RecipeUseOfWater::m_volume_l, Measurement::PhysicalQuantity::Volume),
   },
   // Parent classes lookup.
   {&IngredientInRecipe::typeLookup}
};
static_assert(std::is_base_of<IngredientInRecipe, RecipeUseOfWater>::value);

RecipeUseOfWater::RecipeUseOfWater(QString name, int const recipeId, int const ingredientId) :
   IngredientInRecipe{name, recipeId, ingredientId},
   m_volume_l       {0.0} {
   return;
}

RecipeUseOfWater::RecipeUseOfWater(NamedParameterBundle const & namedParameterBundle) :
   IngredientInRecipe{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_volume_l    , namedParameterBundle, PropertyNames::RecipeUseOfWater::volume_l    ) {
   return;
}

RecipeUseOfWater::RecipeUseOfWater(RecipeUseOfWater const & other) :
   IngredientInRecipe{other               },
   m_volume_l        {other.m_volume_l    } {
   return;
}

RecipeUseOfWater::~RecipeUseOfWater() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
double  RecipeUseOfWater::volume_l    () const { return this->m_volume_l;     }
Water * RecipeUseOfWater::water       () const {
   // Normally there should always be a valid Water in a RecipeUseOfWater.  (The Recipe ID may be -1 if the addition is
   // only just about to be added to the Recipe or has just been removed from it, but there's no great reason for the
   // Water ID not to be valid).
   if (this->m_ingredientId <= 0) {
      qWarning() << Q_FUNC_INFO << "No Water set on RecipeUseOfWater #" << this->key();
      return nullptr;
   }

   return ObjectStoreWrapper::getByIdRaw<Water>(this->m_ingredientId);
}


//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeUseOfWater::setVolume_l    (double const val) { this->m_volume_l     = val; return; }

void RecipeUseOfWater::setWater(Water * const val) {
   if (val) {
      this->setIngredientId(val->key());
      this->setName(tr("Use of %1").arg(val->name()));
   } else {
      // Normally we don't want to invalidate the Water on a RecipeUseOfWater, because it doesn't buy us anything.
      qWarning() << Q_FUNC_INFO << "Null Water set on RecipeUseOfWater #" << this->key();
      this->setIngredientId(-1);
      this->setName(tr("Invalid!"));
   }
   return;
}
