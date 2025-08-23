/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Instruction.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#include "model/Instruction.h"

#include "database/ObjectStoreWrapper.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Instruction.cpp"
#endif

QString Instruction::localisedName() { return tr("Instruction"); }
QString Instruction::localisedName_completed    () { return tr("Completed " ); }
QString Instruction::localisedName_directions   () { return tr("Directions" ); }
QString Instruction::localisedName_hasTimer     () { return tr("Has Timer"  ); }
QString Instruction::localisedName_interval_mins() { return tr("Interval"   ); }
QString Instruction::localisedName_timerValue   () { return tr("Timer Value"); }

bool Instruction::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Instruction const & rhs = static_cast<Instruction const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_directions, PropertyNames::Instruction::directions, propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_hasTimer  , PropertyNames::Instruction::hasTimer  , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_timerValue, PropertyNames::Instruction::timerValue, propertiesThatDiffer) &&
      // Parent classes have to be equal too
      this->EnumeratedBase<Instruction, Recipe>::doCompareWith(rhs, propertiesThatDiffer)
   );
}

TypeLookup const Instruction::typeLookup {
   "Instruction",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Instruction, completed    , m_completed    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Instruction, directions   , m_directions   , NonPhysicalQuantity::String   ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Instruction, hasTimer     , m_hasTimer     , BOOL_INFO(tr("No"), tr("Yes"))),
      PROPERTY_TYPE_LOOKUP_ENTRY(Instruction, interval_mins, m_interval_mins, Measurement::PhysicalQuantity::Time, DisplayInfo::Precision{0}),
      PROPERTY_TYPE_LOOKUP_ENTRY(Instruction, timerValue   , m_timerValue   , NonPhysicalQuantity::String   ),
   },
   // Parent class lookup
   {&NamedEntity::typeLookup,
    std::addressof(EnumeratedBase<Instruction, Recipe>::typeLookup)}
};

Instruction::Instruction(QString name) :
   NamedEntity  {name },
   EnumeratedBase<Instruction, Recipe>{},
   m_directions   {""   },
   m_hasTimer     {false},
   m_timerValue   {""   },
   m_completed    {false},
   m_interval_mins{0.0  } {

   CONSTRUCTOR_END
   return;
}

Instruction::Instruction(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   EnumeratedBase<Instruction, Recipe>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_directions   , namedParameterBundle, PropertyNames::Instruction::directions   ),
   SET_REGULAR_FROM_NPB (m_hasTimer     , namedParameterBundle, PropertyNames::Instruction::hasTimer     ),
   SET_REGULAR_FROM_NPB (m_timerValue   , namedParameterBundle, PropertyNames::Instruction::timerValue   ),
   SET_REGULAR_FROM_NPB (m_completed    , namedParameterBundle, PropertyNames::Instruction::completed    ),
   SET_REGULAR_FROM_NPB (m_interval_mins, namedParameterBundle, PropertyNames::Instruction::interval_mins) {

   CONSTRUCTOR_END
   return;
}

Instruction::Instruction(Instruction const & other) :
   NamedEntity  {other},
   EnumeratedBase<Instruction, Recipe>{other},
   m_directions {other.m_directions},
   m_hasTimer   {other.m_hasTimer  },
   m_timerValue {other.m_timerValue},
   m_completed  {other.m_completed },
   m_interval_mins   {other.m_interval_mins  } {

   CONSTRUCTOR_END
   return;
}

Instruction::~Instruction() = default;

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Instruction::setDirections   (QString const & val) { SET_AND_NOTIFY(PropertyNames::Instruction::directions   , this->m_directions   , val); return; }
void Instruction::setHasTimer     (bool    const   val) { SET_AND_NOTIFY(PropertyNames::Instruction::hasTimer     , this->m_hasTimer     , val); return; }
void Instruction::setTimerValue   (QString const & val) { SET_AND_NOTIFY(PropertyNames::Instruction::timerValue   , this->m_timerValue   , val); return; }
void Instruction::setCompleted    (bool    const   val) { SET_AND_NOTIFY(PropertyNames::Instruction::completed    , this->m_completed    , val); return; }
void Instruction::setInterval_mins(double  const   val) { SET_AND_NOTIFY(PropertyNames::Instruction::interval_mins, this->m_interval_mins, val); return; }

// TODO: figure out.
/*
void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(QString(reagent));
}
*/

void Instruction::addReagent(QString const & reagent) {
   // Reagents aren't stored in the DB, so no call to setAndNotify() etc here
   m_reagents.append(reagent);
   return;
}

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
QString        Instruction::directions   () const { return this->m_directions   ; }
bool           Instruction::hasTimer     () const { return this->m_hasTimer     ; }
QString        Instruction::timerValue   () const { return this->m_timerValue   ; }
bool           Instruction::completed    () const { return this->m_completed    ; }
double         Instruction::interval_mins() const { return this->m_interval_mins; }
QList<QString> Instruction::reagents     () const { return this->m_reagents     ; }



bool operator<(Instruction & lhs, Instruction & rhs) {
   return lhs.stepNumber() < rhs.stepNumber();
}

// Insert boiler-plate wrapper functions that call down to EnumeratedBase
ENUMERATED_COMMON_CODE(Instruction)
