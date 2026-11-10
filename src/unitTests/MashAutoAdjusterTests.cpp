/*======================================================================================================================
 * unitTests/MashAutoAdjusterTests.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "unitTests/MashAutoAdjusterTests.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QtTest/QtTest>
#include <QUndoStack>

#include "MashAutoAdjuster.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/PhysicalConstants.h"
#include "model/Boil.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeUtils.h"
#include "serialization/json/dotBeer/DotBeer.h"
#include "undoRedo/UndoableAutoAdjust.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(MashAutoAdjusterTests)

namespace {
   using UnitTests::fuzzyComp;
   using Severity = MashAutoAdjuster::Problem::Severity;

   double constexpr tolerance = 0.001;

   //
   // Figures used in most of the tests.  They are chosen to make the hand calculations in the comments easy:
   //    - Grain heat capacity = 5 kg × 0.4 = 2 kcal/°C, same as the tun
   //    - With a thickness of 3 L/kg, strike water = 3 × 5 + 1 = 16 L
   //    - Drained grain bed heat capacity = 2 (grain) + 5 × 1 (absorbed water) + 2 (tun) = 9 kcal/°C
   //
   MashAutoAdjuster::Inputs makeInputs() {
      return MashAutoAdjuster::Inputs{
         .grain_kg               =  5.0,
         .grainTemp_c            = 20.0,
         .tunHeatCapacity_kcalC  =  2.0,
         .tunTemp_c              = 20.0,
         .boilingPoint_c         = 100.0,
         .grainAbsorption_LKg    =  1.0,
         .strikeWaterAllowance_l =  1.0,
         .targetTotalWater_l     = 30.0,
         .mashVesselVolume_l     = 40.0,
      };
   }

   MashAutoAdjuster::Step makeStep(QString const & name, MashStep::Type const type, double const startTemp_c) {
      return MashAutoAdjuster::Step{
         .name          = name,
         .type          = type,
         .startTemp_c   = startTemp_c,
         .stepTime_mins = 30.0,
      };
   }

   MashAutoAdjuster::Options thickness(double const thickness_LKg) {
      return MashAutoAdjuster::Options{.thickness_LKg = thickness_LKg};
   }

   //! \return \c true if one of the messages of the given severity contains \c text
   bool hasMessage(MashAutoAdjuster::Result const & result, Severity const severity, QString const & text) {
      QStringList const messages = result.messages(severity);
      bool const found = std::any_of(messages.cbegin(), messages.cend(),
                                     [&text](QString const & message) { return message.contains(text); });
      if (!found) {
         qDebug() << Q_FUNC_INFO << "Did not find" << text << "in" << messages;
      }
      return found;
   }

   int numMessages(MashAutoAdjuster::Result const & result, Severity const severity) {
      return result.messages(severity).size();
   }

   /**
    * \brief Independent check of an infusion: mixing \c amount_l of water at \c waterTemp_c with things of the given
    *        heat capacities at the given temperatures should give \c expectedTemp_c.
    */
   bool infusionBalances(double const amount_l,
                         double const waterTemp_c,
                         QList<std::pair<double, double>> const & heatCapacitiesAndTemps,
                         double const expectedTemp_c) {
      double heatCapacity = amount_l * PhysicalConstants::waterSpecificHeat_calGC;
      double energy       = heatCapacity * waterTemp_c;
      for (auto const & [hc, temp] : heatCapacitiesAndTemps) {
         heatCapacity += hc;
         energy       += hc * temp;
      }
      return fuzzyComp(energy / heatCapacity, expectedTemp_c, tolerance);
   }
}

MashAutoAdjusterTests::MashAutoAdjusterTests() = default;

MashAutoAdjusterTests::~MashAutoAdjusterTests() = default;

void MashAutoAdjusterTests::testStrikeWater() {
   QList<MashAutoAdjuster::Step> steps{makeStep("Saccharification", MashStep::Type::Infusion, 66.0)};
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::BatchSparge, 1);
   QCOMPARE(steps.size(), 2);

   auto const result = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result.ok());
   QCOMPARE(numMessages(result, Severity::Warning), 0);

   //
   // Strike water = 3 L/kg × 5 kg + 1 L allowance = 16 L
   // Heat needed = 2 × (66 - 20) [grain] + 2 × (66 - 20) [tun] = 184 kcal
   // Water temp = 66 + 184 / 16 = 77.5 °C
   //
   auto const & strike = result.steps.at(0);
   QVERIFY(fuzzyComp(strike.amount_l, 16.0, tolerance));
   QVERIFY(strike.infuseTemp_c);
   QVERIFY(fuzzyComp(*strike.infuseTemp_c, 77.5, tolerance));
   QVERIFY(infusionBalances(16.0, *strike.infuseTemp_c, {{2.0, 20.0}, {2.0, 20.0}}, 66.0));
   // The thickness is recorded on the step
   QVERIFY(strike.liquorToGristRatio_lKg);
   QVERIFY(fuzzyComp(*strike.liquorToGristRatio_lKg, 3.0, tolerance));

   //
   // Without a thickness option, we use the step's liquor-to-grist ratio if it has one...
   //
   steps[0].liquorToGristRatio_lKg = 2.5;
   auto const result1 = MashAutoAdjuster::calculate(makeInputs(), steps);
   QVERIFY(result1.ok());
   QVERIFY(fuzzyComp(result1.steps.at(0).amount_l, 13.5, tolerance));
   steps[0].liquorToGristRatio_lKg = std::nullopt;

   //
   // ...otherwise the thickness of the existing strike water if there is any...
   //
   steps[0].amount_l = 11.0; // = 2 L/kg × 5 kg + 1 L
   auto const result2 = MashAutoAdjuster::calculate(makeInputs(), steps);
   QVERIFY(result2.ok());
   QVERIFY(fuzzyComp(result2.steps.at(0).amount_l, 11.0, tolerance));

   // ...otherwise the default
   steps[0].amount_l = 0.0;
   auto const result3 = MashAutoAdjuster::calculate(makeInputs(), steps);
   QVERIFY(result3.ok());
   QVERIFY(fuzzyComp(result3.steps.at(0).amount_l, MashAutoAdjuster::defaultThickness_LKg * 5.0 + 1.0, tolerance));

   //
   // If the tun starts at a different temperature from the grain, that's taken into account.  Eg with the tun
   // pre-heated to 66 °C, it doesn't need any heat, so water temp = 66 + 92 / 16 = 71.75 °C
   //
   auto inputs = makeInputs();
   inputs.tunTemp_c = 66.0;
   auto const result4 = MashAutoAdjuster::calculate(inputs, steps, thickness(3.0));
   QVERIFY(result4.ok());
   QVERIFY(fuzzyComp(*result4.steps.at(0).infuseTemp_c, 71.75, tolerance));
   return;
}

void MashAutoAdjusterTests::testFixedValues() {
   auto makeSteps = [](std::optional<double> const amount_l, std::optional<double> const waterTemp_c) {
      QList<MashAutoAdjuster::Step> steps{makeStep("Strike", MashStep::Type::Infusion, 66.0)};
      if (amount_l) {
         steps[0].amount_l    = *amount_l;
         steps[0].amountFixed = true;
      }
      if (waterTemp_c) {
         steps[0].infuseTemp_c    = *waterTemp_c;
         steps[0].infuseTempFixed = true;
      }
      MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
      return steps;
   };

   // Amount fixed at 20 L (so thickness ignored): water temp = 66 + 184 / 20 = 75.2 °C
   {
      auto const result = MashAutoAdjuster::calculate(makeInputs(), makeSteps(20.0, std::nullopt), thickness(3.0));
      QVERIFY(result.ok());
      QCOMPARE(result.steps.at(0).amount_l, 20.0);
      QVERIFY(fuzzyComp(*result.steps.at(0).infuseTemp_c, 75.2, tolerance));
      // Sparge gets the rest of the water
      QVERIFY(fuzzyComp(result.steps.at(1).amount_l, 10.0, tolerance));
   }

   // Temp fixed at 74 °C: amount = 184 / (74 - 66) = 23 L
   {
      auto const result = MashAutoAdjuster::calculate(makeInputs(), makeSteps(std::nullopt, 74.0), thickness(3.0));
      QVERIFY(result.ok());
      QVERIFY(fuzzyComp(result.steps.at(0).amount_l, 23.0, tolerance));
      QCOMPARE(*result.steps.at(0).infuseTemp_c, 74.0);
   }

   // Both fixed and consistent: no warning
   {
      auto const result = MashAutoAdjuster::calculate(makeInputs(), makeSteps(23.0, 74.0));
      QVERIFY(result.ok());
      QCOMPARE(numMessages(result, Severity::Warning), 0);
   }

   //
   // Both fixed and inconsistent: warning, and values unchanged.  16 L at 70 °C gives
   // (16 × 70 + 2 × 20 + 2 × 20) / (16 + 2 + 2) = 60 °C
   //
   {
      auto const result = MashAutoAdjuster::calculate(makeInputs(), makeSteps(16.0, 70.0));
      QVERIFY(result.ok());
      QCOMPARE(numMessages(result, Severity::Warning), 1);
      QVERIFY(hasMessage(result, Severity::Warning, "Strike"));
      QCOMPARE(result.steps.at(0).amount_l, 16.0);
      QCOMPARE(*result.steps.at(0).infuseTemp_c, 70.0);
   }

   // Temp marked as fixed but not set
   {
      auto steps = makeSteps(std::nullopt, 74.0);
      steps[0].infuseTemp_c = std::nullopt;
      auto const result = MashAutoAdjuster::calculate(makeInputs(), steps);
      QVERIFY(!result.ok());
   }

   // Water temp fixed below the step temp can't heat the mash
   {
      auto const result = MashAutoAdjuster::calculate(makeInputs(), makeSteps(std::nullopt, 60.0));
      QVERIFY(!result.ok());
   }
   return;
}

void MashAutoAdjusterTests::testStepInfusions() {
   QList<MashAutoAdjuster::Step> steps{
      makeStep("Protein rest"    , MashStep::Type::Infusion, 50.0),
      makeStep("Saccharification", MashStep::Type::Infusion, 66.0),
      makeStep("Mash out"        , MashStep::Type::Infusion, 72.0),
   };
   // Mash out with water at 90 °C
   steps[2].infuseTemp_c    = 90.0;
   steps[2].infuseTempFixed = true;
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);

   //
   // With three infusions, we need more water in total than in the other tests, otherwise there's none left for the
   // sparge, and a bigger mash tun (otherwise we get a warning that the mash won't fit).
   //
   auto inputs = makeInputs();
   inputs.targetTotalWater_l = 40.0;
   inputs.mashVesselVolume_l = 50.0;
   auto const result = MashAutoAdjuster::calculate(inputs, steps, thickness(3.0));
   QVERIFY(result.ok());
   QCOMPARE(numMessages(result, Severity::Warning), 0);

   //
   // Strike: 16 L.  Heat needed = 4 × (50 - 20) = 120 kcal, so water temp = 50 + 120 / 16 = 57.5 °C
   //
   QVERIFY(fuzzyComp(result.steps.at(0).amount_l, 16.0, tolerance));
   QVERIFY(fuzzyComp(*result.steps.at(0).infuseTemp_c, 57.5, tolerance));

   //
   // Second infusion uses boiling water.  Mash heat capacity = 2 + 16 + 2 = 20 kcal/°C, so heat needed = 20 × 16 = 320
   // kcal, so amount = 320 / (100 - 66) = 9.41176 L
   //
   double const secondAmount_l = 320.0 / 34.0;
   QVERIFY(fuzzyComp(result.steps.at(1).amount_l, secondAmount_l, tolerance));
   QCOMPARE(*result.steps.at(1).infuseTemp_c, 100.0);
   QVERIFY(infusionBalances(result.steps.at(1).amount_l, 100.0, {{20.0, 50.0}}, 66.0));

   //
   // Third infusion has fixed water temp of 90 °C.  Mash heat capacity = 2 + 2 + 16 + 9.41176 = 29.41176 kcal/°C
   //
   double const mashHeatCapacity = 4.0 + 16.0 + secondAmount_l;
   double const thirdAmount_l = mashHeatCapacity * (72.0 - 66.0) / (90.0 - 72.0);
   QVERIFY(fuzzyComp(result.steps.at(2).amount_l, thirdAmount_l, tolerance));
   QVERIFY(infusionBalances(thirdAmount_l, 90.0, {{mashHeatCapacity, 66.0}}, 72.0));

   // Fly sparge gets the rest
   QVERIFY(fuzzyComp(result.steps.at(3).amount_l, 40.0 - 16.0 - secondAmount_l - thirdAmount_l, tolerance));

   //
   // If a step has an end temperature, the next step starts from that.  Here, the protein rest drops to 48 °C, so the
   // second infusion has to provide 20 × 18 = 360 kcal.
   //
   steps[0].endTemp_c = 48.0;
   auto const result2 = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result2.ok());
   QVERIFY(fuzzyComp(result2.steps.at(1).amount_l, 360.0 / 34.0, tolerance));

   //
   // An infusion can also cool the mash, in which case we use water at the grain temperature (20 °C).  Going from
   // 50 °C to 45 °C needs 20 × 5 = 100 kcal taken out, so amount = 100 / (45 - 20) = 4 L
   //
   QList<MashAutoAdjuster::Step> coolingSteps{
      makeStep("Strike" , MashStep::Type::Infusion, 50.0),
      makeStep("Cooling", MashStep::Type::Infusion, 45.0),
   };
   MashAutoAdjuster::setSparge(coolingSteps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
   auto const result3 = MashAutoAdjuster::calculate(makeInputs(), coolingSteps, thickness(3.0));
   QVERIFY(result3.ok());
   QVERIFY(fuzzyComp(result3.steps.at(1).amount_l, 4.0, tolerance));
   QCOMPARE(*result3.steps.at(1).infuseTemp_c, 20.0);

   //
   // A temperature step (ie direct heat) doesn't need any calculation, but the following infusion starts from its
   // temperature
   //
   QList<MashAutoAdjuster::Step> directHeatSteps{
      makeStep("Strike"     , MashStep::Type::Infusion   , 50.0),
      makeStep("Direct heat", MashStep::Type::Temperature, 60.0),
      makeStep("Infusion"   , MashStep::Type::Infusion   , 66.0),
   };
   MashAutoAdjuster::setSparge(directHeatSteps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
   auto const result4 = MashAutoAdjuster::calculate(makeInputs(), directHeatSteps, thickness(3.0));
   QVERIFY(result4.ok());
   QCOMPARE(result4.steps.at(1).amount_l, 0.0);
   QVERIFY(fuzzyComp(result4.steps.at(2).amount_l, 20.0 * 6.0 / 34.0, tolerance));
   return;
}

void MashAutoAdjusterTests::testDecoction() {
   QList<MashAutoAdjuster::Step> steps{
      makeStep("Protein rest"    , MashStep::Type::Infusion , 50.0),
      makeStep("Saccharification", MashStep::Type::Decoction, 66.0),
   };
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);

   auto const result = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result.ok());

   //
   // After the strike, water and grain heat capacity = 16 + 2 = 18 kcal/°C, total (with tun) = 20 kcal/°C.
   // Proportion of mash to decoct = 20 × (66 - 50) / (18 × (100 - 50)) = 0.35556
   // Mash volume = 16 L water + 5 kg / 0.963 kg/L grain = 21.19211 L
   //
   double const ratio = 20.0 * 16.0 / (18.0 * 50.0);
   double const mashVolume_l = 16.0 + 5.0 / PhysicalConstants::grainDensity_kgL;
   QVERIFY(fuzzyComp(result.steps.at(1).amount_l, ratio * mashVolume_l, tolerance));

   // Check by heat balance: decoction at 100 °C mixed back into the rest of the mash (and tun) at 50 °C
   QVERIFY(infusionBalances(0.0, 0.0, {{ratio * 18.0, 100.0}, {(1.0 - ratio) * 18.0 + 2.0, 50.0}}, 66.0));

   // A decoction doesn't add water, so the sparge gets 30 - 16 = 14 L
   QVERIFY(fuzzyComp(result.steps.at(2).amount_l, 14.0, tolerance));

   //
   // Fixed decoction amount that is too small gives a warning
   //
   steps[1].amount_l    = 5.0;
   steps[1].amountFixed = true;
   auto const result2 = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result2.ok());
   QCOMPARE(numMessages(result2, Severity::Warning), 1);
   QCOMPARE(result2.steps.at(1).amount_l, 5.0);

   // More than the whole mash is an error
   steps[1].amount_l = 50.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0)).ok());

   // A decoction can't cool the mash
   steps[1].amountFixed = false;
   steps[1].startTemp_c = 45.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0)).ok());

   // A decoction can't be the first step
   QList<MashAutoAdjuster::Step> badSteps{makeStep("Decoction", MashStep::Type::Decoction, 66.0)};
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), badSteps, thickness(3.0)).ok());
   return;
}

void MashAutoAdjusterTests::testSparge() {
   QList<MashAutoAdjuster::Step> steps{makeStep("Saccharification", MashStep::Type::Infusion, 66.0)};

   //
   // setSparge
   //
   QCOMPARE(MashAutoAdjuster::spargeMethod(steps), MashAutoAdjuster::SpargeMethod::NoSparge);
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::BatchSparge, 2, 76.0);
   QCOMPARE(steps.size(), 3);
   QCOMPARE(MashAutoAdjuster::spargeMethod(steps), MashAutoAdjuster::SpargeMethod::BatchSparge);
   QCOMPARE(steps.at(1).type, MashStep::Type::BatchSparge);
   QCOMPARE(steps.at(2).type, MashStep::Type::BatchSparge);
   QCOMPARE(steps.at(1).startTemp_c, 76.0);
   QCOMPARE(steps.at(1).sourceIndex, -1);

   // Asking for what's already there leaves the steps alone (so fixed values are kept)
   steps[2].amountFixed = true;
   steps[2].amount_l    = 8.0;
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::BatchSparge, 2, 70.0);
   QCOMPARE(steps.size(), 3);
   QVERIFY(steps.at(2).amountFixed);
   QCOMPARE(steps.at(1).startTemp_c, 76.0);

   //
   // Batch sparge amounts and temperatures.  Total sparge water is 30 - 16 = 14 L, of which 8 L is fixed for the
   // second batch, leaving 6 L for the first.
   //
   // Drained grain bed heat capacity is 9 kcal/°C.  With 5 °C heat loss:
   //    First batch: bed at 66 - 5 = 61 °C.  Water temp = 76 + 9 × (76 - 61) / 6 = 98.5 °C
   //    Second batch: bed at 76 - 5 = 71 °C.  Water temp = 76 + 9 × (76 - 71) / 8 = 81.625 °C
   //
   MashAutoAdjuster::Options options = thickness(3.0);
   options.spargeHeatLoss_c = 5.0;
   auto const result = MashAutoAdjuster::calculate(makeInputs(), steps, options);
   QVERIFY(result.ok());
   QCOMPARE(numMessages(result, Severity::Warning), 0);
   QVERIFY(fuzzyComp(result.steps.at(1).amount_l, 6.0, tolerance));
   QVERIFY(fuzzyComp(*result.steps.at(1).infuseTemp_c, 98.5, tolerance));
   QCOMPARE(result.steps.at(2).amount_l, 8.0);
   QVERIFY(fuzzyComp(*result.steps.at(2).infuseTemp_c, 81.625, tolerance));
   QVERIFY(infusionBalances(6.0, 98.5, {{9.0, 61.0}}, 76.0));

   //
   // With the default 2 °C heat loss, two equal batches of 7 L:
   //    First batch: bed at 66 - 2 = 64 °C.  Water temp = 76 + 9 × 12 / 7 = 91.43 °C
   //    Second batch: bed at 76 - 2 = 74 °C.  Water temp = 76 + 9 × 2 / 7 = 78.57 °C
   //
   steps[2].amountFixed = false;
   auto const result2 = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result2.ok());
   QCOMPARE(numMessages(result2, Severity::Warning), 0);
   QVERIFY(fuzzyComp(result2.steps.at(1).amount_l, 7.0, tolerance));
   QVERIFY(fuzzyComp(result2.steps.at(2).amount_l, 7.0, tolerance));
   QVERIFY(fuzzyComp(*result2.steps.at(1).infuseTemp_c, 76.0 + 9.0 * 12.0 / 7.0, tolerance));
   QVERIFY(fuzzyComp(*result2.steps.at(2).infuseTemp_c, 76.0 + 9.0 *  2.0 / 7.0, tolerance));

   //
   // With a 10 °C heat loss (as the old Mash Wizard assumed), the first batch would need water at
   // 76 + 9 × 20 / 7 = 101.7 °C.  So we use boiling water, and warn that the bed only gets to
   // (7 × 100 + 9 × 56) / (7 + 9) = 75.25 °C.  The second batch then starts from that (less the heat loss), so needs
   // water at 76 + 9 × (76 - 65.25) / 7 = 89.82 °C.
   //
   options.spargeHeatLoss_c = 10.0;
   auto const result2a = MashAutoAdjuster::calculate(makeInputs(), steps, options);
   QVERIFY(result2a.ok());
   QCOMPARE(numMessages(result2a, Severity::Warning), 1);
   QVERIFY(hasMessage(result2a, Severity::Warning, "Batch Sparge 1"));
   QCOMPARE(*result2a.steps.at(1).infuseTemp_c, 100.0);
   QVERIFY(fuzzyComp(*result2a.steps.at(2).infuseTemp_c, 76.0 + 9.0 * (76.0 - 65.25) / 7.0, tolerance));

   //
   // Changing to fly sparge replaces the batch sparges with one fly sparge step, which gets all the sparge water, at
   // the target grain bed temperature
   //
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1, 77.0);
   QCOMPARE(steps.size(), 2);
   QCOMPARE(MashAutoAdjuster::spargeMethod(steps), MashAutoAdjuster::SpargeMethod::FlySparge);
   auto const result3 = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result3.ok());
   QVERIFY(fuzzyComp(result3.steps.at(1).amount_l, 14.0, tolerance));
   QCOMPARE(*result3.steps.at(1).infuseTemp_c, 77.0);

   // And to no sparge removes it
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::NoSparge, 1);
   QCOMPARE(steps.size(), 1);

   // Sparge steps must come at the end
   QList<MashAutoAdjuster::Step> badSteps{
      makeStep("Strike"  , MashStep::Type::Infusion  , 66.0),
      makeStep("Sparge"  , MashStep::Type::FlySparge , 76.0),
      makeStep("Mash out", MashStep::Type::Infusion  , 76.0),
   };
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), badSteps, thickness(3.0)).ok());
   return;
}

void MashAutoAdjusterTests::testNoSparge() {
   //
   // Single infusion: all 30 L of water goes in as strike water, regardless of thickness.  Water temp = 66 + 184 / 30
   //
   QList<MashAutoAdjuster::Step> steps{makeStep("Saccharification", MashStep::Type::Infusion, 66.0)};
   auto const result = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(result.ok());
   QVERIFY(fuzzyComp(result.steps.at(0).amount_l, 30.0, tolerance));
   QVERIFY(fuzzyComp(*result.steps.at(0).infuseTemp_c, 66.0 + 184.0 / 30.0, tolerance));

   //
   // Two infusions: the strike water comes from the thickness (16 L) and the second infusion makes up the rest (14 L).
   // Its temperature = 66 + 20 × (66 - 50) / 14
   //
   QList<MashAutoAdjuster::Step> twoSteps{
      makeStep("Protein rest"    , MashStep::Type::Infusion, 50.0),
      makeStep("Saccharification", MashStep::Type::Infusion, 66.0),
   };
   auto const result2 = MashAutoAdjuster::calculate(makeInputs(), twoSteps, thickness(3.0));
   QVERIFY(result2.ok());
   QVERIFY(fuzzyComp(result2.steps.at(0).amount_l, 16.0, tolerance));
   QVERIFY(fuzzyComp(result2.steps.at(1).amount_l, 14.0, tolerance));
   QVERIFY(fuzzyComp(*result2.steps.at(1).infuseTemp_c, 66.0 + 20.0 * 16.0 / 14.0, tolerance));

   //
   // If the second infusion's amount is fixed, the strike water makes up the rest
   //
   twoSteps[1].amount_l    = 12.0;
   twoSteps[1].amountFixed = true;
   auto const result3 = MashAutoAdjuster::calculate(makeInputs(), twoSteps, thickness(3.0));
   QVERIFY(result3.ok());
   QVERIFY(fuzzyComp(result3.steps.at(0).amount_l, 18.0, tolerance));

   // If that leaves nothing for the strike water, it's an error
   twoSteps[1].amount_l = 31.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), twoSteps, thickness(3.0)).ok());

   // If all the amounts are fixed and don't add up to the total, it's a warning
   twoSteps[0].amount_l    = 16.0;
   twoSteps[0].amountFixed = true;
   twoSteps[1].amount_l    = 12.0;
   auto const result4 = MashAutoAdjuster::calculate(makeInputs(), twoSteps, thickness(3.0));
   QVERIFY(result4.ok());
   QCOMPARE(numMessages(result4, Severity::Warning), 1);
   return;
}

void MashAutoAdjusterTests::testErrorsAndWarnings() {
   QList<MashAutoAdjuster::Step> steps{makeStep("Saccharification", MashStep::Type::Infusion, 66.0)};
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);

   // No steps
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), {}).ok());

   // No grain
   auto noGrain = makeInputs();
   noGrain.grain_kg = 0.0;
   QVERIFY(!MashAutoAdjuster::calculate(noGrain, steps).ok());

   // Bad thickness
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), steps, thickness(0.0)).ok());

   //
   // Mash too thick: 0.8 L/kg gives 5 L, needing water at 66 + 184 / 5 = 102.8 °C.  Nothing should be changed.
   //
   auto const tooThick = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(0.8));
   QVERIFY(!tooThick.ok());
   QVERIFY(hasMessage(tooThick, Severity::Error, "Saccharification"));
   QVERIFY(hasMessage(tooThick, Severity::Error, "thinner"));

   //
   // Lower boiling point (eg at altitude) makes a difference: 1.2 L/kg gives 7 L, needing water at 66 + 184 / 7 =
   // 92.3 °C, which is OK at sea level but not if water boils at 90 °C
   //
   QVERIFY(MashAutoAdjuster::calculate(makeInputs(), steps, thickness(1.2)).ok());
   auto highAltitude = makeInputs();
   highAltitude.boilingPoint_c = 90.0;
   QVERIFY(!MashAutoAdjuster::calculate(highAltitude, steps, thickness(1.2)).ok());

   //
   // Mash too thin: 6 L/kg gives 31 L of strike water, more than the 30 L needed in total.  This is a warning (as the
   // user might be happy to have extra wort), and the sparge gets no water.
   //
   auto const tooThin = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(6.0));
   QVERIFY(tooThin.ok());
   QVERIFY(hasMessage(tooThin, Severity::Warning, "no water left for sparging"));
   QCOMPARE(tooThin.steps.at(1).amount_l, 0.0);

   //
   // Mash doesn't fit in the tun: 16 L water + 5.19 L grain is more than 20 L
   //
   auto smallTun = makeInputs();
   smallTun.mashVesselVolume_l = 20.0;
   auto const overflow = MashAutoAdjuster::calculate(smallTun, steps, thickness(3.0));
   QVERIFY(overflow.ok());
   QVERIFY(hasMessage(overflow, Severity::Warning, "capacity of the mash tun"));
   // But we don't check if we don't know the tun size
   smallTun.mashVesselVolume_l = std::nullopt;
   QCOMPARE(numMessages(MashAutoAdjuster::calculate(smallTun, steps, thickness(3.0)), Severity::Warning), 0);

   // An infusion that doesn't change the temperature can't have its amount calculated
   QList<MashAutoAdjuster::Step> sameTempSteps{
      makeStep("Strike"     , MashStep::Type::Infusion, 66.0),
      makeStep("More water" , MashStep::Type::Infusion, 66.0),
   };
   MashAutoAdjuster::setSparge(sameTempSteps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), sameTempSteps, thickness(3.0)).ok());
   // But it's fine if its amount is fixed
   sameTempSteps[1].amount_l    = 2.0;
   sameTempSteps[1].amountFixed = true;
   auto const sameTemp = MashAutoAdjuster::calculate(makeInputs(), sameTempSteps, thickness(3.0));
   QVERIFY(sameTemp.ok());
   QVERIFY(fuzzyComp(*sameTemp.steps.at(1).infuseTemp_c, 66.0, tolerance));
   return;
}

void MashAutoAdjusterTests::testSensibleTemperatures() {
   QList<MashAutoAdjuster::Step> steps{makeStep("Saccharification", MashStep::Type::Infusion, 66.0)};
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);

   // Grain temperature of 0 °C (which used to be the default for a new mash) almost certainly means "not set"
   auto coldGrain = makeInputs();
   coldGrain.grainTemp_c = 0.0;
   auto const coldGrainResult = MashAutoAdjuster::calculate(coldGrain, steps, thickness(3.0));
   QVERIFY(!coldGrainResult.ok());
   QVERIFY(hasMessage(coldGrainResult, Severity::Error, "initial grain temperature"));
   // The problem is flagged against the mash's grain temperature field, so the editor can highlight it
   QCOMPARE(coldGrainResult.problems.size(), 1);
   auto const * coldGrainResultScope = std::get_if<MashAutoAdjuster::Problem::MashPropertyScope>(
      &coldGrainResult.problems.first().scope
   );
   QVERIFY(coldGrainResultScope != nullptr);
   QCOMPARE(coldGrainResultScope->mashProperty, QString{*PropertyNames::Mash::grainTemp_c});

   // Same for the tun temperature
   auto hotTun = makeInputs();
   hotTun.tunTemp_c = 120.0;
   auto const hotTunResult = MashAutoAdjuster::calculate(hotTun, steps, thickness(3.0));
   QVERIFY(!hotTunResult.ok());
   auto const * hotTunResultScope = std::get_if<MashAutoAdjuster::Problem::MashPropertyScope>(
      &hotTunResult.problems.first().scope
   );
   QVERIFY(hotTunResultScope != nullptr);
   QCOMPARE(hotTunResultScope->mashProperty, QString{*PropertyNames::Mash::initialTunTemp_c});

   //
   // Mash step temperatures have to be between freezing and boiling.  We can't guess what the user wanted, so it's an
   // error.
   //
   auto badStepTemp = steps;
   badStepTemp[0].startTemp_c = 0.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), badStepTemp, thickness(3.0)).ok());
   badStepTemp[0].startTemp_c = 100.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), badStepTemp, thickness(3.0)).ok());

   //
   // But a sparge step's temperature is just the target for the grain bed, so if it doesn't make sense, adjusting the
   // mash sets it to the sparge temperature (with a warning)...
   //
   auto badSpargeTemp = steps;
   badSpargeTemp[1].startTemp_c = 0.0;
   MashAutoAdjuster::Options options = thickness(3.0);
   options.spargeTemp_c = 77.0;
   auto const badSpargeTempResult = MashAutoAdjuster::calculate(makeInputs(), badSpargeTemp, options);
   QVERIFY(badSpargeTempResult.ok());
   QCOMPARE(badSpargeTempResult.steps.at(1).startTemp_c, 77.0);
   QCOMPARE(*badSpargeTempResult.steps.at(1).infuseTemp_c, 77.0);
   QVERIFY(hasMessage(badSpargeTempResult, Severity::Warning, "Fly Sparge"));
   // ...or, if that doesn't make sense either, the default
   options.spargeTemp_c = 0.0;
   QCOMPARE(MashAutoAdjuster::calculate(makeInputs(), badSpargeTemp, options).steps.at(1).startTemp_c,
            MashAutoAdjuster::defaultSpargeTemp_c);
   // ...whereas checking the steps as they are reports it as an error
   auto const badSpargeTempCheck = MashAutoAdjuster::check(makeInputs(), badSpargeTempResult.steps.mid(0, 1) +
                                                                        badSpargeTemp.mid(1));
   QVERIFY(!badSpargeTempCheck.ok());
   QVERIFY(hasMessage(badSpargeTempCheck, Severity::Error, "Fly Sparge"));

   //
   // Fixed amount that would need water colder than the coldest available (20 °C).  Strike to 70 °C, then cool to 60 °C
   // with a fixed 1 L: mash heat capacity is 2 + 2 + 16 = 20 kcal/°C, so we'd need water at 60 - 20 × 10 / 1 = -140 °C.
   //
   QList<MashAutoAdjuster::Step> coolingSteps{
      makeStep("Strike" , MashStep::Type::Infusion, 70.0),
      makeStep("Cooling", MashStep::Type::Infusion, 60.0),
   };
   coolingSteps[1].amount_l    = 1.0;
   coolingSteps[1].amountFixed = true;
   MashAutoAdjuster::setSparge(coolingSteps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
   auto const coolingResult = MashAutoAdjuster::calculate(makeInputs(), coolingSteps, thickness(3.0));
   QVERIFY(!coolingResult.ok());
   QVERIFY(hasMessage(coolingResult, Severity::Error, "Cooling"));
   QVERIFY(hasMessage(coolingResult, Severity::Error, "coldest water available"));
   // Without the fixed amount, it works, using water at 20 °C: 200 / (60 - 20) = 5 L
   coolingSteps[1].amountFixed = false;
   auto const coolingResult2 = MashAutoAdjuster::calculate(makeInputs(), coolingSteps, thickness(3.0));
   QVERIFY(coolingResult2.ok());
   QVERIFY(fuzzyComp(coolingResult2.steps.at(1).amount_l, 5.0, tolerance));
   QCOMPARE(*coolingResult2.steps.at(1).infuseTemp_c, 20.0);

   //
   // With no sparge, the last infusion has to make up the rest of the water.  Strike 16 L to 66 °C, then 14 L to 40 °C
   // would need water at 40 - 20 × 26 / 14 = 2.9 °C.
   //
   QList<MashAutoAdjuster::Step> noSpargeSteps{
      makeStep("Strike"  , MashStep::Type::Infusion, 66.0),
      makeStep("Make up" , MashStep::Type::Infusion, 40.0),
   };
   auto const noSpargeResult = MashAutoAdjuster::calculate(makeInputs(), noSpargeSteps, thickness(3.0));
   QVERIFY(!noSpargeResult.ok());
   QVERIFY(hasMessage(noSpargeResult, Severity::Error, "no sparge"));

   //
   // Batch sparge after a hot mash out.  Mash at 80 °C, so the drained bed is at 78 °C, which is already above the
   // 76 °C sparge temperature.  We use water at 76 °C, and warn that the bed will be at (7 × 76 + 9 × 78) / 16 = 77.1 °C.
   //
   QList<MashAutoAdjuster::Step> hotSteps{makeStep("Strike", MashStep::Type::Infusion, 80.0)};
   MashAutoAdjuster::setSparge(hotSteps, MashAutoAdjuster::SpargeMethod::BatchSparge, 2);
   auto const hotResult = MashAutoAdjuster::calculate(makeInputs(), hotSteps, thickness(3.0));
   QVERIFY(hotResult.ok());
   QCOMPARE(*hotResult.steps.at(1).infuseTemp_c, 76.0);
   QVERIFY(hasMessage(hotResult, Severity::Warning, "already hotter"));
   for (auto const & step : hotResult.steps) {
      QVERIFY(*step.infuseTemp_c >= 20.0);
      QVERIFY(*step.infuseTemp_c <= 100.0);
   }
   return;
}

void MashAutoAdjusterTests::testCheck() {
   //
   // Steps that have just been calculated are fine
   //
   QList<MashAutoAdjuster::Step> steps{
      makeStep("Protein rest"    , MashStep::Type::Infusion , 50.0),
      makeStep("Saccharification", MashStep::Type::Decoction, 66.0),
   };
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::BatchSparge, 2);
   auto const calculated = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(calculated.ok());
   auto const checked = MashAutoAdjuster::check(makeInputs(), calculated.steps);
   QVERIFY(checked.ok());
   QVERIFY2(checked.problems.isEmpty(), qPrintable(checked.messages(Severity::Warning).join("; ")));
   // Checking doesn't change anything (other than possibly filling in values that weren't set, which is not the case
   // here)
   for (qsizetype ii = 0; ii < steps.size(); ++ii) {
      QCOMPARE(checked.steps.at(ii).amount_l    , calculated.steps.at(ii).amount_l    );
      QCOMPARE(checked.steps.at(ii).infuseTemp_c, calculated.steps.at(ii).infuseTemp_c);
   }

   //
   // Strike water that's too cold to reach the step temperature is a warning on that step
   //
   auto tooCold = calculated.steps;
   *tooCold[0].infuseTemp_c -= 5.0;
   auto const tooColdResult = MashAutoAdjuster::check(makeInputs(), tooCold);
   QVERIFY(tooColdResult.ok());
   QCOMPARE(tooColdResult.problems.size(), 1);
   auto const * tooColdResultScope = std::get_if<MashAutoAdjuster::Problem::StepScope>(
      &tooColdResult.problems.first().scope
   );
   QVERIFY(tooColdResultScope != nullptr);
   QCOMPARE(tooColdResultScope->stepIndex, 0);

   //
   // Too small a decoction is a warning on that step
   //
   auto smallDecoction = calculated.steps;
   smallDecoction[1].amount_l /= 2.0;
   auto const smallDecoctionResult = MashAutoAdjuster::check(makeInputs(), smallDecoction);
   QVERIFY(smallDecoctionResult.ok());
   QCOMPARE(smallDecoctionResult.problems.size(), 1);
   auto const * smallDecoctionResultScope = std::get_if<MashAutoAdjuster::Problem::StepScope>(
      &smallDecoctionResult.problems.first().scope
   );
   QVERIFY(smallDecoctionResultScope != nullptr);
   QCOMPARE(smallDecoctionResultScope->stepIndex, 1);
   // Neither of the problems above can be pinned on one particular value in the step
   QVERIFY(tooColdResultScope->stepProperty.isEmpty());
   QVERIFY(smallDecoctionResultScope->stepProperty.isEmpty());

   //
   // A decoction with no amount is a warning on the amount of that step
   //
   auto noDecoctionAmount = calculated.steps;
   noDecoctionAmount[1].amount_l = 0.0;
   auto const noDecoctionAmountResult = MashAutoAdjuster::check(makeInputs(), noDecoctionAmount);
   QVERIFY(noDecoctionAmountResult.ok());
   QCOMPARE(noDecoctionAmountResult.problems.size(), 1);
   auto const * noDecoctionAmountResultScope = std::get_if<MashAutoAdjuster::Problem::StepScope>(
      &noDecoctionAmountResult.problems.first().scope
   );
   QVERIFY(noDecoctionAmountResultScope != nullptr);
   QCOMPARE(noDecoctionAmountResultScope->stepIndex, 1);
   QCOMPARE(noDecoctionAmountResultScope->stepProperty, QString{*PropertyNames::MashStep::amount_l});

   //
   // Water that doesn't add up is a warning about the mash as a whole
   //
   auto tooLittleWater = calculated.steps;
   tooLittleWater[2].amount_l -= 2.0;
   auto const tooLittleWaterResult = MashAutoAdjuster::check(makeInputs(), tooLittleWater);
   QVERIFY(tooLittleWaterResult.ok());
   QVERIFY(
      std::any_of(
         tooLittleWaterResult.problems.cbegin(),
         tooLittleWaterResult.problems.cend(),
         [](auto const & problem) {
            auto const * tooLittleWaterResultScope = std::get_if<MashAutoAdjuster::Problem::MashScope>(
               &problem.scope
            );
            return tooLittleWaterResultScope != nullptr;
         }
      )
   );

   //
   // A missing infusion temperature (eg from a BeerJSON file, which can't hold it) is worked out, so is not a problem
   // as long as it can be, but a missing amount is a warning
   //
   auto missingValues = calculated.steps;
   missingValues[0].infuseTemp_c = std::nullopt;
   missingValues[2].amount_l     = 0.0;
   missingValues[3].amount_l     = 0.0;
   auto const missingValuesResult = MashAutoAdjuster::check(makeInputs(), missingValues);
   QVERIFY(missingValuesResult.ok());
   QCOMPARE(missingValuesResult.problems.size(), 2);
   QVERIFY(hasMessage(missingValuesResult, Severity::Warning, "Batch Sparge 1"));
   QVERIFY(hasMessage(missingValuesResult, Severity::Warning, "No amount"));

   //
   // Water above boiling is an error
   //
   auto tooHot = calculated.steps;
   tooHot[0].infuseTemp_c = std::nullopt;
   tooHot[0].amount_l     = 2.0;
   QVERIFY(!MashAutoAdjuster::check(makeInputs(), tooHot).ok());
   return;
}

void MashAutoAdjusterTests::testRoundedValues() {
   //
   // Strike to 50 °C, then infusion with boiling water to 66 °C.  The second infusion is 320 / 34 = 9.41176 L.  If that
   // amount is stored rounded (eg as 9.41 L in a file), working the water temperature back from it gives 100.006 °C,
   // which is above boiling, but only by a meaningless amount, so should not be a problem.
   //
   QList<MashAutoAdjuster::Step> steps{
      makeStep("Protein rest"    , MashStep::Type::Infusion, 50.0),
      makeStep("Saccharification", MashStep::Type::Infusion, 66.0),
   };
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1);
   auto const calculated = MashAutoAdjuster::calculate(makeInputs(), steps, thickness(3.0));
   QVERIFY(calculated.ok());
   QCOMPARE(*calculated.steps.at(1).infuseTemp_c, 100.0);

   auto rounded = calculated.steps;
   rounded[1].amount_l     = std::round(rounded[1].amount_l * 100.0) / 100.0;
   rounded[1].infuseTemp_c = std::nullopt;
   QVERIFY(rounded[1].amount_l < calculated.steps.at(1).amount_l);

   // Checking the steps as they are is fine, and works out the water at (no more than) boiling
   auto const checked = MashAutoAdjuster::check(makeInputs(), rounded);
   QVERIFY2(checked.ok(), qPrintable(checked.messages(Severity::Error).join("; ")));
   QCOMPARE(*checked.steps.at(1).infuseTemp_c, 100.0);

   // So is adjusting the mash with the rounded amount pinned
   rounded[1].amountFixed = true;
   auto const adjusted = MashAutoAdjuster::calculate(makeInputs(), rounded, thickness(3.0));
   QVERIFY2(adjusted.ok(), qPrintable(adjusted.messages(Severity::Error).join("; ")));
   QCOMPARE(*adjusted.steps.at(1).infuseTemp_c, 100.0);

   // But something meaningfully above boiling is still an error
   rounded[1].amount_l = 9.0;
   QVERIFY(!MashAutoAdjuster::calculate(makeInputs(), rounded, thickness(3.0)).ok());
   return;
}

void MashAutoAdjusterTests::testWithRecipe() {
   double const grain_kg      = 5.0;
   double const thickness_LKg = 3.0;

   auto twoRow = UnitTests::makeFermentable_TwoRow();
   ObjectStoreWrapper::insert(twoRow);

   auto equipment = std::make_shared<Equipment>("Mash calculator test equipment");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);
   equipment->setMashTunVolume_l(40.0);
   equipment->setMashTunWeight_kg(5.0);
   equipment->setMashTunSpecificHeat_calGC(0.4);
   equipment->setGrainAbsorption_LKg(1.0);
   equipment->setFoundationVolume_l(0.0);
   equipment->setMashTunLoss_l(1.0);
   equipment->setLauterTunDeadspaceLoss_l(0.0);
   equipment->setTopUpKettle_l(0.0);
   equipment->setTopUpWater_l(0.0);
   equipment->setBoilingPoint_c(99.0);
   ObjectStoreWrapper::insert(equipment);

   auto recipe = std::make_shared<Recipe>("Mash calculator test recipe");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(23.0);
   recipe->setEquipment(equipment);
   auto boil = recipe->nonOptBoil();
   boil->setPreBoilSize_l(30.0);
   boil->setBoilTime_mins(60.0);
   auto grain = std::make_shared<RecipeAdditionFermentable>("Mash calculator test grain");
   grain->setFermentable(twoRow.get());
   grain->setStage(RecipeAddition::Stage::Mash);
   grain->setQuantity(grain_kg);
   grain->setMeasure(Measurement::PhysicalQuantity::Mass);
   recipe->addAddition(grain);

   //
   // Mash with a strike and two batch sparges, with made-up amounts
   //
   auto mash = std::make_shared<Mash>("Mash calculator test mash");
   mash->setGrainTemp_c(18.0);
   mash->setSpargeTemp_c(75.0);
   ObjectStoreWrapper::insert(mash);
   auto strike = std::make_shared<MashStep>("Strike");
   strike->setType(MashStep::Type::Infusion);
   strike->setStartTemp_c(66.0);
   strike->setStepTime_mins(60.0);
   strike->setAmount_l(1.0);
   mash->add(strike);
   for (int ii = 1; ii <= 2; ++ii) {
      auto sparge = std::make_shared<MashStep>(QString("Sparge %1").arg(ii));
      sparge->setType(MashStep::Type::BatchSparge);
      sparge->setStartTemp_c(75.0);
      sparge->setStepTime_mins(15.0);
      sparge->setAmount_l(1.0);
      mash->add(sparge);
   }
   recipe->setMash(mash);

   //
   // Inputs come from the recipe, equipment and mash
   //
   auto const inputs = MashAutoAdjuster::Inputs::fromRecipe(*recipe);
   QVERIFY(fuzzyComp(inputs.grain_kg              , recipe->grainsInMash_kg()          , tolerance));
   QVERIFY(fuzzyComp(inputs.grainTemp_c           , 18.0                               , tolerance));
   QVERIFY(fuzzyComp(inputs.tunHeatCapacity_kcalC , 2.0                                , tolerance));
   QVERIFY(fuzzyComp(inputs.tunTemp_c             , 18.0                               , tolerance));
   QVERIFY(fuzzyComp(inputs.boilingPoint_c        , 99.0                               , tolerance));
   QVERIFY(fuzzyComp(inputs.grainAbsorption_LKg   , 1.0                                , tolerance));
   QVERIFY(fuzzyComp(inputs.strikeWaterAllowance_l, equipment->strikeWaterAllowance_l(), tolerance));
   QVERIFY(fuzzyComp(inputs.targetTotalWater_l    , recipe->targetTotalMashVol_l()     , tolerance));
   QVERIFY(inputs.mashVesselVolume_l);

   auto steps = MashAutoAdjuster::stepsFrom(*mash);
   QCOMPARE(steps.size(), 3);
   QCOMPARE(MashAutoAdjuster::spargeMethod(steps), MashAutoAdjuster::SpargeMethod::BatchSparge);
   QCOMPARE(steps.at(2).sourceIndex, 2);

   //
   // Switch to fly sparge and calculate
   //
   MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::FlySparge, 1, *mash->spargeTemp_c());
   auto const result = MashAutoAdjuster::calculate(inputs, steps, MashAutoAdjuster::Options{.thickness_LKg = thickness_LKg});
   QVERIFY(result.ok());

   //
   // Apply the results to a copy of the mash for this recipe (as the Mash Wizard does when the mash is shared or
   // locked), and check the original is unchanged
   //
   mash->setLocked(true);
   std::shared_ptr<Mash> copy = RecipeUtils::useCopyFor<Mash>(*recipe);
   QVERIFY(copy);
   QVERIFY(copy.get() != mash.get());
   QCOMPARE(recipe->mash().get(), copy.get());
   MashAutoAdjuster::apply(result.steps, *copy);

   QCOMPARE(mash->mashSteps().size(), 3);
   QCOMPARE(mash->mashSteps().at(0)->amount_l(), 1.0);

   auto const copySteps = copy->mashSteps();
   QCOMPARE(copySteps.size(), 2);
   QCOMPARE(copySteps.at(0)->name(), QString("Strike"));
   QCOMPARE(copySteps.at(1)->type(), MashStep::Type::FlySparge);

   double const strike_l = recipe->targetStrikeWater_l(thickness_LKg);
   QVERIFY(fuzzyComp(copySteps.at(0)->amount_l(), strike_l, tolerance));
   QVERIFY(copySteps.at(0)->infuseTemp_c());
   QVERIFY(infusionBalances(strike_l, *copySteps.at(0)->infuseTemp_c(), {{inputs.grain_kg * PhysicalConstants::grainSpecificHeat_calGC, 18.0}, {2.0, 18.0}}, 66.0));
   QVERIFY(fuzzyComp(copySteps.at(1)->amount_l(), recipe->targetTotalMashVol_l() - strike_l, tolerance));
   QVERIFY(fuzzyComp(copy->totalMashWater_l(), recipe->targetTotalMashVol_l(), tolerance));
   QCOMPARE(*copySteps.at(1)->infuseTemp_c(), 75.0);
   QCOMPARE(copySteps.at(1)->startTemp_c(), 75.0);
   return;
}

void MashAutoAdjusterTests::testExampleProfiles() {
   QString const filePath{QDir{CONFIG_TEST_DATA_DIR}.filePath("../data/DefaultContent006-MashAndWaterProfiles.beer")};
   QString userMessage;
   QTextStream userMessageAsStream{&userMessage};
   bool const imported = DotBeer::import("", filePath, userMessageAsStream);
   userMessageAsStream.flush();
   QVERIFY2(imported, qPrintable(userMessage));

   QStringList const names{
      "Single Infusion, Light Body, Batch Sparge",
      "Single Infusion, Medium Body, Batch Sparge",
      "Single Infusion, Full Body, Batch Sparge",
      "Single Infusion, Medium Body, Fly Sparge",
      "Single Infusion, Medium Body, No Sparge",
      "Step Mash with Protein Rest, Fly Sparge",
      "Step Mash, Direct Heat, No Sparge",
      "Double Decoction, Fly Sparge",
   };
   for (auto const & name : names) {
      qDebug() << Q_FUNC_INFO << "Checking" << name;
      auto const mash = UnitTests::findByName<Mash>(name);
      QVERIFY2(mash, qPrintable(name));
      QVERIFY2(mash->locked(), qPrintable(name));
      QVERIFY2(mash->mashSteps().size() >= 2, qPrintable(name));

      //
      // The profiles don't have water amounts -- they need to be calculated for a recipe.  Here we check that works
      // without errors or warnings for a typical recipe.  (We use the defaults for the thickness, so that any thickness
      // stored in the profile is used.)
      //
      auto const result = MashAutoAdjuster::calculate(makeInputs(), MashAutoAdjuster::stepsFrom(*mash));
      QVERIFY2(result.ok(), qPrintable(name + ": " + result.messages(Severity::Error).join("; ")));
      QVERIFY2(numMessages(result, Severity::Warning) == 0,
               qPrintable(name + ": " + result.messages(Severity::Warning).join("; ")));
      // All the water is accounted for
      double totalWater_l = 0.0;
      for (auto const & step : result.steps) {
         if (step.isInfusion() || step.isSparge()) {
            totalWater_l += step.amount_l;
         }
      }
      QVERIFY2(fuzzyComp(totalWater_l, makeInputs().targetTotalWater_l, tolerance), qPrintable(name));
   }
   return;
}

void MashAutoAdjusterTests::testAutoAdjust() {
   auto twoRow = UnitTests::makeFermentable_TwoRow();
   ObjectStoreWrapper::insert(twoRow);

   auto equipment = std::make_shared<Equipment>("Auto adjust test equipment");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);
   equipment->setMashTunVolume_l(40.0);
   equipment->setMashTunWeight_kg(5.0);
   equipment->setMashTunSpecificHeat_calGC(0.4);
   equipment->setGrainAbsorption_LKg(1.0);
   equipment->setBoilingPoint_c(100.0);
   ObjectStoreWrapper::insert(equipment);

   auto recipe = std::make_shared<Recipe>("Auto adjust test recipe");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(23.0);
   recipe->setEquipment(equipment);
   auto boil = recipe->nonOptBoil();
   boil->setPreBoilSize_l(30.0);
   boil->setBoilTime_mins(60.0);
   auto grain = std::make_shared<RecipeAdditionFermentable>("Auto adjust test grain");
   grain->setFermentable(twoRow.get());
   grain->setStage(RecipeAddition::Stage::Mash);
   grain->setQuantity(5.0);
   grain->setMeasure(Measurement::PhysicalQuantity::Mass);
   recipe->addAddition(grain);

   auto mash = std::make_shared<Mash>("Auto adjust test mash");
   mash->setGrainTemp_c(20.0);
   ObjectStoreWrapper::insert(mash);
   auto strike = std::make_shared<MashStep>("Strike");
   strike->setType(MashStep::Type::Infusion);
   strike->setStartTemp_c(66.0);
   strike->setStepTime_mins(60.0);
   strike->setLiquorToGristRatio_lKg(3.0);
   mash->add(strike);
   auto sparge = std::make_shared<MashStep>("Sparge");
   sparge->setType(MashStep::Type::FlySparge);
   sparge->setStartTemp_c(76.0);
   sparge->setStepTime_mins(30.0);
   mash->add(sparge);
   recipe->setMash(mash);

   auto settle = []() {
      for (int ii = 0; ii < 3; ++ii) {
         QCoreApplication::processEvents();
      }
   };

   //
   // Without auto-adjust, changing the grain bill doesn't change the mash
   //
   settle();
   QCOMPARE(strike->amount_l(), 0.0);
   grain->setQuantity(6.0);
   settle();
   QCOMPARE(strike->amount_l(), 0.0);

   //
   // Turning on auto-adjust adjusts the mash
   //
   mash->setKeptInSync(true);
   settle();
   QVERIFY(fuzzyComp(strike->amount_l(), recipe->targetStrikeWater_l(3.0), tolerance));
   QVERIFY(fuzzyComp(mash->totalMashWater_l(), recipe->targetTotalMashVol_l(), tolerance));

   //
   // Now changing the grain bill does change the mash
   //
   double const strikeFor6kg_l = strike->amount_l();
   grain->setQuantity(5.0);
   settle();
   QVERIFY(strike->amount_l() < strikeFor6kg_l);
   QVERIFY(fuzzyComp(strike->amount_l(), recipe->targetStrikeWater_l(3.0), tolerance));
   QVERIFY(fuzzyComp(mash->totalMashWater_l(), recipe->targetTotalMashVol_l(), tolerance));

   //
   // But not if the mash is locked
   //
   double const strikeFor5kg_l = strike->amount_l();
   mash->setLocked(true);
   grain->setQuantity(6.0);
   settle();
   QCOMPARE(strike->amount_l(), strikeFor5kg_l);
   mash->setLocked(false);
   settle();

   //
   // Copying the recipe gives the copy its own copy of the mash, as an auto-adjusting mash can't be shared
   //
   std::shared_ptr<Recipe> const recipeCopy = ObjectStoreWrapper::insertCopyOf(*recipe);
   QVERIFY(recipeCopy->mash());
   QVERIFY(recipeCopy->mash().get() != mash.get());
   QVERIFY(recipeCopy->mash()->keptInSync());
   QCOMPARE(recipe->mash().get(), mash.get());
   QCOMPARE(RecipeUtils::recipesUsing(*mash).size(), 1);

   //
   // Whereas a mash that doesn't auto-adjust is shared as normal
   //
   recipeCopy->mash()->setKeptInSync(false);
   std::shared_ptr<Recipe> const recipeCopy2 = ObjectStoreWrapper::insertCopyOf(*recipeCopy);
   QCOMPARE(recipeCopy2->mash().get(), recipeCopy->mash().get());
   return;
}
void MashAutoAdjusterTests::testUndo() {
   auto settle = []() {
      for (int ii = 0; ii < 3; ++ii) {
         QCoreApplication::processEvents();
      }
   };

   auto twoRow = UnitTests::makeFermentable_TwoRow();
   ObjectStoreWrapper::insert(twoRow);

   auto equipment = std::make_shared<Equipment>("Undo test equipment");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);
   equipment->setMashTunVolume_l(40.0);
   equipment->setMashTunWeight_kg(5.0);
   equipment->setMashTunSpecificHeat_calGC(0.4);
   equipment->setGrainAbsorption_LKg(1.0);
   equipment->setBoilingPoint_c(100.0);
   ObjectStoreWrapper::insert(equipment);

   auto recipe = std::make_shared<Recipe>("Undo test recipe");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(23.0);
   recipe->setEquipment(equipment);
   auto boil = recipe->nonOptBoil();
   boil->setPreBoilSize_l(30.0);
   boil->setBoilTime_mins(60.0);
   auto grain = std::make_shared<RecipeAdditionFermentable>("Undo test grain");
   grain->setFermentable(twoRow.get());
   grain->setStage(RecipeAddition::Stage::Mash);
   grain->setQuantity(5.0);
   grain->setMeasure(Measurement::PhysicalQuantity::Mass);
   recipe->addAddition(grain);

   auto mash = std::make_shared<Mash>("Undo test mash");
   mash->setGrainTemp_c(20.0);
   ObjectStoreWrapper::insert(mash);
   auto strike = std::make_shared<MashStep>("Strike");
   strike->setType(MashStep::Type::Infusion);
   strike->setStartTemp_c(66.0);
   strike->setStepTime_mins(60.0);
   strike->setLiquorToGristRatio_lKg(3.0);
   strike->setAmount_l(12.3);
   mash->add(strike);
   auto sparge = std::make_shared<MashStep>("Sparge");
   sparge->setType(MashStep::Type::FlySparge);
   sparge->setStartTemp_c(76.0);
   sparge->setStepTime_mins(30.0);
   sparge->setAmount_l(4.5);
   mash->add(sparge);
   recipe->setMash(mash);
   settle();

   //
   // Switching from fly sparge to batch sparge (in two batches) removes the fly sparge step and adds two batch sparge
   // steps, as well as changing the amounts.
   //
   auto makeCommand = [&]() {
      QList<MashAutoAdjuster::Step> steps = MashAutoAdjuster::stepsFrom(*mash);
      MashAutoAdjuster::setSparge(steps, MashAutoAdjuster::SpargeMethod::BatchSparge, 2, 76.0);
      MashAutoAdjuster::Result const result =
         MashAutoAdjuster::calculate(MashAutoAdjuster::Inputs::fromRecipe(*recipe, *mash), steps, {});
      if (!result.ok()) {
         qCritical() << Q_FUNC_INFO << result.messages(MashAutoAdjuster::Problem::Severity::Error);
      }
      auto * command = new UndoableAutoAdjust(
         "Test auto-adjust",
         [mash]() { return mash->keptInSync(); },
         [mash](bool const val) { mash->setKeptInSync(val); return; }
      );
      MashAutoAdjuster::makeApplyCommand(result.steps, mash, command);
      return command;
   };

   QList<std::shared_ptr<MashStep>> const stepsBefore = mash->mashSteps();
   QCOMPARE(stepsBefore.size(), 2);

   QUndoStack undoStack;
   undoStack.push(makeCommand());
   QList<std::shared_ptr<MashStep>> const stepsAfter = mash->mashSteps();
   QCOMPARE(stepsAfter.size(), 3);
   QCOMPARE(stepsAfter.at(0), strike);
   QCOMPARE(stepsAfter.at(1)->type(), MashStep::Type::BatchSparge);
   QCOMPARE(stepsAfter.at(2)->type(), MashStep::Type::BatchSparge);
   double const strikeAfter_l = strike->amount_l();
   QVERIFY(strikeAfter_l != 12.3);
   QVERIFY(fuzzyComp(mash->totalMashWater_l(), recipe->targetTotalMashVol_l(), tolerance));
   double const batch1After_l = stepsAfter.at(1)->amount_l();

   //
   // Undo puts back the same step objects, in the same order, with the same values
   //
   undoStack.undo();
   QCOMPARE(mash->mashSteps(), stepsBefore);
   QCOMPARE(strike->amount_l(), 12.3);
   QCOMPARE(sparge->amount_l(), 4.5);
   QCOMPARE(sparge->type(), MashStep::Type::FlySparge);
   QVERIFY(sparge->key() > 0);
   QCOMPARE(sparge->ownerId(), mash->key());

   //
   // Redo puts back the same new steps
   //
   undoStack.redo();
   QCOMPARE(mash->mashSteps(), stepsAfter);
   QCOMPARE(strike->amount_l(), strikeAfter_l);
   QCOMPARE(stepsAfter.at(1)->amount_l(), batch1After_l);

   // And we can go round again
   undoStack.undo();
   QCOMPARE(mash->mashSteps(), stepsBefore);
   undoStack.redo();
   QCOMPARE(mash->mashSteps(), stepsAfter);

   //
   // If keep in sync is on when the user undoes an auto-adjust, undo turns it off (so the restored values don't get
   // overwritten) and redo turns it back on
   //
   undoStack.undo();
   mash->setKeptInSync(true);
   settle();
   auto * command = makeCommand();
   undoStack.push(command);
   QCOMPARE(mash->mashSteps().size(), 3);
   undoStack.undo();
   QVERIFY(!mash->keptInSync());
   QVERIFY(command->undoTurnedOffKeepInSync());
   QCOMPARE(mash->mashSteps().size(), 2);
   // With keep in sync off, a change to the grain bill doesn't re-adjust the mash
   double const strikeAfterUndo_l = strike->amount_l();
   grain->setQuantity(6.0);
   settle();
   QCOMPARE(strike->amount_l(), strikeAfterUndo_l);
   undoStack.redo();
   QVERIFY(mash->keptInSync());
   QCOMPARE(mash->mashSteps().size(), 3);
   mash->setKeptInSync(false);
   undoStack.clear();
   return;
}
