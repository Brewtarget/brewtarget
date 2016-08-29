/*
 * yeast.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - marker5a
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - plut0nium
 * - Samuel Östling <MrOstling@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDomNode>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "yeast.h"
#include "brewtarget.h"

QStringList Yeast::types = QStringList() << "Ale" << "Lager" << "Wheat" << "Wine" << "Champagne";
QStringList Yeast::forms = QStringList() << "Liquid" << "Dry" << "Slant" << "Culture";
QStringList Yeast::flocculations = QStringList() << "Low" << "Medium" << "High" << "Very High";
QHash<QString,QString> Yeast::tagToProp = Yeast::tagToPropHash();

QHash<QString,QString> Yeast::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   //propHash["TYPE"] = "type";
   //propHash["FORM"] = "form";
   propHash["AMOUNT"] = "amount";
   propHash["INVENTORY"] = "inventory";
   propHash["AMOUNT_IS_WEIGHT"] = "amountIsWeight";
   propHash["LABORATORY"] = "laboratory";
   propHash["PRODUCT_ID"] = "productID";
   propHash["MIN_TEMPERATURE"] = "minTemperature_c";
   propHash["MAX_TEMPERATURE"] = "maxTemperature_c";
   //propHash["FLOCCULATION"] = "flocculation";
   propHash["ATTENUATION"] = "attenuation_pct";
   propHash["NOTES"] = "notes";
   propHash["BEST_FOR"] = "bestFor";
   propHash["TIMES_CULTURED"] = "timesCultured";
   propHash["MAX_REUSE"] = "maxReuse";
   propHash["ADD_TO_SECONDARY"] = "addToSecondary";
   return propHash;
}

bool operator<(Yeast &y1, Yeast &y2)
{
   return y1.name() < y2.name();
}

bool operator==(Yeast &y1, Yeast &y2)
{
   return y1.name() == y2.name();
}

//============================CONSTRUCTORS======================================
Yeast::Yeast()
   : BeerXMLElement()
{
}

Yeast::Yeast(Yeast const& other) : BeerXMLElement(other)
{
}

//============================="GET" METHODS====================================
QString Yeast::laboratory() const { return get("laboratory").toString();; }
QString Yeast::productID() const { return get("product_id").toString(); }
QString Yeast::notes() const { return get("notes").toString(); }
QString Yeast::bestFor() const { return get("best_for").toString(); }

const QString Yeast::typeString() const { return types.at(type()); }
const QString Yeast::formString() const { return forms.at(form()); }
const QString Yeast::flocculationString() const { return flocculations.at(flocculation()); }

double Yeast::amount() const { return get("amount").toDouble(); }
double Yeast::minTemperature_c() const { return get("min_temperature").toDouble(); }
double Yeast::maxTemperature_c() const { return get("max_temperature").toDouble(); }
double Yeast::attenuation_pct() const { return get("attenuation").toDouble(); }

int Yeast::inventory() const { return getInventory("quanta").toInt(); }
int Yeast::timesCultured() const { return get("times_cultured").toInt(); }
int Yeast::maxReuse() const { return get("max_reuse").toInt(); }

bool Yeast::addToSecondary() const { return get("add_to_secondary").toBool(); }
bool Yeast::amountIsWeight() const { return get("amount_is_weight").toBool(); }

Yeast::Form Yeast::form() const { return static_cast<Yeast::Form>( forms.indexOf(get("form").toString())); }
Yeast::Flocculation Yeast::flocculation() const { return static_cast<Yeast::Flocculation>( flocculations.indexOf(get("flocculation").toString())); }
Yeast::Type Yeast::type() const { return static_cast<Yeast::Type>( types.indexOf(get("ytype").toString())); }
const QString Yeast::typeStringTr() const
{
   static QStringList typesTr = QStringList() << QObject::tr("Ale")
                                       << QObject::tr("Lager")
                                       << QObject::tr("Wheat")
                                       << QObject::tr("Wine")
                                       << QObject::tr("Champagne");
   return typesTr.at(type());
}

const QString Yeast::formStringTr() const
{
   static QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   return formsTr.at(form());
}

const QString Yeast::flocculationStringTr() const
{
   static QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   return flocculationsTr.at(flocculation());
}

//============================="SET" METHODS====================================
void Yeast::setType( Yeast::Type t )
{
   set("type", "ytype", types.at(t));
}

void Yeast::setForm( Yeast::Form f )
{
   set("form", "form", forms.at(f));
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: amount < 0: %1").arg(var) );
   else
      set("amount", "amount", var);
}

void Yeast::setInventoryQuanta( int var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: inventory < 0: %1").arg(var) );
   else
      setInventory("inventory", "quanta", var);
}

void Yeast::setAmountIsWeight( bool var )
{
   set("amountIsWeight", "amount_is_weight", var);
}

void Yeast::setLaboratory( const QString& var )
{
   set("laboratory", "laboratory", var);
}

void Yeast::setProductID( const QString& var )
{
   set("productID", "product_id", var);
}

void Yeast::setMinTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else
      set("minTemperature_c", "min_temperature", var);
}

void Yeast::setMaxTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else
      set("maxTemperature_c", "max_temperature", var);
}

void Yeast::setFlocculation( Yeast::Flocculation f )
{
   set("flocculation", "flocculation", flocculations.at(f));
}

void Yeast::setAttenuation_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else
      set("attenuation", "attenuation", var);
}

void Yeast::setNotes( const QString& var )
{
   set("notes", "notes", var);
}

void Yeast::setBestFor( const QString& var )
{
   set("bestFor", "best_for", var);
}

void Yeast::setTimesCultured( int var )
{
   if( var < 0 )
      return;
   else
      set("timesCultured", "times_cultured", var);
}

void Yeast::setMaxReuse( int var )
{
   if( var < 0 )
      return;
   else
      set("maxReuse", "max_reuse", var);
}

void Yeast::setAddToSecondary( bool var )
{
   set("addToSecondary", "add_to_secondary", var);
}

//========================OTHER METHODS=========================================
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
