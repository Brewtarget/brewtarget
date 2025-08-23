/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * measurement/IbuMethods.h is part of Brewtarget, and is copyright the following authors 2009-2025:
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
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
#ifndef MEASUREMENT_IBUMETHODS_H
#define MEASUREMENT_IBUMETHODS_H
#pragma once

#include "utils/BtStringConst.h"
#include "utils/EnumStringMapping.h"

class QString;

//======================================================================================================================
//========================================== Start of property name constants ==========================================
// See comment in model/NamedEntity.h
//
// Of course, since IbuMethods is a namespace rather than a class, it doesn't have properties in the same way that the
// model classes (Hop, Recipe, etc) do.  However, it useful in certain places (eg combo box set-up) to treat it as a
// sort of singleton class.  Hence the property names here.
//
#define AddPropertyName(property) namespace PropertyNames::IbuMethods { inline BtStringConst const property{#property}; }
AddPropertyName(formula)
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================

/*!
 * \namespace IbuMethods
 *
 * \brief Make IBU calculations.
 */
namespace IbuMethods {
   QString localisedName_formula();

   /**
    * \brief The formula used to get IBUs.
    *
    *        Tinseth, Rager and Noonan are long-established formulae.  mIBU and SMPH are more recent refinements.
    *
    *        The Tinseth, Rager and Garetz methods are explained and discussed at http://www.realbeer.com/hops/FAQ.html
    *
    *        The SMPH and mIBU methods are discussed by John-Paul Hosom at https://byo.com/article/ibu/ and, in more
    *        detail, at
    *         - https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *         - https://jphosom.github.io/alchemyoverlord/blog/31-ibus-and-the-smph-model/alchemyoverlord-blog-content31.html
    */
   enum class IbuFormula {
      Tinseth,
      Rager  ,
      Noonan ,
      mIbu   ,
//      Smph   , // Not yet implemented
   };
   // Note that we can't use the Q_ENUM macro here to allow storing the above enum class in a QVariant, because Q_ENUM
   // is designed to be used inside a class that derives from QObject.

   /*!
    * \brief Mapping between \c IbuMethods::IbuFormula and string values suitable for serialisation in
    *        \c PersistentSettings, etc.
    *
    *        This can also be used to obtain the number of values of \c Type, albeit at run-time rather than
    *        compile-time.  (One day, C++ will have reflection and we won't need to do things this way.)
    */
   extern EnumStringMapping const formulaStringMapping;

   /*!
    * \brief Localised names of \c IbuMethods::IbuFormula values suitable for displaying to the end user
    */
   extern EnumStringMapping const formulaDisplayNames;

   extern IbuFormula formula;

   extern TypeLookup const typeLookup;

   /**
    * \brief Read in from persistent settings
    */
   void loadFormula();

   /**
    * \brief Write out to persistent settings
    */
   void saveFormula();

   //! \brief return the bitterness formula's name
   QString formulaName();

   /**
    * \brief Parameters for the various IBU calculation formulae
    *
    * \param AArating decimal alpha-acid rating of the hops added in [0,1] (0.04 means 4% AA for example)
    * \param hops_grams - mass of hops in grams
    * \param postBoilVolume_liters - In some explanations, the phrase “finished volume of beer” is used, implying
    *        “volume into fermenter” ie after both boil-off _and_ any trub & chiller loss. However, the comments at
    *        https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *        say that Tinseth himself confirmed "Post boil volume is correct" because "We are concerned with the mg/L
    *        and any portions of a liter lost post boil doesn’t affect the calculation".
    * \param wortGravity_sg in specific gravity at around 60F I guess.
    * \param timeInBoil_minutes - minutes that the hops are in the boil: usually measured from the point at which hops
    *                             are added until flameout
    *
    * \param coolTime_minutes - (Only used in mIbu)  Time after flameout, without forced cooling.  Note per
    *        https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *        that if we quickly cool the wort at flameout, then coolTime_minutes should be zero and the mIbu method
    *        gives the same results as the original Tinseth formula.  Only when we have some time between flameout and
    *        forced cooling do we get additional IBUs.
    * \param kettleInternalDiameter_cm - (Only used in mIbu)  This is the interior diameter of the kettle at the surface
    *                                    of the wort, and is used to calculate the surface area of wort exposed to air.
    * \param kettleOpeningDiameter_cm - (Only used in mIbu)  This is the interior diameter of the opening in the kettle
    *                                   through which steam can escape, and is used to calculate the surface area of
    *                                   that same opening.
    */
   struct IbuCalculationParms {
      double AArating;
      double hops_grams;
      double postBoilVolume_liters;
      double wortGravity_sg;
      double timeInBoil_minutes;
      std::optional<double> coolTime_minutes          = std::nullopt;
      std::optional<double> kettleInternalDiameter_cm = std::nullopt;
      std::optional<double> kettleOpeningDiameter_cm  = std::nullopt;
   };

   /*!
    * \return IBUs according to selected algorithm.
    */
   double getIbus(IbuCalculationParms const & parms);
}

#endif
