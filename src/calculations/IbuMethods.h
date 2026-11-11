/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * calculations/IbuMethods.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
 =====================================================================================================================*/
#ifndef CALCULATIONS_IBUMETHODS_H
#define CALCULATIONS_IBUMETHODS_H
#pragma once

#include <optional>
#include <vector>

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
    *        Tinseth, Rager, Noonan, Garetz and Daniels are long-established formulae.  mIBU and SMPH are more recent
    *        refinements.
    *
    *        The Tinseth, Rager and Garetz methods are explained and discussed at http://www.realbeer.com/hops/FAQ.html
    *        Garetz and Daniels (along with Tinseth, Rager, Noonan and Mosher) are summarised in Michael L. Hall, "What's
    *        Your IBU?", Zymurgy Special Edition, 1997.
    *
    *        The SMPH and mIBU methods are discussed by John-Paul Hosom at https://byo.com/article/ibu/ and, in more
    *        detail, at
    *         - https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *         - https://jphosom.github.io/alchemyoverlord/blog/11-a-summary-of-factors-affecting-ibus/alchemyoverlord-blog-content11.html
    *         - https://jphosom.github.io/alchemyoverlord/blog/31-ibus-and-the-smph-model/alchemyoverlord-blog-content31.html
    *
    *        For SMPH, the definitive reference is Hosom's own calculator, at
    *        https://jphosom.github.io/alchemyoverlord/ibu_SMPH.html, whose source is
    *        https://github.com/jphosom/alchemyoverlord/blob/master/js/ibu_SMPH.js.  It differs in a few places from the
    *        blog posts -- see comments in IbuMethods.cpp.
    */
   enum class IbuFormula {
      Tinseth,
      Rager  ,
      Noonan ,
      Garetz ,
      Daniels,
      mIbu   ,
      Smph   ,
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

   /**
    * \brief Same as \c formulaStringMapping but with use of upper case letters (eg "Tinseth" rather than "tinseth",
    *        "mIBU" rather than "mibu").
    *
    */
   extern EnumStringMapping const formulaStringMappingUc;

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

   /**
    * \brief Return the localised bitterness formula's name
    */
   QString localisedFormulaName();

   /**
    * \brief One segment of the profile of wort temperature after flameout.  Segments are consecutive, and the first one
    *        starts at flameout.  The caller (eg \c Recipe) builds these from the post-boil steps of the \c Boil, and is
    *        responsible for resolving unset step temperatures (eg per dotBeer, an unset boil step temperature means
    *        boiling).
    *
    *        Within a segment, the wort temperature is modelled in one of two ways:
    *
    *         - If \c naturalCooling is \c true, the wort cools naturally from \c startTemp_c, following John-Paul
    *           Hosom's exponential cooling curve (which depends on the kettle geometry and the wort volume), but does
    *           not drop below \c endTemp_c.  This is what we use for a step that starts at boiling and ends below it,
    *           which is how a "time before forced cooling" (aka "cool time") has always been represented in a boil
    *           profile -- see \c Boil::coolTime_mins.
    *
    *         - Otherwise, the temperature moves linearly from \c startTemp_c to \c endTemp_c over the segment.  So a
    *           segment with the same start and end temperatures is a hop stand / whirlpool held at that temperature,
    *           and a segment from, say, 80°C to 20°C is forced cooling.
    */
   struct PostBoilTemperatureSegment {
      double duration_minutes;
      double startTemp_c;
      double endTemp_c;
      bool   naturalCooling;
   };

   /**
    * \brief The forms of hops that the IBU formulae distinguish between.  This is deliberately simpler than
    *        \c Hop::Form (so we don't need to depend on the model here), and the caller does the mapping.
    *
    *         - \c Whole covers whole cones, including wet (fresh) hops
    *         - \c Plug is whole cones compressed into plugs
    *         - \c Pellet is standard (eg T-90) pellets
    *         - \c Powder is lupulin powder / concentrated pellets (eg Cryo)
    *         - \c Extract is CO₂ or similar extract
    */
   enum class HopForm {
      Whole  ,
      Plug   ,
      Pellet ,
      Powder ,
      Extract,
   };

   /**
    * \brief Yeast flocculation, simplified to the three levels the IBU formulae know about.
    */
   enum class YeastFlocculation {
      Low   ,
      Medium,
      High  ,
   };

   /**
    * \brief Parameters for the various IBU calculation formulae
    *
    * \param AArating decimal alpha-acid rating of the hops added in [0,1] (0.04 means 4% AA for example)
    * \param hops_grams - mass of hops in grams
    * \param postBoilVolume_liters - In some explanations, the phrase “finished volume of beer” is used, implying
    *        “volume into fermentor” ie after both boil-off _and_ any trub & chiller loss. However, the comments at
    *        https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *        say that Tinseth himself confirmed "Post boil volume is correct" because "We are concerned with the mg/L
    *        and any portions of a liter lost post boil doesn’t affect the calculation".
    * \param wortGravity_sg in specific gravity at around 60F I guess.
    * \param timeInBoil_minutes - minutes that the hops are in the boil: usually measured from the point at which hops
    *                             are added until flameout
    *
    * \param coolTime_minutes - How long, after flameout (or, for post-boil additions, after the hops are added) the
    *        hops are in hot wort.  Only used by formulae that model post-boil isomerization (mIbu and SMPH).
    *        \c std::nullopt means the caller doesn't know, and it is up to each formula to decide what to assume.
    *        (Per https://alchemyoverlord.wordpress.com/2015/05/12/a-modified-ibu-measurement-especially-for-late-hopping/
    *        if we quickly cool the wort at flameout, then the cool time is zero and the mIbu method gives the same
    *        results as the original Tinseth formula, so this is what mIbu assumes when the cool time is not known.)
    *        If \c postBoilTemperatureProfile is set, this will typically be the time to the end of that profile, but
    *        it does not have to be.  Beyond the end of the profile, we assume natural cooling.
    * \param kettleInternalDiameter_cm - (Only used in mIbu)  This is the interior diameter of the kettle at the surface
    *                                    of the wort, and is used to calculate the surface area of wort exposed to air.
    * \param kettleOpeningDiameter_cm - (Only used in mIbu)  This is the interior diameter of the opening in the kettle
    *                                   through which steam can escape, and is used to calculate the surface area of
    *                                   that same opening.
    * \param postBoilDelay_minutes - (Only used in mIbu)  For hops added after flameout (eg whirlpool or hop stand
    *                                additions), how many minutes after flameout they are added.  In this case,
    *                                \c timeInBoil_minutes should be 0 and \c coolTime_minutes should be the time from
    *                                when the hops are added until forced cooling.  The delay matters because the wort
    *                                will already have cooled somewhat by the time the hops are added.
    * \param postBoilAddition - \c true if the hops are added after flameout (eg whirlpool or hop stand additions).
    *                           Formulae that do not model post-boil isomerization give no IBUs for such additions.
    *                           (Note that this is not the same as \c timeInBoil_minutes being 0 -- eg Rager's formula
    *                           gives some IBUs for a hop added right at the end of the boil.)
    * \param postBoilTemperatureProfile - (Only used in mIbu and SMPH)  Wort temperature profile after flameout -- see
    *                                     \c PostBoilTemperatureSegment.  If empty, we assume natural cooling from
    *                                     flameout, per Hosom's model.
    *
    * The remaining parameters are optional, and are used only by some formulae.  Where a formula needs one that isn't
    * set, it falls back to a documented default.
    *
    * \param hopForm - Used by Garetz (pellet factor), Daniels (separate utilization table for pellets) and SMPH (pellets
    *                  produce about twice as many oxidized alpha acids during the boil as cones).  For all other
    *                  formulae, we apply Brewken's long-standing generic adjustment (+10% for pellets, +2% for plugs).
    *                  If not set, we assume whole hops.
    * \param betaAcidRating - (Only used in SMPH)  Decimal beta-acid rating of the hops in [0,1].  If not set, we ignore
    *                         oxidized beta acids.  (In SMPH, oxidized beta acids only come from the beta acids lost
    *                         during storage, so they are zero for well-preserved hops, which is what we currently
    *                         assume -- see comments in IbuMethods.cpp.)
    * \param sixMonthAlphaLoss - (Only used in SMPH)  Decimal proportion of alpha acids lost after six months' storage at
    *                            room temperature (aka "Hop Stability Index" in BeerXML), from which we get the Garetz
    *                            hop decay rate constant.  If not set, we use 0.32, which Hosom's calculator uses as the
    *                            average over many varieties.
    * \param polyphenolRating - (Only used in SMPH)  Decimal polyphenol content of the hops in [0,1].  If not set, we
    *                           use Hosom's suggested 0.04.
    * \param originalGravity_sg - (Used in Garetz and SMPH)  If not set, we use \c wortGravity_sg.
    * \param preBoilVolume_liters - (Only used in Garetz)  Volume at the start of the boil, used for the "concentration
    *                               factor".  If not set, we assume no concentration during the boil.
    * \param kettleVolumeAtFlameout_liters - (Used in Garetz and SMPH)  Wort volume in the kettle at the end of the
    *                                        boil.  If not set, we use \c postBoilVolume_liters.
    * \param kettleDissolvedAlphaAcids_ppm - (Only used in SMPH)  Concentration of dissolved (and not yet isomerized)
    *                                        alpha acids already in the kettle, from earlier additions, immediately
    *                                        before these hops are added.  This is used for SMPH's alpha-acid solubility
    *                                        limit, and the caller can get it from
    *                                        \c smphDissolvedAlphaAcidsBeforeAdditions_ppm.  If not set, we assume
    *                                        there are none (ie this is the only kettle addition).
    * \param kettleVolumeAtAddition_liters - (Only used in SMPH)  Wort volume in the kettle at the time these hops are
    *                                        added, used for SMPH's alpha-acid solubility limit.  If not set, we use
    *                                        \c kettleVolumeAtFlameout_liters.
    * \param hoppingRateFactor - (Used in Garetz and Daniels)  The Garetz "hopping rate factor", which depends on the
    *                            total IBUs of the recipe -- see \c garetzHoppingRateFactor.  The caller is responsible
    *                            for working this out (because it depends on all the hop additions in the recipe).
    * \param postBoilPh - (Only used in SMPH)  pH of the wort at the end of the boil.  If not set, we use 5.75, the
    *                     value at which SMPH's pH loss factors are 1.0 (and at which it assumes there has been no pH
    *                     adjustment, which matters for malt polyphenols -- see \c recipeLevelIbus).
    * \param yeastFlocculation - (Only used in SMPH)  If not set, we assume medium.
    * \param boilingPoint_c - (Only used in SMPH)  The temperature at which the wort boils, which is lower at altitude
    *                         (see \c Equipment::boilingPoint_c).  This matters because SMPH models isomerization of
    *                         alpha acids (via Malowicki's temperature-dependent rate constants), whereas the other
    *                         formulae are based on measurements made at (or near) sea level.  If not set, we use 100°C.
    */
   struct IbuCalculationParms {
      double AArating;
      double hops_grams;
      double postBoilVolume_liters;
      double wortGravity_sg;
      double timeInBoil_minutes;   // Seems unlikely in reality that there would be fractions of a minute
      std::optional<double> coolTime_minutes          = std::nullopt;
      std::optional<double> kettleInternalDiameter_cm = std::nullopt;
      std::optional<double> kettleOpeningDiameter_cm  = std::nullopt;
      double                postBoilDelay_minutes     = 0.0;
      bool                  postBoilAddition          = false;
      std::vector<PostBoilTemperatureSegment> postBoilTemperatureProfile = {};
      std::optional<HopForm>           hopForm                       = std::nullopt;
      std::optional<double>            betaAcidRating                = std::nullopt;
      std::optional<double>            sixMonthAlphaLoss             = std::nullopt;
      std::optional<double>            polyphenolRating              = std::nullopt;
      std::optional<double>            originalGravity_sg            = std::nullopt;
      std::optional<double>            preBoilVolume_liters          = std::nullopt;
      std::optional<double>            kettleVolumeAtFlameout_liters = std::nullopt;
      std::optional<double>            kettleDissolvedAlphaAcids_ppm = std::nullopt;
      std::optional<double>            kettleVolumeAtAddition_liters = std::nullopt;
      double                           hoppingRateFactor             = 1.0;
      std::optional<double>            postBoilPh                    = std::nullopt;
      std::optional<YeastFlocculation> yeastFlocculation             = std::nullopt;
      std::optional<double>            boilingPoint_c                = std::nullopt;
   };

   /**
    * \brief IBUs from a hop addition, split between:
    *          - \c boil_ibus: isomerized alpha acids (IAA) from isomerization during the boil proper
    *          - \c postBoil_ibus: IAA from isomerization after flameout (eg during a whirlpool or hop stand, or simply
    *            while the wort is cooling).  Formulae that do not model post-boil isomerization always give 0 for this.
    *          - \c nonIaa_ibus: "auxiliary bittering compounds" (oxidized alpha and beta acids, and polyphenols), which
    *            dissolve quickly rather than needing isomerization.  Only SMPH models these separately -- other
    *            formulae fold them into their utilization curves, and give 0 for this.
    */
   struct IbuBreakdown {
      double boil_ibus     = 0.0;
      double postBoil_ibus = 0.0;
      double nonIaa_ibus   = 0.0;
      double total() const { return this->boil_ibus + this->postBoil_ibus + this->nonIaa_ibus; }
      IbuBreakdown & operator*=(double const factor) {
         this->boil_ibus     *= factor;
         this->postBoil_ibus *= factor;
         this->nonIaa_ibus   *= factor;
         return *this;
      }
   };

   /**
    * \return \c true if \c ibuFormula models isomerization after flameout (and therefore gives IBUs for whirlpool, hop
    *         stand etc additions), \c false otherwise.
    */
   bool modelsPostBoilIsomerization(IbuFormula const ibuFormula);

   /**
    * \return \c true if \c ibuFormula uses the Garetz hopping rate factor, in which case the caller needs to supply
    *         \c IbuCalculationParms::hoppingRateFactor -- see \c garetzHoppingRateFactor.
    */
   bool usesHoppingRateFactor(IbuFormula const ibuFormula);

   /**
    * \brief The Garetz "hopping rate factor", HF = 1 + (CF × IBU / 260), where CF is the concentration factor (final
    *        volume / pre-boil volume) and IBU is the total ("desired") IBUs of the recipe.  Because the total IBUs
    *        themselves depend on HF (since every hop addition is divided by it), we solve for HF directly: if U is the
    *        total IBUs \b before dividing by HF, then IBU = U / HF, so HF satisfies HF² - HF - (CF × U / 260) = 0.  (This
    *        is the same as the "simplified" form of the hopping rate factor described by Michael Hall.)
    *
    * \param totalIbusBeforeHoppingRateFactor Sum of the IBUs of all hop additions, calculated with a hopping rate
    *                                         factor of 1.0
    * \param concentrationFactor Final (post-boil) volume divided by pre-boil volume
    */
   double garetzHoppingRateFactor(double const totalIbusBeforeHoppingRateFactor, double const concentrationFactor);

   /**
    * \brief Some formulae give some IBUs that do not come from any particular hop addition.  (Currently, this is only
    *        SMPH, which includes the contribution of malt polyphenols.)
    *
    * \param originalGravity_sg
    * \param postBoilPh - (Only used in SMPH)  As \c IbuCalculationParms::postBoilPh.  Malt-derived IBUs increase as the
    *                     pH is lowered from 5.75 (which SMPH takes to be the pH before any adjustment).
    */
   double recipeLevelIbus(double const originalGravity_sg, std::optional<double> const postBoilPh = std::nullopt);

   /**
    * \brief A kettle (ie boil, including first wort) hop addition, as far as SMPH's alpha-acid solubility limit is
    *        concerned -- see \c smphDissolvedAlphaAcidsBeforeAdditions_ppm.
    */
   struct KettleAlphaAcidAddition {
      //! How many minutes before flameout the hops are added (negative for post-boil additions)
      double minutesBeforeFlameout;
      //! Mass of alpha acids added
      double alphaAcids_mg;
      //! Volume of wort in the kettle when the hops are added
      double kettleVolume_liters;
   };

   /**
    * \brief For SMPH's alpha-acid solubility limit, work out, for each of the kettle hop additions in a recipe, the
    *        concentration of dissolved alpha acids already in the kettle immediately before it is added (to pass in
    *        \c IbuCalculationParms::kettleDissolvedAlphaAcids_ppm).
    *
    *        Per Hosom's calculator, we go through the additions in time order.  Each addition dissolves as much as the
    *        solubility limit allows on top of what is already dissolved, and, between additions, the dissolved alpha
    *        acids are depleted by isomerization (at a rate that depends on the wort temperature).  Additions made at
    *        the same time are taken in the order given.
    *
    * \param additions All the kettle hop additions in the recipe.  The order only matters for additions made at the
    *                  same time.
    * \param commonParms Used for the post-boil temperature profile, and kettle dimensions and volume for natural
    *                    cooling.
    *
    * \return Concentrations (ppm), in the same order as \c additions
    */
   std::vector<double> smphDissolvedAlphaAcidsBeforeAdditions_ppm(std::vector<KettleAlphaAcidAddition> const & additions,
                                                                  IbuCalculationParms const & commonParms);

   /**
    * \return IBUs according to selected algorithm, split between boil and post-boil.
    */
   IbuBreakdown getIbuBreakdown(IbuCalculationParms const & parms);

   /**
    * \return IBUs according to selected algorithm.
    */
   double getIbus(IbuCalculationParms const & parms);

   /**
    * \brief Wort temperature a given time after flameout.  Exposed mainly for unit testing.
    *
    * \param minutesSinceFlameout
    * \param profile See \c PostBoilTemperatureSegment.  Beyond the end of the profile (including when the profile is
    *                empty), we assume natural cooling from whatever the temperature was at the end of the profile
    *                (or from boiling if the profile is empty).
    * \param naturalCoolingRate_perMinute The \c b parameter of Hosom's natural cooling curve -- see
    *                                     \c naturalCoolingRate.
    * \param boilingPoint_c The temperature at which the wort boils.  The wort cannot be hotter than this (so, eg, a
    *                       profile segment "at boiling" is at this temperature), and, at flameout, natural cooling
    *                       starts from it.  Per Hosom's calculator, below 100°C, the natural cooling curve is the same
    *                       shape, just shifted down by the difference.
    */
   double postBoilWortTemperature_c(double const minutesSinceFlameout,
                                    std::vector<PostBoilTemperatureSegment> const & profile,
                                    double const naturalCoolingRate_perMinute,
                                    double const boilingPoint_c = 100.0);

   /**
    * \brief The \c b parameter of Hosom's natural cooling curve.  Exposed mainly for unit testing.
    */
   double naturalCoolingRate(double const postBoilVolume_liters,
                             double const kettleInternalDiameter_cm,
                             double const kettleOpeningDiameter_cm);

   /**
    * \brief Hosom's relative rate of alpha acid isomerization at a given temperature, compared with at boiling.
    *        Exposed mainly for unit testing.
    */
   double relativeUtilization(double const temp_c);
}

//! \brief Convenience function for logging
template<class S> S & operator<<(S & stream, IbuMethods::IbuCalculationParms const & parms) {
   stream <<
      "[AA Rating:" << parms.AArating << "; Hops (g):" << parms.hops_grams << "Post-boil volume (l):" <<
      parms.postBoilVolume_liters << "; Wort Gravity (SG):" << parms.wortGravity_sg << "; Time in Boil (mins):" <<
      parms.timeInBoil_minutes << "; Cool Time (mins):" << parms.coolTime_minutes << "; Kettle Int Diameter (cm):" <<
      parms.kettleInternalDiameter_cm << "; Kettle Opening (cm):" << parms.kettleOpeningDiameter_cm <<
      "; Post-boil delay (mins):" << parms.postBoilDelay_minutes << "; Post-boil addition:" <<
      parms.postBoilAddition << "; Hop form:" << (parms.hopForm ? static_cast<int>(*parms.hopForm) : -1) <<
      "; Beta:" << parms.betaAcidRating << "; Six-month AA loss:" << parms.sixMonthAlphaLoss << "; Polyphenols:" <<
      parms.polyphenolRating << "; OG:" << parms.originalGravity_sg << "; Pre-boil volume (l):" <<
      parms.preBoilVolume_liters << "; Kettle volume at flameout (l):" << parms.kettleVolumeAtFlameout_liters <<
      "; Kettle dissolved AA (ppm):" << parms.kettleDissolvedAlphaAcids_ppm << "; Kettle volume at addition (l):" <<
      parms.kettleVolumeAtAddition_liters << "; Hopping rate factor:" << parms.hoppingRateFactor <<
      "; Post-boil pH:" << parms.postBoilPh << "; Flocculation:" <<
      (parms.yeastFlocculation ? static_cast<int>(*parms.yeastFlocculation) : -1) << "; Boiling point (°C):" <<
      parms.boilingPoint_c << "; Post-boil profile:";
   for (auto const & segment : parms.postBoilTemperatureProfile) {
      stream <<
         " (" << segment.duration_minutes << " mins, " << segment.startTemp_c << "°C -> " << segment.endTemp_c <<
         "°C" << (segment.naturalCooling ? " natural" : "") << ")";
   }
   stream << "]";
   return stream;
}

#endif