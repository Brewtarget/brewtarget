/*
 * mashstep.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include <QVector>
#include <QDebug>
#include "mashstep.h"
#include "brewtarget.h"

#include "TableSchemaConst.h"
#include "MashStepSchema.h"

QStringList MashStep::types = QStringList() << "Infusion" << "Temperature" << "Decoction" << "Fly Sparge" << "Batch Sparge";
QStringList MashStep::typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction") << QObject::tr("Fly Sparge") << QObject::tr("Batch Sparge");

bool operator<(MashStep &m1, MashStep &m2)
{
   return m1.name() < m2.name();
}

bool operator==(MashStep &m1, MashStep &m2)
{
   return m1.name() == m2.name();
}

QString MashStep::classNameStr()
{
   static const QString name("MashStep");
   return name;
}

//==============================CONSTRUCTORS====================================

MashStep::MashStep(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString(), true),
     m_typeStr(QString()),
     m_type(static_cast<MashStep::Type>(0)),
     m_infuseAmount_l(0.0),
     m_stepTemp_c(0.0),
     m_stepTime_min(0.0),
     m_rampTime_min(0.0),
     m_endTemp_c(0.0),
     m_infuseTemp_c(0.0),
     m_decoctionAmount_l(0.0),
     m_stepNumber(0.0),
     m_cacheOnly(false)
{
}

MashStep::MashStep(bool cache)
   : Ingredient(Brewtarget::MASHSTEPTABLE, -1, QString(), true),
     m_typeStr(QString()),
     m_type(static_cast<MashStep::Type>(0)),
     m_infuseAmount_l(0.0),
     m_stepTemp_c(0.0),
     m_stepTime_min(0.0),
     m_rampTime_min(0.0),
     m_endTemp_c(0.0),
     m_infuseTemp_c(0.0),
     m_decoctionAmount_l(0.0),
     m_stepNumber(0.0),
     m_cacheOnly(cache)
{
}

MashStep::MashStep(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool()),
     m_typeStr(rec.value(kcolMashstepType).toString()),
     m_type(static_cast<MashStep::Type>(types.indexOf(m_typeStr))),
     m_infuseAmount_l(rec.value(kcolMashstepInfuseAmt).toDouble()),
     m_stepTemp_c(rec.value(kcolMashstepStepTemp).toDouble()),
     m_stepTime_min(rec.value(kcolMashstepStepTime).toDouble()),
     m_rampTime_min(rec.value(kcolMashstepRampTime).toDouble()),
     m_endTemp_c(rec.value(kcolMashstepEndTemp).toDouble()),
     m_infuseTemp_c(rec.value(kcolMashstepInfuseTemp).toDouble()),
     m_decoctionAmount_l(rec.value(kcolMashstepDecoctAmt).toDouble()),
     m_stepNumber(rec.value(kcolMashstepStepNumber).toInt()),
     m_cacheOnly(false)
{
}

//================================"SET" METHODS=================================
void MashStep::setInfuseTemp_c(double var )
{
   m_infuseTemp_c = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropInfuseTemp, var);
   }
}

void MashStep::setType( Type t )
{
   m_type = t;
   m_typeStr = types.at(t);
   if ( ! m_cacheOnly ) {
      setEasy(kpropType, m_typeStr);
   }
}

void MashStep::setInfuseAmount_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("%1 number cannot be negative: %2").arg(Q_FUNC_INFO).arg(var) );
      return;
   }
   else
   {
      m_infuseAmount_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropInfuseAmt, var);
      }
   }
}

void MashStep::setStepTemp_c( double var )
{
   if( var < -273.15 )
   {
      Brewtarget::logW( QString("%1: temp below absolute zero: %2").arg(Q_FUNC_INFO).arg(var) );
      return;
   }
   else
   {
      m_stepTemp_c = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropStepTemp, var);
      }
   }
}

void MashStep::setStepTime_min( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("%1: step time cannot be negative: %2").arg(Q_FUNC_INFO).arg(var) );
      return;
   }
   else
   {
      m_stepTime_min = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropStepTime, var);
      }
   }
}

void MashStep::setRampTime_min( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("%1: ramp time cannot be negative: %2").arg(Q_FUNC_INFO).arg(var) );

      return;
   }
   else
   {
      m_rampTime_min = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropRampTime, var);
      }
   }
}

void MashStep::setEndTemp_c( double var )
{
   if( var < -273.15 )
   {
      Brewtarget::logW( QString("%1: temp below absolute zero: %2").arg(Q_FUNC_INFO).arg(var) );
      return;
   }
   else
   {
      m_endTemp_c = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropEndTemp, var);
      }
   }
}

void MashStep::setDecoctionAmount_l(double var )
{
   m_decoctionAmount_l = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropDecoctAmt, var);
   }
}

void MashStep::setCacheOnly( bool cache ) { m_cacheOnly = cache; }

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
bool MashStep::cacheOnly( ) const { return m_cacheOnly; }

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
