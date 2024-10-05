/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/BoilStep.cpp is part of Brewtarget, and is copyright the following authors 2023:
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
#include "model/BoilStep.h"

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "utils/OptionalHelpers.h"

QString BoilStep::localisedName() { return tr("Boil Step"); }

EnumStringMapping const BoilStep::chillingTypeStringMapping {
   {BoilStep::ChillingType::Batch , "batch"  },
   {BoilStep::ChillingType::Inline, "inline" },
};

EnumStringMapping const BoilStep::chillingTypeDisplayNames {
   {BoilStep::ChillingType::Batch , tr("Batch" )},
   {BoilStep::ChillingType::Inline, tr("Inline")},
};

bool BoilStep::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   BoilStep const & rhs = static_cast<BoilStep const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_chillingType == rhs.m_chillingType &&
      // Parent classes have to be equal too
      this->StepExtended::isEqualTo(other)
   );
}

TypeLookup const BoilStep::typeLookup {
   "BoilStep",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::BoilStep::chillingType, BoilStep::m_chillingType, NonPhysicalQuantity::Enum),
   },
   // Parent class lookup.  NB: StepExtended not NamedEntity!
   {&StepExtended::typeLookup}
};
static_assert(std::is_base_of<StepExtended, BoilStep>::value);

//==================================================== CONSTRUCTORS ====================================================

BoilStep::BoilStep(QString name) :
   StepExtended  {name        },
   StepBase<BoilStep, Boil, BoilStepOptions>{},
   m_chillingType{std::nullopt} {

   CONSTRUCTOR_END
   return;
}

BoilStep::BoilStep(NamedParameterBundle const & namedParameterBundle) :
   StepExtended{namedParameterBundle},
   StepBase<BoilStep, Boil, BoilStepOptions>{namedParameterBundle},
   SET_OPT_ENUM_FROM_NPB(m_chillingType, BoilStep::ChillingType, namedParameterBundle, PropertyNames::BoilStep::chillingType) {

   CONSTRUCTOR_END
   return;
}

BoilStep::BoilStep(BoilStep const & other) :
   StepExtended  {other               },
   StepBase<BoilStep, Boil, BoilStepOptions>{other},
   m_chillingType{other.m_chillingType} {

   CONSTRUCTOR_END
   return;
}

BoilStep::~BoilStep() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<BoilStep::ChillingType> BoilStep::chillingType     () const { return                    this->m_chillingType ;}
std::optional<int>                    BoilStep::chillingTypeAsInt() const { return Optional::toOptInt(this->m_chillingType); }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void BoilStep::setChillingType     (std::optional<BoilStep::ChillingType> const val) { SET_AND_NOTIFY(PropertyNames::BoilStep::chillingType, this->m_chillingType,                                    val ); return; }
void BoilStep::setChillingTypeAsInt(std::optional<int>                    const val) { SET_AND_NOTIFY(PropertyNames::BoilStep::chillingType, this->m_chillingType, Optional::fromOptInt<ChillingType>(val)); return; }

// Insert boiler-plate wrapper functions that call down to StepBase
STEP_COMMON_CODE(Boil)
