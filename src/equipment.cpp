/*
 * equipment.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Mik Firestone <mikfire@gmail.com>
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
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "equipment.h"
#include "brewtarget.h"
#include "HeatCalculations.h"

QHash<QString,QString> Equipment::tagToProp = Equipment::tagToPropHash();

QHash<QString,QString> Equipment::tagToPropHash()
{
   QHash<QString,QString> propHash;
   
   propHash["NAME"] = "name";
   propHash["BOIL_SIZE"] = "boilSize_l";
   propHash["BATCH_SIZE"] = "batchSize_l";
   propHash["TUN_VOLUME"] = "tunVolume_l";
   propHash["TUN_WEIGHT"] = "tunWeight_kg";
   propHash["TUN_SPECIFIC_HEAT"] = "tunSpecificHeat_calGC";
   propHash["TOP_UP_WATER"] = "topUpWater_l";
   propHash["TRUB_CHILLER_LOSS"] = "trubChillerLoss_l";
   propHash["EVAP_RATE"] = "evapRate_pctHr";
   propHash["REAL_EVAP_RATE"] = "evapRate_lHr";
   propHash["BOIL_TIME"] = "boilTime_min";
   propHash["CALC_BOIL_VOLUME"] = "calcBoilVolume";
   propHash["LAUTER_DEADSPACE"] = "lauterDeadspace_l";
   propHash["TOP_UP_KETTLE"] = "topUpKettle_l";
   propHash["HOP_UTILIZATION"] = "hopUtilization_pct";
   propHash["NOTES"] = "notes";
   propHash["ABSORPTION"] = "grainAbsorption_LKg";
   propHash["BOILING_POINT"] = "boilingPoint_c";
   
   return propHash;
}

bool operator<(Equipment &e1, Equipment &e2)
{
   return e1.name() < e2.name();
}

bool operator==(Equipment &e1, Equipment &e2)
{
   return e1.name() == e2.name();
}

//=============================CONSTRUCTORS=====================================

/*
void Equipment::setDefaults()
{
   name = "";
   boilSize_l = 0.0;
   batchSize_l = 0.0;
   tunVolume_l = 0.0;
   tunWeight_kg = 0.0;
   tunSpecificHeat_calGC = 0.0;
   topUpWater_l = 0.0;
   trubChillerLoss_l = 0.0;
   evapRate_pctHr = 0.0;
   evapRate_lHr = 0.0;
   boilTime_min = 0.0;
   calcBoilVolume = false;
   lauterDeadspace_l = 0.0;
   topUpKettle_l = 0.0;
   hopUtilization_pct = 0.0;
   notes = "";
   absorption_LKg = HeatCalculations::absorption_LKg;
   boilingPoint_c = 100.0;
}
*/

Equipment::Equipment()
   : BeerXMLElement()
{
}

Equipment::Equipment( Equipment const& other )
   : BeerXMLElement(other)
{
}

/*
void Equipment::fromNode(const QDomNode& equipmentNode)
{
   QDomNode node, child;
   QDomText textNode;
   QString property, value;
   bool hasRealEvapRate = false;
   
   setDefaults();
   
   for( node = equipmentNode.firstChild(); ! node.isNull(); node = node.nextSibling() )
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
            Brewtarget::log(Brewtarget::ERROR, QObject::tr("EQUIPMENT says it is not version %1. Line %2").arg(version).arg(textNode.lineNumber()) );
      }
      else if( property == "BOIL_SIZE" )
      {
         setBoilSize_l(getDouble(textNode));
      }
      else if( property == "BATCH_SIZE" )
      {
         setBatchSize_l(getDouble(textNode));
      }
      else if( property == "TUN_VOLUME" )
      {
         setTunVolume_l(getDouble(textNode));
      }
      else if( property == "TUN_WEIGHT" )
      {
         setTunWeight_kg(getDouble(textNode));
      }
      else if( property == "TUN_SPECIFIC_HEAT" )
      {
         setTunSpecificHeat_calGC(getDouble(textNode));
      }
      else if( property == "TOP_UP_WATER" )
      {
         setTopUpWater_l(getDouble(textNode));
      }
      else if( property == "TRUB_CHILLER_LOSS" )
      {
         setTrubChillerLoss_l(getDouble(textNode));
      }
      else if( property == "EVAP_RATE" && ! hasRealEvapRate )
      {
         setEvapRate_pctHr(getDouble(textNode));
      }
      else if( property == "REAL_EVAP_RATE" )
      {
         setEvapRate_lHr(getDouble(textNode));
         hasRealEvapRate = true;
      }
      else if( property == "BOIL_TIME" )
      {
         setBoilTime_min(getDouble(textNode));
      }
      else if( property == "CALC_BOIL_VOLUME" )
      {
         setCalcBoilVolume(getBool(textNode));
      }
      else if( property == "LAUTER_DEADSPACE" )
      {
         setLauterDeadspace_l(getDouble(textNode));
      }
      else if( property == "TOP_UP_KETTLE" )
      {
         setTopUpKettle_l(getDouble(textNode));
      }
      else if( property == "HOP_UTILIZATION" )
      {
         setHopUtilization_pct(getDouble(textNode));
      }
      else if( property == "NOTES" )
      {
         setNotes(value);
      }
      else if( property == "ABSORPTION" ) // My extension.
      {
         setGrainAbsorption_LKg( getDouble(textNode) );
      }
      else if ( property == "BOILING_POINT")
      {
         setBoilingPoint_c( getDouble(textNode) );
      }
      else
         Brewtarget::log(Brewtarget::WARNING, QObject::tr("Unsupported EQUIPMENT property: %1. Line %2").arg(property).arg(node.lineNumber()) );
   }
   
   // Estimate the actual evaporation rate if we didn't get one.
   if( ! hasRealEvapRate )
      setEvapRate_lHr( evapRate_pctHr/(double)100 * boilSize_l );
}
*/

//============================"SET" METHODS=====================================

void Equipment::setName( const QString &var )
{
   set( "name", "name", var );
   emit changedName(var);
}

void Equipment::setBoilSize_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil size negative: %1").arg(var) );
      return;
   }
   else
   {
      set("boilSize_l", "boil_size", var);
      emit changedBoilSize_l(var);
   }
}

void Equipment::setBatchSize_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: batch size negative: %1").arg(var) );
      return;
   }
   else
   {
      set("batchSize_l", "batch_size", var);
      doCalculations();
   }
}

void Equipment::setTunVolume_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun volume negative: %1").arg(var) );
      return;
   }
   else
   {
      set("tunVolume_l", "tun_volume", var);
   }
}

void Equipment::setTunWeight_kg( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun weight negative: %1").arg(var) );
      return;
   }
   else
   {
      set("tunWeight_kg", "tun_weight", var);
   }
}

void Equipment::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun sp heat negative: %1").arg(var) );
      return;
   }
   else
   {
      set("tunSpecificHeat_calGC", "tun_specific_heat", var);
   }
}

void Equipment::setTopUpWater_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up water negative: %1").arg(var) );
      return;
   }
   else
   {
      set("topUpWater_l", "top_up_water", var);
      doCalculations();
   }
}

void Equipment::setTrubChillerLoss_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: trub chiller loss negative: %1").arg(var) );
      return;
   }
   else
   {
      set("trubChillerLoss_l", "trub_chiller_loss", var);
      doCalculations();
   }
}

void Equipment::setEvapRate_pctHr( double var )
{
   if( var < 0.0 || var > 100.0)
   {
      Brewtarget::logW( QString("Equipment: 0 < evap rate < 100: %1").arg(var) );
      return;
   }
   else
   {
      set("evapRate_pctHr", "evap_rate", var);
      set("evapRate_lHr", "real_evap_rate", var/100.0 * batchSize_l() ); // We always use this one, so set it.
      doCalculations();
   }
}

void Equipment::setEvapRate_lHr( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: evap rate negative: %1").arg(var) );
      return;
   }
   else
   {
      set("evapRate_lHr", "real_evap_rate", var);
      setEvapRate_pctHr( var/batchSize_l() * 100.0 ); // We don't use it, but keep it current.
      doCalculations();
   }
}

void Equipment::setBoilTime_min( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil time negative: %1").arg(var) );
      return;
   }
   else
   {
      set("boilTime_min", "boil_time", var);
      emit changedBoilTime_min(var);
      doCalculations();
   }
}

void Equipment::setCalcBoilVolume( bool var )
{
   set("calcBoilVolume", "calc_boil_volume", var);
   if( var )
      doCalculations();
}

void Equipment::setLauterDeadspace_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: deadspace negative: %1").arg(var) );
      return;
   }
   else
   {
      set("lauterDeadspace_l", "lauter_deadspace", var);
   }
}

void Equipment::setTopUpKettle_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up kettle negative: %1").arg(var) );
      return;
   }
   else
   {
      set("topUpKettle_l", "top_up_kettle", var);
   }
}

void Equipment::setHopUtilization_pct( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: 0 < hop utilization: %1").arg(var) );
      return;
   }
   else
   {
      set("hopUtilization_pct", "hop_utilization", var);
   }
}

void Equipment::setNotes( const QString &var )
{
   set("notes", "notes", var);
}

void Equipment::setGrainAbsorption_LKg(double var)
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: absorption < 0: %1").arg(var) );
      return;
   }
   else
   {
      set("absorption_LKg", "absorption", var);
   }
}

void Equipment::setBoilingPoint_c(double var)
{
   if ( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boiling point of water < 0: %1").arg(var));
      return;
   }
   else 
   {
      set("boilingPoint_c", "boiling_point", var);
   }
}

//============================"GET" METHODS=====================================

QString Equipment::name() const { return get("name").toString(); }
QString Equipment::notes() const { return get("notes").toString(); }
bool Equipment::calcBoilVolume() const { return get("calc_boil_volume").toBool(); }

double Equipment::boilSize_l() const            { return get("boil_size").toDouble(); }
double Equipment::batchSize_l() const           { return get("batch_size").toDouble(); }
double Equipment::tunVolume_l() const           { return get("tun_volume").toDouble(); }
double Equipment::tunWeight_kg() const          { return get("tun_weight").toDouble(); }
double Equipment::tunSpecificHeat_calGC() const { return get("tun_specific_heat").toDouble(); }
double Equipment::topUpWater_l() const          { return get("top_up_water").toDouble(); }
double Equipment::trubChillerLoss_l() const     { return get("trub_chiller_loss").toDouble(); }
double Equipment::evapRate_pctHr() const        { return get("evap_rate").toDouble(); }
double Equipment::evapRate_lHr() const          { return get("real_evap_rate").toDouble(); }
double Equipment::boilTime_min() const          { return get("boil_time").toDouble(); }
double Equipment::lauterDeadspace_l() const     { return get("lauter_deadspace").toDouble(); }
double Equipment::topUpKettle_l() const         { return get("top_up_kettle").toDouble(); }
double Equipment::hopUtilization_pct() const    { return get("hop_utilization").toDouble(); }
double Equipment::grainAbsorption_LKg()         { return get("absorption").toDouble(); }
double Equipment::boilingPoint_c() const        { return get("boiling_point").toDouble(); }

void Equipment::doCalculations()
{
   // Only do the calculation if we're asked to.
   if( ! calcBoilVolume() )
      return;

   setBoilSize_l( batchSize_l() - topUpWater_l() + trubChillerLoss_l() + (boilTime_min()/(double)60)*evapRate_lHr());
}

double Equipment::wortEndOfBoil_l( double kettleWort_l ) const
{
   //return kettleWort_l * (1 - (boilTime_min/(double)60) * (evapRate_pctHr/(double)100) );

   return kettleWort_l - (boilTime_min()/(double)60)*evapRate_lHr();
}
