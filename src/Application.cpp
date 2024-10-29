/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * Application.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Chris Pavetto <chrispavetto@gmail.com>
 *   • Dan Cavanagh <dan@dancavanagh.com>
 *   • Daniel Moreno <danielm5@users.noreply.github.com>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Meess <Daedalus12@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
 *   • Medic Momcilo <medicmomcilo@gmail.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Mikhail Gorbunov <mikhail@sirena2000.ru>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Rob Taylor <robtaylor@floopily.org>
 *   • Scott Peshak <scott@peshak.net>
 *   • Ted Wright <tedwright@users.sourceforge.net>
 *   • Théophane Martin <theophane.m@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "Application.h"

#include <iostream>
#include <mutex> // For std::call_once etc

#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QThread>
#include <QUrl>
#include <QVersionNumber>

#include "Algorithms.h"
#include "BtSplashScreen.h"
#include "config.h"
#include "database/Database.h"
#include "LatestReleaseFinder.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Hop.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Misc.h"
#include "model/Salt.h"
#include "model/Style.h"
#include "model/Water.h"
#include "model/Yeast.h"
#include "PersistentSettings.h"

// Needed for kill(2)
#if defined(Q_OS_UNIX)
#include <sys/types.h>
#include <signal.h>
#endif

namespace {

   bool interactive = true;

   //! \brief If this option is false, do not bother the user about new versions.
   bool tellUserAboutNewRelease = true;

   //! \brief Worker thread for finding the latest released version of the program
   QThread latestReleaseFinderThread;

   /**
    * \brief Create a directory if it doesn't exist, popping a error dialog if creation fails
    */
   bool createDir(QDir dir) {
      if( ! dir.mkpath(dir.absolutePath()) ) {
         // Write a message to the log, the usablity check below will alert the user
         QString errText(QObject::tr("Error attempting to create directory \"%1\""));
         qCritical() << errText.arg(dir.path());
      }

      // It's possible that the path exists, but is useless to us
      if( ! dir.exists() || ! dir.isReadable() ) {

         QString errText{QObject::tr("\"%1\" cannot be read.")};
         qWarning() << errText.arg(dir.path());
         if (Application::isInteractive()) {
            QString errTitle(QObject::tr("Directory Problem"));
            QMessageBox::information(
               nullptr,
               errTitle,
               errText.arg(dir.path())
            );
         }
         return false;
      }

      return true;
   }

   /**
    * \brief Ensure our directories exist.
    */
   bool ensureDirectoriesExist() {
      //
      // A missing resource directory is a serious issue, without it we're missing the default DB, sound files &
      // translations.  We could attempt to create it, like the other config/data directories, but an empty resource
      // dir is just as bad as a missing one.  So, instead, we'll display a little more dire warning.
      //
      // .:TBD:. Maybe we should terminate the app here as it's likely that there's some problem with the install and
      //         users are going to hit other problems, including the program crashing.
      //
      QDir resourceDir = Application::getResourceDir();
      bool resourceDirSuccess = resourceDir.exists();
      if (!resourceDirSuccess) {
         QString errMsg{
            QObject::tr("Resource directory %1 is missing.  Without this directory and its contents, the software "
                        "will not operate correctly and may terminate abruptly.").arg(resourceDir.absolutePath())
         };
         qCritical() << Q_FUNC_INFO << errMsg;

         if (Application::isInteractive()) {
            QMessageBox::critical(
               nullptr,
               QObject::tr("Directory Problem"),
               errMsg
            );
         }
      } else {
         qInfo() << Q_FUNC_INFO << "Resource directory" << resourceDir.absolutePath() << "exists";
         QString directoryListing;
         QTextStream dirListStream{&directoryListing};
         QDirIterator ii(resourceDir.absolutePath(), QDirIterator::Subdirectories);
         while (ii.hasNext()) {
            // For the moment, the output format is a bit clunky, and we do not correctly skip over things we should
            // omit such as the parent directory from the ".." link, or multiple visits to the current directory by the
            // iterator.
            auto fileName = ii.next();
            if (fileName != "..") {
               auto fileInfo = ii.fileInfo();
               dirListStream <<
                  "   " << fileInfo.absoluteFilePath() << "\t\t(" << fileInfo.permissions() << ") " << fileInfo.size() <<
                  " bytes\n";
            }

         }
         qDebug().noquote() << Q_FUNC_INFO << "Resource directory contents:" << '\n' << directoryListing;
      }

      return resourceDirSuccess &&
             createDir(PersistentSettings::getConfigDir()) &&
             createDir(PersistentSettings::getUserDataDir());
   }

   /**
    * \brief Every so often, we need to update the config file itself. This does that.
    */
   void updateConfig() {
      int cVersion = PersistentSettings::value(PersistentSettings::Names::config_version, QVariant(0)).toInt();
      while ( cVersion < CONFIG_VERSION ) {
         switch ( ++cVersion ) {
            case 1:
               // Update the dbtype, because I had to increase the NODB value from -1 to 0
               Database::DbType newType =
                  static_cast<Database::DbType>(
                     PersistentSettings::value(PersistentSettings::Names::dbType,
                                             static_cast<int>(Database::DbType::NODB)).toInt() + 1
                  );
               // Write that back to the config file
               PersistentSettings::insert(PersistentSettings::Names::dbType, static_cast<int>(newType));
               // and make sure we don't do it again.
               PersistentSettings::insert(PersistentSettings::Names::config_version, QVariant(cVersion));
               break;
         }
      }
      return;
   }

   /**
    * \brief This is called to tell us the version number of the latest release of the program in its main GitHub
    *        repository.
    *
    *        NB: AFAICT, because this is a freestanding function, rather than a member function of a QObject subclass,
    *            this gets run on the caller's thread.  In particular this means it could run at the same time as other
    *            code running inside the program's main Qt event loop.  If we wanted to do anything that might risk
    *            clashing with other code then we would either want to add locking here or move this function to be,
    *            say, a slot on one of our objects (eg MainWindow).
    */
   void checkAgainstLatestRelease(QVersionNumber const latestRelease) {

      //
      // We used to just compare if the remote version is the same as the current one, but it then gets annoying if you
      // are running the nightly build and it keeps asking if you want to, eg, download 4.0.0 because you're "only"
      // running 4.0.1.  So now we do it properly, letting QVersionNumber do the heavy lifting for us.
      //
      QVersionNumber const currentlyRunning{QVersionNumber::fromString(CONFIG_VERSION_STRING)};

      qInfo() <<
         Q_FUNC_INFO << "Latest release is" << latestRelease.toString() << ") ; currently running" <<
         CONFIG_VERSION_STRING << "(parsed as" << currentlyRunning.toString() << ")";

      if (latestRelease > currentlyRunning) {
         //
         // Users can turn off the notification about new versions, though note below that this gets reset once they are
         // running the latest version again.
         //
         if (!tellUserAboutNewRelease) {
            qInfo() << Q_FUNC_INFO << "Check for new version is disabled";
            return;
         }

         //
         // The latest release is newer than what we are currently running.
         // See if the user wants to download the newer version.
         //
         bool const wantsToDownload{
            QMessageBox::Yes == QMessageBox::information(
               &MainWindow::instance(),
               QObject::tr("New Version"),
               QObject::tr("Version %1 is now available. Download it?").arg(latestRelease.toString()),
               QMessageBox::Yes | QMessageBox::No,
               QMessageBox::Yes
            )
         };
         if (wantsToDownload) {
            //
            // It would be a bit of a tall order for the program to upgrade itself in place.  We just take the user to
            // the release download page.
            //
            static QString const releasesPage = QString{"%1/releases"}.arg(CONFIG_GITHUB_URL);
            QDesktopServices::openUrl(QUrl(releasesPage));
         } else {
            //
            // If the user doesn't want to be taken to the download page, give them the option to stop being reminded
            // about the new release.
            //
            if(QMessageBox::question(&MainWindow::instance(),
                                     QObject::tr("New Version"),
                                     QObject::tr("Stop bothering you about new versions?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes) == QMessageBox::Yes) {
               // ... make a note to stop bothering the user about the new version.
               tellUserAboutNewRelease = false;
            }
         }
         return;
      }

      //
      // The user is already running the latest release.  Make sure the user gets notified when a newer release is
      // available.  (They can then turn off that notification if they want.)  Essentially, this means that, every time
      // the user installs whatever the latest version of the software is, we ensure that we will tell them at least
      // once when there is a new release.
      //
      tellUserAboutNewRelease = true;
      return;
   }


   /**
    * \brief This is only called from \c Application::getResourceDir to initialise the variable it returns
    *
    * \param resourceDirVar The static local variable inside Application::getResourceDir that is normally not accessible
    *                       outside that function, and which needs to be initialised exactly once.
    */
   void initResourceDir(QDir & resourceDirVar) {
      //
      // Directory locations are complicated on Linux because different distros can do things differently.  (See comment
      // in main CMakeLists.txt for more info.)  Also the documentation for QCoreApplication::applicationDirPath() says
      // "Warning: On Linux, this function will try to get the path from the /proc file system. If that fails, it
      // assumes that argv[0] contains the absolute file name of the executable. The function also assumes that the
      // current directory has not been changed by the application."
      //
      // So, on Linux, our choices are:
      //   (1) Assume that binary is in /usr/bin and resources are in /usr/share/[application name].  This should be
      //       right most of the time, because it's what most of the big distros do.  But it could be a problem, eg,
      //       someone compiling from source might want to use:
      //         - /usr/local/bin and /usr/local/share/[application name], or
      //         - $HOME/.local/bin and $HOME/.local/share/[application name]
      //   (2) Get the directory at run-time from Qt.  If Qt is able to read from the /proc pseudo file system then the
      //       info will be spot on as it comes from the kernel.  AFAICT the warning in the Qt doco is only because,
      //       technically, we can't guarantee that /proc will always be mounted in every instance of every Linux
      //       install.
      //   (3) Set the install directory at compile time and inject that value into the code.  This is fine if you're
      //       building and installing on the same machine, but might be problematic if we're building multiple target
      //       packages on one machine and they don't all have the same install directory.
      //
      // Historically we did (3) for Linux (and (2) for other platforms), but this occasionally led to problems when
      // people were doing their own compilation.   So, now, we do (2) for everything but use (3) as the back-up on
      // Linux in the (hopefully extremely rare) case that /proc is not available.
      //
      // ADDITIONALLY, we need to handle the case of the brewtarget_tests application we build to run unit tests.  This
      // lives in the mbuild (meson) or build (CMake) directory and needs to get its resources from the source tree
      // (../data directory).
      //
      QString path = QCoreApplication::applicationDirPath();
      if (!path.endsWith('/')) {
         path += "/";
      }

      // Note that the Qt "application name" is not the same as the executable name on disk; it is what is set by the
      // call to QCoreApplication::setApplicationName when the application is started.
      QString applicationName = QCoreApplication::applicationName();
      qInfo() << Q_FUNC_INFO << "Application name" << applicationName;
      if (applicationName.endsWith("-test")) {
         qInfo() << Q_FUNC_INFO << "Assuming this is Unit Testing executable and resources are in ../data directory";
         path += "../data/";
      } else {

#if defined(Q_OS_LINUX)
         // === Linux ===
         // We'll assume the return value from QCoreApplication::applicationDirPath is invalid if it does not end in
         // /bin (because there's no way it would make sense for us to be in an sbin directory
         if (path.endsWith("/bin/")) {
            path += QString{"../share/%1/"}.arg(CONFIG_APPLICATION_NAME_LC);
         } else {
            qWarning() <<
               Q_FUNC_INFO << "Cannot determine application binary location (got" << path << ") so using compile-time "
               "constant for resource dir:" << CONFIG_DATA_DIR;
            path = QString(CONFIG_DATA_DIR);
         }
#elif defined(Q_OS_MACOS)
         // === Mac ===
         // We should be inside an app bundle.
         path += "../Resources/";
#elif defined(Q_OS_WIN)
         // === Windows ===
         path += "../data/";
#else
#error "Unsupported OS"
#endif
      }
      resourceDirVar = QDir{path};

      qInfo() << Q_FUNC_INFO << "Determined resource directory is" << resourceDirVar.absolutePath();
      return;
   }

}

QDir Application::getResourceDir() {
   //
   // We want to initialise the resourceDir exactly once.  Using std::call_once means that happens even in a
   // multi-threaded application.
   //
   static std::once_flag initFlag_resourceDir;
   static QDir resourceDir;

   std::call_once(initFlag_resourceDir, initResourceDir, resourceDir);

   return resourceDir;
}

bool Application::initialize() {
   // Need these for changed(QMetaProperty, QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment *>();
   qRegisterMetaType<Mash      *>();
   qRegisterMetaType<Style     *>();
   qRegisterMetaType<Salt      *>();
   qRegisterMetaType<QList<BrewNote    *>>();
   qRegisterMetaType<QList<Hop         *>>();
   qRegisterMetaType<QList<Instruction *>>();
   qRegisterMetaType<QList<Fermentable *>>();
   qRegisterMetaType<QList<Misc        *>>();
   qRegisterMetaType<QList<Yeast       *>>();
   qRegisterMetaType<QList<Water       *>>();
   qRegisterMetaType<QList<Salt        *>>();

   // Make sure all the necessary directories and files we need exist before starting.
   ensureDirectoriesExist();

   // TODO: Seems a bit ugly that we call readSystemOptions here but saveSystemOptions from MainWindow::closeEvent.
   // In the long run, it would be a lot more elegant to use RAII to automatically store everything we read from
   // PersistentSettings.
   //
   Application::readSystemOptions();

///   // Load in the default / remembered translations
///   Localization::loadTranslations();

   QLocale const & locale = Localization::getLocale();
   qInfo() <<
      "Locale:" << locale.name() << "(Decimal point:" << locale.decimalPoint() << "/ Thousands separator:" <<
      locale.groupSeparator() << ")";

#if defined(Q_OS_MACOS)
   qt_set_sequence_auto_mnemonic(true); // turns on Mac Keyboard shortcuts
#endif

   // Uncomment the following to list all the entries in our resource bundle.  This can be helpful at certain points in
   // debugging, but is not normally needed.
//   QDirIterator resource(":", QDirIterator::Subdirectories);
//   while (resource.hasNext()) {
//      qDebug() << "Resource:" << resource.next();
//   }

   // Check if the database was successfully loaded before
   // loading the main window.
   qInfo() << Q_FUNC_INFO << "Loading Database...";
   return Database::instance().loadSuccessful();
}

void Application::cleanup() {
   qDebug() << Q_FUNC_INFO << CONFIG_APPLICATION_NAME_UC << "is cleaning up.";
   // Should I do qApp->removeTranslator() first?
   MainWindow::DeleteMainWindow();

   Database::instance().unload();

   //
   // Ensure the thread we spawned to check for new versions is properly terminated
   //
   latestReleaseFinderThread.quit();
   latestReleaseFinderThread.wait();

   return;
}

bool Application::isInteractive() {
   return interactive;
}

void Application::setInteractive(bool val) {
   interactive = val;
   return;
}

int Application::run() {
   int ret = 0;

   BtSplashScreen splashScreen;
   splashScreen.show();
   qApp->processEvents();
   if (!Application::initialize()) {
      cleanup();
      return 1;
   }
   Database::instance().checkForNewDefaultData();

   //
   // Make sure the MainWindow singleton exists, but don't initialise it just yet.  We're going to use the end of the
   // initialisation to trigger the background thread that checks to see whether a new version of the program is
   // available, so we want to set that background thread up first.  (But we need the MainWindow object to exist, so we
   // can connect signals and slots.)
   //
   MainWindow & mainWindow = MainWindow::instance();

   //
   // It feels a bit wrong these days to be calling `new` directly, but it is the way to do things in Qt.  We tell the
   // framework about the object we've created, and it handles clean-up for us.
   //
   LatestReleaseFinder * latestReleaseFinder = new LatestReleaseFinder{};
   latestReleaseFinder->moveToThread(&latestReleaseFinderThread);
   mainWindow.connect(&latestReleaseFinderThread, &QThread::finished, latestReleaseFinder, &QObject::deleteLater);

   mainWindow.connect(&mainWindow, &MainWindow::initialisedAndVisible, latestReleaseFinder, &LatestReleaseFinder::checkMainRespository);
   mainWindow.connect(latestReleaseFinder, &LatestReleaseFinder::foundLatestRelease, &checkAgainstLatestRelease);
   latestReleaseFinderThread.start();

   mainWindow.initialiseAndMakeVisible();
   splashScreen.finish(&mainWindow);
   do {
      ret = qApp->exec();
   } while (ret == 1000);

   cleanup();

   qDebug() << Q_FUNC_INFO << "Cleaned up.  Returning " << ret;

   return ret;
}

void Application::readSystemOptions() {
   // update the config file before we do anything
   updateConfig();

   //================Version Checking========================
   tellUserAboutNewRelease = PersistentSettings::value(PersistentSettings::Names::check_version, QVariant(true)).toBool();
   qDebug() << Q_FUNC_INFO << "tellUserAboutNewRelease=" << tellUserAboutNewRelease;

   Measurement::loadDisplayScales();

   //===================IBU===================
   IbuMethods::loadIbuFormula();

   //========================Color Formula======================
   ColorMethods::loadColorFormulaSettings();

   //=======================Language & Date format===================
   Localization::loadSettings();

   return;

}

void Application::saveSystemOptions() {
   PersistentSettings::insert(PersistentSettings::Names::check_version, tellUserAboutNewRelease);
   //setOption("user_data_dir", userDataDir);

   Localization::saveSettings();

   IbuMethods::saveIbuFormula();

   ColorMethods::saveColorFormulaSettings();

   Measurement::saveDisplayScales();

   return;
}
