/*
 * model/Mash.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/Mash.h"

#include <iostream>
#include <string>

#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QVector>

#include "brewtarget.h"
#include "database.h"
#include "MashSchema.h"
#include "model/MashStep.h"
#include "TableSchemaConst.h"

bool Mash::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Mash const & rhs = static_cast<Mash const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_grainTemp_c           == rhs.m_grainTemp_c           &&
      this->m_tunTemp_c             == rhs.m_tunTemp_c             &&
      this->m_spargeTemp_c          == rhs.m_spargeTemp_c          &&
      this->m_ph                    == rhs.m_ph                    &&
      this->m_tunWeight_kg          == rhs.m_tunWeight_kg          &&
      this->m_tunSpecificHeat_calGC == rhs.m_tunSpecificHeat_calGC
   );
}


QString Mash::classNameStr()
{
   static const QString name("Mash");
   return name;
}

Mash::Mash(QString name, bool cache)
   : NamedEntity(Brewtarget::MASHTABLE, name, true),
     m_grainTemp_c(0.0),
     m_notes(QString()),
     m_tunTemp_c(0.0),
     m_spargeTemp_c(0.0),
     m_ph(0.0),
     m_tunWeight_kg(0.0),
     m_tunSpecificHeat_calGC(0.0),
     m_equipAdjust(true),
     m_cacheOnly(cache)
{
}

Mash::Mash(TableSchema* table, QSqlRecord rec, int t_key)
   : NamedEntity(table, rec, t_key),
     m_cacheOnly(false)
{
     m_grainTemp_c = rec.value( table->propertyToColumn(PropertyNames::Mash::grainTemp_c)).toDouble();
     m_notes = rec.value( table->propertyToColumn(PropertyNames::Mash::notes)).toString();
     m_tunTemp_c = rec.value( table->propertyToColumn(PropertyNames::Mash::tunTemp_c)).toDouble();
     m_spargeTemp_c = rec.value( table->propertyToColumn(PropertyNames::Mash::spargeTemp_c)).toDouble();
     m_ph = rec.value( table->propertyToColumn(PropertyNames::Mash::ph)).toDouble();
     m_tunWeight_kg = rec.value( table->propertyToColumn(PropertyNames::Mash::tunWeight_kg)).toDouble();
     m_tunSpecificHeat_calGC = rec.value( table->propertyToColumn(PropertyNames::Mash::tunSpecificHeat_calGC)).toDouble();
     m_equipAdjust = rec.value( table->propertyToColumn(PropertyNames::Mash::equipAdjust)).toBool();

}

void Mash::setGrainTemp_c( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::grainTemp_c, var) ) {
      m_grainTemp_c = var;
   }
}

void Mash::setNotes( const QString& var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::notes, var) ) {
      m_notes = var;
   }
}

void Mash::setTunTemp_c( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::tunTemp_c, var) ) {
      m_tunTemp_c = var;
   }
}

void Mash::setSpargeTemp_c( double var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::spargeTemp_c, var) ) {
      m_spargeTemp_c = var;
   }
}

void Mash::setEquipAdjust( bool var )
{
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::equipAdjust, var) ) {
      m_equipAdjust = var;
   }
}

void Mash::setPh( double var )
{
   if( var < 0.0 || var > 14.0 ) {
      qWarning() << QString("Mash: 0 < pH < 14: %1").arg(var);
      return;
   }
   if ( m_cacheOnly || setEasy(PropertyNames::Mash::ph, var) ) {
      m_ph = var;
   }
}

void Mash::setTunWeight_kg( double var )
{
   if( var < 0.0 )
   {
      qWarning() << QString("Mash: tun weight < 0: %1").arg(var);
      return;
   }

   if ( m_cacheOnly || setEasy(PropertyNames::Mash::tunWeight_kg, var) ) {
      m_tunWeight_kg = var;
   }
}

void Mash::setTunSpecificHeat_calGC( double var )
{
   if( var < 0.0 ) {
      qWarning() << QString("Mash: sp heat < 0: %1").arg(var);
      return;
   }

   if ( m_cacheOnly || setEasy(PropertyNames::Mash::tunSpecificHeat_calGC, var) ) {
      m_tunSpecificHeat_calGC = var;
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

void Mash::setCacheOnly(bool cache) { m_cacheOnly = cache; }

//============================="GET" METHODS====================================
QString Mash::notes() const { return m_notes; }

double Mash::grainTemp_c() const { return m_grainTemp_c; }

double Mash::tunTemp_c() const { return m_tunTemp_c; }

double Mash::spargeTemp_c() const { return m_spargeTemp_c; }

double Mash::ph() const { return m_ph; }

double Mash::tunWeight_kg() const { return m_tunWeight_kg; }

double Mash::tunSpecificHeat_calGC() const { return m_tunSpecificHeat_calGC; }

bool Mash::equipAdjust() const { return m_equipAdjust; }

bool Mash::cacheOnly() const { return m_cacheOnly; }

// === other methods ===
double Mash::totalMashWater_l()
{
   int i, size;
   double waterAdded_l = 0.0;
   QList<MashStep*> steps = mashSteps();
   MashStep* step;

   size = steps.size();
   for( i = 0; i < size; ++i ) {
      step = steps[i];

      if( step->isInfusion() )
         waterAdded_l += step->infuseAmount_l();
   }

   return waterAdded_l;
}

double Mash::totalInfusionAmount_l() const
{
   double waterAdded_l = 0.0;

   foreach( MashStep* i, mashSteps() ) {
      if( i->isInfusion() && ! i->isSparge() )
         waterAdded_l += i->infuseAmount_l();
   }

   return waterAdded_l;
}

double Mash::totalSpargeAmount_l() const
{
   double waterAdded_l = 0.0;

   foreach( MashStep* i, mashSteps() ) {
      if( i->isSparge() )
         waterAdded_l += i->infuseAmount_l();
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

bool Mash::hasSparge() const
{
   foreach( MashStep* ms, mashSteps() ) {
      if ( ms->isSparge() ) {
         return true;
      }
   }

   return false;
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

MashStep * Mash::addMashStep(MashStep * mashStep) {
   mashStep->setMash(this);
   mashStep->insertInDatabase();
   return mashStep;
}

MashStep * Mash::removeMashStep(MashStep * mashStep) {
   Database::instance().removeFrom(this, mashStep);
   return mashStep;
}


int Mash::insertInDatabase() {
   return Database::instance().insertMash(this);
}

void Mash::removeFromDatabase() {
   Database::instance().remove(this);
}
