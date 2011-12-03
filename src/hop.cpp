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

#include <iostream>
#include "hop.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

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
   static const QString uses[] = {"Boil", "Dry Hop", "Mash", "First Wort", "Aroma"};
   static const int length = 5;
   
   int i;
   for( i = 0; i < length; ++i )
      if( str == uses[i] )
         return true;
         
   return false;
}

bool Hop::isValidType(const QString& str)
{
   static const QString types[] = {"Bittering", "Aroma", "Both"};
   static const int length = 3;
   
   int i;
   for( i = 0; i < length; ++i )
      if( str == types[i] )
         return true;
         
   return false;
}

bool Hop::isValidForm(const QString& str)
{
   static const QString forms[] = {"Pellet", "Plug", "Leaf", ""};
   static const int length = 4;
   
   int i;
   for( i = 0; i < length; ++i )
      if( str == forms[i] )
         return true;
         
   return false;
}

void Hop::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement hopNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   hopNode = doc.createElement("HOP");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ALPHA");
   tmpText = doc.createTextNode(text(alpha_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(text(amount_kg));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("USE");
   tmpText = doc.createTextNode(getUseString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TIME");
   tmpText = doc.createTextNode(text(time_min));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(getTypeString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("FORM");
   tmpText = doc.createTextNode(getFormString());
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("BETA");
   tmpText = doc.createTextNode(text(beta_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HSI");
   tmpText = doc.createTextNode(text(hsi_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(origin);
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SUBSTITUTES");
   tmpText = doc.createTextNode(substitutes);
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("HUMULENE");
   tmpText = doc.createTextNode(text(humulene_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("CARYOPHYLLENE");
   tmpText = doc.createTextNode(text(caryophyllene_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COHUMULONE");
   tmpText = doc.createTextNode(text(cohumulone_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MYRCENE");
   tmpText = doc.createTextNode(text(myrcene_pct));
   tmpNode.appendChild(tmpText);
   hopNode.appendChild(tmpNode);
   
   parent.appendChild(hopNode);
}

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

Hop::Hop()
{
   setDefaults();
}

Hop::Hop( Hop& other )
        : Observable()
{
   name = other.name;
   alpha_pct = other.alpha_pct;
   amount_kg = other.amount_kg;
   use = other.use;
   time_min = other.time_min;

   notes = other.notes;
   type = other.type;
   form = other.form;
   beta_pct = other.beta_pct;
   hsi_pct = other.hsi_pct;
   origin = other.origin;
   substitutes = other.substitutes;
   humulene_pct = other.humulene_pct;
   caryophyllene_pct = other.caryophyllene_pct;
   cohumulone_pct = other.cohumulone_pct;
   myrcene_pct = other.myrcene_pct;
}

Hop::Hop(const QDomNode& hopNode)
{
   fromNode(hopNode);
}

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

//============================="SET" METHODS====================================
void Hop::setName( const QString& str )
{
   name = QString(str);
   hasChanged();
}

void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < alpha < 100: %1").arg(num) );
      alpha_pct = 0;
   }
   else
   {
      alpha_pct = num;
   }

   hasChanged();
}

void Hop::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: amount < 0: %1").arg(num) );
      amount_kg = 0;
   }
   else
   {
      amount_kg = num;
   }

   hasChanged();
}

bool Hop::setUse(Use u)
{
   use = u;
   hasChanged();
   return true;
}

void Hop::setTime_min( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: time < 0: %1").arg(num) );
      time_min = 0;
   }
   else
   {
      time_min = num;
   }

   hasChanged();
}
      
void Hop::setNotes( const QString& str )
{
   notes = QString(str);
   hasChanged();
}

bool Hop::setType(Type t)
{
   type = t;
   hasChanged();
   return true;
}

bool Hop::setForm( Form f )
{
   form = f;
   hasChanged();
   return true;
}

void Hop::setBeta_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < beta < 100: %1").arg(num) );
      beta_pct = 0;
   }
   else
   {
      beta_pct = num;
   }

   hasChanged();
}

void Hop::setHsi_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < hsi < 100: %1").arg(num) );
      hsi_pct = 100;
   }
   else
   {
      hsi_pct = num;
   }

   hasChanged();
}

void Hop::setOrigin( const QString& str )
{
   origin = QString(str);
   hasChanged();
}

void Hop::setSubstitutes( const QString& str )
{
   substitutes = QString(str);
   hasChanged();
}

void Hop::setHumulene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < humulene < 100: %1").arg(num) );
      humulene_pct = 0;
   }
   else
   {
      humulene_pct = num;
   }

   hasChanged();
}

void Hop::setCaryophyllene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cary < 100: %1").arg(num) );
      caryophyllene_pct = 0;
   }
   else
   {
      caryophyllene_pct = num;
   }

   hasChanged();
}

void Hop::setCohumulone_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < cohumulone < 100: %1").arg(num) );
      cohumulone_pct = 0;
   }
   else
   {
      cohumulone_pct = num;
   }

   hasChanged();
}

void Hop::setMyrcene_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < myrcene < 100: %1").arg(num) );
      myrcene_pct = 0;
   }
   else
   {
      myrcene_pct = num;
   }

   hasChanged();
}

//============================="GET" METHODS====================================

const QString Hop::getName() const
{
   return name;
}

int Hop::getVersion() const
{
   return version;
}

double Hop::getAlpha_pct() const
{
   return alpha_pct;
}

double Hop::getAmount_kg() const
{
   return amount_kg;
}

Hop::Use Hop::getUse() const
{
   return use;
}

const QString Hop::getUseString() const
{
   return uses.at(use);
}

const QString Hop::getUseStringTr() const
{
   QStringList usesTr = QStringList() << QObject::tr("Boil") << QObject::tr("Dry Hop") << QObject::tr("Mash") << QObject::tr("First Wort") << QObject::tr("Aroma");
   return usesTr.at(use);
}

double Hop::getTime_min() const
{
   return time_min;
}

const QString Hop::getNotes() const
{
   return notes;
}

Hop::Type Hop::getType() const
{
   return type;
}

const QString Hop::getTypeString() const
{
   return types.at(type);
}

const QString Hop::getTypeStringTr() const
{
   QStringList typesTr = QStringList() << QObject::tr("Bittering") << QObject::tr("Aroma") << QObject::tr("Both");
   return typesTr.at(type);
}

Hop::Form Hop::getForm() const
{
   return form;
}

const QString Hop::getFormString() const
{
   return forms.at(form);
}

const QString Hop::getFormStringTr() const
{
   QStringList formsTr = QStringList() << QObject::tr("Leaf") << QObject::tr("Pellet") << QObject::tr("Plug");
   return formsTr.at(form);
}

double Hop::getBeta_pct() const
{
   return beta_pct;
}

double Hop::getHsi_pct() const
{
   return hsi_pct;
}

const QString Hop::getOrigin() const
{
   return origin;
}

const QString Hop::getSubstitutes() const
{
   return substitutes;
}

double Hop::getHumulene_pct() const
{
   return humulene_pct;
}

double Hop::getCaryophyllene_pct() const
{
   return caryophyllene_pct;
}

double Hop::getCohumulone_pct() const
{
   return cohumulone_pct;
}

double Hop::getMyrcene_pct() const
{
   return myrcene_pct;
}
