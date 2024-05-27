/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartBase.h is part of Brewtarget, and is copyright the following authors 2023:
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
#ifndef WIDGETS_SMARTBASE_H
#define WIDGETS_SMARTBASE_H
#pragma once

#include "widgets/SmartAmountSettings.h"

/**
 * \class SmartBase
 *
 * \brief Convenience class that uses the Curiously Recurring Template Pattern to provide a consistent interface for
 *        \c SmartLabel and \c SmartField.  See comment in \c widgets/SmartField.h for more details.
 *
 *        Derived classes need to implement:
 *           SmartAmountSettings & settings()
 *           void correctEnteredText(SmartAmounts::ScaleInfo previousScaleInfo);
 *
 *        At some point we might eliminate this class as it does not add a huge amount, but it was quite useful when I
 *        was refactoring duplicated code out of \c SmartLabel and \c SmartField!
 */
template<class Derived>
class SmartBase {
public:
   SmartBase() :
      m_derived{static_cast<Derived *>(this)} {
      return;
   }
   virtual ~SmartBase() = default;

   TypeInfo const & getTypeInfo() const {
      return this->m_derived->settings().getTypeInfo();
   }

   void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement) {
      this->m_derived->settings().setForcedSystemOfMeasurement(systemOfMeasurement);
      return;
   }

   void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale) {
      this->m_derived->settings().setForcedRelativeScale(relativeScale);
      return;
   }

   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const {
      return this->m_derived->settings().getForcedSystemOfMeasurement();
   }

   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const {
      return this->m_derived->settings().getForcedRelativeScale();
   }

   SmartAmounts::ScaleInfo getScaleInfo() const {
      return this->m_derived->settings().getScaleInfo();
   }

   Measurement::UnitSystem const & getUnitSystem(SmartAmounts::ScaleInfo const & scaleInfo) const {
      return this->m_derived->settings().getUnitSystem(scaleInfo);
   }

   Measurement::UnitSystem const & getDisplayUnitSystem() const {
      return this->m_derived->settings().getDisplayUnitSystem();
   }


   Measurement::PhysicalQuantity getPhysicalQuantity() const {
      return this->m_derived->settings().getPhysicalQuantity();
   }

   void selectPhysicalQuantity(Measurement::PhysicalQuantity const physicalQuantity) {
      auto const previousScaleInfo = this->m_derived->getScaleInfo();
      this->m_derived->settings().selectPhysicalQuantity(physicalQuantity);
      this->m_derived->correctEnteredText(previousScaleInfo);
      return;
   }

   [[deprecated]] void selectPhysicalQuantity(bool const isFirst) {
      auto const previousScaleInfo = this->m_derived->getScaleInfo();
      this->m_derived->settings().selectPhysicalQuantity(isFirst);
      this->m_derived->correctEnteredText(previousScaleInfo);
      return;
   }

   [[nodiscard]] QString displayAmount(double quantity, unsigned int precision) const {
      return this->m_derived->settings().displayAmount(quantity, precision);
   }

   [[nodiscard]] QString displayAmount(Measurement::Amount const & amount, unsigned int precision) {
      return this->m_derived->settings().displayAmount(amount, precision);
   }

protected:
   /**
    * \brief This is the 'this' pointer downcast to the derived class, which allows us to call non-virtual member
    *        functions in the derived class from this templated base class.
    */
   Derived * m_derived;

};
#endif
