/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepExtended.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "model/StepExtended.h"

#include "model/NamedParameterBundle.h"
#include "PhysicalConstants.h"
#include "utils/AutoCompare.h"

QString StepExtended::localisedName() { return tr("Extended Step"); }

bool StepExtended::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StepExtended const & rhs = static_cast<StepExtended const &>(other);
   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_startGravity_sg, rhs.m_startGravity_sg) &&
      Utils::AutoCompare(this->m_endGravity_sg  , rhs.m_endGravity_sg  ) &&
      // Parent classes have to be equal too
      this->Step::isEqualTo(other)
   );
}

TypeLookup const StepExtended::typeLookup {
   "StepExtended",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::StepExtended::startGravity_sg, StepExtended::m_startGravity_sg, Measurement::PhysicalQuantity::Density    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::StepExtended::endGravity_sg  , StepExtended::m_endGravity_sg  , Measurement::PhysicalQuantity::Density    ),
   },
   // Parent class lookup
   {&Step::typeLookup}
};

//==================================================== CONSTRUCTORS ====================================================
StepExtended::StepExtended(QString name) :
   Step             {name},
   m_startGravity_sg{std::nullopt},
   m_endGravity_sg  {std::nullopt} {

   CONSTRUCTOR_END
   return;
}

StepExtended::StepExtended(NamedParameterBundle const & namedParameterBundle) :
   Step             (namedParameterBundle                                                                     ),
   SET_REGULAR_FROM_NPB (m_startGravity_sg, namedParameterBundle, PropertyNames::StepExtended::startGravity_sg),
   SET_REGULAR_FROM_NPB (m_endGravity_sg  , namedParameterBundle, PropertyNames::StepExtended::endGravity_sg  ) {

   CONSTRUCTOR_END
   return;
}

StepExtended::StepExtended(StepExtended const & other) :
   Step      {other},
   m_startGravity_sg{other.m_startGravity_sg},
   m_endGravity_sg  {other.m_endGravity_sg  } {

   CONSTRUCTOR_END
   return;
}

StepExtended::~StepExtended() = default;


//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<double> StepExtended::startGravity_sg() const { return this->m_startGravity_sg; }
std::optional<double> StepExtended::endGravity_sg  () const { return this->m_endGravity_sg  ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void StepExtended::setStartGravity_sg(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::StepExtended::startGravity_sg, this->m_startGravity_sg, val); return; }
void StepExtended::  setEndGravity_sg(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::StepExtended::endGravity_sg  , this->m_endGravity_sg  , val); return; }
