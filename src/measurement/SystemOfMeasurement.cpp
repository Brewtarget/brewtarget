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
#include <QString>

#include "utils/EnumStringMapping.h"

namespace {
   //
   // One day we should probably combine these two mapping tables
   //
   EnumStringMapping const systemOfMeasurementToDisplayName {
      {QT_TR_NOOP("British Imperial")                 , Measurement::SystemOfMeasurement::Imperial                 },
      {QT_TR_NOOP("US Customary")                     , Measurement::SystemOfMeasurement::UsCustomary              },
      {QT_TR_NOOP("Metric")                           , Measurement::SystemOfMeasurement::Metric                   },
      {QT_TR_NOOP("Standard Time Units")              , Measurement::SystemOfMeasurement::StandardTimeUnits        },
      {QT_TR_NOOP("SRM (Standard Reference Method)")  , Measurement::SystemOfMeasurement::StandardReferenceMethod  },
      {QT_TR_NOOP("EBC (European Brewery Convention)"), Measurement::SystemOfMeasurement::EuropeanBreweryConvention},
      {QT_TR_NOOP("SG (Specific Gravity)")            , Measurement::SystemOfMeasurement::SpecificGravity          },
      {QT_TR_NOOP("Plato")                            , Measurement::SystemOfMeasurement::Plato                    },
      {QT_TR_NOOP("Lintner")                          , Measurement::SystemOfMeasurement::Lintner                  },
      {QT_TR_NOOP("WindischKolbach")                  , Measurement::SystemOfMeasurement::WindischKolbach          }
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
