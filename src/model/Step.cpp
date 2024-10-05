/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Step.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "model/Step.h"

#include "model/NamedParameterBundle.h"
#include "PhysicalConstants.h"
#include "utils/AutoCompare.h"

QString Step::localisedName() { return tr("Step"); }

bool Step::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Step const & rhs = static_cast<Step const &>(other);
   // Base class will already have ensured names are equal
   return (
      Utils::AutoCompare(this->m_stepTime_mins  , rhs.m_stepTime_mins  ) &&
      Utils::AutoCompare(this->m_startTemp_c    , rhs.m_startTemp_c    ) &&
      Utils::AutoCompare(this->m_endTemp_c      , rhs.m_endTemp_c      ) &&
      Utils::AutoCompare(this->m_stepNumber     , rhs.m_stepNumber     ) &&
      Utils::AutoCompare(this->m_ownerId        , rhs.m_ownerId        ) &&
      Utils::AutoCompare(this->m_description    , rhs.m_description    ) &&
      Utils::AutoCompare(this->m_rampTime_mins  , rhs.m_rampTime_mins  ) &&
      Utils::AutoCompare(this->m_startAcidity_pH, rhs.m_startAcidity_pH) &&
      Utils::AutoCompare(this->m_endAcidity_pH  , rhs.m_endAcidity_pH  )
   );
}

TypeLookup const Step::typeLookup {
   "Step",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::stepTime_mins  , Step::m_stepTime_mins  , Measurement::PhysicalQuantity::Time          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::startTemp_c    , Step::m_startTemp_c    , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::  endTemp_c    , Step::m_endTemp_c      , Measurement::PhysicalQuantity::Temperature   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::stepNumber     , Step::m_stepNumber     ,           NonPhysicalQuantity::OrdinalNumeral),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::ownerId        , Step::m_ownerId        ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::description    , Step::m_description    ,           NonPhysicalQuantity::String        ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::rampTime_mins  , Step::m_rampTime_mins  , Measurement::PhysicalQuantity::Time          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::startAcidity_pH, Step::m_startAcidity_pH, Measurement::PhysicalQuantity::Acidity       ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Step::  endAcidity_pH, Step::m_endAcidity_pH  , Measurement::PhysicalQuantity::Acidity       ),

      // Note that, because days is not our canonical unit of measurement for time, this has to be a
      // NonPhysicalQuantity, not Measurement::PhysicalQuantity::Time.
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Step::stepTime_days, Step::stepTime_days, NonPhysicalQuantity::OrdinalNumeral),
   },
   // Parent class lookup
   {&NamedEntity::typeLookup}
};

//==================================================== CONSTRUCTORS ====================================================
Step::Step(QString name) :
   NamedEntity      {name, true},
   m_stepTime_mins  {std::nullopt},
   m_startTemp_c    {std::nullopt},
   m_endTemp_c      {std::nullopt},
   m_stepNumber     {0           },
   m_ownerId        {-1          },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_description    {""          },
   m_rampTime_mins  {std::nullopt},
   m_startAcidity_pH{std::nullopt},
   m_endAcidity_pH  {std::nullopt} {

   CONSTRUCTOR_END
   return;
}

Step::Step(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity      (namedParameterBundle),
   // See below for m_stepTime_mins
   SET_REGULAR_FROM_NPB (m_startTemp_c    , namedParameterBundle, PropertyNames::Step::startTemp_c    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_endTemp_c      , namedParameterBundle, PropertyNames::Step::  endTemp_c    , std::nullopt),
   SET_REGULAR_FROM_NPB (m_stepNumber     , namedParameterBundle, PropertyNames::Step::stepNumber     , 0),
   SET_REGULAR_FROM_NPB (m_ownerId        , namedParameterBundle, PropertyNames::Step::ownerId        , -1),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_description    , namedParameterBundle, PropertyNames::Step::description    , ""),
   SET_REGULAR_FROM_NPB (m_rampTime_mins  , namedParameterBundle, PropertyNames::Step::rampTime_mins  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_startAcidity_pH, namedParameterBundle, PropertyNames::Step::startAcidity_pH, std::nullopt),
   SET_REGULAR_FROM_NPB (m_endAcidity_pH  , namedParameterBundle, PropertyNames::Step::  endAcidity_pH, std::nullopt) {

   // Note that we cannot call this->setStepTime_mins() or this->setStepTime_days() here, as these are virtual functions
   // which are not callable until our child class constructor is run.  Therefore handling of
   // PropertyNames::Step::stepTime_mins and PropertyNames::Step::stepTime_days is done in StepBase

   CONSTRUCTOR_END
   return;
}

Step::Step(Step const & other) :
   NamedEntity      {other},
   m_stepTime_mins  {other.m_stepTime_mins  },
   m_startTemp_c    {other.m_startTemp_c    },
   m_endTemp_c      {other.m_endTemp_c      },
   m_stepNumber     {other.m_stepNumber     },
   m_ownerId        {other.m_ownerId        },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_description    {other.m_description    },
   m_rampTime_mins  {other.m_rampTime_mins  },
   m_startAcidity_pH{other.m_startAcidity_pH},
   m_endAcidity_pH  {other.m_endAcidity_pH  } {

   CONSTRUCTOR_END
   return;
}

Step::~Step() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
std::optional<double> Step::endTemp_c      () const { return this->m_endTemp_c      ; }
int                   Step::stepNumber     () const { return this->m_stepNumber     ; }
int                   Step::ownerId        () const { return this->m_ownerId        ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
QString               Step::description    () const { return this->m_description    ; }
std::optional<double> Step::startAcidity_pH() const { return this->m_startAcidity_pH; }
std::optional<double> Step::endAcidity_pH  () const { return this->m_endAcidity_pH  ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Step::setEndTemp_c      (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Step::endTemp_c      , this->m_endTemp_c      , this->enforceMin(val, "end temp" , PhysicalConstants::absoluteZero)); return; }
void Step::setStepNumber     (int                   const   val) { SET_AND_NOTIFY(PropertyNames::Step::stepNumber     , this->m_stepNumber     , val); return; }
void Step::setOwnerId        (int                   const   val) { this->m_ownerId = val;    this->propagatePropertyChange(PropertyNames::Step::ownerId, false);    return; }
// ⮜⮜⮜ All below added for Besupport ⮞⮞⮞
void Step::setDescription    (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Step::description    , this->m_description    , val); return; }
void Step::setStartAcidity_pH(std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Step::startAcidity_pH, this->m_startAcidity_pH, val); return; }
void Step::setEndAcidity_pH  (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Step::endAcidity_pH  , this->m_endAcidity_pH  , val); return; }
