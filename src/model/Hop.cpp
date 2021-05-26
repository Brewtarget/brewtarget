/*
 * model/Hop.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/Hop.h"

#include <QDebug>
#include <QDomElement>
#include <QDomText>
#include <QObject>

#include "brewtarget.h"
#include "database.h"
#include "HopSchema.h"
#include "TableSchemaConst.h"

QStringList Hop::types = QStringList() << "Bittering" << "Aroma" << "Both";
QStringList Hop::forms = QStringList() << "Leaf" << "Pellet" << "Plug";
QStringList Hop::uses = QStringList() << "Mash" << "First Wort" << "Boil" << "Aroma" << "Dry Hop";

bool Hop::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Hop const & rhs = static_cast<Hop const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_use               == rhs.m_use               &&
      this->m_type              == rhs.m_type              &&
      this->m_form              == rhs.m_form              &&
      this->m_alpha_pct         == rhs.m_alpha_pct         &&
      this->m_beta_pct          == rhs.m_beta_pct          &&
      this->m_hsi_pct           == rhs.m_hsi_pct           &&
      this->m_origin            == rhs.m_origin            &&
      this->m_humulene_pct      == rhs.m_humulene_pct      &&
      this->m_caryophyllene_pct == rhs.m_caryophyllene_pct &&
      this->m_cohumulone_pct    == rhs.m_cohumulone_pct    &&
      this->m_myrcene_pct       == rhs.m_myrcene_pct
   );
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

Hop::Hop(QString name, bool cache)
   : NamedEntity(Brewtarget::HOPTABLE, name, true),
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

Hop::Hop(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key),
     m_inventory(-1.0),
     m_cacheOnly(false)
{
     m_useStr = rec.value(table->propertyToColumn(PropertyNames::Hop::use)).toString();
     m_typeStr = rec.value(table->propertyToColumn(PropertyNames::Hop::type)).toString();
     m_formStr = rec.value(table->propertyToColumn(PropertyNames::Hop::form)).toString();
     m_alpha_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::alpha_pct)).toDouble();
     m_amount_kg = rec.value(table->propertyToColumn(PropertyNames::Hop::amount_kg)).toDouble();
     m_time_min = rec.value(table->propertyToColumn(PropertyNames::Hop::time_min)).toDouble();
     m_notes = rec.value(table->propertyToColumn(PropertyNames::Hop::notes)).toString();
     m_beta_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::beta_pct)).toDouble();
     m_hsi_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::hsi_pct)).toDouble();
     m_origin = rec.value(table->propertyToColumn(PropertyNames::Hop::origin)).toString();
     m_substitutes = rec.value(table->propertyToColumn(PropertyNames::Hop::substitutes)).toString();
     m_humulene_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::humulene_pct)).toDouble();
     m_caryophyllene_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::caryophyllene_pct)).toDouble();
     m_cohumulone_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::cohumulone_pct)).toDouble();
     m_myrcene_pct = rec.value(table->propertyToColumn(PropertyNames::Hop::myrcene_pct)).toDouble();

     // keys need special handling
     m_inventory_id = rec.value(table->foreignKeyToColumn(PropertyNames::Hop::inventory_id)).toInt();

     // these are not taken directly from the SQL record
     m_use  = static_cast<Hop::Use>(uses.indexOf(m_useStr));
     m_type = static_cast<Hop::Type>(types.indexOf(m_typeStr));
     m_form = static_cast<Hop::Form>(forms.indexOf(m_formStr));
}

Hop::Hop( Hop & other )
   : NamedEntity(other),
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
//
void Hop::setAlpha_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < alpha < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_alpha_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::alpha_pct, num) ) {
      m_alpha_pct = num;
      signalCacheChange(PropertyNames::Hop::alpha_pct, num);
   }
}

void Hop::setAmount_kg( double num )
{
   if( num < 0.0 ) {
      qWarning() << "Hop: amount < 0:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_amount_kg = num;
   }
   else if ( setEasy(PropertyNames::Hop::amount_kg,num) ) {
      m_amount_kg = num;
      signalCacheChange(PropertyNames::Hop::amount_kg,num);
   }
}

void Hop::setInventoryAmount( double num )
{
   if( num < 0.0 ) {
      qWarning() << "Hop: inventory < 0:" << num;
      return;
   }

   m_inventory = num;
   if ( ! m_cacheOnly ) {
      setInventory(num,m_inventory_id);
   }
}

void Hop::setInventoryId( int key )
{
   m_inventory_id = key;
   if ( ! m_cacheOnly ) {
      setEasy(PropertyNames::Hop::inventory_id, key);
   }
}

void Hop::setUse(Use u)
{
   if ( u >= uses.size() ) {
      qWarning() << "Hop: unrecognized use:" << u;
      return;
   }

   if ( m_cacheOnly ) {
      m_use = u;
      m_useStr = uses.at(u);
   }
   else if ( setEasy(PropertyNames::Hop::use, uses.at(u)) ) {
      m_use = u;
      m_useStr = uses.at(u);
      signalCacheChange(PropertyNames::Hop::use, uses.at(u));
   }
}

void Hop::setTime_min( double num )
{
   if( num < 0.0 ) {
      qWarning() << "Hop: time < 0:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_time_min = num;
   }
   else if ( setEasy(PropertyNames::Hop::time_min, num) ) {
      m_time_min = num;
      signalCacheChange(PropertyNames::Hop::time_min, num);
   }
}

void Hop::setNotes( const QString& str )
{
   if ( m_cacheOnly ) {
      m_notes = str;
   }
   else if ( setEasy(PropertyNames::Hop::notes, str) ) {
      m_notes = str;
      signalCacheChange(PropertyNames::Hop::notes, str);
   }
}

void Hop::setType(Type t)
{
   if ( t >= types.size() ) {
      qWarning() << "Hop: unrecognized type:" << t;
      return;
   }

   if ( m_cacheOnly ) {
      m_type = t;
      m_typeStr = types.at(t);
   }
   else if ( setEasy(PropertyNames::Hop::type, m_typeStr) ) {
      m_type = t;
      m_typeStr = types.at(t);
      signalCacheChange(PropertyNames::Hop::type, m_typeStr);
   }
}

void Hop::setForm( Form f )
{
   if ( f >= forms.size() ) {
      qWarning() << "Hop: unrecognized form:" << f;
      return;
   }

   if ( m_cacheOnly ) {
      m_form = f;
      m_formStr = forms.at(f);
   }
   else if ( setEasy(PropertyNames::Hop::form, m_formStr) ) {
      m_form = f;
      m_formStr = forms.at(f);
      signalCacheChange(PropertyNames::Hop::form, m_formStr);
   }
}

void Hop::setBeta_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < beta < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_beta_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::beta_pct, num) ) {
      m_beta_pct = num;
      signalCacheChange(PropertyNames::Hop::beta_pct, num);
   }
}

void Hop::setHsi_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < hsi < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_hsi_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::hsi_pct, num) ) {
      m_hsi_pct = num;
      signalCacheChange(PropertyNames::Hop::hsi_pct, num);
   }
}

void Hop::setOrigin( const QString& str )
{
   if ( m_cacheOnly ) {
      m_origin = str;
   }
   else if ( setEasy(PropertyNames::Hop::origin, str) ) {
      m_origin = str;
      signalCacheChange(PropertyNames::Hop::origin, str);
   }
}

void Hop::setSubstitutes( const QString& str )
{
   if ( m_cacheOnly ) {
      m_substitutes = str;
   }
   else if ( setEasy(PropertyNames::Hop::substitutes, str) ) {
      m_substitutes = str;
      signalCacheChange(PropertyNames::Hop::substitutes, str);
   }
}

void Hop::setHumulene_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < humulene < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_humulene_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::humulene_pct,num) ) {
      m_humulene_pct = num;
      signalCacheChange(PropertyNames::Hop::humulene_pct,num);
   }
}

void Hop::setCaryophyllene_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < cary < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_caryophyllene_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::caryophyllene_pct, num) ) {
      m_caryophyllene_pct = num;
      signalCacheChange(PropertyNames::Hop::caryophyllene_pct, num);
   }
}

void Hop::setCohumulone_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < cohumulone < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_cohumulone_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::cohumulone_pct, num) ) {
      m_cohumulone_pct = num;
      signalCacheChange(PropertyNames::Hop::cohumulone_pct, num);
   }
}

void Hop::setMyrcene_pct( double num )
{
   if( num < 0.0 || num > 100.0 ) {
      qWarning() << "Hop: 0 < myrcene < 100:" << num;
      return;
   }

   if ( m_cacheOnly ) {
      m_myrcene_pct = num;
   }
   else if ( setEasy(PropertyNames::Hop::myrcene_pct, num) ) {
      m_myrcene_pct = num;
      signalCacheChange(PropertyNames::Hop::myrcene_pct, num);
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
      m_inventory = getInventory().toDouble();
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

NamedEntity * Hop::getParent() {
   Hop * myParent = nullptr;

   // If we don't already know our parent, look it up
   if (!this->parentKey) {
      this->parentKey = Database::instance().getParentNamedEntityKey(*this);
   }

   // If we (now) know our parent, get a pointer to it
   if (this->parentKey) {
      myParent = Database::instance().hop(this->parentKey);
   }

   // Return whatever we got
   return myParent;
}

int Hop::insertInDatabase() {
   return Database::instance().insertHop(this);
}

void Hop::removeFromDatabase() {
   Database::instance().remove(this);
}
