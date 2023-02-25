/*
 * Testing.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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

#include <cmath>
#include <exception>
#include <iostream> // For std::cout
#include <math.h>
#include <memory>

#include <xercesc/util/PlatformUtils.hpp>

#include <QDebug>
#include <QString>
#include <QtTest/QtTest>
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
#include <QtGlobal> // For qrand() -- which is superseded by QRandomGenerator in later versions of Qt
#else
#include <QRandomGenerator>
#endif
#include <QVector>

#include "Algorithms.h"
#include "config.h"
#include "database/ObjectStoreWrapper.h"
#include "Localization.h"
#include "Logging.h"
#include "measurement/Measurement.h"
#include "measurement/Unit.h"
#include "measurement/UnitSystem.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/MashStep.h"
#include "model/NamedParameterBundle.h"
#include "model/Recipe.h"
#include "PersistentSettings.h"

namespace {

   struct SgBrixEquivalance {
      double sg;
      double brix;
      double errorMarginSgToBrix;
      double errorMarginBrixToSg;
   };

   //
   // We test conversion in both directions Brix -> SG and SG -> Brix.  As commented elsewhere, I don't think many
   // brewers use Brix.  We need Brix -> SG at least to be able to read any BeerJSON file that uses Brix.  I think our
   // method (of interpolating the USDA published data) is likely more accurate than the best-fit cubic equation that
   // most folks use.  However, for now, we're just testing here that our method is not hugely different than those
   // used to generate tables on popular homebrew sites.
   //
   QVector<SgBrixEquivalance> const sgBrixEquivalances {
      //
      // With a few exceptions, the data below come from
      // https://winning-homebrew.com/wp-content/uploads/2021/03/sgtobrix_conv_table.pdf (linked to from
      // https://winning-homebrew.com/specific-gravity-to-brix.html).  It's not clear exactly how those figures were
      // obtained, so it's not necessarily an error if we get slightly different results.  Still, it would be surprising
      // if we were a long way adrift from these numbers.
      //

      //                Error Margins
      // SG    Brix   SG->Brix  Brix->SG
      {1.00156,  0.4, 0.00001, 0.00001},
      {0.990,  0.00,  0.005,    0.0110},  // ===========================================================================
      {0.991,  0.00,  0.005,    0.0100},  //
      {0.992,  0.00,  0.005,    0.0090},  // For SG < 1.0, the conversion between Brix and SG is not very meaningful:
      {0.993,  0.00,  0.005,    0.0080},  //  - SG to Brix will always give 0
      {0.994,  0.00,  0.005,    0.0070},  //  - Brix to SG will always give 1.0
      {0.995,  0.00,  0.005,    0.0060},  //
      {0.996,  0.00,  0.005,    0.0050},  // We leave these test cases in for now, but with larger error bars then the
      {0.997,  0.00,  0.005,    0.0040},  // more meaningful conversions below
      {0.998,  0.00,  0.005,    0.0030},  //
      {0.999,  0.00,  0.005,    0.0020},  // ===========================================================================
      {1.000,  0.00,  0.005,    0.0002},
      {1.001,  0.26,  0.010,    0.0002},
      {1.002,  0.51,  0.010,    0.0002},
      {1.003,  0.77,  0.010,    0.0002},
      {1.004,  1.03,  0.010,    0.0002},
      {1.005,  1.28,  0.010,    0.0002},
      {1.006,  1.54,  0.010,    0.0002},
      {1.007,  1.80,  0.010,    0.0002},
      {1.008,  2.05,  0.010,    0.0002},
      {1.009,  2.31,  0.010,    0.0002},
      {1.010,  2.56,  0.010,    0.0002},
      {1.011,  2.81,  0.010,    0.0002},
      {1.012,  3.07,  0.010,    0.0002},
      {1.013,  3.32,  0.010,    0.0002},
      {1.014,  3.57,  0.010,    0.0002},
      {1.015,  3.82,  0.010,    0.0002},
      {1.016,  4.08,  0.010,    0.0002},
      {1.017,  4.33,  0.010,    0.0002},
      {1.018,  4.58,  0.010,    0.0002},
      {1.019,  4.83,  0.010,    0.0002},
      {1.020,  5.08,  0.010,    0.0002},
      {1.021,  5.33,  0.010,    0.0002},
      {1.022,  5.57,  0.020,    0.0002},
      {1.023,  5.82,  0.020,    0.0002},
      {1.024,  6.07,  0.020,    0.0002},
      {1.025,  6.32,  0.020,    0.0002},
      {1.026,  6.57,  0.020,    0.0002},
      {1.027,  6.81,  0.020,    0.0002},
      {1.028,  7.06,  0.020,    0.0002},
      {1.029,  7.30,  0.020,    0.0002},
      {1.030,  7.55,  0.020,    0.0002},
      {1.031,  7.80,  0.020,    0.0002},
      {1.032,  8.04,  0.020,    0.0002},
      {1.033,  8.28,  0.020,    0.0002},
      {1.034,  8.53,  0.020,    0.0002},
      {1.035,  8.77,  0.020,    0.0002},
      {1.036,  9.01,  0.020,    0.0002},
      {1.037,  9.26,  0.020,    0.0002},
      {1.038,  9.50,  0.020,    0.0002},
      {1.039,  9.74,  0.020,    0.0002},
      {1.040,  9.98,  0.020,    0.0002},
      {1.041, 10.22,  0.020,    0.0002},
      {1.042, 10.46,  0.020,    0.0002},
      {1.043, 10.70,  0.020,    0.0002},
      {1.044, 10.94,  0.020,    0.0002},
      {1.045, 11.18,  0.020,    0.0002},
      {1.046, 11.42,  0.020,    0.0002},
      {1.047, 11.66,  0.020,    0.0002},
      {1.048, 11.90,  0.020,    0.0002},
      {1.049, 12.14,  0.020,    0.0002},
      {1.050, 12.37,  0.030,    0.0002},
      {1.051, 12.61,  0.030,    0.0002},
      {1.052, 12.85,  0.030,    0.0002},
      {1.053, 13.08,  0.030,    0.0002},
      {1.054, 13.32,  0.030,    0.0002},
      {1.055, 13.55,  0.030,    0.0002},
      {1.056, 13.79,  0.030,    0.0002},
      {1.057, 14.02,  0.030,    0.0002},
      {1.058, 14.26,  0.030,    0.0002},
      {1.059, 14.49,  0.030,    0.0002},
      {1.060, 14.72,  0.030,    0.0002},
      {1.061, 14.96,  0.030,    0.0002},
      {1.062, 15.19,  0.030,    0.0002},
      {1.063, 15.42,  0.030,    0.0002},
      {1.064, 15.65,  0.030,    0.0002},
      {1.065, 15.88,  0.030,    0.0002},
      {1.066, 16.11,  0.030,    0.0002},
      {1.067, 16.34,  0.030,    0.0002},
      {1.068, 16.57,  0.030,    0.0002},
      {1.069, 16.80,  0.030,    0.0002},
      {1.070, 17.03,  0.030,    0.0002},
      {1.071, 17.26,  0.030,    0.0002},
      {1.072, 17.49,  0.030,    0.0002},
      {1.073, 17.72,  0.030,    0.0002},
      {1.074, 17.95,  0.030,    0.0002},
      {1.075, 18.18,  0.030,    0.0002},
      {1.076, 18.40,  0.030,    0.0002},
      {1.077, 18.63,  0.030,    0.0002},
      {1.078, 18.86,  0.030,    0.0002},
      {1.079, 19.08,  0.030,    0.0002},
      {1.080, 19.31,  0.030,    0.0002},
      {1.081, 19.53,  0.030,    0.0002},
      {1.082, 19.76,  0.030,    0.0002},
      {1.083, 19.98,  0.030,    0.0002},
      {1.084, 20.21,  0.030,    0.0002},
      {1.085, 20.43,  0.030,    0.0002},
      {1.086, 20.65,  0.030,    0.0002},
      {1.087, 20.88,  0.030,    0.0002},
      {1.088, 21.10,  0.030,    0.0002},
      {1.089, 21.32,  0.030,    0.0002},
      {1.090, 21.54,  0.032,    0.0002},  // <- Wider difference on SG->Brix
      {1.091, 21.77,  0.030,    0.0002},
      {1.092, 21.99,  0.030,    0.0002},
      {1.093, 22.21,  0.030,    0.0002},
      {1.094, 22.43,  0.030,    0.0002},
      {1.095, 22.65,  0.030,    0.0002},
      {1.096, 22.87,  0.030,    0.0002},
      {1.097, 23.09,  0.030,    0.0002},
      {1.098, 23.31,  0.030,    0.0002},
      {1.099, 23.53,  0.030,    0.0002},
      {1.100, 23.75,  0.030,    0.0002},
      {1.101, 23.96,  0.035,    0.0002},
      {1.102, 24.18,  0.035,    0.0002},
      {1.103, 24.40,  0.035,    0.0002},
      {1.104, 24.62,  0.035,    0.0002},
      {1.105, 24.83,  0.035,    0.0002},
      {1.106, 25.05,  0.035,    0.0002},
      {1.107, 25.27,  0.035,    0.0002},
      {1.108, 25.48,  0.035,    0.0002},
      {1.109, 25.70,  0.035,    0.0002},
      {1.110, 25.91,  0.035,    0.0002},
      {1.111, 26.13,  0.035,    0.0002},
      {1.112, 26.34,  0.035,    0.0002},
      {1.113, 26.56,  0.035,    0.0002},
      {1.114, 26.77,  0.035,    0.0002},
      {1.115, 26.98,  0.035,    0.0002},
      {1.116, 27.20,  0.035,    0.0002},
      {1.117, 27.41,  0.035,    0.0002},
      {1.118, 27.62,  0.035,    0.0002},
      {1.119, 27.83,  0.036,    0.0002},  // <- Wider difference on SG->Brix
      {1.120, 28.05,  0.035,    0.0002},
      {1.121, 28.26,  0.035,    0.0002},
      {1.122, 28.47,  0.035,    0.0002},
      {1.123, 28.68,  0.035,    0.0002},
      {1.124, 28.89,  0.035,    0.0002},
      {1.125, 29.10,  0.035,    0.0002},
      {1.126, 29.31,  0.035,    0.0002},
      {1.127, 29.52,  0.035,    0.0002},
      {1.128, 29.73,  0.035,    0.0002},
      {1.129, 29.94,  0.035,    0.0002},
      {1.130, 30.15,  0.035,    0.0002},
   };


   //! \brief True iff a <= c <= b
   constexpr bool inRange(double c, double a, double b) {
      return (a <= c) && (c <= b);
   }

   //! \brief True iff b - tolerance <= a <= b + tolerance
   bool fuzzyComp(double a, double b, double tolerance) {
      bool ret = inRange(a, b - tolerance, b + tolerance);
      if (!ret) {
         qDebug() << Q_FUNC_INFO << "a:" << a << ", b:" << b << ", diff:" << abs(a - b) << ", tolerance:" << tolerance;
      }
      return ret;
   }

   // method to fill dummy logs with content to build size
   QString randomStringGenerator() {
      QString const posChars = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwwxyz";
      int constexpr randomcharLength = 64;

      QString randSTR;
      for (int i = 0; i < randomcharLength; i++) {
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

Testing::Testing() :
   QObject(),
   tempDir{QDir::tempPath()},
   equipFiveGalNoLoss{},
   cascade_4pct{},
   twoRow{} {
   //
   // Create a unique temporary directory using the current thread ID as part of a subdirectory name inside whatever
   // system-standard temp directory Qt proposes to us.  (We also put the application name in the subdirectory name so
   // that anyone doing a manual clean up of their temp directory doesn't have to guess or wonder what created it.
   // Mostly our temp subdirectories will be deleted in our destructor, but core dumps happen etc.)
   //
   // This is important when using the Meson build system because Meson runs several unit tests in parallel (whereas
   // CMake executes them sequentially).  We are guaranteed a separate instance of this class for each run because
   // both CMake and Meson invoke unit tests by running a program.
   //
   QString subDirName;
   QTextStream{&subDirName} << CONFIG_APPLICATION_NAME_UC << "-UnitTestRun-" << QThread::currentThreadId();
   if (!this->tempDir.mkdir(subDirName)) {
      qCritical() <<
         Q_FUNC_INFO << "Unable to create" << subDirName << "sub-directory of" << this->tempDir.absolutePath();
      throw std::runtime_error{"Unable to create unique temp directory"};
   }
   if (!this->tempDir.cd(subDirName)) {
      qCritical() <<
         Q_FUNC_INFO << "Unable to access" << this->tempDir.absolutePath() << "after creating it";
      throw std::runtime_error{"Unable to access unique temp directory"};
   }

   qDebug() << Q_FUNC_INFO << "Using" << this->tempDir.absolutePath() << "as temporary directory";
   return;
}

Testing::~Testing() {
   //
   // We have to be a bit careful in our cleaning up.  We only want to try to remove the unique temporary directory we
   // created, not the system-wide one.  (It shouldn't be possible for this->tempDir to be the root directory, but it
   // doesn't hurt to check!)
   //
   if (this->tempDir.exists() &&
       this->tempDir.absolutePath() != QDir::tempPath() &&
       !this->tempDir.isRoot()) {
      qInfo() << Q_FUNC_INFO << "Removing temporary directory" << this->tempDir.absolutePath() << "and its contents";
      if (!this->tempDir.removeRecursively()) {
         //
         // It's not the end of the world if we couldn't remove a temporary directory so, if it happens, just log an
         // error rather than throwing an exception (which might prevent other clean-up from happening).
         //
         qInfo() << Q_FUNC_INFO << "Unable to remove temporary directory" << this->tempDir.absolutePath();
      }
   }
   return;
}

//
// If you're building with CMake:
//   - Ensure each unit test has an "ADD_TEST" line in the main CMakeLists.txt
//   - Run unit tests with  make test
//   - Debug log output is in build/Testing/Temporary/LastTest.log (assuming "build" is your CMake build directory)
//
// If you're building with Meson (which NB is not yet fully supported!):
//   - Ensure each unit test has a "test" line in meson.build
//   - Run unit tests with  meson test
//   - Debug log output is in mbuild/meson-logs/testlog.txt (assuming "mbuild" is your Meson build directory)
//
// QTEST_MAIN generates (via horrible macros) a main() function for the unit test runner
//
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
      PersistentSettings::initialise(this->tempDir.absolutePath());

      // Log test setup
      // Verify that the Logging initializes normally
      qDebug() << "Initiallizing Logging module";
      Logging::initializeLogging();
      // Now change/override a few settings
      // We always want debug logging for tests as it's useful when a test fails
      Logging::setLogLevel(Logging::LogLevel_DEBUG);
      // Test logs go to a /tmp (or equivalent) so as not to clutter the application path with dummy data.
      Logging::setDirectory(this->tempDir.absolutePath(), Logging::NewDirectoryIsTemporary);
      qDebug() << "logging initialized";

      // Inside initializeLogging(), there's a check to see whether we're the test application.  If so, it turns off
      // logging output to stderr.
      qDebug() << Q_FUNC_INFO << "Initialised";

      PersistentSettings::insert(PersistentSettings::Names::color_formula, "morey");
      PersistentSettings::insert(PersistentSettings::Names::ibu_formula, "tinseth");

   // Tell Brewtarget not to require any "user" input on starting
      Application::setInteractive(false);

      //
      // Application::initialize() will initialise a bunch of things, including creating a default database in
      // this->tempDir courtesy of the call to PersistentSettings::initialise() above.  If there is a problem creating the DB,
      // it will return false.
      //
      QVERIFY(Application::initialize());

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
      this->cascade_4pct->setName("Cascade 4pct");
      this->cascade_4pct->setAlpha_pct(4.0);
      this->cascade_4pct->setUse(Hop::Use::Boil);
      this->cascade_4pct->setTime_min(60);
      this->cascade_4pct->setType(Hop::Type::Both);
      this->cascade_4pct->setForm(Hop::Form::Leaf);

      // 70% yield, no moisture, 2 SRM
      this->twoRow = std::make_shared<Fermentable>();
      this->twoRow->setName("Two Row");
      this->twoRow->setType(Fermentable::Type::Grain);
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

void Testing::recipeCalcTest_allGrain() {
   // .:TODO:. Would be good to fix and reinstate this test...
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
   singleConversion_convert->setType(MashStep::Type::Infusion);
   singleConversion_convert->setInfuseAmount_l(conversion_l);
   singleConversion->addMashStep(singleConversion_convert);
   auto singleConversion_sparge = std::make_shared<MashStep>();
   singleConversion_sparge->setName("Sparge");
   singleConversion_sparge->setType(MashStep::Type::Infusion);
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

void Testing::postBoilLossOgTest() {
   // .:TODO:. Would be good to fix and reinstate this test...
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
   singleConversion_convert->setType(MashStep::Type::Infusion);
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

void Testing::testUnitConversions() {
   //
   // Originally, some of these tests assumed '.' is the decimal separator and ',' is the digit group separator.  This
   // meant they would fail on locales where this is not the case.  Plan A was just to force the locale to be one that
   // makes the tests work (because we are testing conversions rather than number parsing, eg via:
   //    QLocale::setDefault(QLocale::C);
   // However, this didn't seem to have any effect on a French locale Windows.
   // So Plan B is to construct test data based on the current locale settings.
   //
   QString const decimalSeparator   = Localization::getLocale().decimalPoint();
   QString const thousandsSeparator = Localization::getLocale().groupSeparator();
   qDebug() <<
      Q_FUNC_INFO << "Decimal separator is " << decimalSeparator << " | Thousands separator is " << thousandsSeparator;
   QString testInput{};
   QTextStream testInputAsStream{&testInput};
   // "5.500 gal"
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500 gal";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::volume_UsCustomary.qstringToSI(testInput, // "5.500 gal"
                                                                               Measurement::Units::liters).quantity(),
                      20.820,
                      0.001),
            "Unit conversion error (US gallons to Litres v1)");
   // "5.500"
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::volume_UsCustomary.qstringToSI(testInput, // "5.500"
                                                                               Measurement::Units::us_gallons).quantity(),
                      20.820,
                      0.001),
            "Unit conversion error (US gallons to Litres v2)");
   // "5.500 gal"
   testInput.clear();
   testInputAsStream << "5" << decimalSeparator << "500 gal";
   QVERIFY2(fuzzyComp(Measurement::qStringToSI(testInput, // "5.500 gal"
                                               Measurement::PhysicalQuantity::Volume).quantity(),
                      20.820,
                      0.001),
                      "Unit conversion error (US gallons to Litres v3)");
   // "9.994 P"
   testInput.clear();
   testInputAsStream << "9" << decimalSeparator << "994 P";
   QVERIFY2(fuzzyComp(Measurement::UnitSystems::density_Plato.qstringToSI(testInput, // "9.994 P"
                                                                          Measurement::Units::sp_grav).quantity(),
                      1.040,
                      0.001),
            "Unit conversion error (Plato to SG)");
   // "1,083 ebc"
   testInput.clear();
   testInputAsStream << "1" << thousandsSeparator << "083 ebc";
   QVERIFY2(
      fuzzyComp(Measurement::UnitSystems::color_StandardReferenceMethod.qstringToSI(testInput, // "1,083 ebc"
                                                                                    Measurement::Units::srm).quantity(),
                550,
                1),
      "Unit conversion error (EBC to SRM)"
   );

   return;
}

void Testing::testNamedParameterBundle() {

   //
   // First some basic tests
   //
   NamedParameterBundle npb;

   BtStringConst const myInt{"myInt"};
   npb.insert(myInt, 42);
   QVERIFY2(npb.get(myInt).toInt() == 42, "Error retrieving int");

   BtStringConst const myFalseBool{"myFalseBool"};
   npb.insert(myFalseBool, false);
   QVERIFY2(!npb.get(myFalseBool).toBool(), "Error retrieving false bool");

   BtStringConst const myTrueBool{"myTrueBool"};
   npb.insert(myTrueBool, true);
   QVERIFY2(npb.get(myTrueBool).toBool(), "Error retrieving true bool");

   BtStringConst const myString{"myString"};
   npb.insert(myString, "Sing a string of sixpence");
   QVERIFY2(npb.get(myString).toString() == "Sing a string of sixpence", "Error retrieving string");

   BtStringConst const myDouble{"myDouble"};
   npb.insert(myDouble, 3.1415926535897932384626433);
   QVERIFY2(fuzzyComp(npb.get(myDouble).toDouble(),
                      3.1415926535897932384626433,
                      0.0000000001),
            "Error retrieving double");
/*
   //
   // Test that we can store an int and get it back as a strongly-typed enum
   //
   npb.insert(PropertyNames::Hop::type, static_cast<int>(Hop::Type::AromaAndFlavor));
   Hop::Type retrievedHopType = npb.val<Hop::Type>(PropertyNames::Hop::type);
   QVERIFY2(retrievedHopType == Hop::Type::AromaAndFlavor, "Int -> Strongly-typed enum failed");

   //
   // Now test explicitly nullable fields -- UNCOMMENT THIS ONCE OPTIONAL FIELDS IMPLEMENTED IN BREWTARGET
   //
   QVERIFY2(
      npb.optEnumVal<Fermentable::GrainGroup>(PropertyNames::Fermentable::grainGroup) == std::nullopt,
      "Error getting default value for optional enum"
   );
   std::optional<int> myOptional{static_cast<int>(Fermentable::GrainGroup::Smoked)};
   QVariant myVariant = QVariant::fromValue(myOptional);
   npb.insert(PropertyNames::Fermentable::grainGroup, myVariant);

   QVariant retrievedValueA = npb.get(PropertyNames::Fermentable::grainGroup);
   std::optional<int> castValueA = retrievedValueA.value< std::optional<int> >();
   QVERIFY2(castValueA.has_value(), "Error retrieving optional enum");
   QVERIFY2(castValueA.value() == static_cast<int>(Fermentable::GrainGroup::Smoked),
            "Error retrieving optional enum as int");

   auto retrievedValueB = npb.optEnumVal<Fermentable::GrainGroup>(PropertyNames::Fermentable::grainGroup);
   qDebug() <<
      Q_FUNC_INFO << "retrievedValueB=" << (retrievedValueB.has_value() ? static_cast<int>(*retrievedValueB) : -999) <<
      "; Fermentable::GrainGroup::Smoked = " << static_cast<int>(Fermentable::GrainGroup::Smoked);
   QVERIFY2(retrievedValueB.has_value(), "Expected value, got none");
   QVERIFY2(
      retrievedValueB.value() == Fermentable::GrainGroup::Smoked,
      "Error retrieving optional enum"
   );
*/
   return;
}

void Testing::testAlgorithms() {
   for (auto const & ii : sgBrixEquivalances) {
      qDebug() <<
         Q_FUNC_INFO << "Testing conversions between" << ii.sg << "SG = " << ii.brix << "Brix; SG->Brix=" <<
         Algorithms::SgAt20CToBrix(ii.sg) << "(" << std::abs(ii.brix - Algorithms::SgAt20CToBrix(ii.sg)) <<
         "); Brix->SG=" << Algorithms::BrixToSgAt20C(ii.brix) << "(" <<
         std::abs(Algorithms::BrixToSgAt20C(ii.brix) - ii.sg) << ")";
      QVERIFY2(
         fuzzyComp(Algorithms::BrixToSgAt20C(ii.brix), ii.sg, ii.errorMarginBrixToSg),
         "Error converting Brix to Specific Gravity"
      );
      QVERIFY2(
         fuzzyComp(Algorithms::SgAt20CToBrix(ii.sg), ii.brix, ii.errorMarginSgToBrix),
         "Error converting Specific Gravity to Brix"
      );
   }
   return;
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
   Application::cleanup();
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
   //
   // .:TBD:. I'm not sure this is the most useful of unit tests.  We're effectively checking that one tiny part of the
   // C++11 standard is correctly implemented by the compiler.
   //
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


void Testing::runTest() {
   // .:TBD:. We should probably retire this function... :o)
   QVERIFY( 1==1 );
   /*
   MainWindow& mw = Application::mainWindow();
   QVERIFY( mw );
   */
}
