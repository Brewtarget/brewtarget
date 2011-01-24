/*
 * instruction.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
#include <QVector>
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

void Instruction::setDefaults()
{
   name = QString("");
   directions = QString("");
   hasTimer = false;
   timerValue = QString("00:00:00"); // hh:mm:ss
   completed = false;
   interval  = 0.0;
}

void Instruction::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement insNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   insNode = doc.createElement("INSTRUCTION");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DIRECTIONS");
   tmpText = doc.createTextNode(directions);
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HAS_TIMER");
   tmpText = doc.createTextNode(text(hasTimer));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIMER_VALUE");
   tmpText = doc.createTextNode(timerValue);
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COMPLETED");
   tmpText = doc.createTextNode(text(completed));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   tmpNode = doc.createElement("INTERVAL");
   tmpText = doc.createTextNode(text(interval));
   tmpNode.appendChild(tmpText);
   insNode.appendChild(tmpNode);

   parent.appendChild(insNode);
}

Instruction::Instruction() : Observable()
{
   setDefaults();
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

// "set" methods.
void Instruction::setName(const QString& n)
{
   name = QString(n);
   hasChanged();
}

void Instruction::setDirections(const QString& dir)
{
   directions = QString(dir);
   hasChanged();
}

void Instruction::setHasTimer(bool has)
{
   hasTimer = has;
   hasChanged();
}

void Instruction::setTimerValue(const QString& timerVal)
{
   timerValue = QString(timerVal);
   hasChanged();
}

void Instruction::setCompleted(bool comp)
{
   completed = comp;
   hasChanged();
}

void Instruction::setReagent(const QString& reagent)
{
   reagents.push_back(reagent);
}

void Instruction::setInterval(double time) 
{
   interval = time;
}

QString Instruction::getName()
{
   return name;
}

QString Instruction::getDirections()
{
   return directions;
}

bool Instruction::getHasTimer()
{
   return hasTimer;
}

QString Instruction::getTimerValue()
{
   return timerValue;
}

bool Instruction::getCompleted()
{
   return completed;
}

QString Instruction::getReagent(int i)
{
   if ( i < reagents.size() )
      return reagents[i];
   else
      return QString("");
}

QVector<QString> Instruction::getReagents()
{
   QVector<QString> tmp;
   if ( reagents.size() > 0 )
      tmp = reagents;
   else 
      tmp.push_back(directions);

   return tmp;
}

double Instruction::getInterval() 
{
   return interval;
}
