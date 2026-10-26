/*======================================================================================================================
 * unitTests/TestEnvironment.cpp is part of Brewtarget, and is copyright the following authors 2009-2026:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Mike Evans <mikee@saxicola.co.uk>
 *   • Mik Firestone <mikfire@gmail.com>
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
#include "unitTests/TestEnvironment.h"

#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

#include <libxml2/libxml/parser.h>

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QRandomGenerator>
#include <QSettings>
#include <QTextStream>
#include <QThread>

#include "Application.h"
#include "config.h"
#include "Logging.h"
#include "PersistentSettings.h"
#include "utils/MetaTypes.h"

namespace {
   /**
    * \brief Where we write database and log files etc
    *
    *        I have tried both std::filesystem::path etc and QDir etc for this.  In several cases, the former offered
    *        better diagnostics on Linux & Mac when things go wrong - eg you can get the system error code and it's easy
    *        to look at file permissions etc.  However, trying to get it working on Windows was surprisingly painful
    *        (even when going via UTF8, which should handle all the character set conversions).  So, for now at least,
    *        we stick with QDir, as it mostly "just works" on all three platforms.
    */
   std::optional<QDir> s_tempDir;

   //! \brief The Qt message handler installed by Logging::initializeLogging
   QtMessageHandler s_logMessageHandler = nullptr;
}

void UnitTests::TestEnvironment::useApplicationLogging() {
   // It's a coding error to call this when there is no TestEnvironment
   Q_ASSERT(s_logMessageHandler);
   qInstallMessageHandler(s_logMessageHandler);
   return;
}

QDir const & UnitTests::TestEnvironment::tempDir() {
   // It's a coding error to call this when there is no TestEnvironment
   Q_ASSERT(s_tempDir);
   return *s_tempDir;
}

UnitTests::TestEnvironment::TestEnvironment() {
   // It's a coding error to create more than one TestEnvironment
   Q_ASSERT(!s_tempDir);

   registerMetaTypes();

   //
   // Create a unique temporary directory using a random number as part of a subdirectory name inside whatever
   // system-standard temp directory Qt proposes to us.  (We also put the application name in the subdirectory name so
   // that anyone doing a manual clean up of their temp directory doesn't have to guess or wonder what created it.
   // Mostly our temp subdirectories will be deleted in our destructor, but core dumps happen etc.)
   //
   // This is important when using the Meson build system because Meson runs several unit tests in parallel (whereas
   // CMake executes them sequentially).
   //
   // Previously we used the thread ID in the directory name, as both CMake and Meson invoke unit tests by running a
   // program, so you might expect parallel instances of the program to have different thread IDs.  On Linux, this is
   // indeed the case and we get different values from QThread::currentThreadId() or std::this_thread::get_id().  On
   // MacOS however, two parallel test runs can have the same value back from QThread::currentThreadId() and
   // std::this_thread::get_id(), so you end up with race conditions of two processes trying to create the same
   // directory etc, leading to test failures.
   //
   std::ostringstream buffer;
   buffer << QRandomGenerator::securelySeeded().generate();
   QString randomId = QString::fromStdString(buffer.str());
   qDebug() << Q_FUNC_INFO << "QThread ID:" << QThread::currentThreadId() << "; Random ID:" << randomId;

   QString subDirName;
   QTextStream{&subDirName} << CONFIG_APPLICATION_NAME_UC << "-UnitTestRun-" << randomId;

   QDir tempDir{QDir::tempPath()};
   if (!tempDir.mkdir(subDirName)) {
      qCritical() << Q_FUNC_INFO << "Unable to create" << subDirName << "sub-directory of" << tempDir.absolutePath();
      throw std::runtime_error{"Unable to create unique temp directory"};
   }
   if (!tempDir.cd(subDirName)) {
      qCritical() << Q_FUNC_INFO << "Unable to access" << tempDir.absolutePath() << "after creating it";
      throw std::runtime_error{"Unable to access unique temp directory"};
   }
   s_tempDir = tempDir;
   qDebug() << Q_FUNC_INFO << "Using" << tempDir.absolutePath() << "as temporary directory";

   // See comment in main.cpp for how much longer we need to retain this call
   xmlInitParser();

   std::cout << "Initialising Test Environment" << std::endl;

   // Create a different set of options to avoid clobbering real options
   static QString const unitTestDomain = QString{"%1/test"}.arg(CONFIG_ORGANIZATION_DOMAIN);
   QCoreApplication::setOrganizationDomain(unitTestDomain);
   static QString const unitTestAppName = QString{"%1-test"}.arg(CONFIG_APPLICATION_NAME_LC);
   QCoreApplication::setApplicationName(unitTestAppName);

   qDebug() << "Initialising PersistentSettings";

   // Set options so that any data modification does not affect any other data
   PersistentSettings::initialise(tempDir.absolutePath());

   // Log test setup
   // Verify that the Logging initialises normally
   qDebug() << "Initialising Logging module";
   Logging::initializeLogging();
   // Remember the message handler that Logging::initializeLogging installed -- see useApplicationLogging()
   s_logMessageHandler = qInstallMessageHandler(nullptr);
   qInstallMessageHandler(s_logMessageHandler);
   // Now change/override a few settings
   // We always want debug logging for tests as it's useful when a test fails
   Logging::setLogLevel(Logging::LogLevel_DEBUG);
   // Test logs go to a /tmp (or equivalent) so as not to clutter the application path with dummy data.
   Logging::setDirectory(QDir{tempDir.absolutePath()}, Logging::NewDirectoryIsTemporary);
   qDebug() << "logging initialised";

   // Inside initializeLogging(), there's a check to see whether we're the test application.  If so, it turns off
   // logging output to stderr.
   qDebug() << Q_FUNC_INFO << "Initialised";

   // Setting French locale below forces ',' as decimal separator and '.' as thousands separator.  Hopefully this
   // helps catch cases where we incorrectly assume locale 'C' etc.
   PersistentSettings::insert_ck(PersistentSettings::Names::color_formula, "morey"  );
   PersistentSettings::insert_ck(PersistentSettings::Names::ibu_formula  , "tinseth");
   PersistentSettings::insert_ck(PersistentSettings::Names::forcedLocale , "fr_FR"  );

   // Tell the application not to require any "user" input on starting
   Application::setInteractive(false);

   //
   // Application::initialize() will initialise a bunch of things, including creating a default database in tempDir
   // courtesy of the call to PersistentSettings::initialise() above.  If there is a problem creating the DB, it will
   // return false.
   //
   if (!Application::initialize()) {
      throw std::runtime_error{"Application::initialize() failed"};
   }

   return;
}

UnitTests::TestEnvironment::~TestEnvironment() {
   Application::cleanup();
   Logging::terminateLogging();
   // Clean up the gibberish logs from disk
   QFileInfoList fileList = Logging::getLogFileList();
   for (int i = 0; i < fileList.size(); i++) {
      QFile(QString(fileList.at(i).canonicalFilePath())).remove();
   }

   // Clear all persistent properties linked with this test suite.
   // It will clear all settings that are application specific, user-scoped, and in the application namespace.
   QSettings().clear();

   //
   // We have to be a bit careful in our cleaning up.  We only want to try to remove the unique temporary directory we
   // created, not the system-wide one.  (It shouldn't be possible for s_tempDir to be the root directory, but it
   // doesn't hurt to check!)
   //
   if (s_tempDir &&
       s_tempDir->exists() &&
       s_tempDir->absolutePath() != QDir::tempPath() &&
       !s_tempDir->isRoot()) {
      qInfo() << Q_FUNC_INFO << "Removing temporary directory" << s_tempDir->absolutePath() << "and its contents";
      if (!s_tempDir->removeRecursively()) {
         //
         // It's not the end of the world if we couldn't remove a temporary directory so, if it happens, just log an
         // error rather than throwing an exception (which is not allowed from a destructor anyway).
         //
         qWarning() << Q_FUNC_INFO << "Unable to remove temporary directory" << s_tempDir->absolutePath();
      }
   }
   s_tempDir.reset();
   s_logMessageHandler = nullptr;
   return;
}
