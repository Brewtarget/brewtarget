/*
 * measurement/SystemOfMeasurement.cpp is part of Brewtarget, and is copyright the following
 * authors 2022:
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
      {QObject::tr("British Imperial")                 , Measurement::SystemOfMeasurement::Imperial                 },
      {QObject::tr("US Customary")                     , Measurement::SystemOfMeasurement::UsCustomary              },
      {QObject::tr("Metric")                           , Measurement::SystemOfMeasurement::Metric                   },
      {QObject::tr("Standard Time Units")              , Measurement::SystemOfMeasurement::StandardTimeUnits        },
      {QObject::tr("SRM (Standard Reference Method)")  , Measurement::SystemOfMeasurement::StandardReferenceMethod  },
      {QObject::tr("EBC (European Brewery Convention)"), Measurement::SystemOfMeasurement::EuropeanBreweryConvention},
      {QObject::tr("SG (Specific Gravity)")            , Measurement::SystemOfMeasurement::SpecificGravity          },
      {QObject::tr("Plato")                            , Measurement::SystemOfMeasurement::Plato                    },
      {QObject::tr("Lintner")                          , Measurement::SystemOfMeasurement::Lintner                  },
      {QObject::tr("WindischKolbach")                  , Measurement::SystemOfMeasurement::WindischKolbach          }
   };
   EnumStringMapping const systemOfMeasurementToUniqueName {
      {"Imperial"                 , Measurement::SystemOfMeasurement::Imperial                 },
      {"UsCustomary"              , Measurement::SystemOfMeasurement::UsCustomary              },
      {"Metric"                   , Measurement::SystemOfMeasurement::Metric                   },
      {"StandardTimeUnits"        , Measurement::SystemOfMeasurement::StandardTimeUnits        },
      {"StandardReferenceMethod"  , Measurement::SystemOfMeasurement::StandardReferenceMethod  },
      {"EuropeanBreweryConvention", Measurement::SystemOfMeasurement::EuropeanBreweryConvention},
      {"SpecificGravity"          , Measurement::SystemOfMeasurement::SpecificGravity          },
      {"Plato"                    , Measurement::SystemOfMeasurement::Plato                    },
      {"Lintner"                  , Measurement::SystemOfMeasurement::Lintner                  },
      {"WindischKolbach"          , Measurement::SystemOfMeasurement::WindischKolbach          }
   };
}

QString Measurement::getDisplayName(Measurement::SystemOfMeasurement systemOfMeasurement) {
   auto returnValue = systemOfMeasurementToDisplayName.enumToString(systemOfMeasurement);
   // It's a coding error if we don't find a result!
   if (!returnValue) {
      qCritical() <<
         Q_FUNC_INFO << "No mapping defined for SystemOfMeasurement #" << static_cast<int>(systemOfMeasurement);
      Q_ASSERT(false); // Stop here on debug builds
   }
   return *returnValue;
}

QString Measurement::getUniqueName(SystemOfMeasurement systemOfMeasurement) {
   auto returnValue = systemOfMeasurementToUniqueName.enumToString(systemOfMeasurement);
   // It's a coding error if we don't find a result!
   if (!returnValue) {
      qCritical() <<
         Q_FUNC_INFO << "No mapping defined for SystemOfMeasurement #" << static_cast<int>(systemOfMeasurement);
      Q_ASSERT(false); // Stop here on debug builds
   }
   return *returnValue;
}

std::optional<Measurement::SystemOfMeasurement> Measurement::getFromUniqueName(QString const & uniqueName) {
   return systemOfMeasurementToUniqueName.stringToEnum<Measurement::SystemOfMeasurement>(uniqueName);
}
