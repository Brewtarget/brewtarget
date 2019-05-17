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
   : BeerXMLElement(table, key)
{
   setType( static_cast<Fermentable::Type>(types.indexOf(get(kType).toString())) );
   setAmount_kg( get(kAmount).toDouble() );
   setInventoryAmount( getInventory(kAmount).toDouble() );
   setYield_pct( get(kYield).toDouble() );
   setColor_srm( get(kColor).toDouble() );
   setAddAfterBoil( get(kAddAfterBoil).toBool() );
   setOrigin( get(kOrigin).toString() );
   setSupplier( get(kSupplier).toString() );
   setNotes( get(kNotes).toString() );
   setCoarseFineDiff_pct( get(kCoarseFineDiff).toDouble() );
   setMoisture_pct( get(kMoisture).toDouble() );
   setDiastaticPower_lintner( get(kDiastaticPower).toDouble() );
   setProtein_pct( get(kProtein).toDouble() );
   setMaxInBatch_pct( get(kMaxInBatch).toDouble() );
   setRecommendMash( get(kRecommendMash).toBool() );
   setIbuGalPerLb( get(kIBUGalPerLb).toDouble() );
   setIsMashed( get(kIsMashed).toBool() );
}

Fermentable::Fermentable(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   setName( rec.value(kName).toString(), true );
   setDisplay( rec.value(kDisplay).toBool(), true);
   setDeleted( rec.value(kDeleted).toBool(), true);
   setFolder( rec.value(kFolder).toString(), false, true);
   _typeStr = rec.value(kType).toString();
   _type = static_cast<Fermentable::Type>(types.indexOf(_typeStr));
   _amountKg = rec.value(kAmount).toDouble();
   _yieldPct = rec.value(kYield).toDouble();
   _colorSrm = rec.value(kColor).toDouble();
   _isAfterBoil = rec.value(kAddAfterBoil).toBool();
   _origin = rec.value(kOrigin).toString();
   _supplier = rec.value(kSupplier).toString();
   _notes = rec.value(kNotes).toString();
   _coarseFineDiff = rec.value(kCoarseFineDiff).toDouble();
   _isMashed = rec.value(kIsMashed).toBool();
   _moisturePct = rec.value(kMoisture).toDouble();
   _diastaticPower = rec.value(kDiastaticPower).toDouble();
   _proteinPct = rec.value(kProtein).toDouble();
   _maxInBatchPct = rec.value(kMaxInBatch).toDouble();
   _recommendMash = rec.value(kRecommendMash).toBool();
   _ibuGalPerLb = rec.value(kIBUGalPerLb).toDouble();

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
   return types.at(type());
}

const QString Fermentable::typeStringTr() const
{
   static QStringList typesTr = QStringList () << QObject::tr("Grain") << QObject::tr("Sugar") << QObject::tr("Extract") << QObject::tr("Dry Extract") << QObject::tr("Adjunct");
   return typesTr.at(type());
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

void Fermentable::setType( Type t, bool cacheOnly )
{
   _type = t;
   if ( ! cacheOnly ) {
      set(kTypeProp, kType, types.at(t));
   }
}

void Fermentable::setAdditionMethod( Fermentable::AdditionMethod m, bool cacheOnly )
{
   setIsMashed(m == Fermentable::Mashed, cacheOnly);
}

void Fermentable::setAdditionTime( Fermentable::AdditionTime t, bool cacheOnly )
{
   setAddAfterBoil(t == Fermentable::Late, cacheOnly );
}

void Fermentable::setAddAfterBoil( bool b, bool cacheOnly )
{
   _isAfterBoil = b;
   if ( ! cacheOnly ) {
      set(kAddAfterBoilProp, kAddAfterBoil, b);
   }
}

void Fermentable::setOrigin( const QString& str, bool cacheOnly )
{
   _origin = str;
   if ( ! cacheOnly ) {
      set(kOriginProp, kOrigin, str);
   }
}

void Fermentable::setSupplier( const QString& str, bool cacheOnly)
{
   _supplier = str;
   if ( ! cacheOnly ) {
      set(kSupplierProp, kSupplier, str);
   }
}

void Fermentable::setNotes( const QString& str, bool cacheOnly )
{
   _notes = str;
   if ( ! cacheOnly ) {
      set(kNotesProp, kNotes, str);
   }
}

void Fermentable::setRecommendMash( bool b, bool cacheOnly )
{
   _recommendMash = b;
   if ( ! cacheOnly ) {
      set(kRecommendedMashProp, kRecommendMash, b);
   }
}

void Fermentable::setIsMashed(bool var, bool cacheOnly)
{
   _isMashed = var;
   if ( ! cacheOnly ) {
      set(kIsMashedProp, kIsMashed, var);
   }
}

void Fermentable::setIbuGalPerLb( double num, bool cacheOnly )
{
   _ibuGalPerLb = num;
   if ( ! cacheOnly ) {
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

void Fermentable::setAmount_kg( double num, bool cacheOnly )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative amount: %1").arg(num) );
      return;
   }
   else
   {
      _amountKg = num;
      if ( ! cacheOnly ) {
         set(kAmountProp, kAmount, num);
      }
   }
}
void Fermentable::setInventoryAmount( double num, bool cacheOnly )
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

void Fermentable::setYield_pct( double num, bool cacheOnly )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _yieldPct = num;
      if ( ! cacheOnly ) {
         set(kYieldProp, kYield, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < yield < 100: %1").arg(num) );
   }
}
void Fermentable::setColor_srm( double num, bool cacheOnly )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative color: %1").arg(num) );
      return;
   }
   else
   {
      _colorSrm = num;
      if ( ! cacheOnly ) {
         set(kColorProp, kColor, num);
      }
   }
}
void Fermentable::setCoarseFineDiff_pct( double num, bool cacheOnly )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _coarseFineDiff = num;
      if ( ! cacheOnly ) {
         set(kCoarseFineDiffProp, kCoarseFineDiff, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < coarsefinediff < 100: %1").arg(num) );
   }
}
void Fermentable::setMoisture_pct( double num, bool cacheOnly )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _moisturePct = num;
      if ( ! cacheOnly ) {
         set(kMoistureProp, kMoisture, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < moisture < 100: %1").arg(num) );
   }
}
void Fermentable::setDiastaticPower_lintner( double num, bool cacheOnly )
{
   if( num < 0.0 )
   {
      Brewtarget::logW( QString("Fermentable: negative DP: %1").arg(num) );
      return;
   }
   else
   {
      _diastaticPower = num;
      if ( ! cacheOnly ) {
         set(kDiastaticPowerProp, kDiastaticPower, num);
      }
   }
}
void Fermentable::setProtein_pct( double num, bool cacheOnly )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _proteinPct = num;
      if ( ! cacheOnly ) {
         set(kProteinProp, kProtein, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < protein < 100: %1").arg(num) );
   }
}
void Fermentable::setMaxInBatch_pct( double num, bool cacheOnly )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _maxInBatchPct = num;
      if ( ! cacheOnly ) {
         set(kMaxInBatchProp, kMaxInBatch, num);
      }
   }
   else
   {
      Brewtarget::logW( QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num) );
   }
}

void Fermentable::save()
{
   QVariantMap map = SUPER::getColumnValueMap();
   map.insert(kMaxInBatch, maxInBatch_pct());
   map.insert(kProtein, protein_pct());
   map.insert(kDiastaticPower, diastaticPower_lintner());
   map.insert(kMoisture, moisture_pct());
   map.insert(kCoarseFineDiff, coarseFineDiff_pct());
   map.insert(kColor, color_srm());
   map.insert(kYield, yield_pct());
   map.insert(kAmount, amount_kg());
   map.insert(kIBUGalPerLb, ibuGalPerLb());
   map.insert(kType, typeString());
   map.insert(kAddAfterBoil, addAfterBoil());
   map.insert(kOrigin, origin());
   map.insert(kSupplier, supplier());
   map.insert(kNotes, notes());
   map.insert(kRecommendMash, recommendMash());
   map.insert(kIsMashed, isMashed());

   Database::instance().updateColumns( _table, _key, map);

   emit saved();

}
