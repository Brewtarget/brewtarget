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
#include <QRegularExpression>
#include <QTextStream>
#include <QtTest/QtTest>

#include <boost/json.hpp>

#include "config.h"
#include "database/DefaultContentLoader.h"
#include "database/ObjectStoreWrapper.h"
#include "model/Boil.h"
#include "model/BrewLog.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Fermentation.h"
#include "model/Folder.h"
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
      BeerXML::getInstance().toXml("", QList<Fermentable const *>{noYield.get(), munich.get()}, beerXmlFile);
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
namespace {
   //! \brief Write \c contents to a file in the test temporary directory and return its path
   QString writeTempFile(QString const & fileName, QString const & contents) {
      QString const filePath = UnitTests::TestEnvironment::tempDir().filePath(fileName);
      QFile file{filePath};
      if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
         file.write(contents.toUtf8());
      }
      return filePath;
   }

   QString readFile(QString const & filePath) {
      QFile file{filePath};
      if (!file.open(QIODevice::ReadOnly)) {
         return "";
      }
      return QString::fromUtf8(file.readAll());
   }

   std::shared_ptr<Equipment> equipmentNamed(QString const & name) {
      return ObjectStoreWrapper::findFirstMatching<Equipment>(
         [&name](std::shared_ptr<Equipment> e) { return e->name() == name; }
      );
   }
}

void SerializationTests::testAllInOneEquipmentImport() {
   // AllInOneBrewingSystems.beer was imported (into the root folder) in initTestClass
   auto const g30 = equipmentNamed("Grainfather G30 (v3)");
   QVERIFY2(g30, "Grainfather G30 not imported from AllInOneBrewingSystems.beer");
   QVERIFY(fuzzyComp(g30->foundationVolume_l()         , 3.5, 0.0001));
   QVERIFY(g30->grainAbsorption_LKg() && fuzzyComp(*g30->grainAbsorption_LKg(), 0.8, 0.0001));
   // There's no mash tun in the file, so the mash happens in the kettle
   QVERIFY(fuzzyComp(g30->mashTunVolume_l()            ,  0.0, 0.0001));
   QVERIFY(fuzzyComp(g30->mashTunLoss_l()              ,  0.0, 0.0001));
   QVERIFY(g30->mashesInKettle());
   QVERIFY(fuzzyComp(g30->kettleVolume_l()             , 30.0, 0.0001));
   QVERIFY(fuzzyComp(g30->mashVesselVolume_l()         , 30.0, 0.0001));
   QVERIFY(g30->mashVesselHeatCapacity_kcalC() && fuzzyComp(*g30->mashVesselHeatCapacity_kcalC(), 0.0, 0.0001));
   QVERIFY(fuzzyComp(g30->strikeWaterAllowance_l()     ,  3.5, 0.0001));
   QVERIFY(fuzzyComp(g30->getLauteringDeadspaceLoss_l(),  0.0, 0.0001));
   // Default batch size is in the file; default pre-boil volume is calculated from it plus kettle loss (2 l) plus an
   // hour's boil-off (3 l/h)
   QVERIFY(fuzzyComp(g30->fvBatchSize_l()              , 25.0, 0.0001));
   QVERIFY(fuzzyComp(g30->kettleBoilSize_l()           , 30.0, 0.0001));
   // There's no fermentation vessel in the file, so we use the batch size as the best guess for its capacity
   QVERIFY(fuzzyComp(g30->fvVolume_l()                 , 25.0, 0.0001));
   // NB: Folder paths we get back from objects don't have a leading slash
   QCOMPARE(g30->containedInFolderPath(), QString{"All-in-one systems/Grainfather"});

   auto const g70 = equipmentNamed("Grainfather G70 (v2)");
   QVERIFY(g70 && fuzzyComp(g70->foundationVolume_l(), 6.8, 0.0001));
   // Default batch size is capped below the manufacturer's 58 L so that the pre-boil volume fits in the 60 L kettle
   QVERIFY(fuzzyComp(g70->fvBatchSize_l(), 52.5, 0.0001));

   //
   // For every profile in the file, the default pre-boil volume we calculate (for a 60 minute boil) should fit in the
   // kettle.  (DotBeer's validate.py makes the same check.)
   //
   int numAllInOne = 0;
   for (Equipment const * equipment : ObjectStoreWrapper::getAllRaw<Equipment>()) {
      if (!equipment->containedInFolderPath().startsWith("All-in-one systems")) {
         continue;
      }
      ++numAllInOne;
      QVERIFY2(equipment->kettleBoilSize_l() <= equipment->kettleVolume_l() + 0.0001,
               qPrintable(QString("%1: default pre-boil %2 L is more than kettle volume %3 L").arg(
                  equipment->name()).arg(equipment->kettleBoilSize_l()).arg(equipment->kettleVolume_l())));
   }
   QVERIFY(numAllInOne >= 27);

   // Explicitly 0 in the file
   auto const g40 = equipmentNamed("Grainfather G40");
   QVERIFY(g40 && fuzzyComp(g40->foundationVolume_l(), 0.0, 0.0001));

   // Not in the file
   auto const brewMonk = equipmentNamed("Brew Monk B40");
   QVERIFY(brewMonk && fuzzyComp(brewMonk->foundationVolume_l(), 0.0, 0.0001));

   // US units should be converted
   auto const anvil = equipmentNamed("Anvil Foundry 18 gal");
   QVERIFY(anvil && fuzzyComp(anvil->kettleVolume_l(), 18.0 * 3.785411784, 0.0001));
   return;
}

void SerializationTests::testBeerXmlFoundationVolume() {
   auto equipment = std::make_shared<Equipment>("BeerXML Foundation Volume Test");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);
   equipment->setMashTunVolume_l(30.0);
   equipment->setFoundationVolume_l(3.5);
   equipment->setMashTunLoss_l(0.25);
   ObjectStoreWrapper::insert(equipment);

   QString const filePath = UnitTests::TestEnvironment::tempDir().filePath("foundationVolume.xml");
   {
      QFile beerXmlFile{filePath};
      QVERIFY(beerXmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      BeerXML::getInstance().createXmlFile(beerXmlFile);
      BeerXML::getInstance().toXml("", QList<Equipment const *>{equipment.get()}, beerXmlFile);
   }
   QString beerXml = readFile(filePath);
   QVERIFY2(beerXml.contains("<FOUNDATION_VOLUME_L>3.5</FOUNDATION_VOLUME_L>"), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<MASH_TUN_LOSS_L>0.25</MASH_TUN_LOSS_L>"), qPrintable(beerXml));
   // Equipment is in the root folder, so we shouldn't write a folder path
   QVERIFY2(!beerXml.contains("FOLDER_PATH"), qPrintable(beerXml));

   // Rename so the re-read equipment isn't treated as a duplicate
   beerXml.replace("BeerXML Foundation Volume Test", "BeerXML Foundation Volume Test Reread");
   writeTempFile("foundationVolume.xml", beerXml);
   QString message;
   QTextStream messageStream{&message};
   QVERIFY2(BeerXML::getInstance().importFromXML("", filePath, messageStream), qPrintable(message));
   auto const reread = equipmentNamed("BeerXML Foundation Volume Test Reread");
   QVERIFY2(reread, "Equipment not read back from BeerXML");
   QVERIFY(fuzzyComp(reread->foundationVolume_l(), 3.5 , 0.0001));
   QVERIFY(fuzzyComp(reread->mashTunLoss_l()            , 0.25, 0.0001));
   return;
}

void SerializationTests::testFolderPaths() {
   //
   // FolderCommon::subPath should work on path, not string, prefixes
   //
   QCOMPARE(FolderCommon::subPath("/foo/bar" , "/foo/bar/hum/bug"), QString{"/hum/bug"   });
   QCOMPARE(FolderCommon::subPath("/foo/bar/", "/foo/bar/hum/bug"), QString{"/hum/bug"   });
   QCOMPARE(FolderCommon::subPath("/foo/bar" , "/foo/bar"        ), QString{""           });
   QCOMPARE(FolderCommon::subPath("/foo/bar" , "/foo/barbie"     ), QString{"/foo/barbie"});
   QCOMPARE(FolderCommon::subPath("/"        , "/foo/bar"        ), QString{"/foo/bar"   });
   // Leading slashes (or lack of them) shouldn't matter
   QCOMPARE(FolderCommon::subPath("foo/bar"  , "/foo/bar/hum/bug"), QString{"/hum/bug"   });
   QCOMPARE(FolderCommon::subPath("/foo/bar" , "foo/bar/hum/bug" ), QString{"/hum/bug"   });
   QCOMPARE(FolderCommon::subPath("/foo/bar" , "foo/barbie"      ), QString{"foo/barbie" });
   QCOMPARE(FolderCommon::subPath(""         , "/foo/bar"        ), QString{"/foo/bar"   });

   //
   // dotBeer: a folder_path in the file is relative to the folder we are importing into.  (Previously, when importing
   // into anything other than the root folder, the folder_path in the file was lost.)
   //
   QString const dotBeerFilePath = writeTempFile("folderPaths.beer", R"({
      "DotBeer": {
         "version": "1.0.0",
         "output_by": "Brewken unit tests",
         "timestamp": "2026-10-26T12:00:00Z",
         "equipments": [
            {
               "name": "dotBeer Folder Path Test",
               "local_id": "equip_folder_test",
               "folder_path": "Kit/All-in-one",
               "foundation_volume": { "unit": "l", "value": 3.5 },
               "kettle": {
                  "max_volume": { "unit": "l", "value": 30 },
                  "loss": { "unit": "l", "value": 2 }
               }
            }
         ]
      }
   })");
   {
      QString message;
      QTextStream messageStream{&message};
      QVERIFY2(DotBeer::import("/Imported dotBeer", dotBeerFilePath, messageStream), qPrintable(message));
   }
   auto const fromDotBeer = equipmentNamed("dotBeer Folder Path Test");
   QVERIFY2(fromDotBeer, "Equipment not imported from dotBeer");
   QCOMPARE(fromDotBeer->containedInFolderPath(), QString{"Imported dotBeer/Kit/All-in-one"});
   QVERIFY(fuzzyComp(fromDotBeer->foundationVolume_l(), 3.5, 0.0001));

   //
   // BeerXML: we write folder paths relative to the folder being exported...
   //
   auto equipment = std::make_shared<Equipment>("BeerXML Folder Path Test Equipment");
   equipment->setKettleBoilSize_l(30.0);
   equipment->setFvBatchSize_l(23.0);
   ObjectStoreWrapper::insert(equipment);
   equipment->setContainedInFolderPath("/Export Base/Kit");
   QCOMPARE(equipment->containedInFolderPath(), QString{"Export Base/Kit"});

   auto fermentable = std::make_shared<Fermentable>("BeerXML Folder Path Test Fermentable");
   fermentable->setType(Fermentable::Type::Grain);
   fermentable->setColor_srm(3.0);
   fermentable->setFineGrindYield_pct(80.0);
   ObjectStoreWrapper::insert(fermentable);
   fermentable->setContainedInFolderPath("/Export Base/Malts/Base Malts");

   // An object that is in the folder being exported has no folder path written
   auto fermentableInBase = std::make_shared<Fermentable>("BeerXML Folder Path Test Fermentable In Base");
   fermentableInBase->setType(Fermentable::Type::Grain);
   fermentableInBase->setColor_srm(3.0);
   fermentableInBase->setFineGrindYield_pct(80.0);
   ObjectStoreWrapper::insert(fermentableInBase);
   fermentableInBase->setContainedInFolderPath("/Export Base");

   QString const beerXmlFilePath = UnitTests::TestEnvironment::tempDir().filePath("folderPaths.xml");
   {
      QFile beerXmlFile{beerXmlFilePath};
      QVERIFY(beerXmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      BeerXML::getInstance().createXmlFile(beerXmlFile);
      BeerXML::getInstance().toXml("/Export Base", QList<Fermentable const *>{fermentable.get(), fermentableInBase.get()}, beerXmlFile);
      BeerXML::getInstance().toXml("/Export Base", QList<Equipment   const *>{equipment.get()  }, beerXmlFile);
   }
   QString beerXml = readFile(beerXmlFilePath);
   QVERIFY2(beerXml.contains("<FOLDER_PATH>/Kit</FOLDER_PATH>"            ), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<FOLDER_PATH>/Malts/Base Malts</FOLDER_PATH>"), qPrintable(beerXml));
   QCOMPARE(beerXml.count("<FOLDER_PATH>"), 2);

   //
   // ...and read them relative to the folder being imported into
   //
   beerXml.replace("BeerXML Folder Path Test", "BeerXML Folder Path Reread");
   writeTempFile("folderPaths.xml", beerXml);
   {
      QString message;
      QTextStream messageStream{&message};
      QVERIFY2(BeerXML::getInstance().importFromXML("/Imported BeerXML", beerXmlFilePath, messageStream),
               qPrintable(message));
   }
   auto const rereadEquipment = equipmentNamed("BeerXML Folder Path Reread Equipment");
   QVERIFY2(rereadEquipment, "Equipment not read back from BeerXML");
   QCOMPARE(rereadEquipment->containedInFolderPath(), QString{"Imported BeerXML/Kit"});
   auto const rereadFermentable = ObjectStoreWrapper::findFirstMatching<Fermentable>(
      [](std::shared_ptr<Fermentable> f) { return f->name() == "BeerXML Folder Path Reread Fermentable"; }
   );
   QVERIFY2(rereadFermentable, "Fermentable not read back from BeerXML");
   QCOMPARE(rereadFermentable->containedInFolderPath(), QString{"Imported BeerXML/Malts/Base Malts"});
   auto const rereadFermentableInBase = ObjectStoreWrapper::findFirstMatching<Fermentable>(
      [](std::shared_ptr<Fermentable> f) { return f->name() == "BeerXML Folder Path Reread Fermentable In Base"; }
   );
   QVERIFY2(rereadFermentableInBase, "Fermentable not read back from BeerXML");
   QCOMPARE(rereadFermentableInBase->containedInFolderPath(), QString{"Imported BeerXML"});
   return;
}

void SerializationTests::testEquipmentComparison() {
   auto const original = std::make_shared<Equipment>("Equipment Comparison Test");
   original->setMashTunVolume_l(30.0);
   auto const same = std::make_shared<Equipment>(*original);
   QVERIFY(*original == *same);

   auto const differentLoss = std::make_shared<Equipment>(*original);
   differentLoss->setMashTunLoss_l(1.0);
   QVERIFY2(!(*original == *differentLoss), "Mash tun loss ignored when comparing equipment");

   auto const differentFoundation = std::make_shared<Equipment>(*original);
   differentFoundation->setFoundationVolume_l(3.5);
   QVERIFY2(!(*original == *differentFoundation), "Foundation volume ignored when comparing equipment");

   auto const differentLauterTun = std::make_shared<Equipment>(*original);
   differentLauterTun->setLauterTunVolume_l(40.0);
   QVERIFY2(!(*original == *differentLauterTun), "Lauter tun volume ignored when comparing equipment");
   return;
}

void SerializationTests::testEquipmentVolumes() {
   //
   // dotBeer: max_volume is the capacity of a vessel; default_batch_size is the default batch size; there is no
   // default pre-boil volume, so we calculate one.  No mash tun means the mash happens in the kettle.
   //
   QString const dotBeerFilePath = writeTempFile("equipmentVolumes.beer", R"({
      "DotBeer": {
         "version": "1.0.0",
         "output_by": "Brewken unit tests",
         "timestamp": "2026-10-27T12:00:00Z",
         "equipments": [
            {
               "name": "dotBeer Volumes Test",
               "local_id": "equip_volumes_test",
               "default_batch_size": { "unit": "l", "value": 23 },
               "grain_absorption_rate": { "unit": "l/kg", "value": 0.8 },
               "foundation_volume": { "unit": "l", "value": 3.5 },
               "kettle": {
                  "max_volume": { "unit": "l", "value": 35 },
                  "loss": { "unit": "l", "value": 2 },
                  "boil_rate_per_hour": { "unit": "l", "value": 4 }
               },
               "fermentation_vessel": {
                  "max_volume": { "unit": "l", "value": 30 },
                  "loss": { "unit": "l", "value": 1 }
               }
            },
            {
               "name": "dotBeer Volumes Test No Batch Size",
               "local_id": "equip_volumes_test_no_batch_size",
               "mash_tun": {
                  "max_volume": { "unit": "l", "value": 40 },
                  "loss": { "unit": "l", "value": 1.5 }
               },
               "kettle": {
                  "max_volume": { "unit": "l", "value": 50 },
                  "loss": { "unit": "l", "value": 2 },
                  "boil_rate_per_hour": { "unit": "l", "value": 4 }
               },
               "fermentation_vessel": {
                  "max_volume": { "unit": "l", "value": 30 },
                  "loss": { "unit": "l", "value": 1 }
               }
            }
         ]
      }
   })");
   {
      QString message;
      QTextStream messageStream{&message};
      QVERIFY2(DotBeer::import("", dotBeerFilePath, messageStream), qPrintable(message));
   }
   auto const allInOne = equipmentNamed("dotBeer Volumes Test");
   QVERIFY2(allInOne, "Equipment not imported from dotBeer");
   QVERIFY(fuzzyComp(allInOne->kettleVolume_l()    , 35.0, 0.0001));
   QVERIFY(fuzzyComp(allInOne->fvVolume_l()        , 30.0, 0.0001));
   QVERIFY(fuzzyComp(allInOne->fvBatchSize_l()     , 23.0, 0.0001));
   // 23 l batch + 2 l kettle loss + 4 l boil-off in an hour
   QVERIFY(fuzzyComp(allInOne->kettleBoilSize_l()  , 29.0, 0.0001));
   QVERIFY(allInOne->mashesInKettle());
   QVERIFY(fuzzyComp(allInOne->mashVesselVolume_l(), 35.0, 0.0001));
   QVERIFY(fuzzyComp(allInOne->foundationVolume_l(),  3.5, 0.0001));
   QVERIFY(allInOne->grainAbsorption_LKg() && fuzzyComp(*allInOne->grainAbsorption_LKg(), 0.8, 0.0001));

   auto const noBatchSize = equipmentNamed("dotBeer Volumes Test No Batch Size");
   QVERIFY2(noBatchSize, "Equipment not imported from dotBeer");
   QVERIFY(!noBatchSize->mashesInKettle());
   QVERIFY(fuzzyComp(noBatchSize->mashVesselVolume_l(), 40.0, 0.0001));
   // With no default_batch_size, we fall back to the fermentation vessel capacity (which is what we used to do)
   QVERIFY(fuzzyComp(noBatchSize->fvBatchSize_l()     , 30.0, 0.0001));
   QVERIFY(fuzzyComp(noBatchSize->kettleBoilSize_l()  , 36.0, 0.0001));

   //
   // dotBeer export: no mash tun written when there isn't one; top-level grain absorption and foundation volume;
   // capacities as max_volume; default batch size but not default pre-boil volume.
   //
   QString const exportFilePath = UnitTests::TestEnvironment::tempDir().filePath("equipmentVolumesExport.beer");
   {
      QFile outFile{exportFilePath};
      QVERIFY(outFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      DotBeer::Exporter exporter{outFile, userMessageAsStream};
      exporter.add("", QList<Equipment const *>{allInOne.get(), noBatchSize.get()});
      exporter.close();
      outFile.close();
   }
   {
      boost::json::value const exported = boost::json::parse(readFile(exportFilePath).toStdString());
      auto const & equipments = exported.at("DotBeer").at("equipments").as_array();
      QCOMPARE(equipments.size(), std::size_t{2});
      for (auto const & equipmentValue : equipments) {
         auto const & equipment = equipmentValue.as_object();
         auto const valueOf = [](boost::json::value const & measurement) {
            return measurement.at("value").to_number<double>();
         };
         if (equipment.at("name").as_string() == "dotBeer Volumes Test") {
            QVERIFY2(!equipment.contains("mash_tun"), "Mash tun written when mash happens in kettle");
            QVERIFY(fuzzyComp(valueOf(equipment.at("foundation_volume"    )),  3.5, 0.0001));
            QVERIFY(fuzzyComp(valueOf(equipment.at("grain_absorption_rate")),  0.8, 0.0001));
            QVERIFY(fuzzyComp(valueOf(equipment.at("default_batch_size"   )), 23.0, 0.0001));
            QVERIFY(fuzzyComp(valueOf(equipment.at("kettle"             ).at("max_volume")), 35.0, 0.0001));
            QVERIFY(fuzzyComp(valueOf(equipment.at("fermentation_vessel").at("max_volume")), 30.0, 0.0001));
         } else {
            QVERIFY2(equipment.contains("mash_tun"), "Mash tun not written");
            QVERIFY(fuzzyComp(valueOf(equipment.at("mash_tun").at("max_volume")), 40.0, 0.0001));
         }
      }
   }

   //
   // BeerXML: BOIL_SIZE and BATCH_SIZE are the defaults; capacities are extension tags...
   //
   QString const beerXmlFilePath = UnitTests::TestEnvironment::tempDir().filePath("equipmentVolumes.xml");
   {
      QFile beerXmlFile{beerXmlFilePath};
      QVERIFY(beerXmlFile.open(QIODevice::WriteOnly | QIODevice::Truncate));
      BeerXML::getInstance().createXmlFile(beerXmlFile);
      BeerXML::getInstance().toXml("", QList<Equipment const *>{allInOne.get()}, beerXmlFile);
   }
   QString beerXml = readFile(beerXmlFilePath);
   QVERIFY2(beerXml.contains("<BOIL_SIZE>29</BOIL_SIZE>"                  ), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<BATCH_SIZE>23</BATCH_SIZE>"                ), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<KETTLE_VOLUME_L>35</KETTLE_VOLUME_L>"      ), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<FERMENTER_VOLUME_L>30</FERMENTER_VOLUME_L>"), qPrintable(beerXml));
   QVERIFY2(beerXml.contains("<TUN_VOLUME>0</TUN_VOLUME>"                 ), qPrintable(beerXml));
   {
      QString reread = beerXml;
      reread.replace("dotBeer Volumes Test", "BeerXML Volumes Reread");
      writeTempFile("equipmentVolumes.xml", reread);
      QString message;
      QTextStream messageStream{&message};
      QVERIFY2(BeerXML::getInstance().importFromXML("", beerXmlFilePath, messageStream), qPrintable(message));
      auto const fromBeerXml = equipmentNamed("BeerXML Volumes Reread");
      QVERIFY2(fromBeerXml, "Equipment not read back from BeerXML");
      QVERIFY(fuzzyComp(fromBeerXml->kettleVolume_l()  , 35.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->fvVolume_l()      , 30.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->fvBatchSize_l()   , 23.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->kettleBoilSize_l(), 29.0, 0.0001));
      QVERIFY(fromBeerXml->mashesInKettle());
   }
   //
   // ...so, for a BeerXML file from another program, which won't have the extension tags, we use the default volumes
   // as the best guess for the capacities.
   //
   {
      QString reread = beerXml;
      reread.replace("dotBeer Volumes Test", "BeerXML Volumes Without Extension Tags");
      reread.remove(QRegularExpression{"<KETTLE_VOLUME_L>[^<]*</KETTLE_VOLUME_L>"});
      reread.remove(QRegularExpression{"<FERMENTER_VOLUME_L>[^<]*</FERMENTER_VOLUME_L>"});
      writeTempFile("equipmentVolumes.xml", reread);
      QString message;
      QTextStream messageStream{&message};
      QVERIFY2(BeerXML::getInstance().importFromXML("", beerXmlFilePath, messageStream), qPrintable(message));
      auto const fromBeerXml = equipmentNamed("BeerXML Volumes Without Extension Tags");
      QVERIFY2(fromBeerXml, "Equipment not read back from BeerXML");
      QVERIFY(fuzzyComp(fromBeerXml->kettleVolume_l()  , 29.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->fvVolume_l()      , 23.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->fvBatchSize_l()   , 23.0, 0.0001));
      QVERIFY(fuzzyComp(fromBeerXml->kettleBoilSize_l(), 29.0, 0.0001));
   }
   return;
}

void SerializationTests::testClearUnusedMashTun() {
   //
   // The mash tun is now an "optional" vessel, like the HLT and lauter tun.  If it has no volume, its loss etc are
   // cleared, but grain absorption and foundation volume (which apply to whichever vessel the mash happens in) are not.
   //
   auto equipment = std::make_shared<Equipment>("Test Equipment With Unused Mash Tun");
   equipment->setKettleVolume_l(30.0);
   equipment->setMashTunVolume_l(0.0);
   equipment->setMashTunLoss_l(1.0);
   equipment->setMashTunWeight_kg(5.0);
   equipment->setMashTunSpecificHeat_calGC(0.12);
   equipment->setMashTunNotes("Leftover notes");
   equipment->setFoundationVolume_l(3.5);
   equipment->setGrainAbsorption_LKg(0.8);
   //
   // Even before we clear them, the mash tun's loss and heat capacity should be ignored in calculations, because, with
   // no mash tun, the mash happens in the kettle.  (This is the case for equipment from before we treated a zero mash
   // tun volume this way, as we don't zero such fields in the DB migration.)
   //
   QVERIFY(equipment->mashesInKettle());
   QVERIFY(fuzzyComp(equipment->strikeWaterAllowance_l()     , 3.5, 0.0001));
   QVERIFY(fuzzyComp(equipment->getLauteringDeadspaceLoss_l(), 0.0, 0.0001));
   QVERIFY(equipment->mashVesselHeatCapacity_kcalC() && fuzzyComp(*equipment->mashVesselHeatCapacity_kcalC(), 0.0, 0.0001));
   QVERIFY(equipment->clearUnusedOptionalVessels());
   QVERIFY(fuzzyComp(equipment->mashTunLoss_l(), 0.0, 0.0001));
   QVERIFY(!equipment->mashTunWeight_kg());
   QVERIFY(!equipment->mashTunSpecificHeat_calGC());
   QVERIFY(equipment->mashTunNotes().isEmpty());
   QVERIFY(fuzzyComp(equipment->foundationVolume_l(), 3.5, 0.0001));
   QVERIFY(equipment->grainAbsorption_LKg() && fuzzyComp(*equipment->grainAbsorption_LKg(), 0.8, 0.0001));
   QVERIFY(fuzzyComp(equipment->strikeWaterAllowance_l(), 3.5, 0.0001));
   return;
}