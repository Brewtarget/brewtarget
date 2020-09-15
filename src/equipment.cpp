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

#include "TableSchemaConst.h"
#include "EquipmentSchema.h"

bool operator<(Equipment &e1, Equipment &e2)
{
   return e1.name() < e2.name();
}

bool operator==(Equipment &e1, Equipment &e2)
{
   return e1.name() == e2.name();
}

//=============================CONSTRUCTORS=====================================
Equipment::Equipment(QString t_name, bool cacheOnly)
   : Ingredient(Brewtarget::EQUIPTABLE, -1, t_name, true),
   m_boilSize_l(22.927),
   m_batchSize_l(18.927),
   m_tunVolume_l(0.0),
   m_tunWeight_kg(0.0),
   m_tunSpecificHeat_calGC(0.0),
   m_topUpWater_l(0.0),
   m_trubChillerLoss_l(1.0),
   m_evapRate_pctHr(0.0),
   m_evapRate_lHr(4.0),
   m_boilTime_min(60.0),
   m_calcBoilVolume(true),
   m_lauterDeadspace_l(0.0),
   m_topUpKettle_l(0.0),
   m_hopUtilization_pct(100.0),
   m_notes(QString()),
   m_grainAbsorption_LKg(1.086),
   m_boilingPoint_c(100.0),
   m_cacheOnly(cacheOnly)
{
}

Equipment::Equipment(Brewtarget::DBTable table, int key)
   : Ingredient(table, key, QString(), true ),
   m_boilSize_l(22.927),
   m_batchSize_l(18.927),
   m_tunVolume_l(0.0),
   m_tunWeight_kg(0.0),
   m_tunSpecificHeat_calGC(0.0),
   m_topUpWater_l(0.0),
   m_trubChillerLoss_l(1.0),
   m_evapRate_pctHr(0.0),
   m_evapRate_lHr(4.0),
   m_boilTime_min(60.0),
   m_calcBoilVolume(true),
   m_lauterDeadspace_l(0.0),
   m_topUpKettle_l(0.0),
   m_hopUtilization_pct(100.0),
   m_notes(QString()),
   m_grainAbsorption_LKg(1.086),
   m_boilingPoint_c(100.0),
   m_cacheOnly(false)
{
}

Equipment::Equipment(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool(), rec.value(kcolFolder).toString()),
   m_boilSize_l(rec.value(kcolEquipBoilSize).toDouble()),
   m_batchSize_l(rec.value(kcolEquipBatchSize).toDouble()),
   m_tunVolume_l(rec.value(kcolEquipTunVolume).toDouble()),
   m_tunWeight_kg(rec.value(kcolEquipTunWeight).toDouble()),
   m_tunSpecificHeat_calGC(rec.value(kcolEquipTunSpecHeat).toDouble()),
   m_topUpWater_l(rec.value(kcolEquipTopUpWater).toDouble()),
   m_trubChillerLoss_l(rec.value(kcolEquipTrubChillLoss).toDouble()),
   m_evapRate_pctHr(rec.value(kcolEquipEvapRate).toDouble()),
   m_evapRate_lHr(rec.value(kcolEquipRealEvapRate).toDouble()),
   m_boilTime_min(rec.value(kcolEquipBoilTime).toDouble()),
   m_calcBoilVolume(rec.value(kcolEquipCalcBoilVol).toBool()),
   m_lauterDeadspace_l(rec.value(kcolEquipLauterSpace).toDouble()),
   m_topUpKettle_l(rec.value(kcolEquipTopUpKettle).toDouble()),
   m_hopUtilization_pct(rec.value(kcolEquipHopUtil).toDouble()),
   m_notes(rec.value(kcolNotes).toString()),
   m_grainAbsorption_LKg(rec.value(kcolEquipAbsorption).toDouble()),
   m_boilingPoint_c(rec.value(kcolEquipBoilingPoint).toDouble()),
   m_cacheOnly(false)
{
}

Equipment::Equipment( Equipment const& other )
   : Ingredient(other),
   m_boilSize_l(other.m_boilSize_l),
   m_batchSize_l(other.m_batchSize_l),
   m_tunVolume_l(other.m_tunVolume_l),
   m_tunWeight_kg(other.m_tunWeight_kg),
   m_tunSpecificHeat_calGC(other.m_tunSpecificHeat_calGC),
   m_topUpWater_l(other.m_topUpWater_l),
   m_trubChillerLoss_l(other.m_trubChillerLoss_l),
   m_evapRate_pctHr(other.m_evapRate_pctHr),
   m_evapRate_lHr(other.m_evapRate_lHr),
   m_boilTime_min(other.m_boilTime_min),
   m_calcBoilVolume(other.m_calcBoilVolume),
   m_lauterDeadspace_l(other.m_lauterDeadspace_l),
   m_topUpKettle_l(other.m_topUpKettle_l),
   m_hopUtilization_pct(other.m_hopUtilization_pct),
   m_notes(other.m_notes),
   m_grainAbsorption_LKg(other.m_grainAbsorption_LKg),
   m_boilingPoint_c(other.m_boilingPoint_c),
   m_cacheOnly(other.m_cacheOnly)
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
      m_boilSize_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropBoilSize, var);
         emit changedBoilSize_l(var);
      }
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
      m_batchSize_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropBatchSize, var);
         doCalculations();
      }
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
      m_tunVolume_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTunVolume, var);
      }
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
      m_tunWeight_kg = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTunWeight, var);
      }
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
      m_tunSpecificHeat_calGC = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTunSpecHeat, var);
      }
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
      m_topUpWater_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTopUpWater,var);
         doCalculations();
      }
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
      m_trubChillerLoss_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTrubChillLoss, var);
         doCalculations();
      }
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
      m_evapRate_pctHr = var;
      m_evapRate_lHr = var/100.0 * m_batchSize_l;

      if ( ! m_cacheOnly ) {
         setEasy(kpropEvapRate, var);
         setEasy(kpropRealEvapRate, var/100.0 * batchSize_l() ); // We always use this one, so set it.
      }
      // Right now, I am claiming this needs to happen regardless m_cacheOnly.
      // I could be wrong
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
      m_evapRate_lHr = var;
      m_evapRate_pctHr = var/batchSize_l() * 100.0;
      if ( ! m_cacheOnly ) {
         setEasy(kpropRealEvapRate, var);
         setEasy(kpropEvapRate, var/batchSize_l() * 100.0 ); // We don't use it, but keep it current.
      }
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
      m_boilTime_min = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropBoilTime, var);
         emit changedBoilTime_min(var);
      }
      doCalculations();
   }
}

void Equipment::setCalcBoilVolume( bool var )
{
   m_calcBoilVolume = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropCalcBoilVol, var);
   }
   if ( var ) {
      doCalculations();
   }
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
      m_lauterDeadspace_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropLauterSpace, var);
      }
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
      m_topUpKettle_l = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropTopUpKettle, var);
      }
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
      m_hopUtilization_pct = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropHopUtil, var);
      }
   }
}

void Equipment::setNotes( const QString &var )
{
   m_notes = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropNotes, var);
   }
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
      m_grainAbsorption_LKg = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropAbsorption, var);
      }
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
      m_boilingPoint_c = var;
      if ( ! m_cacheOnly ) {
         setEasy(kpropBoilingPoint, var);
      }
   }
}

void Equipment::setCacheOnly(bool cache) { m_cacheOnly = cache; }

//============================"GET" METHODS=====================================

QString Equipment::notes() const { return m_notes; }
bool Equipment::calcBoilVolume() const { return m_calcBoilVolume; }
double Equipment::boilSize_l() const { return m_boilSize_l; }
double Equipment::batchSize_l() const { return m_batchSize_l; }
double Equipment::tunVolume_l() const { return m_tunVolume_l; }
double Equipment::tunWeight_kg() const { return m_tunWeight_kg; }
double Equipment::tunSpecificHeat_calGC() const { return m_tunSpecificHeat_calGC; }
double Equipment::topUpWater_l() const { return m_topUpWater_l; }
double Equipment::trubChillerLoss_l() const { return m_trubChillerLoss_l; }
double Equipment::evapRate_pctHr() const { return m_evapRate_pctHr; }
double Equipment::evapRate_lHr() const { return m_evapRate_lHr; }
double Equipment::boilTime_min() const { return m_boilTime_min; }
double Equipment::lauterDeadspace_l() const { return m_lauterDeadspace_l; }
double Equipment::topUpKettle_l() const { return m_topUpKettle_l; }
double Equipment::hopUtilization_pct() const { return m_hopUtilization_pct; }
double Equipment::grainAbsorption_LKg() { return m_grainAbsorption_LKg; }
double Equipment::boilingPoint_c() const { return m_boilingPoint_c; }
bool Equipment::cacheOnly() const { return m_cacheOnly; }

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
