/*
 * model/equipment.cpp is part of Brewtarget, and is Copyright the following
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
#include "model/Equipment.h"

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"

bool Equipment::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Equipment const & rhs = static_cast<Equipment const &>(other);
   // Base class will already have ensured names are equal
   return (
      this->m_boilSize_l            == rhs.m_boilSize_l            &&
      this->m_batchSize_l           == rhs.m_batchSize_l           &&
      this->m_tunVolume_l           == rhs.m_tunVolume_l           &&
      this->m_tunWeight_kg          == rhs.m_tunWeight_kg          &&
      this->m_tunSpecificHeat_calGC == rhs.m_tunSpecificHeat_calGC &&
      this->m_topUpWater_l          == rhs.m_topUpWater_l          &&
      this->m_trubChillerLoss_l     == rhs.m_trubChillerLoss_l     &&
      this->m_evapRate_pctHr        == rhs.m_evapRate_pctHr        &&
      this->m_evapRate_lHr          == rhs.m_evapRate_lHr          &&
      this->m_boilTime_min          == rhs.m_boilTime_min          &&
      this->m_lauterDeadspace_l     == rhs.m_lauterDeadspace_l     &&
      this->m_topUpKettle_l         == rhs.m_topUpKettle_l         &&
      this->m_hopUtilization_pct    == rhs.m_hopUtilization_pct
   );
}

ObjectStore & Equipment::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Equipment>::getInstance();
}


//=============================CONSTRUCTORS=====================================
Equipment::Equipment(QString t_name, bool cacheOnly) :
   NamedEntity(-1, cacheOnly, t_name, true),
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
   m_boilingPoint_c(100.0) {
   return;
}



// The default values below are set for the following fields that are not part of BeerXML 1.0 standard and so will
// not be present in BeerXML files (unless we wrote them) but will be present in the database:
//    - evapRate_lHr
//    - grainAbsorption_LKg
//    - boilingPoint_c
//
Equipment::Equipment(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   m_boilSize_l           {namedParameterBundle(PropertyNames::Equipment::boilSize_l           ).toDouble()},
   m_batchSize_l          {namedParameterBundle(PropertyNames::Equipment::batchSize_l          ).toDouble()},
   m_tunVolume_l          {namedParameterBundle(PropertyNames::Equipment::tunVolume_l          ).toDouble()},
   m_tunWeight_kg         {namedParameterBundle(PropertyNames::Equipment::tunWeight_kg         ).toDouble()},
   m_tunSpecificHeat_calGC{namedParameterBundle(PropertyNames::Equipment::tunSpecificHeat_calGC).toDouble()},
   m_topUpWater_l         {namedParameterBundle(PropertyNames::Equipment::topUpWater_l         ).toDouble()},
   m_trubChillerLoss_l    {namedParameterBundle(PropertyNames::Equipment::trubChillerLoss_l    ).toDouble()},
   m_evapRate_pctHr       {namedParameterBundle(PropertyNames::Equipment::evapRate_pctHr       ).toDouble()},
   m_evapRate_lHr         {namedParameterBundle(PropertyNames::Equipment::evapRate_lHr, 4.0)               },
   m_boilTime_min         {namedParameterBundle(PropertyNames::Equipment::boilTime_min         ).toDouble()},
   m_calcBoilVolume       {namedParameterBundle(PropertyNames::Equipment::calcBoilVolume       ).toBool()  },
   m_lauterDeadspace_l    {namedParameterBundle(PropertyNames::Equipment::lauterDeadspace_l    ).toDouble()},
   m_topUpKettle_l        {namedParameterBundle(PropertyNames::Equipment::topUpKettle_l        ).toDouble()},
   m_hopUtilization_pct   {namedParameterBundle(PropertyNames::Equipment::hopUtilization_pct   ).toDouble()},
   m_notes                {namedParameterBundle(PropertyNames::Equipment::notes                ).toString()},
   m_grainAbsorption_LKg  {namedParameterBundle(PropertyNames::Equipment::grainAbsorption_LKg, 1.086)      },
   m_boilingPoint_c       {namedParameterBundle(PropertyNames::Equipment::boilingPoint_c,      100.0)      } {
   return;
}

Equipment::Equipment(Equipment const & other) :
   NamedEntity            {other                        },
   m_boilSize_l           {other.m_boilSize_l           },
   m_batchSize_l          {other.m_batchSize_l          },
   m_tunVolume_l          {other.m_tunVolume_l          },
   m_tunWeight_kg         {other.m_tunWeight_kg         },
   m_tunSpecificHeat_calGC{other.m_tunSpecificHeat_calGC},
   m_topUpWater_l         {other.m_topUpWater_l         },
   m_trubChillerLoss_l    {other.m_trubChillerLoss_l    },
   m_evapRate_pctHr       {other.m_evapRate_pctHr       },
   m_evapRate_lHr         {other.m_evapRate_lHr         },
   m_boilTime_min         {other.m_boilTime_min         },
   m_calcBoilVolume       {other.m_calcBoilVolume       },
   m_lauterDeadspace_l    {other.m_lauterDeadspace_l    },
   m_topUpKettle_l        {other.m_topUpKettle_l        },
   m_hopUtilization_pct   {other.m_hopUtilization_pct   },
   m_notes                {other.m_notes                },
   m_grainAbsorption_LKg  {other.m_grainAbsorption_LKg  },
   m_boilingPoint_c       {other.m_boilingPoint_c       } {
   return;
}

//============================"SET" METHODS=====================================

// The logic through here is similar to what's in Hop. When either cacheOnly
// is true or setEasy return true (we didn't clone), then update the cached
// value. Unfortunately, the additional signals don't allow quite the
// compactness.
void Equipment::setBoilSize_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::boilSize_l,
                      this->m_boilSize_l,
                      this->enforceMin(var, "boil size"));
   if ( ! m_cacheOnly ) {
      // .:TBD:. Do we need a special-purpose signal here or can we not rely on the generic changed one from NamedEntity?
      emit changedBoilSize_l(var);
   }
}

void Equipment::setBatchSize_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::batchSize_l,
                      this->m_batchSize_l,
                      this->enforceMin(var, "batch size"));
   if ( ! m_cacheOnly ) {
      doCalculations();
   }
}

void Equipment::setTunVolume_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::tunVolume_l,
                      this->m_tunVolume_l,
                      this->enforceMin(var, "tun volume"));
}

void Equipment::setTunWeight_kg( double var ) {
   this->setAndNotify(PropertyNames::Equipment::tunWeight_kg,
                      this->m_tunWeight_kg,
                      this->enforceMin(var, "tun weight"));
}

void Equipment::setTunSpecificHeat_calGC( double var ) {
   this->setAndNotify(PropertyNames::Equipment::tunSpecificHeat_calGC,
                      this->m_tunSpecificHeat_calGC,
                      this->enforceMin(var, "tun specific heat"));
}

void Equipment::setTopUpWater_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::topUpWater_l,
                      this->m_topUpWater_l,
                      this->enforceMin(var, "top-up water"));
   if ( ! m_cacheOnly ) {
      doCalculations();
   }
}

void Equipment::setTrubChillerLoss_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::trubChillerLoss_l,
                      this->m_trubChillerLoss_l,
                      this->enforceMin(var, "trub chiller loss"));
   if ( ! m_cacheOnly ) {
      doCalculations();
   }
}

void Equipment::setEvapRate_pctHr( double var ) {
   // NOTE: We never use evapRate_pctHr, but we do use evapRate_lHr. So keep them
   //       synced, and implement the former in terms of the latter.
   this->setEvapRate_lHr(var/100.0 * batchSize_l());
   return;
}

void Equipment::setEvapRate_lHr( double var ) {
   // NOTE: We never use evapRate_pctHr, but we maintain here anyway.
   // Because both values are stored in the DB, and because we only want to call prepareForPropertyChange() once, we
   // can't use the setAndNotify() helper function
   this->prepareForPropertyChange(PropertyNames::Equipment::evapRate_lHr);
   this->m_evapRate_lHr = this->enforceMin(var, "evap rate");
   this->m_evapRate_pctHr = this->m_evapRate_lHr/batchSize_l() * 100.0; // We don't use it, but keep it current.
   this->propagatePropertyChange(PropertyNames::Equipment::evapRate_lHr);
   this->propagatePropertyChange(PropertyNames::Equipment::evapRate_pctHr);

   // Right now, I am claiming this needs to happen regardless m_cacheOnly.
   // I could be wrong
   doCalculations();
}

void Equipment::setBoilTime_min( double var ) {
   this->setAndNotify(PropertyNames::Equipment::boilTime_min,
                      this->m_boilTime_min,
                      this->enforceMin(var, "boil time"));
   if ( ! m_cacheOnly ) {
      // .:TBD:. Do we need a special-purpose signal here or can we not rely on the generic changed one from NamedEntity?
      emit changedBoilTime_min(var);
   }
   doCalculations();
}

void Equipment::setCalcBoilVolume( bool var ) {
   this->setAndNotify(PropertyNames::Equipment::calcBoilVolume, this->m_calcBoilVolume, var);
   if ( var ) {
      doCalculations();
   }
}

void Equipment::setLauterDeadspace_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::lauterDeadspace_l, this->m_lauterDeadspace_l, this->enforceMin(var, "deadspace"));
}

void Equipment::setTopUpKettle_l( double var ) {
   this->setAndNotify(PropertyNames::Equipment::topUpKettle_l, this->m_topUpKettle_l, this->enforceMin(var, "top-up kettle"));
}

void Equipment::setHopUtilization_pct( double var ) {
   this->setAndNotify(PropertyNames::Equipment::hopUtilization_pct, this->m_hopUtilization_pct, this->enforceMin(var, "hop utilization"));
}

void Equipment::setNotes( const QString &var ) {
   this->setAndNotify(PropertyNames::Equipment::notes, this->m_notes, var);
}

void Equipment::setGrainAbsorption_LKg(double var) {
   this->setAndNotify(PropertyNames::Equipment::grainAbsorption_LKg, this->m_grainAbsorption_LKg, this->enforceMin(var, "absorption"));
}

void Equipment::setBoilingPoint_c(double var) {
   this->setAndNotify(PropertyNames::Equipment::boilingPoint_c, this->m_boilingPoint_c, this->enforceMin(var, "boiling point of water"));
}

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

void Equipment::doCalculations() {
   // Only do the calculation if we're asked to.
   if (!this->calcBoilVolume()) {
      return;
   }

   this->setBoilSize_l(this->batchSize_l() -
                       this->topUpWater_l() +
                       this->trubChillerLoss_l() +
                       (this->boilTime_min()/(double)60)*this->evapRate_lHr());
   return;
}

double Equipment::wortEndOfBoil_l( double kettleWort_l ) const {
   //return kettleWort_l * (1 - (boilTime_min/(double)60) * (evapRate_pctHr/(double)100) );

   return kettleWort_l - (boilTime_min()/(double)60)*evapRate_lHr();
}

// Although it's a similar one-liner implementation for many subclasses of NamedEntity, we can't push the
// implementation of this down to the base class, as Recipe::uses() is templated and won't work with type erasure.
Recipe * Equipment::getOwningRecipe() {
   return ObjectStoreWrapper::findFirstMatching<Recipe>( [this](Recipe * rec) {return rec->uses(*this);} );
}
