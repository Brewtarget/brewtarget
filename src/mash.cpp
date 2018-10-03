/*
 * mash.cpp is part of Brewtarget, and is Copyright the following
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

#include <iostream>
#include <string>
#include <QVector>
#include "mash.h"
#include "mashstep.h"
#include "brewtarget.h"
#include "database.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>

/************* Columns *************/
const QString kNotes("notes");
const QString kGrainTemp("grain_temp");
const QString kTunTemp("tun_temp");
const QString kSpargeTemp("sparge_temp");
const QString kPH("ph");
const QString kTunWeight("tun_weight");
const QString kTunSpecificHeat("tun_specific_heat");
const QString kEquipAdjust("equip_adjust");

/************** Props **************/
const QString kNameProp("name");
const QString kGrainTempProp("grainTemp_c");
const QString kNotesProp("notes");
const QString kTunTempProp("tunTemp_c");
const QString kSpargeTempProp("spargeTemp_c");
const QString kPHProp("ph");
const QString kTunWeightProp("tunWeight_kg");
const QString kTunSpecificHeatProp("tunSpecificHeat_calGC");
const QString kEquipAdjustProp("equipAdjust");


QHash<QString,QString> Mash::tagToProp = Mash::tagToPropHash();

QHash<QString,QString> Mash::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   propHash["GRAIN_TEMP"] = kGrainTempProp;
   propHash["NOTES"] = kNotesProp;
   propHash["TUN_TEMP"] = kTunTempProp;
   propHash["SPARGE_TEMP"] = kSpargeTempProp;
   propHash["PH"] = kPHProp;
   propHash["TUN_WEIGHT"] = kTunWeightProp;
   propHash["TUN_SPECIFIC_HEAT"] = kTunSpecificHeatProp;
   propHash["EQUIP_ADJUST"] = kEquipAdjustProp;
   return propHash;
}

bool operator<(Mash &m1, Mash &m2)
{
   return m1.name() < m2.name();
}

bool operator==(Mash &m1, Mash &m2)
{
   return m1.name() == m2.name();
}

QString Mash::classNameStr()
{
   static const QString name("Mash");
   return name;
}

Mash::Mash(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

void Mash::setGrainTemp_c( double var )
{
   set(kGrainTempProp, kGrainTemp, var);
}

void Mash::setNotes( const QString& var )
{
   set(kNotesProp, kNotes, var);
}

void Mash::setTunTemp_c( double var )
{
   set(kTunTempProp, kTunTemp, var);
}

void Mash::setSpargeTemp_c( double var )
{
   set(kSpargeTempProp, kSpargeTemp, var);
}

void Mash::setEquipAdjust( bool var )
{
   set(kEquipAdjustProp, kEquipAdjust, var);
}

void Mash::setPh( double var )
{
   if( var < 0.0 || var > 14.0 )
   {
      Brewtarget::logW( QString("Mash: 0 < pH < 14: %1").arg(var) );
      return;
   }
   else
   {
      set(kPHProp, kPH, var);
   }
}

void Mash::setTunWeight_kg( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Mash: tun weight < 0: %1").arg(var) );
      return;
   }
   else
   {
      set(kTunWeightProp, kTunWeight, var);
   }
}

void Mash::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 )
   {
      Brewtarget::logW( QString("Mash: sp heat < 0: %1").arg(var) );
      return;
   }
   else
   {
      set(kTunSpecificHeatProp, kTunSpecificHeat, var);
   }
}

void Mash::removeAllMashSteps()
{
   int i, size;
   QList<MashStep*> tmpSteps = mashSteps();
   size = tmpSteps.size();
   for( i = 0; i < size; ++i )
      Database::instance().removeFrom(this, tmpSteps[i]);
   emit mashStepsChanged();
}

//============================="GET" METHODS====================================
QString Mash::notes() const
{
   return get(kNotes).toString();
}

double Mash::grainTemp_c() const
{
   return get(kGrainTemp).toDouble();
}

double Mash::tunTemp_c() const
{
   return get(kTunTemp).toDouble();
}

double Mash::spargeTemp_c() const
{
   return get(kSpargeTemp).toDouble();
}

double Mash::ph() const
{
   return get(kPH).toDouble();
}

double Mash::tunWeight_kg() const
{
   return get(kTunWeight).toDouble();
}

double Mash::tunSpecificHeat_calGC() const
{
   return get(kTunSpecificHeat).toDouble();
}

bool Mash::equipAdjust() const
{
   return get(kEquipAdjust).toBool();
}

// === other methods ===
double Mash::totalMashWater_l()
{
   int i, size;
   double waterAdded_l = 0.0;
   QList<MashStep*> steps = mashSteps();
   MashStep* step;
   
   size = steps.size();
   for( i = 0; i < size; ++i )
   {
      step = steps[i];
      
      if( step->isInfusion() )
         waterAdded_l += step->infuseAmount_l();
   }
   
   return waterAdded_l;
}

double Mash::totalTime()
{
   int i, size;
   double totalTime = 0.0;
   QList<MashStep*> steps = mashSteps();
   MashStep* mstep;

   size = steps.size();
   for( i = 0; i < size; ++i )
   {
      mstep = steps[i];
      totalTime += mstep->stepTime_min();
   }
   return totalTime;
}

QList<MashStep*> Mash::mashSteps() const
{
   return Database::instance().mashSteps(this);
}

void Mash::acceptMashStepChange(QMetaProperty prop, QVariant /*val*/)
{
   int i;
   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if( stepSender == 0 )
      return;
   
   // If one of our mash steps changed, our calculated properties
   // may also change, so we need to emit some signals.
   i = mashSteps().indexOf(stepSender);
   if( i >= 0 )
   {
      emit changed(metaProperty("totalMashWater_l"), QVariant());
      emit changed(metaProperty("totalTime"), QVariant());
   }
}
