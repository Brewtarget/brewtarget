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

QStringList MashStep::types = QStringList() << "Infusion" << "Temperature" << "Decoction" << "Fly Sparge" << "Batch Sparge";
QStringList MashStep::typesTr = QStringList() << QObject::tr("Infusion") << QObject::tr("Temperature") << QObject::tr("Decoction") << QObject::tr("Fly Sparge") << QObject::tr("Batch Sparge");

QHash<QString,QString> MashStep::tagToProp = MashStep::tagToPropHash();

QHash<QString,QString> MashStep::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   //propHash["TYPE"] = "type";
   propHash["INFUSE_AMOUNT"] = "infuseAmount_l";
   propHash["STEP_TEMP"] = "stepTemp_c";
   propHash["STEP_TIME"] = "stepTime_min";
   propHash["RAMP_TIME"] = "rampTime_min";
   propHash["END_TEMP"] = "endTemp_c";
   propHash["INFUSE_TEMP"] = "infuseTemp_c";
   propHash["DECOCTION_AMOUNT"] = "decoctionAmount_l";
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

//==============================CONSTRUCTORS====================================

/*
void MashStep::setDefaults()
{
   name = "";
   type = TYPEINFUSION;
   infuseAmount_l = 0.0;
   infuseTemp_c = 0.0;
   stepTemp_c = 0.0;
   stepTime_min = 0.0;
   rampTime_min = 0.0;
   endTemp_c = 0.0;
   decoctionAmount_l = 0.0;
}
*/

MashStep::MashStep()
   : BeerXMLElement()
{
}

/*
void MashStep::fromNode(const QDomNode& mashStepNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   
   setDefaults();
   
   for( node = mashStepNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
   {
      if( ! node.isElement() )
      {
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Node at line %1 is not an element.").arg(textNode.lineNumber()) );
         continue;
      }
      
      child = node.firstChild();
      if( child.isNull() || ! child.isText() )
         continue;
      
      property = node.nodeName();
      textNode = child.toText();
      value = textNode.nodeValue();
      
      if( property == "NAME" )
      {
         name = value;
      }
      else if( property == "VERSION" )
      {
         if( version != getInt(textNode) )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("YEAST says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "TYPE" )
      {
         int ndx = types.indexOf(value);
         if( ndx < 0 )
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("%1 is not a valid type for MASHSTEP. Line %2").arg(value).arg(textNode.lineNumber()) );
         else
            setType(static_cast<MashStep::Type>(ndx));
      }
      else if( property == "INFUSE_AMOUNT" )
      {
         setInfuseAmount_l(getDouble(textNode));
      }
      else if( property == "STEP_TEMP" )
      {
         setStepTemp_c(getDouble(textNode));
      }
      else if( property == "STEP_TIME" )
      {
         setStepTime_min(getDouble(textNode));
      }
      else if( property == "RAMP_TIME" )
      {
         setRampTime_min(getDouble(textNode));
      }
      else if( property == "END_TEMP" )
      {
         setEndTemp_c(getDouble(textNode));
      }
      else if( property == "INFUSE_TEMP" )
      {
         setInfuseTemp_c(getDouble(textNode));
      }
      else if( property == "DECOCTION_AMOUNT" )
      {
         setDecoctionAmount_l(getDouble(textNode));
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported MASHSTEP property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
}
*/

//================================"SET" METHODS=================================
void MashStep::setName( const QString &var )
{
   set("name", "name", var);
   emit changedName(var);
}

void MashStep::setInfuseTemp_c(double var)
{
   set("infuseTemp_c", "infuse_temp", var);
}

void MashStep::setType( Type t )
{
   set("type", "mstype", types.at(t));
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
      set("infuseAmount_l", "infuse_amount", var);
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
      set("stepTemp_c", "step_temp", var);
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
      set("stepTime_min", "step_time", var);
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
      set("rampTime_min", "ramp_time", var);
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
      set("endTemp_c", "end_temp", var);
   }
}

void MashStep::setDecoctionAmount_l(double var)
{
   set("decoctionAmount_l", "decoction_amount", var);
}

//============================="GET" METHODS====================================

QString MashStep::name() const { return get("name").toString(); }

MashStep::Type MashStep::type()        const { return static_cast<MashStep::Type>(types.indexOf(get("mstype").toString())); }

const QString MashStep::typeString()   const { return get("mstype").toString(); }
const QString MashStep::typeStringTr() const { return typesTr.at(type()); }

double MashStep::infuseTemp_c()        const { return get("infuse_temp").toDouble(); }
double MashStep::infuseAmount_l()      const { return get("infuse_amount").toDouble(); }
double MashStep::stepTemp_c()          const { return get("step_temp").toDouble(); }
double MashStep::stepTime_min()        const { return get("step_time").toDouble(); }
double MashStep::rampTime_min()        const { return get("ramp_time").toDouble(); }
double MashStep::endTemp_c()           const { return get("end_temp").toDouble(); }
double MashStep::decoctionAmount_l()   const { return get("decoction_amount").toDouble(); }

int MashStep::stepNumber()             const { return get("step_number").toInt(); }

bool MashStep::isInfusion() const {
   MashStep::Type _type = type();
   return ( _type == MashStep::Infusion    ||
            _type == MashStep::batchSparge ||
            _type == MashStep::flySparge );
}

bool MashStep::isSparge() const {
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
