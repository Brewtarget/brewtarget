/*
 * water.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <QVector>
#include "water.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "water.h"
#include "brewtarget.h"

bool operator<(Water &w1, Water &w2)
{
   return w1.name() < w2.name();
}

bool operator==(Water &w1, Water &w2)
{
   return w1.name() == w2.name();
}

void Water::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement waterNode;
   QDomElement tmpNode;
   QDomText tmpText;

   waterNode = doc.createElement("WATER");

   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(text(amount_l));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CALCIUM");
   tmpText = doc.createTextNode(text(calcium_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("BICARBONATE");
   tmpText = doc.createTextNode(text(bicarbonate_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SULFATE");
   tmpText = doc.createTextNode(text(sulfate_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("CHLORIDE");
   tmpText = doc.createTextNode(text(chloride_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("SODIUM");
   tmpText = doc.createTextNode(text(sodium_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("MAGNESIUM");
   tmpText = doc.createTextNode(text(magnesium_ppm));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("PH");
   tmpText = doc.createTextNode(text(ph));
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpNode.appendChild(tmpText);
   waterNode.appendChild(tmpNode);

   parent.appendChild(waterNode);
}

void Water::setDefaults()
{
   name = "";
   amount_l = 0.0;
   calcium_ppm = 0.0;
   bicarbonate_ppm = 0.0;
   chloride_ppm = 0.0;
   sodium_ppm = 0.0;
   magnesium_ppm = 0.0;
   ph = 7.0;
   notes = "";
}

Water::Water()
{
   setDefaults();
}

Water::Water(const QDomNode& waterNode)
{
   fromNode(waterNode);
}

void Water::fromNode(const QDomNode& waterNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = waterNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line is not an element. Line %1").arg(textNode.lineNumber()) );
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
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("WATER says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "AMOUNT" )
      {
         setAmount_l(getDouble(textNode));
      }
      else if( property == "CALCIUM" )
      {
         setCalcium_ppm(getDouble(textNode));
      }
      else if( property == "BICARBONATE" )
      {
         setBicarbonate_ppm(getDouble(textNode));
      }
      else if( property == "SULFATE" )
      {
         setSulfate_ppm(getDouble(textNode));
      }
      else if( property == "CHLORIDE" )
      {
         setChloride_ppm(getDouble(textNode));
      }
      else if( property == "SODIUM" )
      {
         setSodium_ppm(getDouble(textNode));
      }
      else if( property == "MAGNESIUM" )
      {
         setMagnesium_ppm(getDouble(textNode));
      }
      else if( property == "PH" )
      {
         setPh(getDouble(textNode));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported WATER property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}

//================================"SET" METHODS=================================
void Water::setName( const QString &var )
{
   name = var;
   hasChanged();
}

void Water::setAmount_l( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("amount cannot be negative: " + doubleToString(var) );
   //else
   {
      amount_l = var;
      hasChanged();
   }
}

void Water::setCalcium_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("calcium cannot be negative: " + doubleToString(var) );
   //else
   {
      calcium_ppm = var;
      hasChanged();
   }
}

void Water::setBicarbonate_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("bicarbonate cannot be negative: " + doubleToString(var) );
   //else
   {
      bicarbonate_ppm = var;
      hasChanged();
   }
}

void Water::setChloride_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("chloride cannot be negative: " + doubleToString(var) );
   //else
   {
      chloride_ppm = var;
      hasChanged();
   }
}

void Water::setSodium_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("sodium cannot be negative: " + doubleToString(var) );
   //else
   {
      sodium_ppm = var;
      hasChanged();
   }
}

void Water::setMagnesium_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("magnesium cannot be negative: " + doubleToString(var) );
   //else
   {
      magnesium_ppm = var;
      hasChanged();
   }
}

void Water::setPh( double var )
{
   //if( var < 0.0 || var > 14.0 )
   //   throw WaterException("pH was not in [0,14]: " + doubleToString(var) );
   //else
   {
      ph = var;
      hasChanged();
   }
}

void Water::setSulfate_ppm( double var )
{
   //if( var < 0.0 )
   //   throw WaterException("sulfate cannot be negative: " + doubleToString(var));
   //else
   {
      sulfate_ppm = var;
      hasChanged();
   }
}

void Water::setNotes( const QString &var )
{
   notes = var;
   hasChanged();
}

//=========================="GET" METHODS=======================================
QString Water::getName() const
{
   return name;
}

double Water::getSulfate_ppm() const
{
   return sulfate_ppm;
}

double Water::getAmount_l() const
{
   return amount_l;
}

double Water::getCalcium_ppm() const
{
   return calcium_ppm;
}

double Water::getBicarbonate_ppm() const
{
   return bicarbonate_ppm;
}

double Water::getChloride_ppm() const
{
   return chloride_ppm;
}

double Water::getSodium_ppm() const
{
   return sodium_ppm;
}

double Water::getMagnesium_ppm() const
{
   return magnesium_ppm;
}

double Water::getPh() const
{
   return ph;
}

QString Water::getNotes() const
{
   return notes;
}
