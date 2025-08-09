/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/SystemOfMeasurement.h is part of Brewtarget, and is copyright the following authors 2022-2025:
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
#ifndef MEASUREMENT_SYSTEMOFMEASUREMENT_H
#define MEASUREMENT_SYSTEMOFMEASUREMENT_H
#pragma once

#include <optional>

#include <QString>

namespace Measurement {
   /**
    * \brief It is helpful for every \c UnitSystem to correspond to a \c SystemOfMeasurement.  This is at the cost of
    *        creating some contrived "systems of measurement" for scales such as plato, lintner, EBC, SRM, etc.
    *
    *        See more detailed comment in \c measurement/PhysicalQuantity.h
    */
   enum class SystemOfMeasurement {
      /**
       * Covers Length, Area, Volume, Mass and Weight -- of which we use Volume and Mass
       */
      Imperial,

      /**
       * Covers Length, Area, Fluid Volume, Dry Volume, Mass, Weight and Temperature -- of which wee use Fluid Volume,
       * Mass and Temperature
       */
      UsCustomary,

      /**
       * Despite the fact that the metric system offers many benefits over its imperial predecessors, it's not quite as
       * simple as one might imagine.  In short, there have been multiple metric systems (eg see
       * https://en.wikipedia.org/wiki/Metric_system#Common_metric_systems).  The "International System of Units (SI)"
       * is the official one adopted by all countries except Myanmar, Liberia, and the United States.  But it is not
       * always convenient or practical for brewers to use strict SI units -- eg it's far more likely you want to
       * measure wort temperature in Celsius than in Kelvin.
       *
       * So, \c Measurement::SystemOfMeasurement::Metric is probably best thought of as "metric for brewers".
       * Differences from SI include:
       *
       *    - We use Celsius for temperature rather than Kelvin
       *    - We include a volume scale (liters) which, strictly-speaking, SI does not (because you'd use cubic metres)
       *    - We use centipoise for viscosity rather than newton-seconds per square meter
       *      (Poise/centipoise comes from the "Centimetre–gram–second (CGS)" metric system, whose other units have
       *      largely fallen out of use --
       *      see https://en.wikipedia.org/wiki/Centimetre%E2%80%93gram%E2%80%93second_system_of_units.)
       *
       * Covers Length, Area, Volume, Mass, Weight, Temperature, Time (duration), and Viscosity -- of which we use
       * Volume, Mass and Temperature.  (See comment below in \c Measurement::SystemOfMeasurement::UniversalStandard for
       * why do not use a metric system of time.)
       */
      Metric,

      /**
       * It turns out we occasionally need to support and classify "other" metric units.  In particular, BeerJSON
       * supports both millipascal-seconds and centipoise as viscosity measurements.  Neither of these is an SI unit,
       * so a "MetricSi" category wouldn't help us.  Instead we have this slightly ugly catch-all for "metric systems we
       * might need to deal with but which we don't use"!
       */
      MetricAlternate,

      /**
       * There isn't an obviously sensible system of measurement for duration because, strictly speaking:
       *   • the metric system only measures time in seconds, decaseconds and such.
       *   • Coordinated Universal Time (aka UTC) defines time of day but not duration
       * Of course we know we only want to measure duration in seconds/minutes/hours/etc, so we'll just call that
       * "Universal Standard" and be done with it, especially as it's not something we're going to offer the user a
       * choice about.
       *
       * We'll also use this pseudo system of measurement for all other cases where we are not going to offer the user
       * a choice of systems of measurement - eg acidity is always measured in pH (at least in all the circumstances we
       * care about).
       */
      UniversalStandard,

      //
      // General systems of measurement do not tend to include measures of:
      //   • beer color
      //   • relative density
      //   • diastatic power (aka a "[measure] of a malted grain’s enzymatic content" according to
      //     https://blog.homebrewing.org/what-is-diastatic-power-definition-chart/
      // So, for these things we'll just use the names of the various scales as the names of our pseudo systems of
      // measurement.
      //
      // .:TBD:. Should we have IBUs here too?
      //

      // Color
      StandardReferenceMethod,
      EuropeanBreweryConvention,
      Lovibond,

      // Density
      SpecificGravity,
      Plato,
      Brix,

      // Diastatic power
      Lintner,
      WindischKolbach,

      // Carbonation
      CarbonationVolumes,
      CarbonationMassPerVolume,

      // As explained in measurement/PhysicalQuantity.h, we combine "mass concentration" and "mass fraction" into one
      // grouping.  These are not generally the same thing, but, in the context of what we need as brewers, we can treat
      // them as measures between which we can convert formulaically.
      BrewingConcentration,

      // See comment in measurement/Unit.h about the different viscosity units

      // Specific Heat Capacity
      SpecificHeatCapacityCalories,
      SpecificHeatCapacityJoules,
      SpecificHeatCapacityBtus,

      // Heat Capacity
      HeatCapacityKilocalories, // NB Kilocalories not Calories - per comment in Unit.h
      HeatCapacityJoules,
      HeatCapacityBtus,
   };


   /**
    * \brief Return the (translated if necessary) name of a \c SystemOfMeasurement suitable for display to the user
    */
   QString getDisplayName(SystemOfMeasurement systemOfMeasurement);

   /**
    * \brief Return the fixed unique name of a \c SystemOfMeasurement suitable for storing in config files.  (Of course
    *        we could just store the raw int value, but that is less robust and makes debugging harder.)
    */
   QString getUniqueName(SystemOfMeasurement systemOfMeasurement);

   /**
    * \brief Get a \c SystemOfMeasurement from its unique name.  Useful for deserialising from config files etc.
    */
   std::optional<Measurement::SystemOfMeasurement> getFromUniqueName(QString const & uniqueName);
}

/**
 * \brief Convenience function for logging (output includes \c getUniqueName and \c getDisplayName in some wrapper
 *        text)
 */
template<class S>
S & operator<<(S & stream, Measurement::SystemOfMeasurement const systemOfMeasurement) {
   stream <<
      "SystemOfMeasurement #" << static_cast<int>(systemOfMeasurement) << ": " <<
      Measurement::getUniqueName(systemOfMeasurement) << " (" <<
      Measurement::getDisplayName(systemOfMeasurement) << ")";
   return stream;
}

#endif
