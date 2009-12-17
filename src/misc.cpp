/*
 * misc.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "brewtarget.h"
#include <iostream>
#include <string>
#include <vector>
#include "misc.h"
#include "stringparsing.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>

bool operator<(Misc &m1, Misc &m2)
{
   return m1.name < m2.name;
}

bool operator==(Misc &m1, Misc &m2)
{
   return m1.name == m2.name;
}

void Misc::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement miscNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   miscNode = doc.createElement("MISC");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name.c_str());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(type.c_str());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(use.c_str());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(text(time));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(text(amount));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(text(amountIsWeight));
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE_FOR");
   tmpText = doc.createTextNode(useFor.c_str());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes.c_str());
   tmpNode.appendChild(tmpText);
   miscNode.appendChild(tmpNode);
   
   parent.appendChild(miscNode);
}

//============================CONSTRUCTORS======================================
void Misc::setDefaults()
{
   name = "";
   type = "Other";
   use = "Boil";
   amount = 0.0;
   time = 0.0;
   
   amountIsWeight=false;
   useFor = "";
   notes = "";
}

Misc::Misc() : Observable()
{
   setDefaults();
}

Misc::Misc(Misc& other)
        : Observable()
{
   name = other.name;
   type = other.type;
   use = other.use;
   time = other.time;
   amount = other.amount;

   amountIsWeight = other.amountIsWeight;
   useFor = other.useFor;
   notes = other.notes;
}

Misc::Misc(const QDomNode& miscNode)
{
   fromNode(miscNode);
}

void Misc::fromNode(const QDomNode& miscNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = miscNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QString("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
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
         name = value.toStdString();
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QString("MISC says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         if( isValidType(value.toStdString()) )
            type = value.toStdString();
         else
            Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a valid type for MISC. Line %2").arg(value).arg(textNode.lineNumber()) );
      }
      else if( property == "USE" )
      {
         if( isValidUse(value.toStdString()) )
            use = value.toStdString();
         else
            Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a valid use for MISC. Line %2").arg(value).arg(textNode.lineNumber()) );
      }
      else if( property == "TIME" )
      {
         setTime(getDouble(textNode));
      }
      else if( property == "AMOUNT" )
      {
         setAmount(getDouble(textNode));
      }
      else if( property == "AMOUNT_IS_WEIGHT" )
      {
         setAmountIsWeight(getBool(textNode));
      }
      else if( property == "USE_FOR" )
      {
         setUseFor(value.toStdString());
      }
      else if( property == "NOTES" )
      {
         setNotes(value.toStdString());
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QString("Unsupported MISC property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
   
   hasChanged();
}

//============================"GET" METHODS=====================================
std::string Misc::getName() const
{
   return name;
}

std::string Misc::getType() const
{
   return type;
}

std::string Misc::getUse() const
{
   return use;
}

double Misc::getAmount() const
{
   return amount;
}

double Misc::getTime() const
{
   return time;
}

bool Misc::getAmountIsWeight() const
{
   return amountIsWeight;
}

std::string Misc::getUseFor() const
{
   return useFor;
}

std::string Misc::getNotes() const
{
   return notes;
}

//============================"SET" METHODS=====================================
void Misc::setName( const std::string &var )
{
   name = std::string(var);
   hasChanged();
}

void Misc::setType( const std::string &var )
{
   if( ! isValidType(var) )
      throw MiscException("\""+var+"\" is not a valid type.");
   else
   {
      type = std::string(var);
      hasChanged();
   }
}

void Misc::setUse( const std::string &var )
{
   if( ! isValidUse(var) )
      throw MiscException("\""+var+"\" is not a valid use.");
   else
   {
      use = std::string(var);
      hasChanged();
   }
}

void Misc::setAmount( double var )
{
   if( var < 0.0 )
      throw MiscException("amount cannot be negative: " + doubleToString(var) );
   else
   {
      amount = var;
      hasChanged();
   }
}

void Misc::setTime( double var )
{
   if( var < 0.0 )
      throw MiscException("time cannot be negative: " + doubleToString(var) );
   else
   {
      time = var;
      hasChanged();
   }
}

void Misc::setAmountIsWeight( bool var )
{
   amountIsWeight = var;
   hasChanged();
}

void Misc::setUseFor( const std::string &var )
{
   useFor = std::string(var);
   hasChanged();
}

void Misc::setNotes( const std::string &var )
{
   notes = std::string(var);
   hasChanged();
}

//========================OTHER METHODS=========================================

bool Misc::isValidUse( const std::string &var )
{
   static const std::string uses[] = {"Boil", "Mash", "Primary", "Secondary", "Bottling"};
   static const unsigned int size = 5;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == uses[i] )
         return true;
   
   return false;
}

bool Misc::isValidType( const std::string &var )
{
   static const std::string types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == types[i] )
         return true;
   
   return false;
}
