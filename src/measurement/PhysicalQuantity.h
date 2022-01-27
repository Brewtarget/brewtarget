/*
 * measurement/PhysicalQuantity.h is part of Brewtarget, and is copyright the following
 * authors 2021-2022:
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
#ifndef MEASUREMENT_PHYSICALQUANTITY_H
#define MEASUREMENT_PHYSICALQUANTITY_H
#pragma once

class QString;

namespace Measurement {
   /**
    * \enum PhysicalQuantity lists the various types of measurable physical quantity
    *       (https://en.wikipedia.org/wiki/Physical_quantity) for which we need to be able to store values
    *
    *       A particular physical quantity can be measured using one or more units of measurement
    *       (https://en.wikipedia.org/wiki/Unit_of_measurement) -- see \c Unit -- which in certain contexts we also
    *       call scales of measurement (because otherwise you'd be writing stilted phrases such as "measured in
    *       Celsius units of measurement" instead of the more idiomatic "measured on the Celsius scale").
    *
    *       Certain units of measurement (or scales) are grouped together into systems of measurement
    *       (https://en.wikipedia.org/wiki/System_of_measurement).  A system of measurement will typically cover
    *       various physical quantities and, for each of these, have one or more related units (aka scales) of
    *       measurement.  Where there are several related scales, we will typically implement only a subset of them
    *       that make sense for our application.  We will typically omit very small or very large scales - eg we don't
    *       include the grain or ton scales for weight/mass from the US Customary system of measurement.
    *
    *       Internally, we use a single unit of measurement for each physical quantity that we store any measure of.
    *       (These are usually SI units, where they apply, except that, because we're not in science lab, we store
    *       temperature in Celsius rather than Kelvin and usually refer to "the metric system" rather than "SI" or
    *       "International System of Measurement".)  Thus we always store measurements of volume in liters,
    *       measurements of weight (or, more strictly, mass) in kilograms, and so on.
    *
    *       However, we want users to be able to work with whatever (common) units of measurement are most appropriate
    *       to them.  So, for each physical quantity, we allow the user to choose a system of measurement (where they
    *       exist) or a scale (where no relevant systems of measurement exist) for display and default entry of each
    *       type of physical quantity.
    *
    *       Note that, although a system of measurement may cover several physical quantities (eg volume, mass,
    *       temperature) we allow users to make separate choices for each type of physical quantity, as this reflects
    *       what currently happens in the real world, where there is sometimes, for instance, partial migration from
    *       imperial to metric.
    *
    *       To keep the code simple, we group related scales together into a \c UnitSystem.  Eg, the \c UnitSystem for
    *       metric mass has scales (ie const \c Unit objects) for milligrams, grams and kilograms; the one for US
    *       Customary mass has pounds and ounces.
    *
    *       Thus, each \c Unit belongs to exactly one \c UnitSystem and, with one exception discussed below, each
    *       \c UnitSystem relates to exactly one \c Measurement::PhysicalQuantity (which also means, of course, that
    *       \c Unit relates to exactly one \c Measurement::PhysicalQuantity, as you would expect).
    *
    *       Additionally, each \c Unit knows how to convert itself to and from the canonical metric \c Unit that we
    *       use for internal storage of the corresponding physical quantity type.
    *
    *       It can be that \c Measurement::PhysicalQuantity only has one \c UnitSystem (eg, for time, we only want to
    *       measure things in the Coordinated Universal Time (UTC) units of seconds/minutes/hours/days/etc; we don't,
    *       for instance, want to support Metric time, Hexadecimal time or the beeps, hectobeeps and decidays of the
    *       Lukashian Calendar).  In this case, we do not offer the user a Hobson's choice of \c UnitSystem, we just
    *       use the sole one available.
    *
    *       It can also be that a \c UnitSystem has only one \c Unit (eg the two ways we support for measuring density
    *       each only have one scale).  In this case, it will feel to the user as though s/he is choosing a scale (aka
    *       \c Unit) directly rather than a \c UnitSystem, but of course the \c UnitSystem is still there and used.
    *
    *       The exception mentioned above is \c Measurement::PhysicalQuantity::Mixed which is used when we are allowing
    *       a quantity to be measured either by mass or by volume, according to the user's choice.  Eg for \c Misc
    *       ingredients, some will be best measured by weight and some by volume.  So, eg in \c MiscTableModel, we need
    *       to offer the options of "Imperial", "US Customary" and "Metric/SI" for the amount column without
    *       predetermining whether these will be volume or weight because that will depend on a per-row basis.)  This is
    *       what motivates us to model \c SystemOfMeasurement explicitly.
    *
    *       NOTE that there are other things that users can configure that do not belong with this group of classes
    *       because they do not related to physical quantities, eg date & time format and language choice do not fit
    *       well in here -- see \c NonPhysicalQuantity in \headerfile BtFieldType.h
    */
   enum class PhysicalQuantity {
      Mass,           // Elsewhere we use weight instead of mass because it's more idiomatic (despite being,
                      // strictly speaking, not the same thing)
      Volume,
      Time,           // Note this is durations of time, NOT dates or times of day
      Temperature,
      Color,
      Density,        // Sometimes referred to in comments as "gravity" as a shorthand for "specific gravity"
      Mixed,          // This is used for quantities where we allow measurement as either Mass or Volume
      DiastaticPower
   };

   /**
    * \brief Return the name of a \c PhysicalQuantity suitable either for display to the user or logging
    */
   QString getDisplayName(PhysicalQuantity physicalQuantity);
}
#endif
