/*
 * model/Instruction.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "model/Instruction.h"

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Recipe.h"

// This private implementation class holds all private non-virtual members of Instruction
class Instruction::impl {
public:

   /**
    * Constructor
    */
   impl(Instruction & instruction) :
      instruction{instruction},
      recipe{} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   std::shared_ptr<Recipe> getRecipe() {
      // If we already know which recipe we're in, we just return that...
      if (this->recipe) {
         return this->recipe;
      }

      // ...otherwise we have to ask the recipe object store to find our recipe
      auto result = ObjectStoreTyped<Recipe>::getInstance().findFirstMatching(
         [this](std::shared_ptr<Recipe> rec) {return rec->uses(instruction);}
      );

      if (!result.has_value()) {
         qCritical() << Q_FUNC_INFO << "Unable to find Recipe for Instruction #" << this->instruction.key();
         return nullptr;
      }

      this->recipe = result.value();

      return result.value();
   }

private:
   Instruction & instruction;
   std::shared_ptr<Recipe> recipe;
};

bool Instruction::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Instruction const & rhs = static_cast<Instruction const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_directions == rhs.m_directions &&
      this->m_hasTimer   == rhs.m_hasTimer   &&
      this->m_timerValue == rhs.m_timerValue
   );
}

ObjectStore & Instruction::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Instruction>::getInstance();
}

Instruction::Instruction(Instruction const & other) :
   NamedEntity {other},
   pimpl       {new impl{*this}},
   m_directions{other.m_directions},
   m_hasTimer  {other.m_hasTimer  },
   m_timerValue{other.m_timerValue},
   m_completed {other.m_completed },
   m_interval  {other.m_interval  } {
   return;
}

Instruction::Instruction(QString name, bool cache) :
   NamedEntity (-1, cache, name, true),
   pimpl       {new impl{*this}},
   m_directions(""),
   m_hasTimer  (false),
   m_timerValue(""),
   m_completed (false),
   m_interval  (0.0) {
   return;
}

Instruction::Instruction(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity {namedParameterBundle},
   pimpl       {new impl{*this}},
   m_directions{namedParameterBundle(PropertyNames::Instruction::directions).toString()},
   m_hasTimer  {namedParameterBundle(PropertyNames::Instruction::hasTimer  ).toBool()},
   m_timerValue{namedParameterBundle(PropertyNames::Instruction::timerValue).toString()},
   m_completed {namedParameterBundle(PropertyNames::Instruction::completed ).toBool()},
   m_interval  {namedParameterBundle(PropertyNames::Instruction::interval  ).toDouble()} {
   return;
}

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
Instruction::~Instruction() = default;

// Setters ====================================================================
void Instruction::setDirections(QString const & dir) {
   this->setAndNotify(PropertyNames::Instruction::directions, this->m_directions, dir);
}

void Instruction::setHasTimer(bool has) {
   this->setAndNotify(PropertyNames::Instruction::hasTimer, this->m_hasTimer, has);
}

void Instruction::setTimerValue(QString const & timerVal) {
   this->setAndNotify(PropertyNames::Instruction::timerValue, this->m_timerValue, timerVal);
}

void Instruction::setCompleted(bool comp) {
   this->setAndNotify(PropertyNames::Instruction::completed, this->m_completed, comp);
}

// TODO: figure out.
/*
void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(QString(reagent));
}
*/

void Instruction::setInterval(double time) {
   this->setAndNotify(PropertyNames::Instruction::interval, this->m_interval, time);
}

void Instruction::addReagent(QString const & reagent) {
   // Reagents aren't stored in the DB, so no call to setAndNotify() etc here
   m_reagents.append(reagent);
}

// Accessors ==================================================================
QString Instruction::directions() { return m_directions; }

bool Instruction::hasTimer() { return m_hasTimer; }

QString Instruction::timerValue() { return m_timerValue; }

bool Instruction::completed() { return m_completed; }

QList<QString> Instruction::reagents() { return m_reagents; }

double Instruction::interval() { return m_interval; }

int Instruction::instructionNumber() const {
   return this->pimpl->getRecipe()->instructionNumber(*this);
}

Recipe * Instruction::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
