/*======================================================================================================================
 * calculations/MashFermentability.h is part of Brewtarget, and is copyright the following authors 2026:
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
 =====================================================================================================================*/
#ifndef CALCULATIONS_MASHFERMENTABILITY_H
#define CALCULATIONS_MASHFERMENTABILITY_H
#pragma once

#include <optional>
#include <vector>

/**
 * \brief Estimates how the mash temperature profile (temperatures, rest times and ramps, over any number of steps)
 *        changes the fermentability of the wort, so that the predicted final gravity can take the mash into account.
 *        See \c Recipe::fgCalcUsesMash for where this is used.
 *
 *        BACKGROUND
 *        ----------
 *        A brewer chooses mash temperatures partly to control how much of the extract the yeast can ferment.  Two
 *        enzymes in malt do the work of turning starch into sugar:
 *           - β-amylase snips maltose (a fermentable sugar) off the ends of starch chains.  It is most active at about
 *             60–65 °C but is quickly destroyed by heat above that.
 *           - α-amylase cuts starch chains at random points, producing a mix of sugars and dextrins (chains too long
 *             for yeast to ferment).  It survives, and works fastest, at higher temperatures (up to about 72 °C).
 *        Starch also has to "gelatinise" (the granules swell and burst) before either enzyme can reach it, which
 *        happens mostly above about 60 °C.  So a low mash temperature (or a long rest at one) gives a more fermentable
 *        wort and a drier beer, and a high one gives a less fermentable wort and a fuller-bodied, sweeter beer.  With
 *        step mashes, what matters is the whole history of temperature against time, which is why we can't just use
 *        one "mash temperature".
 *
 *        THE MODEL
 *        ---------
 *        Rather than a rule of thumb, we use the published kinetic model from:
 *
 *           C. Brandam, X.M. Meyer, J. Proth, P. Strehaiano, H. Pingaud, "An original kinetic model for the enzymatic
 *           hydrolysis of starch during mashing", Biochemical Engineering Journal 13 (2003) 43–52.
 *           DOI 10.1016/S1369-703X(02)00100-6
 *
 *        This was developed with Kronenbourg's research centre, fitted to nine pilot-scale (90 kg) mashes of one malt,
 *        and then validated against mashes of five other malts.  Errors on total fermentable sugars were about 3% on
 *        average.  The model tracks, over time, the amounts (in g per kg of mash) of:
 *           - starch that has not yet gelatinised
 *           - gelatinised starch
 *           - dextrins (which yeast cannot ferment)
 *           - glucose, maltose and maltotriose (which, broadly, yeast can ferment)
 *        and the amounts of still-active α- and β-amylase.  The only thing that drives it is the temperature of the
 *        mash over time, which is exactly what a mash profile gives us.  The equations and constants are explained,
 *        with references to the paper, in MashFermentability.cpp.
 *
 *        HOW WE USE IT
 *        -------------
 *        We don't trust any model to predict the *absolute* fermentability of an arbitrary homebrew malt, and yeast
 *        attenuation figures already include the effect of whatever wort the yeast lab used to measure them.  So we
 *        use the model *relatively*:
 *
 *           adjusted attenuation = yeast attenuation × F(recipe's mash) / F(reference mash)
 *
 *        where F is the fermentable fraction of the wort extract at the end of the mash, and the reference mash is a
 *        typical single infusion (see \c referenceMashTemp_c).  Errors in the model's absolute calibration (and the
 *        uncertainty about exactly what malt the paper used) then largely cancel out, leaving just the model's
 *        prediction of how much more or less fermentable this mash is than a typical one.  Treating apparent
 *        attenuation as proportional to fermentable fraction is itself an approximation, but a reasonable one over
 *        the range of adjustments that come out of this (roughly ±15%).
 *
 *        As well as the mash profile, we take account of how much enzyme there is in the mash, which depends on the
 *        diastatic power of the grain bill and on the mash thickness (see \c enzymeLevels).
 *
 *        WHERE WE DEPART FROM THE PAPER
 *        ------------------------------
 *        All explained in detail at the relevant points in MashFermentability.cpp, but, in summary:
 *           - The paper's printed polynomials for α-amylase activity against temperature are unusable (the rounding of
 *             their coefficients swamps the result), so we use the data points plotted in its Fig. 3 instead.
 *           - The paper switches abruptly between two gelatinisation rates at 60 °C, which would make predicted FG
 *             jump.  We blend smoothly between the two over 60–63 °C, the gap in the paper's data.
 *           - Sugar masses include the water taken up in making them.  The paper doesn't say so, but this is needed
 *             to reproduce its results.
 *           - We integrate with fixed time steps rather than a general-purpose ODE solver, which is accurate enough
 *             and much simpler.
 *           - Enzyme levels are scaled for diastatic power (within the range of malts the paper tested) and mash
 *             thickness (within the range tested by the model's predecessor, Marc et al 1983).
 *           - The result is clamped, in a way that keeps it continuous (see \c minAttenuationMultiplier).
 *        With all of this, we still reproduce the paper's own model results to within about 1%.
 *
 *        This file has no dependencies on Qt or on our model classes, so that it is easy to unit test.  It is the
 *        caller's job (in our case \c Recipe) to turn \c MashStep objects into \c MashFermentability::Step structs.
 */
namespace MashFermentability {

   //================================================== Mash profile ==================================================

   /**
    * \brief What sort of mash step this is, insofar as it matters for building a temperature profile.  The caller maps
    *        \c MashStep::Type onto this.
    */
   enum class StepType {
      Infusion   , //!< Hot water added to the mash.  Temperature change is treated as quick unless a ramp time is given.
      Temperature, //!< Mash heated directly.  Without a ramp time, we assume \c defaultHeatingRate_cPerMin.
      Decoction  , //!< Part of the mash boiled and returned.  See comment in \c buildProfile.
      Sparge     , //!< Rinsing the grain.  Marks the end of the mash: this step and all later ones are ignored.
      Other      , //!< Anything else (eg souring steps).  Ignored.
   };

   /**
    * \brief One mash step, as far as this model is concerned.  Fields correspond to those of \c MashStep.
    */
   struct Step {
      StepType              kind;
      double                stepTemp_c;      //!< Temperature the step is held at (MashStep::startTemp_c)
      std::optional<double> endTemp_c;       //!< If set, temperature at the end of the hold (eg if the mash cools)
      double                stepTime_mins;   //!< How long the step is held for, not counting the ramp
      std::optional<double> rampTime_mins;   //!< If set, time taken to get from the previous temperature to this one
   };

   /**
    * \brief One point on a temperature profile.  Between points, temperature changes linearly with time.
    */
   struct ProfilePoint {
      double time_mins;
      double temp_c;
   };

   using TemperatureProfile = std::vector<ProfilePoint>;

   /**
    * \brief Default heating rate where a \c StepType::Temperature step has no ramp time, and for the mash-out we
    *        append (see \c buildProfile).  1 °C/min is typical of homebrew direct-fired or electric mash tuns.  (The
    *        paper's pilot plant managed 2 °C/min.)  The exact figure matters little, because the reference mash gets
    *        the same mash-out as the recipe's mash.
    */
   double constexpr defaultHeatingRate_cPerMin = 1.0;

   /**
    * \brief If the mash profile ends below this temperature, we append a ramp up to it, followed by a hold of
    *        \c mashOutHold_mins.  All the paper's experiments ended this way, and in practice the enzymes are
    *        destroyed at some point after the mash (by mash-out, by hot sparge water, or, at the latest, when the wort
    *        is heated to the boil), and until then they carry on working.  Adding a standard ending means that the
    *        recipe's mash and the reference mash are treated the same way, whether or not the recipe has an explicit
    *        mash-out step.
    */
   double constexpr mashOutTemp_c     = 76.0;
   double constexpr mashOutHold_mins  = 10.0;

   /**
    * \brief Turn a list of mash steps into a temperature profile.
    *
    * \return Empty profile if there are no steps that we can use (in which case the caller should not adjust anything)
    */
   TemperatureProfile buildProfile(std::vector<Step> const & steps);

   //=================================================== Malt enzymes =================================================

   /**
    * \brief Amounts of α- and β-amylase in the mash, relative to the paper's malt (which the model's constants were
    *        fitted to).  1.0 means "same as the paper's malt".
    */
   struct EnzymeLevels {
      double alpha = 1.0;
      double beta  = 1.0;
   };

   /**
    * \brief Diastatic power (in degrees Lintner) that we assume the paper's malt had.
    *
    *        The paper gives the malt's enzyme activities in its own (Megazyme assay) units, but not its diastatic
    *        power.  It was a malt from the Scarlett barley variety used by Kronenbourg, ie a continental pilsner-type
    *        base malt, and current specifications for such malts (eg BESTMALZ Pilsen Malt, Castle Malting Château
    *        Pilsen 2RS and Malteurop Pilsen) all give 250 °WK.  Converting with the usual formula
    *        °Lintner = (°WK + 16) ÷ 3.5 gives 76 °Lintner.
    *
    *        This is an assumption, so we only use it to scale enzyme levels within the range of malts the paper
    *        tested (see \c enzymeLevels), which limits the damage if it is somewhat off.
    */
   double constexpr nominalDiastaticPower_lintner = (250.0 + 16.0) / 3.5;

   /**
    * \brief Water-to-grist ratio (litres of water per kg of malt) of all the paper's experiments: 70 kg of water to
    *        20 kg of malt.
    */
   double constexpr paperMashThickness_lKg = 70.0 / 20.0;

   /**
    * \brief Range of mash thickness over which we scale enzyme levels (see \c enzymeLevels).  Outside this range, we
    *        use the nearest end of it.
    *
    *        This comes from the predecessor of the paper's model: A. Marc, J.M. Engasser, M. Moll, R. Flayeux, "A
    *        kinetic model of starch hydrolysis by α- and β-amylase during mashing", Biotechnology and Bioengineering 25
    *        (1983) 481–496.  Its Table II shows the initial concentrations of α-amylase, β-amylase and starch in the
    *        mash varying roughly in proportion to the malt-to-water ratio, over ratios from 1:2 to 1:5.
    */
   double constexpr minScaledMashThickness_lKg = 2.0;
   double constexpr maxScaledMashThickness_lKg = 5.0;

   /**
    * \brief Work out enzyme levels from the (mass-weighted average) diastatic power of everything in the mash, and the
    *        mash thickness.
    *
    * \param diastaticPower_lintner  If not set, we assume a typical base malt, ie \c nominalDiastaticPower_lintner
    * \param mashThickness_lKg       Litres of water per kg of grain in the mash (not counting sparge water).  If not
    *                                set, we assume \c referenceMashThickness_lKg.
    *
    *        MASH THICKNESS
    *        In a thicker mash, there is more of everything -- enzymes, starch and sugars -- per kg of mash, in
    *        proportion to the malt-to-water ratio (see \c minScaledMashThickness_lKg).  In the model, every rate of
    *        breaking down starch or dextrins is (enzyme activity) × (amount of starch or dextrin), so scaling the
    *        starch and sugars as well as the enzymes would make no difference to the *proportion* of the extract that
    *        is fermentable.  The only thing that matters is the enzyme concentration, so we scale enzyme levels by
    *        \c paperMashThickness_lKg ÷ (this mash's thickness), with the thickness clamped to
    *        [\c minScaledMashThickness_lKg, \c maxScaledMashThickness_lKg].  Thus a thicker mash converts faster and
    *        ends up somewhat more fermentable.  (NB: the model does not include any protective effect of a thick mash on
    *        enzymes against heat, which some brewing texts describe.)
    *
    *        DIASTATIC POWER
    *        Diastatic power measures the combined starch-degrading power of a malt, and in practice malts with more
    *        of one amylase tend to have more of the other, so we scale both amylases by the ratio of
    *        \c diastaticPower_lintner to \c nominalDiastaticPower_lintner.  (We do not use a malt's separate
    *        α-amylase figure, as it is rarely recorded.)  A grist diluted with crystal malts, roasted malts or
    *        unmalted adjuncts (which have no diastatic power) therefore has lower enzyme levels, as it should.
    *
    *        We clamp the effect of diastatic power on each level to the range covered by the six malts in the paper's
    *        Table 1, as the model was only validated over that range.  In the model, the α-amylase level matters much more than the
    *        β-amylase one: over the tested range, α-amylase moves the result by several percent, β-amylase by about
    *        1%.  (That is partly because the paper found α-amylase does most of the work of making sugars, and partly
    *        because its β-amylase constants are not very precisely determined -- see Table 4.)
    */
   EnzymeLevels enzymeLevels(std::optional<double> const diastaticPower_lintner,
                             std::optional<double> const mashThickness_lKg);

   //=================================================== Simulation ===================================================

   /**
    * \brief The state of the mash at the end of the simulation.  All amounts in grams per kilogram of mash, with the
    *        paper's malt and water-to-grist ratio (see MashFermentability.cpp).
    *
    *        NB: The three sugars are given as the actual mass of sugar, which includes the water taken up in breaking
    *        the starch into them -- see comment on \c hydrationFactor_glucose etc in MashFermentability.cpp.
    */
   struct Result {
      double ungelatinisedStarch_gPerKg;
      double gelatinisedStarch_gPerKg;
      double dextrins_gPerKg;
      double glucose_gPerKg;
      double maltose_gPerKg;
      double maltotriose_gPerKg;

      //! \brief Total of the three fermentable sugars
      double fermentableSugars_gPerKg() const;

      /**
       * \brief Fraction (0–1) of the dissolved extract that yeast can ferment.  Gelatinised starch that has not been
       *        broken down is dissolved but not fermentable, so it counts against fermentability.  Ungelatinised
       *        starch stays in the grain, so isn't counted at all.
       */
      double fermentableFraction() const;

      /**
       * \brief Fraction (0–1) of the original starch that has been broken down into sugars or dextrins.  (Normally
       *        close to 1.  It is only low for unusual profiles, eg a mash that is too hot for the enzymes to work.)
       */
      double fractionOfStarchConverted() const;
   };

   /**
    * \brief Run the model over the given temperature profile.
    */
   Result simulate(TemperatureProfile const & profile, EnzymeLevels const & enzymeLevels);

   //=========================================== Effect on yeast attenuation ==========================================

   /**
    * \brief The reference mash against which the recipe's mash is compared: a single infusion at
    *        \c referenceMashTemp_c held for \c referenceMashTime_mins (followed by the standard mash-out -- see
    *        \c mashOutTemp_c), with a typical base malt (\c nominalDiastaticPower_lintner) and a mash thickness of
    *        \c referenceMashThickness_lKg.
    *
    *        The idea is that this represents the sort of wort a yeast lab would have used to measure the attenuation
    *        figures it publishes for its yeasts.  Labs don't, as far as we know, publish what mash they use, but a
    *        single infusion for an hour in the 63–66 °C range is the usual "middle of the road" mash.  We take the
    *        upper end of that range.  (Taking 65 °C instead would lower all mash-adjusted attenuations by about 2%,
    *        eg 75% would become about 73.3%.)
    *
    *        One day, we might let the user override this in the Options dialog.
    */
   double constexpr referenceMashTemp_c    = 66.0;
   double constexpr referenceMashTime_mins = 60.0;

   /**
    * \brief Mash thickness (litres of water per kg of grain) of the reference mash, and what we assume if we don't
    *        know a recipe's mash thickness.
    *
    *        Single-infusion mashes are typically somewhere between about 2.5 and 3.5 L/kg (roughly 1.2–1.7 US quarts
    *        per pound).  (For comparison, MashAutoAdjuster::defaultThickness_LKg, which is for the strike water only, is
    *        2.6 L/kg; and the paper's experiments were all at 3.5 L/kg.)  We take 3.0 L/kg as the middle of that range.
    *        If we used the paper's 3.5 L/kg here, then, because thicker mashes convert faster (see \c enzymeLevels),
    *        a typical homebrew mash would come out a few percent more fermentable than the reference, ie we'd be
    *        systematically raising everyone's attenuation.
    */
   double constexpr referenceMashThickness_lKg = 3.0;

   /**
    * \brief Fermentable fraction of the reference mash.  (Calculated once and then cached.)
    */
   double referenceFermentableFraction();

   /**
    * \brief Limits on the multiplier returned by \c attenuationMultiplier.  For realistic mashes, the model gives
    *        multipliers between about 0.82 (eg a single infusion at 72 °C) and about 1.12 (eg 90 minutes at 60 °C).
    *        These limits sit just outside that range, so they only bite on unusual mash profiles where we'd be
    *        extrapolating beyond what the model was tested on.
    *
    *        The lower limit is deliberately set just above where the model falls off a cliff.  In the model,
    *        gelatinised starch is broken down in a fraction of a second, so even a trace of α-amylase activity converts
    *        all of it.  The paper's α-amylase activity curve reaches exactly zero at 347 K (73.85 °C).  So, for a mash
    *        that starts at or above that temperature, the predicted conversion drops from 100% to 0% over a few
    *        thousandths of a degree, and the multiplier drops from about 0.80 to almost nothing.  (Such a mash really
    *        would give a very unfermentable wort, if it gave a wort at all.)  Just above the drop, the multiplier is
    *        0.80 to within about 0.001, whatever the enzyme levels, because at that temperature β-amylase is
    *        destroyed within seconds and α-amylase alone breaks starch down in fixed proportions.  With the lower limit
    *        at 0.81, the drop happens entirely below the limit, so the multiplier just levels off at 0.81 for very hot
    *        mashes.
    *
    *        NB: We deliberately clamp rather than, say, refusing to adjust attenuation for odd profiles.  Clamping is
    *        continuous, so a small change to a mash step can never make predicted FG jump.  (Refusing to adjust would
    *        make it jump back to the unadjusted value.)  See MashFermentabilityTests::testNoDiscontinuities.
    */
   double constexpr minAttenuationMultiplier = 0.81;
   double constexpr maxAttenuationMultiplier = 1.20;

   /**
    * \brief Adjusting attenuation for the mash never takes it above this.  (A yeast that is \em specified as having
    *        higher attenuation, eg a diastaticus strain, keeps its own figure.)
    */
   double constexpr maxMashAdjustedAttenuation_pct = 95.0;

   /**
    * \brief How much to multiply the yeast's attenuation by to take account of the mash, ie
    *        F(this mash) ÷ F(reference mash), clamped to [\c minAttenuationMultiplier, \c maxAttenuationMultiplier].
    *
    * \return \c std::nullopt if the profile is empty, in which case the caller should not adjust the attenuation.
    */
   std::optional<double> attenuationMultiplier(TemperatureProfile const & profile, EnzymeLevels const & enzymeLevels);

   /**
    * \brief Apply \c multiplier to \c yeastAttenuation_pct, respecting \c maxMashAdjustedAttenuation_pct.
    */
   double adjustedAttenuation_pct(double const yeastAttenuation_pct, double const multiplier);
}

#endif