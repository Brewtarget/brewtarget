/*======================================================================================================================
 * unitTests/SerializationTests.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "unitTests/SerializationTests.h"

#include <cmath>
#include <limits>
#include <optional>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QString>
#include <QTextStream>
#include <QtTest/QtTest>

#include <boost/json.hpp>

#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/BrewLog.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Hop.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Recipe.h"
#include "model/RecipeAdditionFermentable.h"
#include "model/RecipeAdditionHop.h"
#include "model/RecipeAdditionYeast.h"
#include "model/RecipeUtils.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "serialization/NullSentinel.h"
#include "serialization/json/beerJson/BeerJson.h"
#include "serialization/json/dotBeer/DotBeer.h"
#include "serialization/xml/BeerXml.h"
#include "unitTests/TestData.h"
#include "unitTests/TestEnvironment.h"
#include "unitTests/TestRegistry.h"
#include "utils/OptionalHelpers.h"

REGISTER_TEST_CLASS(SerializationTests)

namespace {
   using UnitTests::fuzzyComp;

   bool importDotBeerFile(QString const & filePath, QString & message) {
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      bool const result = DotBeer::import("", filePath, userMessageAsStream);
      userMessageAsStream.flush();
      message = userMessage;
      return result;
   }

   //! \brief Create a brew log for the supplied recipe, the same way MainWindow does
   std::shared_ptr<BrewLog> makeBrewLog(Recipe & recipe, QString const & name) {
      auto brewLog = std::make_shared<BrewLog>(recipe);
      brewLog->populateNote(&recipe);
      brewLog->setName(name);
      ObjectStoreWrapper::insert(brewLog);
      return brewLog;
   }
}

class SerializationTests::impl {
public:
   impl() = default;
   ~impl() = default;
};

SerializationTests::SerializationTests() : UnitTests::TestClassBase(), pimpl{std::make_unique<impl>()} {
   return;
}

SerializationTests::~SerializationTests() = default;

void SerializationTests::initTestClass() {
   QString errorMessage;
   QVERIFY2(UnitTests::importDotBeerExamples(errorMessage), qPrintable(errorMessage));
   return;
}

void SerializationTests::testDotBeerImportedRecipeCalcs() {
   //
   // Check that calculated values on an imported Recipe are right, ie were worked out once all its ingredients were
   // there, rather than (as used to happen) when it was first stored in the database without them.  These are just
   // sanity checks -- the file says OG 1.052 and 38 IBUs, but we don't expect Recipe's calculations to match exactly.
   //
   {
      auto const recipe = ObjectStoreWrapper::findFirstMatching<Recipe>(
         [](std::shared_ptr<Recipe> rec) { return rec->name() == "Cascade Pale Ale"; }
      );
      QVERIFY2(recipe, "Could not find Cascade Pale Ale recipe");
      qInfo() << Q_FUNC_INFO << "Cascade Pale Ale: OG" << recipe->og() << ", IBU" << recipe->IBU();
      QVERIFY2(recipe->og() > 1.040 && recipe->og() < 1.065, "Wrong OG for imported recipe");
      QVERIFY2(recipe->IBU() > 10.0                        , "Wrong IBU for imported recipe");
   }
   //
   // These recipes have, respectively, a boil with no pre-boil size and no boil at all.  Boil gravity used to come out
   // at 2.33 SG in such cases, which gave near-zero IBUs.
   //
   for (QString const recipeName : {"Cold Crash Pils", "Cascade SMaSH"}) {
      auto const recipe = ObjectStoreWrapper::findFirstMatching<Recipe>(
         [&recipeName](std::shared_ptr<Recipe> rec) { return rec->name() == recipeName; }
      );
      QVERIFY2(recipe, qPrintable(QString{"Could not find %1 recipe"}.arg(recipeName)));
      qInfo() << Q_FUNC_INFO << recipeName << ": boil gravity" << recipe->boilGrav() << ", IBU" << recipe->IBU();
      QVERIFY2(recipe->boilGrav() > 1.01 && recipe->boilGrav() < 1.1, qPrintable(QString{"Wrong boil gravity for %1"}.arg(recipeName)));
      QVERIFY2(recipe->IBU() > 5.0, qPrintable(QString{"Wrong IBU for %1"}.arg(recipeName)));
   }
   return;
}

void SerializationTests::testDotBeerAdditionStepCorrection() {
   //
   // AdditionStepOutOfRange.beer has a mash addition for step 3 of a 1-step mash and a culture addition for step 2 of
   // a fermentation that doesn't exist.  We accept the file, but correct the steps.
   //
   auto const recipe = ObjectStoreWrapper::findFirstMatching<Recipe>(
      [](std::shared_ptr<Recipe> rec) { return rec->name() == "Step Confusion"; }
   );
   QVERIFY2(recipe, "Could not find Step Confusion recipe");
   auto const fermentableAdditions = recipe->fermentableAdditions();
   QVERIFY(fermentableAdditions.size() == 1);
   QVERIFY2(fermentableAdditions.first()->step() == 1, "Mash addition step not corrected");
   auto const yeastAdditions = recipe->yeastAdditions();
   QVERIFY(yeastAdditions.size() == 1);
   QVERIFY2(!yeastAdditions.first()->step(), "Fermentation addition step not unset");
   return;
}

void SerializationTests::testBrewLogCalcs() {
   //
   // Check brew log calculations don't give nonsense (or NaN) when gravities have not all been measured
   //
   auto const recipe = ObjectStoreWrapper::findFirstMatching<Recipe>(
      [](std::shared_ptr<Recipe> rec) { return rec->name() == "Cascade Pale Ale"; }
   );
   QVERIFY(recipe);
   auto brewLog = makeBrewLog(*recipe, "Test Brew Log");

   // populateNote pre-fills measured values with expected ones, so clear them to simulate "not yet measured"
   brewLog->setMeasuredOriginalGravity_sg(0.0);
   brewLog->setMeasuredFinalGravity_sg(0.0);

   // FG before OG
   brewLog->setMeasuredFinalGravity_sg(1.010);
   QVERIFY2(std::isfinite(brewLog->computedAlcoholByVolume_pct()) && brewLog->computedAlcoholByVolume_pct() == 0.0,
            "ABV should be 0 until OG measured");
   QVERIFY2(std::isfinite(brewLog->computedAttenuation_pct()) && brewLog->computedAttenuation_pct() == 0.0,
            "Attenuation should be 0 until OG measured");
   brewLog->setMeasuredFinalGravity_sg(0.0);

   // OG but no FG
   brewLog->setMeasuredVolumeIntoFv_l(21.0);
   brewLog->setMeasuredOriginalGravity_sg(1.050);
   QVERIFY2(brewLog->computedAlcoholByVolume_pct() == 0.0, "ABV should be 0 until FG measured");
   QVERIFY2(brewLog->computedAttenuation_pct()     == 0.0, "Attenuation should be 0 until FG measured");
   qInfo() << Q_FUNC_INFO << "Brewhouse efficiency" << brewLog->computedEfficiency_pct();
   QVERIFY2(brewLog->computedEfficiency_pct() > 40.0 && brewLog->computedEfficiency_pct() < 100.0,
            "Wrong brewhouse efficiency");

   // Both
   brewLog->setMeasuredFinalGravity_sg(1.010);
   qInfo() <<
      Q_FUNC_INFO << "ABV" << brewLog->computedAlcoholByVolume_pct() << ", attenuation" <<
      brewLog->computedAttenuation_pct();
   QVERIFY2(fuzzyComp(brewLog->computedAlcoholByVolume_pct(),  5.25, 0.2), "Wrong ABV");
   QVERIFY2(fuzzyComp(brewLog->computedAttenuation_pct()    , 80.0 , 0.1), "Wrong attenuation");
   return;
}

void SerializationTests::testDotBeerVersionHandling() {
   //
   // Check we give a helpful message for files written with a newer version of dotBeer than we support.  A file from a
   // newer minor version can still be read if it doesn't use anything we don't know about; a newer major version can't.
   //
   auto writeMinimalFile = [](QString const & fileName, QString const & version, QString const & extra) {
      QString const filePath = UnitTests::TestEnvironment::tempDir().filePath(fileName);
      QFile file{filePath};
      if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
         file.write(
            QString{"{\"DotBeer\": {\"version\": \"%1\", \"output_by\": \"Test\", "
                    "\"timestamp\": \"2026-10-24T10:00:00Z\"%2}}"}.arg(version).arg(extra).toUtf8()
         );
      }
      return filePath;
   };
   QString const hops{
      ", \"hops\": [{\"name\": \"Version Test Hop\", \"alpha_acid\": {\"unit\": \"%\", \"value\": 5.0}}]"
   };
   QString message;
   QVERIFY2(importDotBeerFile(writeMinimalFile("newerMinor.beer", "1.9.0", hops), message),
            qPrintable(QString{"Failed to read file from newer minor version: %1"}.arg(message)));
   QVERIFY2(!importDotBeerFile(writeMinimalFile("newerMajor.beer", "2.0.0", hops), message),
            "Should not read file from newer major version");
   QVERIFY2(message.contains("1.0.0") && message.contains("2.0.0"), qPrintable(message));
   QVERIFY2(!importDotBeerFile(writeMinimalFile("newerMinorUnknownField.beer",
                                         "1.9.0",
                                         hops + ", \"some_future_field\": true"),
                        message),
            "Should not read file with unknown field");
   QVERIFY2(message.contains("1.9.0"), qPrintable(message));
   return;
}

void SerializationTests::testDotBeerExport() {
   //
   // Add some things that are possible with our internal data structures, but which used to give an invalid dotBeer
   // file on export -- ie things that should be cleaned up for export.
   //
   {
      // We don't want the changes below to create new versions of Recipes
      RecipeUtils::SuspendRecipeVersioning suspendRecipeVersioning;

      // grain_group is only allowed for grains
      auto sugar = std::make_shared<Fermentable>();
      sugar->setName("Test Sugar With Leftover Grain Group");
      sugar->setType(Fermentable::Type::Grain);
      sugar->setGrainGroup(Fermentable::GrainGroup::Base);
      sugar->setType(Fermentable::Type::Sugar);
      sugar->setFineGrindYield_pct(100.0);
      sugar->setColor_srm(0.0);
      ObjectStoreWrapper::insert(sugar);
      QVERIFY2(!sugar->grainGroup(), "Grain group should not be returned for non-grain");

      // Water agents must have a water agent type
      auto untypedWaterAgent = std::make_shared<Misc>();
      untypedWaterAgent->setName("Test Water Agent Without Type");
      untypedWaterAgent->setType(Misc::Type::WaterAgent);
      ObjectStoreWrapper::insert(untypedWaterAgent);

      // Percent acid is allowed for "other" water agents (and needs to be kept)
      auto otherAcid = std::make_shared<Misc>();
      otherAcid->setName("Test Other Acid");
      otherAcid->setType(Misc::Type::WaterAgent);
      otherAcid->setWaterAgentType(Misc::WaterAgentType::Other);
      otherAcid->setWaterAgentPercentAcid(40.0);
      ObjectStoreWrapper::insert(otherAcid);
      QVERIFY2(otherAcid->waterAgentPercentAcid() && fuzzyComp(*otherAcid->waterAgentPercentAcid(), 40.0, 0.001),
               "Percent acid lost for Other water agent");

      // BrewLog::calculateActualABV_pct sets NaN if FG is not less than OG (eg if neither has been measured yet)
      auto const nanRecipe = UnitTests::findByName<Recipe>("Cascade Pale Ale");
      QVERIFY(nanRecipe);
      auto const nanBrewLog = makeBrewLog(*nanRecipe, "Test Brew Log With NaN ABV");
      nanBrewLog->setComputedAlcoholByVolume_pct(std::numeric_limits<double>::quiet_NaN());

      // Optional vessels with zero volume are treated as not present, so their other attributes are not kept
      auto vessels = std::make_shared<Equipment>("Test Equipment With Unused Lauter Tun");
      vessels->setKettleBoilSize_l(30.0);
      vessels->setHltVolume_l(40.0);
      vessels->setHltNotes("Real HLT");
      vessels->setLauterTunVolume_l(0.0);
      vessels->setLauterTunDeadspaceLoss_l(2.0);
      vessels->setLauterTunNotes("Leftover notes");
      vessels->setLauterTunType("Leftover type");
      ObjectStoreWrapper::insert(vessels);
      // Ditto packaging vessel.  (We check below that the export omits both, because their volumes are 0, even
      // though we haven't called Equipment::clearUnusedOptionalVessels.)
      vessels->setPackagingVesselVolume_l(0.0);
      vessels->setPackagingVesselLoss_l(1.0);

      // A Mash must have at least one step
      auto mash = std::make_shared<Mash>("Test Mash Without Steps");
      ObjectStoreWrapper::insert(mash);

      // A fermentable must have a yield
      auto noYield = std::make_shared<Fermentable>();
      noYield->setName("Test Fermentable Without Yield");
      noYield->setType(Fermentable::Type::Other_Adjunct);
      noYield->setColor_srm(1.0);
      ObjectStoreWrapper::insert(noYield);

      // An addition can have at most one of time, gravity and pH
      auto const recipe = ObjectStoreWrapper::findFirstMatching<Recipe>(
         [](std::shared_ptr<Recipe> rec) { return rec->name() == "Cascade Pale Ale"; }
      );
      QVERIFY(recipe);
      std::shared_ptr<RecipeAdditionHop> timedHopAddition;
      for (auto const & hopAddition : recipe->hopAdditions()) {
         if (hopAddition->addAtTime_mins()) {
            timedHopAddition = hopAddition;
            break;
         }
      }
      QVERIFY2(timedHopAddition, "No hop addition with a time");
      timedHopAddition->setAddAtGravity_sg(1.020);
   }
   //
   // Now export everything in the database, and check we can read it back in -- which includes validating it against
   // the schema.  Set DOTBEER_EXPORT_FILE to keep the exported file (eg to check it with DotBeer's validate.py).
   //
   QString exportFilePath = qEnvironmentVariable("DOTBEER_EXPORT_FILE");
   if (exportFilePath.isEmpty()) {
      exportFilePath = UnitTests::TestEnvironment::tempDir().filePath("dotBeerRoundTrip.beer");
   }
   {
      QFile outFile{exportFilePath};
      QVERIFY(outFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      DotBeer::Exporter exporter{outFile, userMessageAsStream};
      exporter.add("", UnitTests::allConst<Fermentable>());
      exporter.add("", UnitTests::allConst<Hop>());
      exporter.add("", UnitTests::allConst<Misc>());
      exporter.add("", UnitTests::allConst<Yeast>());
      exporter.add("", UnitTests::allConst<Style>());
      exporter.add("", UnitTests::allConst<Equipment>());
      exporter.add("", UnitTests::allConst<Water>());
      exporter.add("", UnitTests::allConst<Mash>());
      exporter.add("", UnitTests::allConst<Boil>());
      exporter.add("", UnitTests::allConst<Fermentation>());
      exporter.add("", UnitTests::allConst<Recipe>());
      exporter.close();
      outFile.close();
   }
   qInfo() << Q_FUNC_INFO << "Exported database to" << exportFilePath;

   // Check that percent acid was written for the "Other" water agent
   {
      QFile exportedFile{exportFilePath};
      QVERIFY(exportedFile.open(QIODevice::ReadOnly));
      boost::json::value const exported = boost::json::parse(exportedFile.readAll().toStdString());
      bool foundOtherAcid = false;
      for (auto const & misc : exported.at("DotBeer").at("miscs").as_array()) {
         if (misc.at("name").as_string() == "Test Other Acid") {
            foundOtherAcid = true;
            QVERIFY2(misc.as_object().contains("water_agent_percent_acid"), "Percent acid not written for Other water agent");
         }
      }
      QVERIFY2(foundOtherAcid, "Test Other Acid not exported");

      // Mash without steps should have been given one
      bool foundMash = false;
      for (auto const & mash : exported.at("DotBeer").at("mashes").as_array()) {
         if (mash.at("name").as_string() == "Test Mash Without Steps") {
            foundMash = true;
            QVERIFY2(mash.at("mash_steps").as_array().size() == 1, "Mash without steps not given one");
         }
      }
      QVERIFY2(foundMash, "Test Mash Without Steps not exported");

      // Fermentable without yield should have been given one
      bool foundNoYield = false;
      for (auto const & fermentable : exported.at("DotBeer").at("fermentables").as_array()) {
         if (fermentable.at("name").as_string() == "Test Fermentable Without Yield") {
            foundNoYield = true;
            QVERIFY2(fermentable.as_object().contains("yield_fine_grind"), "Fermentable without yield not given one");
         }
      }
      QVERIFY2(foundNoYield, "Test Fermentable Without Yield not exported");

      // No addition should have more than one of time, gravity and pH
      for (auto const & recipe : exported.at("DotBeer").at("recipes").as_array()) {
         for (auto const & [listName, additions] : recipe.at("ingredients").as_object()) {
            for (auto const & addition : additions.as_array()) {
               auto const & schedule = addition.at("schedule").as_object();
               int const count = (schedule.contains("time") ? 1 : 0) +
                                 (schedule.contains("specific_gravity") ? 1 : 0) +
                                 (schedule.contains("pH") ? 1 : 0);
               QVERIFY2(count <= 1, "Addition with more than one of time, gravity and pH");
            }
         }
      }

      // Equipment with only a kettle should not have other (zero-volume) vessels written
      bool foundMinimalKettle = false;
      for (auto const & equipment : exported.at("DotBeer").at("equipments").as_array()) {
         if (equipment.at("name").as_string() == "Minimal Kettle") {
            foundMinimalKettle = true;
            for (char const * const vessel : {"hlt", "mash_tun", "lauter_tun", "fermentation_vessel", "aging_vessel",
                                              "packaging_vessel"}) {
               QVERIFY2(!equipment.as_object().contains(vessel), vessel);
            }
            QVERIFY(equipment.as_object().contains("kettle"));
         }
      }
      QVERIFY2(foundMinimalKettle, "Minimal Kettle not exported");

      // Optional vessels with zero volume should not be written, even if they have other attributes set
      bool foundVessels = false;
      for (auto const & equipment : exported.at("DotBeer").at("equipments").as_array()) {
         if (equipment.at("name").as_string() == "Test Equipment With Unused Lauter Tun") {
            foundVessels = true;
            QVERIFY2(equipment.as_object().contains("hlt")              , "HLT should be written");
            QVERIFY2(!equipment.as_object().contains("lauter_tun")      , "Zero-volume lauter tun should not be written");
            QVERIFY2(!equipment.as_object().contains("packaging_vessel"), "Zero-volume packaging vessel should not be written");
         }
      }
      QVERIFY2(foundVessels, "Test Equipment With Unused Lauter Tun not exported");
   }
   QString message;
   bool const reimportOk = importDotBeerFile(exportFilePath, message);
   QVERIFY2(reimportOk, qPrintable(QString{"Could not re-import exported file %1: %2"}.arg(exportFilePath).arg(message)));
   return;
}

void SerializationTests::testClearUnusedOptionalVessels() {
   // Optional vessels with zero volume are treated as not present, so their other attributes should be cleared
   auto vessels = std::make_shared<Equipment>("Test Equipment For Clearing Unused Vessels");
   vessels->setKettleBoilSize_l(30.0);
   vessels->setHltVolume_l(40.0);
   vessels->setHltNotes("Real HLT");
   vessels->setLauterTunVolume_l(0.0);
   vessels->setLauterTunDeadspaceLoss_l(2.0);
   vessels->setLauterTunNotes("Leftover notes");
   vessels->setLauterTunType("Leftover type");
   vessels->setPackagingVesselVolume_l(0.0);
   vessels->setPackagingVesselLoss_l(1.0);
   ObjectStoreWrapper::insert(vessels);

   QVERIFY2(vessels->clearUnusedOptionalVessels(), "Should have cleared unused vessels");
   QVERIFY2(vessels->lauterTunDeadspaceLoss_l() == 0.0 &&
            vessels->lauterTunNotes().isEmpty() &&
            vessels->lauterTunType().isEmpty(), "Unused lauter tun not cleared");
   QVERIFY2(vessels->packagingVesselLoss_l() == 0.0, "Unused packaging vessel not cleared");
   QVERIFY2(vessels->hltVolume_l() == 40.0 && vessels->hltNotes() == "Real HLT", "HLT should not be cleared");
   QVERIFY2(!vessels->clearUnusedOptionalVessels(), "Nothing more to clear");
   return;
}

void SerializationTests::testBeerXmlNullYield() {
   // A fermentable must have a yield in BeerXML, but need not for us
   auto noYield = std::make_shared<Fermentable>();
   noYield->setName("BeerXML Test Fermentable Without Yield");
   noYield->setType(Fermentable::Type::Other_Adjunct);
   noYield->setColor_srm(1.0);
   ObjectStoreWrapper::insert(noYield);

   //
   // YIELD is required in BeerXML but fineGrindYield_pct is optional for us.  If it's not set, we should write YIELD
   // of 0, and, when reading, treat YIELD of 0 as not set.
   //
   QVERIFY(noYield && !noYield->fineGrindYield_pct());
   QString const beerXmlFilePath = UnitTests::TestEnvironment::tempDir().filePath("nullYield.xml");
   {
      QFile beerXmlFile{beerXmlFilePath};
      QVERIFY(beerXmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      BeerXML::getInstance().createXmlFile(beerXmlFile);
      // Munich Malt (from AmericanPaleAle.beer) has only a potential yield, which we write as an extension tag
      auto const munich = ObjectStoreWrapper::findFirstMatching<Fermentable>(
         [](std::shared_ptr<Fermentable> f) { return f->name() == "Munich Malt"; }
      );
      QVERIFY(munich && !munich->fineGrindYield_pct() && munich->potentialYield_sg());
      BeerXML::getInstance().toXml(QList<Fermentable const *>{noYield.get(), munich.get()}, beerXmlFile);
   }
   QString beerXml;
   {
      QFile beerXmlFile{beerXmlFilePath};
      QVERIFY(beerXmlFile.open(QIODevice::ReadOnly));
      beerXml = QString::fromUtf8(beerXmlFile.readAll());
   }
   QVERIFY2(beerXml.contains("<YIELD>0</YIELD>"), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<POTENTIAL_YIELD>1.035</POTENTIAL_YIELD>"), qPrintable(beerXml));
   // Rename so the re-read fermentables aren't treated as duplicates
   beerXml.replace("BeerXML Test Fermentable Without Yield", "BeerXML Test Fermentable Without Yield Reread");
   beerXml.replace("<NAME>Munich Malt</NAME>", "<NAME>Munich Malt From BeerXML</NAME>");
   {
      QFile beerXmlFile{beerXmlFilePath};
      QVERIFY(beerXmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      beerXmlFile.write(beerXml.toUtf8());
   }
   QString beerXmlMessage;
   QTextStream beerXmlMessageStream{&beerXmlMessage};
   QVERIFY2(BeerXML::getInstance().importFromXML("", beerXmlFilePath, beerXmlMessageStream),
            qPrintable(beerXmlMessage));
   auto const reread = ObjectStoreWrapper::findFirstMatching<Fermentable>(
      [](std::shared_ptr<Fermentable> f) { return f->name() == "BeerXML Test Fermentable Without Yield Reread"; }
   );
   QVERIFY2(reread, "Fermentable not read back from BeerXML");
   QVERIFY2(!reread->fineGrindYield_pct(), "YIELD of 0 should be read as not set");
   auto const rereadMunich = ObjectStoreWrapper::findFirstMatching<Fermentable>(
      [](std::shared_ptr<Fermentable> f) { return f->name() == "Munich Malt From BeerXML"; }
   );
   QVERIFY2(rereadMunich, "Munich Malt not read back from BeerXML");
   QVERIFY2(!rereadMunich->fineGrindYield_pct(), "YIELD of 0 should be read as not set");
   QVERIFY2(rereadMunich->potentialYield_sg() && fuzzyComp(*rereadMunich->potentialYield_sg(), 1.035, 0.0001),
            "Potential yield lost in BeerXML round trip");
   QVERIFY2(rereadMunich->effectiveFineGrindYield_pct() &&
            fuzzyComp(*rereadMunich->effectiveFineGrindYield_pct(), 76.1, 0.1),
            "Wrong effective yield after BeerXML round trip");
   return;
}

void SerializationTests::testNullSentinel() {
   //
   // Check that a null sentinel substituted on write survives the "remove optional wrapper" step that JsonRecord does
   // straight afterwards.  (QVariant cannot convert plain double to std::optional<double>, so, if the sentinel were
   // substituted without the wrapper, the value would be silently dropped.)
   //
   Serialization::NullSentinel const sentinel{0.0};
   QVariant unsetValue = QVariant::fromValue(std::optional<double>{});
   QVERIFY(sentinel.replaceNullWithSentinel<double>(unsetValue));
   QVERIFY(Optional::removeOptionalWrapperIfPresent<double>(unsetValue, true));
   QCOMPARE(unsetValue.toDouble(), 0.0);
   QVariant setValue = QVariant::fromValue(std::optional<double>{42.0});
   QVERIFY(!sentinel.replaceNullWithSentinel<double>(setValue));
   QVERIFY(Optional::removeOptionalWrapperIfPresent<double>(setValue, true));
   QCOMPARE(setValue.toDouble(), 42.0);
   return;
}

void SerializationTests::testBeerJsonRoundTrip() {
   QString exportFilePath = qEnvironmentVariable("BEERJSON_EXPORT_FILE");
   if (exportFilePath.isEmpty()) {
      exportFilePath = UnitTests::TestEnvironment::tempDir().filePath("beerJsonRoundTrip.json");
   }
   {
      QFile outFile{exportFilePath};
      QVERIFY(outFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      BeerJson::Exporter exporter{outFile, userMessageAsStream};
      exporter.add(UnitTests::allConst<Fermentable>());
      exporter.add(UnitTests::allConst<Hop>());
      exporter.add(UnitTests::allConst<Misc>());
      exporter.add(UnitTests::allConst<Yeast>());
      exporter.add(UnitTests::allConst<Style>());
      exporter.add(UnitTests::allConst<Equipment>());
      exporter.add(UnitTests::allConst<Water>());
      exporter.add(UnitTests::allConst<Mash>());
      exporter.add(UnitTests::allConst<Boil>());
      exporter.add(UnitTests::allConst<Fermentation>());
      exporter.add(UnitTests::allConst<Recipe>());
      exporter.close();
      outFile.close();
   }
   qInfo() << Q_FUNC_INFO << "Exported database to" << exportFilePath;
   QString message;
   QTextStream messageStream{&message};
   bool const reimportOk = BeerJson::import("", exportFilePath, messageStream);
   messageStream.flush();
   QVERIFY2(reimportOk, qPrintable(QString{"Could not re-import exported file %1: %2"}.arg(exportFilePath).arg(message)));
   return;
}