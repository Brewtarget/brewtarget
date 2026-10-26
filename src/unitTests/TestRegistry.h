/*======================================================================================================================
 * unitTests/TestRegistry.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UNITTESTS_TESTREGISTRY_H
#define UNITTESTS_TESTREGISTRY_H
#pragma once

#include <functional>
#include <map>
#include <memory>

#include <QObject>
#include <QString>

/**
 * \brief The unit tests are split across a number of Qt Test classes (\c CoreTests, \c MeasurementTests, etc), each
 *        in its own .h/.cpp files in \c src/unitTests.  They are all built into a single test runner executable, whose
 *        \c main() (in \c unitTests/TestMain.cpp) sets up the common test environment (see \c TestEnvironment) and
 *        then runs one or all of the test classes.  Each test class registers itself with the runner by putting
 *        \c REGISTER_TEST_CLASS(ClassName) in its .cpp file.
 *
 *        To add a new test function to an existing test class:
 *           - Add a new private slot to the class -- eg \c void \c testFooBar()
 *           - Add a corresponding \c test() line in \c meson.build
 *           - Add a corresponding \c add_test line in \c CMakeLists.txt
 *
 *        To add a new test class, additionally:
 *           - Create \c src/unitTests/FooTests.h and \c src/unitTests/FooTests.cpp, following the pattern of the
 *             existing test classes: inherit from \c UnitTests::TestClassBase (and, if you need per-class set-up
 *             or tear-down, override \c initTestClass and/or \c cleanupTestClass rather than declaring
 *             \c initTestCase or \c cleanupTestCase slots), and put \c REGISTER_TEST_CLASS(FooTests) in the .cpp
 *             file
 *           - Add the .cpp file to \c unitTestExtraSourceFiles in \c meson.build and to \c unitTestFiles_cpp in
 *             \c src/CMakeLists.txt
 *           - Add the .h file to \c unitTestMocHeaders in \c meson.build
 *
 *        The test runner is invoked as:
 *
 *           brewken_tests [TestClassName [Qt Test options and/or test function names]]
 *
 *        Eg \c "brewken_tests SerializationTests testBeerJsonRoundTrip" runs one test function (plus the class's
 *        \c initTestCase and \c cleanupTestCase), \c "brewken_tests SerializationTests" runs all the functions in that
 *        class, and \c "brewken_tests" runs all the test classes.  \c "brewken_tests --list-classes" lists the test
 *        classes, and \c "brewken_tests SerializationTests -functions" lists the functions in a class.
 */
namespace UnitTests {

   using TestClassFactory = std::function<std::unique_ptr<QObject>()>;

   /**
    * \brief The registry of test classes, sorted by name.  (We use a function-local static to avoid the "static
    *        initialisation order fiasco", since the registry is populated during static initialisation.)
    */
   std::map<QString, TestClassFactory> & testClassRegistry();

   /**
    * \brief Constructing one of these registers a test class.  Use via \c REGISTER_TEST_CLASS.
    */
   struct TestClassRegistrar {
      TestClassRegistrar(char const * const className, TestClassFactory factory);
   };

}

/**
 * \brief Put this in the .cpp file of each test class.
 *
 *        (This relies on the test class .cpp files being linked directly into the test runner, rather than via a
 *        static library, otherwise the linker might drop the registration because nothing references it.)
 */
#define REGISTER_TEST_CLASS(ClassName) \
   namespace { \
      UnitTests::TestClassRegistrar const testClassRegistrar_##ClassName{ \
         #ClassName, \
         [](){ return std::unique_ptr<QObject>{std::make_unique<ClassName>()}; } \
      }; \
   }

#endif
