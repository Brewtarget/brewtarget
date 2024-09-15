/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Boil.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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

#include "model/Fermentation.h"
#include "model/FermentationStep.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"

QString Boil::localisedName() { return tr("Boil"); }

bool Boil::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Boil const & rhs = static_cast<Boil const &>(other);
   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_description  , rhs.m_description  )   &&
      Utils::AutoCompare(this->m_notes        , rhs.m_notes        ) &&
      Utils::AutoCompare(this->m_preBoilSize_l, rhs.m_preBoilSize_l)
      // .:TBD:. Should we check BoilSteps too?
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
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Boil::boilSteps, Boil::boilSteps),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Boil>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Boil>, Boil>::value);

//==================================================== CONSTRUCTORS ====================================================

Boil::Boil(QString name) :
   NamedEntity{name, true},
   FolderBase<Boil>{},
   StepOwnerBase<Boil, BoilStep>{},
   m_description  {""          },
   m_notes        {""          },
   m_preBoilSize_l{std::nullopt} {
   return;
}

Boil::Boil(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity                {namedParameterBundle},
   FolderBase<Boil>{namedParameterBundle},
   StepOwnerBase<Boil, BoilStep>{},
   SET_REGULAR_FROM_NPB (m_description  , namedParameterBundle, PropertyNames::Boil::description  ),
   SET_REGULAR_FROM_NPB (m_notes        , namedParameterBundle, PropertyNames::Boil::notes        ),
   SET_REGULAR_FROM_NPB (m_preBoilSize_l, namedParameterBundle, PropertyNames::Boil::preBoilSize_l) {
   return;
}

Boil::Boil(Boil const & other) :
   NamedEntity{other},
   FolderBase<Boil>{other},
   StepOwnerBase<Boil, BoilStep>{other},
   m_description  {other.m_description  },
   m_notes        {other.m_notes        },
   m_preBoilSize_l{other.m_preBoilSize_l} {
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
// This is only used by BeerXML processing, so we can be a bit fast and loose.  In particular, we assume the first step
// we find that is a proper boil is also the only such step.
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

void Boil::acceptStepChange([[maybe_unused]] QMetaProperty prop,
                            [[maybe_unused]] QVariant      val) {
   BoilStep * stepSender = qobject_cast<BoilStep*>(sender());
   if (!stepSender) {
      return;
   }

   // If one of our steps changed, our pseudo properties may also change, so we need to emit some signals
   if (stepSender->ownerId() == this->key()) {
      emit changed(metaProperty(*PropertyNames::Boil::boilTime_mins), QVariant());
      emit changed(metaProperty(*PropertyNames::Boil::boilSteps    ), QVariant());
   }

   return;
}

//=============================================== OTHER MEMBER FUNCTIONS ===============================================
void Boil::ensureStandardProfile() {
   //
   // For the moment, the logic here is pretty simple, just ensuring 3 boil steps (pre-boil, boil proper, and
   // post-boil).  If it turns out that there are recipes with more complicated boil profiles then we might need to
   // revisit this.
   //
   auto recipe = this->getOwningRecipe();

   auto boilSteps = this->steps();
   if (boilSteps.size() == 0 || boilSteps.at(0)->startTemp_c().value_or(100.0) > Boil::minimumBoilTemperature_c) {
      // We need to add a ramp-up (aka pre-boil) step
      auto preBoil = std::make_shared<BoilStep>(tr("Pre-boil for %1").arg(recipe->name()));
      // Get the starting temperature for the ramp-up from the end temperature of the mash
      double startingTemp = Boil::minimumBoilTemperature_c - 1.0;
      if (recipe->mash()) {
         auto mashSteps = recipe->mash()->steps();
         if (!mashSteps.isEmpty()) {
            auto lastMashStep = mashSteps.last();
            startingTemp = std::min(lastMashStep->startTemp_c().value_or(startingTemp),
                                    lastMashStep->  endTemp_c().value_or(startingTemp));
         }
      }
      preBoil->setStartTemp_c(startingTemp);
      preBoil->setEndTemp_c(100.0);
      this->insertStep(preBoil, 1);
   }

   if (boilSteps.size() < 2 || boilSteps.at(1)->startTemp_c().value_or(0.0) < Boil::minimumBoilTemperature_c) {
      // We need to add a main (aka boil proper) step
      auto mainBoil = std::make_shared<BoilStep>(tr("Main boil for %1").arg(recipe->name()));
      mainBoil->setStartTemp_c(100.0);
      mainBoil->setEndTemp_c(100.0);
      this->insertStep(mainBoil, 2);
   }

   if (boilSteps.size() < 3 || boilSteps.at(2)->endTemp_c().value_or(100.0) > Boil::minimumBoilTemperature_c) {
      // We need to add a post-boil step
      auto postBoil = std::make_shared<BoilStep>(tr("Post-boil for %1").arg(recipe->name()));
      double endingTemp = 30.0;
      if (recipe->fermentation()) {
         auto fs = recipe->fermentation()->fermentationSteps();
         if (!fs.isEmpty()) {
            auto firstFermentationStep = fs.first();
            endingTemp = firstFermentationStep->startTemp_c().value_or(endingTemp);
         }
      }
      postBoil->setStartTemp_c(100.0);
      postBoil->setEndTemp_c(endingTemp);
      this->insertStep(postBoil, 3);
   }

   return;
}

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Boil)

// Insert boiler-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Boil, boil)
