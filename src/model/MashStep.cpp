/*
 * model/MashStep.cpp is part of Brewtarget, and is Copyright the following
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
#include "model/MashStep.h"

#include <QDebug>
#include <QVector>

#include "brewtarget.h"
#include "database.h"
#include "PhysicalConstants.h"
#include "MashStepSchema.h"
#include "model/Mash.h"
#include "TableSchemaConst.h"

QStringList MashStep::types = QStringList() << "Infusion" << "Temperature" << "Decoction" << "Fly Sparge" << "Batch Sparge";
QStringList MashStep::typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction") << QObject::tr("Fly Sparge") << QObject::tr("Batch Sparge");

bool MashStep::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   MashStep const & rhs = static_cast<MashStep const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_type              == rhs.m_type              &&
      this->m_infuseAmount_l    == rhs.m_infuseAmount_l    &&
      this->m_stepTemp_c        == rhs.m_stepTemp_c        &&
      this->m_stepTime_min      == rhs.m_stepTime_min      &&
      this->m_rampTime_min      == rhs.m_rampTime_min      &&
      this->m_endTemp_c         == rhs.m_endTemp_c         &&
      this->m_infuseTemp_c      == rhs.m_infuseTemp_c      &&
      this->m_decoctionAmount_l == rhs.m_decoctionAmount_l &&
      this->m_stepNumber        == rhs.m_stepNumber
   );
}

QString MashStep::classNameStr()
{
   static const QString name("MashStep");
   return name;
}

//==============================CONSTRUCTORS====================================

MashStep::MashStep(QString name, bool cache)
   : NamedEntity(Brewtarget::MASHSTEPTABLE, cache, name, true),
     m_typeStr(QString()),
     m_type(static_cast<MashStep::Type>(0)),
     m_infuseAmount_l(0.0),
     m_stepTemp_c(0.0),
     m_stepTime_min(0.0),
     m_rampTime_min(0.0),
     m_endTemp_c(0.0),
     m_infuseTemp_c(0.0),
     m_decoctionAmount_l(0.0),
     m_stepNumber(0.0) {
   return;
}

MashStep::MashStep(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key) {
     m_typeStr = rec.value( table->propertyToColumn( PropertyNames::MashStep::type)).toString();
     m_infuseAmount_l = rec.value( table->propertyToColumn( PropertyNames::MashStep::infuseAmount_l)).toDouble();
     m_stepTemp_c = rec.value( table->propertyToColumn( PropertyNames::MashStep::stepTemp_c)).toDouble();
     m_stepTime_min = rec.value( table->propertyToColumn( PropertyNames::MashStep::stepTime_min)).toDouble();
     m_rampTime_min = rec.value( table->propertyToColumn( PropertyNames::MashStep::rampTime_min)).toDouble();
     m_endTemp_c = rec.value( table->propertyToColumn( PropertyNames::MashStep::endTemp_c)).toDouble();
     m_infuseTemp_c = rec.value( table->propertyToColumn( PropertyNames::MashStep::infuseTemp_c)).toDouble();
     m_decoctionAmount_l = rec.value( table->propertyToColumn( PropertyNames::MashStep::decoctionAmount_l)).toDouble();
     m_stepNumber = rec.value( table->propertyToColumn( PropertyNames::MashStep::stepNumber)).toInt();

     m_type = static_cast<MashStep::Type>(types.indexOf(m_typeStr));
}

//================================"SET" METHODS=================================
//
// This is a little special. Since I've declared that modifying the initial
// infusion temp does not version, we have to work harder.
void MashStep::setInfuseTemp_c(double var )
{
   if (  m_cacheOnly ) {
      m_infuseTemp_c = var;
   }
   else if ( m_stepNumber == 1 ) {
      m_infuseTemp_c = var;
      setEasy(PropertyNames::MashStep::infuseTemp_c, var, true, true);
   }
   else if ( setEasy(PropertyNames::MashStep::infuseTemp_c, var) ) {
      m_infuseTemp_c = var;
      signalCacheChange(PropertyNames::MashStep::infuseTemp_c,var);

   }
}

void MashStep::setType( Type t )
{
   if ( t >= types.size() ) {
      qWarning() << "MashStep: invalid type:" << t;
      return;
   }

   if ( m_cacheOnly ) {
      m_type = t;
      m_typeStr = types.at(t);
   }
   else if ( setEasy(PropertyNames::MashStep::type, m_typeStr) ) {
      m_type = t;
      m_typeStr = types.at(t);
      signalCacheChange(PropertyNames::MashStep::type, m_typeStr);
   }
}

void MashStep::setInfuseAmount_l( double var )
{
   if( var < 0.0 ) {
      qWarning() << "Infuse amount cannot be negative:" << var;
      return;
   }

   if ( m_cacheOnly ) {
      m_infuseAmount_l = var;
   }
   else if ( setEasy(PropertyNames::MashStep::infuseAmount_l, var) ) {
      m_infuseAmount_l = var;
      signalCacheChange(PropertyNames::MashStep::infuseAmount_l, var);
   }
}

void MashStep::setStepTemp_c( double var )
{
   if( var < PhysicalConstants::absoluteZero ) {
      qWarning() << "Step temp below absolute zero:" << var;
      return;
   }

   if ( m_cacheOnly ) {
      m_stepTemp_c = var;
   }
   else if ( setEasy(PropertyNames::MashStep::stepTemp_c, var) ) {
      m_stepTemp_c = var;
      signalCacheChange(PropertyNames::MashStep::stepTemp_c, var);
   }
}

void MashStep::setStepTime_min( double var )
{
   if( var < 0.0 ) {
      qWarning() << "step time cannot be negative:" << var;
      return;
   }

   if ( m_cacheOnly ) {
      m_stepTime_min = var;
   }
   else if ( setEasy(PropertyNames::MashStep::stepTime_min, var) ) {
      m_stepTime_min = var;
      signalCacheChange(PropertyNames::MashStep::stepTime_min, var);
   }
}

void MashStep::setRampTime_min( double var )
{
   if( var < 0.0 ) {
      qWarning() << "ramp time cannot be negative:" << var;
      return;
   }

   if ( m_cacheOnly ) {
      m_rampTime_min = var;
   }
   else if ( setEasy(PropertyNames::MashStep::rampTime_min, var) ) {
      m_rampTime_min = var;
      signalCacheChange(PropertyNames::MashStep::rampTime_min, var);
   }
}

void MashStep::setEndTemp_c( double var )
{
   if( var < PhysicalConstants::absoluteZero ) {
      qWarning() << "End temp below absolute zero:" << var;
      return;
   }
   if ( m_cacheOnly ) {
      m_endTemp_c = var;
   }
   else if ( setEasy(PropertyNames::MashStep::endTemp_c, var) ) {
      m_endTemp_c = var;
      signalCacheChange(PropertyNames::MashStep::endTemp_c, var);
   }
}

void MashStep::setDecoctionAmount_l(double var )
{
   if ( var < 0.0 ) {
      qWarning() << "Decoction amount cannot be negative";
      return;
   }

   if ( m_cacheOnly ) {
      m_decoctionAmount_l = var;
   }
   else if ( setEasy(PropertyNames::MashStep::decoctionAmount_l, var) ) {
      m_decoctionAmount_l = var;
      signalCacheChange(PropertyNames::MashStep::decoctionAmount_l, var);
   }
}

void MashStep::setMash( Mash * mash ) { this->m_mash = mash; }

//============================="GET" METHODS====================================
MashStep::Type MashStep::type() const { return m_type; }
const QString MashStep::typeString() const { return m_typeStr; }
const QString MashStep::typeStringTr() const {
   if ( m_type < 0 || m_type > typesTr.length() ) {
      return "";
   }
   return typesTr.at(m_type);
}
double MashStep::infuseTemp_c() const { return m_infuseTemp_c; }
double MashStep::infuseAmount_l() const { return m_infuseAmount_l; }
double MashStep::stepTemp_c() const { return m_stepTemp_c; }
double MashStep::stepTime_min() const { return m_stepTime_min; }
double MashStep::rampTime_min() const { return m_rampTime_min; }
double MashStep::endTemp_c() const { return m_endTemp_c; }
double MashStep::decoctionAmount_l() const { return m_decoctionAmount_l; }
int MashStep::stepNumber() const { return m_stepNumber; }
Mash * MashStep::mash( ) const { return m_mash; }

bool MashStep::isInfusion() const
{
   return ( m_type == MashStep::Infusion    ||
            m_type == MashStep::batchSparge ||
            m_type == MashStep::flySparge );
}

bool MashStep::isSparge() const
{
   return ( m_type == MashStep::batchSparge ||
            m_type == MashStep::flySparge   ||
            name() == "Final Batch Sparge" );
}

bool MashStep::isTemperature() const
{
   return ( m_type == MashStep::Temperature );
}

bool MashStep::isDecoction() const
{
   return ( m_type == MashStep::Decoction );
}

bool MashStep::isValidType( const QString &str ) const
{
   return MashStep::types.contains(str);
}

int MashStep::insertInDatabase() {
   return Database::instance().insertMashStep(this, this->m_mash);
}

void MashStep::removeFromDatabase() {
   Database::instance().remove(this);
}
