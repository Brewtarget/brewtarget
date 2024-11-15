/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * main.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • A.J. Drobnich <aj.drobnich@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Maxime Lavigne <duguigne@gmail.com>
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
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include <boost/json/src.hpp> // Needs to be included exactly once in the code to use header-only version of Boost.JSON

#include <iostream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/Include/PlatformDefinitions.hpp>

#include <QApplication>
#include <QCommandLineParser>
#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSharedMemory>

#include "Application.h"
#include "config.h"
#include "database/Database.h"
#include "Localization.h"
#include "Logging.h"
#include "PersistentSettings.h"
#include "serialization/xml/BeerXml.h"
#include "utils/MetaTypes.h"

namespace {

   /*!
    * \brief Imports the content of an xml file to the database.
    *
    * Use at your own risk.
    */
   void importFromXml(const QString & filename) {

      QString errorMessage;
      QTextStream errorMessageAsStream{&errorMessage};
      if (!BeerXML::getInstance().importFromXML(filename, errorMessageAsStream)) {
         qCritical() << "Unable to import" << filename << "Error: " << errorMessage;
         exit(1);
      }
      Database::instance().unload();
      PersistentSettings::insert(PersistentSettings::Names::converted, QDate().currentDate().toString());
      exit(0);
   }

   //! \brief Creates a blank database using the given filename.
   void createBlankDb(const QString & filename) {
      Database::instance().createBlank(filename);
      exit(0);
   }

   /**
    * \brief Uncaught exceptions in a Qt application will terminate the program with a generic error message that does
    *        not give as much info about the exception as we might like.  This small extension of Qt's \c QApplication
    *        class allows us to attempt to log some additional info in such an event.
    *
    *        NOTE that the info logged here will not be anywhere near as helpful as when we can catch exceptions in the
    *        thread where they are thrown, so this is very much a last resort.
    */
   class ExceptionCatchingQApplication : public QApplication {
   public:
      using QApplication::QApplication;
      virtual bool notify(QObject * receiver, QEvent * event) override {
         try {
            return QApplication::notify(receiver, event);
         } catch (std::exception & e) {
            // Obviously "uncaught" here means "uncaught elsewhere in the code"
            std::cerr << "Uncaught exception: " << e.what() << std::endl;
            qCritical().noquote() <<
               Q_FUNC_INFO << "Uncaught exception: " << e.what() << "\nStacktrace:" << Logging::getStackTrace();

            // If we wanted the application to keep running here, we could just drop through to `return false`.  But we
            // don't know whether continuing is a good idea.  So the safest thing is to exit now that we've logged some
            // diagnostics.
            this->exit();
         }
         return false;
      }
   };
}

int main(int argc, char **argv) {
   // Uncomment this line to debug problems loading Qt plug-ins.  (If you're launching from the command line, you can
   // also just set the QT_DEBUG_PLUGINS environment variable from the shell, but we have had a bug that only appeared
   // when launching from Windows GUI.)  The output goes to the same place as Qt logs, so will be interspersed with our
   // own logging.
   //qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));

   //
   // In Qt5, we used to make the following call here:
   //
   //    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
   //
   // However, in Qt6, this serves no purpose as "‘Qt::AA_EnableHighDpiScaling’ is deprecated: High-DPI scaling is
   // always enabled. This attribute no longer has any effect."
   //

   //
   // This ensures the resources in "resources.qrc" are initialised at startup.
   //
   Q_INIT_RESOURCE(resources);

   //
   // Various bits of Qt initialisation need to be done straight away for other Qt functionality to work correctly
   //
   // You might think that it would be possible to specify application and organization name etc entirely at compile
   // time, but it seems these run-time calls are always required.
   //
   // We take advantage of this to allow different persistent settings in debug mode, so that changes made will not
   // interfere with another installed instance of the application.
   //
   // We deliberately do not call app.setOrganizationName() as it just creates an extra level of directories in the Qt
   // default file locations (eg on Linux config location is ~/.config/orgName/appName if both organization and
   // application name are set on the QApplication object, but omitting the call to setOrganizationName() takes out the
   // extra directory layer).
   //
   ExceptionCatchingQApplication app(argc, argv);
   app.setOrganizationDomain(CONFIG_ORGANIZATION_DOMAIN);
   // We used to vary the application name (and therefore location of config files etc) depending on whether we're
   // building with debug or release version of Qt, but on the whole I don't think this is helpful
   app.setApplicationName(CONFIG_APPLICATION_NAME_LC);
   app.setApplicationVersion(CONFIG_VERSION_STRING);

   // Process command-line options relatively early as some may override other settings
   QCommandLineParser parser;
   QCommandLineOption const importFromXmlOption("from-xml", "Imports DB from XML in <file>", "file");
   parser.addOption(importFromXmlOption);
   QCommandLineOption const createBlankDBOption("create-blank", "Creates an empty database in <file>", "file");
   parser.addOption(createBlankDBOption);
   /*!
    * \brief Forces the application to a specific user directory.
    *
    * If this directory exists, it will replace the user directory taken from QSettings.
    *
    * This is mostly useful for developers who want to have an easy way of running an instance of the app against a
    * test database without messing anything up with their real database.
    */
   QCommandLineOption const userDirectoryOption{
      "user-dir",
      "Override the user data directory used by the application with <directory>",
      "directory",
      QString()
   };
   parser.addOption(userDirectoryOption);
   parser.addHelpOption();
   parser.addVersionOption();
   parser.process(app);

   //
   // Having initialised various QApplication settings and read command line options, we can now allow Qt to work out
   // where to get config from.
   //
   // Note that we need to initialise PersistentSettings before we initialise Logging as the latter needs settings from
   // the former.  (We _can_ still log before Logging is initialised, it's just that such log messages will only go to
   // Qt's default logging location (eg console on Linux) and won't end up in our log file.)
   //
   PersistentSettings::initialise(parser.value(userDirectoryOption));
   qDebug() << Q_FUNC_INFO << "Persistent Settings initialised";

   //
   // And once we have config, we can initialise logging
   //
   Logging::initializeLogging();
   qDebug() << Q_FUNC_INFO << "Logging initialised";

   // Initialize Xerces XML tools
   // NB: This is also where where we would initialise xalanc::XalanTransformer if we were using it
   try {
      xercesc::XMLPlatformUtils::Initialize();
   } catch (xercesc::XMLException const & xercesInitException) {
      qCritical() << Q_FUNC_INFO << "Xerces XML Parser Initialisation Failed: " << xercesInitException.getMessage();
      return 1;
   }

   //
   // Check whether another instance of the application is running.  We want to avoid two instances running at the same
   // time because, at best, one of them will be locked out of the database (if using SQLite) and, at worst, race
   // conditions etc between the two instances could lead to data loss/corruption.
   //
   // Using QSharedMemory seems to be the standard way to do this in Qt according to various discussions on Stack
   // Overflow and Qt forums.  Essentially, we try to create one byte of cross-process shared memory with application
   // name as the identifier.  If this fails, it means another process (ie another instance of the application) has
   // already created such shared memory (which gets automatically destroyed when the application exits).
   //
   // We want to allow the user to override this warning because, according to the Qt documentation, it is possible, on
   // Linux, that we get a "false positive".  Specifically, if the application crashed, then the shared memory will not
   // get cleaned up.  We do attempt to detect and rectify such cases, with the double-check below, but it still seems
   // wise to allow the user to override the warning if for any reason it is triggered incorrectly.
   //
   QSharedMemory sharedMemory(CONFIG_APPLICATION_NAME_UC);
   if (!sharedMemory.create(1)) {
      //
      // According to
      // https://stackoverflow.com/questions/42549904/qsharedmemory-is-not-getting-deleted-on-application-crash we can
      // prevent a lot of false positives by manually calling detach() on the shared memory, as this will delete it if
      // no other processes are using it.  Of course, in order to call detach(), we must first call attach().
      //
      sharedMemory.attach();
      sharedMemory.detach(); // This should delete the shared memory if no other process is using it
      if (!sharedMemory.create(1)) {
         enum QMessageBox::StandardButton buttonPressed =
            QMessageBox::warning(NULL,
                                 QApplication::tr("%1 is already running!").arg(CONFIG_APPLICATION_NAME_UC),
                                 QApplication::tr("Another instance of %1 is already running.\n\n"
                                                  "Running two copies of the program at once may lead to data loss.\n\n"
                                                  "Press OK to quit.").arg(CONFIG_APPLICATION_NAME_UC),
                                 QMessageBox::Ignore | QMessageBox::Ok,
                                 QMessageBox::Ok);
         if (buttonPressed == QMessageBox::Ok) {
            // We haven't yet called exec on QApplication, so I'm not sure we _need_ to call exit() here, but it
            // doesn't seem to hurt.
            app.exit();
            return EXIT_SUCCESS;
         }
      }
   }

   if (parser.isSet(importFromXmlOption)) importFromXml(parser.value(importFromXmlOption));
   if (parser.isSet(createBlankDBOption)) createBlankDb(parser.value(createBlankDBOption));

   try {
      qInfo() <<
         "Starting" << CONFIG_APPLICATION_NAME_UC << "v" << CONFIG_VERSION_STRING << " (app name" <<
         app.applicationName() << ") on " << QSysInfo::prettyProductName();
      qInfo() <<
         "Built at" << CONFIG_BUILD_TIMESTAMP << "on" << CONFIG_BUILD_SYSTEM << "for" << CONFIG_RUN_SYSTEM << "with" <<
         CONFIG_CXX_COMPILER_ID << "compiler";
      qInfo() << "Log directory:" << Logging::getDirectory().absolutePath();
      qInfo() << "Using Qt runtime v" << qVersion() << " (compiled against Qt v" << QT_VERSION_STR << ")";
      qInfo() << "Configuration directory:" << PersistentSettings::getConfigDir().absolutePath();
      qInfo() << "Data directory:" << PersistentSettings::getUserDataDir().absolutePath();
      qInfo() << "Resource directory:" << Application::getResourceDir().absolutePath();

      qDebug() << Q_FUNC_INFO << "Library Paths:" << qApp->libraryPaths();

      registerMetaTypes();

      auto mainAppReturnValue = Application::run();

      //
      // Clean exit of Xerces XML tools
      // If we, in future, want to use XalanTransformer, this needs to be extended to:
      //    XalanTransformer::terminate();
      //    XMLPlatformUtils::Terminate();
      //    XalanTransformer::ICUCleanUp();
      //
      xercesc::XMLPlatformUtils::Terminate();

      qDebug() << Q_FUNC_INFO << "Xerces terminated cleanly.  Returning " << mainAppReturnValue;

      return mainAppReturnValue;
   }
   catch (const QString & error)
   {
      QMessageBox::critical(0,
            QApplication::tr("Application terminates"),
            QApplication::tr("The application encountered a fatal error.\nError message:\n%1").arg(error));
   }
   catch (std::exception & exception)
   {
      QMessageBox::critical(0,
            QApplication::tr("Application terminates"),
            QApplication::tr("The application encountered a fatal error.\nError message:\n%1").arg(exception.what()));
   }
   catch (...)
   {
      QMessageBox::critical(0,
            QApplication::tr("Application terminates"),
            QApplication::tr("The application encountered a fatal error."));
   }
   return EXIT_FAILURE;
}
