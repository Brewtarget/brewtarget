/*
 * hop.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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


#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "hop.h"
#include "brewtarget.h"

QStringList Hop::types = QStringList() << "Bittering" << "Aroma" << "Both";
QStringList Hop::forms = QStringList() << "Leaf" << "Pellet" << "Plug";
QStringList Hop::uses = QStringList() << "Boil" << "Dry Hop" << "Mash" << "First Wort" << "Aroma";

bool operator<( Hop &h1, Hop &h2 )
{
   return h1.name() < h2.name();
}

bool operator==( Hop &h1, Hop &h2 )
{
   return h1.name() == h2.name();
}

bool Hop::isValidUse(const QString& str)
{
   return (uses.indexOf(str) >= 0);
}

bool Hop::isValidType(const QString& str)
{
   return (types.indexOf(str) >= 0);
}

bool Hop::isValidForm(const QString& str)
{
   return (forms.indexOf(str) >= 0);
}

/*
void Hop::setDefaults()
{
   name = "";
   use = USEBOIL;
   notes = "";
   type = TYPEBOTH;
   form = FORMPELLET;
   origin = "";
   substitutes = "";
   
   alpha_pct = 0.0;
   amount_kg = 0.0;
   time_min = 0.0;
   beta_pct = 0.0;
   hsi_pct = 0.0;
   humulene_pct = 0.0;
   caryophyllene_pct = 0.0;
   cohumulone_pct = 0.0;
   myrcene_pct = 0.0;
}
*/

Hop::Hop()
   : BeerXMLElement()
{
}

Hop::Hop( Hop const& other )
   : BeerXMLElement(other)
{
}

/*
void Hop::fromNode(const QDomNode& hopNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = hopNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
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
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("HOP says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "ALPHA" )
      {
         setAlpha_pct(getDouble(textNode));
      }
      else if( property == "AMOUNT" )
      {
         setAmount_kg(getDouble(textNode));
      }
      else if( property == "USE" )
      {
         int ndx = uses.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid use for HOP. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            setUse( static_cast<Hop::Use>(ndx));
      }
      else if( property == "TIME" )
      {
         setTime_min(getDouble(textNode));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for HOP. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            setType(static_cast<Hop::Type>(ndx));
      }
      else if( property == "FORM" )
      {
         int ndx = forms.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid form for HOP. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            setForm( static_cast<Hop::Form>(ndx));
      }
      else if( property == "BETA" )
      {
         setBeta_pct(getDouble(textNode));
      }
      else if( property == "HSI" )
      {
         setHsi_pct(getDouble(textNode));
      }
      else if( property == "ORIGIN" )
      {
         setOrigin(value);
      }
      else if( property == "SUBSTITUTES" )
      {
         setSubstitutes(value);
      }
      else if( property == "HUMULENE" )
      {
         setHumulene_pct(getDouble(textNode));
      }
      else if( property == "CARYOPHYLLENE" )
      {
         setCaryophyllene_pct(getDouble(textNode));
      }
      else if( property == "COHUMULONE" )
      {
         setCohumulone_pct(getDouble(textNode));
      }
      else if( property == "MYRCENE" )
      {
         setMyrcene_pct(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported HOP property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}
*/

//============================="SET" METHODS====================================
void Hop::setName( const QString& str )
{
   set("name","name",str);
}

void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < alpha < 100: %1").arg(num) );
      set("alpha_pct", "alpha", num);
   }
}

void Hop::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: amount < 0: %1").arg(num) );
      return;
   }
   else
   {
      set("amount_kg", "amount", num);
   }
}

void Hop::setUse(Use u)
{
   set("use", "use", uses.at(u));
}

void Hop::setTime_min( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: time < 0: %1").arg(num) );
      return;
   }
   else
   {
      set("time_min", "time", num);
   }
}
      
void Hop::setNotes( const QString& str )
{
   set("notes", "notes", str);
}

void Hop::setType(Type t)
{
   set("type", "htype", types.at(t));
}

void Hop::setForm( Form f )
{
   set("form", "form", forms.at(f));
}

void Hop::setBeta_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < beta < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("beta_pct", "beta", num);
   }
}

void Hop::setHsi_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < hsi < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("hsi_pct", "hsi", num);
   }
}

void Hop::setOrigin( const QString& str )
{
   set("origin", "origin", str);
}

void Hop::setSubstitutes( const QString& str )
{
   set("substitutes", "substitutes", str);
}

void Hop::setHumulene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < humulene < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("humulene_pct", "humulene", num);
   }
}

void Hop::setCaryophyllene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cary < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("caryophyllene_pct", "caryophyllene", num);
   }
}

void Hop::setCohumulone_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cohumulone < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("cohumulone_pct", "cohumulone", num);
   }
}

void Hop::setMyrcene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < myrcene < 100: %1").arg(num) );
      return;
   }
   else
   {
      set("myrcene_pct", "myrcene", num);
   }
}

//============================="GET" METHODS====================================

const QString Hop::name() const
{
   return get("name").toString();
}

double Hop::alpha_pct() const
{
   return get("alpha").toDouble();
}

double Hop::amount_kg() const
{
   return get("amount").toDouble();
}

Hop::Use Hop::use() const
{
   return uses.indexOf(get("use").toString());
}

const QString Hop::getUseString() const
{
   return get("use").toString()
}

const QString Hop::getUseStringTr() const
{
   static QStringList usesTr = QStringList() << tr("Boil") << tr("Dry Hop") << tr("Mash") << tr("First Wort") << tr("Aroma");
   return usesTr.at(use());
}

double Hop::time_min() const
{
   return get("time").toDouble();
}

const QString Hop::notes() const
{
   return get("notes").toString();
}

Hop::Type Hop::type() const
{
   return types.indexOf(get("htype").toString());
}

const QString Hop::typeString() const
{
   return get("htype").toString()
}

const QString Hop::typeStringTr() const
{
   static QStringList typesTr = QStringList() << tr("Bittering") << tr("Aroma") << tr("Both");
   return typesTr.at(type());
}

Hop::Form Hop::form() const
{
   return forms.indexOf(get("form").toString());
}

const QString Hop::formString() const
{
   return get("form").toString();
}

const QString Hop::getFormStringTr() const
{
   static QStringList formsTr = QStringList() << tr("Leaf") << tr("Pellet") << tr("Plug");
   return formsTr.at(form());
}

double Hop::beta_pct() const
{
   return get("beta").toDouble();
}

double Hop::hsi_pct() const
{
   return get("hsi").toDouble();
}

const QString Hop::origin() const
{
   return get("origin").toString();
}

const QString Hop::substitutes() const
{
   return get("substitutes").toString();
}

double Hop::humulene_pct() const
{
   return get("humulene").toDouble();
}

double Hop::caryophyllene_pct() const
{
   return get("caryophyllene").toDouble();
}

double Hop::cohumulone_pct() const
{
   return get("cohumulone").toDouble();
}

double Hop::myrcene_pct() const
{
   return get("myrcene").toDouble();
}
