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

/*
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
*/

Water::Water()
   : BeerXMLElement()
{
}

/*
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
*/

//================================"SET" METHODS=================================
void Water::setName( const QString &var )
{
   set("name", "name", var);
}

void Water::setAmount_l( double var )
{
   set("amount_l", "amount", var);
}

void Water::setCalcium_ppm( double var )
{
   set("calcium_ppm", "calcium", var);
}

void Water::setBicarbonate_ppm( double var )
{
   set("bicarbonate_ppm", "bicarbonate", var);
}

void Water::setChloride_ppm( double var )
{
   set("chloride_ppm", "chloride", var);
}

void Water::setSodium_ppm( double var )
{
   set("sodium_ppm", "sodium", var);
}

void Water::setMagnesium_ppm( double var )
{
   set("magnesium_ppm", "magnesium", var);
}

void Water::setPh( double var )
{
   set("ph", "ph", var);
}

void Water::setSulfate_ppm( double var )
{
   set("sulfate_ppm", "sulfate", var);
}

void Water::setNotes( const QString &var )
{
   set("notes", "notes", var);
}

//=========================="GET" METHODS=======================================
QString Water::name() const
{
   return get("name").toString();
}

double Water::sulfate_ppm() const
{
   return get("sulfate").toDouble();
}

double Water::amount_l() const
{
   return get("amount").toDouble();
}

double Water::calcium_ppm() const
{
   return get("calcium").toDouble();
}

double Water::bicarbonate_ppm() const
{
   return get("bicarbonate").toDouble();
}

double Water::chloride_ppm() const
{
   return get("chloride").toDouble();
}

double Water::sodium_ppm() const
{
   return get("sodium").toDouble();
}

double Water::magnesium_ppm() const
{
   return get("magnesium").toDouble();
}

double Water::ph() const
{
   return get("ph").toDouble();
}

QString Water::notes() const
{
   return get("notes").toString();
}
