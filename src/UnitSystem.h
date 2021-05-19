/*
 * UnitSystem.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#ifndef UNITSYSTEM_H
#define UNITSYSTEM_H
#pragma once

#include <QMap>
#include <QString>
#include "Unit.h"

/*!
 * \class UnitSystem
 *
 * \brief A unit system handles the display and format of physical quantities.
 */
class UnitSystem {
public:
   /*!
    * \brief Constructor
    *
    * \param type
    * \param thickness
    * \param defaultUnit
    * \param scaleToUnitEntries
    * \param qstringToUnitEntries
    * \param name
    */
   UnitSystem(Unit::UnitType type,
              Unit const * thickness,
              Unit const * defaultUnit,
              std::initializer_list<std::pair<Unit::unitScale, Unit const *> > scaleToUnitEntries,
              std::initializer_list<std::pair<QString, Unit const *> > qstringToUnitEntries,
              char const * name);

   ~UnitSystem() = default;

   /*!
    * \brief Returns a string appropriately displaying 'amount' of type 'units' in this UnitSystem.  This string should
    *        also be recognised by \c qstringToSI()
    *
    * \param amount
    * \param units
    * \param precision
    * \param scale
    *
    * \return
    */
   QString displayAmount(double amount, Unit const * units, int precision = -1, Unit::unitScale scale = Unit::noScale) const;

   /*!
    * \brief Returns the double representing the appropriate unit and scale. Similar in nature to \c displayAmount(),
    *        but just returning raw doubles.
    *
    * \param amount
    * \param units
    * \param scale
    *
    * \return
    */
   double amountDisplay(double amount, Unit const * units, Unit::unitScale scale = Unit::noScale) const;

   /*!
    * \brief Converts 'qstr' (consisting of a decimal amount, followed by a unit string) to the appropriate SI amount
    *        under this UnitSystem.
    *
    * \param qstr
    * \param defUnit
    * \param force
    * \param scale
    *
    * \return
    */
   double qstringToSI(QString qstr, Unit const * defUnit = nullptr, bool force = false, Unit::unitScale scale = Unit::noScale) const;

   /*!
    * \brief
    */
   Unit const * scaleUnit(Unit::unitScale scale) const;

   /*!
    * \brief Returns the unit associated with thickness. If this unit system is US weight, it would return lb. If it
    *        were US volume, it would return quarts.
    *
    * \return \c nullptr if thickness does not apply to this unit system (eg a temperature system)
    */
   Unit const * thicknessUnit() const;

   /*!
    * \brief Returns the default unit to use in this system - eg minutes for time, pounds for US weight
    */
   Unit const * unit() const;

   /*!
    * \brief Returns the name of the system of measurement for this unit system
    *
    * .:TODO:.  This is a bit confusing.  It can be a string representation of either
    *           Unit::SystemOfMeasurement, Unit::TempScale or Unit::UnitType!
    */
   QString const & unitType() const;

private:
   // This does most of the work for displayAmount() and amountDisplay()
   std::pair<double, QString> displayableAmount(double amount, Unit const * units, Unit::unitScale scale) const;

   Unit::UnitType const type;
   Unit const * thickness;
   Unit const * defaultUnit;

   // Map from a Unit::unitScale to a concrete Unit - eg in the US weight UnitSystem,
   // Unit::scaleExtraSmall maps to Units::ounces and Unit::scaleSmall maps to Units::pounds
   //
   // Because it's a map, when we iterate over it, we'll traverse from smallest to largest.
   QMap<Unit::unitScale, Unit const *> const scaleToUnit;

   // Map from SI abbreviation to a concrete \c Unit
   QMap<QString, Unit const *> const qstringToUnit;

   QString const name;
};


namespace UnitSystems {
   extern UnitSystem const usWeightUnitSystem;
   extern UnitSystem const siWeightUnitSystem;

   extern UnitSystem const imperialVolumeUnitSystem;
   extern UnitSystem const usVolumeUnitSystem;
   extern UnitSystem const siVolumeUnitSystem;

   extern UnitSystem const celsiusTempUnitSystem;
   extern UnitSystem const fahrenheitTempUnitSystem;

   extern UnitSystem const timeUnitSystem;

   extern UnitSystem const srmColorUnitSystem;
   extern UnitSystem const ebcColorUnitSystem;

   extern UnitSystem const sgDensityUnitSystem;
   extern UnitSystem const platoDensityUnitSystem;

   extern UnitSystem const lintnerDiastaticPowerUnitSystem;
   extern UnitSystem const wkDiastaticPowerUnitSystem;
}

#endif
