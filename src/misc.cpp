/*
 * misc.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "brewtarget.h"
#include <iostream>
#include <string>
#include <QVector>
#include "misc.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

QStringList Misc::uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
QStringList Misc::types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";

//============================CONSTRUCTORS======================================
Misc::Misc() : BeerXMLElement()
{
}

Misc::Misc(Misc const& other) : BeerXMLElement(other)
{
}

// Move all of this to Database to convert XML to SQLite tables.
/*
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
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("MISC says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for MISC. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            type = static_cast<Misc::Type>(ndx);
      }
      else if( property == "USE" )
      {
         int ndx = uses.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid use for MISC. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            use = static_cast<Misc::Use>(ndx);
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
         setUseFor(value);
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported MISC property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
   
   hasChanged();
}
*/

//============================"GET" METHODS=====================================
QString Misc::name() const
{
   return get( "name" ).toString();
}

Misc::Type Misc::type() const
{
   return static_cast<Misc::Type>(types.indexOf(get("mtype").toString()));
}

const QString Misc::typeString() const
{
   return types.at(type());
}

const QString Misc::typeStringTr() const
{
   QStringList typesTr = QStringList() << QObject::tr("Spice") << QObject::tr("Fining") << QObject::tr("Water Agent") << QObject::tr("Herb") << QObject::tr("Flavor") << QObject::tr("Other");
   return typesTr.at(type());
}

Misc::Use Misc::use() const
{
   return static_cast<Misc::Use>(uses.indexOf(get("use").toString()));
}

const QString Misc::useString() const
{
   return uses.at(use());
}

const QString Misc::useStringTr() const
{
   QStringList usesTr = QStringList() << QObject::tr("Boil") << QObject::tr("Mash") << QObject::tr("Primary") << QObject::tr("Secondary") << QObject::tr("Bottling");
   return usesTr.at(use());
}

double Misc::amount() const
{
   return get("amount").toDouble();
}

double Misc::time() const
{
   return get("time").toDouble();
}

bool Misc::amountIsWeight() const
{
   return get("amount_is_weight").toBool();
}

QString Misc::useFor() const
{
   return get("use_for").toString();
}

QString Misc::notes() const
{
   return get("notes").toString();
}

//============================"SET" METHODS=====================================
void Misc::setName( const QString& var )
{
   set( "name", "name", var );
}

void Misc::setType( Type t )
{
   set( "type", "mtype", types.at(t) );
}

void Misc::setUse( Use u )
{
   set( "use", "use", uses.at(u) );
}

void Misc::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: amount < 0: %1").arg(var) );
   else
      set( "amount", "amount", var );
}

void Misc::setTime( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: time < 0: %1").arg(var) );
   else
      set( "time", "time", var );
}

void Misc::setAmountIsWeight( bool var )
{
   set( "amountIsWeight", "amount_is_weight", var );
}

void Misc::setUseFor( const QString& var )
{
   set( "useFor", "use_for", var );
}

void Misc::setNotes( const QString& var )
{
   set( "notes", "notes", var );
}

//========================OTHER METHODS=========================================

bool Misc::isValidUse( const QString& var )
{
   static const QString uses[] = {"Boil", "Mash", "Primary", "Secondary", "Bottling"};
   static const unsigned int size = 5;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == uses[i] )
         return true;
   
   return false;
}

bool Misc::isValidType( const QString& var )
{
   static const QString types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;
   
   for( i = 0; i < size; ++i )
      if( var == types[i] )
         return true;
   
   return false;
}
