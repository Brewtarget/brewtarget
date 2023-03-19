/*
 * measurement/Unit.h is part of Brewtarget, and is copyright the following
 * authors 2009-2023:
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Mark de Wever <koraq@xs4all.nl>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
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
#ifndef MEASUREMENT_UNIT_H
#define MEASUREMENT_UNIT_H
#pragma once

#include <functional>
#include <memory> // For PImpl
#include <optional>

#include <QMultiMap>
#include <QObject>
#include <QString>

#include "measurement/Amount.h"
#include "measurement/PhysicalQuantity.h"

// TODO: implement ppm, percent, ibuGalPerLb,

namespace Measurement {
   class UnitSystem;

   /*!
    * \class Unit
    *
    * \brief Interface for arbitrary physical units and their formatting.
    */
   class Unit {

   public:
      /**
       * \brief Construct a type of unit.  Note that it is \b not intended that users of this class construct their own
       *        \c Unit objects.  Rather they should use pointers or references to the constants defined in the \c Units
       *        namespace.
       *
       * \param unitSystem The \c UnitSystem to which this \c Unit belongs.  Amongst other things, this tells us which
       *                   \c PhysicalQuantity this \c Unit relates to.  (See comment in
       *                   \c measurement/PhysicalQuantity.h for more details on the relationship between classes in the
       *                   \c Measurement namespace.)
       * \param unitName This is singular of the commonly used abbreviation for this unit, eg, in English, "kg" for
       *                 kilograms, "tsp" for teaspoons. Note that this needs to be unique within the \c UnitSystem to
       *                 which this \c Unit belongs but is \c not necessarily globally unique, eg "qt" refers to both
       *                 Imperial quarts and US Customary quarts; "L" refers to liters and Lintner.
       * \param convertToCanonical Converts a quantity of this \c Unit to a quantity of \c canonical \c Unit
       * \param convertFromCanonical  Converts a quantity of \c canonical \c Unit to a quantity of this \c Unit
       * \param boundaryValue
       * \param canonical The canonical units we use for \c PhysicalQuantity this \c Unit relates to.  \c nullptr means
       *                  this \c Unit is the canonical one (and therefore \c convertToCanonical and
       *                  \c convertFromCanonical are no-ops).  (Note that the canonical units may or may not be in the
       *                  same \c UnitSystem as this \c Unit.  Eg canonical units for mass are kilograms so there's a
       *                  conversion to do whether you're starting from pounds, grams, ounces or milligrams.)
       */
      Unit(UnitSystem const & unitSystem,
           QString const unitName,
           std::function<double(double)> convertToCanonical,
           std::function<double(double)> convertFromCanonical,
           double boundaryValue,
           Unit const * canonical = nullptr);

      ~Unit();

      /**
       * \brief This gets called by \c getUnit, \c getCanonicalUnit and \c convertWithoutContext to ensure their lookup
       *        maps are initialised.
       *
       *        It would be private, except we need to call it from an anonymous namespace function in
       *        \c measurement/Unit.cpp.
       *
       *        It would be an anonymous namespace function itself, except it needs access to private members of
       *        \c Unit.
       */
      static void initialiseLookups();

      /**
       * \brief Test whether two \c Unit references are the same.  (This is by no means a full test for equality,
       *        since we assume there is only one, constant, instance of each different \c Unit.
       */
      bool operator==(Measurement::Unit const & other) const;

      /**
       * \brief The unit name will be the singular of the commonly used abbreviation.
       */
      QString const name;

      /**
       * \brief Returns the canonical units we use for \c PhysicalQuantity this \c Unit relates to.  These are the units
       *        we use for internal storage and (for the most part) for calculations.
       */
      Measurement::Unit const & getCanonical() const;

      /**
       * \brief Convert an amount of this unit to its canonical system of measurement (usually, but not always, an SI or
       *        other metric measure)
       */
      Measurement::Amount toCanonical(double amt) const;

      /**
       * \brief Convert an amount of this unit from its canonical system of measurement (usually, but not always, an SI
       *        or other metric measure)
       */
      double fromCanonical(double amt) const;

      /**
       * \brief Returns the \c Measurement::PhysicalQuantity that this \c Measurement::Unit measures.  This is a
       *        convenience function to save you having to first get the \c Measurement::UnitSystem.
       */
      Measurement::PhysicalQuantity getPhysicalQuantity() const;

      /**
       * \brief Returns the \c Measurement::UnitSystem to which this \c Measurement::Unit belongs.
       */
      Measurement::UnitSystem const & getUnitSystem() const;

      /**
       * \brief Used by \c UnitSystem
       *
       *        Returns the threshold below which a smaller unit (of the same type) should be used.  Normally it's 1, eg a
       *        length of time less than a minute should be shown in seconds.  But it can be larger, eg we show minutes for
       *        any length of time below 2 hours.  And it can be smaller, eg a US/imperial volume measure can be as small
       *        as a quarter of cup before we drop down to showing tablespoons.
       */
      double boundary() const;

      /**
       * \brief This mostly gets called when the unit entered in the field does not match what the field has been set
       *        to.  For example, if you displaying in Liters, but enter "20 qt". Since the SIVolumeUnitSystem doesn't
       *        know what "qt" is, we go searching for it.
       *
       * \param name
       * \param physicalQuantity Caller supplies this to help with disambiguation (eg between Liters and Lintner, both
       *                         of which have name/abbreviation "L").
       * \param caseInensitiveMatching If \c true (the default), this means we'll do a case-insensitive search.  Eg,
       *                               we'll match "ml" for milliliters, even though the correct name is "mL".  This
       *                               should always be safe to do, as AFAICT there are no current or foreseeable units
       *                               that _we_ use whose names only differ by case.
       *
       * \return \c nullptr if no sane match could be found
       */
      static Unit const * getUnit(QString const & name,
                                  Measurement::PhysicalQuantity const & physicalQuantity,
                                  bool const caseInensitiveMatching = true);

      /**
       * \brief Try to find a Unit by name in the supplied UnitSystem.  If no unit is found, search against the
       *        PhysicalQuantity to which the supplied UnitSystem relates (which is doable because, per the comment in
       *        measurement/UnitSystem.h, we each UnitSystem relates to a single PhysicalQuantity).
       *
       * \param name
       * \param unitSystem
       * \param caseInensitiveMatching If \c true (the default), this means we'll do a case-insensitive search.  Eg,
       *                               we'll match "ml" for milliliters, even though the correct name is "mL".  This
       *                               should always be safe to do, as AFAICT there are no current or foreseeable units
       *                               that _we_ use whose names only differ by case.
       *
       * \return \c nullptr if no sane match could be found
       */
      static Unit const * getUnit(QString const & name,
                                  Measurement::UnitSystem const & unitSystem,
                                  bool const caseInensitiveMatching = true);

      /**
       * \brief Get the canonical \c Unit for a given \c PhysicalQuantity.  This will be the unit we use for storing
       *        amounts of this type in the database - eg we always store volumes in liters and mass in kilograms.
       *
       *        Note that is not meaningful or permitted to call this function with
       *        \c Measurement::PhysicalQuantity::Mixed as a parameter.
       */
      static Unit const & getCanonicalUnit(Measurement::PhysicalQuantity const physicalQuantity);

      /**
       * \brief Used by \c ConverterTool to do contextless conversions - ie where we don't know what \c PhysicalQuantity
       *        we are dealing with because it's a generic tool to allow the user to convert "3 qt" to liters or "5lb"
       *        to kilograms etc.
       */
      static QString convertWithoutContext(QString const & qstr, QString const & toUnitName);

   private:
      // Private implementation details - see https://herbsutter.com/gotw/_100/
      class impl;
      std::unique_ptr<impl> pimpl;

      //! No copy constructor, as never want anyone, not even our friends, to make copies of a singleton
      Unit(Unit const&) = delete;
      //! No assignment operator , as never want anyone, not even our friends, to make copies of a singleton.
      Unit& operator=(Unit const&) = delete;
      //! No move constructor
      Unit(Unit &&) = delete;
      //! No move assignment
      Unit & operator=(Unit &&) = delete;
   };

   namespace Units {
      // === Mass ===
      extern Unit const kilograms;
      extern Unit const grams;
      extern Unit const milligrams;
      extern Unit const pounds;
      extern Unit const ounces;
      extern Unit const imperial_pounds; // Same as pounds
      extern Unit const imperial_ounces; // Same as ounces
      // === Volume ===
      extern Unit const liters;
      extern Unit const milliliters;
      extern Unit const us_barrels;     // =  31 × us_gallons (for beer; different for other things!)
      extern Unit const us_gallons;     // =   4 × us_quarts
      extern Unit const us_quarts;      // =   2 × us_pints
      extern Unit const us_pints;       // =  16 × us_fluidOunces
      extern Unit const us_cups;        // =   8 × us_fluidOunces
      extern Unit const us_fluidOunces;
      extern Unit const us_tablespoons; // = 1/2 × us_fluidOunces
      extern Unit const us_teaspoons;   // = 1/6 × us_fluidOunces
      extern Unit const imperial_barrels;     // = 36 × imperial_gallons
      extern Unit const imperial_gallons;     // =  4 × imperial_quarts
      extern Unit const imperial_quarts;      // =  2 × imperial_pints
      extern Unit const imperial_pints;       // = 20 × imperial_fluidOunces
      extern Unit const imperial_cups;        // = 10 × imperial_fluidOunces
      extern Unit const imperial_fluidOunces;
      extern Unit const imperial_tablespoons;
      extern Unit const imperial_teaspoons;
      // === Time ===
      extern Unit const minutes;
      extern Unit const weeks;
      extern Unit const days;
      extern Unit const hours;
      extern Unit const seconds;
      // === Temperature ===
      extern Unit const celsius;
      extern Unit const fahrenheit;
      // === Color ===
      extern Unit const srm;
      extern Unit const ebc;
      extern Unit const lovibond;
      // == Density ===
      // Strictly speaking, Plato and Brix are not measures of density but of percentage sugar content, and we're
      // usually interested in density in order to get % sugar content to do calculations about how much sugar turned to
      // alcohol.  However, since our primary measurement (specific gravity) is of density, that's the physical quantity
      // under which we'll group all three units.
      extern Unit const sp_grav;
      extern Unit const plato;
      extern Unit const brix;
      // == Diastatic power ==
      extern Unit const lintner;
      extern Unit const wk;
      // == Acidity ==
      // Because it's such a universal unit, in most of the code we use a pH number directly without going via this Unit
      // object, but it's here for completeness and possible future use.
      extern Unit const pH;
      // == Bitterness ==
      extern Unit const ibu;
      // == Carbonation ==
      extern Unit const carbonationVolumes;
      extern Unit const carbonationGramsPerLiter;
      // == Concentration ==
      extern Unit const milligramsPerLiter;
      extern Unit const partsPerMillion;
      extern Unit const partsPerBillion;
      // == Viscosity ==
      // Per https://en.wikipedia.org/wiki/Viscosity#Measurement, the SI unit of dynamic viscosity is the newton-second
      // per square meter (N·s/m2), which is (by definition) equivalent to a pascal-second (Pa·s).  An alternate metric
      // unit is the poise (P), which is g·cm−1·s−1.  So 1P = 0.1 Pa·s.  More commonly used, including in brewing, is
      // the centipoise (cP), which is convenient because the viscosity of water at 20 °C is about 1 cP.  One centipoise
      // is equal to one millipascal-second (mPa·s).
      //
      // See eg https://www.brewingwithbriess.com/blog/understanding-a-malt-analysis/ for reference to common use of
      // centipoise as viscosity measurement in the brewing industry.
      //
      // (US Customary and Imperial offer us pound-seconds per square foot (lb·s/ft2), but thankfully this does not seem
      // to be much used in brewing, so we do not implement it.)
      extern Unit const centipoise;
      extern Unit const millipascalSecond;
      // == Specific Heat Capacity ==
      // Per https://en.wikipedia.org/wiki/Specific_heat_capacity SI units are "joules per kelvin per kilogram" (which
      // is the same as "joules per degree Celsius per kilogram").  However, historically a measurement involving
      // calories instead of joules was used in chemistry, nutrition and, it seems, brewing.  There are two types of
      // calorie:
      //   - the "small calorie" (or "gram-calorie", "cal") = 4.184 J
      //   - The "grand calorie" (aka "kilocalorie", "kcal" or "Cal") = 1000 small calories = 4184 J
      //
      // However, the specific heat measurement using "cal" is the same as that for "Cal":
      //   1 cal / (°C × g) = 1 Cal / (°C × kg) = 4184 J / (°K × kg) = the specific heat capacity of liquid water
      //
      // So, we only implement "calories per degree Celsius per gram" as it's identical to "kilocalories per degree
      // Celsius per kilogram".
      //
      // NOTE: This is one instance where our "canonical" unit is NOT the metric one.  Historically, the code has always
      //       used "calories per Celsius per gram" rather than "joules per Celsius per kilogram", including for storing
      //       amounts in the DB.   Also the "calories" version is what is used by BeerJSON and BeerXML.
      //
      extern Unit const caloriesPerCelsiusPerGram;
      extern Unit const joulesPerKelvinPerKg;
   }
}

//.:TODO:.     "SpecificVolumeType": "Specific volume is the inverse of density, with units of volume over mass, ie qt/lb or L/kg. Commonly used for mash thickness.",

/**
 * \brief Convenience function to allow output of \c Measurement::Unit to \c QDebug or \c QTextStream stream etc
 */
template<class S>
S & operator<<(S & stream, Measurement::Unit const & unit) {
   stream << unit.name << " (" << unit.getPhysicalQuantity() << ")";
   return stream;
}
template<class S>
S & operator<<(S & stream, Measurement::Unit const * unit) {
   if (unit) {
      stream << *unit;
   } else {
      stream << "NULL";
   }
   return stream;
}

#endif
