/*
 * measurement/PhysicalQuantity.h is part of Brewtarget, and is copyright the following
 * authors 2021-2023:
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

#include <array>
#include <tuple>
#include <utility>
#include <variant>

#include <QString>

#include "utils/BtStringConst.h"

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
    *       Thus, each \c Unit belongs to exactly one \c UnitSystem and each \c UnitSystem relates to exactly one
    *       \c Measurement::PhysicalQuantity (which also means, of course, that \c Unit relates to exactly one
    *       \c Measurement::PhysicalQuantity, as you would expect).
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
    *       There is, however, one additional complication.  In certain places, we need to allow a quantity to be
    *       measured either by mass or by volume, according to the user's choice.  Eg for \c Misc ingredients, some will
    *       be best measured by weight and some by volume.  So, eg in \c MiscTableModel, we need to offer the options of
    *       "Imperial", "US Customary" and "Metric/SI" for the amount column without predetermining whether these will
    *       be volume or weight because that will depend on a per-row basis.)  This is what motivates us to model
    *       \c SystemOfMeasurement explicitly.
    *
    *       NOTE that there are other things that users can configure that do not belong with this group of classes
    *       because they do not related to physical quantities, eg date & time format and language choice do not fit
    *       well in here -- see \c NonPhysicalQuantity in \headerfile BtFieldType.h
    */
   enum class PhysicalQuantity {
      Mass,           // Elsewhere we use weight instead of mass because it's more idiomatic (despite being,
                      // strictly speaking, not the same thing)
      Volume,
      Time,           // Note this is durations of time, NOT dates or times of day  .:TBD:. Rename to TimeDuration
      Temperature,
      Color,
      // Density is sometimes referred to as "gravity" as a shorthand for "specific gravity".  Strictly, what we're
      // measuring as brewers is relative density (aka "ratio of the density ... with respect to water at its densest
      // (at 4 °C)" per https://en.wikipedia.org/wiki/Relative_density) in order to find % sugar content (see
      // https://en.wikipedia.org/wiki/Brix and https://en.wikipedia.org/wiki/Beer_measurement#Other_density_scales).
      // So we could call this RelativeDensity or SugarConcentration or Gravity.  But I think Density is truest to the
      // idea of a measurable physical quantity described above.
      Density,
      DiastaticPower,
      Acidity,
      Bitterness,
      // Per https://www.sciencedirect.com/topics/agricultural-and-biological-sciences/carbonation, "Carbonation is
      // measured as either ‘volumes’ or grams per litre. One volume means 1 L of CO2 in 1 L of drink.  This is
      // equivalent to 1.96 g/L (normally quoted as 2 g/L)."  Thus, although we're using similar units to measures of
      // concentration, the equivalences are different and, in practice, it's easiest to treat them as a completely
      // separate.
      Carbonation,
      // As explained at https://en.wikipedia.org/wiki/Concentration, there are several types of concentration,
      // including "mass concentration", which is expressed as mass-per-volume, and "volume concentration", which is
      // strictly-speaking a dimensionless number (because volume-per-volume cancels out) but is often expressed as
      // parts per million (or similar) or sometimes as a percentage.
      //
      // BeerJSON just bundles mass concentration and volume concentration scales together under "concentration", which
      // is sufficient for its purposes.  However, we don't want to do that, as we'd end up doing some contrived and
      // incorrect conversion between the two -- because there is no generic conversion between milligrams-per-litre and
      // parts-per-xxx.  (Converting mass-per-volume to volume-per-volume (or mass-per-mass) involves temperature and
      // the molar masses of the two substances in question.  Hence why a chemist would use
      // https://en.wikipedia.org/wiki/Molar_concentration instead.
      //
      // (Various converters on the internet will tell you that 1 mg/L is "the same as" 1 ppm, but this is only really
      // true if everything has the density of water.  In fairness to such converters, in practice, in brewing, for
      // small concentrations, it's often not hugely wrong to approximate 1 milligram-per-litre with 1
      // part-per-million.)
      //
      // See also https://en.wikipedia.org/wiki/Parts-per_notation.
      MassConcentration,
      VolumeConcentration,
      // Viscosity -- see https://en.wikipedia.org/wiki/Viscosity
      Viscosity,
      // Specific heat capacity -- see https://en.wikipedia.org/wiki/Specific_heat_capacity
      SpecificHeatCapacity,
      // .:TBD:. Should we add Energy for PropertyNames::Recipe::calories (in which case, should canonical measure be
      //         Joules)?
   };

   /**
    * \brief Array of all possible values of \c Measurement::PhysicalQuantity.  NB: This is \b not guaranteed to be in
    *        the same order as the values of the enum.
    *
    *        This is the least ugly way I could think of to allow other parts of the code to iterate over all values
    *        of enum class \c Measurement::PhysicalQuantity.  Hopefully, one day, when reflection
    *        (https://en.cppreference.com/w/cpp/experimental/reflect) gets incorporated into C++, this will ultimately
    *        be unnecessary.
    */
   extern std::array<Measurement::PhysicalQuantity, 14> const allPhysicalQuantites;

   /**
    * \brief Return the name of a \c PhysicalQuantity suitable either for display to the user or logging
    */
   QString getDisplayName(Measurement::PhysicalQuantity const physicalQuantity);

   /**
    * \brief Return the \c PersistentSettings name for looking up the display \c UnitSystem for the specified
    *        \c PhysicalQuantity
    */
   BtStringConst const & getSettingsName(Measurement::PhysicalQuantity const physicalQuantity);


   /**
    * \brief In a few cases, we want to be able to handle two different ways of measuring a thing (eg Mass and Volume,
    *        or MassConcentration and VolumeConcentration).
    *
    *        We adopt the convention that members of the tuple are in alphabetical order.
    *
    *        At the moment, we don't envisage a need for having more than two ways of measuring the same thing, but it's
    *        relatively obvious how to extend the approach here if we did need to.
    *
    *        Maybe a better name would be EitherOf2PhysicalQuantities of some such, but we retain Mixed for now as
    *        that's the word we used to use when the only pair was Mass and Volume.
    */
   using Mixed2PhysicalQuantities = std::tuple<Measurement::PhysicalQuantity, Measurement::PhysicalQuantity>;

   /**
    * \brief Of course, once we have \c Mixed2PhysicalQuantities, we need a way to store either that or a
    *        \c Measurement::PhysicalQuantity.
    *
    *        (Note that, \c BtFieldType is one place we \b don't use this as we need to add a third possibility there of
    *        \c NonPhysicalQuantity.)
    */
   using PhysicalQuantities = std::variant<Measurement::PhysicalQuantity, Mixed2PhysicalQuantities>;

   /**
    * \brief It's more concise to have a constant for Mass & Volume
    */
   extern Mixed2PhysicalQuantities const PqEitherMassOrVolume;

   /**
    * \brief It's more concise to have a constant for MassConcentration & VolumeConcentration
    */
   extern Mixed2PhysicalQuantities const PqEitherMassOrVolumeConcentration;

}

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, Measurement::PhysicalQuantity const physicalQuantity) {
   stream <<
      "PhysicalQuantity #" << static_cast<int>(physicalQuantity) << ": (" <<
      Measurement::getDisplayName(physicalQuantity) << ")";
   return stream;
}

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, Measurement::Mixed2PhysicalQuantities const mixed2PhysicalQuantities) {
   stream <<
      "Mixed2PhysicalQuantities:" << std::get<0>(mixed2PhysicalQuantities) << std::get<1>(mixed2PhysicalQuantities);
   return stream;
}

/**
 * \brief Convenience function for logging
 */
template<class S>
S & operator<<(S & stream, Measurement::PhysicalQuantities const & physicalQuantities) {
   if (std::holds_alternative<Measurement::PhysicalQuantity>(physicalQuantities)) {
      stream << "PhysicalQuantities:" << std::get<Measurement::PhysicalQuantity>(physicalQuantities);
   } else {
      stream << "PhysicalQuantities:" << std::get<Measurement::Mixed2PhysicalQuantities>(physicalQuantities);
   }
   return stream;
}

#endif
