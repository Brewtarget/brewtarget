/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * widgets/SmartBase.h is part of Brewtarget, and is copyright the following authors 2023-2025:
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

#include "utils/CuriouslyRecurringTemplateBase.h"
#include "widgets/SmartAmountSettings.h"

/**
 * \class SmartBase
 *
 * \brief Convenience class that uses the Curiously Recurring Template Pattern to provide a consistent interface for
 *        \c SmartLabel and \c SmartField.  See comment in \c widgets/SmartField.h for more details.
 *
 *        Derived classes need to implement:
 *           SmartAmountSettings const & settings() const
 *           void correctEnteredText(SmartAmounts::ScaleInfo previousScaleInfo);
 *
 *        We also need a non-const version of the first of the functions above, but we implement that here in the base
 *        class.
 *
 *        At some point we might eliminate this class as it does not add a huge amount, but it was quite useful when I
 *        was refactoring duplicated code out of \c SmartLabel and \c SmartField!
 */
template<class Derived>
class SmartBase : public CuriouslyRecurringTemplateBase<SmartBase, Derived> {
public:
   SmartBase() {
      return;
   }
   virtual ~SmartBase() = default;

   //! Name-hiding means we cannot call this settings(), so we choose a different name
   SmartAmountSettings & mutableSettings() {
      // It's always safe to cast this _to_ const
      Derived const & constSelf{const_cast<Derived const &>(this->derived())};
      SmartAmountSettings const & constSettings{constSelf.settings()};
      // We're casting away constness of the reference, which is a bit less "good practice", but shouldn't break
      // anything...
      return const_cast<SmartAmountSettings &>(constSettings);
   }

   TypeInfo const & getTypeInfo() const {
      return this->derived().settings().getTypeInfo();
   }

   void setForcedSystemOfMeasurement(std::optional<Measurement::SystemOfMeasurement> systemOfMeasurement) {
      this->mutableSettings().setForcedSystemOfMeasurement(systemOfMeasurement);
      return;
   }

   void setForcedRelativeScale(std::optional<Measurement::UnitSystem::RelativeScale> relativeScale) {
      this->mutableSettings().setForcedRelativeScale(relativeScale);
      return;
   }

   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurement() const {
      return this->derived().settings().getForcedSystemOfMeasurement();
   }

   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScale() const {
      return this->derived().settings().getForcedRelativeScale();
   }

   SmartAmounts::ScaleInfo getScaleInfo() const {
      return this->derived().settings().getScaleInfo();
   }

   Measurement::UnitSystem const & getDisplayUnitSystem() const {
      return this->derived().settings().getDisplayUnitSystem();
   }

   Measurement::PhysicalQuantity getPhysicalQuantity() const {
      return this->derived().settings().getPhysicalQuantity();
   }

   void selectPhysicalQuantity(Measurement::PhysicalQuantity const physicalQuantity) {
      auto const previousScaleInfo = this->derived().getScaleInfo();
      this->mutableSettings().selectPhysicalQuantity(physicalQuantity);
      this->derived().correctEnteredText(previousScaleInfo);
      return;
   }

   [[nodiscard]] QString displayQuantity(double quantity, unsigned int precision) const {
      return this->derived().settings().displayQuantity(quantity, precision);
   }

   [[nodiscard]] QString displayAmount(Measurement::Amount const & amount, unsigned int precision) {
      return this->mutableSettings().displayAmount(amount, precision);
   }
};

#endif
