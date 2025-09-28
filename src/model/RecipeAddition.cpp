/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/RecipeAddition.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "model/RecipeAddition.h"

#include "database/ObjectStoreWrapper.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Misc.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_RecipeAddition.cpp"
#endif

QString RecipeAddition::localisedName() { return tr("Recipe Addition"); }
QString RecipeAddition::localisedName_stage          () { return tr("Stage"         ); }
QString RecipeAddition::localisedName_step           () { return tr("Step"          ); }
QString RecipeAddition::localisedName_addAtTime_mins () { return tr("Add At Time"   ); }
QString RecipeAddition::localisedName_addAtGravity_sg() { return tr("Add At Gravity"); }
QString RecipeAddition::localisedName_addAtAcidity_pH() { return tr("Add At Acidity"); }
QString RecipeAddition::localisedName_duration_mins  () { return tr("Duration"      ); }

// Note that RecipeAddition::stageStringMapping is as defined by BeerJSON, but we also use them for the DB and for
// the UI.  We can't use them for BeerXML as it doesn't really support any similar field.
EnumStringMapping const RecipeAddition::stageStringMapping {
   {RecipeAddition::Stage::Mash        , "add_to_mash"        },
   {RecipeAddition::Stage::Boil        , "add_to_boil"        },
   {RecipeAddition::Stage::Fermentation, "add_to_fermentation"},
   {RecipeAddition::Stage::Packaging   , "add_to_package"     },
};

EnumStringMapping const RecipeAddition::stageDisplayNames {
   {RecipeAddition::Stage::Mash        , tr("Add to Mash"        ) },
   {RecipeAddition::Stage::Boil        , tr("Add to Boil"        ) },
   {RecipeAddition::Stage::Fermentation, tr("Add to Fermentation") },
   {RecipeAddition::Stage::Packaging   , tr("Add to Package"     ) },
};

bool RecipeAddition::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   RecipeAddition const & rhs = static_cast<RecipeAddition const &>(other);
   // Base class will already have ensured names are equal
   return (
      //
      // Note that we do _not_ compare m_recipeId.  We need to be able to compare classes with different owners.  Eg,
      // as part of comparing whether two Recipe objects objects are equal, we need, amongst other things, to check
      // whether their owned RecipeAddition objects are equal.
      //
      AUTO_PROPERTY_COMPARE(this, rhs, m_stage          , PropertyNames::RecipeAddition::stage          , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_step           , PropertyNames::RecipeAddition::step           , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_addAtTime_mins , PropertyNames::RecipeAddition::addAtTime_mins , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_addAtGravity_sg, PropertyNames::RecipeAddition::addAtGravity_sg, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_addAtAcidity_pH, PropertyNames::RecipeAddition::addAtAcidity_pH, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_duration_mins  , PropertyNames::RecipeAddition::duration_mins  , propertiesThatDiffer) &&
      // Parent classes have to be equal too
      this->OwnedByRecipe::compareWith(other, propertiesThatDiffer)
   );
}

TypeLookup const RecipeAddition::typeLookup {
   "RecipeAddition",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, stage          , m_stage          , ENUM_INFO(RecipeAddition::stage)      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, step           , m_step           , NonPhysicalQuantity::OrdinalNumeral   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, addAtTime_mins , m_addAtTime_mins , Measurement::PhysicalQuantity::Time   , DisplayInfo::Precision{0}),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, addAtGravity_sg, m_addAtGravity_sg, Measurement::PhysicalQuantity::Density),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, addAtAcidity_pH, m_addAtAcidity_pH, Measurement::PhysicalQuantity::Acidity, DisplayInfo::Precision{1}),
      PROPERTY_TYPE_LOOKUP_ENTRY(RecipeAddition, duration_mins  , m_duration_mins  , Measurement::PhysicalQuantity::Time   ),
   },
   // Parent class lookup.
   {&OwnedByRecipe::typeLookup}
};

RecipeAddition::RecipeAddition(QString name, int const recipeId) :
   OwnedByRecipe{name, recipeId},
   m_stage          {RecipeAddition::Stage::Mash},
   m_step           {std::nullopt},
   m_addAtTime_mins {std::nullopt},
   m_addAtGravity_sg{std::nullopt},
   m_addAtAcidity_pH{std::nullopt},
   m_duration_mins  {std::nullopt} {

   CONSTRUCTOR_END
   return;
}

RecipeAddition::RecipeAddition(NamedParameterBundle const & namedParameterBundle) :
   OwnedByRecipe{namedParameterBundle},
   // Note that we do not set m_stage here as it is for subclasses to determine how that should be defaulted if it is
   // not present.
   SET_REGULAR_FROM_NPB (m_step           , namedParameterBundle, PropertyNames::RecipeAddition::step           , std::nullopt),
   SET_REGULAR_FROM_NPB (m_addAtTime_mins , namedParameterBundle, PropertyNames::RecipeAddition::addAtTime_mins , std::nullopt),
   SET_REGULAR_FROM_NPB (m_addAtGravity_sg, namedParameterBundle, PropertyNames::RecipeAddition::addAtGravity_sg, std::nullopt),
   SET_REGULAR_FROM_NPB (m_addAtAcidity_pH, namedParameterBundle, PropertyNames::RecipeAddition::addAtAcidity_pH, std::nullopt),
   SET_REGULAR_FROM_NPB (m_duration_mins  , namedParameterBundle, PropertyNames::RecipeAddition::duration_mins  , std::nullopt) {

   CONSTRUCTOR_END
   return;
}

RecipeAddition::RecipeAddition(RecipeAddition const & other) :
   OwnedByRecipe{other                 },
   m_stage          {other.m_stage          },
   m_step           {other.m_step           },
   m_addAtTime_mins {other.m_addAtTime_mins },
   m_addAtGravity_sg{other.m_addAtGravity_sg},
   m_addAtAcidity_pH{other.m_addAtAcidity_pH},
   m_duration_mins  {other.m_duration_mins  } {

   CONSTRUCTOR_END
   return;
}

RecipeAddition::~RecipeAddition() = default;

[[nodiscard]] bool RecipeAddition::lessThanByTime(RecipeAddition const & lhs, RecipeAddition const & rhs) {
   //
   // Note that, per https://en.cppreference.com/w/cpp/utility/optional/operator_cmp, operator< is sensibly defined for
   // std::optional values, so we don't have to jump through any special hoops here.
   //

   if (!Utils::AutoCompare(lhs.m_stage, rhs.m_stage)) {
      return lhs.m_stage < rhs.m_stage;
   }

   if (!Utils::AutoCompare(lhs.m_step, rhs.m_step)) {
      return lhs.m_step < rhs.m_step;
   }

   if (!Utils::AutoCompare(lhs.m_addAtTime_mins, rhs.m_addAtTime_mins)) {
      return lhs.m_addAtTime_mins < rhs.m_addAtTime_mins;
   }

   if (!Utils::AutoCompare(lhs.m_duration_mins, rhs.m_duration_mins)) {
      return lhs.m_duration_mins < rhs.m_duration_mins;
   }

   bool const tieBreakResult{lhs.name() < rhs.name()};
   // Normally leave this log statement commented out as it generates a lot of logging
//   qDebug() <<
//      Q_FUNC_INFO << "Tie-break:" << lhs.name() << "is" << (tieBreakResult ? "" : "not") << "less than" << rhs.name();
   return tieBreakResult;
}

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
RecipeAddition::Stage RecipeAddition::stage          () const { return this->m_stage          ; }
std::optional<int>    RecipeAddition::step           () const { return this->m_step           ; }
std::optional<double> RecipeAddition::addAtTime_mins () const { return this->m_addAtTime_mins ; }
std::optional<double> RecipeAddition::addAtGravity_sg() const { return this->m_addAtGravity_sg; }
std::optional<double> RecipeAddition::addAtAcidity_pH() const { return this->m_addAtAcidity_pH; }
std::optional<double> RecipeAddition::duration_mins  () const { return this->m_duration_mins  ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void RecipeAddition::setStage          (Stage                 const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::stage          , this->m_stage          , val); return; }
void RecipeAddition::setStep           (std::optional<int>    const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::step           , this->m_step           , val); return; }
void RecipeAddition::setAddAtTime_mins (std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::addAtTime_mins , this->m_addAtTime_mins , val); return; }
void RecipeAddition::setAddAtGravity_sg(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::addAtGravity_sg, this->m_addAtGravity_sg, val); return; }
void RecipeAddition::setAddAtAcidity_pH(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::addAtAcidity_pH, this->m_addAtAcidity_pH, val); return; }
void RecipeAddition::setDuration_mins  (std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::RecipeAddition::duration_mins  , this->m_duration_mins  , val); return; }


QString RecipeAddition::extraLogInfo() const {
   return QString("Stage: %1").arg(RecipeAddition::stageStringMapping[this->m_stage]);
}
