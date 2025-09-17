/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/PhysicalQuantity.cpp is part of Brewtarget, and is copyright the following authors 2021-2025:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "measurement/PhysicalQuantity.h"

#include <utility>

#include <QDebug>
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#include "utils/EnumStringMapping.h"

//
// Settings we only use in this file.  Strictly we could put these as literals in Measurement::getSettingsName, but
// doing it this way is consistent with how we define other persistent setting name constants.
//
#define AddSettingName(name) namespace { BtStringConst const name{#name}; }
AddSettingName(unitSystem_acidity             )
AddSettingName(unitSystem_bitterness          )
AddSettingName(unitSystem_carbonation         )
AddSettingName(unitSystem_color               )
AddSettingName(unitSystem_count               )
AddSettingName(unitSystem_density             )
AddSettingName(unitSystem_diastaticPower      )
AddSettingName(unitSystem_heatCapacity        )
AddSettingName(unitSystem_length              )
AddSettingName(unitSystem_massFractionOrConc  )
AddSettingName(unitSystem_specificHeatCapacity)
AddSettingName(unitSystem_specificVolume      )
AddSettingName(unitSystem_temperature         )
AddSettingName(unitSystem_time                )
AddSettingName(unitSystem_viscosity           )
AddSettingName(unitSystem_volume              )
AddSettingName(unitSystem_weight              )
#undef AddSettingName

namespace {

   /**
    * \brief Construct a vector of \c To from a vector of \c From
    */
   template <typename To, typename From>
   std::vector<To> copyCast(std::vector<From> const & from) {
      // In theory we can set the size of the To vector in its constructor to be the same as the size of the From
      // vector, but GCC gives us compiler warnings about narrowing conversions from std::vector<From>::size_type to
      // std::vector<To>::size_type.  In practice, these vectors are so short that it's not buying us much to do this,
      // so we prefer to avoid the compiler warning.
      std::vector<To> to{};
      std::transform(from.cbegin(), from.cend(), std::back_inserter(to), [](From const & value) {
         return static_cast<To>(value);
      });
      return to;
   }

   // TBD: It would be nice to be able to make these constexpr, but we need to wait for compilers to catch up.  Eg,
   //      need GCC version 12 for this but, as of 2023-09-03, Ubuntu 22.04 is on GCC 11.4.0.
   std::vector<Measurement::PhysicalQuantity> const allOf_Mass_Volume      {Measurement::PhysicalQuantity::Mass, Measurement::PhysicalQuantity::Volume};
   std::vector<Measurement::PhysicalQuantity> const allOf_Mass_Volume_Count{Measurement::PhysicalQuantity::Mass, Measurement::PhysicalQuantity::Volume, Measurement::PhysicalQuantity::Count};

   std::vector<int> const allOfAsInt_Mass_Volume         = copyCast<int, Measurement::PhysicalQuantity>(allOf_Mass_Volume        );
   std::vector<int> const allOfAsInt_Mass_Volume_Count   = copyCast<int, Measurement::PhysicalQuantity>(allOf_Mass_Volume_Count  );
}

EnumStringMapping const Measurement::physicalQuantityStringMapping {
   {Measurement::PhysicalQuantity::Mass                , "Mass"                },
   {Measurement::PhysicalQuantity::Volume              , "Volume"              },
   {Measurement::PhysicalQuantity::Length              , "Length"              },
   {Measurement::PhysicalQuantity::Count               , "Count"               },
   {Measurement::PhysicalQuantity::Temperature         , "Temperature"         },
   {Measurement::PhysicalQuantity::Time                , "Time"                },
   {Measurement::PhysicalQuantity::Color               , "Color"               },
   {Measurement::PhysicalQuantity::Density             , "Density"             },
   {Measurement::PhysicalQuantity::DiastaticPower      , "DiastaticPower"      },
   {Measurement::PhysicalQuantity::Acidity             , "Acidity"             },
   {Measurement::PhysicalQuantity::Bitterness          , "Bitterness"          },
   {Measurement::PhysicalQuantity::Carbonation         , "Carbonation"         },
   {Measurement::PhysicalQuantity::MassFractionOrConc  , "MassFractionOrConc"  },
   {Measurement::PhysicalQuantity::Viscosity           , "Viscosity"           },
   {Measurement::PhysicalQuantity::SpecificHeatCapacity, "SpecificHeatCapacity"},
   {Measurement::PhysicalQuantity::HeatCapacity        , "HeatCapacity"        },
   {Measurement::PhysicalQuantity::SpecificVolume      , "SpecificVolume"      },
};

EnumStringMapping const Measurement::physicalQuantityDisplayNames {
   {Measurement::PhysicalQuantity::Mass                , QObject::tr("Weight (Mass)"                 )},
   {Measurement::PhysicalQuantity::Volume              , QObject::tr("Volume"                        )},
   {Measurement::PhysicalQuantity::Length              , QObject::tr("Length"                        )},
   {Measurement::PhysicalQuantity::Count               , QObject::tr("Count"                         )},
   {Measurement::PhysicalQuantity::Temperature         , QObject::tr("Temperature"                   )},
   {Measurement::PhysicalQuantity::Time                , QObject::tr("Time"                          )},
   {Measurement::PhysicalQuantity::Color               , QObject::tr("Color"                         )},
   {Measurement::PhysicalQuantity::Density             , QObject::tr("Density"                       )},
   {Measurement::PhysicalQuantity::DiastaticPower      , QObject::tr("Diastatic Power"               )},
   {Measurement::PhysicalQuantity::Acidity             , QObject::tr("Acidity"                       )},
   {Measurement::PhysicalQuantity::Bitterness          , QObject::tr("Bitterness"                    )},
   {Measurement::PhysicalQuantity::Carbonation         , QObject::tr("Carbonation"                   )},
   {Measurement::PhysicalQuantity::MassFractionOrConc  , QObject::tr("Mass Fraction or Concentration")},
   {Measurement::PhysicalQuantity::Viscosity           , QObject::tr("Viscosity"                     )},
   {Measurement::PhysicalQuantity::SpecificHeatCapacity, QObject::tr("Specific Heat Capacity"        )},
   {Measurement::PhysicalQuantity::HeatCapacity        , QObject::tr("Heat Capacity"                 )},
   {Measurement::PhysicalQuantity::SpecificVolume      , QObject::tr("Specific Volume"               )},
};


BtStringConst const & Measurement::getSettingsName(PhysicalQuantity const physicalQuantity) {
   // Some physical quantities, such as Time, only have one UnitSystem, so we don't strictly need to store those in
   // PersistentSettings.  However, it's simpler to keep the same logic for everything.
   switch (physicalQuantity) {
      // Yes, strictly, unitSystem_weight should be unitSystem_mass, but users already have this in their settings files
      // so it would be annoying to just change it now.
      case Measurement::PhysicalQuantity::Mass                : return unitSystem_weight              ;
      case Measurement::PhysicalQuantity::Volume              : return unitSystem_volume              ;
      case Measurement::PhysicalQuantity::Length              : return unitSystem_length              ;
      case Measurement::PhysicalQuantity::Time                : return unitSystem_time                ;
      case Measurement::PhysicalQuantity::Count               : return unitSystem_count               ;
      case Measurement::PhysicalQuantity::Temperature         : return unitSystem_temperature         ;
      case Measurement::PhysicalQuantity::Color               : return unitSystem_color               ;
      case Measurement::PhysicalQuantity::Density             : return unitSystem_density             ;
      case Measurement::PhysicalQuantity::DiastaticPower      : return unitSystem_diastaticPower      ;
      case Measurement::PhysicalQuantity::Acidity             : return unitSystem_acidity             ;
      case Measurement::PhysicalQuantity::Bitterness          : return unitSystem_bitterness          ;
      case Measurement::PhysicalQuantity::Carbonation         : return unitSystem_carbonation         ;
      case Measurement::PhysicalQuantity::MassFractionOrConc  : return unitSystem_massFractionOrConc  ;
      case Measurement::PhysicalQuantity::Viscosity           : return unitSystem_viscosity           ;
      case Measurement::PhysicalQuantity::SpecificHeatCapacity: return unitSystem_specificHeatCapacity;
      case Measurement::PhysicalQuantity::HeatCapacity        : return unitSystem_heatCapacity        ;
      case Measurement::PhysicalQuantity::SpecificVolume      : return unitSystem_specificVolume      ;
   }
   // It's a coding error if we get here
   Q_UNREACHABLE();
}

EnumStringMapping const Measurement::choiceOfPhysicalQuantityStringMapping {
   {Measurement::ChoiceOfPhysicalQuantity::Mass_Volume        , "Mass_Volume"        },
   {Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count  , "Mass_Volume_Count"  },
};

EnumStringMapping const Measurement::choiceOfPhysicalQuantityDisplayNames {
   {Measurement::ChoiceOfPhysicalQuantity::Mass_Volume        , QObject::tr("Mass or Volume"              )},
   {Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count  , QObject::tr("Mass, Volume or Count"       )},
};

// Default case is that PhysicalQuantities holds PhysicalQuantity; specialisations are for all ChoiceOfPhysicalQuantity
// possibilities.  Note that, because this is a function template, we are not allowed _partial_ specialisations.
template<Measurement::PhysicalQuantityTypes PQT, PQT const pqt> Measurement::PhysicalQuantity Measurement::defaultPhysicalQuantity() {
   return pqt;
}
template<> Measurement::PhysicalQuantity Measurement::defaultPhysicalQuantity<Measurement::ChoiceOfPhysicalQuantity,
                                                                              Measurement::ChoiceOfPhysicalQuantity::Mass_Volume      >() {
   return Measurement::PhysicalQuantity::Mass;
}
template<> Measurement::PhysicalQuantity Measurement::defaultPhysicalQuantity<Measurement::ChoiceOfPhysicalQuantity,
                                                                              Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count>() {
   return Measurement::PhysicalQuantity::Mass;
}


Measurement::PhysicalQuantity Measurement::defaultPhysicalQuantity(Measurement::ChoiceOfPhysicalQuantity const val) {
   switch (val) {
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume      :
         return Measurement::defaultPhysicalQuantity<Measurement::ChoiceOfPhysicalQuantity,
                                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume      >();
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count:
         return Measurement::defaultPhysicalQuantity<Measurement::ChoiceOfPhysicalQuantity,
                                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count>();
   }
   Q_UNREACHABLE();
}

template<Measurement::PhysicalQuantity const pq> bool isValid(Measurement::PhysicalQuantity const physicalQuantity) {
   return physicalQuantity == pq;
}
template<> bool Measurement::isValid<Measurement::ChoiceOfPhysicalQuantity,
                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume      >(Measurement::PhysicalQuantity const physicalQuantity) {
   return (physicalQuantity == Measurement::PhysicalQuantity::Mass  ||
           physicalQuantity == Measurement::PhysicalQuantity::Volume);
}
template<> bool Measurement::isValid<Measurement::ChoiceOfPhysicalQuantity,
                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count>(Measurement::PhysicalQuantity const physicalQuantity) {
   return (physicalQuantity == Measurement::PhysicalQuantity::Mass   ||
           physicalQuantity == Measurement::PhysicalQuantity::Volume ||
           physicalQuantity == Measurement::PhysicalQuantity::Count  );
}

bool Measurement::isValid(Measurement::ChoiceOfPhysicalQuantity const choiceOfPhysicalQuantity,
                          Measurement::PhysicalQuantity const physicalQuantity) {
   switch (choiceOfPhysicalQuantity) {
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume       :
         return Measurement::isValid<Measurement::ChoiceOfPhysicalQuantity,
                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume      >(physicalQuantity);
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count :
         return Measurement::isValid<Measurement::ChoiceOfPhysicalQuantity,
                                     Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count>(physicalQuantity);
   }
   Q_UNREACHABLE();
}

std::vector<Measurement::PhysicalQuantity> const & Measurement::allPossibilities(
   Measurement::ChoiceOfPhysicalQuantity const val
) {
   switch (val) {
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume        : return allOf_Mass_Volume        ;
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count  : return allOf_Mass_Volume_Count  ;
   }
   Q_UNREACHABLE();
}

std::vector<int> const & Measurement::allPossibilitiesAsInt(
   Measurement::ChoiceOfPhysicalQuantity const val
) {
   // It's a bit ugly having this as almost copy-and-paste of allPossibilities, but we'll live with it for now.
   switch (val) {
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume        : return allOfAsInt_Mass_Volume        ;
      case Measurement::ChoiceOfPhysicalQuantity::Mass_Volume_Count  : return allOfAsInt_Mass_Volume_Count  ;
   }
   Q_UNREACHABLE();
}

template<class S>
S & operator<<(S & stream, Measurement::PhysicalQuantity const val) {
   stream <<
      "PhysicalQuantity #" << static_cast<int>(val) << ": (" <<
      Measurement::physicalQuantityStringMapping[val] << ")";
   return stream;
}
template QDebug      & operator<<(QDebug      & stream, Measurement::PhysicalQuantity const val);
template QTextStream & operator<<(QTextStream & stream, Measurement::PhysicalQuantity const val);

template<class S>
S & operator<<(S & stream, Measurement::ChoiceOfPhysicalQuantity const val) {
   stream <<
      "PhysicalQuantity #" << static_cast<int>(val) << ": (" <<
      Measurement::choiceOfPhysicalQuantityStringMapping[val] << ")";
   return stream;
}
template QDebug      & operator<<(QDebug      & stream, Measurement::ChoiceOfPhysicalQuantity const val);
template QTextStream & operator<<(QTextStream & stream, Measurement::ChoiceOfPhysicalQuantity const val);
