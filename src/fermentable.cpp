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
static const QString kRecomendMash("recommend_mash");
static const QString kIsMashed("is_mashed");

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
   setRecommendMash( get(kRecomendMash).toBool() );
   setIbuGalPerLb( get(kIBUGalPerLb).toDouble() );
   setIsMashed( get(kIsMashed).toBool() );
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

void Fermentable::setType( Type t )
{
   _type = t;
}

void Fermentable::setAdditionMethod( Fermentable::AdditionMethod m )
{
   setIsMashed(m == Fermentable::Mashed);
}

void Fermentable::setAdditionTime( Fermentable::AdditionTime t )
{
   setAddAfterBoil(t == Fermentable::Late );
}

void Fermentable::setAddAfterBoil( bool b )
{
   _isAfterBoil = b;
}

void Fermentable::setOrigin( const QString& str )
{
   _origin = str;
}

void Fermentable::setSupplier( const QString& str)
{
   _supplier = str;
}

void Fermentable::setNotes( const QString& str )
{
   _notes = str;
}

void Fermentable::setRecommendMash( bool b )
{
   _recommendMash = b;
}

void Fermentable::setIsMashed(bool var)
{
   _isMashed = var;
}

void Fermentable::setIbuGalPerLb( double num )
{
   _ibuGalPerLb = num;
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
      _amountKg = num;
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
      _inventoryAmt = num;
   }
}
void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _yieldPct = num;
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
      _colorSrm = num;
   }
}
void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _coarseFineDiff = num;
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
      _moisturePct = num;
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
      _diastaticPower = num;
   }
}
void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 )
   {
      _proteinPct = num;
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
      _maxInBatchPct = num;
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
   map.insert(kRecomendMash, recommendMash());
   map.insert(kIsMashed, isMashed());

   Database::instance().updateColumns( _table, _key, map);

   setInventory("", kAmount, inventory());
   emit saved();

}
