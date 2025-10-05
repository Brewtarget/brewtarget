/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdjustmentSalt.cpp is part of Brewtarget, and is copyright the following authors 2024-2025:
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

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAdjustmentSalt.cpp"
#endif

QString RecipeAdjustmentSalt::localisedName() { return tr("Salt Addition"); }
QString RecipeAdjustmentSalt::localisedName_salt     () { return tr("Salt"       ); }
QString RecipeAdjustmentSalt::localisedName_whenToAdd() { return tr("When To Add"); }

// Similar to RecipeAdditionMisc, I think it's not helpful to include the word "salt" in the instance-specific name.
// Eg if the addition were lactic acid, it would be a bit of a confusing name.
QString RecipeAdjustmentSalt::instanceNameTemplate() { return tr("Addition of %1"); }

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

bool RecipeAdjustmentSalt::compareWith(NamedEntity const & other,
                                       QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeAdjustmentSalt const & rhs = static_cast<RecipeAdjustmentSalt const &>(other);
   return (
      // Parent classes have to be equal
      this->OwnedByRecipe     ::compareWith(rhs, propertiesThatDiffer) &&
      this->RecipeAdditionBase::compareWith(rhs, propertiesThatDiffer) &&
      this->IngredientAmount  ::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const RecipeAdjustmentSalt::typeLookup {
   "RecipeAdjustmentSalt",
   {
      PROPERTY_TYPE_LOOKUP_NO_MV(RecipeAdjustmentSalt, salt     , salt       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAdjustmentSalt, whenToAdd, m_whenToAdd, ENUM_INFO(RecipeAdjustmentSalt::whenToAdd)),
   },
   // Parent classes lookup.  NB: OwnedByRecipe not NamedEntity!
   {&OwnedByRecipe::typeLookup,
    std::addressof(IngredientAmount<RecipeAdjustmentSalt, Salt>::typeLookup)}
};
static_assert(std::is_base_of<OwnedByRecipe, RecipeAdjustmentSalt>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdjustmentSalt, Salt>, RecipeAdjustmentSalt>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Salt>);
static_assert(!HasTypeLookup<QString>);

RecipeAdjustmentSalt::RecipeAdjustmentSalt(QString name, int const recipeId, int const saltId) :
   OwnedByRecipe{name, recipeId},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{saltId} {

   CONSTRUCTOR_END
   return;
}

RecipeAdjustmentSalt::RecipeAdjustmentSalt(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{namedParameterBundle} {

   CONSTRUCTOR_END
   return;
}

RecipeAdjustmentSalt::RecipeAdjustmentSalt(RecipeAdjustmentSalt const & other) :
   OwnedByRecipe{other},
   RecipeAdditionBase<RecipeAdjustmentSalt, Salt>{},
   IngredientAmount<RecipeAdjustmentSalt, Salt>{other} {

   CONSTRUCTOR_END
   return;
}

RecipeAdjustmentSalt::~RecipeAdjustmentSalt() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
RecipeAdjustmentSalt::WhenToAdd  RecipeAdjustmentSalt::whenToAdd() const { return this->m_whenToAdd; }

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

// Boilerplate code for IngredientAmount and RecipeAddition
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdjustmentSalt, Salt)
RECIPE_ADDITION_CODE(RecipeAdjustmentSalt, Salt, salt)
