/*
 * instruction.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "instruction.h"
#include "brewtarget.h"

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

/*
void Instruction::setDefaults()
{
   name = QString("");
   directions = QString("");
   hasTimer = false;
   timerValue = QString("00:00:00"); // hh:mm:ss
   completed = false;
   interval  = 0.0;
}
*/

Instruction::Instruction()
   : BeerXMLElement()
{
}

/*
Instruction::Instruction(
             const QString& n,
             const QString& dir,
             bool hasT,
             const QString& timerVal
             ) :
             name(n),
             directions(dir),
             hasTimer(hasT),
             timerValue(timerVal)
{
}

Instruction::Instruction(const QDomNode& instructionNode)
{
   fromNode(instructionNode);
}

void Instruction::fromNode(const QDomNode& instructionNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = instructionNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;
      
      property = node.nodeName();
      textNode = child.toText();
      value = textNode.nodeValue();
      
      if( property == "NAME" )
      {
         setName(value);
      }
      else if( property == "DIRECTIONS" )
      {
         setDirections(value);
      }
      else if( property == "HAS_TIMER" )
      {
         setHasTimer(getBool(textNode));
      }
      else if( property == "TIMER_VALUE" )
      {
         setTimerValue(value);
      }
      else if( property == "COMPLETED" )
      {
         setCompleted(getBool(textNode));
      }
      else if ( property == "INTERVAL" )
      {
          setInterval(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported INSTRUCTION property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}
*/

// Setters ====================================================================
void Instruction::setName(const QString& n)
{
   set("name", "name", n);
}

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

QString Instruction::name()
{
   return get("name").toString();
}

QString Instruction::directions()
{
   return get("directions").toString();
}

bool Instruction::hasTimer()
{
   return get("hasTimer").toBool();
}

QString Instruction::timerValue()
{
   return get("timerValue").toString();
}

bool Instruction::completed()
{
   return get("completed").toBool();
}

QVector<QString> Instruction::reagents()
{
   return _reagents;
}

double Instruction::interval() 
{
   return get("interval").toDouble();
}

int Instruction::instructionNumber() const
{
   return get("instruction_number").toInt();
}
