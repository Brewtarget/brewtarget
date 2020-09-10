/*
 * misc.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - mik firestone <mikfire@gmail.com>
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

#include "brewtarget.h"
#include <iostream>
#include <string>
#include <QVector>
#include "misc.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDebug>

#include "TableSchemaConst.h"
#include "MiscSchema.h"

QStringList Misc::uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
QStringList Misc::types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";
QStringList Misc::amountTypes = QStringList() << "Weight" << "Volume";

QString Misc::classNameStr()
{
   static const QString name("Misc");
   return name;
}

//============================CONSTRUCTORS======================================
Misc::Misc(Brewtarget::DBTable table, int key)
   : Ingredient(table, key),
   m_typeString(QString()),
   m_type(static_cast<Misc::Type>(0)),
   m_useString(QString()),
   m_use(static_cast<Misc::Use>(0)),
   m_time(0.0),
   m_amount(0.0),
   m_amountIsWeight(false),
   m_useFor(QString()),
   m_notes(QString()),
   m_inventory(-1.0),
   m_inventory_id(0),
   m_cacheOnly(false)
{
}

Misc::Misc(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
   m_typeString(rec.value(kcolMiscType).toString()),
   m_type(static_cast<Misc::Type>(types.indexOf(m_typeString))),
   m_useString(rec.value(kcolUse).toString()),
   m_use(static_cast<Misc::Use>(uses.indexOf(m_useString))),
   m_time(rec.value(kcolTime).toDouble()),
   m_amount(rec.value(kcolAmount).toDouble()),
   m_amountIsWeight(rec.value(kcolMiscAmtIsWgt).toBool()),
   m_useFor(rec.value(kcolMiscUseFor).toString()),
   m_notes(rec.value(kcolNotes).toString()),
   m_inventory(-1.0),
   m_inventory_id(rec.value(kcolInventoryId).toInt()),
   m_cacheOnly(false)
{
}

Misc::Misc(Misc & other) : Ingredient(other),
   m_typeString(other.m_typeString),
   m_type(other.m_type),
   m_useString(other.m_useString),
   m_use(other.m_use),
   m_time(other.m_time),
   m_amount(other.m_amount),
   m_amountIsWeight(other.m_amountIsWeight),
   m_useFor(other.m_useFor),
   m_notes(other.m_notes),
   m_inventory(other.m_inventory),
   m_inventory_id(other.m_inventory_id),
   m_cacheOnly(other.m_cacheOnly)
{
}

Misc::Misc(QString name, bool cache)
   : Ingredient(Brewtarget::MISCTABLE, -1, name, true),
   m_typeString(QString()),
   m_type(static_cast<Misc::Type>(0)),
   m_useString(QString()),
   m_use(static_cast<Misc::Use>(0)),
   m_time(0.0),
   m_amount(0.0),
   m_amountIsWeight(false),
   m_useFor(QString()),
   m_notes(QString()),
   m_inventory(-1.0),
   m_inventory_id(0),
   m_cacheOnly(cache)
{
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

double Misc::inventory()
{
   if ( m_inventory < 0.0 ) {
      m_inventory = getInventory(kpropInventory).toDouble();
   }
   return m_inventory;
}

int Misc::inventoryId() const { return m_inventory_id; }

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

bool Misc::cacheOnly() const { return m_cacheOnly; }

//============================"SET" METHODS=====================================
void Misc::setType( Type t )
{
   m_type = t;
   m_typeString = types.at(t);
   if ( ! m_cacheOnly ) {
      setEasy( kpropType, m_typeString );
   }
}

void Misc::setUse( Use u )
{
   m_use = u;
   m_useString = uses.at(u);
   if ( ! m_cacheOnly ) {
      setEasy( kpropUse, m_useString );
   }
}

void Misc::setUseFor( const QString& var )
{
   m_useFor = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropUseFor, var );
   }
}

void Misc::setNotes( const QString& var )
{
   m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropNotes, var );
   }
}

void Misc::setAmountType( AmountType t )
{
   m_amountIsWeight = t == AmountType_Weight;
   if ( ! m_cacheOnly ) {
      setAmountIsWeight(m_amountIsWeight);
   }
}

void Misc::setAmountIsWeight( bool var )
{
   m_amountIsWeight = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropAmtIsWgt, var );
   }
}

void Misc::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: amount < 0: %1").arg(var) );
   else {
      m_amount = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropAmount, var );
      }
   }
}

void Misc::setInventoryAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: inventory < 0: %1").arg(var) );
   else {
      m_inventory = var;
      if ( ! m_cacheOnly )
         setInventory(var,m_inventory_id);
   }
}

void Misc::setInventoryId( int key )
{
   m_inventory_id = key;
   if ( ! m_cacheOnly )
      setEasy(kpropInventoryId, key);
}

void Misc::setTime( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: time < 0: %1").arg(var) );
   else {
      m_time = var;
      if ( ! m_cacheOnly ) {
         setEasy( kpropTime, var );
      }
   }
}

void Misc::setCacheOnly(bool cache) { m_cacheOnly = cache; }

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
