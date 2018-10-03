/*
 * Testing.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
 * - Philip Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Testing.h>
#include <math.h>
#include "recipe.h"
#include "equipment.h"
#include "database.h"
#include "hop.h"
#include "fermentable.h"
#include "mash.h"
#include "mashstep.h"

QTEST_MAIN(Testing)

void Testing::initTestCase()
{
   // Create a different set of options to avoid clobbering real options
   QCoreApplication::setOrganizationName("brewtarget-test");
   QCoreApplication::setOrganizationDomain("brewtarget.org/test");
   QCoreApplication::setApplicationName("brewtarget-test");

   // Set options so that any data modification does not affect any other data
   Brewtarget::setOption("user_data_dir", QDir::tempPath());
   Brewtarget::setOption("color_formula", "morey");
   Brewtarget::setOption("ibu_formula", "tinseth");

   // Tell Brewtarget not to require any "user" input on starting
   Brewtarget::setInteractive(false);
   QVERIFY( Brewtarget::initialize() );

   // 5 gallon equipment
   equipFiveGalNoLoss = Database::instance().newEquipment();
   equipFiveGalNoLoss->setName("5 gal No Loss");
   equipFiveGalNoLoss->setBoilSize_l(24.0);
   equipFiveGalNoLoss->setBatchSize_l(20.0);
   equipFiveGalNoLoss->setTunVolume_l(40.0);
   equipFiveGalNoLoss->setTopUpWater_l(0);
   equipFiveGalNoLoss->setTrubChillerLoss_l(0);
   equipFiveGalNoLoss->setEvapRate_lHr(4.0);
   equipFiveGalNoLoss->setBoilTime_min(60);
   equipFiveGalNoLoss->setLauterDeadspace_l(0);
   equipFiveGalNoLoss->setTopUpKettle_l(0);
   equipFiveGalNoLoss->setHopUtilization_pct(100);
   equipFiveGalNoLoss->setGrainAbsorption_LKg(1.0);
   equipFiveGalNoLoss->setBoilingPoint_c(100);

   // Cascade pellets at 4% AA
   cascade_4pct = Database::instance().newHop();
   cascade_4pct->setName("Cascade 4pct");
   cascade_4pct->setAlpha_pct(4.0);
   cascade_4pct->setUse(Hop::Boil);
   cascade_4pct->setTime_min(60);
   cascade_4pct->setType(Hop::Both);
   cascade_4pct->setForm(Hop::Leaf);

   // 70% yield, no moisture, 2 SRM
   twoRow = Database::instance().newFermentable();
   twoRow->setName("Two Row");
   twoRow->setType(Fermentable::Grain);
   twoRow->setYield_pct(70.0);
   twoRow->setColor_srm(2.0);
   twoRow->setMoisture_pct(0);
   twoRow->setIsMashed(true);
}

void Testing::recipeCalcTest_allGrain()
{
   double const grain_kg = 5.0;
   double const conversion_l = grain_kg * 2.8; // 2.8 L/kg mash thickness
   Recipe* rec = Database::instance().newRecipe();
   Equipment* e = equipFiveGalNoLoss;

   // Basic recipe parameters
   rec->setName("TestRecipe");
   rec->setBatchSize_l(e->batchSize_l());
   rec->setBoilSize_l(e->boilSize_l());
   rec->setEfficiency_pct(70.0);

   // Single conversion, single sparge
   Mash* singleConversion = Database::instance().newMash();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);
   MashStep* singleConversion_convert = Database::instance().newMashStep(singleConversion);
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Infusion);
   singleConversion_convert->setInfuseAmount_l(conversion_l);
   MashStep* singleConversion_sparge = Database::instance().newMashStep(singleConversion);
   singleConversion_sparge->setName("Sparge");
   singleConversion_sparge->setType(MashStep::Infusion);
   singleConversion_sparge->setInfuseAmount_l(
      rec->boilSize_l()
      + e->grainAbsorption_LKg() * grain_kg // Grain absorption
      - conversion_l // Water we already added
   );

   // Add equipment
   Database::instance().addToRecipe(rec, e);

   // Add hops (85g)
   cascade_4pct->setAmount_kg(0.085);
   Database::instance().addToRecipe(rec, cascade_4pct);

   // Add grain
   twoRow->setAmount_kg(grain_kg);
   twoRow->save();
   rec->addFermentable(twoRow);

   // Add mash
   Database::instance().addToRecipe(rec, singleConversion);

   // Malt color units
   double mcus =
      twoRow->color_srm()
      * (grain_kg * 2.205) // Grain in lb
      / (rec->batchSize_l() * 0.2642); // Batch size in gal

   // Morey formula
   double srm = 1.49 * pow(mcus, 0.686);

   // Initial og guess in kg/L.
   double og = 1.050;

   // Ground-truth plato (~12)
   double plato =
      grain_kg
      * twoRow->yield_pct()/100.0
      * rec->efficiency_pct()/100.0
      / (rec->batchSize_l() * og) // Total wort mass in kg (not L)
      * 100; // Convert to percent

   // Refine og estimate
   og = 259.0/(259.0-plato);

   // Ground-truth IBUs (mg/L of isomerized alpha acid)
   //   ~40 IBUs
   double ibus =
      cascade_4pct->amount_kg()*1e6     // Hops in mg
      * cascade_4pct->alpha_pct()/100.0 // AA ratio
      * 0.235 // Tinseth utilization (60 min @ 12 Plato)
      / rec->batchSize_l();

   // Verify calculated recipe parameters within some tolerance.
   QVERIFY2( fuzzyComp(rec->boilVolume_l(),  rec->boilSize_l(),  0.1),     "Wrong boil volume calculation" );
   QVERIFY2( fuzzyComp(rec->finalVolume_l(), rec->batchSize_l(), 0.1),     "Wrong final volume calculation" );
   QVERIFY2( fuzzyComp(rec->og(),            og,                 0.002),   "Wrong OG calculation" );
   QVERIFY2( fuzzyComp(rec->IBU(),           ibus,               5.0),     "Wrong IBU calculation" );
   QVERIFY2( fuzzyComp(rec->color_srm(),     srm,                srm*0.1), "Wrong color calculation" );
}

void Testing::postBoilLossOgTest()
{
   double const grain_kg = 5.0;
   Recipe* recNoLoss = Database::instance().newRecipe();
   Recipe* recLoss = Database::instance().newRecipe();
   Equipment* eNoLoss = equipFiveGalNoLoss;
   Equipment* eLoss = Database::instance().newEquipment(eNoLoss);

   // Only difference between the recipes:
   // - 2 L of post-boil loss
   // - 2 L extra of boil size (to hit the same batch size)
   eLoss->setTrubChillerLoss_l(2.0);
   eLoss->setBoilSize_l(eNoLoss->boilSize_l() + eLoss->trubChillerLoss_l());

   // Basic recipe parameters
   recNoLoss->setName("TestRecipe_noLoss");
   recNoLoss->setBatchSize_l(eNoLoss->batchSize_l());
   recNoLoss->setBoilSize_l(eNoLoss->boilSize_l());
   recNoLoss->setEfficiency_pct(70.0);

   recLoss->setName("TestRecipe_loss");
   recLoss->setBatchSize_l(eLoss->batchSize_l() - eLoss->trubChillerLoss_l()); // Adjust for trub losses
   recLoss->setBoilSize_l(eLoss->boilSize_l() - eLoss->trubChillerLoss_l());
   recLoss->setEfficiency_pct(70.0);

   double mashWaterNoLoss_l = recNoLoss->boilSize_l()
      + eNoLoss->grainAbsorption_LKg() * grain_kg
   ;
   double mashWaterLoss_l = recLoss->boilSize_l()
      + eLoss->grainAbsorption_LKg() * grain_kg
   ;

   // Add equipment
   Database::instance().addToRecipe(recNoLoss, eNoLoss);
   Database::instance().addToRecipe(recLoss, eLoss);

   // Add grain
   twoRow->setAmount_kg(grain_kg);
   twoRow->save();
   recNoLoss->addFermentable(twoRow);
   recLoss->addFermentable(twoRow);

   // Single conversion, no sparge
   Mash* singleConversion = Database::instance().newMash();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);

   MashStep* singleConversion_convert = Database::instance().newMashStep(singleConversion);
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Infusion);

   // Infusion for recNoLoss
   singleConversion_convert->setInfuseAmount_l(mashWaterNoLoss_l);
   Database::instance().addToRecipe(recNoLoss, singleConversion);

   // Infusion for recLoss
   singleConversion_convert->setInfuseAmount_l(mashWaterLoss_l);
   Database::instance().addToRecipe(recLoss, singleConversion);

   // Verify we hit the right boil/final volumes (that the test is sane)
   QVERIFY2( fuzzyComp(recNoLoss->boilVolume_l(),  recNoLoss->boilSize_l(),  0.1),     "Wrong boil volume calculation (recNoLoss)" );
   QVERIFY2( fuzzyComp(recLoss->boilVolume_l(),    recLoss->boilSize_l(),    0.1),     "Wrong boil volume calculation (recLoss)" );
   QVERIFY2( fuzzyComp(recNoLoss->finalVolume_l(), recNoLoss->batchSize_l(), 0.1),     "Wrong final volume calculation (recNoLoss)" );
   QVERIFY2( fuzzyComp(recLoss->finalVolume_l(),   recLoss->batchSize_l(),   0.1),     "Wrong final volume calculation (recLoss)" );

   // The OG calc itself is verified in recipeCalcTest_*(), so just verify that
   // the two OGs are the same
   QVERIFY2( fuzzyComp(recLoss->og(), recNoLoss->og(), 0.002), "OG of recipe with post-boil loss is different from no-loss recipe" );
}

void Testing::cleanupTestCase()
{
   Brewtarget::cleanup();
   // Clear all persistent properties linked with this test suite.
   // It will clear all settings that are application specific, user-scoped, and in the brewtarget namespace.
   QSettings().clear();
}
