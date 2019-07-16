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
   : BeerXMLElement(table, key, QString(), true ),
     m_typeString(QString()),
     m_type(static_cast<Yeast::Type>(0)),
     m_formString(QString()),
     m_form(static_cast<Yeast::Form>(0)),
     m_flocculationString(QString()),
     m_flocculation(static_cast<Yeast::Flocculation>(0)),
     m_amount(0.0),
     m_amountIsWeight(false),
     m_laboratory(QString()),
     m_productID(QString()),
     m_minTemperature_c(0.0),
     m_maxTemperature_c(0.0),
     m_attenuation_pct(0.0),
     m_notes(QString()),
     m_bestFor(QString()),
     m_timesCultured(0),
     m_maxReuse(0),
     m_addToSecondary(false)
{
}

Yeast::Yeast(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kName).toString(), rec.value(kDisplay).toBool()),
     m_typeString(rec.value(kType).toString()),
     m_type(static_cast<Yeast::Type>(types.indexOf(m_typeString))),
     m_formString(rec.value(kForm).toString()),
     m_form(static_cast<Yeast::Form>(forms.indexOf(m_formString))),
     m_flocculationString(rec.value(kFlocculation).toString()),
     m_flocculation(static_cast<Yeast::Flocculation>(flocculations.indexOf(m_flocculationString))),
     m_amount(rec.value(kAmount).toDouble()),
     m_amountIsWeight(rec.value(kAmountIsWeight).toBool()),
     m_laboratory(rec.value(kLab).toString()),
     m_productID(rec.value(kProductID).toString()),
     m_minTemperature_c(rec.value(kMinTemp).toDouble()),
     m_maxTemperature_c(rec.value(kMaxTemp).toDouble()),
     m_attenuation_pct(rec.value(kAttenuation).toDouble()),
     m_notes(rec.value(kNotes).toString()),
     m_bestFor(rec.value(kBestFor).toString()),
     m_timesCultured(rec.value(kTimesCultured).toInt()),
     m_maxReuse(rec.value(kMaxReuse).toInt()),
     m_addToSecondary(rec.value(kAddToSecondary).toBool())
{
}

Yeast::Yeast(Yeast const& other) : BeerXMLElement(other)
{
}

//============================="GET" METHODS====================================
QString Yeast::laboratory() const { return m_laboratory; }

QString Yeast::productID() const { return m_productID; }

QString Yeast::notes() const { return m_notes; }

QString Yeast::bestFor() const { return m_bestFor; }

const QString Yeast::typeString() const { return m_typeString; }

const QString Yeast::formString() const { return m_formString; }

const QString Yeast::flocculationString() const { return m_flocculationString; }

double Yeast::amount() const { return m_amount; }

double Yeast::minTemperature_c() const { return m_minTemperature_c; }

double Yeast::maxTemperature_c() const { return m_maxTemperature_c; }

double Yeast::attenuation_pct() const { return m_attenuation_pct; }

int Yeast::inventory() const { return getInventory(kInventory).toInt(); }

int Yeast::timesCultured() const { return m_timesCultured; }

int Yeast::maxReuse() const { return m_maxReuse; }

bool Yeast::addToSecondary() const { return m_addToSecondary; }

bool Yeast::amountIsWeight() const { return m_amountIsWeight; }

Yeast::Form Yeast::form() const { return  m_form; }

Yeast::Flocculation Yeast::flocculation() const { return m_flocculation; }

Yeast::Type Yeast::type() const { return m_type; }

const QString Yeast::typeStringTr() const
{
   static QStringList typesTr = QStringList() << QObject::tr("Ale")
                                       << QObject::tr("Lager")
                                       << QObject::tr("Wheat")
                                       << QObject::tr("Wine")
                                       << QObject::tr("Champagne");
   return typesTr.at(m_type);
}

const QString Yeast::formStringTr() const
{
   static QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   return formsTr.at(m_form);
}

const QString Yeast::flocculationStringTr() const
{
   static QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   return flocculationsTr.at(m_flocculation);
}

//============================="SET" METHODS====================================
void Yeast::setType( Yeast::Type t, bool cacheOnly )
{
   m_type = t;
   m_typeString = types.at(t);
   if ( ! cacheOnly ) {
      set(kTypeProp, kType, m_typeString);
   }
}

void Yeast::setForm( Yeast::Form f, bool cacheOnly )
{
   m_form = f;
   m_formString = forms.at(f);
   if ( ! cacheOnly ) {
      set(kFormProp, kForm, m_formString);
   }
}

void Yeast::setAmount( double var, bool cacheOnly )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: amount < 0: %1").arg(var) );
   else {
      m_amount = var;
      if ( ! cacheOnly ) {
         set(kAmountProp, kAmount, var);
      }
   }
}

void Yeast::setInventoryQuanta( int var, bool cacheOnly )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: inventory < 0: %1").arg(var) );
   else
      if ( ! cacheOnly ) {
         setInventory(kInventoryProp, kInventory, var);
      }
}

void Yeast::setAmountIsWeight( bool var, bool cacheOnly )
{
   m_amountIsWeight = var;
   if ( ! cacheOnly ) {
      set(kAmountIsWeightProp, kAmountIsWeight, var);
   }
}

void Yeast::setLaboratory( const QString& var, bool cacheOnly )
{
   m_laboratory = var;
   if ( ! cacheOnly ) {
      set(kLabProp, kLab, var);
   }
}

void Yeast::setProductID( const QString& var, bool cacheOnly )
{
   m_productID = var;
   if ( ! cacheOnly ) {
      set(kProductIDProp, kProductID, var);
   }
}

void Yeast::setMinTemperature_c( double var, bool cacheOnly )
{
   if( var < -273.15 )
      return;
   else {
      m_minTemperature_c = var;
      if ( ! cacheOnly ) {
         set(kMinTempProp, kMinTemp, var);
      }
   }
}

void Yeast::setMaxTemperature_c( double var, bool cacheOnly )
{
   if( var < -273.15 )
      return;
   else {
      m_maxTemperature_c = var;
      if ( ! cacheOnly ) {
         set(kMaxTempProp, kMaxTemp, var);
      }
   }
}

// Remember -- always make sure the value is in range before we set
// coredumps happen otherwise
void Yeast::setFlocculation( Yeast::Flocculation f, bool cacheOnly)
{
   if ( flocculations.at(f) > 0 ) {
      m_flocculation = f;
      m_flocculationString = flocculations.at(f);

      if ( ! cacheOnly ) {
         set(kFlocculationProp, kFlocculation, flocculations.at(f));
      }
   }
}

void Yeast::setAttenuation_pct( double var, bool cacheOnly )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else {
      m_attenuation_pct = var;
      if ( ! cacheOnly ) {
         set(kAttenuationProp, kAttenuation, var);
      }
   }
}

void Yeast::setNotes( const QString& var, bool cacheOnly )
{
   m_notes = var;
   if ( ! cacheOnly ) {
      set(kNotesProp, kNotes, var);
   }
}

void Yeast::setBestFor( const QString& var, bool cacheOnly )
{
   m_bestFor = var;
   if ( ! cacheOnly ) {
      set(kBestForProp, kBestFor, var);
   }
}

void Yeast::setTimesCultured( int var, bool cacheOnly )
{
   if( var < 0 )
      return;
   else {
      m_timesCultured = var;
      if ( ! cacheOnly ) {
         set(kTimesCulturedProp, kTimesCultured, var);
      }
   }
}

void Yeast::setMaxReuse( int var, bool cacheOnly )
{
   if( var < 0 )
      return;
   else {
      m_maxReuse = var;
      if ( ! cacheOnly ) {
         set(kMaxReuseProp, kMaxReuse, var);
      }
   }
}

void Yeast::setAddToSecondary( bool var, bool cacheOnly )
{
   m_addToSecondary = var;
   if ( ! cacheOnly ) {
      set(kAddToSecondaryProp, kAddToSecondary, var);
   }
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
