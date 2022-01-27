/*
 * brewtarget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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
#include "brewtarget.h"

#include <iostream>

#include <QDebug>
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
   bool checkVersion = true;

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
         if (Brewtarget::isInteractive()) {
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
      // A missing resource directory is a serious issue, without it we're missing the default DB, sound files &
      // translations.  We could attempt to create it, like the other config/data directories, but an empty resource
      // dir is just as bad as a missing one.  So, instead, we'll display a little more dire warning, and not try to
      // create it.
      QDir resourceDir = Brewtarget::getResourceDir();
      bool resourceDirSuccess = resourceDir.exists();
      if (!resourceDirSuccess) {
         QString errMsg{
            QObject::tr("Resource directory \"%1\" is missing.  Some features will be unavailable.").arg(resourceDir.path())
         };
         qCritical() << Q_FUNC_INFO << errMsg;

         if (Brewtarget::isInteractive()) {
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
    * \brief Checks for a newer version and prompts user to download.
    *.:TODO:. This needs to be updated
    */
   void checkForNewVersion(MainWindow* mw) {

      // Don't do anything if the checkVersion flag was set false
      if (checkVersion == false ) {
         return;
      }

      QNetworkAccessManager manager;
   QUrl url("http://brewtarget.sourceforge.net/version");
      QNetworkReply* reply = manager.get( QNetworkRequest(url) );
      QObject::connect( reply, &QNetworkReply::finished, mw, &MainWindow::finishCheckingVersion );
      return;
   }

}

void Brewtarget::setCheckVersion(bool value) {
   checkVersion = value;
   return;
}

QDir Brewtarget::getDataDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_OS_LINUX) // Linux OS.

   dir = QString(CONFIGDATADIR);

#elif defined(Q_OS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";

#elif defined(Q_OS_WIN) // Windows OS.

   dir += "/../data/";

#else
# error "Unsupported OS"
#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

QDir Brewtarget::getDocDir()
{
   QString dir = qApp->applicationDirPath();
#if defined(Q_OS_LINUX) // Linux OS.

   dir = QString(CONFIGDOCDIR);

#elif defined(Q_OS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/en.lproj/";

#elif defined(Q_OS_WIN) // Windows OS.

   dir += "/../doc/";

#else
# error "Unsupported OS"
#endif

   if( ! dir.endsWith('/') )
      dir += "/";

   return dir;
}

const QDir Brewtarget::getConfigDir()
{
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

QDir Brewtarget::getUserDataDir()
{
   return userDataDir;
}

QDir Brewtarget::getDefaultUserDataDir()
{
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

QDir Brewtarget::getResourceDir() {
   // Unlike some of the other directories, the resource dir needs to be something that can be determined at
   // compile-time
   QString dir = qApp->applicationDirPath();
#if defined(Q_OS_LINUX) // Linux OS.

   dir = QString(CONFIGDATADIR);

#elif defined(Q_OS_MAC) // MAC OS.

   // We should be inside an app bundle.
   dir += "/../Resources/";

#elif defined(Q_OS_WIN) // Windows OS.

   dir += "/../data/";

#else
# error "Unsupported OS"
#endif

   if (!dir.endsWith('/')) {
      dir += "/";
   }

   return dir;
}

bool Brewtarget::initialize()
{
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
   qDebug() << "Loading Database...";
   return Database::instance().loadSuccessful();
}

void Brewtarget::cleanup() {
   qDebug() << "Brewtarget is cleaning up.";
   // Should I do qApp->removeTranslator() first?
   MainWindow::DeleteMainWindow();

   Database::instance().unload();
   return;
}

bool Brewtarget::isInteractive() {
   return interactive;
}

void Brewtarget::setInteractive(bool val) {
   interactive = val;
   return;
}

int Brewtarget::run() {
   int ret = 0;

   BtSplashScreen splashScreen;
   splashScreen.show();
   qApp->processEvents();
   if( !initialize() )
   {
      cleanup();
      return 1;
   }
   qInfo() << QString("Starting Brewtarget v%1 on %2.").arg(VERSIONSTRING).arg(QSysInfo::prettyProductName());
   Database::instance().checkForNewDefaultData();

   // .:TBD:. Could maybe move the calls to init and setVisible inside createMainWindowInstance() in MainWindow.cpp
   MainWindow & mainWindow = MainWindow::instance();
   mainWindow.init();
   mainWindow.setVisible(true);
   splashScreen.finish(&mainWindow);

   checkForNewVersion(&mainWindow);
   do {
      ret = qApp->exec();
   } while (ret == 1000);

   cleanup();

   qDebug() << Q_FUNC_INFO << "Cleaned up.  Returning " << ret;

   return ret;
}

void Brewtarget::updateConfig() {
   int cVersion = PersistentSettings::value(PersistentSettings::Names::config_version, QVariant(0)).toInt();
   while ( cVersion < CONFIG_VERSION ) {
      switch ( ++cVersion ) {
         case 1:
            // Update the dbtype, because I had to increase the NODB value from -1 to 0
            int newType = static_cast<Database::DbType>(PersistentSettings::value(PersistentSettings::Names::dbType, Database::NODB).toInt() + 1);
            // Write that back to the config file
            PersistentSettings::insert(PersistentSettings::Names::dbType, static_cast<int>(newType));
            // and make sure we don't do it again.
            PersistentSettings::insert(PersistentSettings::Names::config_version, QVariant(cVersion));
            break;
      }
   }
   return;
}

void Brewtarget::readSystemOptions() {
   // update the config file before we do anything
   updateConfig();

   //================Version Checking========================
   checkVersion = PersistentSettings::value(PersistentSettings::Names::check_version, QVariant(false)).toBool();

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

void Brewtarget::saveSystemOptions() {
   PersistentSettings::insert(PersistentSettings::Names::check_version, checkVersion);
   PersistentSettings::insert(PersistentSettings::Names::last_db_merge_req, Database::lastDbMergeRequest.toString(Qt::ISODate));
   //setOption("user_data_dir", userDataDir);

   Localization::saveSettings();

   IbuMethods::saveIbuFormula();

   ColorMethods::saveColorFormulaSettings();

   Measurement::saveDisplayScales();

   return;
}
