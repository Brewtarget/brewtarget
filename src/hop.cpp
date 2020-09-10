/*
 * hop.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Kregg K <gigatropolis@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDebug>
#include "hop.h"
#include "brewtarget.h"

#include "TableSchemaConst.h"
#include "HopSchema.h"

QStringList Hop::types = QStringList() << "Bittering" << "Aroma" << "Both";
QStringList Hop::forms = QStringList() << "Leaf" << "Pellet" << "Plug";
QStringList Hop::uses = QStringList() << "Mash" << "First Wort" << "Boil" << "Aroma" << "Dry Hop";

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

QString Hop::classNameStr()
{
   static const QString name("Hop");
   return name;
}

Hop::Hop(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString()),
     m_useStr(QString()),
     m_use(static_cast<Hop::Use>(0)),
     m_typeStr(QString()),
     m_type(static_cast<Hop::Type>(0)),
     m_formStr(QString()),
     m_form(static_cast<Hop::Form>(0)),
     m_alpha_pct(0.0),
     m_amount_kg(0.0),
     m_time_min(0.0),
     m_notes(QString()),
     m_beta_pct(0.0),
     m_hsi_pct(0.0),
     m_origin(QString()),
     m_substitutes(QString()),
     m_humulene_pct(0.0),
     m_caryophyllene_pct(0.0),
     m_cohumulone_pct(0.0),
     m_myrcene_pct(0.0),
     m_inventory(-1.0),
     m_inventory_id(0),
     m_cacheOnly(false)
{
}

Hop::Hop(QString name, bool cache)
   : Ingredient(Brewtarget::HOPTABLE, -1, name, true),
     m_useStr(QString()),
     m_use(static_cast<Hop::Use>(0)),
     m_typeStr(QString()),
     m_type(static_cast<Hop::Type>(0)),
     m_formStr(QString()),
     m_form(static_cast<Hop::Form>(0)),
     m_alpha_pct(0.0),
     m_amount_kg(0.0),
     m_time_min(0.0),
     m_notes(QString()),
     m_beta_pct(0.0),
     m_hsi_pct(0.0),
     m_origin(QString()),
     m_substitutes(QString()),
     m_humulene_pct(0.0),
     m_caryophyllene_pct(0.0),
     m_cohumulone_pct(0.0),
     m_myrcene_pct(0.0),
     m_inventory(-1.0),
     m_inventory_id(0),
     m_cacheOnly(cache)
{
}

Hop::Hop(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
     m_useStr(rec.value(kcolUse).toString()),
     m_use(static_cast<Hop::Use>(uses.indexOf(m_useStr))),
     m_typeStr(rec.value(kcolHopType).toString()),
     m_type(static_cast<Hop::Type>(types.indexOf(m_typeStr))),
     m_formStr(rec.value(kcolHopForm).toString()),
     m_form(static_cast<Hop::Form>(forms.indexOf(m_formStr))),
     m_alpha_pct(rec.value(kcolHopAlpha).toDouble()),
     m_amount_kg(rec.value(kcolHopAmount).toDouble()),
     m_time_min(rec.value(kcolTime).toDouble()),
     m_notes(rec.value(kcolNotes).toString()),
     m_beta_pct(rec.value(kcolHopBeta).toDouble()),
     m_hsi_pct(rec.value(kcolHopHSI).toDouble()),
     m_origin(rec.value(kcolOrigin).toString()),
     m_substitutes(rec.value(kcolSubstitutes).toString()),
     m_humulene_pct(rec.value(kcolHopHumulene).toDouble()),
     m_caryophyllene_pct(rec.value(kcolHopCaryophyllene).toDouble()),
     m_cohumulone_pct(rec.value(kcolHopCohumulone).toDouble()),
     m_myrcene_pct(rec.value(kcolHopMyrcene).toDouble()),
     m_inventory(-1.0),
     m_inventory_id(rec.value(kcolInventoryId).toInt()),
     m_cacheOnly(false)
{
}

Hop::Hop( Hop & other )
   : Ingredient(other),
     m_useStr(other.m_useStr),
     m_use(other.m_use),
     m_typeStr(other.m_typeStr),
     m_type(other.m_type),
     m_formStr(other.m_formStr),
     m_form(other.m_form),
     m_alpha_pct(other.m_alpha_pct),
     m_amount_kg(other.m_amount_kg),
     m_time_min(other.m_time_min),
     m_notes(other.m_notes),
     m_beta_pct(other.m_beta_pct),
     m_hsi_pct(other.m_hsi_pct),
     m_origin(other.m_origin),
     m_substitutes(other.m_substitutes),
     m_humulene_pct(other.m_humulene_pct),
     m_caryophyllene_pct(other.m_caryophyllene_pct),
     m_cohumulone_pct(other.m_cohumulone_pct),
     m_myrcene_pct(other.m_myrcene_pct),
     m_inventory(other.m_inventory),
     m_inventory_id(other.m_inventory_id),
     m_cacheOnly(other.m_cacheOnly)
{
}

//============================="SET" METHODS====================================
void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 )
   {
      Brewtarget::logW( QString("Hop: 0 < alpha < 100: %1").arg(num) );
      return;
   }
   else
   {
      m_alpha_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropAlpha, num);
      }
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
      m_amount_kg = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropAmountKg,num);
      }
   }
}

void Hop::setInventoryAmount( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Hop: inventory < 0: %1").arg(num) );
      return;
   }
   else
   {
      m_inventory = num;
      if ( ! m_cacheOnly ) {
         setInventory(num,m_inventory_id);
      }
   }
}

void Hop::setInventoryId( int key )
{
   m_inventory_id = key;
   if ( ! m_cacheOnly ) {
      setEasy(kpropInventoryId, key);
   }
}

void Hop::setUse(Use u)
{
   if ( u < uses.size()) {
      m_use = u;
      m_useStr = uses.at(u);
      if ( ! m_cacheOnly ) {
         setEasy(kpropUse, uses.at(u));
      }
   }
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
      m_time_min = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTime, num);
      }
   }
}

void Hop::setNotes( const QString& str )
{
   m_notes = str;
   if ( ! m_cacheOnly ) {
      setEasy(kpropNotes, str);
   }
}

void Hop::setType(Type t)
{
  if ( t < types.size() ) {
     m_type = t;
     m_typeStr = types.at(t);
     if ( ! m_cacheOnly ) {
      setEasy(kpropType, m_typeStr);
     }
  }
}

void Hop::setForm( Form f )
{
   if ( f < forms.size() ) {
      m_form = f;
      m_formStr = forms.at(f);
      if ( ! m_cacheOnly ) {
         setEasy(kpropForm, m_formStr);
      }
   }
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
      m_beta_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropBeta, num);
      }
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
      m_hsi_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropHSI, num);
      }
   }
}

void Hop::setOrigin( const QString& str )
{
   m_origin = str;
   if ( ! m_cacheOnly ) {
      setEasy(kpropOrigin, str);
   }
}

void Hop::setSubstitutes( const QString& str )
{
   m_substitutes = str;
   if ( ! m_cacheOnly ) {
      setEasy(kpropSubstitutes, str);
   }
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
      m_humulene_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropHumulene,num);
      }
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
      m_caryophyllene_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropCaryophyllene, num);
      }
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
      m_cohumulone_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropCohumulone, num);
      }
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
      m_myrcene_pct = num;
      if ( ! m_cacheOnly ) {
         setEasy(kpropMyrcene, num);
      }
   }
}

void Hop::setCacheOnly(bool cache) { m_cacheOnly = cache; }

//============================="GET" METHODS====================================

Hop::Use Hop::use() const { return m_use; }
const QString Hop::useString() const { return m_useStr; }
const QString Hop::notes() const { return m_notes; }
Hop::Type Hop::type() const { return m_type; }
const QString Hop::typeString() const { return m_typeStr; }
Hop::Form Hop::form() const { return m_form; }
const QString Hop::formString() const { return m_formStr; }
const QString Hop::origin() const { return m_origin; }
const QString Hop::substitutes() const { return m_substitutes; }
double Hop::alpha_pct() const { return m_alpha_pct; }
double Hop::amount_kg() const { return m_amount_kg; }
double Hop::time_min() const { return m_time_min; }
double Hop::beta_pct() const { return m_beta_pct; }
double Hop::hsi_pct() const { return m_hsi_pct; }
double Hop::humulene_pct() const { return m_humulene_pct; }
double Hop::caryophyllene_pct() const { return m_caryophyllene_pct; }
double Hop::cohumulone_pct() const { return m_cohumulone_pct; }
double Hop::myrcene_pct() const { return m_myrcene_pct; }
bool   Hop::cacheOnly() const { return m_cacheOnly; }

// a little different in that we don't get the results in advance, but on the fly. I had to undo some const action to make this work
double Hop::inventory()
{
   if ( m_inventory < 0 ) {
      m_inventory = getInventory(kpropInventory).toDouble();
   }
   return m_inventory;
}

int Hop::inventoryId() const
{
   return m_inventory_id;
}

const QString Hop::useStringTr() const
{
   static QStringList usesTr = QStringList() << tr("Mash") << tr("First Wort") << tr("Boil") << tr("Aroma") << tr("Dry Hop") ;
   if ( m_use < usesTr.size() && m_use >= 0 ) {
      return usesTr.at(m_use);
   }
   else {
      return "";
   }
}

const QString Hop::typeStringTr() const
{
   static QStringList typesTr = QStringList() << tr("Bittering") << tr("Aroma") << tr("Both");
   if ( m_type < typesTr.size()  && m_type >= 0 ) {
      return typesTr.at(m_type);
   }
   else {
      return "";
   }
}

const QString Hop::formStringTr() const
{
   static QStringList formsTr = QStringList() << tr("Leaf") << tr("Pellet") << tr("Plug");
   if ( m_form < formsTr.size() && m_form >= 0 ) {
      return formsTr.at(m_form);
   }
   else {
      return "";
   }
}

