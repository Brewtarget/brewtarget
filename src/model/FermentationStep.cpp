/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/FermentationStep.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "model/FermentationStep.h"

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/AutoCompare.h"

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_FermentationStep.cpp"

QString FermentationStep::localisedName() { return tr("Fermentation Step"); }

bool FermentationStep::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   FermentationStep const & rhs = static_cast<FermentationStep const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_LOG_COMPARE(this, rhs, m_freeRise) &&
      AUTO_LOG_COMPARE(this, rhs, m_vessel  ) &&
      // Parent classes have to be equal too
      this->StepExtended::isEqualTo(other)
   );
}

TypeLookup const FermentationStep::typeLookup {
   "FermentationStep",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::FermentationStep::freeRise, FermentationStep::m_freeRise, NonPhysicalQuantity::Bool  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::FermentationStep::vessel  , FermentationStep::m_vessel  , NonPhysicalQuantity::String),
   },
   // Parent class lookup.  NB: StepExtended not NamedEntity!
   {&StepExtended::typeLookup,
    std::addressof(StepBase<FermentationStep, Fermentation, FermentationStepOptions>::typeLookup)}
};
static_assert(std::is_base_of<StepExtended, FermentationStep>::value);

//==================================================== CONSTRUCTORS ====================================================

FermentationStep::FermentationStep(QString name) :
   StepExtended{name},
   StepBase<FermentationStep, Fermentation, FermentationStepOptions>{},
   m_freeRise  {std::nullopt},
   m_vessel    {""} {

   CONSTRUCTOR_END
   return;
}

FermentationStep::FermentationStep(NamedParameterBundle const & namedParameterBundle) :
   StepExtended  {namedParameterBundle                                                   },
   StepBase<FermentationStep, Fermentation, FermentationStepOptions>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_freeRise, namedParameterBundle, PropertyNames::FermentationStep::freeRise, std::nullopt),
   SET_REGULAR_FROM_NPB (m_vessel  , namedParameterBundle, PropertyNames::FermentationStep::vessel  , QString()   ) {

   CONSTRUCTOR_END
   return;
}

FermentationStep::FermentationStep(FermentationStep const & other) :
   StepExtended{other           },
   StepBase<FermentationStep, Fermentation, FermentationStepOptions>{other},
   m_freeRise  {other.m_freeRise},
   m_vessel    {other.m_vessel  } {

   CONSTRUCTOR_END
   return;
}

FermentationStep::~FermentationStep() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<bool> FermentationStep::freeRise() const { return this->m_freeRise; }
QString             FermentationStep::vessel  () const { return this->m_vessel  ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void FermentationStep::setFreeRise(std::optional<bool> const   val) { SET_AND_NOTIFY(PropertyNames::FermentationStep::freeRise, this->m_freeRise, val); return; }
void FermentationStep::setVessel  (QString             const & val) { SET_AND_NOTIFY(PropertyNames::FermentationStep::vessel  , this->m_vessel  , val); return; }

// Insert boiler-plate wrapper functions that call down to StepBase
STEP_COMMON_CODE(Fermentation)
