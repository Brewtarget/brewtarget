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

/************* Columns *************/
const QString kName("name");
const QString kType("ytype");
const QString kForm("form");
const QString kAmount("amount");
const QString kInventory("quanta");
const QString kAmountIsWeight("amount_is_weight");
const QString kLab("laboratory");
const QString kProductID("product_id");
const QString kMinTemp("min_temperature");
const QString kMaxTemp("max_temperature");
const QString kFlocculation("flocculation");
const QString kAttenuation("attenuation");
const QString kNotes("notes");
const QString kBestFor("best_for");
const QString kTimesCultured("times_cultured");
const QString kMaxReuse("max_reuse");
const QString kAddToSecondary("add_to_secondary");

// these are defined in the parent, but I need them here too
const QString kDeleted("deleted");
const QString kDisplay("display");
const QString kFolder("folder");
/************** Props **************/
const QString kNameProp("name");
const QString kTypeProp("type");
const QString kFormProp("form");
const QString kAmountProp("amount");
const QString kInventoryProp("inventory");
const QString kAmountIsWeightProp("amountIsWeight");
const QString kLabProp("laboratory");
const QString kProductIDProp("productID");
const QString kMinTempProp("minTemperature_c");
const QString kMaxTempProp("maxTemperature_c");
const QString kFlocculationProp("flocculation");
const QString kAttenuationProp("attenuation_pct");
const QString kNotesProp("notes");
const QString kBestForProp("bestFor");
const QString kTimesCulturedProp("timesCultured");
const QString kMaxReuseProp("maxReuse");
const QString kAddToSecondaryProp("addToSecondary");


QStringList Yeast::types = QStringList() << "Ale" << "Lager" << "Wheat" << "Wine" << "Champagne";
QStringList Yeast::forms = QStringList() << "Liquid" << "Dry" << "Slant" << "Culture";
QStringList Yeast::flocculations = QStringList() << "Low" << "Medium" << "High" << "Very High";
QHash<QString,QString> Yeast::tagToProp = Yeast::tagToPropHash();

QHash<QString,QString> Yeast::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   //propHash["TYPE"] = kTypeProp;
   //propHash["FORM"] = kFormProp";
   propHash["AMOUNT"] = kAmountProp;
   propHash["INVENTORY"] = kInventoryProp;
   propHash["AMOUNT_IS_WEIGHT"] = kAmountIsWeightProp;
   propHash["LABORATORY"] = kLabProp;
   propHash["PRODUCT_ID"] = kProductIDProp;
   propHash["MIN_TEMPERATURE"] = kMinTempProp;
   propHash["MAX_TEMPERATURE"] = kMaxTempProp;
   //propHash["FLOCCULATION"] = kFlocculationProp;
   propHash["ATTENUATION"] = kAttenuationProp;
   propHash["NOTES"] = kNotesProp;
   propHash["BEST_FOR"] = kBestForProp;
   propHash["TIMES_CULTURED"] = kTimesCulturedProp;
   propHash["MAX_REUSE"] = kMaxReuseProp;
   propHash["ADD_TO_SECONDARY"] = kAddToSecondaryProp;
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

QString Yeast::classNameStr()
{
   static const QString name("Yeast");
   return name;
}

//============================CONSTRUCTORS======================================
Yeast::Yeast(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

Yeast::Yeast(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   setName( rec.value(kName).toString(), true );
   setDisplay( rec.value(kDisplay).toBool(), true);
   setDeleted( rec.value(kDeleted).toBool(), true);
   setFolder( rec.value(kFolder).toString(), false, true);
   _typeString = rec.value(kType).toString();
   _type = static_cast<Yeast::Type>(types.indexOf(_typeString));
   _formString = rec.value(kForm).toString();
   _form = static_cast<Yeast::Form>(forms.indexOf(_formString));
   _flocculationString = rec.value(kFlocculation).toString();
   _flocculation = static_cast<Yeast::Flocculation>(flocculations.indexOf(_flocculationString));
   _laboratory = rec.value(kLab).toString();
   _productID = rec.value(kProductID).toString();
   _notes = rec.value(kNotes).toString();
   _bestFor = rec.value(kBestFor).toString();
   _amount = rec.value(kAmount).toDouble();
   _minTemperature_c = rec.value(kMinTemp).toDouble();
   _maxTemperature_c = rec.value(kMaxTemp).toDouble();
   _attenuation_pct = rec.value(kAttenuation).toDouble();
   _timesCultured = rec.value(kTimesCultured).toInt();
   _maxReuse = rec.value(kMaxReuse).toInt();
   _addToSecondary = rec.value(kAddToSecondary).toBool();
   _amountIsWeight = rec.value(kAmountIsWeight).toBool();
}

Yeast::Yeast(Yeast const& other) : BeerXMLElement(other)
{
}

//============================="GET" METHODS====================================
QString Yeast::laboratory() const { return _laboratory; }

QString Yeast::productID() const { return _productID; }

QString Yeast::notes() const { return _notes; }

QString Yeast::bestFor() const { return _bestFor; }

const QString Yeast::typeString() const { return _typeString; }

const QString Yeast::formString() const { return _formString; }

const QString Yeast::flocculationString() const { return _flocculationString; }

double Yeast::amount() const { return _amount; }

double Yeast::minTemperature_c() const { return _minTemperature_c; }

double Yeast::maxTemperature_c() const { return _maxTemperature_c; }

double Yeast::attenuation_pct() const { return _attenuation_pct; }

int Yeast::inventory() const { return getInventory(kInventory).toInt(); }

int Yeast::timesCultured() const { return _timesCultured; }

int Yeast::maxReuse() const { return _maxReuse; }

bool Yeast::addToSecondary() const { return _addToSecondary; }

bool Yeast::amountIsWeight() const { return _amountIsWeight; }

Yeast::Form Yeast::form() const { return  _form; }

Yeast::Flocculation Yeast::flocculation() const { return _flocculation; }

Yeast::Type Yeast::type() const { return _type; }

const QString Yeast::typeStringTr() const
{
   static QStringList typesTr = QStringList() << QObject::tr("Ale")
                                       << QObject::tr("Lager")
                                       << QObject::tr("Wheat")
                                       << QObject::tr("Wine")
                                       << QObject::tr("Champagne");
   return typesTr.at(_type);
}

const QString Yeast::formStringTr() const
{
   static QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   return formsTr.at(_form);
}

const QString Yeast::flocculationStringTr() const
{
   static QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   return flocculationsTr.at(_flocculation);
}

//============================="SET" METHODS====================================
void Yeast::setType( Yeast::Type t )
{
   _type = t;
   _typeString = types.at(t);
   set(kTypeProp, kType, _typeString);
}

void Yeast::setForm( Yeast::Form f )
{
   _form = f;
   _formString = forms.at(f);
   set(kFormProp, kForm, _formString);
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: amount < 0: %1").arg(var) );
   else {
      _amount = var;
      set(kAmountProp, kAmount, var);
   }
}

void Yeast::setInventoryQuanta( int var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: inventory < 0: %1").arg(var) );
   else
      setInventory(kInventoryProp, kInventory, var);
}

void Yeast::setAmountIsWeight( bool var )
{
   _amountIsWeight = var;
   set(kAmountIsWeightProp, kAmountIsWeight, var);
}

void Yeast::setLaboratory( const QString& var )
{
   _laboratory = var;
   set(kLabProp, kLab, var);
}

void Yeast::setProductID( const QString& var )
{
   _productID = var;
   set(kProductIDProp, kProductID, var);
}

void Yeast::setMinTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else {
      _minTemperature_c = var;
      set(kMinTempProp, kMinTemp, var);
   }
}

void Yeast::setMaxTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else {
      _maxTemperature_c = var;
      set(kMaxTempProp, kMaxTemp, var);
   }
}

void Yeast::setFlocculation( Yeast::Flocculation f )
{
   set(kFlocculationProp, kFlocculation, flocculations.at(f));
}

void Yeast::setAttenuation_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else {
      _attenuation_pct = var;
      set(kAttenuationProp, kAttenuation, var);
   }
}

void Yeast::setNotes( const QString& var )
{
   _notes = var;
   set(kNotesProp, kNotes, var);
}

void Yeast::setBestFor( const QString& var )
{
   _bestFor = var;
   set(kBestForProp, kBestFor, var);
}

void Yeast::setTimesCultured( int var )
{
   if( var < 0 )
      return;
   else {
      _timesCultured = var;
      set(kTimesCulturedProp, kTimesCultured, var);
   }
}

void Yeast::setMaxReuse( int var )
{
   if( var < 0 )
      return;
   else {
      _maxReuse = var;
      set(kMaxReuseProp, kMaxReuse, var);
   }
}

void Yeast::setAddToSecondary( bool var )
{
   _addToSecondary = var;
   set(kAddToSecondaryProp, kAddToSecondary, var);
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
