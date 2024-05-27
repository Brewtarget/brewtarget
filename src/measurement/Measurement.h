/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Measurement.h is part of Brewtarget, and is copyright the following authors 2010-2023:
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef MEASUREMENT_MEASUREMENT_H
#define MEASUREMENT_MEASUREMENT_H
#pragma once

#include <optional>

#include <QObject>
#include <QPair>
#include <QString>

#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "BtFieldType.h"

class BtStringConst;
class NamedEntity;
struct TypeInfo;

namespace Measurement {

   /**
    * \brief Use this when you want to get the text as a number (and ignore any units or other trailling letters or
    *        symbols).  Valid specialisations are \c int, \c unsigned \c int, and \c double.
    *
    * \param ok If set, used to return \c true if parsing of raw text went OK and \c false otherwise (in which case,
    *           function return value will be 0).
    */
   template<typename T> [[nodiscard]] T extractRawFromString(QString const & input, bool * ok = nullptr);

   /**
    * \brief Alternate version of \c extractRawFromString for when you need a \c QVariant
    */
   [[nodiscard]] QVariant extractRawFromString(QString const & input, TypeInfo const & typeInfo, bool * ok = nullptr);

   void loadDisplayScales();
   void saveDisplayScales();

   /**
    * \brief Set the display \c UnitSystem for the specified \c PhysicalQuantity
    *        Obviously it is a requirement that the caller ensure that physicalQuantity == unitSystem.getPhysicalQuantity()
    */
   void setDisplayUnitSystem(PhysicalQuantity physicalQuantity, UnitSystem const & unitSystem);

   /**
    * \brief Set the supplied \c UnitSystem as the display \c UnitSystem for the \c PhysicalQuantity to which it relates
    */
   void setDisplayUnitSystem(UnitSystem const & unitSystem);

   /**
    * \brief Get the display \c UnitSystem for the specified \c PhysicalQuantity
    */
   UnitSystem const & getDisplayUnitSystem(PhysicalQuantity physicalQuantity);

   /*!
    * \brief Converts a quantity without units to a displayable string
    *
    * \param quantity the quantity to display
    * \param precision how many decimal places
    */
   QString displayQuantity(double quantity, int precision);

   QString displayQuantity(double quantity, int precision, NonPhysicalQuantity const nonPhysicalQuantity);

   /*!
    * \brief Converts a measurement (aka amount) to a displayable string in the appropriate units.
    *
    * \param amount the amount to display
    * \param precision how many decimal places
    * \param forcedSystemOfMeasurement if supplied, which which system of measurement to use, otherwise use the relevant
    *                                  system default
    * \param forcedScale if supplied, which scale to use, otherwise we use the largest scale that generates a value > 1
    */
   QString displayAmount(Measurement::Amount const & amount,
                         int precision = 3,
                         std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement = std::nullopt,
                         std::optional<Measurement::UnitSystem::RelativeScale> forcedScale = std::nullopt);

   /*!
    * \brief Converts a measurement (aka amount) to its numerical equivalent in the specified or default units.
    *
    * \param amount the amount to display
    * \param forcedSystemOfMeasurement if supplied, which which system of measurement to use, otherwise use the relevant
    *                                  system default
    * \param forcedScale if supplied, which scale to use, otherwise we use the largest scale that generates a value > 1
    */
   double amountDisplay(Measurement::Amount const & amount,
                        std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement = std::nullopt,
                        std::optional<Measurement::UnitSystem::RelativeScale> forcedScale = std::nullopt);

   //! \brief Displays thickness in appropriate units from standard thickness in L/kg.
   QString displayThickness(double thick_lkg, bool showUnits = true);
   //! \brief Appropriate thickness units will be placed in \c *volumeUnit and \c *weightUnit.
   void getThicknessUnits(Unit const ** volumeUnit, Unit const ** weightUnit);

   /*!
    * \return SI amount for the string.  Similar to \c Measurement::UnitSystem::qstringToSI
    *
    * \param qstr The string to convert - typically an amount typed in by the user
    * \param physicalQuantity Caller will already know whether the amount is a mass, volume, temperature etc, so they
    *                         should tell us via this parameter.  (We should always know this at compile-time, so we
    *                         could make it a template parameter, but I'm not sure it's worth the bother.)
    * \param forcedSystemOfMeasurement if supplied, which which system of measurement to use, otherwise use the relevant
    *                                  system default
    * \param forcedScale if supplied, which scale to use, otherwise we use the largest scale that generates a value > 1
    *                      the user is entering
    */
   Measurement::Amount qStringToSI(QString qstr,
                                   Measurement::PhysicalQuantity const physicalQuantity,
                                   std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement = std::nullopt,
                                   std::optional<Measurement::UnitSystem::RelativeScale> forcedScale = std::nullopt);
}

#endif
