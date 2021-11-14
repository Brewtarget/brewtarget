/*
 * Testing.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Mattias Måhl <mattias@kejsarsten.com>
 * - Matt Young <mfsy@yahoo.com>
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
#include "Testing.h"

#include <exception>
#include <iostream> // For std::cout
#include <math.h>
#include <memory>

#include <xercesc/util/PlatformUtils.hpp>

#include <QDebug>
#include <QDir>
#include <QString>
#include <QtTest/QtTest>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <QtGlobal> // For qrand() -- which is superseded by QRandomGenerator in later versions of Qt
#else
#include <QRandomGenerator>
#endif

#include "database/ObjectStoreWrapper.h"
#include "Logging.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"

namespace {

   //! \brief True iff. a <= c <= b
   static bool inRange( double c, double a, double b )
   {
      return (a <= c) && (c <= b);
   }

   //! \brief True iff. b-tol <= a <= b+tol
   static bool fuzzyComp( double a, double b, double tol )
   {
      bool ret = inRange( a, b-tol, b+tol );
      if( !ret )
         qDebug() << QString("a: %1, b: %2, tol: %3").arg(a).arg(b).arg(tol);
      return ret;
   }

   // method to fill dummy logs with content to build size
   static QString randomStringGenerator()
   {
      QString posChars = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwwxyz";
      int randomcharLength = 64;

      QString randSTR;
      for (int i = 0; i < randomcharLength; i++)
      {
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
         int index = qrand() % posChars.length();
#else
         int index = QRandomGenerator().generate64() % posChars.length();
#endif
         QChar nChar = posChars.at(index);
         randSTR.append(nChar);
      }
      return randSTR;
   }

}

QTEST_MAIN(Testing)

void Testing::initTestCase() {

   // Initialize Xerces XML tools
   // NB: This is also where where we would initialise xalanc::XalanTransformer if we were using it
   try {
      xercesc::XMLPlatformUtils::Initialize();
   } catch (xercesc::XMLException const & xercesInitException) {
      qCritical() << Q_FUNC_INFO << "Xerces XML Parser Initialisation Failed: " << xercesInitException.getMessage();
      return;
   }
   std::cout << "Initialising Test Case" << std::endl;

   try {
      // Create a different set of options to avoid clobbering real options
      QCoreApplication::setOrganizationDomain("brewtarget.com/test");
      QCoreApplication::setApplicationName("brewtarget-test");

      // Set options so that any data modification does not affect any other data
      PersistentSettings::initialise(QDir::tempPath());

      // Log test setup
      // Verify that the Logging initializes normally
      qDebug() << "Initiallizing Logging module";
      Logging::initializeLogging();
      // Now change/override a few settings
      // We always want debug logging for tests as it's useful when a test fails
      Logging::setLogLevel(Logging::LogLevel_DEBUG);
      // Test logs go to a /tmp (or equivalent) so as not to clutter the application path with dummy data.
      Logging::setDirectory(QDir::tempPath(), Logging::NewDirectoryIsTemporary);
      qDebug() << "logging initialized";

      // Inside initializeLogging(), there's a check to see whether we're the test application.  If so, it turns off
      // logging output to stderr.
      qDebug() << Q_FUNC_INFO << "Initialised";

      PersistentSettings::insert(PersistentSettings::Names::color_formula, "morey");
      PersistentSettings::insert(PersistentSettings::Names::ibu_formula, "tinseth");

   // Tell Brewtarget not to require any "user" input on starting
   Brewtarget::setInteractive(false);
   QVERIFY( Brewtarget::initialize() );

      // 5 gallon equipment
      this->equipFiveGalNoLoss = std::make_shared<Equipment>();
      this->equipFiveGalNoLoss->setName("5 gal No Loss");
      this->equipFiveGalNoLoss->setBoilSize_l(24.0);
      this->equipFiveGalNoLoss->setBatchSize_l(20.0);
      this->equipFiveGalNoLoss->setTunVolume_l(40.0);
      this->equipFiveGalNoLoss->setTopUpWater_l(0);
      this->equipFiveGalNoLoss->setTrubChillerLoss_l(0);
      this->equipFiveGalNoLoss->setEvapRate_lHr(4.0);
      this->equipFiveGalNoLoss->setBoilTime_min(60);
      this->equipFiveGalNoLoss->setLauterDeadspace_l(0);
      this->equipFiveGalNoLoss->setTopUpKettle_l(0);
      this->equipFiveGalNoLoss->setHopUtilization_pct(100);
      this->equipFiveGalNoLoss->setGrainAbsorption_LKg(1.0);
      this->equipFiveGalNoLoss->setBoilingPoint_c(100);

      // Cascade pellets at 4% AA
      this->cascade_4pct = std::make_shared<Hop>();
      ObjectStoreWrapper::insert(this->cascade_4pct);
      this->cascade_4pct->setCacheOnly(false);
      this->cascade_4pct->setName("Cascade 4pct");
      this->cascade_4pct->setAlpha_pct(4.0);
      this->cascade_4pct->setUse(Hop::Boil);
      this->cascade_4pct->setTime_min(60);
      this->cascade_4pct->setType(Hop::Both);
      this->cascade_4pct->setForm(Hop::Leaf);

      // 70% yield, no moisture, 2 SRM
      this->twoRow = std::make_shared<Fermentable>();
      this->twoRow->setName("Two Row");
      this->twoRow->setType(Fermentable::Grain);
      this->twoRow->setYield_pct(70.0);
      this->twoRow->setColor_srm(2.0);
      this->twoRow->setMoisture_pct(0);
      this->twoRow->setIsMashed(true);
   } catch (std::exception const & e) {
      // When an exception gets to Qt, it will barf something along the lines of "Caught unhandled exception" without
      // leaving you much the wiser.  If we can intercept the exception along the way, we can ensure more details are
      // output to the console.
      std::cerr << "Caught exception: " << e.what() << std::endl;
      throw;
   }

   return;
}

void Testing::recipeCalcTest_allGrain()
{
   return;
   double const grain_kg = 5.0;
   double const conversion_l = grain_kg * 2.8; // 2.8 L/kg mash thickness
   auto rec = std::make_shared<Recipe>("TestRecipe");

   // Basic recipe parameters
   rec->setBatchSize_l(equipFiveGalNoLoss->batchSize_l());
   rec->setBoilSize_l(equipFiveGalNoLoss->boilSize_l());
   rec->setEfficiency_pct(70.0);

   // Single conversion, single sparge
   auto singleConversion = std::make_shared<Mash>();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);
   auto singleConversion_convert = std::make_shared<MashStep>();
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Infusion);
   singleConversion_convert->setInfuseAmount_l(conversion_l);
   singleConversion->addMashStep(singleConversion_convert);
   auto singleConversion_sparge = std::make_shared<MashStep>();
   singleConversion_sparge->setName("Sparge");
   singleConversion_sparge->setType(MashStep::Infusion);
   singleConversion_sparge->setInfuseAmount_l(
      rec->boilSize_l()
      + equipFiveGalNoLoss->grainAbsorption_LKg() * grain_kg // Grain absorption
      - conversion_l // Water we already added
   );
   singleConversion->addMashStep(singleConversion_sparge);

   // Add equipment
   rec->setEquipment(equipFiveGalNoLoss.get());

   // Add hops (85g)
   cascade_4pct->setAmount_kg(0.085);
   rec->add(this->cascade_4pct);

   // Add grain
   twoRow->setAmount_kg(grain_kg);
   rec->add<Fermentable>(this->twoRow);

   // Add mash
   rec->setMash(singleConversion.get());

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
   return;
   double const grain_kg = 5.0;
   Recipe* recNoLoss = new Recipe(QString("TestRecipe_noLoss"));
   Recipe* recLoss = new Recipe(QString("TestRecipe_loss"));
   Equipment* eLoss = new Equipment(*equipFiveGalNoLoss.get());

   // Only difference between the recipes:
   // - 2 L of post-boil loss
   // - 2 L extra of boil size (to hit the same batch size)
   eLoss->setTrubChillerLoss_l(2.0);
   eLoss->setBoilSize_l(equipFiveGalNoLoss->boilSize_l() + eLoss->trubChillerLoss_l());

   // Basic recipe parameters
   recNoLoss->setBatchSize_l(equipFiveGalNoLoss->batchSize_l());
   recNoLoss->setBoilSize_l(equipFiveGalNoLoss->boilSize_l());
   recNoLoss->setEfficiency_pct(70.0);

   recLoss->setBatchSize_l(eLoss->batchSize_l() - eLoss->trubChillerLoss_l()); // Adjust for trub losses
   recLoss->setBoilSize_l(eLoss->boilSize_l() - eLoss->trubChillerLoss_l());
   recLoss->setEfficiency_pct(70.0);

   double mashWaterNoLoss_l = recNoLoss->boilSize_l()
      + equipFiveGalNoLoss->grainAbsorption_LKg() * grain_kg
   ;
   double mashWaterLoss_l = recLoss->boilSize_l()
      + eLoss->grainAbsorption_LKg() * grain_kg
   ;

   // Add equipment
   recNoLoss->setEquipment(equipFiveGalNoLoss.get());
   recLoss->setEquipment(eLoss);

   // Add grain
   twoRow->setAmount_kg(grain_kg);
   recNoLoss->add<Fermentable>(twoRow);
   recLoss->add<Fermentable>(twoRow);

   // Single conversion, no sparge
   auto singleConversion = std::make_shared<Mash>();
   singleConversion->setName("Single Conversion");
   singleConversion->setGrainTemp_c(20.0);
   singleConversion->setSpargeTemp_c(80.0);

   auto singleConversion_convert = std::make_shared<MashStep>();
   singleConversion_convert->setName("Conversion");
   singleConversion_convert->setType(MashStep::Infusion);
   singleConversion->addMashStep(singleConversion_convert);

   // Infusion for recNoLoss
   singleConversion_convert->setInfuseAmount_l(mashWaterNoLoss_l);
   recNoLoss->setMash(singleConversion.get());

   // Infusion for recLoss
   singleConversion_convert->setInfuseAmount_l(mashWaterLoss_l);
   recLoss->setMash(singleConversion.get());

   // Verify we hit the right boil/final volumes (that the test is sane)
   QVERIFY2( fuzzyComp(recNoLoss->boilVolume_l(),  recNoLoss->boilSize_l(),  0.1),     "Wrong boil volume calculation (recNoLoss)" );
   QVERIFY2( fuzzyComp(recLoss->boilVolume_l(),    recLoss->boilSize_l(),    0.1),     "Wrong boil volume calculation (recLoss)" );
   QVERIFY2( fuzzyComp(recNoLoss->finalVolume_l(), recNoLoss->batchSize_l(), 0.1),     "Wrong final volume calculation (recNoLoss)" );
   QVERIFY2( fuzzyComp(recLoss->finalVolume_l(),   recLoss->batchSize_l(),   0.1),     "Wrong final volume calculation (recLoss)" );

   // The OG calc itself is verified in recipeCalcTest_*(), so just verify that
   // the two OGs are the same
   QVERIFY2( fuzzyComp(recLoss->og(), recNoLoss->og(), 0.002), "OG of recipe with post-boil loss is different from no-loss recipe" );
}

void Testing::testLogRotation() {
   // Turning off logging to stderr console, this is so you won't have to watch 100k rows generate in the console.
   Logging::setLoggingToStderr(false);

   //generate 40 000 log rows giving roughly 10 files with dummy/random logs
   // This should have to log rotate a few times leaving 5 log files in the directory which we can test for size and number of files.
   for (int i=0; i < 8000; i++) {
      qDebug() << QString("iteration %1-1; (%2)").arg(i).arg(randomStringGenerator());
      qWarning() << QString("iteration %1-2; (%2)").arg(i).arg(randomStringGenerator());
      qCritical() << QString("iteration %1-3; (%2)").arg(i).arg(randomStringGenerator());
      qInfo() << QString("iteration %1-4; (%2)").arg(i).arg(randomStringGenerator());
   }

   // Put logging back to normal
   Logging::setLoggingToStderr(true);

   QFileInfoList fileList = Logging::getLogFileList();
   //There is always a "logFileCount" number of old files + 1 current file
   QCOMPARE(fileList.size(), Logging::logFileCount + 1);

   for (int i = 0; i < fileList.size(); i++)
   {
      QFile f(QString(fileList.at(i).canonicalFilePath()));
      //Here we test if the file is more than 10% bigger than the specified logFileSize", if so, fail.
      QVERIFY2(f.size() <= (Logging::logFileSize * 1.1), "Wrong Sized file");
   }
   return;
}

void Testing::cleanupTestCase()
{
   Brewtarget::cleanup();
   Logging::terminateLogging();
   //Clean up the gibberish logs from disk by removing the
   QFileInfoList fileList = Logging::getLogFileList();
   for (int i = 0; i < fileList.size(); i++) {
      QFile(QString(fileList.at(i).canonicalFilePath())).remove();
   }

   // Clear all persistent properties linked with this test suite.
   // It will clear all settings that are application specific, user-scoped, and in the Brewtarget namespace.
   QSettings().clear();

   //
   // Clean exit of Xerces XML tools
   // If we, in future, want to use XalanTransformer, this needs to be extended to:
   //    XalanTransformer::terminate();
   //    XMLPlatformUtils::Terminate();
   //    XalanTransformer::ICUCleanUp();
   //
   xercesc::XMLPlatformUtils::Terminate();

   return;
}


void Testing::pstdintTest() {
   QVERIFY( sizeof(int8_t) == 1 );
   QVERIFY( sizeof(int16_t) == 2 );
   QVERIFY( sizeof(int32_t) == 4 );
#ifdef stdint_int64_defined
   QVERIFY( sizeof(int64_t) == 8 );
#endif

   QVERIFY( sizeof(uint8_t) == 1 );
   QVERIFY( sizeof(uint16_t) == 2 );
   QVERIFY( sizeof(uint32_t) == 4 );
#ifdef stdint_int64_defined
   QVERIFY( sizeof(uint64_t) == 8 );
#endif
   return;
}


void Testing::runTest()
{
   QVERIFY( 1==1 );
   /*
   MainWindow* mw = Brewtarget::mainWindow();
   QVERIFY( mw );
   */
}
