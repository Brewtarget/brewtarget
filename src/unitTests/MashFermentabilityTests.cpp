/*======================================================================================================================
 * unitTests/MashFermentabilityTests.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "unitTests/MashFermentabilityTests.h"

#include <cmath>
#include <optional>
#include <vector>

#include <QtTest/QtTest>

#include "calculations/MashFermentability.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(MashFermentabilityTests)

namespace {
   using UnitTests::fuzzyComp;
   using MashFermentability::Step;
   using MashFermentability::StepType;
   using MashFermentability::TemperatureProfile;

   Step infusion(double const temp_c, double const time_mins) {
      return Step{.kind = StepType::Infusion, .stepTemp_c = temp_c, .endTemp_c = std::nullopt,
                  .stepTime_mins = time_mins, .rampTime_mins = std::nullopt};
   }

   Step heated(double const temp_c, double const time_mins) {
      return Step{.kind = StepType::Temperature, .stepTemp_c = temp_c, .endTemp_c = std::nullopt,
                  .stepTime_mins = time_mins, .rampTime_mins = std::nullopt};
   }

   /**
    * \brief Multiplier for a mash profile in typical conditions, ie a typical base malt and the reference mash thickness
    *        (which is what a recipe gets if we don't know its grains' diastatic power or its mash thickness)
    */
   double multiplierFor(std::vector<Step> const & steps) {
      return MashFermentability::attenuationMultiplier(MashFermentability::buildProfile(steps),
                                                       MashFermentability::enzymeLevels(std::nullopt, std::nullopt))
         .value_or(-1.0);
   }

   /**
    * \brief The temperature profile of the paper's experiments E1–E7 (Section 2.1): 10 min at 37 °C, up at 2 °C/min to
    *        50 °C, 30 min at 50 °C, up at 2 °C/min to the saccharification rest, the rest itself, then up at 0.5 °C/min
    *        to 76 °C and 10 min there.  (We build this directly rather than with buildProfile, so that it is exactly
    *        what the paper did.)
    */
   TemperatureProfile paperProfile(double const restTemp_c, double const restTime_mins) {
      TemperatureProfile profile{{0.0, 37.0}};
      auto add = [&profile](double const duration_mins, double const temp_c) {
         profile.push_back({profile.back().time_mins + duration_mins, temp_c});
      };
      add(10.0                        , 37.0);
      add((50.0 - 37.0) / 2.0         , 50.0);
      add(30.0                        , 50.0);
      add((restTemp_c - 50.0) / 2.0   , restTemp_c);
      add(restTime_mins               , restTemp_c);
      add((76.0 - restTemp_c) / 0.5   , 76.0);
      add(10.0                        , 76.0);
      return profile;
   }

   bool sameProfile(TemperatureProfile const & lhs, TemperatureProfile const & rhs) {
      if (lhs.size() != rhs.size()) {
         return false;
      }
      for (std::size_t ii = 0; ii < lhs.size(); ++ii) {
         if (!fuzzyComp(lhs[ii].time_mins, rhs[ii].time_mins, 1e-9) ||
             !fuzzyComp(lhs[ii].temp_c   , rhs[ii].temp_c   , 1e-9)) {
            return false;
         }
      }
      return true;
   }
}

MashFermentabilityTests::MashFermentabilityTests() = default;

MashFermentabilityTests::~MashFermentabilityTests() = default;

void MashFermentabilityTests::testReproducesPaper() {
   //
   // Total fermentable sugars (glucose + maltose + maltotriose, g/kg of mash) predicted by the paper's own model
   // ("Model 1" in its Table 5).  We don't expect an exact match -- we take bigger time steps, we use points read off
   // the paper's Fig. 3 rather than its (unusable) α-amylase polynomials, and we smooth the change in gelatinisation
   // rate between 60 and 63 °C -- but we should be within about 1%.
   //
   struct PaperResult {
      double restTemp_c;
      double restTime_mins;
      double paperModelSugars_gPerKg;
   };
   for (PaperResult const & paperResult : {
      PaperResult{60.0,  15.0, 103.7}, // E1
      PaperResult{63.0,  15.0, 103.0}, // E2
      PaperResult{65.0,  15.0, 100.2}, // E3
      PaperResult{70.0,  15.0,  94.3}, // E4
      PaperResult{63.0,   0.0,  96.0}, // E5
      PaperResult{63.0,  60.0, 114.0}, // E6
      PaperResult{63.0, 180.0, 123.2}, // E7
   }) {
      auto const result = MashFermentability::simulate(paperProfile(paperResult.restTemp_c, paperResult.restTime_mins),
                                                       {});
      QVERIFY2(fuzzyComp(result.fermentableSugars_gPerKg(), paperResult.paperModelSugars_gPerKg, 1.5),
               "Total sugars do not match paper's model");
   }

   // E8 and E9 were isothermal: 100 minutes at 63 °C and at 70 °C respectively
   auto const e8 = MashFermentability::simulate({{0.0, 63.0}, {100.0, 63.0}}, {});
   QVERIFY2(fuzzyComp(e8.fermentableSugars_gPerKg(), 116.6, 1.5), "E8 total sugars do not match paper's model");
   auto const e9 = MashFermentability::simulate({{0.0, 70.0}, {100.0, 70.0}}, {});
   QVERIFY2(fuzzyComp(e9.fermentableSugars_gPerKg(),  92.5, 1.5), "E9 total sugars do not match paper's model");

   // Longer rests at 63 °C break down more dextrins (paper's Table 6)
   double previousDextrins = 1000.0;
   for (double const restTime_mins : {0.0, 15.0, 60.0, 180.0}) {
      double const dextrins = MashFermentability::simulate(paperProfile(63.0, restTime_mins), {}).dextrins_gPerKg;
      QVERIFY2(dextrins < previousDextrins, "Dextrins should fall with longer rest");
      previousDextrins = dextrins;
   }
   return;
}

void MashFermentabilityTests::testTemperatureAndTime() {
   // The reference mash is, by definition, neutral
   QVERIFY(fuzzyComp(multiplierFor({infusion(MashFermentability::referenceMashTemp_c,
                                             MashFermentability::referenceMashTime_mins)}), 1.0, 1e-9));

   // Over the usual saccharification range, hotter means less fermentable
   double previous = 10.0;
   for (double temp_c = 62.0; temp_c <= 72.0; temp_c += 1.0) {
      double const multiplier = multiplierFor({infusion(temp_c, 60.0)});
      QVERIFY2(multiplier < previous, "Hotter mash should be less fermentable");
      previous = multiplier;
   }

   // And, at any given temperature, longer means more fermentable
   for (double const temp_c : {62.0, 64.0, 66.0, 68.0, 70.0}) {
      QVERIFY(multiplierFor({infusion(temp_c, 30.0)}) < multiplierFor({infusion(temp_c, 60.0)}));
      QVERIFY(multiplierFor({infusion(temp_c, 60.0)}) < multiplierFor({infusion(temp_c, 90.0)}));
   }

   // Time matters a lot at low temperatures but hardly at all at high ones, where β-amylase is quickly destroyed
   double const lowTempEffect  = multiplierFor({infusion(62.0, 90.0)}) - multiplierFor({infusion(62.0, 30.0)});
   double const highTempEffect = multiplierFor({infusion(72.0, 90.0)}) - multiplierFor({infusion(72.0, 30.0)});
   QVERIFY(lowTempEffect > 5.0 * highTempEffect);

   //
   // Some specific values, as a guard against accidental changes.  NB: These are what the model predicts, not
   // independent measurements!  (If you deliberately change the model, update these.)
   //
   QVERIFY(fuzzyComp(multiplierFor({infusion(64.0, 60.0)}), 1.043, 0.005));
   QVERIFY(fuzzyComp(multiplierFor({infusion(68.0, 60.0)}), 0.939, 0.005));
   QVERIFY(fuzzyComp(multiplierFor({infusion(70.0, 60.0)}), 0.871, 0.005));
   QVERIFY(fuzzyComp(multiplierFor({infusion(72.0, 60.0)}), 0.823, 0.005));
   return;
}

void MashFermentabilityTests::testBuildProfile() {
   using MashFermentability::buildProfile;
   double const mashOut_c = MashFermentability::mashOutTemp_c;
   double const mashOutHold_mins = MashFermentability::mashOutHold_mins;
   double const heatingRate = MashFermentability::defaultHeatingRate_cPerMin;

   // Nothing in, nothing out
   QVERIFY(buildProfile({}).empty());

   // Single infusion: hold, then the standard mash-out
   TemperatureProfile const single = buildProfile({infusion(66.0, 60.0)});
   TemperatureProfile const expectedSingle{
      {0.0                                          , 66.0     },
      {60.0                                         , 66.0     },
      {60.0 + (mashOut_c - 66.0) / heatingRate      , mashOut_c},
      {60.0 + (mashOut_c - 66.0) / heatingRate + mashOutHold_mins, mashOut_c},
   };
   QVERIFY2(sameProfile(single, expectedSingle), "Single infusion profile wrong");

   // Direct heating without a ramp time uses the default heating rate; an infusion without one is instant
   TemperatureProfile const twoStep = buildProfile({infusion(63.0, 30.0), heated(72.0, 20.0), infusion(74.0, 5.0)});
   QVERIFY(twoStep.size() >= 6);
   QVERIFY(fuzzyComp(twoStep[2].time_mins, 30.0 + 9.0 / heatingRate, 1e-9));
   QVERIFY(fuzzyComp(twoStep[2].temp_c   , 72.0, 1e-9));
   QVERIFY(fuzzyComp(twoStep[4].time_mins, twoStep[3].time_mins, 1e-9));
   QVERIFY(fuzzyComp(twoStep[4].temp_c   , 74.0, 1e-9));

   // An explicit ramp time is respected, as is an end temperature
   Step rampedAndCooling = heated(70.0, 30.0);
   rampedAndCooling.rampTime_mins = 25.0;
   rampedAndCooling.endTemp_c     = 68.0;
   TemperatureProfile const ramped = buildProfile({infusion(62.0, 30.0), rampedAndCooling});
   QVERIFY(fuzzyComp(ramped[2].time_mins, 55.0, 1e-9));
   QVERIFY(fuzzyComp(ramped[3].time_mins, 85.0, 1e-9));
   QVERIFY(fuzzyComp(ramped[3].temp_c   , 68.0, 1e-9));

   // Sparging ends the mash: anything after it is ignored, and souring steps are ignored altogether
   Step const sparge {.kind = StepType::Sparge, .stepTemp_c = 76.0, .endTemp_c = std::nullopt, .stepTime_mins = 30.0,
                      .rampTime_mins = std::nullopt};
   Step const souring{.kind = StepType::Other , .stepTemp_c = 45.0, .endTemp_c = std::nullopt, .stepTime_mins = 600.0,
                      .rampTime_mins = std::nullopt};
   QVERIFY2(sameProfile(buildProfile({infusion(66.0, 60.0), sparge, infusion(50.0, 30.0)}), expectedSingle),
            "Steps after sparge not ignored");
   QVERIFY2(sameProfile(buildProfile({souring, infusion(66.0, 60.0)}), expectedSingle), "Souring step not ignored");

   // No mash-out is added if the mash already ends hot enough
   TemperatureProfile const withMashOut = buildProfile({infusion(66.0, 60.0), heated(78.0, 10.0)});
   QVERIFY(fuzzyComp(withMashOut.back().temp_c, 78.0, 1e-9));
   return;
}

void MashFermentabilityTests::testStepMashes() {
   // A β-amylase rest followed by an α-amylase rest is more fermentable than the α-amylase rest alone...
   QVERIFY(multiplierFor({infusion(63.0, 30.0), heated(72.0, 30.0)}) > multiplierFor({infusion(72.0, 60.0)}));

   // ...and the order matters: once β-amylase has been destroyed at 72 °C, a later rest at 62 °C can't bring it back
   QVERIFY(multiplierFor({infusion(62.0, 30.0), heated(72.0, 30.0)}) >
           multiplierFor({infusion(72.0, 30.0), infusion(62.0, 30.0)}));

   // A long β-amylase rest gives a drier beer than the reference; a short one followed by a long α rest, a fuller one
   QVERIFY(multiplierFor({infusion(62.0, 45.0), heated(72.0, 15.0)}) > 1.0);
   QVERIFY(multiplierFor({infusion(68.0, 20.0), heated(72.0, 40.0)}) < 1.0);

   // A protein rest (below gelatinisation temperature) followed by an infusion makes little difference...
   QVERIFY(fuzzyComp(multiplierFor({infusion(52.0, 15.0), infusion(66.0, 60.0)}),
                     multiplierFor({infusion(66.0, 60.0)}),
                     0.01));
   // ...but heating slowly up from it, rather than infusing, gives β-amylase time to work on the way through its
   // favourite temperatures, which makes the wort more fermentable
   QVERIFY(multiplierFor({infusion(52.0, 15.0), heated(66.0, 60.0)}) >
           multiplierFor({infusion(52.0, 15.0), infusion(66.0, 60.0)}) + 0.01);
   return;
}

void MashFermentabilityTests::testNoDiscontinuities() {
   //
   // A small change to a mash step should only ever make a small change to the result.  This particularly checks the
   // smoothing of the change in gelatinisation rate between 60 and 63 °C (see MashFermentability.cpp), without which
   // there would be a jump of 2–3% at 60 °C, and the region around 74 °C, where α-amylase stops working altogether.
   //
   double previousSingle = multiplierFor({infusion(55.0, 60.0)});
   double previousStep   = multiplierFor({infusion(55.0, 20.0), heated(72.0, 20.0)});
   for (int tenths = 551; tenths <= 800; ++tenths) {
      double const temp_c = tenths / 10.0;
      double const single = multiplierFor({infusion(temp_c, 60.0)});
      double const step   = multiplierFor({infusion(temp_c, 20.0), heated(72.0, 20.0)});
      QVERIFY2(std::abs(single - previousSingle) < 0.004, "Jump in single infusion result");
      QVERIFY2(std::abs(step   - previousStep  ) < 0.004, "Jump in step mash result");
      previousSingle = single;
      previousStep   = step;
   }

   // (The first few minutes of a 63 °C rest make a big difference -- nearly 0.01 per minute -- so we step in tenths
   // of a minute to tell a steep slope from a jump.)
   double previous = multiplierFor({infusion(63.0, 0.0)});
   for (int tenths = 1; tenths <= 1800; ++tenths) {
      double const multiplier = multiplierFor({infusion(63.0, tenths / 10.0)});
      QVERIFY2(std::abs(multiplier - previous) < 0.002, "Jump in result as rest time changes");
      previous = multiplier;
   }
   return;
}

void MashFermentabilityTests::testEnzymeLevels() {
   using MashFermentability::enzymeLevels;

   // The paper's malt at the paper's mash thickness is, by definition, 1
   auto const nominal = enzymeLevels(MashFermentability::nominalDiastaticPower_lintner,
                                     MashFermentability::paperMashThickness_lKg);
   QVERIFY(fuzzyComp(nominal.alpha, 1.0, 1e-9));
   QVERIFY(fuzzyComp(nominal.beta , 1.0, 1e-9));

   // Nothing known means a typical base malt at the reference mash thickness, which is thicker than the paper's
   auto const typical = enzymeLevels(std::nullopt, std::nullopt);
   double const referenceThicknessFactor =
      MashFermentability::paperMashThickness_lKg / MashFermentability::referenceMashThickness_lKg;
   QVERIFY(fuzzyComp(typical.alpha, referenceThicknessFactor, 1e-9));
   QVERIFY(fuzzyComp(typical.beta , referenceThicknessFactor, 1e-9));

   // Diastatic power scales both enzymes, clamped to the range of malts in the paper's Table 1
   auto const somewhatHigher = enzymeLevels(MashFermentability::nominalDiastaticPower_lintner * 1.2,
                                            MashFermentability::paperMashThickness_lKg);
   QVERIFY(fuzzyComp(somewhatHigher.alpha, 1.2, 1e-9));
   QVERIFY(fuzzyComp(somewhatHigher.beta , 1.2, 1e-9));
   auto const none = enzymeLevels(0.0, MashFermentability::paperMashThickness_lKg);
   QVERIFY(fuzzyComp(none.alpha, 110.0 / 210.0, 1e-9));
   QVERIFY(fuzzyComp(none.beta , 345.0 / 380.0, 1e-9));
   auto const huge = enzymeLevels(1000.0, MashFermentability::paperMashThickness_lKg);
   QVERIFY(fuzzyComp(huge.alpha, 290.0 / 210.0, 1e-9));
   QVERIFY(fuzzyComp(huge.beta , 550.0 / 380.0, 1e-9));

   // Mash thickness scales enzyme concentration, clamped to 2–5 L/kg
   QVERIFY(fuzzyComp(enzymeLevels(std::nullopt,  2.0).alpha, 3.5 / 2.0, 1e-9));
   QVERIFY(fuzzyComp(enzymeLevels(std::nullopt,  1.0).alpha, 3.5 / 2.0, 1e-9));
   QVERIFY(fuzzyComp(enzymeLevels(std::nullopt,  5.0).beta , 3.5 / 5.0, 1e-9));
   QVERIFY(fuzzyComp(enzymeLevels(std::nullopt, 10.0).beta , 3.5 / 5.0, 1e-9));

   // The two effects combine
   auto const combined = enzymeLevels(MashFermentability::nominalDiastaticPower_lintner * 1.2, 2.5);
   QVERIFY(fuzzyComp(combined.alpha, 1.2 * 3.5 / 2.5, 1e-9));

   // Less enzyme means less fermentable, and a thicker mash (than the reference) means more fermentable
   auto const profile = MashFermentability::buildProfile({infusion(66.0, 60.0)});
   double const lowDp     = *MashFermentability::attenuationMultiplier(profile, enzymeLevels(30.0, std::nullopt));
   double const thick     = *MashFermentability::attenuationMultiplier(profile, enzymeLevels(std::nullopt, 2.5));
   double const thin      = *MashFermentability::attenuationMultiplier(profile, enzymeLevels(std::nullopt, 4.5));
   QVERIFY(lowDp < 1.0);
   QVERIFY(thick > 1.0);
   QVERIFY(thin  < 1.0);
   return;
}

void MashFermentabilityTests::testLimits() {
   using MashFermentability::attenuationMultiplier;

   // No profile, no adjustment
   QVERIFY(!attenuationMultiplier({}, {}));

   // A "mash" that is too hot for any enzyme to work converts almost no starch, and so is as unfermentable as we allow
   auto const tooHot = attenuationMultiplier({{0.0, 85.0}, {60.0, 85.0}}, {});
   QVERIFY(tooHot);
   QVERIFY(fuzzyComp(*tooHot, MashFermentability::minAttenuationMultiplier, 1e-9));
   QVERIFY(MashFermentability::simulate({{0.0, 85.0}, {60.0, 85.0}}, {}).fractionOfStarchConverted() < 0.1);

   // However odd the mash, the multiplier stays within limits
   for (auto const & steps : {std::vector<Step>{infusion(58.0, 600.0)},
                              std::vector<Step>{infusion(30.0, 60.0)},
                              std::vector<Step>{infusion(74.0, 5.0)},
                              std::vector<Step>{infusion(60.0, 1000.0), heated(65.0, 1000.0)}}) {
      for (auto const & levels : {MashFermentability::enzymeLevels(1000.0, 1.0),
                                  MashFermentability::enzymeLevels(   0.0, 9.0)}) {
         auto const multiplier = attenuationMultiplier(MashFermentability::buildProfile(steps), levels);
         if (multiplier) {
            QVERIFY(*multiplier >= MashFermentability::minAttenuationMultiplier);
            QVERIFY(*multiplier <= MashFermentability::maxAttenuationMultiplier);
         }
      }
   }

   // Adjusted attenuation is capped, but a yeast's own higher figure is not reduced to the cap
   using MashFermentability::adjustedAttenuation_pct;
   QVERIFY(fuzzyComp(adjustedAttenuation_pct(75.0, 1.1), 82.5, 1e-9));
   QVERIFY(fuzzyComp(adjustedAttenuation_pct(90.0, 1.1), MashFermentability::maxMashAdjustedAttenuation_pct, 1e-9));
   QVERIFY(fuzzyComp(adjustedAttenuation_pct(98.0, 1.1), 98.0, 1e-9));
   QVERIFY(fuzzyComp(adjustedAttenuation_pct(98.0, 0.9), 88.2, 1e-9));
   return;
}