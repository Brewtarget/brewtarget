/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/StepExtended.cpp is part of Brewtarget, and is copyright the following authors 2023-2025:
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
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_StepExtended.cpp"
#endif

QString StepExtended::localisedName() { return tr("Extended Step"); }
QString StepExtended::localisedName_startGravity_sg() { return tr("Start Gravity"); }
QString StepExtended::localisedName_endGravity_sg  () { return tr("End Gravity"  ); }

bool StepExtended::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   StepExtended const & rhs = static_cast<StepExtended const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_startGravity_sg, PropertyNames::StepExtended::startGravity_sg, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_endGravity_sg  , PropertyNames::StepExtended::endGravity_sg  , propertiesThatDiffer) &&
      // Parent classes have to be equal too
      this->Step::compareWith(other, propertiesThatDiffer)
   );
}

TypeLookup const StepExtended::typeLookup {
   "StepExtended",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(StepExtended, startGravity_sg, m_startGravity_sg, Measurement::PhysicalQuantity::Density),
      PROPERTY_TYPE_LOOKUP_ENTRY(StepExtended, endGravity_sg  , m_endGravity_sg  , Measurement::PhysicalQuantity::Density),
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
