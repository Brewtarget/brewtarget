/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * model/Equipment.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
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
#include "model/Equipment.h"

#include "database/ObjectStoreWrapper.h"
#include "HeatCalculations.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "utils/AutoCompare.h"

QString Equipment::localisedName() { return tr("Equipment"); }

bool Equipment::isEqualTo(NamedEntity const & other) const {
   // Base class (NamedEntity) will have ensured this cast is valid
   Equipment const & rhs = static_cast<Equipment const &>(other);
   // Base class will already have ensured names are equal
   return (
      AUTO_LOG_COMPARE(this, rhs, m_kettleBoilSize_l          ) &&
      AUTO_LOG_COMPARE(this, rhs, m_fermenterBatchSize_l      ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunVolume_l           ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunWeight_kg          ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunSpecificHeat_calGC ) &&
      AUTO_LOG_COMPARE(this, rhs, m_topUpWater_l              ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kettleTrubChillerLoss_l   ) &&
      AUTO_LOG_COMPARE(this, rhs, m_evapRate_pctHr            ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kettleEvaporationPerHour_l) &&
      AUTO_LOG_COMPARE(this, rhs, m_boilTime_min              ) &&
      AUTO_LOG_COMPARE(this, rhs, m_calcBoilVolume            ) &&
      AUTO_LOG_COMPARE(this, rhs, m_lauterTunDeadspaceLoss_l  ) &&
      AUTO_LOG_COMPARE(this, rhs, m_topUpKettle_l             ) &&
      AUTO_LOG_COMPARE(this, rhs, m_hopUtilization_pct        ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kettleNotes               ) &&
      AUTO_LOG_COMPARE(this, rhs, m_mashTunGrainAbsorption_LKg) &&
      AUTO_LOG_COMPARE(this, rhs, m_boilingPoint_c            ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kettleInternalDiameter_cm ) &&
      AUTO_LOG_COMPARE(this, rhs, m_kettleOpeningDiameter_cm  )
   );
}

ObjectStore & Equipment::getObjectStoreTypedInstance() const {
   return ObjectStoreTyped<Equipment>::getInstance();
}

TypeLookup const Equipment::typeLookup {
   "Equipment",
   {
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleBoilSize_l           , Equipment::m_kettleBoilSize_l           , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::fermenterBatchSize_l       , Equipment::m_fermenterBatchSize_l       , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunVolume_l            , Equipment::m_mashTunVolume_l            , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunWeight_kg           , Equipment::m_mashTunWeight_kg           , Measurement::PhysicalQuantity::Mass                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunSpecificHeat_calGC  , Equipment::m_mashTunSpecificHeat_calGC  , Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::topUpWater_l               , Equipment::m_topUpWater_l               , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleTrubChillerLoss_l    , Equipment::m_kettleTrubChillerLoss_l    , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::evapRate_pctHr             , Equipment::m_evapRate_pctHr             ,           NonPhysicalQuantity::Percentage          ), // The "per hour" bit is fixed, so we simplify
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleEvaporationPerHour_l , Equipment::m_kettleEvaporationPerHour_l , Measurement::PhysicalQuantity::Volume              ), // The "per hour" bit is fixed, so we simplify
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::boilTime_min               , Equipment::m_boilTime_min               , Measurement::PhysicalQuantity::Time                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::calcBoilVolume             , Equipment::m_calcBoilVolume             ,           NonPhysicalQuantity::Bool                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunDeadspaceLoss_l   , Equipment::m_lauterTunDeadspaceLoss_l   , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::topUpKettle_l              , Equipment::m_topUpKettle_l              , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hopUtilization_pct         , Equipment::m_hopUtilization_pct         ,           NonPhysicalQuantity::Percentage          ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleNotes                , Equipment::m_kettleNotes                ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunGrainAbsorption_LKg , Equipment::m_mashTunGrainAbsorption_LKg , Measurement::PhysicalQuantity::SpecificVolume      ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::boilingPoint_c             , Equipment::m_boilingPoint_c             , Measurement::PhysicalQuantity::Temperature         ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleInternalDiameter_cm  , Equipment::m_kettleInternalDiameter_cm  , Measurement::PhysicalQuantity::Length              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleOpeningDiameter_cm   , Equipment::m_kettleOpeningDiameter_cm   , Measurement::PhysicalQuantity::Length              ),
      // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltType                    , Equipment::m_hltType                    ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunType                , Equipment::m_mashTunType                ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunType              , Equipment::m_lauterTunType              ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleType                 , Equipment::m_kettleType                 ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::fermenterType              , Equipment::m_fermenterType              ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::agingVesselType            , Equipment::m_agingVesselType            ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::packagingVesselType        , Equipment::m_packagingVesselType        ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltVolume_l                , Equipment::m_hltVolume_l                , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunVolume_l          , Equipment::m_lauterTunVolume_l          , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::agingVesselVolume_l        , Equipment::m_agingVesselVolume_l        , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::packagingVesselVolume_l    , Equipment::m_packagingVesselVolume_l    , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltLoss_l                  , Equipment::m_hltLoss_l                  , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunLoss_l              , Equipment::m_mashTunLoss_l              , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::fermenterLoss_l            , Equipment::m_fermenterLoss_l            , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::agingVesselLoss_l          , Equipment::m_agingVesselLoss_l          , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::packagingVesselLoss_l      , Equipment::m_packagingVesselLoss_l      , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleOutflowPerMinute_l   , Equipment::m_kettleOutflowPerMinute_l   , Measurement::PhysicalQuantity::Volume              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltWeight_kg               , Equipment::m_hltWeight_kg               , Measurement::PhysicalQuantity::Mass                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunWeight_kg         , Equipment::m_lauterTunWeight_kg         , Measurement::PhysicalQuantity::Mass                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleWeight_kg            , Equipment::m_kettleWeight_kg            , Measurement::PhysicalQuantity::Mass                ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltSpecificHeat_calGC      , Equipment::m_hltSpecificHeat_calGC      , Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunSpecificHeat_calGC, Equipment::m_lauterTunSpecificHeat_calGC, Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::kettleSpecificHeat_calGC   , Equipment::m_kettleSpecificHeat_calGC   , Measurement::PhysicalQuantity::SpecificHeatCapacity),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::hltNotes                   , Equipment::m_hltNotes                   ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::mashTunNotes               , Equipment::m_mashTunNotes               ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::lauterTunNotes             , Equipment::m_lauterTunNotes             ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::fermenterNotes             , Equipment::m_fermenterNotes             ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::agingVesselNotes           , Equipment::m_agingVesselNotes           ,           NonPhysicalQuantity::String              ),
      PROPERTY_TYPE_LOOKUP_ENTRY(PropertyNames::Equipment::packagingVesselNotes       , Equipment::m_packagingVesselNotes       ,           NonPhysicalQuantity::String              ),
   },
   // Parent classes lookup
   {&NamedEntity::typeLookup,
    std::addressof(FolderBase<Equipment>::typeLookup)}
};
static_assert(std::is_base_of<FolderBase<Equipment>, Equipment>::value);

//=============================CONSTRUCTORS=====================================
Equipment::Equipment(QString name) :
   NamedEntity        {name, true},
   FolderBase<Equipment>{},
   m_kettleBoilSize_l           {22.927      },
   m_fermenterBatchSize_l       {18.927      },
   m_mashTunVolume_l            {0.0         },
   m_mashTunWeight_kg           {std::nullopt},
   m_mashTunSpecificHeat_calGC  {std::nullopt},
   m_topUpWater_l               {std::nullopt},
   m_kettleTrubChillerLoss_l    {1.0         },
   m_evapRate_pctHr             {0.0         },
   m_kettleEvaporationPerHour_l {std::nullopt}, // Previously 4.0
   m_boilTime_min               {std::nullopt}, // Previously 60.0
   m_calcBoilVolume             {true        },
   m_lauterTunDeadspaceLoss_l   {0.0         },
   m_topUpKettle_l              {std::nullopt},
   m_hopUtilization_pct         {std::nullopt}, // Previously 100.0
   m_kettleNotes                {""          },
   m_mashTunGrainAbsorption_LKg {std::nullopt}, // Previously 1.086
   m_boilingPoint_c             {100.0       },
   m_kettleInternalDiameter_cm  {std::nullopt},
   m_kettleOpeningDiameter_cm   {std::nullopt},
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_hltType                    {""          },
   m_mashTunType                {""          },
   m_lauterTunType              {""          },
   m_kettleType                 {""          },
   m_fermenterType              {""          },
   m_agingVesselType            {""          },
   m_packagingVesselType        {""          },
   m_hltVolume_l                {0.0         },
   m_lauterTunVolume_l          {0.0         },
   m_agingVesselVolume_l        {0.0         },
   m_packagingVesselVolume_l    {0.0         },
   m_hltLoss_l                  {0.0         },
   m_mashTunLoss_l              {0.0         },
   m_fermenterLoss_l            {0.0         },
   m_agingVesselLoss_l          {0.0         },
   m_packagingVesselLoss_l      {0.0         },
   m_kettleOutflowPerMinute_l   {std::nullopt},
   m_hltWeight_kg               {std::nullopt},
   m_lauterTunWeight_kg         {std::nullopt},
   m_kettleWeight_kg            {std::nullopt},
   m_hltSpecificHeat_calGC      {std::nullopt},
   m_lauterTunSpecificHeat_calGC{std::nullopt},
   m_kettleSpecificHeat_calGC   {std::nullopt},
   m_hltNotes                   {""          },
   m_mashTunNotes               {""          },
   m_lauterTunNotes             {""          },
   m_fermenterNotes             {""          },
   m_agingVesselNotes           {""          },
   m_packagingVesselNotes       {""          } {

   CONSTRUCTOR_END
   return;
}

// The default values below are set for the following fields that are not part of BeerXML 1.0 standard and so will
// not be present in BeerXML files (unless we wrote them) but will be present in the database:
//    - kettleEvaporationPerHour_l
//    - mashTunGrainAbsorption_LKg
//    - boilingPoint_c
//    - kettleInternalDiameter_cm
//    - kettleOpeningDiameter_cm
//
Equipment::Equipment(NamedParameterBundle const & namedParameterBundle) :
   NamedEntity{namedParameterBundle},
   FolderBase<Equipment>{namedParameterBundle},
   SET_REGULAR_FROM_NPB (m_kettleBoilSize_l           , namedParameterBundle, PropertyNames::Equipment::kettleBoilSize_l           ),
   SET_REGULAR_FROM_NPB (m_fermenterBatchSize_l       , namedParameterBundle, PropertyNames::Equipment::fermenterBatchSize_l       ),
   SET_REGULAR_FROM_NPB (m_mashTunVolume_l            , namedParameterBundle, PropertyNames::Equipment::mashTunVolume_l            ),
   SET_REGULAR_FROM_NPB (m_mashTunWeight_kg           , namedParameterBundle, PropertyNames::Equipment::mashTunWeight_kg           , std::nullopt),
   SET_REGULAR_FROM_NPB (m_mashTunSpecificHeat_calGC  , namedParameterBundle, PropertyNames::Equipment::mashTunSpecificHeat_calGC  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_topUpWater_l               , namedParameterBundle, PropertyNames::Equipment::topUpWater_l               , std::nullopt),
   SET_REGULAR_FROM_NPB (m_kettleTrubChillerLoss_l    , namedParameterBundle, PropertyNames::Equipment::kettleTrubChillerLoss_l    ),
   SET_REGULAR_FROM_NPB (m_evapRate_pctHr             , namedParameterBundle, PropertyNames::Equipment::evapRate_pctHr             ),
   SET_REGULAR_FROM_NPB (m_kettleEvaporationPerHour_l , namedParameterBundle, PropertyNames::Equipment::kettleEvaporationPerHour_l , std::nullopt),
   SET_REGULAR_FROM_NPB (m_boilTime_min               , namedParameterBundle, PropertyNames::Equipment::boilTime_min               , std::nullopt),
   SET_REGULAR_FROM_NPB (m_calcBoilVolume             , namedParameterBundle, PropertyNames::Equipment::calcBoilVolume             ),
   SET_REGULAR_FROM_NPB (m_lauterTunDeadspaceLoss_l   , namedParameterBundle, PropertyNames::Equipment::lauterTunDeadspaceLoss_l   ),
   SET_REGULAR_FROM_NPB (m_topUpKettle_l              , namedParameterBundle, PropertyNames::Equipment::topUpKettle_l              , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hopUtilization_pct         , namedParameterBundle, PropertyNames::Equipment::hopUtilization_pct         , std::nullopt),
   SET_REGULAR_FROM_NPB (m_kettleNotes                , namedParameterBundle, PropertyNames::Equipment::kettleNotes                , ""),
   SET_REGULAR_FROM_NPB (m_mashTunGrainAbsorption_LKg , namedParameterBundle, PropertyNames::Equipment::mashTunGrainAbsorption_LKg , std::nullopt),
   SET_REGULAR_FROM_NPB (m_boilingPoint_c             , namedParameterBundle, PropertyNames::Equipment::boilingPoint_c             , 100.0),
   SET_REGULAR_FROM_NPB (m_kettleInternalDiameter_cm  , namedParameterBundle, PropertyNames::Equipment::kettleInternalDiameter_cm  , std::nullopt),
   SET_REGULAR_FROM_NPB (m_kettleOpeningDiameter_cm   , namedParameterBundle, PropertyNames::Equipment::kettleOpeningDiameter_cm   , std::nullopt),
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   SET_REGULAR_FROM_NPB (m_hltType                    , namedParameterBundle, PropertyNames::Equipment::hltType                    , ""          ),
   SET_REGULAR_FROM_NPB (m_mashTunType                , namedParameterBundle, PropertyNames::Equipment::mashTunType                , ""          ),
   SET_REGULAR_FROM_NPB (m_lauterTunType              , namedParameterBundle, PropertyNames::Equipment::lauterTunType              , ""          ),
   SET_REGULAR_FROM_NPB (m_kettleType                 , namedParameterBundle, PropertyNames::Equipment::kettleType                 , ""          ),
   SET_REGULAR_FROM_NPB (m_fermenterType              , namedParameterBundle, PropertyNames::Equipment::fermenterType              , ""          ),
   SET_REGULAR_FROM_NPB (m_agingVesselType            , namedParameterBundle, PropertyNames::Equipment::agingVesselType            , ""          ),
   SET_REGULAR_FROM_NPB (m_packagingVesselType        , namedParameterBundle, PropertyNames::Equipment::packagingVesselType        , ""          ),
   SET_REGULAR_FROM_NPB (m_hltVolume_l                , namedParameterBundle, PropertyNames::Equipment::hltVolume_l                , 0.0         ),
   SET_REGULAR_FROM_NPB (m_lauterTunVolume_l          , namedParameterBundle, PropertyNames::Equipment::lauterTunVolume_l          , 0.0         ),
   SET_REGULAR_FROM_NPB (m_agingVesselVolume_l        , namedParameterBundle, PropertyNames::Equipment::agingVesselVolume_l        , 0.0         ),
   SET_REGULAR_FROM_NPB (m_packagingVesselVolume_l    , namedParameterBundle, PropertyNames::Equipment::packagingVesselVolume_l    , 0.0         ),
   SET_REGULAR_FROM_NPB (m_hltLoss_l                  , namedParameterBundle, PropertyNames::Equipment::hltLoss_l                  , 0.0         ),
   SET_REGULAR_FROM_NPB (m_mashTunLoss_l              , namedParameterBundle, PropertyNames::Equipment::mashTunLoss_l              , 0.0         ),
   SET_REGULAR_FROM_NPB (m_fermenterLoss_l            , namedParameterBundle, PropertyNames::Equipment::fermenterLoss_l            , 0.0         ),
   SET_REGULAR_FROM_NPB (m_agingVesselLoss_l          , namedParameterBundle, PropertyNames::Equipment::agingVesselLoss_l          , 0.0         ),
   SET_REGULAR_FROM_NPB (m_packagingVesselLoss_l      , namedParameterBundle, PropertyNames::Equipment::packagingVesselLoss_l      , 0.0         ),
   SET_REGULAR_FROM_NPB (m_kettleOutflowPerMinute_l   , namedParameterBundle, PropertyNames::Equipment::kettleOutflowPerMinute_l   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hltWeight_kg               , namedParameterBundle, PropertyNames::Equipment::hltWeight_kg               , std::nullopt),
   SET_REGULAR_FROM_NPB (m_lauterTunWeight_kg         , namedParameterBundle, PropertyNames::Equipment::lauterTunWeight_kg         , std::nullopt),
   SET_REGULAR_FROM_NPB (m_kettleWeight_kg            , namedParameterBundle, PropertyNames::Equipment::kettleWeight_kg            , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hltSpecificHeat_calGC      , namedParameterBundle, PropertyNames::Equipment::hltSpecificHeat_calGC      , std::nullopt),
   SET_REGULAR_FROM_NPB (m_lauterTunSpecificHeat_calGC, namedParameterBundle, PropertyNames::Equipment::lauterTunSpecificHeat_calGC, std::nullopt),
   SET_REGULAR_FROM_NPB (m_kettleSpecificHeat_calGC   , namedParameterBundle, PropertyNames::Equipment::kettleSpecificHeat_calGC   , std::nullopt),
   SET_REGULAR_FROM_NPB (m_hltNotes                   , namedParameterBundle, PropertyNames::Equipment::hltNotes                   , ""          ),
   SET_REGULAR_FROM_NPB (m_mashTunNotes               , namedParameterBundle, PropertyNames::Equipment::mashTunNotes               , ""          ),
   SET_REGULAR_FROM_NPB (m_lauterTunNotes             , namedParameterBundle, PropertyNames::Equipment::lauterTunNotes             , ""          ),
   SET_REGULAR_FROM_NPB (m_fermenterNotes             , namedParameterBundle, PropertyNames::Equipment::fermenterNotes             , ""          ),
   SET_REGULAR_FROM_NPB (m_agingVesselNotes           , namedParameterBundle, PropertyNames::Equipment::agingVesselNotes           , ""          ),
   SET_REGULAR_FROM_NPB (m_packagingVesselNotes       , namedParameterBundle, PropertyNames::Equipment::packagingVesselNotes       , ""          ) {

   CONSTRUCTOR_END
   return;
}

Equipment::Equipment(Equipment const & other) :
   NamedEntity          {other},
   FolderBase<Equipment>{other},
   m_kettleBoilSize_l           {other.m_kettleBoilSize_l           },
   m_fermenterBatchSize_l       {other.m_fermenterBatchSize_l       },
   m_mashTunVolume_l            {other.m_mashTunVolume_l            },
   m_mashTunWeight_kg           {other.m_mashTunWeight_kg           },
   m_mashTunSpecificHeat_calGC  {other.m_mashTunSpecificHeat_calGC  },
   m_topUpWater_l               {other.m_topUpWater_l               },
   m_kettleTrubChillerLoss_l    {other.m_kettleTrubChillerLoss_l    },
   m_evapRate_pctHr             {other.m_evapRate_pctHr             },
   m_kettleEvaporationPerHour_l {other.m_kettleEvaporationPerHour_l },
   m_boilTime_min               {other.m_boilTime_min               },
   m_calcBoilVolume             {other.m_calcBoilVolume             },
   m_lauterTunDeadspaceLoss_l   {other.m_lauterTunDeadspaceLoss_l   },
   m_topUpKettle_l              {other.m_topUpKettle_l              },
   m_hopUtilization_pct         {other.m_hopUtilization_pct         },
   m_kettleNotes                {other.m_kettleNotes                },
   m_mashTunGrainAbsorption_LKg {other.m_mashTunGrainAbsorption_LKg },
   m_boilingPoint_c             {other.m_boilingPoint_c             },
   m_kettleInternalDiameter_cm  {other.m_kettleInternalDiameter_cm  },
   m_kettleOpeningDiameter_cm   {other.m_kettleOpeningDiameter_cm   },
   // ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
   m_hltType                    {other.m_hltType                    },
   m_mashTunType                {other.m_mashTunType                },
   m_lauterTunType              {other.m_lauterTunType              },
   m_kettleType                 {other.m_kettleType                 },
   m_fermenterType              {other.m_fermenterType              },
   m_agingVesselType            {other.m_agingVesselType            },
   m_packagingVesselType        {other.m_packagingVesselType        },
   m_hltVolume_l                {other.m_hltVolume_l                },
   m_lauterTunVolume_l          {other.m_lauterTunVolume_l          },
   m_agingVesselVolume_l        {other.m_agingVesselVolume_l        },
   m_packagingVesselVolume_l    {other.m_packagingVesselVolume_l    },
   m_hltLoss_l                  {other.m_hltLoss_l                  },
   m_mashTunLoss_l              {other.m_mashTunLoss_l              },
   m_fermenterLoss_l            {other.m_fermenterLoss_l            },
   m_agingVesselLoss_l          {other.m_agingVesselLoss_l          },
   m_packagingVesselLoss_l      {other.m_packagingVesselLoss_l      },
   m_kettleOutflowPerMinute_l   {other.m_kettleOutflowPerMinute_l   },
   m_hltWeight_kg               {other.m_hltWeight_kg               },
   m_lauterTunWeight_kg         {other.m_lauterTunWeight_kg         },
   m_kettleWeight_kg            {other.m_kettleWeight_kg            },
   m_hltSpecificHeat_calGC      {other.m_hltSpecificHeat_calGC      },
   m_lauterTunSpecificHeat_calGC{other.m_lauterTunSpecificHeat_calGC},
   m_kettleSpecificHeat_calGC   {other.m_kettleSpecificHeat_calGC   },
   m_hltNotes                   {other.m_hltNotes                   },
   m_mashTunNotes               {other.m_mashTunNotes               },
   m_lauterTunNotes             {other.m_lauterTunNotes             },
   m_fermenterNotes             {other.m_fermenterNotes             },
   m_agingVesselNotes           {other.m_agingVesselNotes           },
   m_packagingVesselNotes       {other.m_packagingVesselNotes       } {

   CONSTRUCTOR_END
   return;
}

Equipment::~Equipment() = default;

//============================================= "GETTER" MEMBER FUNCTIONS ==============================================

double                Equipment::kettleBoilSize_l           () const { return m_kettleBoilSize_l           ; }
double                Equipment::fermenterBatchSize_l       () const { return m_fermenterBatchSize_l       ; }
double                Equipment::mashTunVolume_l            () const { return m_mashTunVolume_l            ; }
std::optional<double> Equipment::mashTunWeight_kg           () const { return m_mashTunWeight_kg           ; }
std::optional<double> Equipment::mashTunSpecificHeat_calGC  () const { return m_mashTunSpecificHeat_calGC  ; }
std::optional<double> Equipment::topUpWater_l               () const { return m_topUpWater_l               ; }
double                Equipment::kettleTrubChillerLoss_l    () const { return m_kettleTrubChillerLoss_l    ; }
std::optional<double> Equipment::evapRate_pctHr             () const { return m_evapRate_pctHr             ; }
std::optional<double> Equipment::kettleEvaporationPerHour_l () const { return m_kettleEvaporationPerHour_l ; }
std::optional<double> Equipment::boilTime_min               () const { return m_boilTime_min               ; }
bool                  Equipment::calcBoilVolume             () const { return m_calcBoilVolume             ; }
double                Equipment::lauterTunDeadspaceLoss_l   () const { return m_lauterTunDeadspaceLoss_l   ; }
std::optional<double> Equipment::topUpKettle_l              () const { return m_topUpKettle_l              ; }
std::optional<double> Equipment::hopUtilization_pct         () const { return m_hopUtilization_pct         ; }
QString               Equipment::kettleNotes                () const { return m_kettleNotes                ; }
std::optional<double> Equipment::mashTunGrainAbsorption_LKg () const { return m_mashTunGrainAbsorption_LKg ; }
double                Equipment::boilingPoint_c             () const { return m_boilingPoint_c             ; }
std::optional<double> Equipment::kettleInternalDiameter_cm  () const { return m_kettleInternalDiameter_cm  ; }
std::optional<double> Equipment::kettleOpeningDiameter_cm   () const { return m_kettleOpeningDiameter_cm   ; }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
QString               Equipment::hltType                    () const { return m_hltType                    ; }
QString               Equipment::mashTunType                () const { return m_mashTunType                ; }
QString               Equipment::lauterTunType              () const { return m_lauterTunType              ; }
QString               Equipment::kettleType                 () const { return m_kettleType                 ; }
QString               Equipment::fermenterType              () const { return m_fermenterType              ; }
QString               Equipment::agingVesselType            () const { return m_agingVesselType            ; }
QString               Equipment::packagingVesselType        () const { return m_packagingVesselType        ; }
double                Equipment::hltVolume_l                () const { return m_hltVolume_l                ; }
double                Equipment::lauterTunVolume_l          () const { return m_lauterTunVolume_l          ; }
double                Equipment::agingVesselVolume_l        () const { return m_agingVesselVolume_l        ; }
double                Equipment::packagingVesselVolume_l    () const { return m_packagingVesselVolume_l    ; }
double                Equipment::hltLoss_l                  () const { return m_hltLoss_l                  ; }
double                Equipment::mashTunLoss_l              () const { return m_mashTunLoss_l              ; }
double                Equipment::fermenterLoss_l            () const { return m_fermenterLoss_l            ; }
double                Equipment::agingVesselLoss_l          () const { return m_agingVesselLoss_l          ; }
double                Equipment::packagingVesselLoss_l      () const { return m_packagingVesselLoss_l      ; }
std::optional<double> Equipment::kettleOutflowPerMinute_l   () const { return m_kettleOutflowPerMinute_l   ; }
std::optional<double> Equipment::hltWeight_kg               () const { return m_hltWeight_kg               ; }
std::optional<double> Equipment::lauterTunWeight_kg         () const { return m_lauterTunWeight_kg         ; }
std::optional<double> Equipment::kettleWeight_kg            () const { return m_kettleWeight_kg            ; }
std::optional<double> Equipment::hltSpecificHeat_calGC      () const { return m_hltSpecificHeat_calGC      ; }
std::optional<double> Equipment::lauterTunSpecificHeat_calGC() const { return m_lauterTunSpecificHeat_calGC; }
std::optional<double> Equipment::kettleSpecificHeat_calGC   () const { return m_kettleSpecificHeat_calGC   ; }
QString               Equipment::hltNotes                   () const { return m_hltNotes                   ; }
QString               Equipment::mashTunNotes               () const { return m_mashTunNotes               ; }
QString               Equipment::lauterTunNotes             () const { return m_lauterTunNotes             ; }
QString               Equipment::fermenterNotes             () const { return m_fermenterNotes             ; }
QString               Equipment::agingVesselNotes           () const { return m_agingVesselNotes           ; }
QString               Equipment::packagingVesselNotes       () const { return m_packagingVesselNotes       ; }

//============================================= "SETTER" MEMBER FUNCTIONS ==============================================

// The logic through here is similar to what's in Hop. Unfortunately, the additional signals don't allow quite the
// compactness.
void Equipment::setKettleBoilSize_l         (double                const val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleBoilSize_l         , this->m_kettleBoilSize_l         , this->enforceMin(val, "boil size"        )); }
void Equipment::setFermenterBatchSize_l     (double                const val) { SET_AND_NOTIFY(PropertyNames::Equipment::fermenterBatchSize_l     , this->m_fermenterBatchSize_l     , this->enforceMin(val, "batch size"       )); if (this->key() > 0) { doCalculations(); } }
void Equipment::setMashTunVolume_l          (double                const val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunVolume_l          , this->m_mashTunVolume_l          , this->enforceMin(val, "tun volume"       )); }
void Equipment::setMashTunWeight_kg         (std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunWeight_kg         , this->m_mashTunWeight_kg         , this->enforceMin(val, "tun weight"       )); }
void Equipment::setMashTunSpecificHeat_calGC(std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunSpecificHeat_calGC, this->m_mashTunSpecificHeat_calGC, this->enforceMin(val, "tun specific heat")); }
void Equipment::setTopUpWater_l             (std::optional<double> const val) { SET_AND_NOTIFY(PropertyNames::Equipment::topUpWater_l             , this->m_topUpWater_l             , this->enforceMin(val, "top-up water"     )); if (this->key() > 0) { doCalculations(); } }
void Equipment::setKettleTrubChillerLoss_l  (double                const val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleTrubChillerLoss_l  , this->m_kettleTrubChillerLoss_l  , this->enforceMin(val, "trub chiller loss")); if (this->key() > 0) { doCalculations(); } }

void Equipment::setEvapRate_pctHr(std::optional<double> const val) {
   // NOTE: We never use evapRate_pctHr, but we do use kettleEvaporationPerHour_l. So keep them
   //       synced, and implement the former in terms of the latter.
   if (val) {
      this->setKettleEvaporationPerHour_l(*val/100.0 * m_fermenterBatchSize_l);
   } else {
      this->setKettleEvaporationPerHour_l(std::nullopt);
   }
   return;
}

void Equipment::setKettleEvaporationPerHour_l(std::optional<double> const val) {
   // NOTE: We never use evapRate_pctHr, but we maintain here anyway.
   // Because both values are stored in the DB, and because we only want to call prepareForPropertyChange() once, we
   // can't use the setAndNotify() helper function
   this->prepareForPropertyChange(PropertyNames::Equipment::kettleEvaporationPerHour_l);
   this->m_kettleEvaporationPerHour_l = this->enforceMin(val, "evap rate");
   if (this->m_kettleEvaporationPerHour_l) {
      this->m_evapRate_pctHr = *this->m_kettleEvaporationPerHour_l/this->m_fermenterBatchSize_l * 100.0; // We don't use it, but keep it current.
   } else {
      this->m_evapRate_pctHr = std::nullopt;
   }
   this->propagatePropertyChange(PropertyNames::Equipment::kettleEvaporationPerHour_l);
   this->propagatePropertyChange(PropertyNames::Equipment::evapRate_pctHr);

   // Right now, I am claiming this needs to happen regardless of whether we're yet stored in the database.
   // I could be wrong
   doCalculations();
}

void Equipment::setBoilTime_min               (std::optional<double> const   val) { if (SET_AND_NOTIFY(PropertyNames::Equipment::boilTime_min              , this->m_boilTime_min              , this->enforceMin(val, "boil time"))) {       doCalculations();    }    return; }
void Equipment::setCalcBoilVolume             (bool                  const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::calcBoilVolume            , this->m_calcBoilVolume            , val);    if ( val ) {       doCalculations();    } }
void Equipment::setLauterTunDeadspaceLoss_l   (double                const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunDeadspaceLoss_l     , this->m_lauterTunDeadspaceLoss_l         , this->enforceMin(val, "deadspace")); }
void Equipment::setTopUpKettle_l              (std::optional<double> const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::topUpKettle_l             , this->m_topUpKettle_l             , this->enforceMin(val, "top-up kettle")); }
void Equipment::setHopUtilization_pct         (std::optional<double> const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::hopUtilization_pct        , this->m_hopUtilization_pct        , this->enforceMin(val, "hop utilization")); }
void Equipment::setKettleNotes                (QString               const & val) {     SET_AND_NOTIFY(PropertyNames::Equipment::kettleNotes               , this->m_kettleNotes               , val); }
void Equipment::setMashTunGrainAbsorption_LKg (std::optional<double> const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::mashTunGrainAbsorption_LKg, this->m_mashTunGrainAbsorption_LKg, this->enforceMin(val, "absorption")); }
void Equipment::setBoilingPoint_c             (double                const   val) {     SET_AND_NOTIFY(PropertyNames::Equipment::boilingPoint_c            , this->m_boilingPoint_c            , this->enforceMin(val, "boiling point of water")); }
void Equipment::setKettleInternalDiameter_cm  (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleInternalDiameter_cm  , this->m_kettleInternalDiameter_cm  , val); }
void Equipment::setKettleOpeningDiameter_cm   (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleOpeningDiameter_cm   , this->m_kettleOpeningDiameter_cm   , val); }
// ⮜⮜⮜ All below added for BeerJSON support ⮞⮞⮞
void Equipment::setHltType                    (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltType                    , this->m_hltType                    , val); }
void Equipment::setMashTunType                (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunType                , this->m_mashTunType                , val); }
void Equipment::setLauterTunType              (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunType              , this->m_lauterTunType              , val); }
void Equipment::setKettleType                 (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleType                 , this->m_kettleType                 , val); }
void Equipment::setFermenterType              (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::fermenterType              , this->m_fermenterType              , val); }
void Equipment::setAgingVesselType            (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::agingVesselType            , this->m_agingVesselType            , val); }
void Equipment::setPackagingVesselType        (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::packagingVesselType        , this->m_packagingVesselType        , val); }
void Equipment::setHltVolume_l                (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltVolume_l                , this->m_hltVolume_l                , val); }
void Equipment::setLauterTunVolume_l          (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunVolume_l          , this->m_lauterTunVolume_l          , val); }
void Equipment::setAgingVesselVolume_l        (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::agingVesselVolume_l        , this->m_agingVesselVolume_l        , val); }
void Equipment::setPackagingVesselVolume_l    (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::packagingVesselVolume_l    , this->m_packagingVesselVolume_l    , val); }
void Equipment::setHltLoss_l                  (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltLoss_l                  , this->m_hltLoss_l                  , val); }
void Equipment::setMashTunLoss_l              (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunLoss_l              , this->m_mashTunLoss_l              , val); }
void Equipment::setFermenterLoss_l            (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::fermenterLoss_l            , this->m_fermenterLoss_l            , val); }
void Equipment::setAgingVesselLoss_l          (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::agingVesselLoss_l          , this->m_agingVesselLoss_l          , val); }
void Equipment::setPackagingVesselLoss_l      (double                const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::packagingVesselLoss_l      , this->m_packagingVesselLoss_l      , val); }
void Equipment::setKettleOutflowPerMinute_l   (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleOutflowPerMinute_l   , this->m_kettleOutflowPerMinute_l   , val); }
void Equipment::setHltWeight_kg               (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltWeight_kg               , this->m_hltWeight_kg               , val); }
void Equipment::setLauterTunWeight_kg         (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunWeight_kg         , this->m_lauterTunWeight_kg         , val); }
void Equipment::setKettleWeight_kg            (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleWeight_kg            , this->m_kettleWeight_kg            , val); }
void Equipment::setHltSpecificHeat_calGC      (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltSpecificHeat_calGC      , this->m_hltSpecificHeat_calGC      , val); }
void Equipment::setLauterTunSpecificHeat_calGC(std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunSpecificHeat_calGC, this->m_lauterTunSpecificHeat_calGC, val); }
void Equipment::setKettleSpecificHeat_calGC   (std::optional<double> const   val) { SET_AND_NOTIFY(PropertyNames::Equipment::kettleSpecificHeat_calGC   , this->m_kettleSpecificHeat_calGC   , val); }
void Equipment::setHltNotes                   (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::hltNotes                   , this->m_hltNotes                   , val); }
void Equipment::setMashTunNotes               (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::mashTunNotes               , this->m_mashTunNotes               , val); }
void Equipment::setLauterTunNotes             (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::lauterTunNotes             , this->m_lauterTunNotes             , val); }
void Equipment::setFermenterNotes             (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::fermenterNotes             , this->m_fermenterNotes             , val); }
void Equipment::setAgingVesselNotes           (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::agingVesselNotes           , this->m_agingVesselNotes           , val); }
void Equipment::setPackagingVesselNotes       (QString               const & val) { SET_AND_NOTIFY(PropertyNames::Equipment::packagingVesselNotes       , this->m_packagingVesselNotes       , val); }


void Equipment::doCalculations() {
   // Only do the calculation if we're asked to.
   if (!this->calcBoilVolume()) {
      return;
   }

   this->setKettleBoilSize_l(
      this->fermenterBatchSize_l() -
      this->topUpWater_l().value_or(Equipment::default_topUpWater_l) +
      this->kettleTrubChillerLoss_l() +
      (this->boilTime_min().value_or(Equipment::default_boilTime_mins) / 60.0) *
       this->kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l)
   );
   return;
}

double Equipment::getLauteringDeadspaceLoss_l() const {
   return this->m_mashTunLoss_l + (this->m_lauterTunVolume_l > 0 ? this->m_lauterTunDeadspaceLoss_l : 0.0);
}

double Equipment::wortEndOfBoil_l( double kettleWort_l ) const {
   //return kettleWort_l * (1 - (boilTime_min/(double)60) * (evapRate_pctHr/(double)100) );

   return kettleWort_l - (boilTime_min().value_or(Equipment::default_boilTime_mins)/(double)60)*kettleEvaporationPerHour_l().value_or(Equipment::default_kettleEvaporationPerHour_l);
}

// Boilerplate code for FolderBase
FOLDER_BASE_COMMON_CODE(Equipment)
