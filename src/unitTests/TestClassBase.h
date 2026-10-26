/*======================================================================================================================
 * unitTests/TestClassBase.h is part of Brewtarget, and is copyright the following authors 2026:
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
#ifndef UNITTESTS_TESTCLASSBASE_H
#define UNITTESTS_TESTCLASSBASE_H
#pragma once

#include <QObject>

namespace UnitTests {

   /**
    * \brief Base class for all our Qt Test classes.  (See comment in \c unitTests/TestRegistry.h.)
    *
    *        This provides the \c initTestCase and \c cleanupTestCase slots, so subclasses should NOT declare those
    *        slots, but should instead override \c initTestClass and/or \c cleanupTestClass if they need per-class
    *        set-up or tear-down.  (\c QVERIFY etc work in these functions in the same way as in a test function.)
    *
    *        The reason for this is that \c QTest::qExec installs its own Qt message handler, which would otherwise
    *        mean that output from \c qDebug(), \c qInfo() etc goes to the test output rather than to our log files (as
    *        it does when running the application).  We want it to go to the log files, both so that it's available
    *        in the same way when debugging a test failure, and because some tests (eg \c CoreTests::testLogRotation)
    *        depend on it.  So, in \c initTestCase, which runs inside \c QTest::qExec, we reinstate our logging message
    *        handler.
    */
   class TestClassBase : public QObject {
      Q_OBJECT

   public:
      TestClassBase();
      virtual ~TestClassBase();

   protected:
      //! \brief Override for any set-up needed before the test functions in a class are run.  Default does nothing.
      virtual void initTestClass();

      //! \brief Override for any tear-down needed after the test functions in a class are run.  Default does nothing
      virtual void cleanupTestClass();

   private slots:
      // Run once before all the test functions in the class
      void initTestCase();

      // Run once after all the test functions in the class
      void cleanupTestCase();
   };

}

#endif
