/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/Unit.h is part of Brewtarget, and is copyright the following authors 2009-2023:
 *   • Jeff Bailey <skydvr38@verizon.net>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
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
#ifndef MEASUREMENT_UNIT_H
#define MEASUREMENT_UNIT_H
#pragma once

#include <functional>
#include <memory> // For PImpl
#include <optional>
#include <utility> // For std::pair

#include <QMultiMap>
#include <QObject>
#include <QString>

#include "measurement/Amount.h"
#include "measurement/PhysicalQuantity.h"
#include "utils/ObjectAddressStringMapping.h"

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
       *
       * .:TODO:. Although we already do case-insensitive matching for unit names, I think we could do more on having
       *          generous matching for the longer/more complex units (eg "c/g·C" and "mPa-s").  Even a knowledgeable
       *          user isn't necessarily going to guess the exact abbreviation we've used, so it would be better to have
       *          either a list of valid alternatives or, possibly, a list of regular expressions.
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
       * \brief Given a string of quantity plus optional units, this function breaks it down into the quantity (or 0.0
       *        if none was found) and the units (or "" if none was found).  This is a bit fiddly to get right (partly
       *        because of locale-specific thousands and decimal separators, and partly because unit names can contain
       *        symbols such as '/').  So we only want to do it in one place!
       */
      static std::pair<double, QString> splitAmountString(QString const & inputString, bool * ok = nullptr);

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
       * \brief In certain circumstances, we expect things to be in canonical units, so this is a useful function for
       *        checking or asserting that.
       */
      bool isCanonical() const;

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


   //! This alias makes things a bit more concise eg in \c ObjectStore
   using UnitStringMapping = ObjectAddressStringMapping<Unit>;

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

      // === Count ===
      extern Unit const numberOf;

      // === Temperature ===
      extern Unit const celsius;
      extern Unit const fahrenheit;

      // === Time ===
      extern Unit const minutes;
      extern Unit const weeks;
      extern Unit const days;
      extern Unit const hours;
      extern Unit const seconds;

      // === Color ===
      extern Unit const srm;
      extern Unit const ebc;
      extern Unit const lovibond;
      // == Density ===
      // Strictly speaking, Plato and Brix are not measures of density but of percentage sugar content, and we're
      // usually interested in density in order to get % sugar content to do calculations about how much sugar turned to
      // alcohol.  However, since our primary measurement (specific gravity) is of density, that's the physical quantity
      // under which we'll group all three units.
      extern Unit const specificGravity;
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
      // == Mass Fraction & Mass Concentration ==
      // See comment in measurement/PhysicalQuantity.h for why we combine "mass concentration" and "mass fraction" into
      // one grouping, why we treat 1 mg/L as equal to 1 ppm, and why we use the vernacular "ppm" and "ppb" instead of
      // the SI units "mg/kg" and "μg/kg".
      extern Unit const partsPerMillionMass;
      extern Unit const partsPerBillionMass;
      extern Unit const milligramsPerLiter;
      // == Viscosity ==
      // Per https://en.wikipedia.org/wiki/Viscosity#Measurement, the SI unit of dynamic viscosity is the newton-second
      // per square meter (N·s/m²), which is (by definition) equivalent to a pascal-second (Pa·s).
      //
      // An alternate metric unit is the poise (P), which is defined as:
      //   1 P = 0.1 m⁻¹·kg·s⁻¹ = 1 cm⁻¹·g ·s⁻¹
      //
      // So 1P = 0.1 Pa·s.
      //
      // In a number of applications, including brewing, it's more common to use centipoise (cP), where 1 cP = 0.01 P,
      // or millipascal-second (mPa·s), where 1 mPa·s = 0.001 Pa·s.  (Thus 1 cP = 1 mPa·s.)   This is because
      // because the viscosity of water at 20 °C is about 1 cP = 1 mPa·s.
      //
      // We always try to use metric units as our "canonical" ones.  We often prefer SI units, but not at the expense of
      // a more commonly-used metric alternative (eg we prefer Celsius over Kelvin).  For viscosity, we use centipoise
      // as canonical because (a) AFAICT it's slightly more commonly used in brewing and (b) it's shorter.
      //
      // See eg https://www.brewingwithbriess.com/blog/understanding-a-malt-analysis/ and
      // https://www.morebeer.com/articles/Understanding_Malt_Analysis_Sheets for reference to common use of
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
      //       amounts in the DB.   Also the "calories" version is what is used by BeerXML.
      //
      // BeerJSON also offers us BTU / (lb × °F).  Note however, that, because a British Thermal Unit (BTU) is defined
      // as "the amount of heat required to raise the temperature of one pound of water by one degree Fahrenheit", we
      // have 1 BTU/lb·F = 1 c/g·C.
      //
      // NOTE that you often see a couple of other terms used in brewing-related circles:
      //
      //   - "Heat Capacity" (see https://en.wikipedia.org/wiki/Heat_capacity) is different from Specific Heat Capacity.
      //     For a given object (eg a brewing vessel), its Heat Capacity is obtained by multiplying its Mass by its
      //     Specific Heat Capacity.
      //
      //   - "Specific Heat" is just an abbreviation for "Specific Heat Capacity".  We use it for variable names etc in
      //     the interests of brevity, but we try always to use full "Specific Heat Capacity" for display strings to
      //     avoid any ambiguity or confusion.
      //
      extern Unit const caloriesPerCelsiusPerGram;
      extern Unit const joulesPerKelvinPerKg     ;
      extern Unit const btuPerFahrenheitPerPound ;
      // == Specific Volume ==
      // Per https://en.wikipedia.org/wiki/Specific_volume, specific volume is the reciprocal of density; in other words
      // it is "an intrinsic property of a substance, defined as the ratio of the substance's volume to its mass.  In
      // brewing, it's typically used to measure mash thickness.  The standard unit of specific volume is cubic meters
      // per kilogram (m³/kg), but there are various other metric combinations in widespread use.  We have historically
      // used litres per kilogram as our canonical units, and I see no pressing reason to change that.
      // We have a lot of units here because BeerJSON supports them all.  (I'm assuming the non-metric BeerJSON units
      // are US Customary rather than Imperial because I think the imperial versions are getting a bit obscure.)
      extern Unit const litresPerKilogram     ;
      extern Unit const litresPerGram         ;
      extern Unit const cubicMetersPerKilogram;
      extern Unit const us_quartsPerPound     ;
      extern Unit const us_gallonsPerPound    ;
      extern Unit const us_gallonsPerOunce    ;
      extern Unit const us_fluidOuncesPerOunce;
      extern Unit const cubicFeetPerPound     ;

      //================================================================================================================

      /**
       * \brief Serialisation of \c Unit IDs suitable for database storage -- eg maps between "kilograms" (untranslated)
       *        and \c Measurement::Units::kilograms.
       *
       *        We use this a little, but not a lot.  Normally, when something has only one way of being measured, we
       *        store in canonical units (eg °C for a temperature) and the unit names are a suffix of the DB column name
       *        (eg \c carbonationtemp_c).  However, when something can be measured more than one way (eg by mass or by
       *        volume) then we need to store more info.  We still always use canonical units (eg kilograms for mass and
       *        liters for volume) but we store a human-readable string of what the units are (eg in \c unit column of
       *        the \c fermentable_in_recipe table) so that units are obvious to someone looking at the DB without
       *        having to delve into the C++ code.  Amongst other things, this can help with bug reports -- eg a user
       *        might be able to tell us that data in the DB is wrong even though s/he is not familiar with the C++
       *        code.)
       */
      extern UnitStringMapping const unitStringMapping;
   }
}

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
