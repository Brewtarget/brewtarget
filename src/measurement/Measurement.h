/*
 * measurement/Measurement.h is part of Brewtarget, and is copyright the following
 * authors 2010-2022:
 * - Mark de Wever <koraq@xs4all.nl>
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
#ifndef MEASUREMENT_MEASUREMENT_H
#define MEASUREMENT_MEASUREMENT_H
#pragma once

#include <optional>

#include <QObject>
#include <QPair>
#include <QString>

#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"

class BtStringConst;
class NamedEntity;

namespace Measurement {
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
    *        Callers should not call this with \c Mixed as a parameter
    */
   UnitSystem const & getDisplayUnitSystem(PhysicalQuantity physicalQuantity);

   /**
    * \brief Returns the \c Unit (usually a Metric/SI one where this is an option) that we use for storing a given
    *        \c PhysicalQuantity
    */
   Unit const & getUnitForInternalStorage(PhysicalQuantity physicalQuantity);

   /*!
    * \brief Converts a quantity without units to a displayable string
    *
    * \param quantity the quantity to display
    * \param precision how many decimal places
    */
   QString displayQuantity(double quantity, int precision);

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
    * \brief Converts a measurement (aka amount) to a displayable string in the appropriate units.
    *
    * \param namedEntity Named Entity of which we want to display a property
    * \param guiObject the GUI object doing the display, used to access configured unit system & scale
    * \param propertyName the \c QObject::property of \c namedEntity that returns the amount we wish to display
    * \param units which unit system it is in
    * \param precision how many decimal places to use, defaulting to 3
    */
   QString displayAmount(NamedEntity * namedEntity,
                         QObject * guiObject,
                         BtStringConst const & propertyName,
                         Measurement::Unit const & units,
                         int precision = 3);

   /*!
    * \brief Converts a measurement (aka amount) to a displayable string in the appropriate units.
    *
    * \param amount the amount to display
    * \param section the name of the object to reference to get unit system & scales from the config file
    * \param propertyName the property name to complete the lookup for units&scales
    * \param precision how many decimal places to use, defaulting to 3
    */
   QString displayAmount(Measurement::Amount const & amount,
                         BtStringConst const & section,
                         BtStringConst const & propertyName,
                         int precision = 3);

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

   /*!
    * \brief Converts a measurement (aka amount) to its numerical equivalent in the specified or default units.
    *
    * \param namedEntity Named Entity of which we want to display a property
    * \param guiObject the GUI object doing the display, used to access configured unit system & scale
    * \param propertyName the \c QObject::property of \c namedEntity that returns the amount we wish to display
    * \param units the units that the measurement (amount) is in
    */
   double amountDisplay(NamedEntity* namedEntity,
                        QObject* guiObject,
                        BtStringConst const & propertyName,
                        Unit const * units = nullptr);

   /**
    * \brief Converts a range (ie min/max pair) of measurements (aka amounts) to its numerical equivalent in whatever
    *        units are configured for this property.
    *
    * \param namedEntity Named Entity of which we want to display a property
    * \param guiObject the GUI object doing the display, used to access configured unit system & scale
    * \param propertyNameMin
    * \param propertyNameMax
    * \param units the units that the measurement (amount) is in
    */
   QPair<double,double> displayRange(NamedEntity* namedEntity,
                                     QObject *guiObject,
                                     BtStringConst const & propertyNameMin,
                                     BtStringConst const & propertyNameMax,
                                     Unit const * units = nullptr);

   /**
    * \brief Converts a range (ie min/max pair) of measurements (aka amounts) to its numerical equivalent in whatever
    *        units are configured for this property.
    *
    * \param guiObject the GUI object doing the display, used to access configured unit system & scale
    * \param propertyName
    * \param min
    * \param max
    * \param units the units that the measurement (amount) is in
    */
   QPair<double,double> displayRange(QObject *guiObject,
                                     BtStringConst const & propertyName,
                                     double min,
                                     double max,
                                     Unit const & units);

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

   std::optional<Measurement::SystemOfMeasurement> getForcedSystemOfMeasurementForField(QString const & field,
                                                                                        QString const & section);
   std::optional<Measurement::UnitSystem::RelativeScale> getForcedRelativeScaleForField(QString const & field,
                                                                                        QString const & section);

   void setForcedSystemOfMeasurementForField(QString const & field,
                                             QString const & section,
                                             std::optional<Measurement::SystemOfMeasurement> forcedSystemOfMeasurement);

   void setForcedRelativeScaleForField(QString const & field,
                                       QString const & section,
                                       std::optional<Measurement::UnitSystem::RelativeScale> forcedScale);

   /**
    * \brief Returns the \c SystemOfMeasurement that should be used to display this field, based on the forced
    *        \c SystemOfMeasurement for the field if there is one or otherwise on the the system-wide default
    *        \c UnitSystem for the specified \c PhysicalQuantity.  NB: It \b is valid to call this for
    *        \c Measurement::PhysicalQuantity::Mixed.
    */
   Measurement::SystemOfMeasurement getSystemOfMeasurementForField(QString const & field,
                                                                   QString const & section,
                                                                   Measurement::PhysicalQuantity physicalQuantity);

   /**
    * \brief Returns the \c UnitSystem that should be used to display this field, based on the forced
    *        \c SystemOfMeasurement for the field if there is one or otherwise on the the system-wide default
    *        \c UnitSystem for the specified \c PhysicalQuantity.  NB: It is \b not valid to call this for
    *        \c Measurement::PhysicalQuantity::Mixed.
    */
   Measurement::UnitSystem const & getUnitSystemForField(QString const & field,
                                                         QString const & section,
                                                         Measurement::PhysicalQuantity physicalQuantity);
}

#endif
