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

const QString kName("name");
const QString kDirections("directions");
const QString kHasTimer("hastimer");
const QString kTimerValue("timervalue");
const QString kCompleted("completed");
const QString kInterval("interval");

// these are defined in the parent, but I need them here too
const QString kDeleted("deleted");
const QString kDisplay("display");
const QString kFolder("folder");

const QString kNameProp("name");
const QString kDirectionsProp("directions");
const QString kHasTimerProp("hastimer");
const QString kTimerValueProp("timervalue");
const QString kCompletedProp("completed");
const QString kIntervalProp("interval");
QHash<QString,QString> Instruction::tagToProp = Instruction::tagToPropHash();

QHash<QString,QString> Instruction::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = "name";
   propHash["DIRECTIONS"] = "directions";
   propHash["HAS_TIMER"] = "hasTimer";
   propHash["TIMER_VALUE"] = "timerValue";
   propHash["COMPLETED"] = "completed";
   propHash["INTERVAL"] = "interval";
   
   return propHash;
}

QString Instruction::classNameStr()
{
   static const QString name("Instruction");
   return name;
}

Instruction::Instruction(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key, QString(), true),
     m_directions(QString()),
     m_hasTimer  (false),
     m_timerValue(QString()),
     m_completed (false),
     m_interval  (0.0)
{
}

Instruction::Instruction(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kName).toString(), rec.value(kDisplay).toBool() ),
     m_directions(rec.value(kDirections).toString()),
     m_hasTimer  (rec.value(kHasTimer).toBool()),
     m_timerValue(rec.value(kTimerValue).toString()),
     m_completed (rec.value(kCompleted).toBool()),
     m_interval  (rec.value(kInterval).toDouble())
{
}

// Setters ====================================================================
void Instruction::setDirections(const QString& dir, bool cacheOnly)
{
   m_directions = dir;
   if ( ! cacheOnly ) {
      set(kDirections, kDirectionsProp, dir);
   }
}

void Instruction::setHasTimer(bool has, bool cacheOnly)
{
   m_hasTimer = has;
   if ( ! cacheOnly ) {
      set(kHasTimer, kHasTimerProp, has);
   }
}

void Instruction::setTimerValue(const QString& timerVal, bool cacheOnly)
{
   m_timerValue = timerVal;
   if ( ! cacheOnly ) {
      set(kTimerValue, kTimerValueProp, timerVal);
   }
}

void Instruction::setCompleted(bool comp, bool cacheOnly)
{
   m_completed = comp;
   if ( ! cacheOnly ) {
      set(kCompleted, kCompletedProp, comp);
   }
}

// TODO: figure out.
/*
void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(QString(reagent));
}
*/

void Instruction::setInterval(double time, bool cacheOnly) 
{
   m_interval = time;
   if ( ! cacheOnly ) {
      set(kInterval, kIntervalProp, time);
   }
}

void Instruction::addReagent(const QString& reagent)
{
   m_reagents.append(reagent);
}

// Accessors ==================================================================
QString Instruction::directions() { return m_directions; }

bool Instruction::hasTimer() { return m_hasTimer; }

QString Instruction::timerValue() { return m_timerValue; }

bool Instruction::completed() { return m_completed; }

QList<QString> Instruction::reagents() { return m_reagents; }

double Instruction::interval() { return m_interval; }

int Instruction::instructionNumber() const { return Database::instance().instructionNumber(this); }
