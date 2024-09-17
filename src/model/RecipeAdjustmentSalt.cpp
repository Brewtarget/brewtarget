/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdjustmentSalt.cpp is part of Brewtarget, and is copyright the following authors 2024:
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
#include "model/RecipeAdjustmentSalt.h"

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Boil.h"
#include "model/BoilStep.h"

QString RecipeAdjustmentSalt::localisedName() { return tr("Salt Addition"); }

EnumStringMapping const RecipeAdjustmentSalt::whenToAddStringMapping {
///   {RecipeAdjustmentSalt::WhenToAdd::Never , "never" },
   {RecipeAdjustmentSalt::WhenToAdd::Mash  , "mash"  },
   {RecipeAdjustmentSalt::WhenToAdd::Sparge, "sparge"},
   {RecipeAdjustmentSalt::WhenToAdd::Ratio , "ratio" },
   {RecipeAdjustmentSalt::WhenToAdd::Equal , "equal" },
};

EnumStringMapping const RecipeAdjustmentSalt::whenToAddDisplayNames {
///   {RecipeAdjustmentSalt::WhenToAdd::Never , tr("Never" )},
   {RecipeAdjustmentSalt::WhenToAdd::Mash  , tr("Mash"  )},
   {RecipeAdjustmentSalt::WhenToAdd::Sparge, tr("Sparge")},
   {RecipeAdjustmentSalt::WhenToAdd::Ratio , tr("Ratio" )},
   {RecipeAdjustmentSalt::WhenToAdd::Equal , tr("Equal" )},
};

ObjectStore & RecipeAdjustmentSalt::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeAdjustmentSalt>::getInstance();
}

TypeLookup const RecipeAdjustmentSalt::typeLookup {
   "RecipeAdjustmentSalt",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::RecipeAdjustmentSalt::salt     , RecipeAdjustmentSalt::salt     ),
      PROPERTY_TYPE_LOOKUP_ENTRY      (PropertyNames::RecipeAdjustmentSalt::whenToAdd, RecipeAdjustmentSalt::m_whenToAdd),
   },
   // Parent classes lookup.  NB: IngredientInRecipe not NamedEntity!
   {&IngredientInRecipe::typeLookup,
    std::addressof(IngredientAmount<RecipeAdjustmentSalt, Salt>::typeLookup)}
};
static_assert(std::is_base_of<IngredientInRecipe, RecipeAdjustmentSalt>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdjustmentSalt, Salt>, RecipeAdjustmentSalt>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Salt>);
static_assert(!HasTypeLookup<QString>);

RecipeAdjustmentSalt::RecipeAdjustmentSalt(QString name, int const recipeId, int const saltId) :
   IngredientInRecipe{name, recipeId, saltId},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{} {
   return;
}

RecipeAdjustmentSalt::RecipeAdjustmentSalt(NamedParameterBundle const & namedParameterBundle) :
   IngredientInRecipe{namedParameterBundle},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{namedParameterBundle} {
   return;
}

RecipeAdjustmentSalt::RecipeAdjustmentSalt(RecipeAdjustmentSalt const & other) :
   IngredientInRecipe{other},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{other} {
   return;
}

RecipeAdjustmentSalt::~RecipeAdjustmentSalt() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
RecipeAdjustmentSalt::WhenToAdd  RecipeAdjustmentSalt::whenToAdd() const { return this->m_whenToAdd; }

Salt * RecipeAdjustmentSalt::salt() const {
   // Normally there should always be a valid Salt in a RecipeAdjustmentSalt.  (The Recipe ID may be -1 if the addition is
   // only just about to be added to the Recipe or has just been removed from it, but there's no great reason for the
   // Salt ID not to be valid).
   if (this->m_ingredientId <= 0) {
      qWarning() << Q_FUNC_INFO << "No Salt set on RecipeAdjustmentSalt #" << this->key();
      return nullptr;
   }

///   qDebug() << Q_FUNC_INFO << "RecipeAdjustmentSalt #" << this->key() << ": Recipe #" << this->m_recipeId << ", Salt #" << this->m_ingredientId << "@" << ObjectStoreWrapper::getByIdRaw<Salt>(this->m_ingredientId);
   return ObjectStoreWrapper::getByIdRaw<Salt>(this->m_ingredientId);
}

///Recipe * RecipeAdjustmentSalt::getOwningRecipe() const {
///   return ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);
///}

NamedEntity * RecipeAdjustmentSalt::ensureExists(BtStringConst const & property) {
   if (property == PropertyNames::RecipeAdjustmentSalt::salt) {
      // It's a coding error if a RecipeAdjustmentSalt doesn't have a Salt by the time we're accessing it via the property
      // system.
      Salt * salt = this->salt();
      if (!salt) {
         qCritical() << Q_FUNC_INFO << "No Salt set on RecipeAdjustmentSalt #" << this->key();
         // Stop here on debug builds
         Q_ASSERT(false);
      }
      return salt;
   }
   // It's a coding error if we're asked to "create" a relational property we don't know about
   qCritical() << Q_FUNC_INFO << "Don't know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeAdjustmentSalt::setWhenToAdd(RecipeAdjustmentSalt::WhenToAdd const val) {
   this->m_whenToAdd = val;
   return;
}

void RecipeAdjustmentSalt::setSalt(Salt * const val) {
   if (val) {
      this->setIngredientId(val->key());
      this->setName(tr("Addition of %1").arg(val->name()));
   } else {
      // Normally we don't want to invalidate the Salt on a RecipeAdjustmentSalt, because it doesn't buy us anything.
      qWarning() << Q_FUNC_INFO << "Null Salt set on RecipeAdjustmentSalt #" << this->key();
      this->setIngredientId(-1);
      this->setName(tr("Invalid!"));
   }
   return;
}

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdjustmentSalt, Salt)
RECIPE_ADDITION_CODE(RecipeAdjustmentSalt, Salt)
