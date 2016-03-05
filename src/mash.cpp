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

QHash<QString,QString> Mash::tagToProp = Mash::tagToPropHash();

QHash<QString,QString> Mash::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   propHash["GRAIN_TEMP"] = "grainTemp_c";
   propHash["NOTES"] = "notes";
   propHash["TUN_TEMP"] = "tunTemp_c";
   propHash["SPARGE_TEMP"] = "spargeTemp_c";
   propHash["PH"] = "ph";
   propHash["TUN_WEIGHT"] = "tunWeight_kg";
   propHash["TUN_SPECIFIC_HEAT"] = "tunSpecificHeat_calGC";
   propHash["EQUIP_ADJUST"] = "equipAdjust";
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

Mash::Mash()
   : BeerXMLElement()
{
}

void Mash::setName( const QString& var )
{
   set("name", "name", var);
   emit changedName(var);
}

void Mash::setGrainTemp_c( double var )
{
   set("grainTemp_c", "grain_temp", var);
}

void Mash::setNotes( const QString& var )
{
   set("notes", "notes", var);
}

void Mash::setTunTemp_c( double var )
{
   set("tunTemp_c", "tun_temp", var);
}

void Mash::setSpargeTemp_c( double var )
{
   set("spargeTemp_c", "sparge_temp", var);
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
      set("ph", "ph", var);
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
      set("tunWeight_kg", "tun_weight", var);
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
      set("tunSpecificHeat_calGC", "tun_specific_heat", var);
   }
}

void Mash::setEquipAdjust( bool var )
{
   set("equipAdjust", "equip_adjust", var);
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

QString Mash::name()                 const { return get("name").toString(); }
QString Mash::notes()                const { return get("notes").toString(); }

double Mash::grainTemp_c()           const { return get("grain_temp").toDouble(); }
double Mash::tunTemp_c()             const { return get("tun_temp").toDouble(); }
double Mash::spargeTemp_c()          const { return get("sparge_temp").toDouble(); }
double Mash::ph()                    const { return get("ph").toDouble(); }
double Mash::tunWeight_kg()          const { return get("tun_weight").toDouble(); }
double Mash::tunSpecificHeat_calGC() const { return get("tun_specific_heat").toDouble(); }

bool Mash::equipAdjust()             const { return get("equip_adjust").toBool(); }

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
