/*
 * model/Misc.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "model/Misc.h"

#include <iostream>
#include <string>

#include <QDebug>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QVector>

#include "brewtarget.h"
#include "database.h"
#include "MiscSchema.h"
#include "TableSchemaConst.h"

QStringList Misc::uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
QStringList Misc::types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";
QStringList Misc::amountTypes = QStringList() << "Weight" << "Volume";

QString Misc::classNameStr()
{
   static const QString name("Misc");
   return name;
}

bool Misc::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Misc const & rhs = static_cast<Misc const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type == rhs.m_type &&
      this->m_use  == rhs.m_use
   );
}

//============================CONSTRUCTORS======================================

Misc::Misc(TableSchema* table, QSqlRecord rec, int t_key) :
   NamedEntityWithInventory(table, rec, t_key) {
   m_typeString = rec.value( table->propertyToColumn( PropertyNames::Misc::type)).toString();
   m_useString = rec.value( table->propertyToColumn( PropertyNames::Misc::use)).toString();
   m_time = rec.value( table->propertyToColumn( PropertyNames::Misc::time)).toDouble();
   m_amount = rec.value( table->propertyToColumn( PropertyNames::Misc::amount)).toDouble();
   m_amountIsWeight = rec.value( table->propertyToColumn( PropertyNames::Misc::amountIsWeight)).toBool();
   m_useFor = rec.value( table->propertyToColumn( PropertyNames::Misc::useFor)).toString();
   m_notes = rec.value( table->propertyToColumn( PropertyNames::Misc::notes)).toString();

   // not read from the db
   m_type = static_cast<Misc::Type>(types.indexOf(m_typeString));
   m_use = static_cast<Misc::Use>(uses.indexOf(m_useString));
}

Misc::Misc(Misc const & other) : NamedEntityWithInventory(other),
   m_typeString(other.m_typeString),
   m_type(other.m_type),
   m_useString(other.m_useString),
   m_use(other.m_use),
   m_time(other.m_time),
   m_amount(other.m_amount),
   m_amountIsWeight(other.m_amountIsWeight),
   m_useFor(other.m_useFor),
   m_notes(other.m_notes) {
   return;
}

Misc::Misc(QString name, bool cache) :
   NamedEntityWithInventory(Brewtarget::MISCTABLE, cache, name, true),
   m_typeString(QString()),
   m_type(static_cast<Misc::Type>(0)),
   m_useString(QString()),
   m_use(static_cast<Misc::Use>(0)),
   m_time(0.0),
   m_amount(0.0),
   m_amountIsWeight(false),
   m_useFor(QString()),
   m_notes(QString()) {
   return;
}

//============================"GET" METHODS=====================================
Misc::Type Misc::type() const { return m_type; }

const QString Misc::typeString() const { return m_typeString; }

Misc::Use Misc::use() const { return m_use; }

const QString Misc::useString() const { return m_useString; }

double Misc::amount() const { return m_amount; }

double Misc::time() const { return m_time; }

bool Misc::amountIsWeight() const { return m_amountIsWeight; }

QString Misc::useFor() const { return m_useFor; }

QString Misc::notes() const { return m_notes; }

Misc::AmountType Misc::amountType() const { return m_amountIsWeight ? AmountType_Weight : AmountType_Volume; }

const QString Misc::amountTypeString() const { return amountTypes.at(amountType()); }

const QString Misc::typeStringTr() const
{
   QStringList typesTr = QStringList() << tr("Spice") << tr("Fining") << tr("Water Agent") << tr("Herb") << tr("Flavor") << tr("Other");
   if ( m_type >=  Spice && m_type < typesTr.size()  ) {
      return typesTr.at(m_type);
   }
   else {
      return QString("Spice");
   }
}

const QString Misc::useStringTr() const
{
   QStringList usesTr = QStringList() << tr("Boil") << tr("Mash") << tr("Primary") << tr("Secondary") << tr("Bottling");
   if ( m_use >= Boil && m_use < usesTr.size() ) {
      return usesTr.at(use());
   }
   else {
      return QString("Boil");
   }
}

const QString Misc::amountTypeStringTr() const
{
   QStringList amountTypesTr = QStringList() << tr("Weight") << tr("Volume");
   if ( amountType() ) {
      return amountTypesTr.at(amountType());
   }
   else {
      return QString("Weight");
   }
}

//============================"SET" METHODS=====================================
void Misc::setType( Type t )
{
   if ( t >= types.size() ) {
      qWarning() << "Unrecognized misc type:" << t;
      return;
   }

   if ( m_cacheOnly ) {
      m_type = t;
      m_typeString = types.at(t);
   }
   else if ( setEasy( PropertyNames::Misc::type, types.at(t) ) ) {
      m_type = t;
      m_typeString = types.at(t);
      signalCacheChange( PropertyNames::Misc::type, types.at(t) );
   }
}

void Misc::setUse( Use u )
{
   if ( u >= uses.size() ) {
      qWarning() << "Unrecognized misc use:" << u;
      return;
   }
   if ( m_cacheOnly ) {
      m_use = u;
      m_useString = uses.at(u);
   }
   else if ( setEasy( PropertyNames::Misc::use, uses.at(u) ) ) {
      m_use = u;
      m_useString = uses.at(u);
      signalCacheChange( PropertyNames::Misc::use, uses.at(u) );
   }
}

void Misc::setUseFor( const QString& var )
{
   if ( m_cacheOnly ) {
      m_useFor = var;
   }
   else if ( setEasy( PropertyNames::Misc::useFor, var ) ) {
      m_useFor = var;
      signalCacheChange( PropertyNames::Misc::useFor, var );
   }
}

void Misc::setNotes( const QString& var )
{
   if ( m_cacheOnly ) {
      m_notes = var;
   }
   else if ( setEasy( PropertyNames::Misc::notes, var ) ) {
      m_notes = var;
      signalCacheChange( PropertyNames::Misc::notes, var );
   }
}

void Misc::setAmountType( AmountType t )
{
   bool is_a_weight = t == AmountType_Weight;
   setAmountIsWeight(is_a_weight);
}

void Misc::setAmountIsWeight( bool var )
{
   if ( m_cacheOnly ) {
      m_amountIsWeight = var;
   }
   else if ( setEasy( PropertyNames::Misc::amountIsWeight, var ) ) {
      m_amountIsWeight = var;
      signalCacheChange( PropertyNames::Misc::amountIsWeight, var );
   }
}

void Misc::setAmount( double var )
{
   if( var < 0.0 ) {
      qWarning() << "Misc: amount < 0:" << var;
      return;
   }

   if ( m_cacheOnly ) {
      m_amount = var;
   }
   else if ( setEasy( PropertyNames::Misc::amount, var ) ) {
      m_amount = var;
      signalCacheChange( PropertyNames::Misc::amount, var );
   }
}

void Misc::setTime( double var )
{
   if( var < 0.0 ) {
      qWarning() << "Misc: time < 0:" << var;
      return;
   }
   if ( m_cacheOnly ) {
      m_time = var;
   }
   else if ( setEasy( PropertyNames::Misc::time, var ) ) {
      m_time = var;
      signalCacheChange( PropertyNames::Misc::time, var );
   }
}

//========================OTHER METHODS=========================================

bool Misc::isValidUse( const QString& var )
{
   static const QString uses[] = {"Boil", "Mash", "Primary", "Secondary", "Bottling"};
   static const unsigned int size = 5;
   unsigned int i;

   for( i = 0; i < size; ++i )
      if( var == uses[i] )
         return true;

   return false;
}

bool Misc::isValidType( const QString& var )
{
   static const QString types[] = {"Spice", "Fining", "Water Agent", "Herb", "Flavor", "Other"};
   static const unsigned int size = 6;
   unsigned int i;

   for( i = 0; i < size; ++i )
      if( var == types[i] )
         return true;

   return false;
}

NamedEntity * Misc::getParent() {
   Misc * myParent = nullptr;

   // If we don't already know our parent, look it up
   if (!this->parentKey) {
      this->parentKey = Database::instance().getParentNamedEntityKey(*this);
   }

   // If we (now) know our parent, get a pointer to it
   if (this->parentKey) {
      myParent = Database::instance().misc(this->parentKey);
   }

   // Return whatever we got
   return myParent;
}

int Misc::insertInDatabase() {
   return Database::instance().insertMisc(this);
}

void Misc::removeFromDatabase() {
   Database::instance().remove(this);
}
