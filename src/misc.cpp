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

QStringList Misc::uses = QStringList() << "Boil" << "Mash" << "Primary" << "Secondary" << "Bottling";
QStringList Misc::types = QStringList() << "Spice" << "Fining" << "Water Agent" << "Herb" << "Flavor" << "Other";
QHash<QString,QString> Misc::tagToProp = Misc::tagToPropHash();

QHash<QString,QString> Misc::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   //propHash["TYPE"] = "type";
   //propHash["USE"] = "use";
   propHash["TIME"] = "time";
   propHash["AMOUNT"] = "amount";
   propHash["AMOUNT_IS_WEIGHT"] = "amountIsWeight";
   propHash["INVENTORY"] = "inventory";
   propHash["USE_FOR"] = "useFor";
   propHash["NOTES"] = "notes";
   return propHash;
}
//============================CONSTRUCTORS======================================
Misc::Misc() : BeerXMLElement()
{
}

Misc::Misc(Misc const& other) : BeerXMLElement(other)
{
}

//============================"GET" METHODS=====================================
QString Misc::name() const
{
   return get( "name" ).toString();
}

Misc::Type Misc::type() const
{
   return static_cast<Misc::Type>(types.indexOf(get("mtype").toString()));
}

const QString Misc::typeString() const
{
   return types.at(type());
}

const QString Misc::typeStringTr() const
{
   QStringList typesTr = QStringList() << QObject::tr("Spice") << QObject::tr("Fining") << QObject::tr("Water Agent") << QObject::tr("Herb") << QObject::tr("Flavor") << QObject::tr("Other");
   return typesTr.at(type());
}

Misc::Use Misc::use() const
{
   return static_cast<Misc::Use>(uses.indexOf(get("use").toString()));
}

const QString Misc::useString() const
{
   return uses.at(use());
}

const QString Misc::useStringTr() const
{
   QStringList usesTr = QStringList() << QObject::tr("Boil") << QObject::tr("Mash") << QObject::tr("Primary") << QObject::tr("Secondary") << QObject::tr("Bottling");
   return usesTr.at(use());
}

double Misc::amount()    const { return Brewtarget::toDouble(get("amount").toString(), "Misc::amount()"); }
double Misc::time()      const { return Brewtarget::toDouble(get("time").toString(), "Misc::time()"); }

bool Misc::amountIsWeight() const { return get("amount_is_weight").toBool(); }

QString Misc::useFor() const { return get("use_for").toString(); }
QString Misc::notes() const { return get("notes").toString(); }

double Misc::inventory() const 
{ 
   QString amount = getInventory("amount").toString();

   double amt = Brewtarget::toDouble( amount, "Misc::getInventory" ); 

   return amt;
}

//============================"SET" METHODS=====================================
void Misc::setName( const QString& var )
{
   set( "name", "name", var );
   emit changedName(var);
}

void Misc::setType( Type t )
{
   set( "type", "mtype", types.at(t) );
}

void Misc::setUse( Use u )
{
   set( "use", "use", uses.at(u) );
}

void Misc::setAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: amount < 0: %1").arg(var) );
   else
      set( "amount", "amount", var );
}

void Misc::setInventoryAmount( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: inventory < 0: %1").arg(var) );
   else
      setInventory("inventory", "amount", var );
}

void Misc::setTime( double var )
{
   if( var < 0.0 )
      Brewtarget::logW( QString("Misc: time < 0: %1").arg(var) );
   else
      set( "time", "time", var );
}

void Misc::setAmountIsWeight( bool var )
{
   set( "amountIsWeight", "amount_is_weight", var );
}

void Misc::setUseFor( const QString& var )
{
   set( "useFor", "use_for", var );
}

void Misc::setNotes( const QString& var )
{
   set( "notes", "notes", var );
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
