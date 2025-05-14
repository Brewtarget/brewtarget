/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Mash.cpp is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "model/Mash.h"

#include <iostream>
#include <string>

#include <QObject>

#include "database/ObjectStoreWrapper.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Mash.cpp"
#endif

QString Mash::localisedName() { return tr("Mash"); }

bool Mash::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Mash const & rhs = static_cast<Mash const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_LOG_COMPARE(this, rhs, m_grainTemp_c              ) &&
      AUTO_LOG_COMPARE(this, rhs, m_tunTemp_c                ) &&
      AUTO_LOG_COMPARE(this, rhs, m_spargeTemp_c             ) &&
      AUTO_LOG_COMPARE(this, rhs, m_ph                       ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunWeight_kg         ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunSpecificHeat_calGC) &&
      // Parent classes have to be equal too
      this->FolderBase<Mash>::doIsEqualTo(rhs) &&
      this->StepOwnerBase<Mash, MashStep>::doIsEqualTo(rhs)
   );
}

ObjectStore & Mash::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Mash>::getInstance();
}

TypeLookup const Mash::typeLookup {
   "Mash",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::equipAdjust              , Mash::m_equipAdjust     ,           NonPhysicalQuantity::Bool                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::grainTemp_c              , Mash::m_grainTemp_c     , Measurement::PhysicalQuantity::Temperature         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::notes                    , Mash::m_notes           ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::ph                       , Mash::m_ph              , Measurement::PhysicalQuantity::Acidity             ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::spargeTemp_c             , Mash::m_spargeTemp_c    , Measurement::PhysicalQuantity::Temperature         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::mashTunSpecificHeat_calGC, Mash::m_mashTunSpecificHeat_calGC, Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::tunTemp_c                , Mash::m_tunTemp_c       , Measurement::PhysicalQuantity::Temperature         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Mash::mashTunWeight_kg         , Mash::m_mashTunWeight_kg, Measurement::PhysicalQuantity::Mass                ),
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Mash::totalMashWater_l   , Mash::totalMashWater_l  , Measurement::PhysicalQuantity::Volume), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_ENTRY_NO_MV(PropertyNames::Mash::totalTime_mins     , Mash::totalTime_mins    , Measurement::PhysicalQuantity::Time  ), // Calculated, not in DB
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Mash>::typeLookup),
    std::addressof(StepOwnerBase<Mash, MashStep>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Mash>, Mash>::value);

//==================================================== CONSTRUCTORS ====================================================

Mash::Mash(QString name) :
   NamedEntity{name},
   FolderBase<Mash>{},
   StepOwnerBase<Mash, MashStep>{},
   m_grainTemp_c              {0.0 },
   m_notes                    {""  },
   m_tunTemp_c                {0.0 },
   m_spargeTemp_c             {0.0 },
   m_ph                       {0.0 },
   m_mashTunWeight_kg         {0.0 },
   m_mashTunSpecificHeat_calGC{0.0 },
   m_equipAdjust              {true} {

   CONSTRUCTOR_END
   return;
}

Mash::Mash(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity                {namedParameterBundle},
   FolderBase<Mash>{namedParameterBundle},
   StepOwnerBase<Mash, MashStep>{},
   SET_REGULAR_FROM_NPB (m_grainTemp_c              , namedParameterBundle, PropertyNames::Mash::grainTemp_c              ),
   SET_REGULAR_FROM_NPB (m_notes                    , namedParameterBundle, PropertyNames::Mash::notes                    , ""),
   SET_REGULAR_FROM_NPB (m_tunTemp_c                , namedParameterBundle, PropertyNames::Mash::tunTemp_c                ),
   SET_REGULAR_FROM_NPB (m_spargeTemp_c             , namedParameterBundle, PropertyNames::Mash::spargeTemp_c             ),
   SET_REGULAR_FROM_NPB (m_ph                       , namedParameterBundle, PropertyNames::Mash::ph                       ),
   SET_REGULAR_FROM_NPB (m_mashTunWeight_kg         , namedParameterBundle, PropertyNames::Mash::mashTunWeight_kg         ),
   SET_REGULAR_FROM_NPB (m_mashTunSpecificHeat_calGC, namedParameterBundle, PropertyNames::Mash::mashTunSpecificHeat_calGC),
   SET_REGULAR_FROM_NPB (m_equipAdjust              , namedParameterBundle, PropertyNames::Mash::equipAdjust              ) {

   CONSTRUCTOR_END
   return;
}

Mash::Mash(Mash const & other) :
   NamedEntity{other},
   FolderBase<Mash>{other},
   StepOwnerBase<Mash, MashStep>{other},
   m_grainTemp_c              {other.m_grainTemp_c              },
   m_notes                    {other.m_notes                    },
   m_tunTemp_c                {other.m_tunTemp_c                },
   m_spargeTemp_c             {other.m_spargeTemp_c             },
   m_ph                       {other.m_ph                       },
   m_mashTunWeight_kg         {other.m_mashTunWeight_kg         },
   m_mashTunSpecificHeat_calGC{other.m_mashTunSpecificHeat_calGC},
   m_equipAdjust              {other.m_equipAdjust              } {

   CONSTRUCTOR_END
   return;
}

Mash::~Mash() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
double                Mash::grainTemp_c              () const { return this->m_grainTemp_c              ; }
QString               Mash::notes                    () const { return this->m_notes                    ; }
std::optional<double> Mash::tunTemp_c                () const { return this->m_tunTemp_c                ; }
std::optional<double> Mash::spargeTemp_c             () const { return this->m_spargeTemp_c             ; }
std::optional<double> Mash::ph                       () const { return this->m_ph                       ; }
std::optional<double> Mash::mashTunWeight_kg         () const { return this->m_mashTunWeight_kg         ; }
std::optional<double> Mash::mashTunSpecificHeat_calGC() const { return this->m_mashTunSpecificHeat_calGC; }
bool                  Mash::equipAdjust              () const { return this->m_equipAdjust              ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Mash::setGrainTemp_c              (double                const   val) { SET_AND_NOTIFY(PropertyNames::Mash::grainTemp_c              , this->m_grainTemp_c              , val                                              ); return; }
void Mash::setNotes                    (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Mash::notes                    , this->m_notes                    , val                                              ); return; }
void Mash::setTunTemp_c                (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::tunTemp_c                , this->m_tunTemp_c                , val                                              ); return; }
void Mash::setSpargeTemp_c             (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::spargeTemp_c             , this->m_spargeTemp_c             , val                                              ); return; }
void Mash::setPh                       (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::ph                       , this->m_ph                       , this->enforceMinAndMax(val, "pH", 0.0, 14.0, 7.0)); return; }
void Mash::setTunWeight_kg             (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::mashTunWeight_kg         , this->m_mashTunWeight_kg         , this->enforceMin(val, "tun weight")              ); return; }
void Mash::setMashTunSpecificHeat_calGC(std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::mashTunSpecificHeat_calGC, this->m_mashTunSpecificHeat_calGC, this->enforceMin(val, "specific heat")           ); return; }
void Mash::setEquipAdjust              (bool                  const   val) { SET_AND_NOTIFY(PropertyNames::Mash::equipAdjust              , this->m_equipAdjust              , val                                              ); return; }

// === other methods ===
double Mash::totalMashWater_l() const {
   double waterAdded_l = 0.0;

   for (auto step : this-> mashSteps()) {
      if (step->isInfusion()) {
         waterAdded_l += step->amount_l();
      }
   }

   return waterAdded_l;
}

double Mash::totalInfusionAmount_l() const {
   double waterAdded_l = 0.0;

   for (auto step :  this->mashSteps()) {
      if (step->isInfusion() && !step->isSparge() ) {
         waterAdded_l += step->amount_l();
      }
   }

   return waterAdded_l;
}

double Mash::totalSpargeAmount_l() const {
   double waterAdded_l = 0.0;

   for (auto step : this->mashSteps()) {
      if (step->isSparge()) {
         waterAdded_l += step->amount_l();
      }
   }

   return waterAdded_l;
}

double Mash::totalTime_mins() const {
   double totalTime = 0.0;
   for (auto step : this->mashSteps()) {
      totalTime += step->stepTime_mins().value_or(0.0);
   }
   return totalTime;
}

bool Mash::hasSparge() const {
   for (auto step : this->mashSteps()) {
      if (step->isSparge()) {
         return true;
      }
   }
   return false;
}

void Mash::acceptStepChange(QMetaProperty prop, QVariant val) {
   // TBD I don't think anything listens for changes to totalMashWater_l or totalTime
   this->doAcceptStepChange(this->sender(), prop, val, {&PropertyNames::Mash::totalMashWater_l,
                                                        &PropertyNames::Mash::totalTime_mins  });
   return;
}

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Mash)

// Insert boiler-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Mash, mash)
