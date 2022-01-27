/*
 * measurement/SystemOfMeasurement.h is part of Brewtarget, and is copyright the following
 * authors 2022:
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
#ifndef MEASUREMENT_SYSTEMOFMEASUREMENT_H
#define MEASUREMENT_SYSTEMOFMEASUREMENT_H
#pragma once

#include <optional>

class QString;
class QTextStream;

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
       * This is similar to the "International System of Units (SI)" (which is derived from the original Metric system)
       * but is more practical for use by brewers.  It differs for instance in using Celsius for temperature rather than
       * Kelvin, and in including a volume scale (liters) which, strictly-speaking, SI does not.
       *
       * Covers Length, Area, Volume, Mass, Weight, Temperature and Time (duration) -- of which we use Volume, Mass and
       * Temperature
       */
      Metric,

      /**
       * There isn't an obviously sensible system of measurement for duration because, strictly speaking:
       *   • the metric system only measures time in seconds, decaseconds and such.
       *   • Coordinated Universal Time (aka UTC) defines time of day but not duration
       * Of course we know we only want to measure duration in seconds/minutes/hours/etc, so we'll just call that
       * "Standard" and be done with it, especially as it's not something we're going to offer the user a choice about.
       */
      StandardTimeUnits,

      //
      // General systems of measurement do not tend to include measures of:
      //   • beer color
      //   • relative density
      //   • diastatic power (aka a "[measure] of a malted grain’s enzymatic content" according to
      //     https://blog.homebrewing.org/what-is-diastatic-power-definition-chart/
      // So, for these things we'll just use the names of the various scales as the names of our pseudo systems of
      // measurement.
      //
      StandardReferenceMethod,
      EuropeanBreweryConvention,
      SpecificGravity,
      Plato,
      Lintner,
      WindischKolbach
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
