/*
 * model/Water.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "model/Water.h"

#include <QDomElement>
#include <QDomText>
#include <QObject>

#include "brewtarget.h"
#include "database.h"
#include "TableSchemaConst.h"
#include "WaterSchema.h"

bool Water::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Water const & rhs = static_cast<Water const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_calcium_ppm      == rhs.m_calcium_ppm      &&
      this->m_bicarbonate_ppm  == rhs.m_bicarbonate_ppm  &&
      this->m_sulfate_ppm      == rhs.m_sulfate_ppm      &&
      this->m_chloride_ppm     == rhs.m_chloride_ppm     &&
      this->m_sodium_ppm       == rhs.m_sodium_ppm       &&
      this->m_magnesium_ppm    == rhs.m_magnesium_ppm    &&
      this->m_ph               == rhs.m_ph
   );
}

QString Water::classNameStr()
{
   static const QString name("Water");
   return name;
}

Water::Water(QString name, bool cache)
   : NamedEntity(Brewtarget::WATERTABLE, name, true),
   m_amount(0.0),
   m_calcium_ppm(0.0),
   m_bicarbonate_ppm(0.0),
   m_sulfate_ppm(0.0),
   m_chloride_ppm(0.0),
   m_sodium_ppm(0.0),
   m_magnesium_ppm(0.0),
   m_ph(0.0),
   m_alkalinity(0.0),
   m_notes(QString()),
   m_cacheOnly(cache),
   m_type(NONE),
   m_mash_ro(0.0),
   m_sparge_ro(0.0),
   m_alkalinity_as_hco3(true)
{
}

Water::Water(Water const& other, bool cache)
   : NamedEntity(Brewtarget::WATERTABLE, other.name(), true),
   m_amount(other.m_amount),
   m_calcium_ppm(other.m_calcium_ppm),
   m_bicarbonate_ppm(other.m_bicarbonate_ppm),
   m_sulfate_ppm(other.m_sulfate_ppm),
   m_chloride_ppm(other.m_chloride_ppm),
   m_sodium_ppm(other.m_sodium_ppm),
   m_magnesium_ppm(other.m_magnesium_ppm),
   m_ph(other.m_ph),
   m_alkalinity(other.m_alkalinity),
   m_notes(other.m_notes),
   m_cacheOnly(cache),
   m_type(other.m_type),
   m_mash_ro(other.m_mash_ro),
   m_sparge_ro(other.m_sparge_ro),
   m_alkalinity_as_hco3(other.m_alkalinity_as_hco3)
{
}

Water::Water(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key),
   m_cacheOnly(false)
{
   m_amount = rec.value( table->propertyToColumn( PropertyNames::Water::amount)).toDouble();
   m_calcium_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::calcium_ppm)).toDouble();
   m_bicarbonate_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::bicarbonate_ppm)).toDouble();
   m_sulfate_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::sulfate_ppm)).toDouble();
   m_chloride_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::chloride_ppm)).toDouble();
   m_sodium_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::sodium_ppm)).toDouble();
   m_magnesium_ppm = rec.value( table->propertyToColumn( PropertyNames::Water::magnesium_ppm)).toDouble();
   m_ph = rec.value( table->propertyToColumn( PropertyNames::Water::ph)).toDouble();
   m_alkalinity = rec.value( table->propertyToColumn( PropertyNames::Water::alkalinity)).toDouble();
   m_notes = rec.value( table->propertyToColumn( PropertyNames::Water::notes)).toString();
   m_type = static_cast<Water::Types>(rec.value( table->propertyToColumn( PropertyNames::Water::type)).toInt());
   m_mash_ro = rec.value( table->propertyToColumn( PropertyNames::Water::mashRO)).toDouble();
   m_sparge_ro = rec.value( table->propertyToColumn( PropertyNames::Water::spargeRO)).toDouble();

   m_alkalinity_as_hco3 = rec.value( table->propertyToColumn( PropertyNames::Water::alkalinityAsHCO3)).toBool();

}

//================================"SET" METHODS=================================
void Water::setAmount( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::amount, var) ) {
      m_amount = var;
   }
}

void Water::setCalcium_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::calcium_ppm, var) ) {
      m_calcium_ppm = var;
   }
}

void Water::setBicarbonate_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::bicarbonate_ppm, var) ) {
      m_bicarbonate_ppm = var;
   }
}

void Water::setChloride_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::chloride_ppm, var) ) {
      m_chloride_ppm = var;
   }
}

void Water::setSodium_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::sodium_ppm, var) ) {
      m_sodium_ppm = var;
   }
}

void Water::setMagnesium_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::magnesium_ppm, var) ) {
      m_magnesium_ppm = var;
   }
}

void Water::setPh( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::ph, var) ) {
      m_ph = var;
   }
}

void Water::setAlkalinity(double var)
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::alkalinity, var) ) {
      m_alkalinity = var;
   }
}

void Water::setSulfate_ppm( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::sulfate_ppm, var) ) {
      m_sulfate_ppm = var;
   }
}

void Water::setNotes( const QString &var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::notes, var) ) {
      m_notes = var;
   }
}

void Water::setCacheOnly(bool cache) { m_cacheOnly = cache; }

void Water::setType(Types type)
{
   if ( type < NONE || type > TARGET ) {
      return;
   }

   if ( m_cacheOnly || setEasy(PropertyNames::Water::type, type) ) {
      m_type = type;
   }
}

void Water::setMashRO( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::mashRO, var) ) {
      m_mash_ro = var;
   }
}

void Water::setSpargeRO( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::spargeRO, var) ) {
      m_sparge_ro = var;
   }
}

void Water::setAlkalinityAsHCO3(bool var)
{
   if ( m_cacheOnly || setEasy(PropertyNames::Water::alkalinityAsHCO3, var) ) {
      m_alkalinity_as_hco3 = var;
   }
}

//=========================="GET" METHODS=======================================
QString Water::notes() const { return m_notes; }
double Water::sulfate_ppm() const { return m_sulfate_ppm; }
double Water::amount() const { return m_amount; }
double Water::calcium_ppm() const { return m_calcium_ppm; }
double Water::bicarbonate_ppm() const { return m_bicarbonate_ppm; }
double Water::chloride_ppm() const { return m_chloride_ppm; }
double Water::sodium_ppm() const { return m_sodium_ppm; }
double Water::magnesium_ppm() const { return m_magnesium_ppm; }
double Water::ph() const { return m_ph; }
double Water::alkalinity() const { return m_alkalinity; }
bool Water::cacheOnly() const { return m_cacheOnly; }
Water::Types Water::type() const { return m_type; }
double Water::mashRO() const { return m_mash_ro; }
double Water::spargeRO() const { return m_sparge_ro; }
bool Water::alkalinityAsHCO3() const { return m_alkalinity_as_hco3; }

double Water::ppm( Water::Ions ion )
{
   switch(ion) {
      case Water::Ca:   return m_calcium_ppm;
      case Water::Cl:   return m_chloride_ppm;
      case Water::HCO3: return m_bicarbonate_ppm;
      case Water::Mg:   return m_magnesium_ppm;
      case Water::Na:   return m_sodium_ppm;
      case Water::SO4:  return m_sulfate_ppm;
      default: return 0.0;
   }

   return 0.0;
}

NamedEntity * Water::getParent() {
   Water * myParent = nullptr;

   // If we don't already know our parent, look it up
   if (!this->parentKey) {
      this->parentKey = Database::instance().getParentNamedEntityKey(*this);
   }

   // If we (now) know our parent, get a pointer to it
   if (this->parentKey) {
      myParent = Database::instance().water(this->parentKey);
   }

   // Return whatever we got
   return myParent;
}

int Water::insertInDatabase() {
   return Database::instance().insertWater(this);
}

void Water::removeFromDatabase() {
   Database::instance().remove(this);
}
