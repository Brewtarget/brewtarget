/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Instruction.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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

// Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
#include "moc_Instruction.cpp"

QString Instruction::localisedName() { return tr("Instruction"); }

bool Instruction::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Instruction const & rhs = static_cast<Instruction const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_directions == rhs.m_directions &&
      this->m_hasTimer   == rhs.m_hasTimer   &&
      this->m_timerValue == rhs.m_timerValue &&
      // Parent classes have to be equal too
      this->EnumeratedBase<Instruction, Recipe>::doIsEqualTo(rhs)
   );
}

TypeLookup const Instruction::typeLookup {
   "Instruction",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Instruction::completed , Instruction::m_completed ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Instruction::directions, Instruction::m_directions),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Instruction::hasTimer  , Instruction::m_hasTimer  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Instruction::interval  , Instruction::m_interval  ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Instruction::timerValue, Instruction::m_timerValue),
   },
   // Parent class lookup
   {&NamedEntity::typeLookup,
    std::addressof(EnumeratedBase<Instruction, Recipe>::typeLookup)}
};

Instruction::Instruction(QString name) :
   NamedEntity  {name },
   EnumeratedBase<Instruction, Recipe>{},
   m_directions {""   },
   m_hasTimer   {false},
   m_timerValue {""   },
   m_completed  {false},
   m_interval   {0.0  } {

   CONSTRUCTOR_END
   return;
}

Instruction::Instruction(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   EnumeratedBase<Instruction, Recipe>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_directions, namedParameterBundle, PropertyNames::Instruction::directions),
   SET_REGULAR_FROM_NPB (m_hasTimer  , namedParameterBundle, PropertyNames::Instruction::hasTimer  ),
   SET_REGULAR_FROM_NPB (m_timerValue, namedParameterBundle, PropertyNames::Instruction::timerValue),
   SET_REGULAR_FROM_NPB (m_completed , namedParameterBundle, PropertyNames::Instruction::completed ),
   SET_REGULAR_FROM_NPB (m_interval  , namedParameterBundle, PropertyNames::Instruction::interval  ) {

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
   m_interval   {other.m_interval  } {

   CONSTRUCTOR_END
   return;
}

Instruction::~Instruction() = default;

// Setters ====================================================================
void Instruction::setDirections(QString const & dir) {
   SET_AND_NOTIFY(PropertyNames::Instruction::directions, this->m_directions, dir);
}

void Instruction::setHasTimer(bool has) {
   SET_AND_NOTIFY(PropertyNames::Instruction::hasTimer, this->m_hasTimer, has);
}

void Instruction::setTimerValue(QString const & timerVal) {
   SET_AND_NOTIFY(PropertyNames::Instruction::timerValue, this->m_timerValue, timerVal);
}

void Instruction::setCompleted(bool comp) {
   SET_AND_NOTIFY(PropertyNames::Instruction::completed, this->m_completed, comp);
}

// TODO: figure out.
/*
void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(QString(reagent));
}
*/

void Instruction::setInterval(double time) {
   SET_AND_NOTIFY(PropertyNames::Instruction::interval, this->m_interval, time);
}

void Instruction::addReagent(QString const & reagent) {
   // Reagents aren't stored in the DB, so no call to setAndNotify() etc here
   m_reagents.append(reagent);
}

// Accessors ==================================================================
QString Instruction::directions() { return m_directions; }

bool    Instruction::hasTimer() { return m_hasTimer; }

QString Instruction::timerValue() { return m_timerValue; }

bool    Instruction::completed() { return m_completed; }

QList<QString> Instruction::reagents() { return m_reagents; }

double Instruction::interval() { return m_interval; }

///int Instruction::instructionNumber() const {
///   return this->pimpl->getRecipe()->instructionNumber(*this);
///}

///std::shared_ptr<Recipe> Instruction::owningRecipe() const {
///   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](std::shared_ptr<Recipe> rec) {return rec->uses(*this);} );
///}

bool operator<(Instruction & lhs, Instruction & rhs) {
   return lhs.stepNumber() < rhs.stepNumber();
}

// Insert boiler-plate wrapper functions that call down to EnumeratedBase
ENUMERATED_COMMON_CODE(Instruction)
