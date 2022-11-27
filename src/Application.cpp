/*
 * Application.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Rob Taylor <robtaylor@floopily.org>
 * - Ted Wright <unsure>
 * - Mattias Måhl <mattias@kejsarsten.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Application.h"

#include <iostream>
#include <mutex> // For std::call_once etc

#include <QDebug>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStandardPaths>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>

#include "Algorithms.h"
#include "BtSplashScreen.h"
#include "config.h"
#include "database/Database.h"
#include "Localization.h"
#include "MainWindow.h"
#include "measurement/ColorMethods.h"
#include "measurement/IbuMethods.h"
#include "measurement/Measurement.h"
#include "model/Equipment.h"
#include "model/Fermentable.h"
#include "model/Instruction.h"
#include "model/Mash.h"
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
   bool checkVersion = true;

   void setCheckVersion(bool value) {
      checkVersion = value;
   }

   //! \brief Where the user says the database files are
   QDir userDataDir;

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
      //         users are going to hit other problems.
      //
      QDir resourceDir = Application::getResourceDir();
      bool resourceDirSuccess = resourceDir.exists();
      if (!resourceDirSuccess) {
         QString errMsg{
            QObject::tr("Resource directory \"%1\" is missing.  The software might not operate correctly without this "
                        "directory and its contents.").arg(resourceDir.path())
         };
         qCritical() << Q_FUNC_INFO << errMsg;

         if (Application::isInteractive()) {
            QMessageBox::critical(
               nullptr,
               QObject::tr("Directory Problem"),
               errMsg
            );
         }
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

   QNetworkReply * responseToCheckForNewVersion = nullptr;

   /**
    * \brief Once the response is received to the web request to get latest version info, this parses it and, if
    *        necessary, prompts the user to upgrade.
    *        See \c initiateCheckForNewVersion.
    */
   void finishCheckForNewVersion() {
      if (!responseToCheckForNewVersion) {
         qDebug() << Q_FUNC_INFO << "Invalid sender";
         return;
      }

      // If there is an error, just return.
      if (responseToCheckForNewVersion->error() != QNetworkReply::NoError) {
         qDebug() << Q_FUNC_INFO << "Error checking for update:" << responseToCheckForNewVersion->error();
         return;
      }

      //
      // Checking a version number on Sourceforge is easy, eg a GET request to
      // https://brewtarget.sourceforge.net/version just returns the last version of Brewtarget that was hosted on
      // Sourceforge (quite an old one).
      //
      // On GitHub, it's a bit harder as there's a REST API that gives back loads of info in JSON format.  We don't want
      // to do anything clever with the JSON response, just extract one field, so the Qt JSON support suffices here.
      // (See comments elsewhere for why we don't use it for BeerJSON.)
      //
      QByteArray rawContent = responseToCheckForNewVersion->readAll();
      QJsonParseError jsonParseError{};

      QJsonDocument jsonDocument = QJsonDocument::fromJson(rawContent, &jsonParseError);
      if (QJsonParseError::ParseError::NoError != jsonParseError.error) {
         qWarning() <<
            Q_FUNC_INFO << "Error parsing JSON from version check response:" << jsonParseError.error << "at offset" <<
            jsonParseError.offset;
         return;

      }

      QJsonObject jsonObject = jsonDocument.object();

      QString remoteVersion = jsonObject.value("name").toString();
      qDebug() << Q_FUNC_INFO << "Latest release is" << remoteVersion << "; this release is" << CONFIG_VERSION_STRING;

      // Version names are usually "v3.0.2" etc, so we want to strip the 'v' off the front
      if (remoteVersion.startsWith("v", Qt::CaseInsensitive)) {
         remoteVersion.remove(0, 1);
      }

      // If the remote version is newer...
      if (!remoteVersion.startsWith(CONFIG_VERSION_STRING)) {
         // ...and the user wants to download the new version...
         if( QMessageBox::information(&MainWindow::instance(),
                                    QObject::tr("New Version"),
                                    QObject::tr("Version %1 is now available. Download it?").arg(remoteVersion),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes) == QMessageBox::Yes ) {
            // ...take them to the website.
            QDesktopServices::openUrl(QUrl("https://github.com/Brewtarget/brewtarget/releases"));
         } else  {
            // ... and the user does NOT want to download the new version...
            // ... and they want us to stop bothering them...
            if( QMessageBox::question(&MainWindow::instance(),
                                    QObject::tr("New Version"),
                                    QObject::tr("Stop bothering you about new versions?"),
                                    QMessageBox::Yes | QMessageBox::No,
                                    QMessageBox::Yes) == QMessageBox::Yes) {
               // ... make a note to stop bothering the user about the new version.
               setCheckVersion(false);
            }
         }
         return;
      }

      // The current version is newest so make a note to bother users about future new versions.
      // This means that when a user downloads the new version, this variable will always get reset to true.
      setCheckVersion(true);
      return;
   }


   /**
    * \brief Sends a web request to check whether there is a newer version of the software available
    */
   void initiateCheckForNewVersion(MainWindow* mw) {

      // Don't do anything if the checkVersion flag was set false
      if (!checkVersion) {
         qDebug() << Q_FUNC_INFO << "Check for new version is disabled";
         return;
      }

      // Nobody else needs to access this QNetworkAccessManager object, but it needs to carry on existing after this
      // function returns (otherwise the HTTP GET request will get cancelled), hence why we make it static.
      static QNetworkAccessManager * manager = new QNetworkAccessManager();
      QUrl url("https://api.github.com/repos/Brewtarget/brewtarget/releases/latest");
      responseToCheckForNewVersion = manager->get(QNetworkRequest(url));
      // Since Qt5, you can connect signals to simple functions (see https://wiki.qt.io/New_Signal_Slot_Syntax)
      QObject::connect(responseToCheckForNewVersion, &QNetworkReply::finished, mw, &finishCheckForNewVersion);
      qDebug() <<
         Q_FUNC_INFO << "Sending request to check for new version (request running =" <<
         responseToCheckForNewVersion->isRunning() << ")";
      return;
   }

   /**
    * \brief This is only called from \c Application::getResourceDir to initialise the variable it returns
    *
    * \param resourceDirVar The static local variable inside Application::getResourceDir that is normally not accessible
    *        outside that function, and which needs to be initialised exactly once.
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


      QString path = QCoreApplication::applicationDirPath();
      if (!path.endsWith('/')) {
         path += "/";
      }

#if defined(Q_OS_LINUX)
      // === Linux ===
      // We'll assume the return value from QCoreApplication::applicationDirPath is invalid if it does not end in /bin
      // (because there's no way it would make sense for us to be in an sbin directory
      if (path.endsWith("/bin/")) {
         path += "../share/brewtarget/";
      } else {
         qWarning() <<
            Q_FUNC_INFO << "Cannot determine application binary location (got" << path << ") so using compile-time "
            "constant for resource dir:" << CONFIG_DATA_DIR;
         path = QString(CONFIG_DATA_DIR);
      }
#elif defined(Q_OS_MAC)
      // === Mac ===
      // We should be inside an app bundle.
      path += "../Resources/";
#elif defined(Q_OS_WIN)
      // === Windows ===
      path += "../data/";
#else
#error "Unsupported OS"
#endif

      resourceDirVar = QDir{path};

      qInfo() << Q_FUNC_INFO << "Determined resource directory is" << resourceDirVar.absolutePath();
      return;
   }
}

const QDir Application::getConfigDir() {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.
   QDir dir;
   QFileInfo fileInfo;

   // First, try XDG_CONFIG_HOME.
   // If that variable doesn't exist, create ~/.config
   char* xdg_config_home = getenv("XDG_CONFIG_HOME");

   if (xdg_config_home) {
     qInfo() << QString("XDG_CONFIG_HOME directory is %1").arg(xdg_config_home);
     dir.setPath(QString(xdg_config_home).append("/brewtarget"));
   }
   else {
     // If XDG_CONFIG_HOME doesn't exist, config goes in ~/.config/brewtarget
      qInfo() << QString("XDG_CONFIG_HOME not set.  HOME directory is %1").arg(QDir::homePath());
     QString dirPath = QDir::homePath().append("/.config/brewtarget");
     dir = QDir(dirPath);
   }

   return dir.absolutePath() + "/";

#elif defined(Q_OS_WIN) // Windows OS.

   QDir dir;
   // This is the bin/ directory.
   dir = QDir(QCoreApplication::applicationDirPath());
   dir.cdUp();
   // Now we should be in the base directory (i.e. Brewtarget-2.0.0/)

   dir.cd("data");
   return dir.absolutePath() + "/";

#else
# error "Unsupported OS"
#endif

}

QDir Application::getUserDataDir() {
   return userDataDir;
}

QDir Application::getDefaultUserDataDir() {
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.#if defined(Q_OS_LINUX) || defined(Q_OS_MAC) // Linux OS or Mac OS.
   return getConfigDir();
#elif defined(Q_OS_WIN) // Windows OS.
   // On Windows the Programs directory is normally not writable so we need to get the appData path from the environment instead.
   userDataDir.setPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
   qDebug() << QString("userDataDir=%1").arg(userDataDir.path());
   if (!userDataDir.exists()) {
      qDebug() << QString("User data dir \"%1\" does not exist, trying to create").arg(userDataDir.path());
      createDir(userDataDir);
      qDebug() << "UserDatadit Created";
   }
   return userDataDir;
#else
# error "Unsupported OS"
#endif
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
   // Need these for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment*>();
   qRegisterMetaType<Mash*>();
   qRegisterMetaType<Style*>();
   qRegisterMetaType<Salt*>();
   qRegisterMetaType< QList<BrewNote*> >();
   qRegisterMetaType< QList<Hop*> >();
   qRegisterMetaType< QList<Instruction*> >();
   qRegisterMetaType< QList<Fermentable*> >();
   qRegisterMetaType< QList<Misc*> >();
   qRegisterMetaType< QList<Yeast*> >();
   qRegisterMetaType< QList<Water*> >();
   qRegisterMetaType< QList<Salt*> >();

   // Make sure all the necessary directories and files we need exist before starting.
   ensureDirectoriesExist();

   readSystemOptions();

   Localization::loadTranslations(); // Do internationalization.

#if defined(Q_OS_MAC)
   qt_set_sequence_auto_mnemonic(true); // turns on Mac Keyboard shortcuts
#endif

   // Check if the database was successfully loaded before
   // loading the main window.
   qDebug() << Q_FUNC_INFO << "Loading Database...";
   return Database::instance().loadSuccessful();
}

void Application::cleanup() {
   qDebug() << Q_FUNC_INFO << "Brewtarget is cleaning up.";

//   if (responseToCheckForNewVersion) {
//      qDebug() <<
//         Q_FUNC_INFO << "Request to check for new version (" << responseToCheckForNewVersion->request().url() << ") "
//         "running =" << responseToCheckForNewVersion->isRunning() << ", error =" << responseToCheckForNewVersion->error();
//   }

   // Should I do qApp->removeTranslator() first?
   MainWindow::DeleteMainWindow();

   Database::instance().unload();
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
   if( !initialize() )
   {
      cleanup();
      return 1;
   }
   Database::instance().checkForNewDefaultData();

   // .:TBD:. Could maybe move the calls to init and setVisible inside createMainWindowInstance() in MainWindow.cpp
   MainWindow & mainWindow = MainWindow::instance();
   mainWindow.init();
   mainWindow.setVisible(true);
   splashScreen.finish(&mainWindow);

   initiateCheckForNewVersion(&mainWindow);
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
   checkVersion = PersistentSettings::value(PersistentSettings::Names::check_version, QVariant(true)).toBool();
   qDebug() << Q_FUNC_INFO << "checkVersion=" << checkVersion;

   //=====================Last DB Merge Request======================
   if (PersistentSettings::contains(PersistentSettings::Names::last_db_merge_req)) {
      Database::lastDbMergeRequest = QDateTime::fromString(PersistentSettings::value(PersistentSettings::Names::last_db_merge_req,"").toString(), Qt::ISODate);
   }

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
   PersistentSettings::insert(PersistentSettings::Names::check_version, checkVersion);
   PersistentSettings::insert(PersistentSettings::Names::last_db_merge_req, Database::lastDbMergeRequest.toString(Qt::ISODate));
   //setOption("user_data_dir", userDataDir);

   Localization::saveSettings();

   IbuMethods::saveIbuFormula();

   ColorMethods::saveColorFormulaSettings();

   Measurement::saveDisplayScales();

   return;
}
