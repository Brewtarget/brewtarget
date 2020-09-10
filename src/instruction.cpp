/*
 * instruction.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "instruction.h"
#include "brewtarget.h"
#include "database.h"

#include "TableSchemaConst.h"
#include "InstructionSchema.h"

QString Instruction::classNameStr()
{
   static const QString name("Instruction");
   return name;
}

Instruction::Instruction(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString(), true),
     m_directions(QString()),
     m_hasTimer  (false),
     m_timerValue(QString()),
     m_completed (false),
     m_interval  (0.0),
     m_cacheOnly(false)
{
}

Instruction::Instruction(QString name, bool cache)
   : Ingredient(Brewtarget::INSTRUCTIONTABLE, -1, name, true),
     m_directions(QString()),
     m_hasTimer  (false),
     m_timerValue(QString()),
     m_completed (false),
     m_interval  (0.0),
     m_cacheOnly(cache)
{
}

Instruction::Instruction(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool() ),
     m_directions(rec.value(kcolInstructionDirections).toString()),
     m_hasTimer  (rec.value(kcolInstructionHasTimer).toBool()),
     m_timerValue(rec.value(kcolInstructionTimerValue).toString()),
     m_completed (rec.value(kcolInstructionCompleted).toBool()),
     m_interval  (rec.value(kcolInstructionInterval).toDouble()),
     m_cacheOnly(false)
{
}

// Setters ====================================================================
void Instruction::setDirections(const QString& dir)
{
   m_directions = dir;
   if ( ! m_cacheOnly ) {
      setEasy(kpropDirections,  dir);
   }
}

void Instruction::setHasTimer(bool has)
{
   m_hasTimer = has;
   if ( ! m_cacheOnly ) {
      setEasy(kpropHasTimer,  has);
   }
}

void Instruction::setTimerValue(const QString& timerVal)
{
   m_timerValue = timerVal;
   if ( ! m_cacheOnly ) {
      setEasy(kpropTimerValue,  timerVal);
   }
}

void Instruction::setCompleted(bool comp)
{
   m_completed = comp;
   if ( ! m_cacheOnly ) {
      setEasy(kpropCompleted,  comp);
   }
}

// TODO: figure out.
/*
void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(QString(reagent));
}
*/

void Instruction::setInterval(double time) 
{
   m_interval = time;
   if ( ! m_cacheOnly ) {
      setEasy(kpropInterval,  time);
   }
}

void Instruction::addReagent(const QString& reagent)
{
   m_reagents.append(reagent);
}

void Instruction::setCacheOnly(bool cache) { m_cacheOnly = cache; }
// Accessors ==================================================================
QString Instruction::directions() { return m_directions; }

bool Instruction::hasTimer() { return m_hasTimer; }

QString Instruction::timerValue() { return m_timerValue; }

bool Instruction::completed() { return m_completed; }

QList<QString> Instruction::reagents() { return m_reagents; }

double Instruction::interval() { return m_interval; }

int Instruction::instructionNumber() const { return Database::instance().instructionNumber(this); }

bool Instruction::cacheOnly() { return m_cacheOnly; }
