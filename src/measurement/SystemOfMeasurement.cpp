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
   //
   // One day we should probably combine these two mapping tables
   //
   EnumStringMapping const systemOfMeasurementToDisplayName {
      {QObject::tr("British Imperial"                 ), Measurement::SystemOfMeasurement::Imperial                  },
      {QObject::tr("US Customary"                     ), Measurement::SystemOfMeasurement::UsCustomary               },
      {QObject::tr("Metric"                           ), Measurement::SystemOfMeasurement::Metric                    },
      {QObject::tr("Metric Alternate"                 ), Measurement::SystemOfMeasurement::MetricAlternate           },
      {QObject::tr("Universal Standard"               ), Measurement::SystemOfMeasurement::UniversalStandard         },
      {QObject::tr("SRM (Standard Reference Method)"  ), Measurement::SystemOfMeasurement::StandardReferenceMethod   },
      {QObject::tr("EBC (European Brewery Convention)"), Measurement::SystemOfMeasurement::EuropeanBreweryConvention },
      {QObject::tr("Lovibond"                         ), Measurement::SystemOfMeasurement::Lovibond                  },
      {QObject::tr("SG (Specific Gravity)"            ), Measurement::SystemOfMeasurement::SpecificGravity           },
      {QObject::tr("Plato"                            ), Measurement::SystemOfMeasurement::Plato                     },
      {QObject::tr("Brix"                             ), Measurement::SystemOfMeasurement::Brix                      },
      {QObject::tr("Lintner"                          ), Measurement::SystemOfMeasurement::Lintner                   },
      {QObject::tr("Windisch Kolbach"                 ), Measurement::SystemOfMeasurement::WindischKolbach           },
      {QObject::tr("Carbonation Volumes"              ), Measurement::SystemOfMeasurement::CarbonationVolumes        },
      {QObject::tr("Carbonation Mass Per Volume"      ), Measurement::SystemOfMeasurement::CarbonationMassPerVolume  },
      {QObject::tr("Concentration Parts Per"          ), Measurement::SystemOfMeasurement::ConcentrationPartsPer     },
      {QObject::tr("Concentration Mass Per Volume"    ), Measurement::SystemOfMeasurement::ConcentrationMassPerVolume},
   };
   EnumStringMapping const systemOfMeasurementToUniqueName {
      {"Imperial"                  , Measurement::SystemOfMeasurement::Imperial                  },
      {"UsCustomary"               , Measurement::SystemOfMeasurement::UsCustomary               },
      {"Metric"                    , Measurement::SystemOfMeasurement::Metric                    },
      {"MetricAlternate"           , Measurement::SystemOfMeasurement::MetricAlternate           },
      {"UniversalStandard"         , Measurement::SystemOfMeasurement::UniversalStandard         },
      {"StandardReferenceMethod"   , Measurement::SystemOfMeasurement::StandardReferenceMethod   },
      {"EuropeanBreweryConvention" , Measurement::SystemOfMeasurement::EuropeanBreweryConvention },
      {"Lovibond"                  , Measurement::SystemOfMeasurement::Lovibond                  },
      {"SpecificGravity"           , Measurement::SystemOfMeasurement::SpecificGravity           },
      {"Plato"                     , Measurement::SystemOfMeasurement::Plato                     },
      {"Brix"                      , Measurement::SystemOfMeasurement::Brix                      },
      {"Lintner"                   , Measurement::SystemOfMeasurement::Lintner                   },
      {"WindischKolbach"           , Measurement::SystemOfMeasurement::WindischKolbach           },
      {"CarbonationVolumes"        , Measurement::SystemOfMeasurement::CarbonationVolumes        },
      {"CarbonationMassPerVolume"  , Measurement::SystemOfMeasurement::CarbonationMassPerVolume  },
      {"ConcentrationPartsPer"     , Measurement::SystemOfMeasurement::ConcentrationPartsPer     },
      {"ConcentrationMassPerVolume", Measurement::SystemOfMeasurement::ConcentrationMassPerVolume},
   };
}

QString Measurement::getDisplayName(Measurement::SystemOfMeasurement systemOfMeasurement) {
   // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToString will log an error and
   // throw an exception).
   return systemOfMeasurementToDisplayName.enumToString(systemOfMeasurement);
}

QString Measurement::getUniqueName(SystemOfMeasurement systemOfMeasurement) {
   // It's a coding error if we don't find a result (in which case EnumStringMapping::enumToString will log an error and
   // throw an exception).
   return systemOfMeasurementToUniqueName.enumToString(systemOfMeasurement);
}

std::optional<Measurement::SystemOfMeasurement> Measurement::getFromUniqueName(QString const & uniqueName) {
   return systemOfMeasurementToUniqueName.stringToEnumOrNull<Measurement::SystemOfMeasurement>(uniqueName);
}
