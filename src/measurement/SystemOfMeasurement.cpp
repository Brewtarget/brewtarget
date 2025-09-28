/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/SystemOfMeasurement.cpp is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#include "measurement/SystemOfMeasurement.h"

#include <QDebug>
#include <qglobal.h> // For Q_ASSERT and Q_UNREACHABLE

#include "utils/EnumStringMapping.h"

namespace {
   EnumStringMapping const systemOfMeasurementToUniqueName {
      {Measurement::SystemOfMeasurement::Imperial                    , "Imperial"                    },
      {Measurement::SystemOfMeasurement::UsCustomary                 , "UsCustomary"                 },
      {Measurement::SystemOfMeasurement::Metric                      , "Metric"                      },
      {Measurement::SystemOfMeasurement::MetricAlternate             , "MetricAlternate"             },
      {Measurement::SystemOfMeasurement::UniversalStandard           , "UniversalStandard"           },
      {Measurement::SystemOfMeasurement::StandardReferenceMethod     , "StandardReferenceMethod"     },
      {Measurement::SystemOfMeasurement::EuropeanBreweryConvention   , "EuropeanBreweryConvention"   },
      {Measurement::SystemOfMeasurement::Lovibond                    , "Lovibond"                    },
      {Measurement::SystemOfMeasurement::SpecificGravity             , "SpecificGravity"             },
      {Measurement::SystemOfMeasurement::Plato                       , "Plato"                       },
      {Measurement::SystemOfMeasurement::Brix                        , "Brix"                        },
      {Measurement::SystemOfMeasurement::Lintner                     , "Lintner"                     },
      {Measurement::SystemOfMeasurement::WindischKolbach             , "WindischKolbach"             },
      {Measurement::SystemOfMeasurement::CarbonationVolumes          , "CarbonationVolumes"          },
      {Measurement::SystemOfMeasurement::CarbonationMassPerVolume    , "CarbonationMassPerVolume"    },
      {Measurement::SystemOfMeasurement::BrewingConcentration        , "BrewingConcentration"        },
      {Measurement::SystemOfMeasurement::SpecificHeatCapacityCalories, "SpecificHeatCapacityCalories"},
      {Measurement::SystemOfMeasurement::SpecificHeatCapacityJoules  , "SpecificHeatCapacityJoules"  },
      {Measurement::SystemOfMeasurement::SpecificHeatCapacityBtus    , "SpecificHeatCapacityBtus"    },
      {Measurement::SystemOfMeasurement::HeatCapacityKilocalories    , "HeatCapacityKilocalories"    },
      {Measurement::SystemOfMeasurement::HeatCapacityJoules          , "HeatCapacityJoules"          },
      {Measurement::SystemOfMeasurement::HeatCapacityBtus            , "HeatCapacityBtus"            },
   };
}

QString Measurement::getDisplayName(Measurement::SystemOfMeasurement const systemOfMeasurement) {
   switch (systemOfMeasurement) {
      // TBD: Maybe we need a better display name for BrewingConcentration.  "Mass Fraction or Mass Concentration for
      //      Brewers" seems a bit unwieldy though!
      case Measurement::SystemOfMeasurement::Imperial                    : return QObject::tr("British Imperial"                   );
      case Measurement::SystemOfMeasurement::UsCustomary                 : return QObject::tr("US Customary"                       );
      case Measurement::SystemOfMeasurement::Metric                      : return QObject::tr("Metric"                             );
      case Measurement::SystemOfMeasurement::MetricAlternate             : return QObject::tr("Metric Alternate"                   );
      case Measurement::SystemOfMeasurement::UniversalStandard           : return QObject::tr("Universal Standard"                 );
      case Measurement::SystemOfMeasurement::StandardReferenceMethod     : return QObject::tr("SRM (Standard Reference Method)"    );
      case Measurement::SystemOfMeasurement::EuropeanBreweryConvention   : return QObject::tr("EBC (European Brewery Convention)"  );
      case Measurement::SystemOfMeasurement::Lovibond                    : return QObject::tr("Lovibond"                           );
      case Measurement::SystemOfMeasurement::SpecificGravity             : return QObject::tr("SG (Specific Gravity)"              );
      case Measurement::SystemOfMeasurement::Plato                       : return QObject::tr("Plato"                              );
      case Measurement::SystemOfMeasurement::Brix                        : return QObject::tr("Brix"                               );
      case Measurement::SystemOfMeasurement::Lintner                     : return QObject::tr("Lintner"                            );
      case Measurement::SystemOfMeasurement::WindischKolbach             : return QObject::tr("Windisch Kolbach"                   );
      case Measurement::SystemOfMeasurement::CarbonationVolumes          : return QObject::tr("Carbonation Volumes"                );
      case Measurement::SystemOfMeasurement::CarbonationMassPerVolume    : return QObject::tr("Carbonation Mass Per Volume"        );
      case Measurement::SystemOfMeasurement::BrewingConcentration        : return QObject::tr("Brewing Concentration"              );
      case Measurement::SystemOfMeasurement::SpecificHeatCapacityCalories: return QObject::tr("Specific Heat Capacity Calories per");
      case Measurement::SystemOfMeasurement::SpecificHeatCapacityJoules  : return QObject::tr("Specific Heat Capacity Joules per"  );
      case Measurement::SystemOfMeasurement::SpecificHeatCapacityBtus    : return QObject::tr("Specific Heat Capacity Btus per"    );
      case Measurement::SystemOfMeasurement::HeatCapacityKilocalories    : return QObject::tr("Heat Capacity Kilocalories per"     );
      case Measurement::SystemOfMeasurement::HeatCapacityJoules          : return QObject::tr("Heat Capacity Joules per"           );
      case Measurement::SystemOfMeasurement::HeatCapacityBtus            : return QObject::tr("Heat Capacity Btus per"             );
   }
   // It's a coding error if we get here
   Q_UNREACHABLE();
}

QString Measurement::getUniqueName(SystemOfMeasurement systemOfMeasurement) {
   // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToValue will log an error and
   // throw an exception).
   return systemOfMeasurementToUniqueName[systemOfMeasurement];
}

std::optional<Measurement::SystemOfMeasurement> Measurement::getFromUniqueName(QString const & uniqueName) {
   return systemOfMeasurementToUniqueName.stringToEnumOrNull<Measurement::SystemOfMeasurement>(uniqueName);
}
