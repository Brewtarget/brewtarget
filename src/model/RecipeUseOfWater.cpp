/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeUseOfWater.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeUseOfWater.cpp"
#endif

QString RecipeUseOfWater::localisedName() { return tr("Recipe Use Of Water"); }
QString RecipeUseOfWater::localisedName_recipeId() { return tr("Recipe ID"); }
QString RecipeUseOfWater::localisedName_water   () { return tr("Water"    ); }
QString RecipeUseOfWater::localisedName_volume_l() { return tr("Volume"   ); }

QString RecipeUseOfWater::instanceNameTemplate() { return tr("Use of %1 water"); }

ObjectStore & RecipeUseOfWater::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeUseOfWater>::getInstance();
}

bool RecipeUseOfWater::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeUseOfWater const & rhs = static_cast<RecipeUseOfWater const &>(other);
///   // Base class will already have ensured names are equal
   return (
///      AUTO_PROPERTY_COMPARE(this, rhs, m_volume_l, PropertyNames::RecipeUseOfWater::volume_l, propertiesThatDiffer) &&
      // Parent classes have to be equal
      this->OwnedByRecipe     ::compareWith  (rhs, propertiesThatDiffer) &&
      this->RecipeAdditionBase::compareWith  (rhs, propertiesThatDiffer) &&
      this->IngredientAmount  ::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const RecipeUseOfWater::typeLookup {
   "RecipeUseOfWater",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeUseOfWater, water   , water     ),
///      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeUseOfWater, volume_l, m_volume_l, Measurement::PhysicalQuantity::Volume),
   },
   // Parent classes lookup.  NB: OwnedByRecipe not NamedEntity!
   {&OwnedByRecipe::typeLookup,
    std::addressof(IngredientAmount<RecipeUseOfWater, Water>::typeLookup)}
};
static_assert(std::is_base_of<OwnedByRecipe, RecipeUseOfWater>::value);

RecipeUseOfWater::RecipeUseOfWater(QString name, int const recipeId, int const ingredientId) :
   OwnedByRecipe{name, recipeId},
   IngredientAmount<RecipeUseOfWater, Water>{ingredientId} {

   CONSTRUCTOR_END
   return;
}

RecipeUseOfWater::RecipeUseOfWater(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   IngredientAmount<RecipeUseOfWater, Water>{namedParameterBundle} {

   CONSTRUCTOR_END
   return;
}

RecipeUseOfWater::RecipeUseOfWater(RecipeUseOfWater const & other) :
   OwnedByRecipe{other               },
   IngredientAmount<RecipeUseOfWater, Water>{other} {

   CONSTRUCTOR_END
   return;
}

RecipeUseOfWater::~RecipeUseOfWater() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
///double  RecipeUseOfWater::volume_l    () const { return this->m_volume_l; }
double  RecipeUseOfWater::volume_l    () const { return this->amount().quantity; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
///void RecipeUseOfWater::setVolume_l    (double const val) { this->m_volume_l = val; return; }
void RecipeUseOfWater::setVolume_l    (double const val) { this->setAmount(Measurement::Amount{val, Measurement::Units::liters}); return; }

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeUseOfWater, Water)
RECIPE_ADDITION_CODE(RecipeUseOfWater, Water, water)
