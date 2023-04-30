/*
 * measurement/SystemOfMeasurement.cpp is part of Brewtarget, and is copyright the following
 * authors 2022-2023:
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
#include "measurement/SystemOfMeasurement.h"

#include <QDebug>

#include "utils/EnumStringMapping.h"

namespace {
   EnumStringMapping const systemOfMeasurementToUniqueName {
      {"Imperial"                    , Measurement::SystemOfMeasurement::Imperial                    },
      {"UsCustomary"                 , Measurement::SystemOfMeasurement::UsCustomary                 },
      {"Metric"                      , Measurement::SystemOfMeasurement::Metric                      },
      {"MetricAlternate"             , Measurement::SystemOfMeasurement::MetricAlternate             },
      {"UniversalStandard"           , Measurement::SystemOfMeasurement::UniversalStandard           },
      {"StandardReferenceMethod"     , Measurement::SystemOfMeasurement::StandardReferenceMethod     },
      {"EuropeanBreweryConvention"   , Measurement::SystemOfMeasurement::EuropeanBreweryConvention   },
      {"Lovibond"                    , Measurement::SystemOfMeasurement::Lovibond                    },
      {"SpecificGravity"             , Measurement::SystemOfMeasurement::SpecificGravity             },
      {"Plato"                       , Measurement::SystemOfMeasurement::Plato                       },
      {"Brix"                        , Measurement::SystemOfMeasurement::Brix                        },
      {"Lintner"                     , Measurement::SystemOfMeasurement::Lintner                     },
      {"WindischKolbach"             , Measurement::SystemOfMeasurement::WindischKolbach             },
      {"CarbonationVolumes"          , Measurement::SystemOfMeasurement::CarbonationVolumes          },
      {"CarbonationMassPerVolume"    , Measurement::SystemOfMeasurement::CarbonationMassPerVolume    },
      {"MetricConcentration"         , Measurement::SystemOfMeasurement::MetricConcentration         },
      {"SpecificHeatCapacityCalories", Measurement::SystemOfMeasurement::SpecificHeatCapacityCalories},
      {"SpecificHeatCapacityJoules"  , Measurement::SystemOfMeasurement::SpecificHeatCapacityJoules  },
   };
}

QString Measurement::getDisplayName(Measurement::SystemOfMeasurement const systemOfMeasurement) {
   switch (systemOfMeasurement) {
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
      case Measurement::SystemOfMeasurement::MetricConcentration         : return QObject::tr("Metric Concentration"               );
      case Measurement::SystemOfMeasurement::SpecificHeatCapacityCalories: return QObject::tr("Specific Heat Capacity Calories per");
      case Measurement::SystemOfMeasurement::SpecificHeatCapacityJoules  : return QObject::tr("Specific Heat Capacity Joules per"  );
      // In C++23, we'd add:
      // default: std::unreachable();
   }
   // In C++23, we'd add:
   // std::unreachable()
   // It's a coding error if we get here
   Q_ASSERT(false);
}

QString Measurement::getUniqueName(SystemOfMeasurement systemOfMeasurement) {
   // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToString will log an error and
   // throw an exception).
   return systemOfMeasurementToUniqueName.enumToString(systemOfMeasurement);
}

std::optional<Measurement::SystemOfMeasurement> Measurement::getFromUniqueName(QString const & uniqueName) {
   return systemOfMeasurementToUniqueName.stringToEnumOrNull<Measurement::SystemOfMeasurement>(uniqueName);
}
