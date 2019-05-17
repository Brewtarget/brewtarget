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
#include "mashstep.h"
#include "brewtarget.h"


/************* Columns *************/
const QString kName("name");
const QString kType("mstype");
const QString kInfuseAmount("infuse_amount");
const QString kStepTemp("step_temp");
const QString kStepTime("step_time");
const QString kRampTime("ramp_time");
const QString kEndTemp("end_temp");
const QString kInfuseTemp("infuse_temp");
const QString kDecoctionAmount("decoction_amount");
const QString kStepNumber("step_number");


/************** Props **************/
const QString kNameProp("name");
const QString kTypeProp("type");
const QString kInfuseAmountProp("infuseAmount_l");
const QString kStepTempProp("stepTemp_c");
const QString kStepTimeProp("stepTime_min");
const QString kRampTimeProp("rampTime_min");
const QString kEndTempProp("endTemp_c");
const QString kInfuseTempProp("infuseTemp_c");
const QString kDecoctionAmountProp("decoctionAmount_l");


QStringList MashStep::types = QStringList() << "Infusion" << "Temperature" << "Decoction" << "Fly Sparge" << "Batch Sparge";
QStringList MashStep::typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction") << QObject::tr("Fly Sparge") << QObject::tr("Batch Sparge");

QHash<QString,QString> MashStep::tagToProp = MashStep::tagToPropHash();

QHash<QString,QString> MashStep::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   //propHash["TYPE"] = kTypeProp;
   propHash["INFUSE_AMOUNT"] = kInfuseAmountProp;
   propHash["STEP_TEMP"] = kStepTimeProp;
   propHash["STEP_TIME"] = kStepTimeProp;
   propHash["RAMP_TIME"] = kRampTimeProp;
   propHash["END_TEMP"] = kEndTempProp;
   propHash["INFUSE_TEMP"] = kInfuseTempProp;
   propHash["DECOCTION_AMOUNT"] = kDecoctionAmountProp;
   return propHash;
}

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
   : BeerXMLElement(table, key)
{
}

MashStep::MashStep(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   QString _typeStr = rec.value(kType).toString();
   _type = static_cast<MashStep::Type>(types.indexOf(_typeStr));
   _infuseAmount_l = rec.value(kInfuseAmount).toDouble();
   _stepTemp_c = rec.value(kStepTemp).toDouble();
   _stepTime_min = rec.value(kStepTime).toDouble();
   _rampTime_min = rec.value(kRampTime).toDouble();
   _endTemp_c = rec.value(kEndTemp).toDouble();
   _infuseTemp_c = rec.value(kInfuseTemp).toDouble();
   _decoctionAmount_l = rec.value(kDecoctionAmount).toDouble();
   _stepNumber = rec.value(kStepNumber).toInt();
}

//================================"SET" METHODS=================================
void MashStep::setInfuseTemp_c(double var)
{
   _infuseTemp_c = var;
   set(kInfuseTempProp, kInfuseTemp, var);
}

void MashStep::setType( Type t )
{
   _type = t;
   _typeStr = types.at(t);
   set(kTypeProp, kType, _typeStr);
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
      _infuseAmount_l = var;
      set(kInfuseAmountProp, kInfuseAmount, var);
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
      _stepTemp_c = var;
      set(kStepTempProp, kStepTemp, var);
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
      _stepTime_min = var;
      set(kStepTimeProp, kStepTime, var);
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
      _rampTime_min = var;
      set(kRampTimeProp, kRampTime, var);
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
      _endTemp_c = var;
      set(kEndTempProp, kEndTemp, var);
   }
}

void MashStep::setDecoctionAmount_l(double var)
{
   _decoctionAmount_l = var;
   set(kDecoctionAmountProp, kDecoctionAmount, var);
}

//============================="GET" METHODS====================================
MashStep::Type MashStep::type() const { return _type; }
const QString MashStep::typeString() const { return _typeStr; }
const QString MashStep::typeStringTr() const { return typesTr.at(_type); }
double MashStep::infuseTemp_c() const { return _infuseTemp_c; }
double MashStep::infuseAmount_l() const { return _infuseAmount_l; }
double MashStep::stepTemp_c() const { return _stepTemp_c; }
double MashStep::stepTime_min() const { return _stepTime_min; }
double MashStep::rampTime_min() const { return _rampTime_min; }
double MashStep::endTemp_c() const { return _endTemp_c; }
double MashStep::decoctionAmount_l() const { return _decoctionAmount_l; }
int MashStep::stepNumber() const { return _stepNumber; }

bool MashStep::isInfusion() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Infusion    ||
            _type == MashStep::batchSparge ||
            _type == MashStep::flySparge );
}

bool MashStep::isSparge() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::batchSparge ||
            _type == MashStep::flySparge   || 
            name() == "Final Batch Sparge" );
}

bool MashStep::isTemperature() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Temperature );
}

bool MashStep::isDecoction() const
{
   MashStep::Type _type = type();
   return ( _type == MashStep::Decoction );
}

bool MashStep::isValidType( const QString &str ) const
{
   return MashStep::types.contains(str);
}
