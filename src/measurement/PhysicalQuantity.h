/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/PhysicalQuantity.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
#ifndef MEASUREMENT_PHYSICALQUANTITY_H
#define MEASUREMENT_PHYSICALQUANTITY_H
#pragma once

#include <array>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <QString>

#include "utils/BtStringConst.h"
#include "utils/TypeTraits.h"

class EnumStringMapping;

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
    *       because they do not relate to physical quantities, eg date & time format and language choice do not fit well
    *       in here -- see \c NonPhysicalQuantity in \headerfile measurement/NonPhysicalQuantity.h
    */
   enum class PhysicalQuantity {
      // Elsewhere we use weight instead of mass because it's more idiomatic (despite being, strictly speaking, not the
      // same thing)
      Mass,

      Volume,

      // Currently used for a couple of equipment dimension measures
      Length,

      // This is not really a physical quantity.  However, in our domain, it makes life simpler for us to pretend that
      // it is.  This is because "mass", "volume" and "number of" are the three canonical ways of measuring ingredients.
      // Note that this _is_ allowed to be fractional because you might want to add 1½ cinnamon sticks or 2.5 packets of
      // yeast.  In reality, this means we store it in a double and, typically, would want to show the number to 1
      // decimal place.
      Count,

      Temperature,

      // Note this NOT dates or times of day but length-of-time or elapsed time, eg duration of a mash step, or how long
      // after the start of the boil to add something.  I was wondering whether to rename it LengthOfTime, or
      // TimeDuration or ElapsedTime or something, but all those names are slightly unsatisfactory (eg several sound
      // wrong for "when to add hops").  Since SI units talk about seconds as base unit of time, it doesn't seem crazy
      // to stick with just Time here.  If we ever (elsewhere) need time of day or something else, we'll pick TimeOfDay
      // or TimeStamp etc for that.
      Time,

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

      //
      // As explained at https://en.wikipedia.org/wiki/Concentration, there are several types of concentration,
      // including "mass concentration", which is expressed as mass-per-volume, and "volume concentration", which is
      // strictly-speaking a dimensionless number (because volume-per-volume cancels out) but is often expressed as
      // parts per million (or similar) or sometimes as a percentage.
      //
      // Additionally, there is "mass fraction" (see https://en.wikipedia.org/wiki/Mass_fraction_(chemistry)) and
      // "volume fraction" (see https://en.wikipedia.org/wiki/Volume_fraction), both of which are dimensionless (because
      // mass-per-mass and volume-per-volume cancel out).
      //
      // Volume fraction is, strictly, different than volume concentration because the former is measured before mixing
      // everything together and the latter afterwards.  So they are only the same in an "ideal solution", where the
      // volumes of the constituents are additive (the volume of the solution is equal to the sum of the volumes of its
      // ingredients).
      //
      // In BeerJSON, there is only ConcentrationType, and its units are "ppm", "ppb" and "mg/l".  Since the last of
      // these is mass concentration, we assume that the first two are mass fraction.  In the context of brewing, the
      // concentrations we are measuring are typically dilute aqueous solutions -- ie mostly water.  In that context,
      // it's approximately true that 1 mg/L mass concentration = 1 parts per million (ppm) mass fraction.  This is
      // because one liter of water weighs about a kilogram.  (Of course it actually depends on the water temperature
      // and pressure -- eg see table at https://en.wikipedia.org/wiki/Density#Water, but this is a reasonable
      // approximation.)
      //
      // So, we go with the flow and treat "ppm", "ppb" and "mg/l" as all measures of mass fraction or mass
      // concentration.  Moreover, we assume that, in the context of brewing software we can convert between mass
      // fraction and mass concentration per the formula above.  (We _could_ insist that mass fraction and mass
      // concentration are different things, but the implication of the BeerJSON unit groupings is that it's not what
      // users would want or expect.)
      //
      // Per https://en.wikipedia.org/wiki/Parts-per_notation, strictly speaking for a mass fraction, we should use
      // "mg/kg" as instead of the more ambiguous "ppm" and "μg/kg" instead of "ppb".  However, users are going to
      // expect to be able to type "ppm" and "ppb" as these are the more day-to-day terms used in brewing.  So, again we
      // take the pragmatic rather than pedantic route.
      //
      MassFractionOrConc,

      // Viscosity -- see https://en.wikipedia.org/wiki/Viscosity
      Viscosity,

      // Specific heat capacity -- see https://en.wikipedia.org/wiki/Specific_heat_capacity
      SpecificHeatCapacity,

      // Heat capacity -- see https://en.wikipedia.org/wiki/Heat_capacity -- equals Mass × Specific Heat Capacity
      HeatCapacity,

      // Specific volume (= the reciprocal of density) -- see https://en.wikipedia.org/wiki/Specific_volume
      SpecificVolume,

      // .:TBD:. Should we add Energy for PropertyNames::Recipe::calories (in which case, should canonical measure be
      //         Joules)?
   };

   /**
    * \brief Mapping between \c Measurement::PhysicalQuantity and string values suitable for logging or serialisation in
    *        the DB.
    *
    *        This can also be used to obtain the number of values of \c PhysicalQuantity, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   extern EnumStringMapping const physicalQuantityStringMapping;

   /**
    * \brief Localised names of \c Hop::Form values suitable for displaying to the end user
    */
   extern EnumStringMapping const physicalQuantityDisplayNames;

   /**
    * \brief Return the \c PersistentSettings name for looking up the display \c UnitSystem for the specified
    *        \c PhysicalQuantity
    */
   BtStringConst const & getSettingsName(Measurement::PhysicalQuantity const physicalQuantity);

   /**
    * \brief In a few cases, we want to be able to handle two or three different ways of measuring a thing (eg \c Mass,
    *        \c Volume and \c Count; or \c MassConcentration and \c VolumeConcentration).
    *
    *        We could try to do a lot of really clever compile-time stuff with flags and templates to cover all possible
    *        permutations.  However, I don't think the complexity would be justified, as there are actually very few
    *        cases:
    *          \c Mass || \c Volume (eg measurements of \c Fermentable and \c Hop)
    *          \c Mass || \c Volume || \c Count (eg measurements of \c Misc and \c Yeast)
    *
    *        Since it also seems unlikely that the number of cases will grow very much (even if we change our minds
    *        about distinguishing between Mass Concentration and Mass Fraction), I think this enum is simpler and
    *        sufficient (with the helper functions below).
    */
   enum class ChoiceOfPhysicalQuantity {
      Mass_Volume        ,
      Mass_Volume_Count  ,
   };

   /*!
    * \brief Mapping between \c Measurement::ChoiceOfPhysicalQuantity and string values suitable for logging (or
    *        serialisation, though this is not currently needed).
    */
   extern EnumStringMapping const choiceOfPhysicalQuantityStringMapping;

   /*!
    * \brief Localised names of \c Hop::Form values suitable for displaying to the end user
    */
   extern EnumStringMapping const choiceOfPhysicalQuantityDisplayNames;

   /**
    * \brief Of course, once we have \c Measurement::ChoiceOfPhysicalQuantity, we need a way to store either that or a
    *        \c Measurement::PhysicalQuantity.
    *
    *        (Note that, \c QuantityFieldType is one place we \b don't use this as we need to add a third possibility
    *        there of \c NonPhysicalQuantity.)
    */
   using PhysicalQuantities = std::variant<Measurement::PhysicalQuantity, Measurement::ChoiceOfPhysicalQuantity>;

   //
   // It's also useful to be able to template on "either PhysicalQuantity or ChoiceOfPhysicalQuantity".
   //
   // See comment in utils/TypeTraits.h for definition of CONCEPT_FIX_UP (and why, for now, we need it)
   template <typename T>
   concept CONCEPT_FIX_UP PhysicalQuantityTypes =
      std::same_as<T, Measurement::PhysicalQuantity> ||
      std::same_as<T, Measurement::ChoiceOfPhysicalQuantity>;

   /**
    * \brief For each set of alternates implied by a value of \c ChoiceOfPhysicalQuantity, there needs to be a default
    *        \c PhysicalQuantity that we assume in the absence of other information (eg to default construct a
    *        \c ConstrainedAmount).  When we know this at compile time, we can do it all through template
    *        specialisations.
    *
    *        Note that we have two template parameters, so we can handle cases where we are writing general "constrained
    *        to a particular PhysicalQuantity or ChoiceOfPhysicalQuantity" code.
    *
    *        Default case is for \c PhysicalQuantity; specialisations are for all \c ChoiceOfPhysicalQuantity
    *        possibilities.  Note that, because this is a function template, we are not allowed \b partial
    *        specialisations.
    */
   template<PhysicalQuantityTypes PQT, PQT const pqt> PhysicalQuantity defaultPhysicalQuantity();

   /**
    * \brief We also need to handle the case where things need to be resolved at run-time
    */
   PhysicalQuantity defaultPhysicalQuantity(ChoiceOfPhysicalQuantity const val);

   /**
    * \brief And for generic programming, it's helpful to be able to list all the \c PhysicalQuantity values
    *        corresponding to a \c ChoiceOfPhysicalQuantity
    */
   std::vector<PhysicalQuantity> const & allPossibilities(ChoiceOfPhysicalQuantity const val);
   std::vector<int> const & allPossibilitiesAsInt(ChoiceOfPhysicalQuantity const val);

   /**
    * \return \c true if \c physicalQuantity is a valid option for \c variantPhysicalQuantity, false otherwise
    */
   template<PhysicalQuantityTypes PQT, PQT const pqt> bool isValid(PhysicalQuantity const physicalQuantity);

   /**
    * \brief Alternate version for when both parameters are only known at run-time
    */
   bool isValid(ChoiceOfPhysicalQuantity const choiceOfPhysicalQuantity, PhysicalQuantity const physicalQuantity);

}

/**
 * \brief Convenience functions for logging
 */
/**@{*/
template<class S> S & operator<<(S & stream, Measurement::PhysicalQuantity         const val);
template<class S> S & operator<<(S & stream, Measurement::ChoiceOfPhysicalQuantity const val);

template<class S>
S & operator<<(S & stream, Measurement::PhysicalQuantities const & val) {
   if (std::holds_alternative<Measurement::PhysicalQuantity>(val)) {
      stream << "PhysicalQuantities:" << std::get<Measurement::PhysicalQuantity>(val);
   } else {
      Q_ASSERT(std::holds_alternative<Measurement::ChoiceOfPhysicalQuantity>(val));
      stream << "PhysicalQuantities:" << std::get<Measurement::ChoiceOfPhysicalQuantity>(val);
   }
   return stream;
}

/**@}*/
#endif
