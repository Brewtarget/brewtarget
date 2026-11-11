/*======================================================================================================================
 * calculations/MashFermentability.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "calculations/MashFermentability.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

//
// Throughout this file, "the paper" means Brandam et al (2003) -- see full reference in MashFermentability.h -- and
// equation, table and figure numbers refer to it.  Where the paper has errors or omissions, we say so, and explain what
// we've done instead.  We checked our implementation by reproducing the paper's own model results (its Table 5, "Model
// 1" column) for all nine of its experiments to within about 1 g/kg (ie about 1%) -- see
// unitTests/MashFermentabilityTests.cpp.
//
// Units: as in the paper, amounts of starch, dextrins and sugars are in grams per kilogram of mash ("maïsche" in the
// paper), and enzyme activities are in U (enzyme activity units) per kilogram of mash.  Time inside the simulation is
// in seconds, because the paper's rate constants are per second.  Temperatures in the Arrhenius equations are in
// kelvin.
//
namespace {

   //================================================ Physical constants ==============================================

   /**
    * \brief Gas constant, J/(mol·K).  We use the same (slightly rounded) value as the paper, because its activation
    *        energies and pre-exponential factors were fitted using it, and, with numbers as large as the paper's
    *        pre-exponential factors, small differences in R matter.
    */
   double constexpr R = 8.31;

   double constexpr kelvinOffset = 273.15;

   /**
    * \brief The "Arrhenius equation", used throughout the paper for how the speed of a reaction depends on temperature:
    *        rate constant = A × exp(−E / RT), where A is the "pre-exponential factor" (s⁻¹) and E the "activation
    *        energy" (J/mol).
    *
    *        Some of the paper's pre-exponential factors are enormous (up to 7.6 × 10⁶⁰), and are multiplied by
    *        correspondingly tiny exponentials.  To keep well clear of overflow and underflow, we do the calculation in
    *        log space, ie exp(ln A − E / RT).
    */
   double lnArrhenius(double const preExponentialFactor, double const activationEnergy_jPerMol, double const temp_k) {
      return std::log(preExponentialFactor) - activationEnergy_jPerMol / (R * temp_k);
   }
   double arrhenius(double const preExponentialFactor, double const activationEnergy_jPerMol, double const temp_k) {
      return std::exp(lnArrhenius(preExponentialFactor, activationEnergy_jPerMol, temp_k));
   }

   //=============================================== Starch gelatinisation ============================================
   //
   // Section 4.1 and Table 4 of the paper.  Starch has to gelatinise before enzymes can act on it.  This is modelled as
   // a first-order reaction (ie the rate is proportional to the amount of ungelatinised starch left), with an Arrhenius
   // rate constant (Equations 3 and 4).  The paper found the rate changes abruptly between 60 and 63 °C (Fig. 2), so it
   // uses one set of constants below a threshold temperature Tg (60 °C) and another above.
   //
   // However, switching abruptly at exactly 60 °C makes our results jump: eg a 60-minute single infusion at 59.9 °C
   // came out about 2% less fermentable than one at 60.0 °C, and a two-step mash about 3%.  A tiny change to a mash
   // step should not make a visible jump in predicted FG.  And the paper has no measurements between 60 and 63 °C
   // (Fig. 2), so all it really tells us is that the change happens somewhere in that range.  So, between 60 and 63 °C,
   // we interpolate the logarithm of the rate constant linearly between the two Arrhenius lines (which is to say we
   // draw a straight line on Fig. 2 between the end of the lower line and the start of the upper one).  Below 60 °C
   // and above 63 °C, this is exactly the paper's model.
   //
   // (The model's predecessor, Marc et al 1983, used an on/off gelatinisation step at 55 °C, which would give the same
   // sort of jump.  The paper's model avoids this except at its threshold, which we smooth as described.)
   //
   // The paper also tried modelling small starch granules separately (Eq. 14), but found that this did not
   // significantly improve the prediction of sugars (Section 5.3), so, like the paper's own "Model 1", we don't.
   //
   double constexpr gelatinisationThreshold_c = 60.0; // Tg, where the paper's lower line ends
   double constexpr gelatinisationUpper_c     = 63.0; // Where the paper's upper line starts (its next data point)
   double constexpr gelatinisationBelow_preExp = 5.7e31 ; // kg1, s⁻¹
   double constexpr gelatinisationBelow_jPerMol = 220.6e3; // Eg1
   double constexpr gelatinisationAbove_preExp = 3.1e14 ; // kg2, s⁻¹
   double constexpr gelatinisationAbove_jPerMol = 108.3e3; // Eg2

   double gelatinisationRate_perSec(double const temp_c) {
      double const temp_k = temp_c + kelvinOffset;
      if (temp_c <= gelatinisationThreshold_c) {
         return arrhenius(gelatinisationBelow_preExp, gelatinisationBelow_jPerMol, temp_k);
      }
      if (temp_c >= gelatinisationUpper_c) {
         return arrhenius(gelatinisationAbove_preExp, gelatinisationAbove_jPerMol, temp_k);
      }
      // In the gap: straight line (in log space) between the two lines' values at the two ends of the gap
      double const lnRateAtLower = lnArrhenius(gelatinisationBelow_preExp,
                                               gelatinisationBelow_jPerMol,
                                               gelatinisationThreshold_c + kelvinOffset);
      double const lnRateAtUpper = lnArrhenius(gelatinisationAbove_preExp,
                                               gelatinisationAbove_jPerMol,
                                               gelatinisationUpper_c + kelvinOffset);
      double const fraction = (temp_c - gelatinisationThreshold_c) / (gelatinisationUpper_c - gelatinisationThreshold_c);
      return std::exp(lnRateAtLower + fraction * (lnRateAtUpper - lnRateAtLower));
   }

   //================================================= Amylase activity ===============================================
   //
   // Section 4.2 of the paper.  The activity of each amylase at any moment is the product of two things:
   //
   //    (a) How much of the enzyme is still intact.  Heat gradually destroys ("denatures") enzymes, which, again, is
   //        modelled as a first-order reaction with an Arrhenius rate constant (Eq. 5).  Once destroyed, an enzyme
   //        doesn't come back, so this is where the history of the mash matters: eg a long rest at 63 °C destroys most
   //        of the β-amylase, so a following rest at 70 °C has very little β-amylase to work with.  The paper (Fig. 5)
   //        shows this agreeing well with measurements.
   //
   //    (b) How fast each intact enzyme molecule works at the current temperature, relative to how fast it works at
   //        the temperature at which malt enzyme activity is measured in the lab (30 °C for β-amylase and 40 °C for
   //        α-amylase).  The paper calls this the "relative specific activity", aₛ(T), and gives it as curves fitted
   //        to lab measurements (Fig. 3).  Enzymes speed up as it gets warmer, until the temperature is high enough
   //        to start disrupting the enzyme's shape, when they slow down and then stop working altogether.
   //
   // So the activity that actually drives the reactions is: a(T) = [E] × aₛ(T), where [E] is the intact enzyme (in
   // units of activity measured in the lab).  (NB: The paper's Eq. 6 garbles this, but Eq. 2 and the text make the
   // meaning clear.)
   //
   double constexpr alphaDenaturation_preExp   = 6.9e30 ; // kdα, s⁻¹
   double constexpr alphaDenaturation_jPerMol  = 224.2e3; // Edα
   double constexpr betaDenaturation_preExp    = 7.6e60 ; // kdβ, s⁻¹
   double constexpr betaDenaturation_jPerMol   = 410.7e3; // Edβ
   //
   // To give a feel for what these mean, they imply half-lives for intact α-amylase of about 4 hours at 60 °C,
   // 75 minutes at 65 °C and 23 minutes at 70 °C, and for intact β-amylase of about 40 minutes at 60 °C, 4.5 minutes
   // at 65 °C and 30 seconds at 70 °C.
   //

   /**
    * Relative specific activity of α-amylase, aₛ(T), from Fig. 3 of the paper.
    *
    * The paper gives this as two cubic polynomials in temperature in kelvin (eg y = −0.0011x³ + 1.091x² − 352.89x +
    * 38008.3).  Unfortunately, the coefficients are printed to only 3–5 significant figures, and, with x around 330,
    * the terms of the polynomial are each in the tens of thousands and almost cancel out, so the rounding completely
    * swamps the answer: the printed polynomials give values of around 700–880 and −90, where the plotted curve is
    * between 0 and 7.2.
    *
    * So instead, we use the data points plotted on Fig. 3 (as read off the graph, to about ±0.2), interpolating
    * linearly between them.  As explained in the paper, the curve is flat at 1 below 40 °C (313 K, the temperature at
    * which α-amylase activity is measured) and zero from 74 °C (347 K) upwards, where α-amylase no longer works at all.
    *
    * Errors in reading the graph matter little: perturbing every point by up to ±0.2 changes the results of
    * attenuationMultiplier() by at most about 0.005, and scaling the whole curve by ±10% changes them by at most about
    * 0.008.
    */
   struct CurvePoint { double temp_k; double value; };
   std::array<CurvePoint, 18> constexpr alphaRelativeActivity{{
      {310.0, 1.0}, {313.0, 1.0}, {318.0, 1.6}, {323.0, 3.7}, {325.0, 4.5}, {326.0, 4.9},
      {328.0, 5.8}, {329.0, 6.1}, {333.0, 7.0}, {336.0, 7.2}, {338.0, 6.6}, {339.0, 6.2},
      {340.0, 5.5}, {341.0, 4.8}, {343.0, 3.1}, {344.0, 2.2}, {345.0, 1.4}, {347.0, 0.0},
   }};

   double alphaRelativeSpecificActivity(double const temp_c) {
      double const temp_k = temp_c + kelvinOffset;
      if (temp_k <= alphaRelativeActivity.front().temp_k) { return alphaRelativeActivity.front().value; }
      if (temp_k >= alphaRelativeActivity.back ().temp_k) { return alphaRelativeActivity.back ().value; }
      for (std::size_t ii = 1; ii < alphaRelativeActivity.size(); ++ii) {
         CurvePoint const & upper = alphaRelativeActivity[ii];
         if (temp_k <= upper.temp_k) {
            CurvePoint const & lower = alphaRelativeActivity[ii - 1];
            double const fraction = (temp_k - lower.temp_k) / (upper.temp_k - lower.temp_k);
            return lower.value + fraction * (upper.value - lower.value);
         }
      }
      // Should be unreachable given the checks above
      return 0.0;
   }

   /**
    * Relative specific activity of β-amylase, aₛ(T), from Fig. 3 of the paper.  Here the paper's fitted lines are
    * straight, and their printed coefficients reproduce the plotted points correctly, so we use them as given:
    * y = 0.049x − 13.9 up to 336 K (63 °C); y = −0.374x + 128.3 from 336 K to 343 K (70 °C); zero above that, where
    * β-amylase no longer works at all.  (The first line reaches 1 at about 304 K, ie 30 °C, the temperature at which
    * β-amylase activity is measured, which is a good sanity check.)
    */
   double betaRelativeSpecificActivity(double const temp_c) {
      double const temp_k = temp_c + kelvinOffset;
      if (temp_k <= 336.0) { return std::max(0.0, 0.049 * temp_k - 13.9); }
      if (temp_k >= 343.0) { return 0.0; }
      return std::max(0.0, -0.374 * temp_k + 128.3);
   }

   //================================================== Making sugars =================================================
   //
   // Section 4.3 and Table 4 of the paper.  The paper's reaction scheme (Fig. 1) is:
   //
   //    - α-amylase breaks gelatinised starch into glucose, maltose, maltotriose and dextrins (Eqs. 7–10)
   //    - β-amylase breaks gelatinised starch into maltose (second term of Eq. 8)
   //    - α-amylase breaks dextrins into glucose, maltose and maltotriose (Eqs. 11–13)
   //    - β-amylase breaks dextrins into maltose (second term of Eq. 12)
   //
   // Each reaction rate is (constant) × (enzyme activity) × (amount of starch or dextrin).  All the effect of
   // temperature comes through the enzyme activities.  The constants are in kg/(U·s).
   //
   // Breaking down gelatinised starch is very fast (fractions of a second once it's gelatinised), so, in effect, the
   // constants for starch decide what proportions starch is turned into.  Eg with only α-amylase active, about 47%
   // becomes maltose, 3% glucose, 14% maltotriose and 37% dextrins.  Breaking down dextrins is much slower (taking tens
   // of minutes to hours), and that is the main reason a long rest gives a more fermentable wort.
   //
   // NB: Table 4 shows large uncertainties for some of these constants (eg ±112% for kβ,mal), which the paper puts down
   // to not having measured dextrins during mashing.  But the overall result (total fermentable sugars) still matches
   // experiments well.
   //
   double constexpr starchToGlucose_byAlpha      = 0.023 ; // kgl
   double constexpr starchToMaltose_byAlpha      = 0.389 ; // kα,mal
   double constexpr starchToMaltose_byBeta       = 0.137 ; // kβ,mal
   double constexpr starchToMaltotriose_byAlpha  = 0.117 ; // kmlt
   double constexpr starchToDextrins_byAlpha     = 0.317 ; // kdex
   double constexpr dextrinToGlucose_byAlpha     = 2.9e-8; // k'gl
   double constexpr dextrinToMaltose_byAlpha     = 1.2e-7; // k'α,mal
   double constexpr dextrinToMaltose_byBeta      = 8.4e-8; // k'β,mal
   double constexpr dextrinToMaltotriose_byAlpha = 1.5e-8; // k'mlt

   //
   // Breaking a starch or dextrin chain into sugars adds one molecule of water for every link broken.  The model's
   // mass balances count everything as the mass of its glucose units *within a chain* (C₆H₁₀O₅, 162.14 g/mol), so, to
   // get the actual mass of each sugar, we have to add the water back in.  The paper doesn't mention this, but its
   // published results only come out right if this is done: without it, our total sugars were consistently about 5%
   // below the paper's own model figures; with it, they agree to about 1%.  (Dextrins are long chains, so the water
   // added in breaking them off the starch is negligible.)
   //
   //    Glucose     C₆H₁₂O₆     180.16 g/mol, from one glucose unit       → × 180.16 / 162.14
   //    Maltose     C₁₂H₂₂O₁₁   342.30 g/mol, from two glucose units      → × 342.30 / 324.28
   //    Maltotriose C₁₈H₃₂O₁₆   504.44 g/mol, from three glucose units    → × 504.44 / 486.42
   //
   double constexpr hydrationFactor_glucose     = 180.16 / 162.14;
   double constexpr hydrationFactor_maltose     = 342.30 / 324.28;
   double constexpr hydrationFactor_maltotriose = 504.44 / 486.42;

   //================================================ Starting conditions =============================================
   //
   // The paper's malt "S1" (Table 1), mashed at 20 kg of malt to 70 kg of water (ie about 3.5 L/kg).
   //
   // Table 1 labels the amounts as being per kg of dry malt ("MS" = "matière sèche"), but the numbers only make sense
   // as per kg of mash: eg 113.5 g/kg of starch is right for a mash that is 22% malt, and Fig. 5 shows the mash
   // starting with about 230 U/kg of α-amylase activity at 37 °C, which is what you get from Table 1's 210 and the α
   // curve in Fig. 3.  So that's how we use them.  (Because we only ever use the model to compare one mash profile
   // with another, these absolute amounts largely cancel out anyway.)
   //
   // These are for the paper's mash thickness (see MashFermentability::paperMashThickness_lKg).  For other mash
   // thicknesses, we scale the enzymes -- see MashFermentability::enzymeLevels for why only the enzymes.
   //
   double constexpr initialStarch_gPerKg  = 113.5;
   double constexpr initialGlucose_gPerKg =   4.0; // Sugars already present in the malt
   double constexpr initialMaltose_gPerKg =   5.0;
   double constexpr initialAlpha_uPerKg   = 210.0;
   double constexpr initialBeta_uPerKg    = 380.0;

   //
   // Range of enzyme activities across the six malts in Table 1, used to limit how far we scale enzyme levels from
   // those of S1 for diastatic power (see MashFermentability::enzymeLevels).
   //
   double constexpr minTestedAlpha_uPerKg = 110.0; // Malt M1
   double constexpr maxTestedAlpha_uPerKg = 290.0; // Malt S3
   double constexpr minTestedBeta_uPerKg  = 345.0; // Malt M1
   double constexpr maxTestedBeta_uPerKg  = 550.0; // Malt M2

   //=============================================== Numerical settings ===============================================

   /**
    * \brief Simulation time step, in seconds.
    *
    *        The paper solves its equations with a general-purpose "stiff" ODE solver.  (Equations are "stiff" when some
    *        things change very much faster than others -- here, gelatinised starch is broken down in a fraction of a
    *        second, whereas dextrins take hours.)  Instead, we take fixed time steps, and within each step use the
    *        exact solution for first-order decay at constant temperature (amount × exp(−rate × time)), which is
    *        stable however fast the reaction.  Checked against a stiff ODE solver over a range of single-infusion and
    *        step mashes, this gives fermentable fractions within 0.0005 at 15 second steps (and within 0.0009 at 30
    *        seconds), which is far more accurate than the model itself.  A three-hour mash takes 720 steps, which is
    *        quick enough not to matter.
    */
   double constexpr timeStep_secs = 15.0;

   /**
    * \brief Limits for numerical sanity (not physics).  Outside about 30–80 °C the model is self-limiting anyway: below
    *        that, starch barely gelatinises and nothing much happens; above it, both enzymes are destroyed within
    *        seconds.
    */
   double constexpr minTemp_c = 0.0;
   double constexpr maxTemp_c = 100.0;
   double constexpr maxDuration_mins = 24.0 * 60.0;

   /**
    * \brief Returns the temperature at time \c time_mins on \c profile, interpolating linearly between points.
    *        \c hint is the index of the profile segment last used, so that walking forward through time is quick.
    */
   double temperatureAt(MashFermentability::TemperatureProfile const & profile,
                        double const time_mins,
                        std::size_t & hint) {
      while (hint + 1 < profile.size() && profile[hint + 1].time_mins < time_mins) {
         ++hint;
      }
      if (hint + 1 >= profile.size()) {
         return profile.back().temp_c;
      }
      auto const & start = profile[hint];
      auto const & end   = profile[hint + 1];
      double const span_mins = end.time_mins - start.time_mins;
      if (span_mins <= 0.0) {
         return end.temp_c;
      }
      double const fraction = std::clamp((time_mins - start.time_mins) / span_mins, 0.0, 1.0);
      return start.temp_c + fraction * (end.temp_c - start.temp_c);
   }

   /**
    * \brief Helper for building a profile.  Appends a point at \c duration_mins after the last one.  (Zero-length
    *        segments are fine -- they represent instant temperature changes.)
    */
   void appendPoint(MashFermentability::TemperatureProfile & profile, double const duration_mins, double const temp_c) {
      double const lastTime_mins = profile.empty() ? 0.0 : profile.back().time_mins;
      profile.push_back({lastTime_mins + std::clamp(duration_mins, 0.0, maxDuration_mins),
                         std::clamp(temp_c, minTemp_c, maxTemp_c)});
      return;
   }
}

MashFermentability::TemperatureProfile MashFermentability::buildProfile(std::vector<Step> const & steps) {
   TemperatureProfile profile;

   for (Step const & step : steps) {
      if (step.kind == StepType::Sparge) {
         // Sparging means the mash is over.  We'll add the standard mash-out below.
         break;
      }
      if (step.kind == StepType::Other) {
         // Eg a souring step, which is not part of starch conversion.
         continue;
      }

      double const stepTemp_c = std::clamp(step.stepTemp_c, minTemp_c, maxTemp_c);
      if (profile.empty()) {
         //
         // First step: the mash starts at the step temperature.  (Any ramp time on the first step is the time to heat
         // the strike water, which doesn't matter to us, as there's no starch in it.)
         //
         profile.push_back({0.0, stepTemp_c});
      } else {
         //
         // Get from wherever the previous step ended to this step's temperature.  Without an explicit ramp time:
         //    - For an infusion, adding the hot water raises the temperature more or less straight away.
         //    - For a decoction, returning the boiled portion likewise raises the temperature of the main mash more
         //      or less straight away.  (Strictly, a decoction is two mashes at different temperatures -- the boiled
         //      portion has its starch fully gelatinised and its enzymes destroyed -- which this model can't
         //      represent.  We just treat it as the main mash at the step temperature, which is what most of the mash
         //      experiences.)
         //    - For direct heating, we assume defaultHeatingRate_cPerMin.
         //
         double const previousTemp_c = profile.back().temp_c;
         double rampTime_mins = 0.0;
         if (step.rampTime_mins) {
            rampTime_mins = *step.rampTime_mins;
         } else if (step.kind == StepType::Temperature) {
            rampTime_mins = std::abs(stepTemp_c - previousTemp_c) / defaultHeatingRate_cPerMin;
         }
         appendPoint(profile, rampTime_mins, stepTemp_c);
      }

      // The hold, during which the temperature might drift from the step temperature to the end temperature
      appendPoint(profile, step.stepTime_mins, step.endTemp_c.value_or(stepTemp_c));
   }

   if (profile.empty()) {
      return profile;
   }

   // Add the standard ending (see comment on mashOutTemp_c), unless the mash already ends at or above it
   double const finalTemp_c = profile.back().temp_c;
   if (finalTemp_c < mashOutTemp_c) {
      appendPoint(profile, (mashOutTemp_c - finalTemp_c) / defaultHeatingRate_cPerMin, mashOutTemp_c);
      appendPoint(profile, mashOutHold_mins, mashOutTemp_c);
   }

   return profile;
}

MashFermentability::EnzymeLevels MashFermentability::enzymeLevels(std::optional<double> const diastaticPower_lintner,
                                                                  std::optional<double> const mashThickness_lKg) {
   //
   // Two separate effects, each clamped to the range it was tested over, then multiplied together.  (We clamp them
   // separately because they come from different experiments: diastatic power from the range of malts in the paper,
   // mash thickness from the range of malt-to-water ratios in Marc et al.)
   //
   double const diastaticPowerRatio =
      std::max(0.0, diastaticPower_lintner.value_or(nominalDiastaticPower_lintner)) / nominalDiastaticPower_lintner;
   double const alphaForDiastaticPower = std::clamp(diastaticPowerRatio,
                                                    minTestedAlpha_uPerKg / initialAlpha_uPerKg,
                                                    maxTestedAlpha_uPerKg / initialAlpha_uPerKg);
   double const betaForDiastaticPower  = std::clamp(diastaticPowerRatio,
                                                    minTestedBeta_uPerKg  / initialBeta_uPerKg ,
                                                    maxTestedBeta_uPerKg  / initialBeta_uPerKg );

   double const clampedThickness_lKg = std::clamp(mashThickness_lKg.value_or(referenceMashThickness_lKg),
                                                  minScaledMashThickness_lKg,
                                                  maxScaledMashThickness_lKg);
   double const concentrationForThickness = paperMashThickness_lKg / clampedThickness_lKg;

   return EnzymeLevels{
      .alpha = alphaForDiastaticPower * concentrationForThickness,
      .beta  = betaForDiastaticPower  * concentrationForThickness
   };
}

double MashFermentability::Result::fermentableSugars_gPerKg() const {
   return this->glucose_gPerKg + this->maltose_gPerKg + this->maltotriose_gPerKg;
}

double MashFermentability::Result::fermentableFraction() const {
   double const sugars = this->fermentableSugars_gPerKg();
   double const dissolvedExtract = sugars + this->dextrins_gPerKg + this->gelatinisedStarch_gPerKg;
   return dissolvedExtract > 0.0 ? sugars / dissolvedExtract : 0.0;
}

double MashFermentability::Result::fractionOfStarchConverted() const {
   return 1.0 - (this->ungelatinisedStarch_gPerKg + this->gelatinisedStarch_gPerKg) / initialStarch_gPerKg;
}

MashFermentability::Result MashFermentability::simulate(TemperatureProfile const & profile,
                                                       EnzymeLevels const & enzymeLevels) {
   //
   // State of the mash.  Sugars here are in "glucose units in chains" (see comment on hydrationFactor_glucose), and
   // we convert them to actual sugar mass at the end.
   //
   double ungelatinisedStarch = initialStarch_gPerKg;
   double gelatinisedStarch   = 0.0;
   double dextrins            = 0.0;
   double glucose             = 0.0;
   double maltose             = 0.0;
   double maltotriose         = 0.0;
   double intactAlpha         = initialAlpha_uPerKg * enzymeLevels.alpha;
   double intactBeta          = initialBeta_uPerKg  * enzymeLevels.beta;

   double const totalTime_secs = profile.empty() ? 0.0 : profile.back().time_mins * 60.0;
   std::size_t profileHint = 0;

   for (double time_secs = 0.0; time_secs < totalTime_secs; time_secs += timeStep_secs) {
      double const dt_secs = std::min(timeStep_secs, totalTime_secs - time_secs);
      //
      // We treat the temperature as constant during the step, taking the temperature at the middle of the step.
      //
      double const temp_c = temperatureAt(profile, (time_secs + dt_secs / 2.0) / 60.0, profileHint);

      // Enzyme activities right now: a(T) = [E] × aₛ(T)
      double const alpha = intactAlpha * alphaRelativeSpecificActivity(temp_c);
      double const beta  = intactBeta  * betaRelativeSpecificActivity (temp_c);

      //
      // 1. Gelatinisation (Eqs. 3 and 4)
      //
      double const newlyGelatinised = ungelatinisedStarch * (1.0 - std::exp(-gelatinisationRate_perSec(temp_c) * dt_secs));
      ungelatinisedStarch -= newlyGelatinised;

      //
      // 2. Breakdown of gelatinised starch (Eqs. 7–10).  All four reactions are first order in gelatinised starch, so
      //    the total amount broken down follows exp(−(sum of rates) × time), and it is shared between the products in
      //    proportion to their individual rates.  For simplicity, we add the starch gelatinised in this step to the
      //    pool at the start of the step; this makes negligible difference at our step size.
      //
      double const toGlucose     = starchToGlucose_byAlpha     * alpha;
      double const toMaltose     = starchToMaltose_byAlpha     * alpha + starchToMaltose_byBeta * beta;
      double const toMaltotriose = starchToMaltotriose_byAlpha * alpha;
      double const toDextrins    = starchToDextrins_byAlpha    * alpha;
      double const starchRate    = toGlucose + toMaltose + toMaltotriose + toDextrins;
      double newDextrins = 0.0;
      gelatinisedStarch += newlyGelatinised;
      if (starchRate > 0.0) {
         double const brokenDown = gelatinisedStarch * (1.0 - std::exp(-starchRate * dt_secs));
         gelatinisedStarch -= brokenDown;
         glucose     += brokenDown * toGlucose     / starchRate;
         maltose     += brokenDown * toMaltose     / starchRate;
         maltotriose += brokenDown * toMaltotriose / starchRate;
         newDextrins  = brokenDown * toDextrins    / starchRate;
      }

      //
      // 3. Breakdown of dextrins (Eqs. 11–13), handled the same way.  (Dextrins made in this step are added afterwards,
      //    again making negligible difference at our step size.)
      //
      double const dexToGlucose     = dextrinToGlucose_byAlpha     * alpha;
      double const dexToMaltose     = dextrinToMaltose_byAlpha     * alpha + dextrinToMaltose_byBeta * beta;
      double const dexToMaltotriose = dextrinToMaltotriose_byAlpha * alpha;
      double const dextrinRate      = dexToGlucose + dexToMaltose + dexToMaltotriose;
      if (dextrinRate > 0.0) {
         double const brokenDown = dextrins * (1.0 - std::exp(-dextrinRate * dt_secs));
         dextrins    -= brokenDown;
         glucose     += brokenDown * dexToGlucose     / dextrinRate;
         maltose     += brokenDown * dexToMaltose     / dextrinRate;
         maltotriose += brokenDown * dexToMaltotriose / dextrinRate;
      }
      dextrins += newDextrins;

      //
      // 4. Enzymes being destroyed by heat (Eq. 5)
      //
      double const temp_k = temp_c + kelvinOffset;
      intactAlpha *= std::exp(-arrhenius(alphaDenaturation_preExp, alphaDenaturation_jPerMol, temp_k) * dt_secs);
      intactBeta  *= std::exp(-arrhenius(betaDenaturation_preExp , betaDenaturation_jPerMol , temp_k) * dt_secs);
   }

   return Result{
      .ungelatinisedStarch_gPerKg = ungelatinisedStarch,
      .gelatinisedStarch_gPerKg   = gelatinisedStarch,
      .dextrins_gPerKg            = dextrins,
      .glucose_gPerKg             = initialGlucose_gPerKg + glucose     * hydrationFactor_glucose,
      .maltose_gPerKg             = initialMaltose_gPerKg + maltose     * hydrationFactor_maltose,
      .maltotriose_gPerKg         =                         maltotriose * hydrationFactor_maltotriose,
   };
}

double MashFermentability::referenceFermentableFraction() {
   // Function-local static is initialised once, in a thread-safe way, the first time we're called
   static double const referenceFraction = simulate(
      buildProfile({Step{.kind          = StepType::Infusion,
                         .stepTemp_c    = referenceMashTemp_c,
                         .endTemp_c     = std::nullopt,
                         .stepTime_mins = referenceMashTime_mins,
                         .rampTime_mins = std::nullopt}}),
      enzymeLevels(std::nullopt, referenceMashThickness_lKg)
   ).fermentableFraction();
   return referenceFraction;
}

std::optional<double> MashFermentability::attenuationMultiplier(TemperatureProfile const & profile,
                                                                EnzymeLevels const & enzymeLevels) {
   if (profile.empty()) {
      return std::nullopt;
   }

   double const multiplier = simulate(profile, enzymeLevels).fermentableFraction() / referenceFermentableFraction();
   return std::clamp(multiplier, minAttenuationMultiplier, maxAttenuationMultiplier);
}

double MashFermentability::adjustedAttenuation_pct(double const yeastAttenuation_pct, double const multiplier) {
   double const adjusted = yeastAttenuation_pct * multiplier;
   // Never push attenuation above the cap, but never pull a yeast's own (higher) figure down to it either
   return std::min(adjusted, std::max(yeastAttenuation_pct, maxMashAdjustedAttenuation_pct));
}