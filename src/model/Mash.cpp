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

#include "database/ObjectStoreWrapper.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"


// This private implementation class holds all private non-virtual members of Mash
class Mash::impl {
public:

   /**
    * Constructor
    */
   impl(Mash & self) :
      self{self} {
      return;
   }

   /**
    * Destructor
    */
   ~impl() = default;

   // The ordering of MashSteps within a Mash is stored in the MashSteps.  If we remove a MashStep from the list, it
   // doesn't break the ordering, but debugging is easier if the step numbers are always sequential starting from 1.
   void setCanonicalMashStepNumbers() {
      int stepNumber = 1;
      for (auto ms : self.mashSteps()) {
         ms->setStepNumber(stepNumber++);
      }
      return;
   }

   // Member variables
   Mash & self;

   QVector<int> mashStepIds;
};

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

Mash::Mash(QString name) :
   NamedEntity{name, true},
   pimpl{std::make_unique<impl>(*this)},
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
   pimpl{std::make_unique<impl>(*this)},
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
      this->pimpl->mashStepIds.append(mashStepToAdd->key());

      // Connect signals so that we are notified when there are changes to the MashStep we just added to
      // our Mash.
      connect(mashStepToAdd.get(), &NamedEntity::changed, this, &Mash::acceptMashStepChange);
   }

   return;
}


Mash::Mash(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity            {namedParameterBundle},
   pimpl{std::make_unique<impl>(*this)},
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

// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the
// header file)
Mash::~Mash() = default;

void Mash::connectSignals() {
   for (auto mash : ObjectStoreTyped<Mash>::getInstance().getAllRaw()) {
      for (auto mashStep : mash->mashSteps()) {
         connect(mashStep, &NamedEntity::changed, mash, &Mash::acceptMashStepChange);
      }
   }
   return;
}

void Mash::setKey(int key) {
   // First call the base class function
   this->NamedEntity::setKey(key);
   // Now give our ID (key) to our MashSteps
   for (auto mashStepId : this->pimpl->mashStepIds) {
      if (!ObjectStoreWrapper::contains<MashStep>(mashStepId)) {
         // This is almost certainly a coding error, as each MashStep is owned by one Mash, but we can (probably)
         // recover by ignoring the missing MashStep.
         qCritical() << Q_FUNC_INFO << "Unable to retrieve MashStep #" << mashStepId << "for Mash #" << this->key();
      } else {
         ObjectStoreWrapper::getById<MashStep>(mashStepId)->setMashId(key);
      }
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

   this->pimpl->setCanonicalMashStepNumbers();

   qDebug() <<
      Q_FUNC_INFO << "Swapping steps" << ms1.stepNumber() << "(#" << ms1.key() << ") and " << ms2.stepNumber() <<
      " (#" << ms2.key() << ")";

   int temp = ms1.stepNumber();
   ms1.setStepNumber(ms2.stepNumber());
   ms2.setStepNumber(temp);

   int indexOf1 = this->pimpl->mashStepIds.indexOf(ms1.key());
   int indexOf2 = this->pimpl->mashStepIds.indexOf(ms2.key());

   // We can't swap them if we can't find both of them
   // There's no point swapping them if they're the same
   if (-1 == indexOf1 || -1 == indexOf2 || indexOf1 == indexOf2) {
      return;
   }

   // As of Qt 5.14 we could write:
   //    this->mashStepIds.swapItemsAt(indexOf1, indexOf2);
   // However, we still need to support slightly older versions of Qt (5.12 in particular), hence the more cumbersome
   // way here.
   std::swap(this->pimpl->mashStepIds[indexOf1], this->pimpl->mashStepIds[indexOf2]);

   return;
}


void Mash::removeAllMashSteps() {
   for (auto ms : this->mashSteps()) {
      ObjectStoreWrapper::softDelete(*ms);
   }
   this->pimpl->mashStepIds.clear();
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
   //
   // The Mash owns its MashSteps, but, for the moment at least, it's the MashStep that knows which Mash it's in
   // (and in what order) rather than the Mash which knows which MashSteps it has, so we have to ask.  The only
   // exception to this is if the Mash is not yet stored in the DB, in which case there is not yet any Mash ID to give
   // the MashSteps, so we store an internal list of them.
   //
   // .:TBD:. Do we actually ever have the case where MashSteps are added to a new Mash that is not yet saved in the DB?
   //         If not, we can get rid of this->mashStepIds and simplify a lot of this code.
   //
   int const mashId = this->key();

   QList<MashStep*> mashSteps;
   if (mashId < 0) {
      for (int ii : this->pimpl->mashStepIds) {
         mashSteps.append(ObjectStoreWrapper::getByIdRaw<MashStep>(ii));
      }
   } else {
      mashSteps = ObjectStoreWrapper::findAllMatching<MashStep>(
         [mashId](MashStep const * ms) {return ms->getMashId() == mashId;}
      );

      // Now we've got the MashSteps, we need to make sure they're in the right order
      std::sort(mashSteps.begin(),
                mashSteps.end(),
                [](MashStep const * lhs, MashStep const * rhs) { return lhs->stepNumber() < rhs->stepNumber(); });
   }

   return mashSteps;
}

void Mash::acceptMashStepChange(QMetaProperty prop, QVariant /*val*/) {
   MashStep* stepSender = qobject_cast<MashStep*>(sender());
   if (stepSender == nullptr) {
      return;
   }

   // If one of our mash steps changed, our calculated properties may also change, so we need to emit some signals
   if (stepSender->getMashId() == this->key()) {
      emit changed(metaProperty(*PropertyNames::Mash::totalMashWater_l), QVariant());
      emit changed(metaProperty(*PropertyNames::Mash::totalTime), QVariant());
   }

   return;
}

std::shared_ptr<MashStep> Mash::addMashStep(std::shared_ptr<MashStep> mashStep) {
   if (this->key() > 0) {
      qDebug() << Q_FUNC_INFO << "Add MashStep #" << mashStep->key() << "to Mash #" << this->key();
      mashStep->setMashId(this->key());
   }

   mashStep->setStepNumber(this->mashSteps().size() + 1);

   // MashStep needs to be in the DB for us to add it to the Mash
   if (mashStep->key() < 0) {
      qDebug() << Q_FUNC_INFO << "Inserting MashStep in DB for Mash #" << this->key();
      ObjectStoreWrapper::insert(mashStep);
   }

   Q_ASSERT(mashStep->key() > 0);

   //
   // If the Mash itself is not yet stored in the DB then it needs to hang on to its list of MashSteps so that, when the
   // Mash does get stored, it can tell all the MashSteps what their Mash ID is (see Mash::setKey()).
   //
   // (Conversely, if the Mash is in the DB, then we don't need to do anything further.  We can get all our MashSteps
   // any time by just asking the relevant ObjectStore for all MashSteps with Mash ID the same as ours.)
   //
   if (this->key() < 0) {
      qDebug() << Q_FUNC_INFO << "Adding MashStep #" << mashStep->key() << "to Mash #" << this->key();
      this->pimpl->mashStepIds.append(mashStep->key());
   }

   return mashStep;
}

std::shared_ptr<MashStep> Mash::removeMashStep(std::shared_ptr<MashStep> mashStep) {
   // Disassociate the MashStep from this Mash
   mashStep->setMashId(-1);

   // As per Mash::addMashStep(), if we're not yet stored in the database, then we also need to update our list of
   // MashSteps.
   if (this->key() < 0) {
      int indexOfStep = this->pimpl->mashStepIds.indexOf(mashStep->key());
      if (indexOfStep < 0 ) {
         // This shouldn't happen, but it doesn't inherently break anything, so just log a warning and carry on
         qWarning() <<
            Q_FUNC_INFO << "Tried to remove MashStep #" << mashStep->key() << " (from unsaved Mash #" << this->key() <<
            ") but couldn't find it";
      } else {
         this->pimpl->mashStepIds.removeAt(indexOfStep);
      }
   }

   //
   // Since a Mash owns its MashSteps, we need to remove the MashStep from the DB when we remove it from the Mash.  It
   // then makes sense (in the context of undo/redo) to put the MashStep object back into "new" state, which
   // ObjectStoreTyped will do for us.
   //
   ObjectStoreWrapper::hardDelete(mashStep);

   this->pimpl->setCanonicalMashStepNumbers();

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
