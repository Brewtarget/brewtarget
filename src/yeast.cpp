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
#include <QDebug>
#include "yeast.h"
#include "brewtarget.h"

#include "TableSchemaConst.h"
#include "YeastSchema.h"

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

QString Yeast::classNameStr()
{
   static const QString name("Yeast");
   return name;
}

//============================CONSTRUCTORS======================================
Yeast::Yeast(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString(), true ),
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
     m_addToSecondary(false),
     m_inventory(-1.0),
     m_inventory_id(0),
     m_cacheOnly(false)
{
}

Yeast::Yeast(QString name, bool cache )
   : Ingredient(Brewtarget::YEASTTABLE, -1, name, true ),
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
     m_addToSecondary(false),
     m_inventory(-1),
     m_inventory_id(0),
     m_cacheOnly(cache)
{
}

Yeast::Yeast(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
     m_typeString(rec.value(kcolYeastType).toString()),
     m_type(static_cast<Yeast::Type>(types.indexOf(m_typeString))),
     m_formString(rec.value(kcolYeastForm).toString()),
     m_form(static_cast<Yeast::Form>(forms.indexOf(m_formString))),
     m_flocculationString(rec.value(kcolYeastFloc).toString()),
     m_flocculation(static_cast<Yeast::Flocculation>(flocculations.indexOf(m_flocculationString))),
     m_amount(rec.value(kcolYeastAmount).toDouble()),
     m_amountIsWeight(rec.value(kcolYeastAmtIsWgt).toBool()),
     m_laboratory(rec.value(kcolYeastLab).toString()),
     m_productID(rec.value(kcolYeastProductID).toString()),
     m_minTemperature_c(rec.value(kcolYeastMinTemp).toDouble()),
     m_maxTemperature_c(rec.value(kcolYeastMaxTemp).toDouble()),
     m_attenuation_pct(rec.value(kcolYeastAtten).toDouble()),
     m_notes(rec.value(kcolNotes).toString()),
     m_bestFor(rec.value(kcolYeastBestFor).toString()),
     m_timesCultured(rec.value(kcolYeastTimesCultd).toInt()),
     m_maxReuse(rec.value(kcolYeastMaxReuse).toInt()),
     m_addToSecondary(rec.value(kcolYeastAddToSec).toBool()),
     m_inventory(-1),
     m_inventory_id(rec.value(kcolInventoryId).toInt()),
     m_cacheOnly(false)
{
}

Yeast::Yeast(Yeast & other) : Ingredient(other),
     m_typeString(other.m_typeString),
     m_type(other.m_type),
     m_formString(other.m_formString),
     m_form(other.m_form),
     m_flocculationString(other.m_flocculationString),
     m_flocculation(other.m_flocculation),
     m_amount(other.m_amount),
     m_amountIsWeight(other.m_amountIsWeight),
     m_laboratory(other.m_laboratory),
     m_productID(other.m_productID),
     m_minTemperature_c(other.m_minTemperature_c),
     m_maxTemperature_c(other.m_maxTemperature_c),
     m_attenuation_pct(other.m_attenuation_pct),
     m_notes(other.m_notes),
     m_bestFor(other.m_bestFor),
     m_timesCultured(other.m_timesCultured),
     m_maxReuse(other.m_maxReuse),
     m_addToSecondary(other.m_addToSecondary),
     m_inventory(other.m_inventory),
     m_inventory_id(other.m_inventory_id),
     m_cacheOnly(other.m_cacheOnly)
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

int Yeast::inventory() {
   if ( m_inventory < 0 ) {
      m_inventory = getInventory(kpropInventory).toInt();
   }
   return m_inventory;
}

int Yeast::inventoryId() const { return m_inventory_id; }

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

   if ( m_type < typesTr.size() && m_type >= 0 ) {
      return typesTr.at(m_type);
   }
   else {
      return typesTr.at(0);
   }
}

const QString Yeast::formStringTr() const
{
   static QStringList formsTr = QStringList() << QObject::tr("Liquid")
                                       << QObject::tr("Dry")
                                       << QObject::tr("Slant")
                                       << QObject::tr("Culture");
   if ( m_form < formsTr.size() && m_form >= 0  ) {
      return formsTr.at(m_form);
   }
   else {
      return formsTr.at(0);
   }
}

const QString Yeast::flocculationStringTr() const
{
   static QStringList flocculationsTr = QStringList() << QObject::tr("Low")
                                               << QObject::tr("Medium")
                                               << QObject::tr("High")
                                               << QObject::tr("Very High");
   if ( m_flocculation < flocculationsTr.size() && m_flocculation >= 0 ) {
      return flocculationsTr.at(m_flocculation);
   }
   else {
      return flocculationsTr.at(0);
   }
}

bool Yeast::cacheOnly() const { return m_cacheOnly; }

//============================="SET" METHODS====================================
void Yeast::setType( Yeast::Type t )
{
   m_type = t;
   m_typeString = types.at(t);
   if ( ! m_cacheOnly ) {
      setEasy(kpropType, m_typeString);
   }
}

void Yeast::setForm( Yeast::Form f )
{
   m_form = f;
   m_formString = forms.at(f);
   if ( ! m_cacheOnly ) {
      setEasy(kpropForm, m_formString);
   }
}

void Yeast::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Yeast: amount < 0: %1").arg(var) );
   else {
      m_amount = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropAmount, var);
      }
   }
}

void Yeast::setInventoryQuanta( int var )
{
   if( var < 0.0 ) {
      Brewtarget::logW( QString("Yeast: inventory < 0: %1").arg(var) );
   }
   else {
      m_inventory = var;
      if ( ! m_cacheOnly ) {
         setInventory(var,m_inventory_id);
      }
   }
}

void Yeast::setInventoryId( int key )
{
   m_inventory_id = key;
   if ( ! m_cacheOnly ) {
      setEasy(kpropInventoryId, key);
   }
}

void Yeast::setAmountIsWeight( bool var )
{
   m_amountIsWeight = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAmtIsWgt, var);
   }
}

void Yeast::setLaboratory( const QString& var )
{
   m_laboratory = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropLab, var);
   }
}

void Yeast::setProductID( const QString& var )
{
   m_productID = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropProductID, var);
   }
}

void Yeast::setMinTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else {
      m_minTemperature_c = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropMinTemp, var);
      }
   }
}

void Yeast::setMaxTemperature_c( double var )
{
   if( var < -273.15 )
      return;
   else {
      m_maxTemperature_c = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropMaxTemp, var);
      }
   }
}

// Remember -- always make sure the value is in range before we set
// coredumps happen otherwise
void Yeast::setFlocculation( Yeast::Flocculation f)
{
   if ( flocculations.at(f) != nullptr ) {
      m_flocculation = f;
      m_flocculationString = flocculations.at(f);

      if ( ! m_cacheOnly ) {
         setEasy(kpropFloc, flocculations.at(f));
      }
   }
}

void Yeast::setAttenuation_pct( double var )
{
   if( var < 0.0 || var > 100.0 )
      return;
   else {
      m_attenuation_pct = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropAttenPct, var);
      }
   }
}

void Yeast::setNotes( const QString& var )
{
   m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropNotes, var);
   }
}

void Yeast::setBestFor( const QString& var )
{
   m_bestFor = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropBestFor, var);
   }
}

void Yeast::setTimesCultured( int var )
{
   if( var < 0 )
      return;
   else {
      m_timesCultured = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTimesCultd, var);
      }
   }
}

void Yeast::setMaxReuse( int var )
{
   if( var < 0 )
      return;
   else {
      m_maxReuse = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropMaxReuse, var);
      }
   }
}

void Yeast::setAddToSecondary( bool var )
{
   m_addToSecondary = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAddToSec, var);
   }
}

void Yeast::setCacheOnly(bool cache) { m_cacheOnly = cache; }

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
