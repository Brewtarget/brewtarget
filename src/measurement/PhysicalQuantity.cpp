/*
 * measurement/PhysicalQuantity.cpp is part of Brewtarget, and is copyright the following
 * authors 2021-2023:
 * - Matt Young <mfsy@yahoo.com>
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
#include "measurement/PhysicalQuantity.h"

#include <utility>
#include <QDebug>

// Settings we only use in this file.  Strictly we could put these as literals in Measurement::getSettingsName, but
// doing it this way is consistent with how we define other persistent setting name constants.
#define AddSettingName(name) namespace { BtStringConst const name{#name}; }
AddSettingName(unitSystem_acidity             )
AddSettingName(unitSystem_bitterness          )
AddSettingName(unitSystem_carbonation         )
AddSettingName(unitSystem_color               )
AddSettingName(unitSystem_density             )
AddSettingName(unitSystem_diastaticPower      )
AddSettingName(unitSystem_massConcentration   )
AddSettingName(unitSystem_specificHeatCapacity)
AddSettingName(unitSystem_temperature         )
AddSettingName(unitSystem_time                )
AddSettingName(unitSystem_viscosity           )
AddSettingName(unitSystem_volume              )
AddSettingName(unitSystem_volumeConcentration )
AddSettingName(unitSystem_weight              )
#undef AddSettingName

std::array<Measurement::PhysicalQuantity, 14> const Measurement::allPhysicalQuantites{
   Measurement::PhysicalQuantity::Mass                , // 1
   Measurement::PhysicalQuantity::Volume              , // 2
   Measurement::PhysicalQuantity::Time                , // 3
   Measurement::PhysicalQuantity::Temperature         , // 4
   Measurement::PhysicalQuantity::Color               , // 5
   Measurement::PhysicalQuantity::Density             , // 6
   Measurement::PhysicalQuantity::DiastaticPower      , // 7
   Measurement::PhysicalQuantity::Acidity             , // 8
   Measurement::PhysicalQuantity::Bitterness          , // 9
   Measurement::PhysicalQuantity::Carbonation         , // 10
   Measurement::PhysicalQuantity::MassConcentration   , // 11
   Measurement::PhysicalQuantity::VolumeConcentration , // 12
   Measurement::PhysicalQuantity::Viscosity           , // 13
   Measurement::PhysicalQuantity::SpecificHeatCapacity, // 14
};

QString Measurement::getDisplayName(Measurement::PhysicalQuantity const physicalQuantity) {
   //
   // We could use an EnumStringMapping object to hold all the data and then call its enumToString member function.
   // However, the advantage of using a switch statement is that the compiler will warn us if we have missed one of the
   // enum values (because it's a strongly-typed enum).  This is better than waiting until run time for
   // EnumStringMapping::enumToString to log an error and throw an exception).
   //
   switch (physicalQuantity) {
      case Measurement::PhysicalQuantity::Mass                : return QObject::tr("Mass"                  );
      case Measurement::PhysicalQuantity::Volume              : return QObject::tr("Volume"                );
      case Measurement::PhysicalQuantity::Time                : return QObject::tr("Time"                  );
      case Measurement::PhysicalQuantity::Temperature         : return QObject::tr("Temperature"           );
      case Measurement::PhysicalQuantity::Color               : return QObject::tr("Color"                 );
      case Measurement::PhysicalQuantity::Density             : return QObject::tr("Density"               );
      case Measurement::PhysicalQuantity::DiastaticPower      : return QObject::tr("Diastatic Power"       );
      case Measurement::PhysicalQuantity::Acidity             : return QObject::tr("Acidity"               );
      case Measurement::PhysicalQuantity::Bitterness          : return QObject::tr("Bitterness"            );
      case Measurement::PhysicalQuantity::Carbonation         : return QObject::tr("Carbonation"           );
      case Measurement::PhysicalQuantity::MassConcentration   : return QObject::tr("Mass Concentration"    );
      case Measurement::PhysicalQuantity::VolumeConcentration : return QObject::tr("Volume Concentration"  );
      case Measurement::PhysicalQuantity::Viscosity           : return QObject::tr("Viscosity"             );
      case Measurement::PhysicalQuantity::SpecificHeatCapacity: return QObject::tr("Specific Heat Capacity");
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here
   Q_ASSERT(false);
}


BtStringConst const & Measurement::getSettingsName(PhysicalQuantity const physicalQuantity) {
   // Some physical quantities, such as Time, only have one UnitSystem, so we don't strictly need to store those in
   // PersistentSettings.  However, it's simpler to keep the same logic for everything.
   switch (physicalQuantity) {
      // Yes, strictly, unitSystem_weight should be unitSystem_mass, but users already have this in their settings files
      // so it would be annoying to just change it now.
      case Measurement::PhysicalQuantity::Mass                : return unitSystem_weight              ;
      case Measurement::PhysicalQuantity::Volume              : return unitSystem_volume              ;
      case Measurement::PhysicalQuantity::Time                : return unitSystem_time                ;
      case Measurement::PhysicalQuantity::Temperature         : return unitSystem_temperature         ;
      case Measurement::PhysicalQuantity::Color               : return unitSystem_color               ;
      case Measurement::PhysicalQuantity::Density             : return unitSystem_density             ;
      case Measurement::PhysicalQuantity::DiastaticPower      : return unitSystem_diastaticPower      ;
      case Measurement::PhysicalQuantity::Acidity             : return unitSystem_acidity             ;
      case Measurement::PhysicalQuantity::Bitterness          : return unitSystem_bitterness          ;
      case Measurement::PhysicalQuantity::Carbonation         : return unitSystem_carbonation         ;
      case Measurement::PhysicalQuantity::MassConcentration   : return unitSystem_massConcentration   ;
      case Measurement::PhysicalQuantity::VolumeConcentration : return unitSystem_volumeConcentration ;
      case Measurement::PhysicalQuantity::Viscosity           : return unitSystem_viscosity           ;
      case Measurement::PhysicalQuantity::SpecificHeatCapacity: return unitSystem_specificHeatCapacity;
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here
   Q_ASSERT(false);
}


namespace Measurement {
   Mixed2PhysicalQuantities const PqEitherMassOrVolume              {std::make_tuple(PhysicalQuantity::Mass,              PhysicalQuantity::Volume             )};
   Mixed2PhysicalQuantities const PqEitherMassOrVolumeConcentration {std::make_tuple(PhysicalQuantity::MassConcentration, PhysicalQuantity::VolumeConcentration)};
}
