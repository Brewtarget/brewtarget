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

#include <QObject>

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "model/MashStep.h"
#include "model/Recipe.h"

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
      // .:TBD:. Should we check MashSteps too?
   );
}

ObjectStore & Mash::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Mash>::getInstance();
}

Mash::Mash(QString name, bool cache) :
   NamedEntity(-1, cache, name, true),
   m_grainTemp_c(0.0),
   m_notes(QString()),
   m_tunTemp_c(0.0),
   m_spargeTemp_c(0.0),
   m_ph(0.0),
   m_tunWeight_kg(0.0),
   m_tunSpecificHeat_calGC(0.0),
   m_equipAdjust(true) {
   return;
}

Mash::Mash(Mash const & other) :
   NamedEntity{other},
   m_grainTemp_c          {other.m_grainTemp_c          },
   m_notes                {other.m_notes                },
   m_tunTemp_c            {other.m_tunTemp_c            },
   m_spargeTemp_c         {other.m_spargeTemp_c         },
   m_ph                   {other.m_ph                   },
   m_tunWeight_kg         {other.m_tunWeight_kg         },
   m_tunSpecificHeat_calGC{other.m_tunSpecificHeat_calGC},
   m_equipAdjust          {other.m_equipAdjust          } {

   // Deep copy of MashSteps
   for (auto mashStep : other.mashSteps()) {
      // Make a copy of the current MashStep object we're looking at in the other Mash
      auto mashStepToAdd = std::make_shared<MashStep>(*mashStep);

      // This is where things get a bit tricky.
      // We don't have an ID yet, so we can't give it to the new MashStep
      mashStepToAdd->setMashId(-1);

      // However, if we insert the new MashStep in the object store, that will give it its own ID
      ObjectStoreWrapper::insert(mashStepToAdd);

      // Store the ID of the copy MashStep
      // If and when we get our ID then we can give it to our MashSteps
      // .:TBD:. It would be nice to find a more automated way of doing this
      this->mashStepIds.append(mashStepToAdd->key());

      // Connect signals so that we are notified when there are changes to the MashStep we just added to
      // our Mash.
      connect(mashStepToAdd.get(), &NamedEntity::changed, this, &Mash::acceptMashStepChange);
   }

   return;
}


Mash::Mash(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity            {namedParameterBundle},
   m_grainTemp_c          {namedParameterBundle(PropertyNames::Mash::grainTemp_c          ).toDouble()},
   m_notes                {namedParameterBundle(PropertyNames::Mash::notes                ).toString()},
   m_tunTemp_c            {namedParameterBundle(PropertyNames::Mash::tunTemp_c            ).toDouble()},
   m_spargeTemp_c         {namedParameterBundle(PropertyNames::Mash::spargeTemp_c         ).toDouble()},
   m_ph                   {namedParameterBundle(PropertyNames::Mash::ph                   ).toDouble()},
   m_tunWeight_kg         {namedParameterBundle(PropertyNames::Mash::tunWeight_kg         ).toDouble()},
   m_tunSpecificHeat_calGC{namedParameterBundle(PropertyNames::Mash::tunSpecificHeat_calGC).toDouble()},
   m_equipAdjust          {namedParameterBundle(PropertyNames::Mash::equipAdjust          ).toBool()} {
   return;
}

void Mash::connectSignals() {
   for (auto mash : ObjectStoreTyped<Mash>::getInstance().getAllRaw()) {
      for (auto mashStep : mash->mashSteps()) {
         connect(mashStep, SIGNAL(changed(QMetaProperty,QVariant)), mash, SLOT(acceptMashStepChange(QMetaProperty,QVariant)) );
      }
   }
   return;
}

void Mash::setKey(int key) {
   // First call the base class function
   this->NamedEntity::setKey(key);
   // Now give our ID (key) to our MashSteps
   for (auto mashStepId : this->mashStepIds) {
      ObjectStoreWrapper::getById<MashStep>(mashStepId)->setMashId(key);
   }
   return;
}


void Mash::setGrainTemp_c(double var) {
   this->setAndNotify(PropertyNames::Mash::grainTemp_c, this->m_grainTemp_c, var);
}

void Mash::setNotes(QString const & var) {
   this->setAndNotify(PropertyNames::Mash::notes, this->m_notes, var);
}

void Mash::setTunTemp_c(double var) {
   this->setAndNotify(PropertyNames::Mash::tunTemp_c, this->m_tunTemp_c, var);
}

void Mash::setSpargeTemp_c(double var) {
   this->setAndNotify(PropertyNames::Mash::spargeTemp_c, this->m_spargeTemp_c, var);
}

void Mash::setEquipAdjust(bool var) {
   this->setAndNotify(PropertyNames::Mash::equipAdjust, this->m_equipAdjust, var);
}

void Mash::setPh(double var) {
   this->setAndNotify(PropertyNames::Mash::ph, this->m_ph, this->enforceMinAndMax(var, "pH", 0.0, 14.0, 7.0));
}

void Mash::setTunWeight_kg(double var) {
   this->setAndNotify(PropertyNames::Mash::tunWeight_kg, this->m_tunWeight_kg, this->enforceMin(var, "tun weight"));
}

void Mash::setTunSpecificHeat_calGC(double var) {
   this->setAndNotify(PropertyNames::Mash::tunSpecificHeat_calGC, this->m_tunSpecificHeat_calGC, this->enforceMin(var, "specific heat"));
}

void Mash::swapMashSteps(MashStep & ms1, MashStep & ms2) {
   // It's a coding error if either of the steps does not belong to this mash
   Q_ASSERT(ms1.getMashId() == this->key());
   Q_ASSERT(ms2.getMashId() == this->key());

   // It's also a coding error if we're trying to swap a step with itself
   Q_ASSERT(ms1.key() != ms2.key());

   int temp = ms1.stepNumber();
   ms1.setStepNumber(ms2.stepNumber());
   ms2.setStepNumber(temp);

   int indexOf1 = this->mashStepIds.indexOf(ms1.key());
   int indexOf2 = this->mashStepIds.indexOf(ms2.key());

   // We can't swap them if we can't find both of them
   // There's no point swapping them if they're the same
   if (-1 == indexOf1 || -1 == indexOf2 || indexOf1 == indexOf2) {
      return;
   }

   // As of Qt 5.14 we could write:
   //    this->mashStepIds.swapItemsAt(indexOf1, indexOf2);
   // However, we still need to support slightly older versions of Qt (5.12 in particular), hence the more cumbersome
   // way here.
   std::swap(this->mashStepIds[indexOf1], this->mashStepIds[indexOf2]);

   return;
}


void Mash::removeAllMashSteps() {
   for (int ii : this->mashStepIds) {
      ObjectStoreTyped<MashStep>::getInstance().softDelete(ii);
   }
   this->mashStepIds.clear();
   emit mashStepsChanged();
   return;
}

//============================="GET" METHODS====================================
QString Mash::notes() const { return m_notes; }

double Mash::grainTemp_c() const { return m_grainTemp_c; }

double Mash::tunTemp_c() const { return m_tunTemp_c; }

double Mash::spargeTemp_c() const { return m_spargeTemp_c; }

double Mash::ph() const { return m_ph; }

double Mash::tunWeight_kg() const { return m_tunWeight_kg; }

double Mash::tunSpecificHeat_calGC() const { return m_tunSpecificHeat_calGC; }

bool Mash::equipAdjust() const { return m_equipAdjust; }

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

QList<MashStep*> Mash::mashSteps() const {
   // The Mash owns its MashSteps, but, for the moment at least, it's the MashStep that knows which Mash it's in
   // (and in what order) rather than the Mash which knows which MashSteps it has, so we have to ask.
   int const mashId = this->key();
   QList<MashStep*> mashSteps = ObjectStoreTyped<MashStep>::getInstance().findAllMatching(
      [mashId](MashStep const * ms) {return ms->getMashId() == mashId;}
   );

   // Now we've got the MashSteps, we need to make sure they're in the right order
   std::sort(mashSteps.begin(),
             mashSteps.end(),
             [](MashStep const * lhs, MashStep const * rhs) { return lhs->stepNumber() < rhs->stepNumber(); });

   return mashSteps;
}

void Mash::acceptMashStepChange(QMetaProperty prop, QVariant /*val*/) {
   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if( stepSender == 0 )
      return;

   // If one of our mash steps changed, our calculated properties
   // may also change, so we need to emit some signals.
   int i = mashSteps().indexOf(stepSender);
   if( i >= 0 ) {
      emit changed(metaProperty(PropertyNames::Mash::totalMashWater_l), QVariant());
      emit changed(metaProperty(PropertyNames::Mash::totalTime), QVariant());
   }
}

MashStep * Mash::addMashStep(MashStep * mashStep) {
   if (this->key() > 0) {
      mashStep->setMashId(this->key());
   } else {
      this->mashStepIds.append(mashStep->key());
   }
   return mashStep;
}

MashStep * Mash::removeMashStep(MashStep * mashStep) {
   mashStep->setMashId(-1);

   int indexOfStep = this->mashStepIds.indexOf(mashStep->key());
   if (indexOfStep < 0 ) {
      // This shouldn't happen, but it doesn't inherently break anything, so just log a warning and carry on
      qWarning() <<
         Q_FUNC_INFO << "Tried to remove MashStep #" << mashStep->key() << " (from Mash #" << this->key() <<
         ") but couldn't find it";
      return mashStep;
   }

   this->mashStepIds.removeAt(indexOfStep);
//   ObjectStoreWrapper::updateProperty(*this, PropertyNames::Mash::mashStepIds);

   return mashStep;
}

Recipe * Mash::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}

void Mash::hardDeleteOwnedEntities() {
   // It's the MashStep that stores its Mash ID, so all we need to do is delete our MashSteps then the subsequent
   // database delete of this Mash won't hit any foreign key problems.
   auto mashSteps = this->mashSteps();
   for (auto mashStep : mashSteps) {
      ObjectStoreWrapper::hardDelete<MashStep>(*mashStep);
   }
   return;
}
