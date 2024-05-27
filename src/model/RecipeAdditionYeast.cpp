/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAdditionYeast.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "model/RecipeAdditionYeast.h"

#include "database/ObjectStoreTyped.h"
#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"

QString RecipeAdditionYeast::localisedName() { return tr("Yeast Addition"); }

ObjectStore & RecipeAdditionYeast::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<RecipeAdditionYeast>::getInstance();
}

TypeLookup const RecipeAdditionYeast::typeLookup {
   "RecipeAdditionYeast",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::RecipeAdditionYeast::yeast            , RecipeAdditionYeast::yeast              ),
      PROPERTY_TYPE_LOOKUP_ENTRY      (PropertyNames::RecipeAdditionYeast::attenuation_pct  , RecipeAdditionYeast::m_attenuation_pct  , NonPhysicalQuantity::Percentage),
      PROPERTY_TYPE_LOOKUP_ENTRY      (PropertyNames::RecipeAdditionYeast::timesCultured    , RecipeAdditionYeast::m_timesCultured    , NonPhysicalQuantity::OrdinalNumeral),
      PROPERTY_TYPE_LOOKUP_ENTRY      (PropertyNames::RecipeAdditionYeast::cellCountBillions, RecipeAdditionYeast::m_cellCountBillions, NonPhysicalQuantity::OrdinalNumeral),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::RecipeAdditionYeast::addToSecondary   , RecipeAdditionYeast::addToSecondary     ),
   },
   // Parent classes lookup.  NB: RecipeAddition not NamedEntity!
   {&RecipeAddition::typeLookup,
    std::addressof(IngredientAmount<RecipeAdditionYeast, Yeast>::typeLookup)}
};
static_assert(std::is_base_of<RecipeAddition, RecipeAdditionYeast>::value);
static_assert(std::is_base_of<IngredientAmount<RecipeAdditionYeast, Yeast>, RecipeAdditionYeast>::value);

//
// This is a compile-time check that HasTypeLookup is working properly.  It doesn't particularly belong here, but I
// didn't yet find a much better place for it to live!
//
static_assert(HasTypeLookup<Yeast>);
static_assert(!HasTypeLookup<QString>);


RecipeAdditionYeast::RecipeAdditionYeast(QString name, int const recipeId, int const ingredientId) :
   RecipeAddition{name, recipeId, ingredientId},
   RecipeAdditionBase<RecipeAdditionYeast, Yeast>{},
   IngredientAmount<RecipeAdditionYeast, Yeast>{},
   m_attenuation_pct  {std::nullopt},
   m_timesCultured    {std::nullopt},
   m_cellCountBillions{std::nullopt} {
   return;
}

RecipeAdditionYeast::RecipeAdditionYeast(NamedParameterBundle const & namedParameterBundle) :
   RecipeAddition{namedParameterBundle},
   RecipeAdditionBase<RecipeAdditionYeast, Yeast>{},
   IngredientAmount<RecipeAdditionYeast, Yeast>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_attenuation_pct  , namedParameterBundle, PropertyNames::RecipeAdditionYeast::attenuation_pct  ),
   SET_REGULAR_FROM_NPB (m_timesCultured    , namedParameterBundle, PropertyNames::RecipeAdditionYeast::timesCultured    ),
   SET_REGULAR_FROM_NPB (m_cellCountBillions, namedParameterBundle, PropertyNames::RecipeAdditionYeast::cellCountBillions) {
   //
   // If the addition stage is not specified then we assume it is fermentation, as this is the first stage at which it is usual
   // to add yeast.
   //
   m_stage = namedParameterBundle.val<RecipeAddition::Stage>(PropertyNames::RecipeAddition::stage,
                                                             RecipeAddition::Stage::Fermentation);
///   qDebug() << Q_FUNC_INFO << "RecipeAdditionYeast #" << this->key() << ": Recipe #" << this->m_recipeId << ", Yeast #" << this->m_ingredientId;
   return;
}

RecipeAdditionYeast::RecipeAdditionYeast(RecipeAdditionYeast const & other) :
   RecipeAddition{other},
   RecipeAdditionBase<RecipeAdditionYeast, Yeast>{},
   IngredientAmount<RecipeAdditionYeast, Yeast>{other},
   m_attenuation_pct  {other.m_attenuation_pct  },
   m_timesCultured    {other.m_timesCultured    },
   m_cellCountBillions{other.m_cellCountBillions} {
   return;
}

RecipeAdditionYeast::~RecipeAdditionYeast() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
Yeast * RecipeAdditionYeast::yeast() const {
   // Normally there should always be a valid Yeast in a RecipeAdditionYeast.  (The Recipe ID may be -1 if the addition is
   // only just about to be added to the Recipe or has just been removed from it, but there's no great reason for the
   // Yeast ID not to be valid).
   if (this->m_ingredientId <= 0) {
      qWarning() << Q_FUNC_INFO << "No Yeast set on RecipeAdditionYeast #" << this->key();
      return nullptr;
   }

///   qDebug() << Q_FUNC_INFO << "RecipeAdditionYeast #" << this->key() << ": Recipe #" << this->m_recipeId << ", Yeast #" << this->m_ingredientId << "@" << ObjectStoreWrapper::getByIdRaw<Yeast>(this->m_ingredientId);
   return ObjectStoreWrapper::getByIdRaw<Yeast>(this->m_ingredientId);
}

std::optional<double> RecipeAdditionYeast::attenuation_pct  () const { return m_attenuation_pct; }
std::optional<int>    RecipeAdditionYeast::timesCultured    () const { return m_timesCultured  ; }
std::optional<int>    RecipeAdditionYeast::cellCountBillions() const { return m_cellCountBillions  ; }
std::optional<bool>   RecipeAdditionYeast::addToSecondary   () const {
   if (1 == this->step()) {
      return false;
   }
   return true;
}

///Recipe * RecipeAdditionYeast::getOwningRecipe() const {
///   return ObjectStoreWrapper::getByIdRaw<Recipe>(this->m_recipeId);
///}

NamedEntity * RecipeAdditionYeast::ensureExists(BtStringConst const & property) {
   if (property == PropertyNames::RecipeAdditionYeast::yeast) {
      // It's a coding error if a RecipeAdditionYeast doesn't have a Yeast by the time we're accessing it via the property
      // system.
      Yeast * yeast = this->yeast();
      if (!yeast) {
         qCritical() << Q_FUNC_INFO << "No Yeast set on RecipeAdditionYeast #" << this->key();
         // Stop here on debug builds
         Q_ASSERT(false);
      }
      return yeast;
   }
   // It's a coding error if we're asked to "create" a relational property we don't know about
   qCritical() << Q_FUNC_INFO << "Don't know how to ensure property" << property << "exists";
   // Stop here on debug builds
   Q_ASSERT(false);
   return nullptr;
}

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeAdditionYeast::setYeast(Yeast * const val) {
   if (val) {
      this->setIngredientId(val->key());
      this->setName(tr("Addition of %1").arg(val->name()));
   } else {
      // Normally we don't want to invalidate the Yeast on a RecipeAdditionYeast, because it doesn't buy us anything.
      qWarning() << Q_FUNC_INFO << "Null Yeast set on RecipeAdditionYeast #" << this->key();
      this->setIngredientId(-1);
      this->setName(tr("Invalid!"));
   }
   return;
}

void RecipeAdditionYeast::setAttenuation_pct(std::optional<double> const val) {
   SET_AND_NOTIFY(PropertyNames::RecipeAdditionYeast::attenuation_pct,
                  m_attenuation_pct,
                  this->enforceMinAndMax(val, "pct attenuation", 0.0, 100.0, 0.0));
   return;
}
void RecipeAdditionYeast::setTimesCultured(std::optional<int>     const val) { SET_AND_NOTIFY(PropertyNames::RecipeAdditionYeast::timesCultured    , m_timesCultured    , this->enforceMin      (val, "times cultured" )); return; }
void RecipeAdditionYeast::setCellCountBillions(std::optional<int> const val) { SET_AND_NOTIFY(PropertyNames::RecipeAdditionYeast::cellCountBillions, m_cellCountBillions, this->enforceMin      (val, "cell count billions" )); return; }


void RecipeAdditionYeast::setAddToSecondary (std::optional<bool  > const val) {
   if (val && *val) {
      this->setStep(2);
   } else {
      this->setStep(1);
   }
   return;
}


// Boilerplate code for IngredientAmount
INGREDIENT_AMOUNT_COMMON_CODE(RecipeAdditionYeast, Yeast)
