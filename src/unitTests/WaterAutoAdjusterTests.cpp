/*======================================================================================================================
 * unitTests/WaterAutoAdjusterTests.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
#include "unitTests/WaterAutoAdjusterTests.h"

#include <QDebug>
#include <QDir>
#include <QTextStream>
#include <QtTest/QtTest>
#include <QUndoStack>

#include "WaterAutoAdjuster.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "measurement/PhysicalConstants.h"
#include "measurement/Unit.h"
#include "model/Fermentable.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionMisc.h"
#include "model/Water.h"
#include "undoRedo/SimpleUndoableUpdate.h"
#include "serialization/json/dotBeer/DotBeer.h"
#include "undoRedo/UndoableAutoAdjust.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(WaterAutoAdjusterTests)

namespace {
   using UnitTests::fuzzyComp;
   using Ion  = Water::MineralIon;
   using Salt = Misc::WaterAgentType;

   //
   // Ions added by one gram of each salt in one liter of water, worked out by hand from the molar masses in
   // measurement/PhysicalConstants.h.  Eg gypsum is CaSO₄·2H₂O, with molar mass 40.0784 + 96.0584 + 2 × 18.01514 =
   // 172.16708 g/mol, so 1 g/L gives 40.0784 ÷ 172.16708 × 1000 = 232.788 mg/L of calcium.  These agree with the
   // figures in the usual brewing references (eg 1 g/L of gypsum giving 233 ppm Ca and 558 ppm SO₄).
   //
   double constexpr gypsum_Ca      = 232.788;
   double constexpr gypsum_SO4     = 557.937;
   double constexpr cacl2Flake_Ca  = 272.626;
   double constexpr cacl2Flake_Cl  = 482.284;
   double constexpr epsom_Mg       =  98.613;
   double constexpr epsom_SO4      = 389.737;
   double constexpr mgcl2_Mg       = 119.556;
   double constexpr mgcl2_Cl       = 348.752;
   double constexpr salt_Na        = 393.393;
   double constexpr salt_Cl        = 606.607;
   double constexpr bakingSoda_Na  = 273.667;
   double constexpr bakingSoda_HCO3= 726.333;

   double constexpr chemistryTolerance = 0.01;

   //! Tolerance for amounts of salts, in grams
   double constexpr amountTolerance_g = 0.001;

   //! Tolerance for ion concentrations, in ppm
   double constexpr ppmTolerance = 0.5;

   /**
    * \brief Inputs for the given volume of RO water with no target set.  Callers then set the target and salts.
    */
   WaterAutoAdjuster::Inputs roWater(double const totalWater_l) {
      WaterAutoAdjuster::Inputs inputs;
      inputs.totalWater_l      = totalWater_l;
      inputs.baseWaterFraction = 0.0;
      return inputs;
   }

   bool allNonNegative(WaterAutoAdjuster::Result const & result) {
      for (auto it = result.amounts_g.cbegin(); it != result.amounts_g.cend(); ++it) {
         if (it.value() < 0.0) {
            qCritical() << Q_FUNC_INFO << it.key() << "has negative amount" << it.value();
            return false;
         }
      }
      return true;
   }

   bool predictedMatchesTarget(WaterAutoAdjuster::Inputs const & inputs, WaterAutoAdjuster::Result const & result) {
      bool matches = true;
      for (auto const ion : WaterAutoAdjuster::ions) {
         if (!fuzzyComp(result.predicted_ppm.value(ion), inputs.target_ppm.value(ion), ppmTolerance)) {
            matches = false;
         }
      }
      return matches;
   }
}

WaterAutoAdjusterTests::WaterAutoAdjusterTests() = default;

WaterAutoAdjusterTests::~WaterAutoAdjusterTests() = default;

void WaterAutoAdjusterTests::testSaltChemistry() {
   auto check = [](Ion const ion, Salt const salt, double const expected) {
      return fuzzyComp(WaterAutoAdjuster::concentrationPerGramPerLiter_ppm(ion, salt), expected, chemistryTolerance);
   };
   QVERIFY(check(Ion::Calcium    , Salt::CalciumSulfate_Dihydrate     , gypsum_Ca      ));
   QVERIFY(check(Ion::Sulfate    , Salt::CalciumSulfate_Dihydrate     , gypsum_SO4     ));
   QVERIFY(check(Ion::Calcium    , Salt::CalciumChloride_Dihydrate    , cacl2Flake_Ca  ));
   QVERIFY(check(Ion::Chloride   , Salt::CalciumChloride_Dihydrate    , cacl2Flake_Cl  ));
   QVERIFY(check(Ion::Magnesium  , Salt::MagnesiumSulfate_Heptahydrate, epsom_Mg       ));
   QVERIFY(check(Ion::Sulfate    , Salt::MagnesiumSulfate_Heptahydrate, epsom_SO4      ));
   QVERIFY(check(Ion::Magnesium  , Salt::MagnesiumChloride_Hexahydrate, mgcl2_Mg       ));
   QVERIFY(check(Ion::Chloride   , Salt::MagnesiumChloride_Hexahydrate, mgcl2_Cl       ));
   QVERIFY(check(Ion::Sodium     , Salt::SodiumChloride               , salt_Na        ));
   QVERIFY(check(Ion::Chloride   , Salt::SodiumChloride               , salt_Cl        ));
   QVERIFY(check(Ion::Sodium     , Salt::SodiumBicarbonate            , bakingSoda_Na  ));
   QVERIFY(check(Ion::Bicarbonate, Salt::SodiumBicarbonate            , bakingSoda_HCO3));

   // Salts only add the ions they contain.  (Magnesium chloride used to hit Q_UNREACHABLE when asked for calcium.)
   QVERIFY(check(Ion::Calcium    , Salt::MagnesiumChloride_Hexahydrate, 0.0));
   QVERIFY(check(Ion::Sulfate    , Salt::CalciumChloride_Dihydrate    , 0.0));
   QVERIFY(check(Ion::Chloride   , Salt::CalciumSulfate_Dihydrate     , 0.0));
   QVERIFY(check(Ion::Calcium    , Salt::LacticAcid                   , 0.0));

   // Ca(OH)₂ = 40.0784 + 2 × (15.9991 + 1.00802) = 74.093
   QVERIFY(fuzzyComp(MolarMass::CalciumHydroxide, 74.093, chemistryTolerance));

   QVERIFY( WaterAutoAdjuster::canCalculate(Salt::CalciumSulfate_Dihydrate));
   QVERIFY( WaterAutoAdjuster::canCalculate(Salt::CalciumChloride_Anhydrous));
   QVERIFY(!WaterAutoAdjuster::canCalculate(Salt::CalciumCarbonate));
   QVERIFY(!WaterAutoAdjuster::canCalculate(Salt::LacticAcid));
   QVERIFY(!WaterAutoAdjuster::canCalculate(Salt::Other));
   for (auto const salt : WaterAutoAdjuster::defaultSalts) {
      QVERIFY(WaterAutoAdjuster::canCalculate(salt));
   }
   return;
}

void WaterAutoAdjusterTests::testExactAmounts() {
   //
   // 4 g gypsum and 2 g calcium chloride flake in 25 L of RO water gives:
   //    Ca  = (4 × 232.788 + 2 × 272.626) ÷ 25 = 59.056 ppm
   //    SO₄ = 4 × 557.937 ÷ 25                 = 89.270 ppm
   //    Cl  = 2 × 482.284 ÷ 25                 = 38.583 ppm
   //
   WaterAutoAdjuster::Inputs inputs = roWater(25.0);
   inputs.target_ppm[Ion::Calcium ] = 59.056;
   inputs.target_ppm[Ion::Sulfate ] = 89.270;
   inputs.target_ppm[Ion::Chloride] = 38.583;
   inputs.salts = {Salt::CalciumSulfate_Dihydrate, Salt::CalciumChloride_Dihydrate};

   WaterAutoAdjuster::Result const result = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(result.ok());
   QCOMPARE(result.amounts_g.size(), 2);
   QVERIFY(fuzzyComp(result.amounts_g.value(Salt::CalciumSulfate_Dihydrate ), 4.0, amountTolerance_g));
   QVERIFY(fuzzyComp(result.amounts_g.value(Salt::CalciumChloride_Dihydrate), 2.0, amountTolerance_g));
   QVERIFY(predictedMatchesTarget(inputs, result));
   QVERIFY(result.warnings.isEmpty());
   return;
}

void WaterAutoAdjusterTests::testDefaultSalts() {
   //
   // Make a target from a known set of additions.  With all the default salts, there is more than one way to hit it
   // exactly (eg gypsum + magnesium chloride adds the same ions as calcium chloride + Epsom salt), so we just check
   // that it is hit.
   //
   WaterAutoAdjuster::Inputs inputs = roWater(20.0);
   inputs.target_ppm = WaterAutoAdjuster::predict(inputs, {{Salt::CalciumSulfate_Dihydrate     , 5.0},
                                                         {Salt::CalciumChloride_Dihydrate    , 3.0},
                                                         {Salt::MagnesiumSulfate_Heptahydrate, 2.0},
                                                         {Salt::SodiumChloride               , 1.0},
                                                         {Salt::SodiumBicarbonate            , 1.5}});
   inputs.salts = WaterAutoAdjuster::defaultSalts;

   WaterAutoAdjuster::Result const result = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(result.ok());
   QCOMPARE(result.amounts_g.size(), WaterAutoAdjuster::defaultSalts.size());
   QVERIFY(allNonNegative(result));
   QVERIFY(predictedMatchesTarget(inputs, result));
   QVERIFY(result.warnings.isEmpty());

   // Duplicate salts are ignored
   inputs.salts.append(Salt::CalciumSulfate_Dihydrate);
   WaterAutoAdjuster::Result const resultWithDuplicate = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(resultWithDuplicate.ok());
   QCOMPARE(resultWithDuplicate.amounts_g.size(), WaterAutoAdjuster::defaultSalts.size());
   QVERIFY(predictedMatchesTarget(inputs, resultWithDuplicate));

   // Interchangeable salts (both forms of calcium chloride) don't stop us finding an answer
   inputs.salts = {Salt::CalciumChloride_Anhydrous, Salt::CalciumChloride_Dihydrate};
   inputs.target_ppm = {{Ion::Calcium , 3.0 * cacl2Flake_Ca / 20.0},
                        {Ion::Chloride, 3.0 * cacl2Flake_Cl / 20.0}};
   WaterAutoAdjuster::Result const resultInterchangeable = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(resultInterchangeable.ok());
   QVERIFY(allNonNegative(resultInterchangeable));
   QVERIFY(predictedMatchesTarget(inputs, resultInterchangeable));
   // We prefer to use just one of them
   QVERIFY(resultInterchangeable.amounts_g.value(Salt::CalciumChloride_Anhydrous) == 0.0 ||
           resultInterchangeable.amounts_g.value(Salt::CalciumChloride_Dihydrate) == 0.0);
   return;
}

void WaterAutoAdjusterTests::testBaseWaterAndFixedAdditions() {
   //
   // Half the water is base water (so it contributes half its ion concentrations), and there is an addition we're not
   // changing that contributes 5 ppm of calcium.  On top of this, we want the same additions as in testExactAmounts
   // (but in 25 L).
   //
   WaterAutoAdjuster::Inputs inputs;
   inputs.totalWater_l      = 25.0;
   inputs.baseWaterFraction = 0.5;
   inputs.base_ppm = {{Ion::Calcium    ,  50.0},
                      {Ion::Magnesium  ,  10.0},
                      {Ion::Sodium     ,  15.0},
                      {Ion::Chloride   ,  20.0},
                      {Ion::Sulfate    ,  30.0},
                      {Ion::Bicarbonate, 100.0}};
   inputs.fixed_ppm = {{Ion::Calcium, 5.0}};
   inputs.target_ppm = {{Ion::Calcium    , 25.0 + 5.0 + 59.056},
                        {Ion::Magnesium  ,  5.0                },
                        {Ion::Sodium     ,  7.5                },
                        {Ion::Chloride   , 10.0 + 38.583       },
                        {Ion::Sulfate    , 15.0 + 89.270       },
                        {Ion::Bicarbonate, 50.0                }};
   inputs.salts = {Salt::CalciumSulfate_Dihydrate, Salt::CalciumChloride_Dihydrate};

   WaterAutoAdjuster::Result const result = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(result.ok());
   QVERIFY(fuzzyComp(result.amounts_g.value(Salt::CalciumSulfate_Dihydrate ), 4.0, amountTolerance_g));
   QVERIFY(fuzzyComp(result.amounts_g.value(Salt::CalciumChloride_Dihydrate), 2.0, amountTolerance_g));
   QVERIFY(predictedMatchesTarget(inputs, result));
   QVERIFY(result.warnings.isEmpty());

   // With no salts added, predict just gives the starting water
   auto const startingWater = WaterAutoAdjuster::predict(inputs, {});
   QVERIFY(fuzzyComp(startingWater.value(Ion::Calcium    ), 30.0, ppmTolerance));
   QVERIFY(fuzzyComp(startingWater.value(Ion::Bicarbonate), 50.0, ppmTolerance));
   return;
}

void WaterAutoAdjusterTests::testUnreachableTarget() {
   //
   // Lots of calcium but no sulfate or chloride can't be done, as every calcium salt adds one or the other.  We should
   // get some way towards it, and be told why we didn't get all the way.
   //
   WaterAutoAdjuster::Inputs inputs = roWater(20.0);
   inputs.target_ppm = {{Ion::Calcium, 100.0}};
   inputs.salts = WaterAutoAdjuster::defaultSalts;
   WaterAutoAdjuster::Result const calciumOnly = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(calciumOnly.ok());
   QVERIFY(allNonNegative(calciumOnly));
   QVERIFY(calciumOnly.predicted_ppm.value(Ion::Calcium) < 100.0);
   QVERIFY(!calciumOnly.warnings.isEmpty());

   //
   // Base water with more bicarbonate than the target: we can't take it away, so we don't add baking soda, and we
   // suggest diluting
   //
   inputs.baseWaterFraction = 1.0;
   inputs.base_ppm   = {{Ion::Bicarbonate, 200.0}};
   inputs.target_ppm = {{Ion::Bicarbonate,  50.0}};
   WaterAutoAdjuster::Result const tooAlkaline = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(tooAlkaline.ok());
   for (auto it = tooAlkaline.amounts_g.cbegin(); it != tooAlkaline.amounts_g.cend(); ++it) {
      QCOMPARE(it.value(), 0.0);
   }
   QCOMPARE(tooAlkaline.warnings.size(), 1);
   QVERIFY(tooAlkaline.warnings.at(0).contains("RO"));

   //
   // If none of the salts adds an ion, we say so
   //
   inputs = roWater(20.0);
   inputs.target_ppm = {{Ion::Calcium, 50.0}, {Ion::Sulfate, 120.0}, {Ion::Magnesium, 20.0}};
   inputs.salts = {Salt::CalciumSulfate_Dihydrate};
   WaterAutoAdjuster::Result const noMagnesium = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(noMagnesium.ok());
   QVERIFY(std::any_of(noMagnesium.warnings.cbegin(),
                       noMagnesium.warnings.cend(),
                       [](QString const & warning) { return warning.contains("None of the salts"); }));
   QVERIFY(std::none_of(noMagnesium.warnings.cbegin(),
                        noMagnesium.warnings.cend(),
                        [](QString const & warning) { return warning.contains("chalk"); }));

   //
   // A Dublin-like profile, high in calcium and bicarbonate but not sulfate or chloride, can't be matched with the salts
   // we calculate, so we suggest chalk or pickling lime
   //
   inputs = roWater(20.0);
   inputs.target_ppm = {{Ion::Calcium , 110.0}, {Ion::Magnesium,  4.0}, {Ion::Sodium     ,  12.0},
                        {Ion::Chloride,  19.0}, {Ion::Sulfate  , 53.0}, {Ion::Bicarbonate, 280.0}};
   inputs.salts = WaterAutoAdjuster::defaultSalts;
   WaterAutoAdjuster::Result const dublin = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(dublin.ok());
   QVERIFY(allNonNegative(dublin));
   QVERIFY(std::any_of(dublin.warnings.cbegin(),
                       dublin.warnings.cend(),
                       [](QString const & warning) { return warning.contains("chalk"); }));
   return;
}

void WaterAutoAdjusterTests::testMinimumAddition() {
   //
   // 0.05 g of table salt in 20 L gives Na = 0.05 × 393.393 ÷ 20 = 0.983 ppm and Cl = 0.05 × 606.607 ÷ 20 = 1.517 ppm
   //
   WaterAutoAdjuster::Inputs inputs = roWater(20.0);
   inputs.target_ppm = {{Ion::Sodium, 0.983}, {Ion::Chloride, 1.517}};
   inputs.salts = {Salt::SodiumChloride};

   inputs.minAddition_g = 0.1;
   WaterAutoAdjuster::Result const dropped = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(dropped.ok());
   QCOMPARE(dropped.amounts_g.value(Salt::SodiumChloride), 0.0);
   // Being 1 or 2 ppm out isn't worth warning about
   QVERIFY(dropped.warnings.isEmpty());

   inputs.minAddition_g = 0.01;
   WaterAutoAdjuster::Result const kept = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(kept.ok());
   QVERIFY(fuzzyComp(kept.amounts_g.value(Salt::SodiumChloride), 0.05, amountTolerance_g));
   return;
}

void WaterAutoAdjusterTests::testErrors() {
   WaterAutoAdjuster::Inputs inputs = roWater(0.0);
   inputs.target_ppm = {{Ion::Calcium, 50.0}};
   inputs.salts = WaterAutoAdjuster::defaultSalts;
   QVERIFY(!WaterAutoAdjuster::calculate(inputs).ok());

   inputs.totalWater_l = 20.0;
   inputs.salts = {Salt::LacticAcid, Salt::CalciumCarbonate, Salt::Other};
   QVERIFY(!WaterAutoAdjuster::calculate(inputs).ok());

   inputs.salts = {};
   QVERIFY(!WaterAutoAdjuster::calculate(inputs).ok());
   return;
}

namespace {
   //
   // Acid figures worked out by hand.  Eg lactic acid (C₃H₆O₃) has molar mass 3 × 12.0112 + 6 × 1.00802 + 3 × 15.9991
   // = 90.07902 g/mol.  88% lactic acid has density 1.206 g/mL, so 1 mL contains 1.206 × 0.88 = 1.06128 g of lactic
   // acid, which is 1061.28 ÷ 90.07902 = 11.7817 mmol, and, as lactic acid gives up one proton at mash pH, 11.7817 mEq.
   //
   double constexpr lactic88_mEqPerMl       = 11.7817;
   double constexpr phosphoric10_mEqPerMl   =  1.0746;
   double constexpr sulfuric10_mEqPerMl     =  2.1739; // Two protons
   double constexpr hydrochloric10_mEqPerMl =  2.8745;
   // Interpolated between 75% (1.177) and 88% (1.206)
   double constexpr lactic80_density_gPerMl = 1.18815;

   double constexpr acidTolerance = 0.001;
   double constexpr phTolerance   = 0.0001;

   /**
    * \brief Inputs for a simple mash: 5 kg of base malt (3 SRM) giving a 12 °P, 6 SRM beer, with 15 L of mash water and
    *        15 L of sparge water.  The distilled water mash pH is 5.6 − 0.21 × 6 ÷ 12 = 5.495.
    */
   WaterAutoAdjuster::Inputs simpleMash() {
      WaterAutoAdjuster::Inputs inputs = roWater(30.0);
      inputs.mashWater_l           = 15.0;
      inputs.mashBaseWaterFraction = 0.0;
      inputs.grains        = {WaterAutoAdjuster::Grain{.amount_kg = 5.0, .color_srm = 3.0, .isSpecialty = false}};
      inputs.og_plato      = 12.0;
      inputs.beerColor_srm =  6.0;
      return inputs;
   }

   /**
    * \brief As \c simpleMash, but with base water with an alkalinity of 3 mEq/L and 40.0784 ppm calcium (ie 2 mEq/L).
    *        So RA = 3 − 2 ÷ 3.5 = 2.428571 mEq/L, which raises the pH by 2.428571 × 15 ÷ (50 × 5) = 0.145714, to
    *        5.640714.
    */
   WaterAutoAdjuster::Inputs alkalineMash() {
      WaterAutoAdjuster::Inputs inputs = simpleMash();
      inputs.mashBaseWaterFraction  = 1.0;
      inputs.baseAlkalinity_mEqPerL = 3.0;
      inputs.base_ppm[Ion::Calcium] = MolarMass::Calcium;
      return inputs;
   }
   double constexpr alkalineMash_pH = 5.640714;
}

void WaterAutoAdjusterTests::testAcidChemistry() {
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidMEqPerMl(Salt::LacticAcid      , 88.0), lactic88_mEqPerMl      , acidTolerance));
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidMEqPerMl(Salt::PhosphoricAcid  , 10.0), phosphoric10_mEqPerMl  , acidTolerance));
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidMEqPerMl(Salt::SulfuricAcid    , 10.0), sulfuric10_mEqPerMl    , acidTolerance));
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidMEqPerMl(Salt::HydrochloricAcid, 10.0), hydrochloric10_mEqPerMl, acidTolerance));

   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidDensity_gPerMl(Salt::LacticAcid, 80.0), lactic80_density_gPerMl, acidTolerance));
   // Out of range strengths use the nearest end of the table; non-acids are treated as water
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidDensity_gPerMl(Salt::HydrochloricAcid, 50.0), 1.184, acidTolerance));
   QVERIFY(fuzzyComp(WaterAutoAdjuster::acidDensity_gPerMl(Salt::SodiumChloride  , 50.0), 1.0  , acidTolerance));
   QCOMPARE(WaterAutoAdjuster::acidMEqPerMl(Salt::LacticAcid    ,  0.0), 0.0);
   QCOMPARE(WaterAutoAdjuster::acidMEqPerMl(Salt::SodiumChloride, 50.0), 0.0);

   for (auto const acid : WaterAutoAdjuster::acids) {
      QVERIFY( WaterAutoAdjuster::isAcid(acid));
      QVERIFY(!WaterAutoAdjuster::canCalculate(acid));
   }
   QVERIFY(!WaterAutoAdjuster::isAcid(Salt::CalciumSulfate_Dihydrate));
   QVERIFY(!WaterAutoAdjuster::isAcid(Salt::Other));
   QCOMPARE(WaterAutoAdjuster::defaultAcidStrength_pct(Salt::LacticAcid    ), 88.0);
   QCOMPARE(WaterAutoAdjuster::defaultAcidStrength_pct(Salt::PhosphoricAcid), 10.0);
   return;
}

void WaterAutoAdjusterTests::testMashPh() {
   WaterAutoAdjuster::Inputs inputs = simpleMash();
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::distilledWaterMashPh(inputs), 5.495, phTolerance));
   // With RO water, RA is 0, so the mash pH is the distilled water mash pH
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(inputs, {}, 0.0), 5.495, phTolerance));

   // Alkaline base water raises the pH
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(alkalineMash(), {}, 0.0), alkalineMash_pH, phTolerance));

   // 25 mEq of acid lowers the pH by 25 ÷ (50 × 5) = 0.1, whether it's fixed or added
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(inputs, {}, 25.0), 5.395, phTolerance));
   inputs.fixedAcid_mEq = 25.0;
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(inputs, {}, 0.0), 5.395, phTolerance));
   inputs.fixedAcid_mEq = 0.0;

   //
   // Salts: 3 g of calcium chloride flake in 30 L gives 3 × 272.626 ÷ 30 = 27.2626 ppm calcium in all the water,
   // including the mash water.  That's 27.2626 ÷ 40.0784 × 2 = 1.360464 mEq/L, so RA = −1.360464 ÷ 3.5 = −0.388704,
   // lowering the pH by 0.388704 × 15 ÷ 250 = 0.023322.
   //
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(inputs, {{Salt::CalciumChloride_Dihydrate, 3.0}}, 0.0),
                     5.495 - 0.023322,
                     phTolerance));
   // Baking soda raises it.  1 g in 30 L is 726.333 ÷ 30 = 24.2111 ppm HCO₃⁻ = 0.396797 mEq/L → +0.023808
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::mashPh(inputs, {{Salt::SodiumBicarbonate, 1.0}}, 0.0),
                     5.495 + 0.023808,
                     phTolerance));

   //
   // Specialty malt.  4.5 kg of 3 SRM base malt and 0.5 kg of 60 SRM crystal malt, giving a 10 SRM beer.  In °L, the
   // base malt is (3 + 0.6) ÷ 1.35 = 2.6667 and the crystal (60 + 0.6) ÷ 1.35 = 44.8889, so the weighted color is
   // 0.9 × 2.6667 + 0.1 × 44.8889 = 6.8889, of which 4.4889 (65.16%) is from specialty malt.  So the pH is
   // 5.6 − (0.21 × 0.3484 + 0.06 × 0.6516) × 10 ÷ 12 = 5.506452.
   //
   inputs.beerColor_srm = 10.0;
   inputs.grains = {WaterAutoAdjuster::Grain{.amount_kg = 4.5, .color_srm =  3.0, .isSpecialty = false},
                    WaterAutoAdjuster::Grain{.amount_kg = 0.5, .color_srm = 60.0, .isSpecialty = true }};
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::distilledWaterMashPh(inputs), 5.506452, phTolerance));
   // Two specialty malts should both count.  (Previously only the last one did.)
   inputs.grains = {WaterAutoAdjuster::Grain{.amount_kg = 4.5 , .color_srm =  3.0, .isSpecialty = false},
                    WaterAutoAdjuster::Grain{.amount_kg = 0.25, .color_srm = 60.0, .isSpecialty = true },
                    WaterAutoAdjuster::Grain{.amount_kg = 0.25, .color_srm = 60.0, .isSpecialty = true }};
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::distilledWaterMashPh(inputs), 5.506452, phTolerance));

   // Can't work out pH without grain or mash water
   inputs.grains.clear();
   QVERIFY(!WaterAutoAdjuster::distilledWaterMashPh(inputs));
   QVERIFY(!WaterAutoAdjuster::mashPh(inputs, {}, 0.0));
   inputs = simpleMash();
   inputs.mashWater_l = 0.0;
   QVERIFY(!WaterAutoAdjuster::mashPh(inputs, {}, 0.0));
   return;
}

void WaterAutoAdjusterTests::testAcidAmounts() {
   //
   // To get from 5.640714 to 5.4 we need (5.640714 − 5.4) × 50 × 5 = 60.1786 mEq of acid, which is
   // 60.1786 ÷ 11.7817 = 5.1078 mL of 88% lactic acid, rounded to 5.1 mL.  That gives a pH of
   // 5.640714 − 5.1 × 11.7817 ÷ 250 = 5.40037.
   //
   WaterAutoAdjuster::Inputs inputs = alkalineMash();
   inputs.targetMash_pH    = 5.4;
   inputs.acid             = Salt::LacticAcid;
   inputs.acidStrength_pct = 88.0;
   WaterAutoAdjuster::Result const lactic = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(lactic.ok());
   QVERIFY(lactic.amounts_g.isEmpty());
   QVERIFY(fuzzyComp(lactic.acid_ml, 5.1, acidTolerance));
   QVERIFY(fuzzyComp(*lactic.predictedMash_pH, 5.40037, phTolerance));
   QVERIFY(lactic.warnings.isEmpty());

   // Acid from eg acidulated malt reduces the amount needed: (60.1786 − 20) ÷ 11.7817 = 3.4103 → 3.4 mL
   inputs.fixedAcid_mEq = 20.0;
   QVERIFY(fuzzyComp(WaterAutoAdjuster::calculate(inputs).acid_ml, 3.4, acidTolerance));
   inputs.fixedAcid_mEq = 0.0;

   // Weaker acid, more of it: 60.1786 ÷ 1.0746 = 56.0 mL of 10% phosphoric acid
   inputs.acid             = Salt::PhosphoricAcid;
   inputs.acidStrength_pct = 10.0;
   QVERIFY(fuzzyComp(WaterAutoAdjuster::calculate(inputs).acid_ml, 56.0, 0.05));

   // If the pH is already below the target, we can't add negative acid, so add none and warn
   inputs.acid             = Salt::LacticAcid;
   inputs.acidStrength_pct = 88.0;
   inputs.targetMash_pH    = 5.7;
   WaterAutoAdjuster::Result const tooLow = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(tooLow.ok());
   QCOMPARE(tooLow.acid_ml, 0.0);
   QCOMPARE(tooLow.warnings.size(), 1);
   QVERIFY(fuzzyComp(*tooLow.predictedMash_pH, alkalineMash_pH, phTolerance));

   // But if it's only just below the target, no warning
   inputs.targetMash_pH = alkalineMash_pH + WaterAutoAdjuster::warningTolerance_pH / 2.0;
   QVERIFY(WaterAutoAdjuster::calculate(inputs).warnings.isEmpty());

   //
   // Salts and acid together.  The calcium from the salts lowers the pH a little, so we need slightly less acid than
   // without them.
   //
   inputs.targetMash_pH = 5.4;
   inputs.target_ppm[Ion::Calcium ] = 3.0 * cacl2Flake_Ca / 30.0;
   inputs.target_ppm[Ion::Chloride] = 3.0 * cacl2Flake_Cl / 30.0;
   inputs.salts = {Salt::CalciumChloride_Dihydrate};
   inputs.baseWaterFraction = 0.0;
   WaterAutoAdjuster::Result const both = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(both.ok());
   QVERIFY(fuzzyComp(both.amounts_g.value(Salt::CalciumChloride_Dihydrate), 3.0, amountTolerance_g));
   // 60.1786 − 0.023322 × 250 = 54.3481 mEq → 4.6129 → 4.6 mL
   QVERIFY(fuzzyComp(both.acid_ml, 4.6, acidTolerance));
   QVERIFY(fuzzyComp(*both.predictedMash_pH, 5.4, 0.01));

   // Without grain, we can't work out the acid, but we still do the salts
   inputs.grains.clear();
   WaterAutoAdjuster::Result const noGrain = WaterAutoAdjuster::calculate(inputs);
   QVERIFY(noGrain.ok());
   QCOMPARE(noGrain.acid_ml, 0.0);
   QVERIFY(!noGrain.predictedMash_pH);
   QCOMPARE(noGrain.warnings.size(), 1);
   QVERIFY(fuzzyComp(noGrain.amounts_g.value(Salt::CalciumChloride_Dihydrate), 3.0, amountTolerance_g));
   return;
}

void WaterAutoAdjusterTests::testRecipeUndoAndKeepInSync() {
   auto settle = []() {
      for (int ii = 0; ii < 3; ++ii) {
         QCoreApplication::processEvents();
      }
   };

   //
   // Water agents
   //
   auto makeWaterAgent = [](QString const & name, Salt const type, std::optional<double> percentAcid = std::nullopt) {
      auto misc = std::make_shared<Misc>(name);
      misc->setType(Misc::Type::WaterAgent);
      misc->setWaterAgentType(type);
      misc->setWaterAgentPercentAcid(percentAcid);
      ObjectStoreWrapper::insert(misc);
      return misc;
   };
   auto gypsum = makeWaterAgent("Water auto-adjust test gypsum"      , Salt::CalciumSulfate_Dihydrate );
   auto cacl2  = makeWaterAgent("Water auto-adjust test CaCl2 flake" , Salt::CalciumChloride_Dihydrate);
   auto lactic = makeWaterAgent("Water auto-adjust test lactic acid" , Salt::LacticAcid, 88.0);

   //
   // Recipe with 5 kg of grain, 15 L of strike water and 15 L of sparge water
   //
   auto twoRow = UnitTests::makeFermentable_TwoRow();
   ObjectStoreWrapper::insert(twoRow);
   auto recipe = std::make_shared<Recipe>("Water auto-adjust test recipe");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(23.0);
   auto grain = std::make_shared<RecipeAdditionFermentable>("Water auto-adjust test grain");
   grain->setFermentable(twoRow.get());
   grain->setStage(RecipeAddition::Stage::Mash);
   grain->setQuantity(5.0);
   grain->setMeasure(Measurement::PhysicalQuantity::Mass);
   recipe->addAddition(grain);

   auto mash = std::make_shared<Mash>("Water auto-adjust test mash");
   ObjectStoreWrapper::insert(mash);
   auto strike = std::make_shared<MashStep>("Strike");
   strike->setType(MashStep::Type::Infusion);
   strike->setStartTemp_c(66.0);
   strike->setStepTime_mins(60.0);
   strike->setAmount_l(15.0);
   mash->add(strike);
   auto sparge = std::make_shared<MashStep>("Sparge");
   sparge->setType(MashStep::Type::FlySparge);
   sparge->setStartTemp_c(76.0);
   sparge->setStepTime_mins(30.0);
   sparge->setAmount_l(15.0);
   mash->add(sparge);
   recipe->setMash(mash);
   QVERIFY(fuzzyComp(mash->totalMashWater_l(), 30.0, 0.001));

   //
   // Target is what you get from 4 g gypsum and 2 g calcium chloride flake in 30 L of RO water
   //
   auto target = std::make_shared<Water>("Water auto-adjust test target");
   target->setIonConcentration_ppm(Ion::Calcium , (4.0 * gypsum_Ca + 2.0 * cacl2Flake_Ca) / 30.0);
   target->setIonConcentration_ppm(Ion::Sulfate ,  4.0 * gypsum_SO4                       / 30.0);
   target->setIonConcentration_ppm(Ion::Chloride,  2.0 * cacl2Flake_Cl                    / 30.0);
   for (auto const ion : {Ion::Magnesium, Ion::Sodium, Ion::Bicarbonate}) {
      target->setIonConcentration_ppm(ion, 0.0);
   }
   ObjectStoreWrapper::insert(target);
   recipe->setWaterTarget(target);
   recipe->setTargetMashAcidity_pH(5.3);

   //
   // The user has chosen gypsum, calcium chloride and lactic acid by adding them to the recipe
   //
   auto addWaterAgent = [&recipe](Misc & misc, Measurement::Amount const & amount) {
      auto addition = std::make_shared<RecipeAdditionMisc>(*recipe, misc);
      addition->setStage(RecipeAddition::Stage::Mash);
      addition->setAmount(amount);
      recipe->addAddition(addition);
      return addition;
   };
   auto gypsumAddition = addWaterAgent(*gypsum, Measurement::Amount{0.001, Measurement::Units::kilograms});
   auto cacl2Addition  = addWaterAgent(*cacl2 , Measurement::Amount{0.001, Measurement::Units::kilograms});
   auto lacticAddition = addWaterAgent(*lactic, Measurement::Amount{0.0  , Measurement::Units::liters   });
   settle();

   std::optional<double> const startingPh = WaterAutoAdjuster::predict(*recipe)->mash_pH;
   QVERIFY(startingPh);
   QVERIFY2(*startingPh > 5.35, qPrintable(QString("Starting pH %1").arg(*startingPh)));

   QUndoStack undoStack;
   auto newCommand = [&recipe](bool const turnOnKeepInSync) {
      auto * command = new UndoableAutoAdjust(
         "Test auto-adjust",
         [recipe]() { return recipe->waterKeptInSync(); },
         [recipe](bool const val) { recipe->setWaterKeptInSync(val); return; }
      );
      if (turnOnKeepInSync) {
         new SimpleUndoableUpdate(*recipe, TYPE_INFO(Recipe, waterKeptInSync), true, "Test turn on", command);
      }
      WaterAutoAdjuster::Preparation const preparation = WaterAutoAdjuster::prepare(*recipe, *command);
      if (!preparation.ok()) {
         qCritical() << Q_FUNC_INFO << preparation.result.error;
      }
      return command;
   };

   //
   // Click Auto-Adjust: salts match the target exactly, and enough acid is added to get the mash pH to the target
   //
   undoStack.push(newCommand(false));
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.004, 0.000001));
   QVERIFY(fuzzyComp(cacl2Addition ->amount().quantity, 0.002, 0.000001));
   double const adjustedAcid_l = lacticAddition->amount().quantity;
   QVERIFY(adjustedAcid_l > 0.0);
   QVERIFY(lacticAddition->amount().unit == &Measurement::Units::liters);
   // Acid is rounded to 0.1 mL, which, with 5 kg of grain, is within 0.1 × 11.78 ÷ 250 = 0.005 pH
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::predict(*recipe)->mash_pH, 5.3, 0.005));
   // We didn't add any salts other than the ones the user chose
   QCOMPARE(recipe->miscAdditions().size(), 3);

   //
   // Undo puts everything back as it was; redo does it again
   //
   undoStack.undo();
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.001, 0.000001));
   QVERIFY(fuzzyComp(cacl2Addition ->amount().quantity, 0.001, 0.000001));
   QCOMPARE(lacticAddition->amount().quantity, 0.0);
   QVERIFY(!recipe->waterKeptInSync());
   undoStack.redo();
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.004, 0.000001));
   QCOMPARE(lacticAddition->amount().quantity, adjustedAcid_l);

   //
   // Turn on keep in sync, which is one undoable step with the adjustment.  Undoing it turns keep in sync off again
   // (via the child command, so UndoableAutoAdjust doesn't need to do anything extra).
   //
   undoStack.undo();
   auto * turnOn = newCommand(true);
   undoStack.push(turnOn);
   QVERIFY(recipe->waterKeptInSync());
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.004, 0.000001));
   undoStack.undo();
   QVERIFY(!recipe->waterKeptInSync());
   QVERIFY(!turnOn->undoTurnedOffKeepInSync());
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.001, 0.000001));
   undoStack.redo();
   QVERIFY(recipe->waterKeptInSync());
   settle();
   QCOMPARE(undoStack.count(), 1);

   //
   // With keep in sync on, changing the grain bill re-adjusts the acid automatically, without anything going on the
   // undo stack.  (More grain means more buffering, so it takes a different amount of acid to get the same pH.)
   //
   grain->setQuantity(6.0);
   settle();
   QVERIFY(lacticAddition->amount().quantity != adjustedAcid_l);
   QVERIFY(fuzzyComp(*WaterAutoAdjuster::predict(*recipe)->mash_pH, 5.3, 0.005));
   QCOMPARE(undoStack.count(), 1);
   QVERIFY(undoStack.canUndo());

   //
   // Changing the water agents yourself doesn't trigger an automatic adjustment (otherwise undoing an auto-adjust would
   // just get redone straight away)
   //
   gypsumAddition->setAmount(Measurement::Amount{0.003, Measurement::Units::kilograms});
   settle();
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.003, 0.000001));

   //
   // Clicking Auto-Adjust while keep in sync is on, then undoing it, turns keep in sync off, so the values from before
   // the click stay put.  Redo turns it back on.
   //
   auto * click = newCommand(false);
   undoStack.push(click);
   QCOMPARE(undoStack.count(), 2);
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.004, 0.000001));
   undoStack.undo();
   QVERIFY(!recipe->waterKeptInSync());
   QVERIFY(click->undoTurnedOffKeepInSync());
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.003, 0.000001));
   grain->setQuantity(5.0);
   settle();
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.003, 0.000001));
   undoStack.redo();
   QVERIFY(recipe->waterKeptInSync());
   QVERIFY(!click->undoTurnedOffKeepInSync());
   QVERIFY(fuzzyComp(gypsumAddition->amount().quantity, 0.004, 0.000001));

   //
   // A locked recipe can't be adjusted, either by clicking or automatically
   //
   double const lockedAcid_l = lacticAddition->amount().quantity;
   recipe->setLocked(true);
   QUndoCommand dummy;
   QVERIFY(!WaterAutoAdjuster::prepare(*recipe, dummy).ok());
   QCOMPARE(dummy.childCount(), 0);
   grain->setQuantity(7.0);
   settle();
   QCOMPARE(lacticAddition->amount().quantity, lockedAcid_l);
   recipe->setLocked(false);
   recipe->setWaterKeptInSync(false);
   undoStack.clear();
   return;
}

void WaterAutoAdjusterTests::testSuggestedTargetMashPh() {
   auto recipe = std::make_shared<Recipe>("Suggested target pH test recipe");
   ObjectStoreWrapper::insert(recipe);
   QVERIFY(!WaterAutoAdjuster::suggestedTargetMashPh(*recipe));

   auto mash = std::make_shared<Mash>("Suggested target pH test mash");
   ObjectStoreWrapper::insert(mash);
   auto makeStep = [&mash](QString const & name, MashStep::Type const type) {
      auto step = std::make_shared<MashStep>(name);
      step->setType(type);
      step->setStartTemp_c(65.0);
      step->setStepTime_mins(30.0);
      mash->add(step);
      return step;
   };
   // Sparge steps don't count, even if they come first
   auto sparge = makeStep("Odd sparge", MashStep::Type::BatchSparge);
   sparge->setStartAcidity_pH(5.8);
   auto strike = makeStep("Strike", MashStep::Type::Infusion);
   auto rest   = makeStep("Rest"  , MashStep::Type::Temperature);
   recipe->setMash(mash);
   QVERIFY(!WaterAutoAdjuster::suggestedTargetMashPh(*recipe));

   // End pH is used if there's no start pH
   rest->setEndAcidity_pH(5.3);
   QCOMPARE(*WaterAutoAdjuster::suggestedTargetMashPh(*recipe), 5.3);

   // The first step with a pH wins
   strike->setStartAcidity_pH(5.4);
   QCOMPARE(*WaterAutoAdjuster::suggestedTargetMashPh(*recipe), 5.4);

   // We only ever read the mash
   QVERIFY(!recipe->targetMashAcidity_pH());
   return;
}

void WaterAutoAdjusterTests::testExampleProfiles() {
   QString const filePath{QDir{CONFIG_TEST_DATA_DIR}.filePath("../data/DefaultContent006-MashAndWaterProfiles.beer")};
   QString userMessage;
   QTextStream userMessageAsStream{&userMessage};
   bool const imported = DotBeer::import("", filePath, userMessageAsStream);
   userMessageAsStream.flush();
   QVERIFY2(imported, qPrintable(userMessage));

   QStringList const targetNames{
      "Light and malty",
      "Pale, Soft (Pilsner, Helles)",
      "Pale, Balanced (Blonde, Kölsch, Wheat)",
      "Pale, Hoppy (Pale Ale, West Coast IPA)",
      "Pale, Juicy (New England IPA)",
      "Amber, Hoppy (Bitter, ESB, Amber Ale)",
      "Amber, Balanced (Vienna, Altbier)",
      "Amber, Malty (Märzen, Scottish, Bock)",
      "Brown, Balanced (Brown Ale, Porter)",
      "Brown, Malty (Dunkel, Dubbel)",
      "Black, Dry (Dry Stout, Black IPA)",
      "Black, Full (Sweet & Imperial Stout)",
   };
   for (auto const & name : targetNames) {
      auto const water = UnitTests::findByName<Water>(name);
      QVERIFY2(water, qPrintable(name));
      QVERIFY2(!water->notes().isEmpty(), qPrintable(name));

      //
      // Each style target is designed to be reachable from RO water with the salts Auto-Adjust uses by default, so we
      // should hit every ion (within the warning tolerance) with no warnings.
      //
      WaterAutoAdjuster::Inputs inputs = roWater(30.0);
      for (auto const ion : WaterAutoAdjuster::ions) {
         inputs.target_ppm[ion] = water->ionConcentration_ppm(ion).value_or(0.0);
      }
      inputs.salts = WaterAutoAdjuster::defaultSalts;
      WaterAutoAdjuster::Result const result = WaterAutoAdjuster::calculate(inputs);
      QVERIFY2(result.ok(), qPrintable(name + ": " + result.error));
      QVERIFY2(result.warnings.isEmpty(), qPrintable(name + ": " + result.warnings.join("; ")));
      for (auto const ion : WaterAutoAdjuster::ions) {
         QVERIFY2(fuzzyComp(result.predicted_ppm.value(ion), inputs.target_ppm.value(ion), 2.0),
                  qPrintable(QString{"%1: %2 is %3, target %4"}.arg(name)
                                                              .arg(Water::ionDisplayNames[ion])
                                                              .arg(result.predicted_ppm.value(ion))
                                                              .arg(inputs.target_ppm.value(ion))));
      }
   }

   QStringList const cityNames{
      "Paris tap water (2026)",
      "Amsterdam tap water (2025)",
      "Berlin tap water (2025)",
      "Munich tap water (2026)",
      "New York City tap water (2024)",
      "Denver tap water, South system (2025)",
      "Denver tap water, North system (2025)",
      "San Diego tap water (2025)",
      "Seattle tap water (2026)",
      "San Francisco tap water (2025)",
      "Melbourne tap water (2025)",
   };
   for (auto const & name : cityNames) {
      auto const water = UnitTests::findByName<Water>(name);
      QVERIFY2(water, qPrintable(name));
      // Every city profile says where its figures came from
      QVERIFY2(water->notes().startsWith("Source: "), qPrintable(name));
      QVERIFY2(water->ionConcentration_ppm(Water::MineralIon::Calcium).value_or(0.0) > 0.0, qPrintable(name));
   }
   return;
}
