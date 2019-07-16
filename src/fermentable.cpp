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

#define SUPER BeerXMLElement

/************* Columns *************/
static const QString kAmount("amount");
static const QString kYield("yield");
static const QString kColor("color");
static const QString kCoarseFineDiff("coarse_fine_diff");
static const QString kMoisture("moisture");
static const QString kDiastaticPower("diastatic_power");
static const QString kProtein("protein");
static const QString kMaxInBatch("max_in_batch");
static const QString kIBUGalPerLb("ibu_gal_per_lb");
static const QString kType("ftype");
static const QString kAddAfterBoil("add_after_boil");
static const QString kOrigin("origin");
static const QString kSupplier("supplier");
static const QString kNotes("notes");
static const QString kRecommendMash("recommend_mash");
static const QString kIsMashed("is_mashed");

// these are defined in the parent, but I need them here too
const QString kName("name");
const QString kDeleted("deleted");
const QString kDisplay("display");
const QString kFolder("folder");
/************** Props **************/
static const QString kNameProp("name");
static const QString kTypeProp("type");
static const QString kAmountProp("amount_kg");
static const QString kInventoryProp("inventory");
static const QString kYieldProp("yield_pct");
static const QString kColorProp("color_srm");
static const QString kAddAfterBoilProp("addAfterBoil");
static const QString kOriginProp("origin");
static const QString kSupplierProp("supplier");
static const QString kNotesProp("notes");
static const QString kCoarseFineDiffProp("coarseFineDiff_pct");
static const QString kMoistureProp("moisture_pct");
static const QString kDiastaticPowerProp("diastaticPower_lintner");
static const QString kProteinProp("protein_pct");
static const QString kMaxInBatchProp("maxInBatch_pct");
static const QString kRecommendedMashProp("recommendMash");
static const QString kIsMashedProp("isMashed");
static const QString kIBUGalPerLbProp("ibuGalPerLb");

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";
QHash<QString,QString> Fermentable::tagToProp = Fermentable::tagToPropHash();

QHash<QString,QString> Fermentable::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = kNameProp;
   // NOTE: since type is actually stored as a string (not integer), have to handle separately.
   //propHash["TYPE"] = kTypeProp
   propHash["AMOUNT"] = kAmountProp;
   propHash["INVENTORY"] = kInventoryProp;
   propHash["YIELD"] = kYieldProp;
   propHash["COLOR"] = kColorProp;
   propHash["ADD_AFTER_BOIL"] = kAddAfterBoilProp;
   propHash["ORIGIN"] = kOriginProp;
   propHash["SUPPLIER"] = kSupplierProp;
   propHash["NOTES"] = kNotesProp;
   propHash["COARSE_FINE_DIFF"] = kCoarseFineDiffProp;
   propHash["MOISTURE"] = kMoistureProp;
   propHash["DIASTATIC_POWER"] = kDiastaticPowerProp;
   propHash["PROTEIN"] = kProteinProp;
   propHash["MAX_IN_BATCH"] = kMaxInBatchProp;
   propHash["RECOMMEND_MASH"] = kRecommendedMashProp;
   propHash["IS_MASHED"] = kIsMashedProp;
   propHash["IBU_GAL_PER_LB"] = kIBUGalPerLbProp;
   return propHash;
}

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

Fermentable::Fermentable(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key, QString(), true),
     m_typeStr(QString()),
     m_type(static_cast<Fermentable::Type>(0)),
     m_amountKg(0.0),
     m_yieldPct(0.0),
     m_colorSrm(0.0),
     m_isAfterBoil(0.0),
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
     m_isMashed(true),
     m_cacheOnly(false)
{
}

Fermentable::Fermentable(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kName).toString(), rec.value(kDisplay).toBool()),
     m_typeStr(rec.value(kType).toString()),
     m_type(static_cast<Fermentable::Type>(types.indexOf(m_typeStr))),
     m_amountKg(rec.value(kAmount).toDouble()),
     m_yieldPct(rec.value(kYield).toDouble()),
     m_colorSrm(rec.value(kColor).toDouble()),
     m_isAfterBoil(rec.value(kAddAfterBoil).toBool()),
     m_origin(rec.value(kOrigin).toString()),
     m_supplier(rec.value(kSupplier).toString()),
     m_notes(rec.value(kNotes).toString()),
     m_coarseFineDiff(rec.value(kCoarseFineDiff).toDouble()),
     m_moisturePct(rec.value(kMoisture).toDouble()),
     m_diastaticPower(rec.value(kDiastaticPower).toDouble()),
     m_proteinPct(rec.value(kProtein).toDouble()),
     m_maxInBatchPct(rec.value(kMaxInBatch).toDouble()),
     m_recommendMash(rec.value(kRecommendMash).toBool()),
     m_ibuGalPerLb(rec.value(kIBUGalPerLb).toDouble()),
     m_isMashed(rec.value(kIsMashed).toBool()),
     m_cacheOnly(false)
{
}

Fermentable::Fermentable( Fermentable const& other )
        : BeerXMLElement( other )
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
   setIbuGalPerLb( other.ibuGalPerLb() );
   setIsMashed(other.isMashed());
}

// Gets

const Fermentable::Type Fermentable::type() const { return m_type; }
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

const Fermentable::AdditionMethod Fermentable::additionMethod() const
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

const Fermentable::AdditionTime Fermentable::additionTime() const
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
   if ( m_type > typesTr.length()) {
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
      set(kTypeProp, kType, types.at(t));
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
      set(kAddAfterBoilProp, kAddAfterBoil, b);
   }
}

void Fermentable::setOrigin( const QString& str )
{
   m_origin = str;
   if ( ! m_cacheOnly ) {
      set(kOriginProp, kOrigin, str);
   }
}

void Fermentable::setSupplier( const QString& str)
{
   m_supplier = str;
   if ( ! m_cacheOnly ) {
      set(kSupplierProp, kSupplier, str);
   }
}

void Fermentable::setNotes( const QString& str )
{
   m_notes = str;
   if ( ! m_cacheOnly ) {
      set(kNotesProp, kNotes, str);
   }
}

void Fermentable::setRecommendMash( bool b )
{
   m_recommendMash = b;
   if ( ! m_cacheOnly ) {
      set(kRecommendedMashProp, kRecommendMash, b);
   }
}

void Fermentable::setIsMashed(bool var)
{
   m_isMashed = var;
   if ( ! m_cacheOnly ) {
      set(kIsMashedProp, kIsMashed, var);
   }
}

void Fermentable::setIbuGalPerLb( double num )
{
   m_ibuGalPerLb = num;
   if ( ! m_cacheOnly ) {
      set(kIBUGalPerLbProp, kIBUGalPerLb, num);
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
         set(kAmountProp, kAmount, num);
      }
   }
}

void Fermentable::setInventoryAmount( double num )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative inventory: %1").arg(num) );
      return;
   }
   else
   {
      setInventory(kInventoryProp, kAmount, num);
   }
}

double Fermentable::inventory() const
{
   return getInventory(kAmount).toDouble();
}

void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_yieldPct = num;
      if ( ! m_cacheOnly ) {
         set(kYieldProp, kYield, num);
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
         set(kColorProp, kColor, num);
      }
   }
}

void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_coarseFineDiff = num;
      if ( ! m_cacheOnly ) {
         set(kCoarseFineDiffProp, kCoarseFineDiff, num);
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
         set(kMoistureProp, kMoisture, num);
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
         set(kDiastaticPowerProp, kDiastaticPower, num);
      }
   }
}

void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      m_proteinPct = num;
      if ( ! m_cacheOnly ) {
         set(kProteinProp, kProtein, num);
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
         set(kMaxInBatchProp, kMaxInBatch, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num) );
   }
}

void Fermentable::setCacheOnly( bool cache ) { m_cacheOnly = cache; }

