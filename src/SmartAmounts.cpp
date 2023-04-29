/*
 * SmartAmounts.cpp is part of Brewtarget, and is copyright the following authors 2023:
 *   • Matt Young <mfsy@yahoo.com>
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
#include "SmartAmounts.h"

#include <QLabel>

#include "measurement/Measurement.h"
#include "PersistentSettings.h"
#include "utils/TypeLookup.h"
#include "widgets/SmartLabel.h"
#include "SmartField.h"

template<> void SmartAmounts::Init<SmartLabel>(char const * const editorName,
                                               char const * const labelName,
                                               char const * const labelFqName,
                                               SmartLabel &       label,
                                               char const * const fieldName,
                                               char const * const fieldlFqName,
                                               SmartField &    field,
                                               TypeInfo                    const & typeInfo,
                                               std::optional<unsigned int> const   precision,
                                               QString                     const & maximalDisplayString) {
   label.init(editorName, labelName, labelFqName, &field, typeInfo);
   field.init(editorName, fieldName, fieldlFqName, label, typeInfo, precision, maximalDisplayString);
   return;
}

template<> void SmartAmounts::Init<QLabel    >(char const * const editorName,
                                               [[maybe_unused]] char const * const labelName,
                                               [[maybe_unused]] char const * const labelFqName,
                                               QLabel &           label,
                                               char const * const fieldName,
                                               char const * const fieldlFqName,
                                               SmartField &       field,
                                               TypeInfo                    const & typeInfo,
                                               std::optional<unsigned int> const   precision,
                                               QString                     const & maximalDisplayString) {
   field.init(editorName, fieldName, fieldlFqName, label, typeInfo, precision, maximalDisplayString);
   return;
}

// .:TBD:. I think it is unnecessary to have precision and maximalDisplayString when there is no SmartField, but leaving
// them in for the moment, until I'm 100% sure.`
void SmartAmounts::InitNoSf(char const * const   editorName,
                            char const * const   labelName,
                            char const * const   labelFqName,
                            SmartLabel &         label,
                            TypeInfo     const & typeInfo,
                            [[maybe_unused]] std::optional<unsigned int> const   precision,
                            [[maybe_unused]] QString                     const & maximalDisplayString) {
   label.init(editorName, labelName, labelFqName, nullptr, typeInfo);
   return;
}

void SmartAmounts::InitFixed(char const * const editorName,
                             QLabel &           label,
                             char const * const fieldName,
                             char const * const fieldlFqName,
                             SmartField &       field,
                             TypeInfo          const & typeInfo,
                             Measurement::Unit const & fixedDisplayUnit,
                             std::optional<unsigned int> const   precision,
                             QString                     const & maximalDisplayString) {
   field.initFixed(editorName,
                   fieldName,
                   fieldlFqName,
                   label,
                   typeInfo,
                   fixedDisplayUnit,
                   precision,
                   maximalDisplayString);
   return;
}

void SmartAmounts::setForcedSystemOfMeasurement(char const * const owningWindowName,
                                              char const * const fieldName,
                                              std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) {
   if (forcedSystemOfMeasurement) {
      PersistentSettings::insert(fieldName,
                                 Measurement::getUniqueName(*forcedSystemOfMeasurement),
                                 owningWindowName,
                                 PersistentSettings::Extension::UNIT);
   } else {
      PersistentSettings::remove(fieldName,
                                 owningWindowName,
                                 PersistentSettings::Extension::UNIT);
   }
   return;
}

void SmartAmounts::setForcedRelativeScale(char const * const owningWindowName,
                                          char const * const fieldName,
                                          std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) {
   if (forcedScale) {
      PersistentSettings::insert(fieldName,
                                 Measurement::UnitSystem::getUniqueName(*forcedScale),
                                 owningWindowName,
                                 PersistentSettings::Extension::SCALE);
   } else {
      PersistentSettings::remove(fieldName,
                                 owningWindowName,
                                 PersistentSettings::Extension::SCALE);
   }
   return;
}

std::optional<Measurement::SystemOfMeasurement> SmartAmounts::getForcedSystemOfMeasurement(char const * const owningWindowName,
                                                                                           char const * const fieldName) {
   return Measurement::getFromUniqueName(
      PersistentSettings::value(fieldName,
                                "None", // This, or any invalid name, will give "no value" return from getFromUniqueName()
                                owningWindowName,
                                PersistentSettings::Extension::UNIT).toString()
   );
}

std::optional<Measurement::UnitSystem::RelativeScale> SmartAmounts::getForcedRelativeScale(char const * const owningWindowName,
                                                                                           char const * const fieldName) {
   return Measurement::UnitSystem::getScaleFromUniqueName(
      PersistentSettings::value(fieldName,
                                "None", // This, or any invalid name, will give "no value" return from getFromUniqueName()
                                owningWindowName,
                                PersistentSettings::Extension::SCALE).toString()
   );
}

Measurement::SystemOfMeasurement SmartAmounts::getSystemOfMeasurement(char const * const owningWindowName,
                                                                    char const * const fieldName,
                                                                    Measurement::PhysicalQuantities physicalQuantities) {
   auto forcedSystemOfMeasurement = SmartAmounts::getForcedSystemOfMeasurement(owningWindowName, fieldName);
   if (forcedSystemOfMeasurement) {
      return *forcedSystemOfMeasurement;
   }

   Measurement::PhysicalQuantity const physicalQuantity =
      std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities) ?
         std::get<Measurement::PhysicalQuantity>(physicalQuantities) :
         std::get<0>(std::get<Measurement::Mixed2PhysicalQuantities>(physicalQuantities));

   return Measurement::getDisplayUnitSystem(physicalQuantity).systemOfMeasurement;
}

Measurement::UnitSystem const & SmartAmounts::getUnitSystem(char const * const owningWindowName,
                                                          char const * const fieldName,
                                                          Measurement::PhysicalQuantity physicalQuantity) {
   auto forcedSystemOfMeasurement = SmartAmounts::getForcedSystemOfMeasurement(owningWindowName, fieldName);
   if (forcedSystemOfMeasurement) {
      return Measurement::UnitSystem::getInstance(*forcedSystemOfMeasurement, physicalQuantity);
   }
   return Measurement::getDisplayUnitSystem(physicalQuantity);
}

SmartAmounts::ScaleInfo SmartAmounts::getScaleInfo(char const * const owningWindowName,
                                               char const * const fieldName,
                                               Measurement::PhysicalQuantities physicalQuantities) {
   return SmartAmounts::ScaleInfo{
      SmartAmounts::getSystemOfMeasurement(owningWindowName, fieldName, physicalQuantities),
      SmartAmounts::getForcedRelativeScale(owningWindowName, fieldName)
   };
}
