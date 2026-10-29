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
      + this->pimpl->m_equipFiveGalNoLoss->grainAbsorption_LKg().value_or(Equipment::default_grainAbsorption_LKg) * grain_kg // Grain absorption
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
      + this->pimpl->m_equipFiveGalNoLoss->grainAbsorption_LKg().value_or(Equipment::default_grainAbsorption_LKg) * grain_kg
   ;
   double mashWaterLoss_l = *recLoss->nonOptBoil()->preBoilSize_l()
      + eLoss->grainAbsorption_LKg().value_or(Equipment::default_grainAbsorption_LKg) * grain_kg
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

   //
   // First wort and aroma additions are identified by step temperatures, not step numbers
   //
   auto const firstWort   = makeAddition("First wort"       , 1, std::nullopt);
   // Additions migrated from older DBs kept their old Hop time
   auto const firstWort60 = makeAddition("Migrated FWH 60"  , 1, 60.0        );
   QVERIFY2( firstWort  ->isFirstWort(), "Addition in pre-boil step not first wort");
   QVERIFY2( firstWort60->isFirstWort(), "Migrated first wort addition with time not first wort");
   QVERIFY2(!boil60     ->isFirstWort(), "Boil proper addition treated as first wort");
   QVERIFY2(!whirlpool20->isFirstWort(), "Post-boil addition treated as first wort");
   QVERIFY2( whirlpool20->isAroma    (), "Post-boil addition not aroma");
   QVERIFY2(!boil60     ->isAroma    (), "Boil proper addition treated as aroma");
   QVERIFY2(!firstWort  ->isAroma    (), "Pre-boil addition treated as aroma");

   return;
}

void RecipeCalcTests::testBoilStepsWithoutTemperatures() {
   //
   // Per dotBeer, a boil step whose start or end temperature is not set is at boiling at that point.  See
   // https://github.com/Brewtarget/brewtarget/issues/1165
   //
   auto recipe = std::make_shared<Recipe>("Boil Steps Without Temperatures Test");
   ObjectStoreWrapper::insert(recipe);
   recipe->setBatchSize_l(20.0);
   recipe->setEquipment(this->pimpl->m_equipFiveGalNoLoss);

   auto boil = std::make_shared<Boil>("Single step boil without temperatures");
   ObjectStoreWrapper::insert(boil);
   auto mainStep = std::make_shared<BoilStep>("Boil");
   mainStep->setStepTime_mins(60.0);
   boil->add(mainStep);
   recipe->setBoil(boil);

   QVERIFY2(fuzzyComp(boil->boilTime_mins(), 60.0, 0.001), "Step without temperatures not counted as boil proper");
   QVERIFY2(!boil->coolTime_mins(), "Step without temperatures treated as cooling");
   auto const [firstBoilProperStep, lastBoilProperStep] = boil->boilProperSteps();
   QCOMPARE(firstBoilProperStep, 1);
   QCOMPARE(lastBoilProperStep , 1);

   // A cooling step with no start temperature starts at boiling, so is a cooling step
   auto coolStep = std::make_shared<BoilStep>("Cool");
   coolStep->setEndTemp_c(20.0);
   coolStep->setStepTime_mins(15.0);
   boil->add(coolStep);
   QVERIFY2(boil->coolTime_mins() && fuzzyComp(*boil->coolTime_mins(), 15.0, 0.001), "Cooling step not recognised");
   QVERIFY2(fuzzyComp(boil->boilTime_mins(), 60.0, 0.001), "Cooling step counted as boil proper");

   // ensureStandardProfile should add a pre-boil step, but not an extra "main boil" step
   boil->ensureStandardProfile();
   auto const boilSteps = boil->boilSteps();
   QCOMPARE(boilSteps.size(), 3);
   QVERIFY2(!Boil::startsAtBoil(*boilSteps[0]), "First step should be pre-boil");
   QVERIFY2(Boil::isBoilProper(*boilSteps[1]), "Second step should be the original boil step");
   QVERIFY2(fuzzyComp(boil->boilTime_mins(), 60.0, 0.001), "Boil time changed by ensureStandardProfile");

   // A boil addition in the boil proper with no time is at the start of the step
   auto addition = std::make_shared<RecipeAdditionHop>("Start of boil");
   addition->setHop(this->pimpl->m_cascade_4pct.get());
   addition->setStage(RecipeAddition::Stage::Boil);
   addition->setStep(2);
   addition->setQuantity(0.020);
   addition->setMeasure(Measurement::PhysicalQuantity::Mass);
   recipe->addAddition(addition);
   auto const timing = recipe->boilAdditionTiming(*addition);
   QVERIFY(!timing.postBoil);
   QVERIFY2(fuzzyComp(timing.boilProper_mins, 60.0, 0.001), "Addition in step without temperatures not in boil");

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

void RecipeCalcTests::testMashWaterVolumes() {
   double const grain_kg          =  5.0;
   double const preBoil_l         = 30.0;
   double const absorption_LKg    =  0.8;
   double const thickness_LKg     =  2.7;
   double const foundation_l      =  3.5;
   double const mashTunLoss_l     =  1.0;
   double const lauterTunLoss_l   =  2.0;

   auto twoRow = this->pimpl->m_twoRow;
   ObjectStoreWrapper::insert(twoRow);

   auto makeEquipment = [&](QString const & name,
                            double const foundationVolume_l,
                            double const lauterTunVolume_l) {
      auto equipment = std::make_shared<Equipment>(name);
      equipment->setKettleBoilSize_l(preBoil_l);
      equipment->setFvBatchSize_l(23.0);
      equipment->setMashTunVolume_l(40.0);
      equipment->setGrainAbsorption_LKg(absorption_LKg);
      equipment->setFoundationVolume_l(foundationVolume_l);
      equipment->setMashTunLoss_l(mashTunLoss_l);
      equipment->setLauterTunVolume_l(lauterTunVolume_l);
      equipment->setLauterTunDeadspaceLoss_l(lauterTunLoss_l);
      equipment->setTopUpKettle_l(0.0);
      equipment->setTopUpWater_l(0.0);
      equipment->setKettleTrubChillerLoss_l(0.0);
      equipment->setKettleEvaporationPerHour_l(3.0);
      ObjectStoreWrapper::insert(equipment);
      return equipment;
   };

   auto makeRecipe = [&](QString const & name, std::shared_ptr<Equipment> equipment) {
      auto recipe = std::make_shared<Recipe>(name);
      ObjectStoreWrapper::insert(recipe);
      recipe->setBatchSize_l(23.0);
      recipe->setEquipment(equipment);
      auto boil = recipe->nonOptBoil();
      boil->setPreBoilSize_l(preBoil_l);
      boil->setBoilTime_mins(60.0);
      auto grain = std::make_shared<RecipeAdditionFermentable>(name + " grain");
      grain->setFermentable(twoRow.get());
      grain->setStage(RecipeAddition::Stage::Mash);
      grain->setQuantity(grain_kg);
      grain->setMeasure(Measurement::PhysicalQuantity::Mass);
      recipe->addAddition(grain);
      return recipe;
   };

   //
   // Single-step infusion with a batch sparge, with the strike and sparge water set the same way MashWizard does it
   //
   auto addMash = [&](Recipe & recipe) {
      auto mash = std::make_shared<Mash>(recipe.name() + " mash");
      ObjectStoreWrapper::insert(mash);
      auto strike = std::make_shared<MashStep>("Strike");
      strike->setType(MashStep::Type::Infusion);
      strike->setStartTemp_c(66.0);
      strike->setStepTime_mins(60.0);
      strike->setAmount_l(recipe.targetStrikeWater_l(thickness_LKg));
      mash->add(strike);
      auto sparge = std::make_shared<MashStep>("Sparge");
      sparge->setType(MashStep::Type::BatchSparge);
      sparge->setStartTemp_c(76.0);
      sparge->setStepTime_mins(15.0);
      sparge->setAmount_l(recipe.targetTotalMashVol_l() - strike->amount_l());
      mash->add(sparge);
      recipe.setMash(mash);
      return mash;
   };

   //
   // Three set-ups: an all-in-one with no foundation volume, the same with a foundation volume (eg the space below the
   // malt pipe), and a three-vessel set-up with a separate lauter tun.
   //
   auto const allInOneNoFoundation = makeEquipment("All-in-one, no foundation" , 0.0         ,  0.0);
   auto const allInOne             = makeEquipment("All-in-one with foundation", foundation_l,  0.0);
   auto const threeVessel          = makeEquipment("Separate lauter tun"       , 0.0         , 40.0);

   QVERIFY(fuzzyComp(allInOneNoFoundation->strikeWaterAllowance_l(), mashTunLoss_l               , 0.0001));
   QVERIFY(fuzzyComp(allInOne            ->strikeWaterAllowance_l(), mashTunLoss_l + foundation_l, 0.0001));
   QVERIFY2(fuzzyComp(threeVessel        ->strikeWaterAllowance_l(), mashTunLoss_l               , 0.0001),
            "Lauter tun losses are not in the mash tun, so do not belong in the strike water");
   QVERIFY2(fuzzyComp(allInOne->getLauteringDeadspaceLoss_l(), mashTunLoss_l, 0.0001),
            "Foundation volume is recovered, so is not a lautering loss");
   QVERIFY2(fuzzyComp(allInOneNoFoundation->getLauteringDeadspaceLoss_l(), mashTunLoss_l, 0.0001),
            "Lauter tun loss should be ignored when there is no lauter tun");
   QVERIFY(fuzzyComp(threeVessel->getLauteringDeadspaceLoss_l(), mashTunLoss_l + lauterTunLoss_l, 0.0001));

   auto const recipeNoFoundation = makeRecipe("Mash water - no foundation", allInOneNoFoundation);
   auto const recipeFoundation   = makeRecipe("Mash water - foundation"   , allInOne            );
   auto const recipeThreeVessel  = makeRecipe("Mash water - three vessel" , threeVessel         );
   QVERIFY(fuzzyComp(recipeFoundation->grainsInMash_kg(), grain_kg, 0.0001));

   //
   // Total water is what reaches the boil, plus what the grain absorbs, plus what is lost in the vessel(s).  Foundation
   // volume does not change it, because it is recovered.
   //
   double const expectedTotalAllInOne_l = preBoil_l + absorption_LKg * grain_kg + mashTunLoss_l;
   double const expectedTotalThreeVessel_l = expectedTotalAllInOne_l + lauterTunLoss_l;
   qDebug() <<
      Q_FUNC_INFO << "Target total mash water: no foundation" << recipeNoFoundation->targetTotalMashVol_l() <<
      ", foundation" << recipeFoundation->targetTotalMashVol_l() << ", three vessel" <<
      recipeThreeVessel->targetTotalMashVol_l();
   QVERIFY2(fuzzyComp(recipeNoFoundation->targetTotalMashVol_l(), expectedTotalAllInOne_l   , 0.0001),
            "Mash tun loss not allowed for (exactly once) in total water");
   QVERIFY2(fuzzyComp(recipeFoundation  ->targetTotalMashVol_l(), expectedTotalAllInOne_l   , 0.0001),
            "Foundation volume should not change the total water needed");
   QVERIFY2(fuzzyComp(recipeThreeVessel ->targetTotalMashVol_l(), expectedTotalThreeVessel_l, 0.0001),
            "Lauter tun loss not allowed for (exactly once) in total water");
   QVERIFY2(fuzzyComp(recipeFoundation->targetCollectedWortVol_l(), preBoil_l, 0.0001),
            "Wort collected from the mash should be the pre-boil volume (no top-up or post-mash additions here)");

   //
   // Strike water: the grain bed's water plus the foundation volume plus the mash tun loss
   //
   QVERIFY(fuzzyComp(recipeNoFoundation->targetStrikeWater_l(thickness_LKg),
                     thickness_LKg * grain_kg + mashTunLoss_l, 0.0001));
   QVERIFY2(fuzzyComp(recipeFoundation->targetStrikeWater_l(thickness_LKg) -
                      recipeNoFoundation->targetStrikeWater_l(thickness_LKg), foundation_l, 0.0001),
            "Foundation volume should be added to the strike water");
   QVERIFY(fuzzyComp(recipeThreeVessel->targetStrikeWater_l(thickness_LKg),
                     thickness_LKg * grain_kg + mashTunLoss_l, 0.0001));
   // And going back the other way should give us the same thickness (otherwise MashWizard would change the strike
   // water each time it was run on a no-sparge mash)
   auto const thickness = recipeFoundation->mashThicknessForStrikeWater_LKg(
      recipeFoundation->targetStrikeWater_l(thickness_LKg)
   );
   QVERIFY(thickness && fuzzyComp(*thickness, thickness_LKg, 0.0001));

   //
   // Now put the water in the mashes, and check that, in all three cases, we hit the pre-boil volume.  This checks that
   // the foundation volume is not counted as lost (it is in the mash water, but not subtracted from the wort) and that
   // the mash/lauter losses are.
   //
   addMash(*recipeNoFoundation);
   addMash(*recipeFoundation  );
   addMash(*recipeThreeVessel );
   for (auto const & recipe : {recipeNoFoundation, recipeFoundation, recipeThreeVessel}) {
      qDebug() <<
         Q_FUNC_INFO << recipe->name() << ": total mash water" << recipe->mash()->totalMashWater_l() <<
         ", wort from mash" << recipe->wortFromMash_l() << ", boil volume" << recipe->boilVolume_l();
      QVERIFY2(fuzzyComp(recipe->mash()->totalMashWater_l(), recipe->targetTotalMashVol_l(), 0.0001),
               qPrintable(recipe->name()));
      QVERIFY2(fuzzyComp(recipe->wortFromMash_l(),
                         recipe->targetTotalMashVol_l() - absorption_LKg * grain_kg,
                         0.0001),
               qPrintable(recipe->name() + ": wort from mash should be total water less grain absorption"));
      QVERIFY2(fuzzyComp(recipe->boilVolume_l(), preBoil_l, 0.0001),
               qPrintable(recipe->name() + ": did not hit pre-boil volume"));
   }

   //
   // With a foundation volume, the strike water is more, and the sparge water is correspondingly less
   //
   auto const strikeNoFoundation = recipeNoFoundation->mash()->mashSteps().first()->amount_l();
   auto const strikeFoundation   = recipeFoundation  ->mash()->mashSteps().first()->amount_l();
   auto const spargeNoFoundation = recipeNoFoundation->mash()->mashSteps().last ()->amount_l();
   auto const spargeFoundation   = recipeFoundation  ->mash()->mashSteps().last ()->amount_l();
   QVERIFY(fuzzyComp(strikeFoundation - strikeNoFoundation, foundation_l, 0.0001));
   QVERIFY(fuzzyComp(spargeNoFoundation - spargeFoundation, foundation_l, 0.0001));
   return;
}

void RecipeCalcTests::testEvaporationRate() {
   auto equipment = std::make_shared<Equipment>("Evaporation Rate Test");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);

   // BeerXML EVAP_RATE is a percentage of the boil size (not the batch size) per hour
   equipment->setEvapRate_pctHr(10.0);
   QVERIFY(equipment->kettleEvaporationPerHour_l());
   QVERIFY2(fuzzyComp(*equipment->kettleEvaporationPerHour_l(), 3.0, 0.0001),
            "Evaporation percentage should be relative to boil size");

   equipment->setKettleEvaporationPerHour_l(4.5);
   QVERIFY(equipment->evapRate_pctHr());
   QVERIFY(fuzzyComp(*equipment->evapRate_pctHr(), 15.0, 0.0001));

   // If the boil size changes, the percentage should follow (as the liters per hour is what we actually use)
   equipment->setKettleBoilSize_l(45.0);
   QVERIFY(equipment->evapRate_pctHr());
   QVERIFY(fuzzyComp(*equipment->evapRate_pctHr(), 10.0, 0.0001));

   // No batch size (eg equipment read from a file with no fermentation vessel) should not give us infinity or NaN
   equipment->setFvBatchSize_l(0.0);
   equipment->setKettleEvaporationPerHour_l(4.0);
   QVERIFY(equipment->evapRate_pctHr() && std::isfinite(*equipment->evapRate_pctHr()));

   //
   // If we only have the percentage (as when reading a BeerXML file written by another program, which won't have our
   // REAL_EVAP_RATE extension tag), we should still get the liters per hour.
   //
   NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
   npb.insert(PropertyNames::NamedEntity::name              , QString{"Evaporation Rate From Percentage"});
   npb.insert(PropertyNames::Equipment::kettleBoilSize_l    , 25.0);
   npb.insert(PropertyNames::Equipment::fvBatchSize_l       , 20.0);
   npb.insert(PropertyNames::Equipment::evapRate_pctHr      , QVariant::fromValue(std::optional<double>{12.0}));
   Equipment const fromPercentage{npb};
   QVERIFY2(fromPercentage.kettleEvaporationPerHour_l(), "Evaporation rate percentage ignored");
   QVERIFY(fuzzyComp(*fromPercentage.kettleEvaporationPerHour_l(), 3.0, 0.0001));
   return;
}

void RecipeCalcTests::testEquipmentDefaultVolumes() {
   //
   // BeerXML from another program: BOIL_SIZE and BATCH_SIZE but no capacities, so the capacities are taken to be (at
   // least) the default volumes.
   //
   {
      NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
      npb.insert(PropertyNames::NamedEntity::name          , QString{"Default Volumes - BeerXML"});
      npb.insert(PropertyNames::Equipment::kettleBoilSize_l, 28.0);
      npb.insert(PropertyNames::Equipment::fvBatchSize_l   , 23.0);
      Equipment const equipment{npb};
      QVERIFY(fuzzyComp(equipment.kettleVolume_l()  , 28.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvVolume_l()      , 23.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.kettleBoilSize_l(), 28.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvBatchSize_l()   , 23.0, 0.0001));
      // The pre-boil volume was given (not calculated by us), so we don't know what boil time it was for, and don't
      // adjust it
      QVERIFY(fuzzyComp(equipment.defaultPreBoilSize_l(90.0), 28.0, 0.0001));
   }
   //
   // BeerJSON: capacities but no default volumes, so the batch size falls back to the fermenter capacity, and the
   // pre-boil volume is calculated from it.
   //
   {
      NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
      npb.insert(PropertyNames::NamedEntity::name                    , QString{"Default Volumes - BeerJSON"});
      npb.insert(PropertyNames::Equipment::kettleVolume_l            , 40.0);
      npb.insert(PropertyNames::Equipment::fvVolume_l                , 25.0);
      npb.insert(PropertyNames::Equipment::kettleTrubChillerLoss_l   , 1.5);
      npb.insert(PropertyNames::Equipment::kettleEvaporationPerHour_l, QVariant::fromValue(std::optional<double>{3.5}));
      Equipment const equipment{npb};
      QVERIFY(fuzzyComp(equipment.kettleVolume_l()  , 40.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvVolume_l()      , 25.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvBatchSize_l()   , 25.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.kettleBoilSize_l(), 25.0 + 1.5 + 3.5, 0.0001));
   }
   //
   // dotBeer: capacities and default batch size
   //
   {
      NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
      npb.insert(PropertyNames::NamedEntity::name                    , QString{"Default Volumes - dotBeer"});
      npb.insert(PropertyNames::Equipment::kettleVolume_l            , 40.0);
      npb.insert(PropertyNames::Equipment::fvVolume_l                , 30.0);
      npb.insert(PropertyNames::Equipment::fvBatchSize_l             , 21.0);
      npb.insert(PropertyNames::Equipment::kettleTrubChillerLoss_l   , 1.0);
      npb.insert(PropertyNames::Equipment::topUpWater_l              , QVariant::fromValue(std::optional<double>{2.0}));
      npb.insert(PropertyNames::Equipment::kettleEvaporationPerHour_l, QVariant::fromValue(std::optional<double>{4.0}));
      Equipment const equipment{npb};
      QVERIFY(fuzzyComp(equipment.kettleVolume_l()  , 40.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvVolume_l()      , 30.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvBatchSize_l()   , 21.0, 0.0001));
      // Top-up water in the fermentor means less needs to come out of the kettle
      QVERIFY(fuzzyComp(equipment.kettleBoilSize_l(), 21.0 + 1.0 - 2.0 + 4.0, 0.0001));
      // Longer boil, more boil-off
      QVERIFY(fuzzyComp(equipment.calculatedPreBoilSize_l(90.0), 21.0 + 1.0 - 2.0 + 6.0, 0.0001));
      // We calculated the default pre-boil volume, so we know how to adjust it for a recipe's boil time
      QVERIFY(fuzzyComp(equipment.defaultPreBoilSize_l(90.0), 21.0 + 1.0 - 2.0 + 6.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.defaultPreBoilSize_l(60.0), 21.0 + 1.0 - 2.0 + 4.0, 0.0001));
   }
   //
   // Loading from the database: all fields are present, so an explicit 0 (eg a capacity the user doesn't know) is
   // respected rather than replaced by a guess.
   //
   {
      NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
      npb.insert(PropertyNames::NamedEntity::name                    , QString{"Default Volumes - explicit zeros"});
      npb.insert(PropertyNames::Equipment::kettleBoilSize_l          , 28.0);
      npb.insert(PropertyNames::Equipment::fvBatchSize_l             , 23.0);
      npb.insert(PropertyNames::Equipment::kettleVolume_l            ,  0.0);
      npb.insert(PropertyNames::Equipment::fvVolume_l                ,  0.0);
      Equipment const equipment{npb};
      QVERIFY(fuzzyComp(equipment.kettleVolume_l()  ,  0.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvVolume_l()      ,  0.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.kettleBoilSize_l(), 28.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvBatchSize_l()   , 23.0, 0.0001));
   }
   {
      NamedParameterBundle npb{NamedParameterBundle::OperationMode::NotStrict};
      npb.insert(PropertyNames::NamedEntity::name                    , QString{"Default Volumes - explicit zero defaults"});
      npb.insert(PropertyNames::Equipment::kettleBoilSize_l          ,  0.0);
      npb.insert(PropertyNames::Equipment::fvBatchSize_l             ,  0.0);
      npb.insert(PropertyNames::Equipment::kettleVolume_l            , 40.0);
      npb.insert(PropertyNames::Equipment::fvVolume_l                , 30.0);
      Equipment const equipment{npb};
      QVERIFY(fuzzyComp(equipment.kettleVolume_l()  , 40.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvVolume_l()      , 30.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.kettleBoilSize_l(),  0.0, 0.0001));
      QVERIFY(fuzzyComp(equipment.fvBatchSize_l()   ,  0.0, 0.0001));
   }
   return;
}

void RecipeCalcTests::testMashTunHeatCapacity() {
   auto makeEquipment = [](QString const & name, double const mashTunVolume_l) {
      auto equipment = std::make_shared<Equipment>(name);
      equipment->setKettleVolume_l(35.0);
      equipment->setMashTunVolume_l(mashTunVolume_l);
      ObjectStoreWrapper::insert(equipment);
      return equipment;
   };
   auto makeRecipe = [](QString const & name, std::shared_ptr<Equipment> equipment) {
      auto recipe = std::make_shared<Recipe>(name);
      ObjectStoreWrapper::insert(recipe);
      if (equipment) {
         recipe->setEquipment(equipment);
      }
      auto mash = std::make_shared<Mash>(name + " mash");
      mash->setGrainTemp_c(18.0);
      ObjectStoreWrapper::insert(mash);
      recipe->setMash(mash);
      return recipe;
   };

   // A new Mash has no tun figures, rather than a tun at 0°C
   {
      Mash const newMash{"New Mash"};
      QVERIFY2(!newMash.initialTunTemp_c(), "New Mash should not have a tun temperature");
      QVERIFY(!newMash.tunWeight_kg());
      QVERIFY(!newMash.tunSpecificHeat_calGC());
   }

   // Mash tun with weight and specific heat on the equipment: these are used in preference to the Mash ones
   auto const withTunFigures = makeEquipment("Mash Tun With Figures", 40.0);
   withTunFigures->setMashTunWeight_kg(5.0);
   withTunFigures->setMashTunSpecificHeat_calGC(0.12);
   auto const recipeEquipmentFigures = makeRecipe("Tun heat capacity - equipment", withTunFigures);
   recipeEquipmentFigures->mash()->setTunWeight_kg(10.0);
   recipeEquipmentFigures->mash()->setTunSpecificHeat_calGC(0.3);
   QVERIFY(fuzzyComp(recipeEquipmentFigures->mashTunHeatCapacity_kcalC(), 5.0 * 0.12, 0.0001));

   // Mash tun with no figures on the equipment: fall back to the Mash ones (eg from a BeerXML MASH record)
   auto const withoutTunFigures = makeEquipment("Mash Tun Without Figures", 40.0);
   auto const recipeMashFigures = makeRecipe("Tun heat capacity - mash", withoutTunFigures);
   recipeMashFigures->mash()->setTunWeight_kg(10.0);
   recipeMashFigures->mash()->setTunSpecificHeat_calGC(0.3);
   QVERIFY(fuzzyComp(recipeMashFigures->mashTunHeatCapacity_kcalC(), 10.0 * 0.3, 0.0001));

   // No figures anywhere: tun is ignored
   auto const recipeNoFigures = makeRecipe("Tun heat capacity - none", withoutTunFigures);
   QVERIFY(fuzzyComp(recipeNoFigures->mashTunHeatCapacity_kcalC(), 0.0, 0.0001));

   // No equipment: use the Mash figures
   auto const recipeNoEquipment = makeRecipe("Tun heat capacity - no equipment", nullptr);
   recipeNoEquipment->mash()->setTunWeight_kg(10.0);
   recipeNoEquipment->mash()->setTunSpecificHeat_calGC(0.3);
   QVERIFY(fuzzyComp(recipeNoEquipment->mashTunHeatCapacity_kcalC(), 10.0 * 0.3, 0.0001));

   //
   // No mash tun, so the mash happens in the kettle, where the strike water is heated: no allowance for the vessel,
   // even if the Mash has tun figures (eg from a BeerXML file).
   //
   auto const allInOne = makeEquipment("All-in-one For Tun Heat Capacity", 0.0);
   allInOne->setKettleWeight_kg(29.0);
   allInOne->setKettleSpecificHeat_calGC(0.12);
   QVERIFY(allInOne->mashesInKettle());
   QVERIFY(fuzzyComp(allInOne->mashVesselVolume_l(), 35.0, 0.0001));
   auto const recipeInKettle = makeRecipe("Tun heat capacity - kettle", allInOne);
   recipeInKettle->mash()->setTunWeight_kg(10.0);
   recipeInKettle->mash()->setTunSpecificHeat_calGC(0.3);
   QVERIFY(fuzzyComp(recipeInKettle->mashTunHeatCapacity_kcalC(), 0.0, 0.0001));

   // Tun temperature: grain temperature unless set
   QVERIFY(fuzzyComp(recipeInKettle->mashTunTemp_c(), 18.0, 0.0001));
   recipeInKettle->mash()->setInitialTunTemp_c(25.0);
   QVERIFY(fuzzyComp(recipeInKettle->mashTunTemp_c(), 25.0, 0.0001));
   return;
}