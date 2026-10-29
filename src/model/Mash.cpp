/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Mash.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "model/Folder.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

#ifdef MANUALLY_INCLUDE_MOC
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_Mash.cpp"
#endif

QString Mash::localisedName() { return tr("Mash"); }
QString Mash::localisedName_equipAdjust          () { return tr("Adjust For Equipment"     ); }
QString Mash::localisedName_grainTemp_c          () { return tr("Initial Grain Temperature"); }
QString Mash::localisedName_tunSpecificHeat_calGC() { return tr("Mash Tun Specific Heat"   ); }
QString Mash::localisedName_tunWeight_kg         () { return tr("Mash Tun Weight"          ); }
QString Mash::localisedName_notes                () { return tr("Notes"                    ); }
QString Mash::localisedName_ph                   () { return tr("pH"                       ); }
QString Mash::localisedName_spargeTemp_c         () { return tr("Sparge Temperature"       ); }
QString Mash::localisedName_totalMashWater_l     () { return tr("Mash Water"               ); } // Shorter than "Total Mash Water"
QString Mash::localisedName_totalTime_mins       () { return tr("Total Time"               ); }
QString Mash::localisedName_initialTunTemp_c     () { return tr("Tun Temperature"          ); }

bool Mash::compareWith(NamedEntity const & other, QList<BtStringConst const *> * propertiesThatDiffer) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Mash const & rhs = static_cast<Mash const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_PROPERTY_COMPARE(this, rhs, m_grainTemp_c          , PropertyNames::Mash::grainTemp_c          , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_initialTunTemp_c     , PropertyNames::Mash::initialTunTemp_c     , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_spargeTemp_c         , PropertyNames::Mash::spargeTemp_c         , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_ph                   , PropertyNames::Mash::ph                   , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_tunWeight_kg         , PropertyNames::Mash::tunWeight_kg         , propertiesThatDiffer) &&
      AUTO_PROPERTY_COMPARE(this, rhs, m_tunSpecificHeat_calGC, PropertyNames::Mash::tunSpecificHeat_calGC, propertiesThatDiffer) &&
      // Parent classes have to be equal too
      this->FolderPropertyBase<Mash>             ::doCompareWith(rhs, propertiesThatDiffer) &&
      this->StepOwnerBase<Mash, MashStep>::doCompareWith(rhs, propertiesThatDiffer)
   );
}

ObjectStore & Mash::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Mash>::getInstance();
}

TypeLookup const Mash::typeLookup {
   "Mash",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, equipAdjust          , m_equipAdjust          , BOOL_INFO(tr("No"), tr("Yes"))            ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, grainTemp_c          , m_grainTemp_c          , Measurement::PhysicalQuantity::Temperature, DisplayInfo::Precision{1}),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, notes                , m_notes                ,           NonPhysicalQuantity::String     ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, ph                   , m_ph                   , Measurement::PhysicalQuantity::Acidity    ),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, spargeTemp_c         , m_spargeTemp_c         , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, tunSpecificHeat_calGC, m_tunSpecificHeat_calGC, Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, initialTunTemp_c     , m_initialTunTemp_c     , Measurement::PhysicalQuantity::Temperature),
      PROPERTY_TYPE_LOOKUP_ENTRY(Mash, tunWeight_kg         , m_tunWeight_kg         , Measurement::PhysicalQuantity::Mass       ),
      PROPERTY_TYPE_LOOKUP_NO_MV(Mash, totalMashWater_l     , totalMashWater_l       , Measurement::PhysicalQuantity::Volume     , DisplayInfo::Precision{1}), // Calculated, not in DB
      PROPERTY_TYPE_LOOKUP_NO_MV(Mash, totalTime_mins       , totalTime_mins         , Measurement::PhysicalQuantity::Time       , DisplayInfo::Precision{0}), // Calculated, not in DB
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderPropertyBase<Mash>::typeLookup),
    std::addressof(StepOwnerBase<Mash, MashStep>::typeLookup)}
};
static_assert(std::is_base_of<FolderPropertyBase<Mash>, Mash>::value);

//==================================================== CONSTRUCTORS ====================================================

Mash::Mash(QString name) :
   NamedEntity{name},
   FolderPropertyBase<Mash>{},
   StepOwnerBase<Mash, MashStep>{},
   m_grainTemp_c          {0.0 },
   m_notes                {""  },
   m_initialTunTemp_c     {std::nullopt},
   m_spargeTemp_c         {0.0 },
   m_ph                   {0.0 },
   // Not set means use the figures from the equipment -- see Recipe::mashTunHeatCapacity_kcalC
   m_tunWeight_kg         {std::nullopt},
   m_tunSpecificHeat_calGC{std::nullopt},
   m_equipAdjust          {true} {

   CONSTRUCTOR_END
   return;
}

Mash::Mash(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity                {namedParameterBundle},
   FolderPropertyBase<Mash>{namedParameterBundle},
   StepOwnerBase<Mash, MashStep>{},
   SET_REGULAR_FROM_NPB(m_grainTemp_c          , namedParameterBundle, PropertyNames::Mash::grainTemp_c          ),
   SET_REGULAR_FROM_NPB(m_notes                , namedParameterBundle, PropertyNames::Mash::notes                , ""),
   SET_REGULAR_FROM_NPB(m_initialTunTemp_c     , namedParameterBundle, PropertyNames::Mash::initialTunTemp_c     ),
   SET_REGULAR_FROM_NPB(m_spargeTemp_c         , namedParameterBundle, PropertyNames::Mash::spargeTemp_c         ),
   SET_REGULAR_FROM_NPB(m_ph                   , namedParameterBundle, PropertyNames::Mash::ph                   ),
   SET_REGULAR_FROM_NPB(m_tunWeight_kg         , namedParameterBundle, PropertyNames::Mash::tunWeight_kg         ),
   SET_REGULAR_FROM_NPB(m_tunSpecificHeat_calGC, namedParameterBundle, PropertyNames::Mash::tunSpecificHeat_calGC),
   SET_REGULAR_FROM_NPB(m_equipAdjust          , namedParameterBundle, PropertyNames::Mash::equipAdjust          ) {

   CONSTRUCTOR_END
   return;
}

Mash::Mash(Mash const & other) :
   NamedEntity{other},
   FolderPropertyBase<Mash>{other},
   StepOwnerBase<Mash, MashStep>{other},
   m_grainTemp_c          {other.m_grainTemp_c          },
   m_notes                {other.m_notes                },
   m_initialTunTemp_c     {other.m_initialTunTemp_c     },
   m_spargeTemp_c         {other.m_spargeTemp_c         },
   m_ph                   {other.m_ph                   },
   m_tunWeight_kg         {other.m_tunWeight_kg         },
   m_tunSpecificHeat_calGC{other.m_tunSpecificHeat_calGC},
   m_equipAdjust          {other.m_equipAdjust          } {

   CONSTRUCTOR_END
   return;
}

Mash::~Mash() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================
double                Mash::grainTemp_c          () const { return this->m_grainTemp_c          ; }
QString               Mash::notes                () const { return this->m_notes                ; }
std::optional<double> Mash::initialTunTemp_c     () const { return this->m_initialTunTemp_c     ; }
std::optional<double> Mash::spargeTemp_c         () const { return this->m_spargeTemp_c         ; }
std::optional<double> Mash::ph                   () const { return this->m_ph                   ; }
std::optional<double> Mash::tunWeight_kg         () const { return this->m_tunWeight_kg         ; }
std::optional<double> Mash::tunSpecificHeat_calGC() const { return this->m_tunSpecificHeat_calGC; }
bool                  Mash::equipAdjust          () const { return this->m_equipAdjust          ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================
void Mash::setGrainTemp_c          (double                const   val) { SET_AND_NOTIFY(PropertyNames::Mash::grainTemp_c          , this->m_grainTemp_c          , val                                              ); return; }
void Mash::setNotes                (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Mash::notes                , this->m_notes                , val                                              ); return; }
void Mash::setInitialTunTemp_c     (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::initialTunTemp_c     , this->m_initialTunTemp_c     , val                                              ); return; }
void Mash::setSpargeTemp_c         (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::spargeTemp_c         , this->m_spargeTemp_c         , val                                              ); return; }
void Mash::setPh                   (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::ph                   , this->m_ph                   , this->enforceMinAndMax(val, "pH", 0.0, 14.0, 7.0)); return; }
void Mash::setTunWeight_kg         (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::tunWeight_kg         , this->m_tunWeight_kg         , this->enforceMin(val, "tun weight")              ); return; }
void Mash::setTunSpecificHeat_calGC(std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Mash::tunSpecificHeat_calGC, this->m_tunSpecificHeat_calGC, this->enforceMin(val, "specific heat")           ); return; }
void Mash::setEquipAdjust          (bool                  const   val) { SET_AND_NOTIFY(PropertyNames::Mash::equipAdjust          , this->m_equipAdjust          , val                                              ); return; }

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

   for (auto const & step :  this->mashSteps()) {
      if (step->isInfusion() && !step->isSparge() ) {
         waterAdded_l += step->amount_l();
      }
   }

   return waterAdded_l;
}

double Mash::totalSpargeAmount_l() const {
   double waterAdded_l = 0.0;

   for (auto const & step : this->mashSteps()) {
      if (step->isSparge()) {
         waterAdded_l += step->amount_l();
      }
   }

   return waterAdded_l;
}

double Mash::totalTime_mins() const {
   double totalTime = 0.0;
   for (auto const & step : this->mashSteps()) {
      totalTime += step->stepTime_mins();
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

void Mash::acceptSetMemberChange(QMetaProperty prop, QVariant val) {
   this->doAcceptSetMemberChange(this->sender(), prop, val);
   return;
}

// This class supports NamedEntity::numRecipesUsedIn
IMPLEMENT_NUM_RECIPES_USED_IN(Mash)

// Boilerplate code for FolderPropertyBase
FOLDER_BASE_COMMON_CODE(Mash)

// Insert boiler-plate wrapper functions that call down to StepOwnerBase
STEP_OWNER_COMMON_CODE(Mash, mash)