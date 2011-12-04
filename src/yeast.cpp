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
   return y1.name() < y2.name();
}

bool operator==(Yeast &y1, Yeast &y2)
{
   return y1.name() == y2.name();
}

//============================CONSTRUCTORS======================================
Yeast::Yeast()
{
   setDefaults();
}

Yeast::Yeast(Yeast& other) : BeerXMLElement(other)
{
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

//============================="GET" METHODS====================================
QString Yeast::name() const
{
   return get("name").toString();
}

Yeast::Type Yeast::type() const {
   return get("name").toInt();
}

const QString Yeast::typeString() const
{
   return types.at(type());
}

const QString Yeast::typeStringTr() const
{
   QStringList typesTr = QStringList() << QObject::tr("Ale")
                                       << QObject::tr("Lager")
                                       << QObject::tr("Wheat")
                                       << QObject::tr("Wine")
                                       << QObject::tr("Champagne");
   return typesTr.at(type());
}

Yeast::Form Yeast::form() const
{
   return get("form").toInt();
}

const QString Yeast::formString() const
{
   return forms.at(form());
}

const QString Yeast::formStringTr() const
{
   QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   return formsTr.at(form());
}

double Yeast::amount() const
{
   return get("amount").toDouble();
}

bool Yeast::amountIsWeight() const
{
   return get("amount_is_weight").toBool();
}

QString Yeast::laboratory() const
{
   return get("laboratory").toString();;
}

QString Yeast::productID() const
{
   return get("product_id").toString();
}

double Yeast::minTemperature_c() const
{
   return get("min_temperature").toDouble();
}

double Yeast::maxTemperature_c() const
{
   return get("max_temperature").toDouble();
}

Yeast::Flocculation Yeast::flocculation() const
{
   return get("flocculation").toInt();
}

const QString Yeast::flocculationString() const
{
   return flocculations.at(flocculation());
}

const QString Yeast::flocculationStringTr() const
{
   QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   return flocculationsTr.at(flocculation());
}

double Yeast::attenuation_pct() const
{
   return get("attenuation_pct").toDouble();
}

QString Yeast::notes() const
{
   return get("notes").toString();
}

QString Yeast::bestFor() const
{
   return get("best_for").toString();
}

int Yeast::timesCultured() const
{
   return get("times_cultured").toInt();
}

int Yeast::maxReuse() const
{
   return get("max_reuse").toInt();
}

bool Yeast::addToSecondary() const
{
   return get("add_to_secondary").toBool();
}

//============================="SET" METHODS====================================
void Yeast::setName( const QString& var )
{
   set("name", "name", var);
}

void Yeast::setType( Yeast::Type t )
{
   set("type", "ytype", static_cast<int>(t));
}

void Yeast::setForm( Yeast::Form f )
{
   set("form", "form", static_cast<int>(f));
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: amount < 0: %1").arg(var) );
   else
      set("amount", "amount", var);
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
   set("flocculation", "flocculation", static_cast<int>(f));
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
