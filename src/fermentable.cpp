/*
 * fermentable.cpp is part of Brewtarget, and is Copyright the following
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
#include <QVariant>
#include <QObject>
#include <QDebug>
#include "fermentable.h"
#include "brewtarget.h"
#include "database.h"

#include "TableSchemaConst.h"
#include "FermentableSchema.h"
#define SUPER Ingredient

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";

bool operator<(Fermentable &f1, Fermentable &f2)
{
   return f1.name() < f2.name();
}

bool operator==(Fermentable &f1, Fermentable &f2)
{
   return f1.name() == f2.name();
}

QString Fermentable::classNameStr()
{
   static const QString name("Fermentable");
   return name;
}

Fermentable::Fermentable(QString name, bool cache)
   : Ingredient(Brewtarget::FERMTABLE, -1, name, true),
     m_typeStr(QString()),
     m_type(static_cast<Fermentable::Type>(0)),
     m_amountKg(0.0),
     m_yieldPct(0.0),
     m_colorSrm(0.0),
     m_isAfterBoil(false),
     m_origin(QString()),
     m_supplier(QString()),
     m_notes(QString()),
     m_coarseFineDiff(0.0),
     m_moisturePct(0.0),
     m_diastaticPower(0.0),
     m_proteinPct(0.0),
     m_maxInBatchPct(100.0),
     m_recommendMash(false),
     m_ibuGalPerLb(0.0),
     m_inventory(-1.0),
     m_inventory_id(0),
     m_isMashed(false),
     m_cacheOnly(cache)
{
}

Fermentable::Fermentable(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString(), true),
     m_typeStr(QString()),
     m_type(static_cast<Fermentable::Type>(0)),
     m_amountKg(0.0),
     m_yieldPct(0.0),
     m_colorSrm(0.0),
     m_isAfterBoil(false),
     m_origin(QString()),
     m_supplier(QString()),
     m_notes(QString()),
     m_coarseFineDiff(0.0),
     m_moisturePct(0.0),
     m_diastaticPower(0.0),
     m_proteinPct(0.0),
     m_maxInBatchPct(0.0),
     m_recommendMash(true),
     m_ibuGalPerLb(0.0),
     m_inventory(-1.0),
     m_inventory_id(0),
     m_isMashed(true),
     m_cacheOnly(false)
{
}

Fermentable::Fermentable(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
     m_typeStr(rec.value(kcolFermType).toString()),
     m_type(static_cast<Fermentable::Type>(types.indexOf(m_typeStr))),
     m_amountKg(rec.value(kcolFermAmount).toDouble()),
     m_yieldPct(rec.value(kcolFermYield).toDouble()),
     m_colorSrm(rec.value(kcolFermColor).toDouble()),
     m_isAfterBoil(rec.value(kcolFermAddAfterBoil).toBool()),
     m_origin(rec.value(kcolFermOrigin).toString()),
     m_supplier(rec.value(kcolFermSupplier).toString()),
     m_notes(rec.value(kcolNotes).toString()),
     m_coarseFineDiff(rec.value(kcolFermCoarseFineDiff).toDouble()),
     m_moisturePct(rec.value(kcolFermMoisture).toDouble()),
     m_diastaticPower(rec.value(kcolFermDiastaticPower).toDouble()),
     m_proteinPct(rec.value(kcolFermProtein).toDouble()),
     m_maxInBatchPct(rec.value(kcolFermMaxInBatch).toDouble()),
     m_recommendMash(rec.value(kcolFermRecommendMash).toBool()),
     m_ibuGalPerLb(rec.value(kcolFermIBUGalPerLb).toDouble()),
     m_inventory(-1.0),
     m_inventory_id(rec.value(kcolInventoryId).toInt()),
     m_isMashed(rec.value(kcolFermIsMashed).toBool()),
     m_cacheOnly(false)
{
}

Fermentable::Fermentable( Fermentable &other )
        : Ingredient( other ),
     m_typeStr(other.m_typeStr),
     m_type(other.m_type),
     m_amountKg(other.m_amountKg),
     m_yieldPct(other.m_yieldPct),
     m_colorSrm(other.m_colorSrm),
     m_isAfterBoil(other.m_isAfterBoil),
     m_origin(other.m_origin),
     m_supplier(other.m_supplier),
     m_notes(other.m_notes),
     m_coarseFineDiff(other.m_coarseFineDiff),
     m_moisturePct(other.m_moisturePct),
     m_diastaticPower(other.m_diastaticPower),
     m_proteinPct(other.m_proteinPct),
     m_maxInBatchPct(other.m_maxInBatchPct),
     m_recommendMash(other.m_recommendMash),
     m_ibuGalPerLb(other.m_ibuGalPerLb),
     m_inventory(other.m_inventory),
     m_inventory_id(other.m_inventory_id),
     m_isMashed(other.m_isMashed),
     m_cacheOnly(other.m_cacheOnly)
{
   setType( other.type() );
   setAmount_kg( other.amount_kg() );
   setInventoryAmount( other.inventory() );
   setYield_pct( other.yield_pct() );
   setColor_srm( other.color_srm() );
   setAddAfterBoil( other.addAfterBoil() );
   setOrigin( other.origin() );
   setSupplier( other.supplier() );
   setNotes( other.notes() );
   setCoarseFineDiff_pct( other.coarseFineDiff_pct() );
   setMoisture_pct( other.moisture_pct() );
   setDiastaticPower_lintner( other.diastaticPower_lintner() );
   setProtein_pct( other.protein_pct() );
   setMaxInBatch_pct( other.maxInBatch_pct() );
   setRecommendMash( other.recommendMash() );
   setInventoryAmount( other.inventory() );
   setInventoryId( other.inventoryId() );
   setIbuGalPerLb( other.ibuGalPerLb() );
   setIsMashed(other.isMashed());
}

// Gets

Fermentable::Type Fermentable::type() const { return m_type; }
double Fermentable::amount_kg() const { return m_amountKg; }
double Fermentable::yield_pct() const { return m_yieldPct; }
double Fermentable::color_srm() const { return m_colorSrm; }
bool Fermentable::addAfterBoil() const { return m_isAfterBoil; }
const QString Fermentable::origin() const { return m_origin; }
const QString Fermentable::supplier() const { return m_supplier; }
const QString Fermentable::notes() const { return m_notes; }
double Fermentable::coarseFineDiff_pct() const { return m_coarseFineDiff; }
double Fermentable::moisture_pct() const { return m_moisturePct; }
double Fermentable::diastaticPower_lintner() const { return m_diastaticPower; }
double Fermentable::protein_pct() const { return m_proteinPct; }
double Fermentable::maxInBatch_pct() const { return m_maxInBatchPct; }
bool Fermentable::recommendMash() const { return m_recommendMash; }
double Fermentable::ibuGalPerLb() const { return m_ibuGalPerLb; }
bool Fermentable::isMashed() const { return m_isMashed; }
bool Fermentable::cacheOnly() const { return m_cacheOnly; }

Fermentable::AdditionMethod Fermentable::additionMethod() const
{
   Fermentable::AdditionMethod additionMethod;
   if(isMashed())
      additionMethod = Fermentable::Mashed;
   else
   {
      if(type() == Fermentable::Grain)
         additionMethod = Fermentable::Steeped;
      else
         additionMethod = Fermentable::Not_Mashed;
   }
   return additionMethod;
}

Fermentable::AdditionTime Fermentable::additionTime() const
{
   Fermentable::AdditionTime additionTime;
   if(addAfterBoil())
      additionTime = Fermentable::Late;
   else
      additionTime = Fermentable::Normal;

   return additionTime;
}

const QString Fermentable::typeString() const
{
   if ( m_type > types.length()) {
      return "";
   }
   return types.at(type());
}

const QString Fermentable::typeStringTr() const
{
   static QStringList typesTr = QStringList () << QObject::tr("Grain") << QObject::tr("Sugar") << QObject::tr("Extract") << QObject::tr("Dry Extract") << QObject::tr("Adjunct");
   if ( m_type > typesTr.length() || m_type < 0 ) {
      return "";
   }

   return typesTr.at(m_type);
}

const QString Fermentable::additionMethodStringTr() const
{
    QString retString;

    if(isMashed())
       retString = tr("Mashed");
    else
    {
       if(type() == Fermentable::Grain)
          retString = tr("Steeped");
       else
          retString = tr("Not mashed");
    }
    return retString;
}

const QString Fermentable::additionTimeStringTr() const
{
    QString retString;

    if(addAfterBoil())
       retString = tr("Late");
    else
       retString = tr("Normal");

    return retString;
}

bool Fermentable::isExtract() const
{
   return ((type() == Extract) || (type() == Dry_Extract));
}

bool Fermentable::isSugar() const
{
   return (type() == Sugar);
}

bool Fermentable::isValidType( const QString& str )
{
   return (types.indexOf(str) >= 0);
}


// Sets
void Fermentable::setType( Type t )
{
   m_type = t;
   if ( ! m_cacheOnly ) {
      setEasy(kpropType, types.at(t));
   }
}

void Fermentable::setAdditionMethod( Fermentable::AdditionMethod m )
{
   setIsMashed(m == Fermentable::Mashed);
}

void Fermentable::setAdditionTime( Fermentable::AdditionTime t )
{
   setAddAfterBoil(t == Fermentable::Late);
}

void Fermentable::setAddAfterBoil( bool b )
{
   m_isAfterBoil = b;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAddAfterBoil, b);
   }
}

void Fermentable::setOrigin( const QString& str )
{
   m_origin = str;
   if ( ! m_cacheOnly ) {
      setEasy( kpropOrigin, str);
   }
}

void Fermentable::setSupplier( const QString& str)
{
   m_supplier = str;
   if ( ! m_cacheOnly ) {
      setEasy( kpropSupplier, str);
   }
}

void Fermentable::setNotes( const QString& str )
{
   m_notes = str;
   if ( ! m_cacheOnly ) {
      setEasy( kpropNotes, str);
   }
}

void Fermentable::setRecommendMash( bool b )
{
   m_recommendMash = b;
   if ( ! m_cacheOnly ) {
      setEasy( kpropRecommendMash, b);
   }
}

void Fermentable::setIsMashed(bool var)
{
   m_isMashed = var;
   if ( ! m_cacheOnly ) {
      setEasy( kpropIsMashed, var);
   }
}

void Fermentable::setIbuGalPerLb( double num )
{
   m_ibuGalPerLb = num;
   if ( ! m_cacheOnly ) {
      setEasy( kpropIBUGalPerLb, num);
   }
}

double Fermentable::equivSucrose_kg() const
{
   double ret = amount_kg() * yield_pct() * (1.0-moisture_pct()/100.0) / 100.0;

   // If this is a steeped grain...
   if( type() == Grain && !isMashed() )
      return 0.60 * ret; // Reduce the yield by 60%.
   else
      return ret;
}

void Fermentable::setAmount_kg( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative amount: %1").arg(num) );
      return;
   }
   else
   {
      m_amountKg = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropAmountKg, num);
      }
   }
}

void Fermentable::setInventoryAmount( double num )
{
   if( num < 0.0 ) {
      Brewtarget::logW( QString("Fermentable: negative inventory: %1").arg(num) );
      return;
   }
   else
   {
      m_inventory = num;
      if ( ! m_cacheOnly )
         setInventory(num,m_inventory_id);
   }
}

void Fermentable::setInventoryId( int key )
{
   if( key < 1 ) {
      Brewtarget::logW( QString("Fermentable: bad inventory id: %1").arg(key) );
      return;
   }
   else
   {
      m_inventory_id = key;
      if ( ! m_cacheOnly )
         setEasy(kpropInventoryId,key);
   }
}

double Fermentable::inventory()
{
   if ( m_inventory < 0 ) {
      m_inventory = getInventory(kpropInventory).toDouble();
   }
   return m_inventory;
}

int Fermentable::inventoryId()
{
   return m_inventory_id;
}

void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_yieldPct = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropYield, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < yield < 100: %1").arg(num) );
   }
}

void Fermentable::setColor_srm( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative color: %1").arg(num) );
      return;
   }
   else
   {
      m_colorSrm = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropColor, num);
      }
   }
}

void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_coarseFineDiff = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropCoarseFineDiff, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < coarsefinediff < 100: %1").arg(num) );
   }
}

void Fermentable::setMoisture_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_moisturePct = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropMoisture, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < moisture < 100: %1").arg(num) );
   }
}

void Fermentable::setDiastaticPower_lintner( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative DP: %1").arg(num) );
      return;
   }
   else
   {
      m_diastaticPower = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropDiastaticPower, num);
      }
   }
}

void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_proteinPct = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropProtein, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < protein < 100: %1").arg(num) );
   }
}

void Fermentable::setMaxInBatch_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_maxInBatchPct = num;
      if ( ! m_cacheOnly ) {
         setEasy( kpropMaxInBatch, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num) );
   }
}

void Fermentable::setCacheOnly( bool cache ) { m_cacheOnly = cache; }

