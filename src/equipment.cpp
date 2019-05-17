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

// these are defined in the parent, but I need them here too
const QString kDeleted("deleted");
const QString kDisplay("display");
const QString kFolder("folder");
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

Equipment::Equipment(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   setName( rec.value(kName).toString(), true );
   setDisplay( rec.value(kDisplay).toBool(), true);
   setDeleted( rec.value(kDeleted).toBool(), true);
   setFolder( rec.value(kFolder).toString(), false, true);
   _boilSize_l = rec.value(kBoilSize).toDouble();
   _batchSize_l = rec.value(kBatchSize).toDouble();
   _tunVolume_l = rec.value(kTunVolume).toDouble();
   _tunWeight_kg = rec.value(kTunWeight).toDouble();
   _tunSpecificHeat_calGC = rec.value(kTunSpecificHeat).toDouble();
   _topUpWater_l = rec.value(kTopUpWater).toDouble();
   _trubChillerLoss_l = rec.value(kTrubChillerLoss).toDouble();
   _evapRate_pctHr = rec.value(kEvaporationRate).toDouble();
   _evapRate_lHr = rec.value(kRealEvaporationRate).toDouble();
   _boilTime_min = rec.value(kBoilTime).toDouble();
   _calcBoilVolume = rec.value(kCalcBoilVolume).toBool();
   _lauterDeadspace_l = rec.value(kLauterDeadspace).toDouble();
   _topUpKettle_l = rec.value(kTopUpKettle).toDouble();
   _hopUtilization_pct = rec.value(kHopUtilization).toDouble();
   _notes = rec.value(kNotes).toString();
   _grainAbsorption_LKg =rec.value(kAbsorption).toDouble();
   _boilingPoint_c =rec.value(kBoilingPoint).toDouble();
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

void Equipment::setBoilSize_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil size negative: %1").arg(var) );
      return;
   }
   else
   {
      _boilSize_l = var;
      if ( ! cachedOnly ) {
         set(kBoilSizeProp, kBoilSize, var);
         emit changedBoilSize_l(var);
      }
   }
}

void Equipment::setBatchSize_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: batch size negative: %1").arg(var) );
      return;
   }
   else
   {
      _batchSize_l = var;
      if ( ! cachedOnly ) {
         set(kBatchSizeProp, kBatchSize, var);
         doCalculations();
      }
   }
}

void Equipment::setTunVolume_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun volume negative: %1").arg(var) );
      return;
   }
   else
   {
      _tunVolume_l = var;
      if ( ! cachedOnly ) {
         set(kTunVolumeProp, kTunVolume, var);
      }
   }
}

void Equipment::setTunWeight_kg( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun weight negative: %1").arg(var) );
      return;
   }
   else
   {
      _tunWeight_kg = var;
      if ( ! cachedOnly ) {
         set(kTunWeightProp, kTunWeight, var);
      }
   }
}

void Equipment::setTunSpecificHeat_calGC( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: tun sp heat negative: %1").arg(var) );
      return;
   }
   else
   {
      _tunSpecificHeat_calGC = var;
      if ( ! cachedOnly ) {
         set(kTunSpecificHeatProp, kTunSpecificHeat, var);
      }
   }
}

void Equipment::setTopUpWater_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up water negative: %1").arg(var) );
      return;
   }
   else
   {
      _topUpWater_l = var;
      if ( ! cachedOnly ) {
         set(kTopUpWaterProp, kTopUpWater, var);
         doCalculations();
      }
   }
}

void Equipment::setTrubChillerLoss_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: trub chiller loss negative: %1").arg(var) );
      return;
   }
   else
   {
      _trubChillerLoss_l = var;
      if ( ! cachedOnly ) {
         set(kTrubChillerLossProp, kTrubChillerLoss, var);
         doCalculations();
      }
   }
}

void Equipment::setEvapRate_pctHr( double var, bool cachedOnly )
{
   if( var < 0.0 || var > 100.0)
   {
      Brewtarget::logW( QString("Equipment: 0 < evap rate < 100: %1").arg(var) );
      return;
   }
   else
   {
      _evapRate_pctHr = var;
      _evapRate_lHr = var/100.0 * _batchSize_l;

      if ( ! cachedOnly ) {
         set(kEvaporationRateProp, kEvaporationRate, var);
         set(kRealEvaporationRateProp, kRealEvaporationRate, var/100.0 * batchSize_l() ); // We always use this one, so set it.
         doCalculations();
      }
   }
}

void Equipment::setEvapRate_lHr( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: evap rate negative: %1").arg(var) );
      return;
   }
   else
   {
      _evapRate_lHr = var;
      _evapRate_pctHr = var/batchSize_l() * 100.0;
      if ( ! cachedOnly ) {
         set(kRealEvaporationRateProp, kRealEvaporationRate, var);
         setEvapRate_pctHr( var/batchSize_l() * 100.0 ); // We don't use it, but keep it current.
         doCalculations();
      }
   }
}

void Equipment::setBoilTime_min( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boil time negative: %1").arg(var) );
      return;
   }
   else
   {
      _boilTime_min = var;
      if ( ! cachedOnly ) {
         set(kBoilTimeProp, kBoilTime, var);
         emit changedBoilTime_min(var);
         doCalculations();
      }
   }
}

void Equipment::setCalcBoilVolume( bool var, bool cachedOnly )
{
   _calcBoilVolume = var;
   if ( ! cachedOnly ) {
      set(kCalcBoilVolumeProp, kCalcBoilVolume, var);
      if( var )
         doCalculations();
   }
}

void Equipment::setLauterDeadspace_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: deadspace negative: %1").arg(var) );
      return;
   }
   else
   {
      _lauterDeadspace_l = var;
      if ( ! cachedOnly ) {
         set(kLauterDeadspaceProp, kLauterDeadspace, var);
      }
   }
}

void Equipment::setTopUpKettle_l( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: top up kettle negative: %1").arg(var) );
      return;
   }
   else
   {
      _topUpKettle_l = var;
      if ( ! cachedOnly ) {
         set(kTopUpKettleProp, kTopUpKettle, var);
      }
   }
}

void Equipment::setHopUtilization_pct( double var, bool cachedOnly )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: 0 < hop utilization: %1").arg(var) );
      return;
   }
   else
   {
      _hopUtilization_pct = var;
      if ( ! cachedOnly ) {
         set(kHopUtilizationProp, kHopUtilization, var);
      }
   }
}

void Equipment::setNotes( const QString &var, bool cachedOnly )
{
   _notes = var;
   if ( ! cachedOnly ) {
      set(kNotesProp, kNotes, var);
   }
}

void Equipment::setGrainAbsorption_LKg(double var, bool cachedOnly)
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: absorption < 0: %1").arg(var) );
      return;
   }
   else
   {
      _grainAbsorption_LKg = var;
      if ( ! cachedOnly ) {
         set(kAbsorptionProp, kAbsorption, var);
      }
   }
}

void Equipment::setBoilingPoint_c(double var, bool cachedOnly)
{
   if ( var < 0.0 )
   {
      Brewtarget::logW( QString("Equipment: boiling point of water < 0: %1").arg(var));
      return;
   }
   else 
   {
      _boilingPoint_c = var;
      if ( ! cachedOnly ) {
         set(kBoildPointProp, kBoilingPoint, var);
      }
   }
}

//============================"GET" METHODS=====================================

QString Equipment::notes() const { return _notes; }
bool Equipment::calcBoilVolume() const { return _calcBoilVolume; }
double Equipment::boilSize_l() const { return _boilSize_l; }
double Equipment::batchSize_l() const { return _batchSize_l; }
double Equipment::tunVolume_l() const { return _tunVolume_l; }
double Equipment::tunWeight_kg() const { return _tunWeight_kg; }
double Equipment::tunSpecificHeat_calGC() const { return _tunSpecificHeat_calGC; }
double Equipment::topUpWater_l() const { return _topUpWater_l; }
double Equipment::trubChillerLoss_l() const { return _trubChillerLoss_l; }
double Equipment::evapRate_pctHr() const { return _evapRate_pctHr; }
double Equipment::evapRate_lHr() const { return _evapRate_lHr; }
double Equipment::boilTime_min() const { return _boilTime_min; }
double Equipment::lauterDeadspace_l() const { return _lauterDeadspace_l; }
double Equipment::topUpKettle_l() const { return _topUpKettle_l; }
double Equipment::hopUtilization_pct() const { return _hopUtilization_pct; }
double Equipment::grainAbsorption_LKg() { return _grainAbsorption_LKg; }
double Equipment::boilingPoint_c() const { return _boilingPoint_c; }

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
