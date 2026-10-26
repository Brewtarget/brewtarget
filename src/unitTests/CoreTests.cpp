/*======================================================================================================================
 * unitTests/CoreTests.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#include "unitTests/CoreTests.h"

#include <cstdint>
#include <vector>

#include <QDebug>
#include <QFile>
#include <QRandomGenerator>
#include <QString>
#include <QtTest/QtTest>

#include "Logging.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/NamedParameterBundle.h"
#include "unitTests/TestData.h"
#include "unitTests/TestRegistry.h"
#include "utils/BtStringConst.h"
#include "utils/MultiVector.h"

REGISTER_TEST_CLASS(CoreTests)

namespace {
   using UnitTests::fuzzyComp;

   // method to fill dummy logs with content to build size
   QString randomStringGenerator() {
      QString const posChars = "ABCDEFGHIJKLMNOPQRSTUVWXYabcdefghijklmnopqrstuvwwxyz";
      int constexpr randomcharLength = 64;

      QString randomString;
      for (int ii = 0; ii < randomcharLength; ii++) {

         int const index = static_cast<int>(QRandomGenerator().generate64()) % static_cast<int>(posChars.length());
         QChar const nChar = posChars.at(index);
         randomString.append(nChar);
      }
      return randomString;
   }

   // For testMultiVector
   std::vector<BtStringConst> fruit{"apples", "oranges", "bananas"};
   std::vector<BtStringConst> animals{"llamas", "alpacas", "wombats", "hares", "elephants"};
   std::vector<BtStringConst> shorty{"single entry"};
}

class CoreTests::impl {
public:
   impl() = default;
   ~impl() = default;
};

CoreTests::CoreTests() : UnitTests::TestClassBase(), pimpl{std::make_unique<impl>()} {
   return;
}

CoreTests::~CoreTests() = default;

void CoreTests::pstdintTest() {
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

void CoreTests::testNamedParameterBundle() {

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

   //
   // Test that we can store an int and get it back as a strongly-typed enum
   //
   npb.insert(PropertyNames::Hop::type, static_cast<int>(Hop::Type::AromaAndFlavor));
   Hop::Type retrievedHopType = npb.val<Hop::Type>(PropertyNames::Hop::type);
   QVERIFY2(retrievedHopType == Hop::Type::AromaAndFlavor, "Int -> Strongly-typed enum failed");

   //
   // Now test explicitly nullable fields
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

   return;
}

void CoreTests::testTypeLookups() {
   QVERIFY2(Hop::typeLookup.getType(PropertyNames::Hop::alpha_pct).typeIndex == TypeInfo::Index::Double,
            "PropertyNames::Hop::alpha_pct not a double");
   auto const & grainGroupTypeInfo = Fermentable::typeLookup.getType(PropertyNames::Fermentable::grainGroup);
   QVERIFY2(grainGroupTypeInfo.isOptional(),
            "PropertyNames::Fermentable::grainGroup not optional");
   QVERIFY2(grainGroupTypeInfo.classification == TypeInfo::Classification::OptionalEnum,
            "PropertyNames::Fermentable::grainGroup not optional enum");
   return;
}

void CoreTests::testMultiVector() {
   // Part of the motivation for creating MultiVector is to deal with structs containing BtStringConst, so it's a good
   // thing to test it with
   MultiVector<BtStringConst> const multiVector{
      {"one", "two", "three", "four"},
      // These are deliberately not in the same order as above!
      {&animals, &fruit, &shorty}
   };
   QVERIFY(multiVector[0] == BtStringConst{"one"});
   QVERIFY(multiVector[3] == BtStringConst{"four"});
   QVERIFY(multiVector[5] == BtStringConst{"alpacas"});
   QVERIFY(multiVector[9] == BtStringConst{"apples"});
   QVERIFY(multiVector[12] == BtStringConst{"single entry"});
   QVERIFY(multiVector.size() == 13);
   auto iterator = multiVector.begin();
   QVERIFY(*iterator == BtStringConst{"one"});
   ++iterator;
   QVERIFY(*iterator == BtStringConst{"two"});
   ++iterator;
   ++iterator;
   ++iterator;
   QVERIFY(*iterator == BtStringConst{"llamas"});
   iterator = multiVector.end();
   --iterator;
   QVERIFY(*iterator == BtStringConst{"single entry"});
   --iterator;
   QVERIFY(*iterator == BtStringConst{"bananas"});
   --iterator;
   QVERIFY(*iterator == BtStringConst{"oranges"});

   return;
}

void CoreTests::testLogRotation() {
   qDebug() << Q_FUNC_INFO << "Logging to" << Logging::getDirectory();

   // Turning off logging to stderr console, this is so you won't have to watch 100k rows generate in the console.
   Logging::setLoggingToStderr(false);

   // Generate 40 000 log rows giving roughly 10 files with dummy/random logs
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
   qDebug() << Q_FUNC_INFO << "Logging::getLogFileList() has" << fileList.size() << "entries";
   qDebug() << Q_FUNC_INFO << "Logging::logFileCount =" << Logging::logFileCount;
   // There is always a "logFileCount" number of old files + 1 current file
   QCOMPARE(fileList.size(), Logging::logFileCount + 1);

   qDebug() << Q_FUNC_INFO << "Logging::logFileSize =" << Logging::logFileSize;
   for (int i = 0; i < fileList.size(); i++) {
      QFile f(QString(fileList.at(i).canonicalFilePath()));
      qDebug() << Q_FUNC_INFO << "File" << f.fileName() << "has size" << f.size();
      //Here we test if the file is more than 10% bigger than the specified logFileSize", if so, fail.
      QVERIFY2(f.size() <= static_cast<int>(Logging::logFileSize * 1.1), "Wrong Sized file");
   }
   return;
}