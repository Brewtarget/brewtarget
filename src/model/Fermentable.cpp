/*
 * model/Fermentable.cpp is part of Brewtarget, and is Copyright the following
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
#include "model/Fermentable.h"

#include <QDebug>
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QVariant>

#include "brewtarget.h"
#include "database.h"
#include "FermentableSchema.h"
#include "TableSchemaConst.h"

#define SUPER NamedEntity

QStringList Fermentable::types = QStringList() << "Grain" << "Sugar" << "Extract" << "Dry Extract" << "Adjunct";

bool Fermentable::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Fermentable const & rhs = static_cast<Fermentable const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type           == rhs.m_type           &&
      this->m_yieldPct       == rhs.m_yieldPct       &&
      this->m_colorSrm       == rhs.m_colorSrm       &&
      this->m_origin         == rhs.m_origin         &&
      this->m_supplier       == rhs.m_supplier       &&
      this->m_coarseFineDiff == rhs.m_coarseFineDiff &&
      this->m_moisturePct    == rhs.m_moisturePct    &&
      this->m_diastaticPower == rhs.m_diastaticPower &&
      this->m_proteinPct     == rhs.m_proteinPct     &&
      this->m_maxInBatchPct  == rhs.m_maxInBatchPct
   );
}


QString Fermentable::classNameStr()
{
   static const QString name("Fermentable");
   return name;
}

Fermentable::Fermentable(QString name, bool cache)
   : NamedEntity(Brewtarget::FERMTABLE, name, true),
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

Fermentable::Fermentable(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key),
     m_inventory(-1.0),
     m_cacheOnly(false)
{
     m_typeStr = rec.value( table->propertyToColumn( PropertyNames::Fermentable::type)).toString();
     m_amountKg = rec.value( table->propertyToColumn( PropertyNames::Fermentable::amount_kg)).toDouble();
     m_yieldPct = rec.value( table->propertyToColumn( PropertyNames::Fermentable::yield_pct)).toDouble();
     m_colorSrm = rec.value( table->propertyToColumn( PropertyNames::Fermentable::color_srm)).toDouble();
     m_isAfterBoil = rec.value( table->propertyToColumn( PropertyNames::Fermentable::addAfterBoil)).toBool();
     m_origin = rec.value( table->propertyToColumn( PropertyNames::Fermentable::origin)).toString();
     m_supplier = rec.value( table->propertyToColumn( PropertyNames::Fermentable::supplier)).toString();
     m_notes = rec.value(table->propertyToColumn( PropertyNames::Fermentable::notes)).toString();
     m_coarseFineDiff = rec.value( table->propertyToColumn( PropertyNames::Fermentable::coarseFineDiff_pct)).toDouble();
     m_moisturePct = rec.value( table->propertyToColumn( PropertyNames::Fermentable::moisture_pct)).toDouble();
     m_diastaticPower = rec.value( table->propertyToColumn( PropertyNames::Fermentable::diastaticPower_lintner)).toDouble();
     m_proteinPct = rec.value( table->propertyToColumn( PropertyNames::Fermentable::protein_pct)).toDouble();
     m_maxInBatchPct = rec.value( table->propertyToColumn( PropertyNames::Fermentable::maxInBatch_pct)).toDouble();
     m_recommendMash = rec.value( table->propertyToColumn( PropertyNames::Fermentable::recommendMash)).toBool();
     m_ibuGalPerLb = rec.value( table->propertyToColumn( PropertyNames::Fermentable::ibuGalPerLb)).toDouble();
     m_isMashed = rec.value( table->propertyToColumn( PropertyNames::Fermentable::isMashed)).toBool();

     // keys is different critters
     m_inventory_id = rec.value( table->foreignKeyToColumn(PropertyNames::Fermentable::inventory_id)).toInt();

     // calculated, not retrieved from the db
     m_type = static_cast<Fermentable::Type>(types.indexOf(m_typeStr));
}

Fermentable::Fermentable( Fermentable &other )
        : NamedEntity( other ),
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
   if ( m_cacheOnly || setEasy(PropertyNames::Fermentable::type, types.at(t)) ) {
      m_type = t;
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
   if ( m_cacheOnly || setEasy(PropertyNames::Fermentable::addAfterBoil, b) ) {
      m_isAfterBoil = b;
   }
}

void Fermentable::setOrigin( const QString& str )
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::origin, str) ) {
      m_origin = str;
   }
}

void Fermentable::setSupplier( const QString& str)
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::supplier, str) ) {
      m_supplier = str;
   }
}

void Fermentable::setNotes( const QString& str )
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::notes, str) ) {
      m_notes = str;
   }
}

void Fermentable::setRecommendMash( bool b )
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::recommendMash, b) ) {
      m_recommendMash = b;
   }
}

void Fermentable::setIsMashed(bool var)
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::isMashed, var) ) {
      m_isMashed = var;
   }
}

void Fermentable::setIbuGalPerLb( double num )
{
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::ibuGalPerLb, num) ) {
      m_ibuGalPerLb = num;
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
   if( num < 0.0 ) {
      qWarning() << QString("Fermentable: negative amount: %1").arg(num);
      return;
   }

   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::amount_kg, num) ) {
      qInfo() << Q_FUNC_INFO << "updating cache:" << num;
      m_amountKg = num;
   }
}

// changes to inventory amounts do NOT create a version
void Fermentable::setInventoryAmount( double num )
{
   if( num < 0.0 ) {
      qWarning() << QString("Fermentable: negative inventory: %1").arg(num);
      return;
   }

   m_inventory = num;
   if ( ! m_cacheOnly ) {
      setInventory(num,m_inventory_id);
   }
}

// I will regret this, but I don't think this happens at any point other than
// load time, so no versions
void Fermentable::setInventoryId( int key )
{
   if( key < 1 ) {
      qWarning() << QString("Fermentable: bad inventory id: %1").arg(key);
      return;
   }
   m_inventory_id = key;
   if ( ! m_cacheOnly ) {
      setEasy(kpropInventoryId,key);
   }
}

double Fermentable::inventory()
{
   if ( m_inventory < 0 ) {
      m_inventory = getInventory().toDouble();
   }
   return m_inventory;
}

int Fermentable::inventoryId()
{
   return m_inventory_id;
}

void Fermentable::setYield_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 ) {
      if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::yield_pct, num) ) {
         m_yieldPct = num;
      }
   }
   else {
      qWarning() << QString("Fermentable: 0 < yield < 100: %1").arg(num);
   }
}

void Fermentable::setColor_srm( double num )
{
   if( num < 0.0 ) {
      qWarning() << QString("Fermentable: negative color: %1").arg(num);
      return;
   }
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::color_srm, num) ) {
      m_colorSrm = num;
   }
}

void Fermentable::setCoarseFineDiff_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 ) {
      if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::coarseFineDiff_pct, num) ) {
         m_coarseFineDiff = num;
      }
   }
   else {
      qWarning() << QString("Fermentable: 0 < coarsefinediff < 100: %1").arg(num);
   }
}

void Fermentable::setMoisture_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 ) {
      if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::moisture_pct, num) ) {
         m_moisturePct = num;
      }
   }
   else {
      qWarning() << QString("Fermentable: 0 < moisture < 100: %1").arg(num);
   }
}

void Fermentable::setDiastaticPower_lintner( double num )
{
   if( num < 0.0 )
   {
      qWarning() << QString("Fermentable: negative DP: %1").arg(num);
      return;
   }
   if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::diastaticPower_lintner, num) ) {
      m_diastaticPower = num;
   }
}

void Fermentable::setProtein_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 ) {
      if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::protein_pct, num) ) {
         m_proteinPct = num;
      }
   }
   else {
      qWarning() << QString("Fermentable: 0 < protein < 100: %1").arg(num);
   }
}

void Fermentable::setMaxInBatch_pct( double num )
{
   if( num >= 0.0 && num <= 100.0 ) {
      if ( m_cacheOnly || setEasy( PropertyNames::Fermentable::maxInBatch_pct, num) ) {
         m_maxInBatchPct = num;
      }
   }
   else {
      qWarning() << QString("Fermentable: 0 < maxinbatch < 100: %1").arg(num);
   }
}

void Fermentable::setCacheOnly( bool cache ) { m_cacheOnly = cache; }

NamedEntity * Fermentable::getParent() {
   Fermentable * myParent = nullptr;

   // If we don't already know our parent, look it up
   if (!this->parentKey) {
      this->parentKey = Database::instance().getParentNamedEntityKey(*this);
   }

   // If we (now) know our parent, get a pointer to it
   if (this->parentKey) {
      myParent = Database::instance().fermentable(this->parentKey);
   }

   // Return whatever we got
   return myParent;
}

int Fermentable::insertInDatabase() {
   return Database::instance().insertFermentable(this);
}

void Fermentable::removeFromDatabase() {
   Database::instance().remove(this);
}
