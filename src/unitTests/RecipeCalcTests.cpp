/*======================================================================================================================
 * unitTests/RecipeCalcTests.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Mattias Måhl <mattias@kejsarsten.com>
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
#include "unitTests/RecipeCalcTests.h"

#include <cmath>
#include <optional>

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtTest/QtTest>

#include "database/ObjectStoreWrapper.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "model/Boil.h"
#include "model/BoilStep.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/StockPurchaseHop.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"

REGISTER_TEST_CLASS(RecipeCalcTests)

namespace {
   using UnitTests::fuzzyComp;
}

class RecipeCalcTests::impl {
public:
   impl() = default;
   ~impl() = default;

   //================================================ MEMBER VARIABLES =================================================
   std::shared_ptr<Equipment> m_equipFiveGalNoLoss;
   //! \brief Cascade pellets at 4% AA
   std::shared_ptr<Hop>       m_cascade_4pct;
   //! \brief 70% yield, no moisture, 2 SRM
   std::shared_ptr<Fermentable> m_twoRow;
};

RecipeCalcTests::RecipeCalcTests() : UnitTests::TestClassBase(), pimpl{std::make_unique<impl>()} {
   return;
}

RecipeCalcTests::~RecipeCalcTests() = default;

void RecipeCalcTests::initTestClass() {
   this->pimpl->m_equipFiveGalNoLoss = UnitTests::makeEquipment_FiveGalNoLoss();
   this->pimpl->m_cascade_4pct       = UnitTests::makeHop_Cascade();
   this->pimpl->m_twoRow             = UnitTests::makeFermentable_TwoRow();
   return;
}

void RecipeCalcTests::recipeCalcTest_allGrain() {
   // .:TODO:. Would be good to fix and reinstate this test...
   return;
   double const grain_kg = 5.0;
   double const conversion_l = grain_kg * 2.8; // 2.8 L/kg mash thickness
   auto rec = std::make_shared<Recipe>("TestRecipe");

   // Basic recipe parameters
   rec->setBatchSize_l(this->pimpl->m_equipFiveGalNoLoss->fvBatchSize_l());
   rec->nonOptBoil()->setPreBoilSize_l(this->pimpl->m_equipFiveGalNoLoss->kettleBoilSize_l());
   rec->setEfficiency_pct(70.0);

   // Single conversion, single sparge
   auto singleConversion = std::make_shared<Mash>();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);
   auto singleConversion_convert = std::make_shared<MashStep>();
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Type::Infusion);
   singleConversion_convert->setAmount_l(conversion_l);
   singleConversion->add(singleConversion_convert);
   auto singleConversion_sparge = std::make_shared<MashStep>();
   singleConversion_sparge->setName("Sparge");
   singleConversion_sparge->setType(MashStep::Type::Infusion);
   singleConversion_sparge->setAmount_l(
      rec->boil()->preBoilSize_l().value_or(0.0)
      + this->pimpl->m_equipFiveGalNoLoss->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg) * grain_kg // Grain absorption
      - conversion_l // Water we already added
   );
   singleConversion->add(singleConversion_sparge);

   // Add equipment
   rec->setEquipment(this->pimpl->m_equipFiveGalNoLoss);

   // Add hops (85g)
   auto cascadeHopAddition = std::make_shared<RecipeAdditionHop>("Cascade 4% Hop Addition");
   cascadeHopAddition->setHop(this->pimpl->m_cascade_4pct.get());
   cascadeHopAddition->setStage(RecipeAddition::Stage::Boil);
   cascadeHopAddition->setAddAtTime_mins(60);
   cascadeHopAddition->setQuantity(0.085);
   cascadeHopAddition->setMeasure(Measurement::PhysicalQuantity::Mass);
   rec->addAddition(cascadeHopAddition);

   // Add grain
   auto twoRowFermentableAddition = std::make_shared<RecipeAdditionFermentable>("Two Row Grain Addition");
   twoRowFermentableAddition->setFermentable(this->pimpl->m_twoRow.get());
   twoRowFermentableAddition->setStage(RecipeAddition::Stage::Mash);
   twoRowFermentableAddition->setQuantity(grain_kg);
   twoRowFermentableAddition->setMeasure(Measurement::PhysicalQuantity::Mass);
   rec->addAddition(twoRowFermentableAddition);

   // Add mash
   rec->setMash(singleConversion);

   // Malt color units
   double mcus =
      this->pimpl->m_twoRow->color_srm()
      * (grain_kg * 2.205) // Grain in lb
      / (rec->batchSize_l() * 0.2642); // Batch size in gal

   // Morey formula
   double srm = 1.49 * pow(mcus, 0.686);

   // Initial og guess in kg/L.
   double og = 1.050;

   // Ground-truth plato (~12)
   double plato =
      grain_kg
      * *this->pimpl->m_twoRow->fineGrindYield_pct()/100.0
      * rec->efficiency_pct()/100.0
      / (rec->batchSize_l() * og) // Total wort mass in kg (not L)
      * 100; // Convert to percent

   // Refine og estimate
   og = 259.0/(259.0-plato);

   // Ground-truth IBUs (mg/L of isomerized alpha acid)
   //   ~40 IBUs
   double ibus =
      cascadeHopAddition->quantity()*1e6     // Hops in mg
      * this->pimpl->m_cascade_4pct->alpha_pct()/100.0 // AA ratio
      * 0.235 // Tinseth utilization (60 min @ 12 Plato)
      / rec->batchSize_l();

   // Verify calculated recipe parameters within some tolerance.
   QVERIFY2( fuzzyComp(rec->boilVolume_l(),  rec->boil()->preBoilSize_l().value_or(0.0),  0.1),     "Wrong boil volume calculation" );
   QVERIFY2( fuzzyComp(rec->finalVolume_l(), rec->batchSize_l(), 0.1),     "Wrong final volume calculation" );
   QVERIFY2( fuzzyComp(rec->og(),            og,                 0.002),   "Wrong OG calculation" );
   QVERIFY2( fuzzyComp(rec->IBU(),           ibus,               5.0),     "Wrong IBU calculation" );
   QVERIFY2( fuzzyComp(rec->color_srm(),     srm,                srm*0.1), "Wrong color calculation" );
}

void RecipeCalcTests::postBoilLossOgTest() {
   // .:TODO:. Would be good to fix and reinstate this test...
   return;
   double const grain_kg = 5.0;
   Recipe* recNoLoss = new Recipe(QString("TestRecipe_noLoss"));
   Recipe* recLoss = new Recipe(QString("TestRecipe_loss"));
   auto eLoss = std::make_shared<Equipment>(*this->pimpl->m_equipFiveGalNoLoss.get());

   // Only difference between the recipes:
   // - 2 L of post-boil loss
   // - 2 L extra of boil size (to hit the same batch size)
   eLoss->setKettleTrubChillerLoss_l(2.0);
   eLoss->setKettleBoilSize_l(this->pimpl->m_equipFiveGalNoLoss->kettleBoilSize_l() + eLoss->kettleTrubChillerLoss_l());

   // Basic recipe parameters
   recNoLoss->setBatchSize_l(this->pimpl->m_equipFiveGalNoLoss->fvBatchSize_l());
   recNoLoss->nonOptBoil()->setPreBoilSize_l(this->pimpl->m_equipFiveGalNoLoss->kettleBoilSize_l());
   recNoLoss->setEfficiency_pct(70.0);

   recLoss->setBatchSize_l(eLoss->fvBatchSize_l() - eLoss->kettleTrubChillerLoss_l()); // Adjust for trub losses
   recLoss->nonOptBoil()->setPreBoilSize_l(eLoss->kettleBoilSize_l() - eLoss->kettleTrubChillerLoss_l());
   recLoss->setEfficiency_pct(70.0);

   double mashWaterNoLoss_l = *recNoLoss->nonOptBoil()->preBoilSize_l()
      + this->pimpl->m_equipFiveGalNoLoss->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg) * grain_kg
   ;
   double mashWaterLoss_l = *recLoss->nonOptBoil()->preBoilSize_l()
      + eLoss->mashTunGrainAbsorption_LKg().value_or(Equipment::default_mashTunGrainAbsorption_LKg) * grain_kg
   ;

   // Add equipment
   recNoLoss->setEquipment(this->pimpl->m_equipFiveGalNoLoss);
   recLoss->setEquipment(eLoss);

   // Add grain
   auto twoRowFermentableAddition1 = std::make_shared<RecipeAdditionFermentable>("Two Row Grain Addition 1");
   auto twoRowFermentableAddition2 = std::make_shared<RecipeAdditionFermentable>("Two Row Grain Addition 2");
   twoRowFermentableAddition1->setFermentable(this->pimpl->m_twoRow.get());
   twoRowFermentableAddition2->setFermentable(this->pimpl->m_twoRow.get());
   twoRowFermentableAddition1->setStage(RecipeAddition::Stage::Mash);
   twoRowFermentableAddition2->setStage(RecipeAddition::Stage::Mash);
   twoRowFermentableAddition1->setQuantity(grain_kg);
   twoRowFermentableAddition2->setQuantity(grain_kg);
   twoRowFermentableAddition1->setMeasure(Measurement::PhysicalQuantity::Mass);
   twoRowFermentableAddition2->setMeasure(Measurement::PhysicalQuantity::Mass);
   recNoLoss->addAddition(twoRowFermentableAddition1);
   recLoss  ->addAddition(twoRowFermentableAddition2);

   // Single conversion, no sparge
   auto singleConversion = std::make_shared<Mash>();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);

   auto singleConversion_convert = std::make_shared<MashStep>();
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Type::Infusion);
   singleConversion->add(singleConversion_convert);

   // Infusion for recNoLoss
   singleConversion_convert->setAmount_l(mashWaterNoLoss_l);
   recNoLoss->setMash(singleConversion);

   // Infusion for recLoss
   singleConversion_convert->setAmount_l(mashWaterLoss_l);
   recLoss->setMash(singleConversion);

   // Verify we hit the right boil/final volumes (that the test is sane)
   QVERIFY2(fuzzyComp(recNoLoss->boilVolume_l(),  *recNoLoss->nonOptBoil()->preBoilSize_l(), 0.1), "Wrong boil volume calculation (recNoLoss)");
   QVERIFY2(fuzzyComp(  recLoss->boilVolume_l(),    *recLoss->nonOptBoil()->preBoilSize_l(), 0.1), "Wrong boil volume calculation (recLoss)"  );
   QVERIFY2(fuzzyComp(recNoLoss->finalVolume_l(), recNoLoss->batchSize_l(), 0.1), "Wrong final volume calculation (recNoLoss)");
   QVERIFY2(fuzzyComp(  recLoss->finalVolume_l(),   recLoss->batchSize_l(), 0.1), "Wrong final volume calculation (recLoss)"  );

   // The OG calc itself is verified in recipeCalcTest_*(), so just verify that
   // the two OGs are the same
   QVERIFY2( fuzzyComp(recLoss->og(), recNoLoss->og(), 0.002), "OG of recipe with post-boil loss is different from no-loss recipe" );
}

void RecipeCalcTests::testBoilAdditionTiming() {
   qDebug() << Q_FUNC_INFO << "Starting";

   auto recipe = std::make_shared<Recipe>("Boil Addition Timing Test");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(20.0);
   recipe->setEquipment(this->pimpl->m_equipFiveGalNoLoss);

   // Standard three-step boil profile: pre-boil, 60 minute boil proper, 30 minute post-boil
   auto boil = recipe->nonOptBoil();
   boil->setPreBoilSize_l(24.0);
   boil->setBoilTime_mins(60.0); // This also ensures the standard profile
   auto const boilSteps = boil->boilSteps();
   QCOMPARE(boilSteps.size(), 3);
   boilSteps[2]->setStepTime_mins(30.0);
   QVERIFY(fuzzyComp(boil->boilTime_mins(), 60.0, 0.001));

   auto makeAddition = [&](QString const & name,
                           std::optional<int>    const step,
                           std::optional<double> const addAtTime_mins,
                           std::optional<double> const duration_mins = std::nullopt) {
      auto addition = std::make_shared<RecipeAdditionHop>(name);
      addition->setHop(this->pimpl->m_cascade_4pct.get());
      addition->setStage(RecipeAddition::Stage::Boil);
      addition->setStep(step);
      addition->setAddAtTime_mins(addAtTime_mins);
      addition->setDuration_mins(duration_mins);
      addition->setQuantity(0.020);
      addition->setMeasure(Measurement::PhysicalQuantity::Mass);
      return recipe->addAddition(addition);
   };

   auto const boil60          = makeAddition("Boil 60"         , 2           , 60.0        );
   auto const boil20          = makeAddition("Boil 20"         , 2           , 20.0        );
   auto const boilNoTime      = makeAddition("Boil no time"    , 2           , std::nullopt);
   auto const noStep15        = makeAddition("No step 15"      , std::nullopt, 15.0        );
   // 20 minutes before the end of the 30 minute post-boil step, ie 10 minutes after flameout
   auto const whirlpool20     = makeAddition("Whirlpool 20"    , 3           , 20.0        );
   auto const whirlpoolNoTime = makeAddition("Whirlpool start" , 3           , std::nullopt);
   // This is how a BeerXML aroma hop with TIME of 15 gets read in
   auto const aromaBeerXml    = makeAddition("BeerXML aroma 15", 3           , std::nullopt, 15.0);

   //
   // Check the timing calculations
   //
   auto t = recipe->boilAdditionTiming(*boil60);
   QVERIFY(!t.postBoil);
   QVERIFY(fuzzyComp(t.boilProper_mins, 60.0, 0.001));

   t = recipe->boilAdditionTiming(*boilNoTime);
   QVERIFY(!t.postBoil);
   QVERIFY2(fuzzyComp(t.boilProper_mins, 60.0, 0.001), "Step addition with no time should be at start of step");

   t = recipe->boilAdditionTiming(*noStep15);
   QVERIFY(!t.postBoil);
   QVERIFY(fuzzyComp(t.boilProper_mins, 15.0, 0.001));

   t = recipe->boilAdditionTiming(*whirlpool20);
   QVERIFY(t.postBoil);
   QVERIFY2(fuzzyComp(t.boilProper_mins   ,  0.0, 0.001), "Whirlpool addition should not be in the boil");
   QVERIFY (fuzzyComp(t.afterFlameout_mins, 10.0, 0.001));
   QVERIFY (t.untilEndOfStep_mins    && fuzzyComp(*t.untilEndOfStep_mins   , 20.0, 0.001));
   QVERIFY (t.postBoilRemaining_mins && fuzzyComp(*t.postBoilRemaining_mins, 20.0, 0.001));

   t = recipe->boilAdditionTiming(*whirlpoolNoTime);
   QVERIFY(t.postBoil);
   QVERIFY(fuzzyComp(t.afterFlameout_mins, 0.0, 0.001));
   QVERIFY(t.untilEndOfStep_mins && fuzzyComp(*t.untilEndOfStep_mins, 30.0, 0.001));

   t = recipe->boilAdditionTiming(*aromaBeerXml);
   QVERIFY(t.postBoil);
   QVERIFY(t.postBoilRemaining_mins && fuzzyComp(*t.postBoilRemaining_mins, 15.0, 0.001));

   //
   // Check IBUs.  We are using Tinseth (see TestEnvironment), which gives no IBUs for hops not in the boil.
   //
   double const ibuBoil60      = recipe->ibuFromHopAddition(*boil60     );
   double const ibuBoil20      = recipe->ibuFromHopAddition(*boil20     );
   double const ibuBoilNoTime  = recipe->ibuFromHopAddition(*boilNoTime );
   double const ibuWhirlpool20 = recipe->ibuFromHopAddition(*whirlpool20);
   qDebug() <<
      Q_FUNC_INFO << "IBUs: boil60" << ibuBoil60 << ", boil20" << ibuBoil20 << ", boilNoTime" << ibuBoilNoTime <<
      ", whirlpool20" << ibuWhirlpool20;
   QVERIFY(ibuBoil60 > ibuBoil20);
   QVERIFY(ibuBoil20 > 1.0);
   QVERIFY2(fuzzyComp(ibuBoilNoTime , ibuBoil60, 0.001), "Step addition with no time should be at start of step");
   QVERIFY2(fuzzyComp(ibuWhirlpool20, 0.0      , 0.001), "Whirlpool addition time treated as boil time");

   //
   // Check instructions.  Flameout should come before all the post-boil additions, which should show how long they
   // steep for, not some function of the boil length.
   //
   recipe->generateInstructions();
   auto const instructions = recipe->instructions();
   QStringList summary;
   int flameoutIndex = -1;
   for (int ii = 0; ii < instructions.size(); ++ii) {
      summary.append(QString("%1 (%2)").arg(instructions[ii]->name()).arg(instructions[ii]->interval_mins()));
      if (instructions[ii]->name() == Recipe::tr("Flameout")) {
         flameoutIndex = ii;
      }
   }
   qDebug() << Q_FUNC_INFO << "Instructions:" << summary.join("; ");
   QVERIFY2(flameoutIndex >= 0, "No flameout instruction");
   QVERIFY(flameoutIndex + 3 < instructions.size());
   // Post-boil additions sorted by time remaining: whirlpool start (30 mins), whirlpool 20, BeerXML aroma 15
   QVERIFY2(fuzzyComp(instructions[flameoutIndex + 1]->interval_mins(), 30.0, 0.001), "Wrong post-boil step-start addition");
   QVERIFY2(fuzzyComp(instructions[flameoutIndex + 2]->interval_mins(), 20.0, 0.001), "Wrong whirlpool addition");
   QVERIFY2(fuzzyComp(instructions[flameoutIndex + 3]->interval_mins(), 15.0, 0.001), "Wrong aroma addition");
   // Before flameout, the 15 minute addition with no step should come after the 20 minute one, and nothing should have
   // the bogus durations we used to calculate (60 - (30 - 20) = 50 for the whirlpool addition).
   int boil20Index = -1;
   int noStep15Index = -1;
   for (int ii = 0; ii < flameoutIndex; ++ii) {
      double const interval = instructions[ii]->interval_mins();
      QVERIFY2(!fuzzyComp(interval, 50.0, 0.001), "Whirlpool addition time treated as boil time");
      if (fuzzyComp(interval, 20.0, 0.001)) { boil20Index   = ii; }
      if (fuzzyComp(interval, 15.0, 0.001)) { noStep15Index = ii; }
   }
   QVERIFY2(boil20Index >= 0 && noStep15Index > boil20Index, "Boil addition without step sorted wrongly");

   return;
}

void RecipeCalcTests::testInventory() {
   qDebug() << Q_FUNC_INFO << "Starting";
   Measurement::Amount amountOfHop{123.45, Measurement::Units::kilograms};
   auto hopPurchase = std::make_shared<StockPurchaseHop>("Hop Purchase");
   hopPurchase->setHop(this->pimpl->m_cascade_4pct.get());
   hopPurchase->setAmount(amountOfHop);
   ObjectStoreWrapper::insert(hopPurchase);
   QVariant inventoryRaw = this->pimpl->m_cascade_4pct->property(*PropertyNames::Ingredient::totalInventory);
   QVERIFY2(inventoryRaw.canConvert<Measurement::Amount>(), "Error retrieving hop inventory property");
   Measurement::Amount inventory = inventoryRaw.value<Measurement::Amount>();

   QVERIFY2(fuzzyComp(inventory.quantity, 123.45,  0.00000001), "Wrong hop inventory amount");
   QVERIFY2(inventory.unit == &Measurement::Units::kilograms, "Wrong hop inventory units");

   return;
}
