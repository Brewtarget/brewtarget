/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartAmountSettings.cpp is part of Brewtarget, and is copyright the following authors 2023-2024:
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
#include "widgets/SmartAmountSettings.h"

#include "measurement/Measurement.h"

// This private implementation class holds all private non-virtual members of SmartAmountSettings
class SmartAmountSettings::impl {
public:
   impl(SmartAmountSettings & self,
        char const * const        editorName,
        char const * const        labelOrFieldName,
        TypeInfo     const &      typeInfo,
        Measurement::Unit const * fixedDisplayUnit) :
      m_self             {self            },
      m_editorName       {editorName      },
      m_labelOrFieldName {labelOrFieldName},
      m_typeInfo         {typeInfo        },
      m_currentPhysicalQuantity{std::nullopt},
      m_fixedDisplayUnit {fixedDisplayUnit} {

      if (std::holds_alternative<NonPhysicalQuantity>(*this->m_typeInfo.fieldType)) {
         // It's a coding error to have a fixedDisplayUnit for a NonPhysicalQuantity
         Q_ASSERT(!this->m_fixedDisplayUnit);
      } else if (std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*this->m_typeInfo.fieldType)) {
         // If there is a choice of physical quantities (eg MassOrVolume) then start off with the first one
         this->m_currentPhysicalQuantity = Measurement::defaultPhysicalQuantity(std::get<Measurement::ChoiceOfPhysicalQuantity>(*this->m_typeInfo.fieldType));
      } else {
         Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->m_typeInfo.fieldType));
         this->m_currentPhysicalQuantity = std::get<Measurement::PhysicalQuantity>(*this->m_typeInfo.fieldType);
      }

      return;
   }

   ~impl() = default;

   SmartAmountSettings &     m_self            ;
   char const * const        m_editorName      ;
   char const * const        m_labelOrFieldName;
   TypeInfo     const &      m_typeInfo        ;
   // If m_typeInfo.fieldType is a ChoiceOfPhysicalQuantity (eg Mass_Volume), this is where we store which of
   // the two PhysicalQuantity values (eg Mass or Volume) is currently set.  If m_typeInfo.fieldType is a
   // PhysicalQuantity, then this will just be a copy of it.
   std::optional<Measurement::PhysicalQuantity> m_currentPhysicalQuantity;
   Measurement::Unit const * m_fixedDisplayUnit;

};

SmartAmountSettings::SmartAmountSettings(char const * const        editorName,
                                         char const * const        labelOrFieldName,
                                         TypeInfo     const &      typeInfo,
                                         Measurement::Unit const * fixedDisplayUnit) :
   pimpl{std::make_unique<impl>(*this,
                                editorName,
                                labelOrFieldName,
                                typeInfo,
                                fixedDisplayUnit)} {
   return;
}

SmartAmountSettings::~SmartAmountSettings() = default;

TypeInfo const & SmartAmountSettings::getTypeInfo() const {
   return this->pimpl->m_typeInfo;
}

void SmartAmountSettings::setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement) {
   // It's a coding error to call this when we have a fixed display unit
   Q_ASSERT(!this->pimpl->m_fixedDisplayUnit);

   SmartAmounts::setForcedSystemOfMeasurement(this->pimpl->m_editorName,
                                              this->pimpl->m_labelOrFieldName,
                                              forcedSystemOfMeasurement);
   return;
}

void SmartAmountSettings::setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> forcedScale) {
   // It's a coding error to call this when we have a fixed display unit
   Q_ASSERT(!this->pimpl->m_fixedDisplayUnit);

   SmartAmounts::setForcedRelativeScale(this->pimpl->m_editorName, this->pimpl->m_labelOrFieldName, forcedScale);
   return;
}

std::optional<Measurement::SystemOfMeasurement> SmartAmountSettings::getForcedSystemOfMeasurement() const {
   if (this->pimpl->m_fixedDisplayUnit) {
      return this->pimpl->m_fixedDisplayUnit->getUnitSystem().systemOfMeasurement;
   }
   return SmartAmounts::getForcedSystemOfMeasurement(this->pimpl->m_editorName, this->pimpl->m_labelOrFieldName);
}

std::optional<Measurement::UnitSystem::RelativeScale> SmartAmountSettings::getForcedRelativeScale() const {
   if (this->pimpl->m_fixedDisplayUnit) {
      //
      // NB: Not every Unit has a RelativeScale.
      // For the moment, I'm assuming there are no cases where RelativeScale matters when we have fixed units.  If we
      // find a case where this is not true, then we'd need to extend UnitSystem to allow it to give us a
      // std::optional<Measurement::UnitSystem::RelativeScale> for a specified Unit in that UnitSystem.
      //
      return std::nullopt;
   }
   return SmartAmounts::getForcedRelativeScale(this->pimpl->m_editorName, this->pimpl->m_labelOrFieldName);
}

SmartAmounts::ScaleInfo SmartAmountSettings::getScaleInfo() const {
   // Uncomment the next statement for diagnosing asserts!
//   qDebug().noquote() <<
//      Q_FUNC_INFO << this->pimpl->editorName << this->pimpl->labelOrFieldName << ":" << this->pimpl->m_typeInfo <<
//      "Stack trace:" << Logging::getStackTrace();

   if (this->pimpl->m_fixedDisplayUnit) {
      return SmartAmounts::ScaleInfo{this->pimpl->m_fixedDisplayUnit->getUnitSystem().systemOfMeasurement,
                                     std::nullopt};
   }

   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));
   return SmartAmounts::getScaleInfo(this->pimpl->m_editorName,
                                     this->pimpl->m_labelOrFieldName,
                                     ConvertToPhysicalQuantities(*this->pimpl->m_typeInfo.fieldType));
}

Measurement::UnitSystem const & SmartAmountSettings::getDisplayUnitSystem() const {
   // It's a coding error to call this for NonPhysicalQuantity, and we assert we never have a ChoiceOfPhysicalQuantity
   // for a SmartLabel that has no associated SmartField.
   Q_ASSERT(std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));
   return SmartAmounts::getUnitSystem(this->pimpl->m_editorName,
                                      this->pimpl->m_labelOrFieldName,
                                      std::get<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));
}

Measurement::PhysicalQuantity SmartAmountSettings::getPhysicalQuantity() const {
   // It's a coding error to call this for NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   return *this->pimpl->m_currentPhysicalQuantity;
}

void SmartAmountSettings::selectPhysicalQuantity(Measurement::PhysicalQuantity const physicalQuantity) {
   // It's a coding error to call this for NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   // It's a coding error to call this if we only hold one PhysicalQuantity
   Q_ASSERT(!std::holds_alternative<Measurement::PhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   // Uncomment the next statement for diagnosing asserts!
//   qDebug().noquote() <<
//      Q_FUNC_INFO << this->pimpl->m_editorName << "->" << this->pimpl->m_labelOrFieldName << " - TypeInfo:" <<
//      this->pimpl->m_typeInfo << ", physicalQuantity:" << physicalQuantity << ", Stack trace:" <<
//      Logging::getStackTrace();

   // It's a coding error to try to select a PhysicalQuantity that was not specified in the constructor
   auto const choiceOfPhysicalQuantity =
      std::get<Measurement::ChoiceOfPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType);
   Q_ASSERT(Measurement::isValid(choiceOfPhysicalQuantity, physicalQuantity));

   this->pimpl->m_currentPhysicalQuantity = physicalQuantity;

   return;
}

[[nodiscard]] QString SmartAmountSettings::displayAmount(double quantity, unsigned int precision) const {
   // It's a coding error to call this for NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   // I find this a nice level of abstraction. This lets all of the setText()
   // methods make a single call w/o having to do the logic for finding the
   // unit and scale.
   return Measurement::displayAmount(
      Measurement::Amount{quantity, Measurement::Unit::getCanonicalUnit(*this->pimpl->m_currentPhysicalQuantity)},
      precision,
      this->getForcedSystemOfMeasurement(),
      this->getForcedRelativeScale()
   );
}

[[nodiscard]] QString SmartAmountSettings::displayAmount(Measurement::Amount const & amount,
                                                         unsigned int precision) {
   // It's a coding error to call this for NonPhysicalQuantity
   Q_ASSERT(!std::holds_alternative<NonPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   // For now, I"m saying it's also a coding error to call this for a fixed physical quantity
   Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(*this->pimpl->m_typeInfo.fieldType));

   // Since we're given units, that tells us whether we're measuring by mass, volume, etc
   this->selectPhysicalQuantity(amount.unit->getPhysicalQuantity());

   qDebug() << Q_FUNC_INFO << "Precision:" << precision;

   return Measurement::displayAmount(
      amount,
      precision,
      this->getForcedSystemOfMeasurement(),
      this->getForcedRelativeScale()
   );
}
