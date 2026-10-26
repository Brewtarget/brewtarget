/*======================================================================================================================
 * unitTests/TestMain.cpp is part of Brewtarget, and is copyright the following authors 2026:
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
//
// If you're building with CMake:
//   - Ensure each unit test has an "add_test" line in the main CMakeLists.txt
//   - Run unit tests with  make test
//   - Debug log output is in build/Testing/Temporary/LastTest.log (assuming "build" is your CMake build directory)
//
// If you're building with Meson:
//   - Ensure each unit test has a "test" line in meson.build
//   - Run unit tests with  meson test  (or, eg,  meson test --suite serialization  to run just one group of tests)
//   - Debug log output is in mbuild/meson-logs/testlog.txt (assuming "mbuild" is your Meson build directory)
//
// See comment in unitTests/TestRegistry.h for how the test runner is invoked and how to add new tests.
//
#include <exception>
#include <iostream>
#include <vector>

#include <boost/json/src.hpp> // Needs to be included exactly once in the code to use header-only version of Boost.JSON

#include <QApplication>
#include <QStringList>
#include <QtTest/QtTest>

#include "unitTests/TestEnvironment.h"
#include "unitTests/TestRegistry.h"

std::map<QString, UnitTests::TestClassFactory> & UnitTests::testClassRegistry() {
   static std::map<QString, TestClassFactory> registry;
   return registry;
}

UnitTests::TestClassRegistrar::TestClassRegistrar(char const * const className, TestClassFactory factory) {
   testClassRegistry().emplace(QString{className}, std::move(factory));
   return;
}

namespace {
   void listTestClasses(std::ostream & stream) {
      for (auto const & [className, factory] : UnitTests::testClassRegistry()) {
         stream << "   " << className.toStdString() << std::endl;
      }
      return;
   }
}

//
// This does the same as QTEST_MAIN would do for a single test class, except that we choose the test class(es) to run
// from the first command line argument, and we set up the test environment once for all of them.
//
int main(int argc, char * argv[]) {
   //
   // Work out which test class(es) to run.  If the first argument is the name of a test class, we run just that class,
   // and pass the remaining arguments to Qt Test.  Otherwise, we run all the test classes, passing all the arguments
   // to Qt Test.
   //
   // We do this before creating the QApplication so that, eg, --list-classes works without a display.
   //
   std::vector<QString> classesToRun;
   QStringList qtTestArgs;
   for (int ii = 0; ii < argc; ++ii) {
      qtTestArgs.append(QString::fromLocal8Bit(argv[ii]));
   }
   if (qtTestArgs.size() > 1) {
      QString const firstArg = qtTestArgs.at(1);
      if (firstArg == "--list-classes") {
         listTestClasses(std::cout);
         return 0;
      }
      if (UnitTests::testClassRegistry().contains(firstArg)) {
         classesToRun.push_back(firstArg);
         qtTestArgs.removeAt(1);
      } else if (!firstArg.startsWith('-')) {
         //
         // It's most likely a test function name.  We used to have all the test functions in a single class, so
         // invocations such as "brewken_tests testFooBar" used to work.  Now you need to say which class the
         // function is in.
         //
         std::cerr <<
            "Unrecognised test class \"" << firstArg.toStdString() << "\".  Usage:\n   " <<
            qtTestArgs.at(0).toStdString() << " [TestClassName [Qt Test options and/or test function names]]\n"
            "Test classes are:" << std::endl;
         listTestClasses(std::cerr);
         return 1;
      }
   }
   if (classesToRun.empty()) {
      for (auto const & [className, factory] : UnitTests::testClassRegistry()) {
         classesToRun.push_back(className);
      }
   }

   QApplication app(argc, argv);
   app.setAttribute(Qt::AA_Use96Dpi, true);
   QTest::setMainSourcePath(__FILE__);

   int result = 0;
   try {
      UnitTests::TestEnvironment testEnvironment;
      for (QString const & className : classesToRun) {
         std::unique_ptr<QObject> testObject = UnitTests::testClassRegistry().at(className)();
         // QTest::qExec returns the number of failed tests (capped at 127)
         result += QTest::qExec(testObject.get(), qtTestArgs);
      }
   } catch (std::exception const & e) {
      // When an exception gets to Qt, it will barf something along the lines of "Caught unhandled exception" without
      // leaving you much the wiser.  If we can intercept the exception along the way, we can ensure more details are
      // output to the console.
      std::cerr << "Caught exception: " << e.what() << std::endl;
      return 1;
   }

   return std::min(result, 127);
}
