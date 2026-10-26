/*======================================================================================================================
 * unitTests/TestEnvironment.h is part of Brewtarget, and is copyright the following authors 2009-2026:
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
#ifndef UNITTESTS_TESTENVIRONMENT_H
#define UNITTESTS_TESTENVIRONMENT_H
#pragma once

#include <QDir>
#include <QtGlobal>

namespace UnitTests {

   /**
    * \brief RAII class that sets up (and afterwards tears down) everything the unit tests need from the application:
    *        a unique temporary directory, separate persistent settings, logging, a (new) database, and so on.
    *
    *        Exactly one of these is created, in \c main() (see \c unitTests/TestMain.cpp), before any test class is
    *        run.  This is the expensive set-up that we only want to do once per test run.  Fixtures that are only
    *        needed by some tests should instead be created in \c initTestClass() of the relevant test class.
    *
    *        Note that, when running via Meson or CMake, each test is a separate run of the test runner, so each test
    *        gets its own temporary directory and a fresh database.
    */
   class TestEnvironment {
   public:
      /**
       * \brief Throws \c std::runtime_error if set-up fails
       */
      TestEnvironment();
      ~TestEnvironment();

      /**
       * \brief The unique temporary directory for this test run -- where we write database and log files etc.  It,
       *        and its contents, are removed when the \c TestEnvironment is destroyed.
       */
      static QDir const & tempDir();

      /**
       * \brief Install the application's logging message handler (ie the one \c Logging::initializeLogging installs)
       *        as the Qt message handler.  This needs to be called from inside \c QTest::qExec, because that installs
       *        its own message handler.  See \c UnitTests::TestClassBase.
       */
      static void useApplicationLogging();

   private:
      // No copying or moving
      TestEnvironment(TestEnvironment const &) = delete;
      TestEnvironment & operator=(TestEnvironment const &) = delete;
      TestEnvironment(TestEnvironment &&) = delete;
      TestEnvironment & operator=(TestEnvironment &&) = delete;
   };

}

#endif
