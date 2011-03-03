/*
 * yeast.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include <string>
#include <iostream>
#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "yeast.h"
#include "brewtarget.h"

QStringList Yeast::types = QStringList() << "Ale" << "Lager" << "Wheat" << "Wine" << "Champagne";
QStringList Yeast::forms = QStringList() << "Liquid" << "Dry" << "Slant" << "Culture";
QStringList Yeast::flocculations = QStringList() << "Low" << "Medium" << "High" << "Very High";

bool operator<(Yeast &y1, Yeast &y2)
{
   return y1.name < y2.name;
}

bool operator==(Yeast &y1, Yeast &y2)
{
   return y1.name == y2.name;
}

void Yeast::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement yeastNode;
   QDomElement tmpElement;
   QDomText tmpText;

   yeastNode = doc.createElement("YEAST");
   
   tmpElement = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);
   
   tmpElement = doc.createElement("VERSION");
   tmpText = doc.createTextNode(QString("%1").arg(version));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TYPE");
   tmpText = doc.createTextNode(types.at(type));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FORM");
   tmpText = doc.createTextNode(forms.at(form));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(text(amount));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("AMOUNT_IS_WEIGHT");
   tmpText = doc.createTextNode(text(amountIsWeight));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("LABORATORY");
   tmpText = doc.createTextNode(laboratory);
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("PRODUCT_ID");
   tmpText = doc.createTextNode(productID);
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MIN_TEMPERATURE");
   tmpText = doc.createTextNode(text(minTemperature_c));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_TEMPERATURE");
   tmpText = doc.createTextNode(text(maxTemperature_c));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("FLOCCULATION");
   tmpText = doc.createTextNode(flocculations.at(flocculation));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ATTENUATION");
   tmpText = doc.createTextNode(text(attenuation_pct));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("BEST_FOR");
   tmpText = doc.createTextNode(bestFor);
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("TIMES_CULTURED");
   tmpText = doc.createTextNode(text(timesCultured));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("MAX_REUSE");
   tmpText = doc.createTextNode(text(maxReuse));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   tmpElement = doc.createElement("ADD_TO_SECONDARY");
   tmpText = doc.createTextNode(text(addToSecondary));
   tmpElement.appendChild(tmpText);
   yeastNode.appendChild(tmpElement);

   parent.appendChild(yeastNode);
}

void Yeast::setDefaults()
{
   // Required fields.
   name = "";
   type = TYPEALE;
   form = FORMLIQUID;
   amount = 0.0;
   
   // Optional fields.
   amountIsWeight = false;
   laboratory = "";
   productID = "";
   minTemperature_c = 0.0;
   maxTemperature_c = 0.0;
   flocculation = FLOCMEDIUM;
   attenuation_pct = 0.0;
   notes = "";
   bestFor = "";
   timesCultured = 0;
   maxReuse = 0;
   addToSecondary = false;
}

Yeast::Yeast()
{
   setDefaults();
}

Yeast::Yeast(Yeast& other)
        : Observable()
{
   name = other.name;
   type = other.type;
   form = other.form;
   amount = other.amount;

   amountIsWeight = other.amountIsWeight;
   laboratory = other.laboratory;
   productID = other.productID;
   minTemperature_c = other.minTemperature_c;
   maxTemperature_c = other.maxTemperature_c;
   flocculation = other.flocculation;
   attenuation_pct = other.attenuation_pct;
   notes = other.notes;
   bestFor = other.bestFor;
   timesCultured = other.timesCultured;
   maxReuse = other.maxReuse;
   addToSecondary = other.addToSecondary;
}

Yeast::Yeast(const QDomNode& yeastNode)
{
   fromNode(yeastNode);
}

void Yeast::fromNode(const QDomNode& yeastNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = yeastNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
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
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("YEAST says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log( Brewtarget::ERROR, QObject::tr("%1 is not a valid type for yeast. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            type = static_cast<Yeast::Type>(ndx);
      }
      else if( property == "FORM" )
      {
         int ndx = forms.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log( Brewtarget::ERROR, QObject::tr("%1 is not a valid form for yeast. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            form = static_cast<Yeast::Form>(ndx);
      }
      else if( property == "AMOUNT" )
      {
         amount = getDouble(textNode);
      }
      else if( property == "AMOUNT_IS_WEIGHT" )
      {
         amountIsWeight = getBool(textNode);
      }
      else if( property == "LABORATORY" )
      {
         laboratory = value;
      }
      else if( property == "PRODUCT_ID" )
      {
         productID = value;
      }
      else if( property == "MIN_TEMPERATURE" )
      {
         minTemperature_c = getDouble(textNode);
      }
      else if( property == "MAX_TEMPERATURE" )
      {
         maxTemperature_c = getDouble(textNode);
      }
      else if( property == "FLOCCULATION" )
      {
         int ndx = flocculations.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log( Brewtarget::ERROR, QObject::tr("%1 is not a valid flocculation for yeast. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            flocculation = static_cast<Yeast::Flocculation>(ndx);
      }
      else if( property == "ATTENUATION" )
      {
         attenuation_pct = getDouble(textNode);
      }
      else if( property == "NOTES" )
      {
         notes = value;
      }
      else if( property == "BEST_FOR" )
      {
         bestFor = value;
      }
      else if( property == "TIMES_CULTURED" )
      {
         timesCultured = getInt(textNode);
      }
      else if( property == "MAX_REUSE" )
      {
         maxReuse = getInt(textNode);
      }
      else if( property == "ADD_TO_SECONDARY" )
      {
         addToSecondary = getBool(textNode);
      }
      else
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported YEAST property: %1. Line %2").arg(property).arg(node.lineNumber()) );
      }
   }
   
   hasChanged();
}

//============================="SET" METHODS====================================
void Yeast::setName( const QString& var )
{
   name = QString(var);
   hasChanged();
}

void Yeast::setType( Yeast::Type t )
{
   type = t;
   hasChanged();
}

void Yeast::setForm( Yeast::Form f )
{
   form = f;
   hasChanged();
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      return;
   else
   {
      amount = var;
      hasChanged();
   }
}

void Yeast::setAmountIsWeight( bool var )
{
   amountIsWeight = var;
   hasChanged();
}

void Yeast::setLaboratory( const QString& var )
{
   laboratory = QString(var);
   hasChanged();
}

void Yeast::setProductID( const QString& var )
{
   productID = QString(var);
   hasChanged();
}

void Yeast::setMinTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else
   {
      minTemperature_c = var;
      hasChanged();
   }
}

void Yeast::setMaxTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else
   {
      maxTemperature_c = var;
      hasChanged();
   }
}

void Yeast::setFlocculation( Yeast::Flocculation f )
{
   flocculation = f;
   hasChanged();
}

void Yeast::setAttenuation_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
   {
      attenuation_pct = var;
      hasChanged();
   }
}

void Yeast::setNotes( const QString& var )
{
   notes = QString(var);
   hasChanged();
}

void Yeast::setBestFor( const QString& var )
{
   bestFor = QString(var);
   hasChanged();
}

void Yeast::setTimesCultured( int var )
{
   if( var < 0 )
      return;
   else
   {
      timesCultured = var;
      hasChanged();
   }
}
void Yeast::setMaxReuse( int var )
{
   if( var < 0 )
      return;
   else
   {
      maxReuse = var;
      hasChanged();
   }
}

void Yeast::setAddToSecondary( bool var )
{
   addToSecondary = var;
   hasChanged();
}
   
//============================="GET" METHODS====================================
QString Yeast::getName() const { return name; }
Yeast::Type Yeast::getType() const { return type; }
const QString Yeast::getTypeString() const { return types.at(type); }
const QString Yeast::getTypeStringTr() const
{
   QStringList typesTr = QStringList() << QObject::tr("Ale") << QObject::tr("Lager") << QObject::tr("Wheat") << QObject::tr("Wine") << QObject::tr("Champagne");
   return typesTr.at(type);
}
Yeast::Form Yeast::getForm() const { return form; }
const QString Yeast::getFormString() const { return forms.at(form); }
const QString Yeast::getFormStringTr() const
{
   QStringList formsTr = QStringList() << QObject::tr("Liquid") << QObject::tr("Dry") << QObject::tr("Slant") << QObject::tr("Culture");
   return formsTr.at(form);
}
double Yeast::getAmount() const { return amount; }
bool Yeast::getAmountIsWeight() const { return amountIsWeight; }
QString Yeast::getLaboratory() const { return laboratory; }
QString Yeast::getProductID() const { return productID; }
double Yeast::getMinTemperature_c() const { return minTemperature_c; }
double Yeast::getMaxTemperature_c() const { return maxTemperature_c; }
Yeast::Flocculation Yeast::getFlocculation() const { return flocculation; }
const QString Yeast::getFlocculationString() const { return flocculations.at(flocculation); }
const QString Yeast::getFlocculationStringTr() const
{
   QStringList flocculationsTr = QStringList() << QObject::tr("Low") << QObject::tr("Medium") << QObject::tr("High") << QObject::tr("Very High");
   return flocculationsTr.at(flocculation);
}
double Yeast::getAttenuation_pct() const { return attenuation_pct; }
QString Yeast::getNotes() const { return notes; }
QString Yeast::getBestFor() const { return bestFor; }
int Yeast::getTimesCultured() const { return timesCultured; }
int Yeast::getMaxReuse() const { return maxReuse; }
bool Yeast::getAddToSecondary() const { return addToSecondary; }

bool Yeast::isValidType(const QString& str) const
{
   static const QString types[] = {"Ale", "Lager", "Wheat", "Wine", "Champagne"};
   unsigned int i, size = 5;
   
   for( i = 0; i < size; ++i )
      if( str == types[i] )
         return true;
   
   return false;
}

bool Yeast::isValidForm(const QString& str) const
{
   static const QString forms[] = {"Liquid", "Dry", "Slant", "Culture"};
   unsigned int i, size=4;
   
   for( i = 0; i < size; ++i )
      if( str == forms[i] )
         return true;
   
   return false;
}

bool Yeast::isValidFlocculation(const QString& str) const
{
   static const QString floc[] = {"Low", "Medium", "High", "Very High"};
   unsigned int i, size=4;
   
   for( i = 0; i < size; ++i )
      if( str == floc[i] )
         return true;
   
   return false;
}
