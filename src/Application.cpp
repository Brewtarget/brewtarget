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

//¥¥vv
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
//¥¥^^

#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QVersionNumber>

#include "Algorithms.h"
#include "BtSplashScreen.h"
#include "config.h"
#include "database/Database.h"
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
   bool checkVersion = true;

   void setCheckVersion(bool value) {
      checkVersion = value;
   }

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
    * \brief Check whether there is a newer version of the software available to download from GitHub.  If so, ask the
    *        user whether they want to upgrade.  (We don't actually DO the upgrade.  We just take them to the place they
    *        can download it.)
    */
   void checkForNewVersion() {
      //
      // Users can turn off the check for new versions
      //
      if (!checkVersion) {
         qDebug() << Q_FUNC_INFO << "Check for new version is disabled";
         return;
      }

      //
      // Checking for the latest version involves requesting a JSON object from the GitHub API over HTTPS.
      //
      // Previously we used the Qt framework (QNetworkAccessManager / QNetworkRequest / QNetworkReply) to do the HTTP
      // request/response.  The problem with this is that, when something goes wrong it can be rather hard to diagnose.
      // Eg we had a bug that triggered a stack overflow in the Qt internals but there was only a limited amount of
      // logging we could add to try to determine what was going on.
      //
      // So now, instead, we use Boost.Beast (which sits on top of Boost.Asio) and OpenSSL.  This is very slightly
      // lower-level -- in that fewer things are magically defaulted for you -- and requires us to use std::string
      // rather than QString.  But at least it does not require a callback function.  And, should we have future
      // problems, it should be easier to delve into.
      //
      // Although it's a bit long-winded, we're not really doing anything clever here.  The example code at
      // https://www.boost.org/doc/libs/1_86_0/libs/beast/doc/html/beast/quick_start/http_client.html explains a lot of
      // what's going on.  We're just doing a bit extra to do HTTPS rather than HTTP.
      //
      std::string const host{"api.github.com"};
      // It would be neat to construct this string at compile-time, but I haven't yet worked out how!
      std::string const path = QString{"/repos/%1/%2/releases/latest"}.arg(CONFIG_APPLICATION_NAME_UC, CONFIG_APPLICATION_NAME_LC).toStdString();
      std::string const port{"443"};
      //
      // Here 11 means HTTP/1.1, 20 means HTTP/2.0, 30 means HTTP/3.0.  (See
      // https://www.boost.org/doc/libs/1_86_0/libs/beast/doc/html/beast/ref/boost__beast__http__message/version/overload1.html.)
      // If we were doing something generic then we'd stick with HTTP/1.1 since that has 100% support.  But, since we're
      // only making one request, and it's to GitHub, and we know they support they newer version of HTTP, we might as
      // well use the newer standard.
      //
      boost::beast::http::request<boost::beast::http::string_body> httpRequest{boost::beast::http::verb::get,
                                                                               path,
                                                                               30};
      httpRequest.set(boost::beast::http::field::host, host);
      //
      // GitHub will respond with an error if the user agent field is not present, but it doesn't care what it's set to
      // and will even accept empty string.
      //
      httpRequest.set(boost::beast::http::field::user_agent, "");

      boost::asio::io_service ioService;
      //
      // A lot of old example code for Boost still uses sslv23_client.  However, TLS 1.3 has been out since 2018, and we
      // know GitHub (along with most other web sites) supports it.  So there's no reason not to use that.
      //
      boost::asio::ssl::context securityContext(boost::asio::ssl::context::tlsv13_client);
      boost::asio::ssl::stream<boost::asio::ip::tcp::socket> secureSocket{ioService, securityContext};
      // The resolver essentially does the DNS requests to look up the host address etc
      boost::asio::ip::tcp::resolver tcpIpResolver{ioService};
      auto endpoint = tcpIpResolver.resolve(host, port);
      // TODO: Need to add time-outs here.

      // Once we have the address, we can connect, do the SSL handshake, and then send the request
      boost::asio::connect(secureSocket.lowest_layer(), endpoint);
      secureSocket.handshake(boost::asio::ssl::stream_base::handshake_type::client);
      boost::beast::http::write(secureSocket, httpRequest);

      // We're expecting a response back pretty quickly, so we'll just wait for it here.  If we find response times are
      // too
      boost::beast::http::response<boost::beast::http::string_body> httpResponse;
      boost::beast::flat_buffer buffer;
      boost::beast::http::read(secureSocket, buffer, httpResponse);

      if (httpResponse.result() != boost::beast::http::status::ok) {
         //
         // It's not the end of the world if we could't check for an update, but we should record the fact.  With some
         // things in Boost.Beast, the easiest way to convert them to a string is via a standard library output stream,
         // so we construct the whole error message like that rather then try to mix-and-match with Qt logging output
         // streams.
         //
         std::ostringstream errorMessage;
         errorMessage << "Error checking for update: " << httpResponse.result_int() <<
            ".\nResponse headers:" << httpResponse.base();
         qInfo().noquote() << Q_FUNC_INFO << QString::fromStdString(errorMessage.str());
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
      QByteArray rawContent = QByteArray::fromStdString(httpResponse.body());
      QJsonParseError jsonParseError{};

      QJsonDocument jsonDocument = QJsonDocument::fromJson(rawContent, &jsonParseError);
      if (QJsonParseError::ParseError::NoError != jsonParseError.error) {
         qWarning() <<
            Q_FUNC_INFO << "Error parsing JSON from version check response:" << jsonParseError.error << "at offset" <<
            jsonParseError.offset;
         return;

      }

      QJsonObject jsonObject = jsonDocument.object();

      QString remoteVersion = jsonObject.value("tag_name").toString();
      // Version names are usually "v3.0.2" etc, so we want to strip the 'v' off the front
      if (remoteVersion.startsWith("v", Qt::CaseInsensitive)) {
         remoteVersion.remove(0, 1);
      }

      //
      // We used to just compare if the remote version is the same as the current one, but it then gets annoying if you
      // are running the nightly build and it keeps asking if you want to, eg, download 4.0.0 because you're "only"
      // running 4.0.1.  So now we do it properly, letting QVersionNumber do the heavy lifting for us.
      //
      QVersionNumber const currentlyRunning{QVersionNumber::fromString(CONFIG_VERSION_STRING)};
      QVersionNumber const latestRelease   {QVersionNumber::fromString(remoteVersion)};

      qInfo() <<
         Q_FUNC_INFO << "Latest release is" << remoteVersion << "(parsed as" << latestRelease << ") ; "
         "currently running" << CONFIG_VERSION_STRING << "(parsed as" << currentlyRunning << ")";

      // If the remote version is newer...
      if (latestRelease > currentlyRunning) {
         // ...and the user wants to download the new version...
         if(QMessageBox::information(&MainWindow::instance(),
                                     QObject::tr("New Version"),
                                     QObject::tr("Version %1 is now available. Download it?").arg(remoteVersion),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::Yes) == QMessageBox::Yes) {
            // ...take them to the website.
            static QString const releasesPage = QString{"%1/releases"}.arg(CONFIG_GITHUB_URL);
            QDesktopServices::openUrl(QUrl(releasesPage));
         } else  {
            // ... and the user does NOT want to download the new version...
            // ... and they want us to stop bothering them...
            if(QMessageBox::question(&MainWindow::instance(),
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

   Application::readSystemOptions();

   Localization::loadTranslations(); // Do internationalization.

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

   // .:TBD:. Could maybe move the calls to init and setVisible inside createMainWindowInstance() in MainWindow.cpp
   MainWindow & mainWindow = MainWindow::instance();
   mainWindow.init();
   mainWindow.setVisible(true);
   splashScreen.finish(&mainWindow);

   checkForNewVersion();
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
   //setOption("user_data_dir", userDataDir);

   Localization::saveSettings();

   IbuMethods::saveIbuFormula();

   ColorMethods::saveColorFormulaSettings();

   Measurement::saveDisplayScales();

   return;
}
