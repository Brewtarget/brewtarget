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

/************* Columns *************/
const QString kName("name");
const QString kBoilSize("boil_size");
const QString kBatchSize("batch_size");
const QString kTunVolume("tun_volume");
const QString kTunWeight("tun_weight");
const QString kTunSpecificHeat("tun_specific_heat");
const QString kTopUpWater("top_up_water");
const QString kTrubChillerLoss("trub_chiller_loss");
const QString kEvaporationRate("evap_rate");
const QString kRealEvaporationRate("real_evap_rate");
const QString kBoilTime("boil_time");
const QString kCalcBoilVolume("calc_boil_volume");
const QString kLauterDeadspace("lauter_deadspace");
const QString kTopUpKettle("top_up_kettle");
const QString kHopUtilization("hop_utilization");
const QString kNotes("notes");
const QString kAbsorption("absorption");
const QString kBoilingPoint("boiling_point");

/************** Props **************/

const QString kNameProp("name");
const QString kBoilSizeProp("boilSize_l");
const QString kBatchSizeProp("batchSize_l");
const QString kTunVolumeProp("tunVolume_l");
const QString kTunWeightProp("tunWeight_kg");
const QString kTunSpecificHeatProp("tunSpecificHeat_calGC");
const QString kTopUpWaterProp("topUpWater_l");
const QString kTrubChillerLossProp("trubChillerLoss_l");
const QString kEvaporationRateProp("evapRate_pctHr");
const QString kRealEvaporationRateProp("evapRate_lHr");
const QString kBoilTimeProp("boilTime_min");
const QString kCalcBoilVolumeProp("calcBoilVolume");
const QString kLauterDeadspaceProp("lauterDeadspace_l");
const QString kTopUpKettleProp("topUpKettle_l");
const QString kHopUtilizationProp("hopUtilization_pct");
const QString kNotesProp("notes");
const QString kGrainAbsorptionProp("grainAbsorption_LKg");
const QString kAbsorptionProp("absorption_LKg");
const QString kBoildPointProp("boilingPoint_c");


QHash<QString,QString> Equipment::tagToProp = Equipment::tagToPropHash();

QHash<QString,QString> Equipment::tagToPropHash()
{
   QHash<QString,QString> propHash;

   propHash["NAME"] = kNameProp;
   propHash["BOIL_SIZE"] = kBoilSizeProp;
   propHash["BATCH_SIZE"] = kBatchSizeProp;
   propHash["TUN_VOLUME"] = kTunVolumeProp;
   propHash["TUN_WEIGHT"] = kTunWeightProp;
   propHash["TUN_SPECIFIC_HEAT"] = kTunSpecificHeatProp;
   propHash["TOP_UP_WATER"] = kTopUpWaterProp;
   propHash["TRUB_CHILLER_LOSS"] = kTrubChillerLossProp;
   propHash["EVAP_RATE"] = kEvaporationRateProp;
   propHash["REAL_EVAP_RATE"] = kRealEvaporationRateProp;
   propHash["BOIL_TIME"] = kBoilTimeProp;
   propHash["CALC_BOIL_VOLUME"] = kCalcBoilVolumeProp;
   propHash["LAUTER_DEADSPACE"] = kLauterDeadspaceProp;
   propHash["TOP_UP_KETTLE"] = kTopUpKettleProp;
   propHash["HOP_UTILIZATION"] = kHopUtilizationProp;
   propHash["NOTES"] = kNotesProp;
   propHash["ABSORPTION"] = kGrainAbsorptionProp;
   propHash["BOILING_POINT"] = kBoildPointProp;

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
Equipment::Equipment(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

Equipment::Equipment( Equipment const& other )
   : BeerXMLElement(other)
{
}

QString Equipment::classNameStr()
{
   static const QString name("Equipment");
   return name;
}

//============================"SET" METHODS=====================================

void Equipment::setBoilSize_l( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil size negative: %1").arg(var) );
      return;
   }
   else
   {
      set(kBoilSizeProp, kBoilSize, var);
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
      set(kBatchSizeProp, kBatchSize, var);
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
      set(kTunVolumeProp, kTunVolume, var);
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
      set(kTunWeightProp, kTunWeight, var);
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
      set(kTunSpecificHeatProp, kTunSpecificHeat, var);
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
      set(kTopUpWaterProp, kTopUpWater, var);
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
      set(kTrubChillerLossProp, kTrubChillerLoss, var);
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
      set(kEvaporationRateProp, kEvaporationRate, var);
      set(kRealEvaporationRateProp, kRealEvaporationRate, var/100.0 * batchSize_l() ); // We always use this one, so set it.
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
      set(kRealEvaporationRateProp, kRealEvaporationRate, var);
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
      set(kBoilTimeProp, kBoilTime, var);
      emit changedBoilTime_min(var);
      doCalculations();
   }
}

void Equipment::setCalcBoilVolume( bool var )
{
   set(kCalcBoilVolumeProp, kCalcBoilVolume, var);
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
      set(kLauterDeadspaceProp, kLauterDeadspace, var);
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
      set(kTopUpKettleProp, kTopUpKettle, var);
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
      set(kHopUtilizationProp, kHopUtilization, var);
   }
}

void Equipment::setNotes( const QString &var )
{
   set(kNotesProp, kNotes, var);
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
      set(kAbsorptionProp, kAbsorption, var);
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
      set(kBoildPointProp, kBoilingPoint, var);
   }
}

//============================"GET" METHODS=====================================

QString Equipment::notes() const
{
   return get(kNotes).toString();
}

bool Equipment::calcBoilVolume() const
{
   return get(kCalcBoilVolume).toBool();
}

double Equipment::boilSize_l() const
{
   return get(kBoilSize).toDouble();
}

double Equipment::batchSize_l() const
{
   return get(kBatchSize).toDouble();
}

double Equipment::tunVolume_l() const
{
   return get(kTunVolume).toDouble();
}

double Equipment::tunWeight_kg() const
{
   return get(kTunWeight).toDouble();
}

double Equipment::tunSpecificHeat_calGC() const
{
   return get(kTunSpecificHeat).toDouble();
}

double Equipment::topUpWater_l() const
{
   return get(kTopUpWater).toDouble();
}

double Equipment::trubChillerLoss_l() const
{
   return get(kTrubChillerLoss).toDouble();
}

double Equipment::evapRate_pctHr() const
{
   return get(kEvaporationRate).toDouble();
}

double Equipment::evapRate_lHr() const
{
   return get(kRealEvaporationRate).toDouble();
}

double Equipment::boilTime_min() const
{
   return get(kBoilTime).toDouble();
}

double Equipment::lauterDeadspace_l() const
{
   return get(kLauterDeadspace).toDouble();
}

double Equipment::topUpKettle_l() const
{
   return get(kTopUpKettle).toDouble();
}

double Equipment::hopUtilization_pct() const
{
   return get(kHopUtilization).toDouble();
}

double Equipment::grainAbsorption_LKg()
{
   return get(kAbsorption).toDouble();
}

double Equipment::boilingPoint_c() const
{
   return get(kBoilingPoint).toDouble();
}

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
