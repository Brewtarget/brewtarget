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
   : BeerXMLElement(table, key)
{
   setObjectName("Instruction"); 
}

// Setters ====================================================================
void Instruction::setDirections(const QString& dir)
{
   set("directions", "directions", dir);
}

void Instruction::setHasTimer(bool has)
{
   set("hasTimer", "hasTimer", has);
}

void Instruction::setTimerValue(const QString& timerVal)
{
   set("timerValue", "timerValue", timerVal);
}

void Instruction::setCompleted(bool comp)
{
   set("completed", "completed", comp);
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
   set("interval", "interval", time);
}

void Instruction::addReagent(const QString& reagent)
{
   _reagents.append(reagent);
}

// Accessors ==================================================================
QString Instruction::directions() { return get("directions").toString(); }

bool Instruction::hasTimer() { return get("hasTimer").toBool(); }

QString Instruction::timerValue() { return get("timerValue").toString(); }

bool Instruction::completed() { return get("completed").toBool(); }

QList<QString> Instruction::reagents() { return _reagents; }

double Instruction::interval() { return get("interval").toDouble(); }

int Instruction::instructionNumber() const { return Database::instance().instructionNumber(this); }
