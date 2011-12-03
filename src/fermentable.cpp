/*
 * fermentable.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <string>
#include "fermentable.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QVariant>
#include <QObject>

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";

bool operator<(Fermentable &f1, Fermentable &f2)
{
   return f1.name() < f2.name();
}

bool operator==(Fermentable &f1, Fermentable &f2)
{
   return f1.name() == f2.name();
}

void Fermentable::toXml(QDomDocument& doc, QDomNode& parent)
{
   QDomElement fermNode;
   QDomElement tmpNode;
   QDomText tmpText;
   
   fermNode = doc.createElement("FERMENTABLE");
   
   tmpNode = doc.createElement("NAME");
   tmpText = doc.createTextNode(name);
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("VERSION");
   tmpText = doc.createTextNode(text(version));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("TYPE");
   tmpText = doc.createTextNode(types.at(type));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("AMOUNT");
   tmpText = doc.createTextNode(text(amount_kg));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("YIELD");
   tmpText = doc.createTextNode(text(yield_pct));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COLOR");
   tmpText = doc.createTextNode(text(color_srm));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ADD_AFTER_BOIL");
   tmpText = doc.createTextNode(text(addAfterBoil));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("ORIGIN");
   tmpText = doc.createTextNode(origin);
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("SUPPLIER");
   tmpText = doc.createTextNode(supplier);
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("NOTES");
   tmpText = doc.createTextNode(notes);
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("COARSE_FINE_DIFF");
   tmpText = doc.createTextNode(text(coarseFineDiff_pct));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MOISTURE");
   tmpText = doc.createTextNode(text(moisture_pct));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("DIASTATIC_POWER");
   tmpText = doc.createTextNode(text(diastaticPower_lintner));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("PROTEIN");
   tmpText = doc.createTextNode(text(protein_pct));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("MAX_IN_BATCH");
   tmpText = doc.createTextNode(text(maxInBatch_pct));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("RECOMMEND_MASH");
   tmpText = doc.createTextNode(text(recommendMash));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("IS_MASHED");
   tmpText = doc.createTextNode(text(isMashed));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   tmpNode = doc.createElement("IBU_GAL_PER_LB");
   tmpText = doc.createTextNode(text(ibuGalPerLb));
   tmpNode.appendChild(tmpText);
   fermNode.appendChild(tmpNode);
   
   parent.appendChild(fermNode);
}

Fermentable::Fermentable()
{
   setDefaults();
}

Fermentable::Fermentable( Fermentable& other )
        : Observable()
{
   name = other.name;
   type = other.type;
   amount_kg = other.amount_kg;
   yield_pct = other.yield_pct;
   color_srm = other.color_srm;

   addAfterBoil = other.addAfterBoil;
   origin = other.origin;
   supplier = other.supplier;
   notes = other.notes;
   coarseFineDiff_pct = other.coarseFineDiff_pct;
   moisture_pct = other.moisture_pct;
   diastaticPower_lintner = other.diastaticPower_lintner;
   protein_pct = other.protein_pct;
   maxInBatch_pct = other.maxInBatch_pct;
   recommendMash = other.recommendMash;
   isMashed = other.isMashed;
   ibuGalPerLb = other.ibuGalPerLb;
}

Fermentable::Fermentable(const QDomNode& fermentableNode)
{
   fromNode(fermentableNode);
}

void Fermentable::fromNode(const QDomNode& fermentableNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = fermentableNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
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
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("FERMENTABLE says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for FERMENTABLE. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            type = static_cast<Fermentable::Type>( ndx );
      }
      else if( property == "AMOUNT" )
      {
         setAmount_kg(getDouble(textNode));
      }
      else if( property == "YIELD" )
      {
         setYield_pct(getDouble(textNode));
      }
      else if( property == "COLOR" )
      {
         setColor_srm(getDouble(textNode));
      }
      else if( property == "ADD_AFTER_BOIL" )
      {
         setAddAfterBoil(getBool(textNode));
      }
      else if( property == "ORIGIN" )
      {
         setOrigin(value);
      }
      else if( property == "SUPPLIER" )
      {
         setSupplier(value);
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "COARSE_FINE_DIFF" )
      {
         setCoarseFineDiff_pct(getDouble(textNode));
      }
      else if( property == "MOISTURE" )
      {
         setMoisture_pct(getDouble(textNode));
      }
      else if( property == "DIASTATIC_POWER" )
      {
         setDiastaticPower_lintner(getDouble(textNode));
      }
      else if( property == "PROTEIN" )
      {
         setProtein_pct(getDouble(textNode));
      }
      else if( property == "MAX_IN_BATCH" )
      {
         setMaxInBatch_pct(getDouble(textNode));
      }
      else if( property == "RECOMMEND_MASH" )
      {
         setRecommendMash(getBool(textNode));
      }
      else if( property == "IS_MASHED" )
      {
         setIsMashed(getBool(textNode));
      }
      else if( property == "IBU_GAL_PER_LB" )
      {
         setIbuGalPerLb(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported FERMENTABLE property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}

void Fermentable::setDefaults()
{
   name = "";
   type = TYPEGRAIN;
   amount_kg = 0.0;
   yield_pct = 0.0;
   color_srm = 0.0;

   addAfterBoil = false;
   origin = "";
   supplier = "";
   notes = "";
   coarseFineDiff_pct = 0.0;
   moisture_pct = 0.0;
   diastaticPower_lintner = 0.0;
   protein_pct = 0.0;
   maxInBatch_pct = 0.0;
   recommendMash = false;
   isMashed = false;
   ibuGalPerLb = 0.0;
}

// Get
const QString Fermentable::getName() const { return name; }
int Fermentable::getVersion() const { return version; }
const Fermentable::Type Fermentable::getType() const { return type; }
const QString Fermentable::getTypeString() const
{
   return types.at(type);
}
const QString Fermentable::getTypeStringTr() const
{
   QStringList typesTr = QStringList () << QObject::tr("Grain") << QObject::tr("Sugar") << QObject::tr("Extract") << QObject::tr("Dry Extract") << QObject::tr("Adjunct");
   return typesTr.at(type);
}
double Fermentable::getAmount_kg() const { return amount_kg; }
double Fermentable::getYield_pct() const { return yield_pct; }
double Fermentable::getColor_srm() const { return color_srm; }

bool Fermentable::getAddAfterBoil() const { return addAfterBoil; }
const QString Fermentable::getOrigin() const { return origin; }
const QString Fermentable::getSupplier() const { return supplier; }
const QString Fermentable::getNotes() const { return notes; }
double Fermentable::getCoarseFineDiff_pct() const { return coarseFineDiff_pct; }
double Fermentable::getMoisture_pct() const { return moisture_pct; }
double Fermentable::getDiastaticPower_lintner() const { return diastaticPower_lintner; }
double Fermentable::getProtein_pct() const { return protein_pct; }
double Fermentable::getMaxInBatch_pct() const { return maxInBatch_pct; }
bool Fermentable::getRecommendMash() const { return recommendMash; }
bool Fermentable::getIsMashed() const { return isMashed; }
double Fermentable::getIbuGalPerLb() const { return ibuGalPerLb; }

double Fermentable::getEquivSucrose_kg() const
{
   // If this is a steeped grain...
   if( type == TYPEGRAIN && !isMashed )
      return 0.60 * amount_kg * yield_pct / (double)100; // Reduce the yield to 60%.
   else
      return amount_kg * yield_pct / (double)100;
}

// Set
void Fermentable::setName( const QString& str )
{
   name = QString(str);
   hasChanged(QVariant(NAME));
}
void Fermentable::setType( Type t )
{
   /*
   if( isValidType( str ) )
   {
      type = QString(str);
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: invalid type %1").arg(str()) );
      type = "Grain";
   }
   */

   type = t;

   hasChanged(QVariant(TYPE));
}

/*
void Fermentable::setType( Type type )
{
   if( type == GRAIN )
      setType("Grain");
   else if( type == SUGAR )
      setType("Sugar");
   else if( type == EXTRACT )
      setType("Extract");
   else if( type == DRY_EXTRACT )
      setType("Dry Extract");
   else if( type == ADJUNCT )
      setType("Adjunct");
}
*/
void Fermentable::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative amount: %1").arg(num) );
      amount_kg = 0;
   }
   else
   {
      amount_kg = num;
   }

   hasChanged(QVariant(AMOUNT));
}
void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      yield_pct = num;
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < yield < 100: %1").arg(num) );
      yield_pct = 0;
   }

   hasChanged(QVariant(YIELD));
}
void Fermentable::setColor_srm( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative color: %1").arg(num) );
      color_srm = 0;
   }
   else
   {
      color_srm = num;
   }

   hasChanged(QVariant(COLOR));
}

void Fermentable::setAddAfterBoil( bool b )
{
   addAfterBoil = b;
   hasChanged(QVariant(AFTERBOIL));
}
void Fermentable::setOrigin( const QString& str ) { origin = QString(str); hasChanged(QVariant(ORIGIN));}
void Fermentable::setSupplier( const QString& str) { supplier = QString(str); hasChanged(QVariant(SUPPLIER));}
void Fermentable::setNotes( const QString& str ) { notes = QString(str); hasChanged(QVariant(NOTES));}
void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      coarseFineDiff_pct = num;
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < coarsefinediff < 100: %1").arg(num) );
      coarseFineDiff_pct = 0;
   }

   hasChanged(QVariant(COARSEFINEDIFF));
}
void Fermentable::setMoisture_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      moisture_pct = num;
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < moisture < 100: %1").arg(num) );
      moisture_pct = 0;
   }

   hasChanged(QVariant(MOISTURE));
}
void Fermentable::setDiastaticPower_lintner( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative DP: %1").arg(num) );
      diastaticPower_lintner = 0;
   }
   else
   {
      diastaticPower_lintner = num;
   }

   hasChanged(QVariant(DIASTATICPOWER));
}
void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      protein_pct = num;
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < protein < 100: %1").arg(num) );
      protein_pct = 0;
   }

   hasChanged(QVariant(PROTEIN));
}
void Fermentable::setMaxInBatch_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      maxInBatch_pct = num;
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num) );
      maxInBatch_pct = 100;
   }

   hasChanged(QVariant(MAXINBATCH));
}
void Fermentable::setRecommendMash( bool b ) { recommendMash = b; hasChanged();}
void Fermentable::setIsMashed(bool var) { isMashed = var; hasChanged(QVariant(ISMASHED)); }
void Fermentable::setIbuGalPerLb( double num ) { ibuGalPerLb = num; hasChanged();}

bool Fermentable::isValidType( const QString& str )
{
   static const QString validTypes[] = {"Grain", "Sugar", "Extract", "Dry Extract", "Adjunct"};
   unsigned int i, size = 5;
   
   for( i = 0; i < size; ++i )
      if( str == validTypes[i] )
         return true;
   
   return false;
}
