/*======================================================================================================================
 * unitTests/TestData.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "unitTests/TestData.h"

#include <cmath>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

#include "config.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "serialization/json/dotBeer/DotBeer.h"
#include "utils/ImportRecordCount.h"

bool UnitTests::fuzzyComp(double const a, double const b, double const tolerance) {
   bool ret = inRange(a, b - tolerance, b + tolerance);
   if (!ret) {
      qDebug() <<
         Q_FUNC_INFO << "a:" << a << ", b:" << b << ", diff:" << std::abs(a - b) << ", tolerance:" << tolerance;
   }
   return ret;
}

std::shared_ptr<Hop> UnitTests::makeHop_Cascade() {
   // Cascade pellets at 4% AA
   auto hop = std::make_shared<Hop>();
   ObjectStoreWrapper::insert(hop);
   hop->setName("Cascade 4pct");
   hop->setAlpha_pct(4.0);
   hop->setType(Hop::Type::AromaAndBittering);
   hop->setForm(Hop::Form::Pellet);
   return hop;
}

std::shared_ptr<Equipment> UnitTests::makeEquipment_FiveGalNoLoss() {
   auto equipment = std::make_shared<Equipment>();
   equipment->setName("5 gal No Loss");
   equipment->setKettleBoilSize_l(24.0);
   equipment->setFvBatchSize_l(20.0);
   equipment->setMashTunVolume_l(40.0);
   equipment->setTopUpWater_l(0);
   equipment->setKettleTrubChillerLoss_l(0);
   equipment->setKettleEvaporationPerHour_l(4.0);
   equipment->setMashTunLoss_l(0); // This is lautering deadspace loss as no separate lautering tun
   equipment->setTopUpKettle_l(0);
   equipment->setHopUtilization_pct(100);
   equipment->setMashTunGrainAbsorption_LKg(1.0);
   equipment->setBoilingPoint_c(100);
   return equipment;
}

std::shared_ptr<Fermentable> UnitTests::makeFermentable_TwoRow() {
   auto twoRow = std::make_shared<Fermentable>();
   twoRow->setName("Two Row");
   twoRow->setType(Fermentable::Type::Grain);
   twoRow->setFineGrindYield_pct(70.0);
   twoRow->setColor_srm(2.0);
   twoRow->setMoisture_pct(0);
   return twoRow;
}

namespace {
   bool importDotBeerFile(QString const & filePath, QString & message) {
      QString userMessage;
      QTextStream userMessageAsStream{&userMessage};
      bool const result = DotBeer::import("", filePath, userMessageAsStream);
      userMessageAsStream.flush();
      message = userMessage;
      return result;
   }
}

bool UnitTests::importDotBeerExamples(QString & errorMessage) {
   QDir const examplesDir{QDir{CONFIG_TEST_DATA_DIR}.filePath("dotBeer/examples")};
   // Make sure we fail, rather than silently test nothing, if the examples aren't where we think they are
   if (!examplesDir.exists()) {
      errorMessage = QString{"dotBeer examples directory %1 not found"}.arg(examplesDir.path());
      return false;
   }

   //
   // Every valid example should import
   //
   int numValidExamples = 0;
   for (QString const subDir : {"library", "recipes", "stock"}) {
      QDir const dir{examplesDir.filePath(subDir)};
      for (QFileInfo const & fileInfo : dir.entryInfoList({"*.beer"}, QDir::Files, QDir::Name)) {
         QString message;
         bool ok = importDotBeerFile(fileInfo.absoluteFilePath(), message);
         // A valid file with no records in it (eg Minimal.beer) is reported as a failed import, which is fine.
         if (!ok && message == ImportRecordCount::tr("Couldn't find any recognisable data in the file!")) {
            qInfo() << Q_FUNC_INFO << fileInfo.fileName() << "contains no records";
            ok = true;
         }
         qInfo() << Q_FUNC_INFO << "Import" << fileInfo.fileName() << (ok ? "succeeded" : "FAILED") << ":" << message;
         if (!ok) {
            errorMessage = QString{"Failed to import %1: %2"}.arg(fileInfo.fileName()).arg(message);
            return false;
         }
         ++numValidExamples;
      }
   }
   if (numValidExamples == 0) {
      errorMessage = "No valid dotBeer examples found";
      return false;
   }

   //
   // We don't require that we reject every invalid example (some rules are not ones we need to enforce to read a file
   // safely), but it's useful to log what we do.  Some tests rely on our having accepted (and corrected) some of the
   // invalid examples -- eg AdditionStepOutOfRange.beer.
   //
   int numInvalidExamples = 0;
   QDir const dir{examplesDir.filePath("invalid")};
   for (QFileInfo const & fileInfo : dir.entryInfoList({"*.beer"}, QDir::Files, QDir::Name)) {
      QString message;
      bool const ok = importDotBeerFile(fileInfo.absoluteFilePath(), message);
      qInfo() <<
         Q_FUNC_INFO << "Invalid example" << fileInfo.fileName() << (ok ? "was ACCEPTED" : "was rejected") << ":" <<
         message;
      ++numInvalidExamples;
   }
   if (numInvalidExamples == 0) {
      errorMessage = "No invalid dotBeer examples found";
      return false;
   }

   return true;
}