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
#include <vector>
#include "stringparsing.h"
#include "xml.h"

void Instruction::setDefaults()
{
   name = QString("");
   directions = QString("");
   hasTimer = false;
   timerValue = QString("00:00:00"); // hh:mm:ss
   completed = false;
}

std::string Instruction::toXml()
{
   std::string ret = "<INSTRUCTION>\n";

   ret += "<NAME>"+name.toStdString()+"</NAME>\n";
   ret += "<DIRECTIONS>"+directions.toStdString()+"</DIRECTIONS>\n";
   ret += "<HAS_TIMER>"+boolToString(hasTimer)+"</HAS_TIMER>\n";
   ret += "<TIMER_VALUE>"+timerValue.toStdString()+"</TIMER_VALUE>\n";
   ret += "<COMPLETED>"+boolToString(completed)+"</COMPLETED>\n";

   ret += "</INSTRUCTION>\n";

   return ret;
}

Instruction::Instruction() : Observable()
{
   setDefaults();
}

Instruction::Instruction(const XmlNode* node)
{
   std::vector<XmlNode *> children;
   std::vector<XmlNode *> tmpVec;
   std::string tag;
   std::string leafText;
   XmlNode* leaf;
   unsigned int i, childrenSize;

   setDefaults();

   if( node->getTag() != "INSTRUCTION" )
      throw BeerXmlException("initializer not passed an INSTRUCTION node.");

   node->getChildren( children );
   childrenSize = children.size();

   for( i = 0; i < childrenSize; ++i )
   {
      tag = children[i]->getTag();
      children[i]->getChildren( tmpVec );

      // All valid children of INSTRUCTION only have zero or one child.
      if( tmpVec.size() > 1 )
         throw BeerXmlException("Tag \""+tag+"\" has more than one child.");

      // Have to deal with the fact that this node might not have
      // and children at all.
      if( tmpVec.size() == 1 )
         leaf = tmpVec[0];
      else
         leaf = &XmlNode();

      // It must be a leaf if it is a valid BeerXML entry.
      if( ! leaf->isLeaf() )
         throw BeerXmlException("Should have been a leaf but is not.");

      leafText = leaf->getLeafText();

      if( tag == "NAME" )
      {
         setName(QString(leafText.c_str()));
      }
      else if( tag == "DIRECTIONS" )
      {
         setDirections(QString(leafText.c_str()));
      }
      else if( tag == "HAS_TIMER" )
      {
         setHasTimer(parseBool(leafText));
      }
      else if( tag == "TIMER_VALUE" )
      {
         setTimerValue(QString(leafText.c_str()));
      }
      else if( tag == "COMPLETED" )
      {
         setCompleted(parseBool(leafText));
      }
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
