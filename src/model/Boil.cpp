/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Boil.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "model/Boil.h"

#include <algorithm>

#include "database/ObjectStoreWrapper.h"
#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Boil.cpp"
#endif

QString Boil::localisedName() { return tr("Boil"); }

bool Boil::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Boil const & rhs = static_cast<Boil const &>(other);
   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_description  , rhs.m_description  ) &&
      Utils::AutoCompare(this->m_notes        , rhs.m_notes        ) &&
      Utils::AutoCompare(this->m_preBoilSize_l, rhs.m_preBoilSize_l) &&
      // Parent classes have to be equal too
      this->FolderBase<Boil>::doIsEqualTo(rhs) &&
      this->StepOwnerBase<Boil, BoilStep>::doIsEqualTo(rhs)
   );
}

ObjectStore & Boil::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Boil>::getInstance();
}

TypeLookup const Boil::typeLookup {
   "Boil",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Boil::description  , Boil::m_description  ,           NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Boil::notes        , Boil::m_notes        ,           NonPhysicalQuantity::String),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Boil::preBoilSize_l, Boil::m_preBoilSize_l, Measurement::PhysicalQuantity::Volume),

      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Boil::boilTime_mins, Boil::boilTime_mins, Measurement::PhysicalQuantity::Time),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Boil>::typeLookup),
    std::addressof(StepOwnerBase<Boil, BoilStep>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Boil>, Boil>::value);

//==================================================== CONSTRUCTORS ====================================================

Boil::Boil(QString name) :
   NamedEntity{name},
   FolderBase<Boil>{},
   StepOwnerBase<Boil, BoilStep>{},
   m_description  {""          },
   m_notes        {""          },
   m_preBoilSize_l{std::nullopt} {

   CONSTRUCTOR_END
   return;
}

Boil::Boil(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity     {namedParameterBundle},
   FolderBase<Boil>{namedParameterBundle},
   StepOwnerBase<Boil, BoilStep>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_description  , namedParameterBundle, PropertyNames::Boil::description  ),
   SET_REGULAR_FROM_NPB (m_notes        , namedParameterBundle, PropertyNames::Boil::notes        ),
   SET_REGULAR_FROM_NPB (m_preBoilSize_l, namedParameterBundle, PropertyNames::Boil::preBoilSize_l) {

   // If we're being constructed from a BeerXML file, we use the property boilTime_mins for RECIPE > BOIL_TIME
   SET_IF_PRESENT_FROM_NPB_NO_MV(Boil::setBoilTime_mins, namedParameterBundle, PropertyNames::Boil::boilTime_mins);

   CONSTRUCTOR_END
   return;
}

Boil::Boil(Boil const & other) :
   NamedEntity{other},
   FolderBase<Boil>{other},
   StepOwnerBase<Boil, BoilStep>{other},
   m_description  {other.m_description  },
   m_notes        {other.m_notes        },
   m_preBoilSize_l{other.m_preBoilSize_l} {

   CONSTRUCTOR_END
   return;
}

Boil::~Boil() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QString               Boil::description  () const { return this->m_description  ; }
QString               Boil::notes        () const { return this->m_notes        ; }
std::optional<double> Boil::preBoilSize_l() const { return this->m_preBoilSize_l; }

double Boil::boilTime_mins() const {
   double boilTimeProper_mins = 0.0;
   for (auto const & step : this->steps()) {
      if (step->startTemp_c() && *step->startTemp_c() > Boil::minimumBoilTemperature_c &&
          step->  endTemp_c() && *step->  endTemp_c() > Boil::minimumBoilTemperature_c &&
          step->stepTime_mins()) {
          boilTimeProper_mins += *step->stepTime_mins();
      }
   }
   return boilTimeProper_mins;
}

std::optional<double> Boil::coolTime_mins() const {
   auto boilSteps = this->steps();
   if (boilSteps.size() > 0 &&
       boilSteps.last()->startTemp_c() > Boil::minimumBoilTemperature_c &&
       boilSteps.last()->endTemp_c  () < Boil::minimumBoilTemperature_c) {
      return boilSteps.last()->stepTime_mins();
   }
   return std::nullopt;
}


//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Boil::setDescription  (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Boil::description  , this->m_description  , val); return; }
void Boil::setNotes        (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Boil::notes        , this->m_notes        , val); return; }
void Boil::setPreBoilSize_l(std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Boil::preBoilSize_l, this->m_preBoilSize_l, val); return; }

//
// This setter is only used by BeerXML processing, so we can be a bit fast and loose.  In particular, we assume the
// first step we find that is a proper boil is also the only such step.
//
void Boil::setBoilTime_mins(double const val) {
   this->ensureStandardProfile();
   for (auto step : this->steps()) {
      if (step->startTemp_c() && *step->startTemp_c() > Boil::minimumBoilTemperature_c &&
          step->  endTemp_c() && *step->  endTemp_c() > Boil::minimumBoilTemperature_c) {
         step->setStepTime_mins(val);
         break;
      }
   }
   return;
}

void Boil::acceptStepChange(QMetaProperty prop, QVariant val) {
   // TBD I don't think anything listens for changes to boilTime_mins
   this->doAcceptStepChange(this->sender(), prop, val, {&PropertyNames::Boil::boilTime_mins});
   return;
}

//=============================================== OTHER MEMBER FUNCTIONS ===============================================
void Boil::ensureStandardProfile() {
   //
   // For the moment, the logic here is pretty simple, just ensuring 3 boil steps (pre-boil, boil proper, and
   // post-boil).  If it turns out that there are recipes with more complicated boil profiles then we might need to
   // revisit this.
   //
   // We are sort of assuming that, if this function needs to do anything then probably the Boil is only used by one
   // Recipe -- because we're typically doing this when we're creating a new Boil for a Recipe being read in from a
   // BeerXML file.  That said, I don't think anything would break too horribly if in the event the Boil were used by
   // more than one Recipe.
   //
   auto recipe = ObjectStoreWrapper::findFirstMatching<Recipe>([this](Recipe * rec) {return rec->uses(*this);});
   QString const recipeStrippedName = recipe ? recipe->strippedName() : tr("a recipe");

   auto boilSteps = this->steps();
   if (boilSteps.size() == 0 || boilSteps.at(0)->startTemp_c().value_or(100.0) > Boil::minimumBoilTemperature_c) {
      // We need to add a ramp-up (aka pre-boil) step
      auto preBoil = std::make_shared<BoilStep>(tr("Pre-boil for %1").arg(recipeStrippedName));
      preBoil->setDescription(tr("Automatically-generated pre-boil step for %1").arg(recipeStrippedName));
      // Get the starting temperature for the ramp-up from the end temperature of the mash
      double startingTemp = Boil::minimumBoilTemperature_c - 1.0;
      if (recipe && recipe->mash()) {
         auto mashSteps = recipe->mash()->steps();
         if (!mashSteps.isEmpty()) {
            auto lastMashStep = mashSteps.last();
            startingTemp = std::min(lastMashStep->startTemp_c().value_or(startingTemp),
                                    lastMashStep->  endTemp_c().value_or(startingTemp));
         }
      }
      preBoil->setStartTemp_c(startingTemp);
      preBoil->setEndTemp_c(100.0);
      this->insert(preBoil, 1);
   }

   if (boilSteps.size() < 2 || boilSteps.at(1)->startTemp_c().value_or(0.0) < Boil::minimumBoilTemperature_c) {
      // We need to add a main (aka boil proper) step
      auto mainBoil = std::make_shared<BoilStep>(tr("Main boil for %1").arg(recipeStrippedName));
      mainBoil->setDescription(tr("Automatically-generated boil proper step for %1").arg(recipeStrippedName));
      mainBoil->setStartTemp_c(100.0);
      mainBoil->setEndTemp_c(100.0);
      this->insert(mainBoil, 2);
   }

   if (boilSteps.size() < 3 || boilSteps.at(2)->endTemp_c().value_or(100.0) > Boil::minimumBoilTemperature_c) {
      // We need to add a post-boil step
      auto postBoil = std::make_shared<BoilStep>(tr("Post-boil for %1").arg(recipeStrippedName));
      postBoil->setDescription(tr("Automatically-generated post-boil step for %1").arg(recipeStrippedName));
      double endingTemp = 30.0;
      if (recipe && recipe->fermentation()) {
         auto fs = recipe->fermentation()->fermentationSteps();
         if (!fs.isEmpty()) {
            auto firstFermentationStep = fs.first();
            endingTemp = firstFermentationStep->startTemp_c().value_or(endingTemp);
         }
      }
      postBoil->setStartTemp_c(100.0);
      postBoil->setEndTemp_c(endingTemp);
      this->insert(postBoil, 3);
   }

   return;
}

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Boil)

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Boil)

// Insert boiler-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Boil, boil)
