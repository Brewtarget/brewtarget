/*
 * main.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QSharedMemory>

#include <xercesc/util/PlatformUtils.hpp>
#include <xalanc/Include/PlatformDefinitions.hpp>

#include "config.h"
#include "xml/BeerXml.h"
#include "brewtarget.h"
#include "database.h"

void importFromXml(const QString & filename);
void createBlankDb(const QString & filename);

int main(int argc, char **argv)
{
   QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

   // Initialize Xerces XML tools
   // NB: This is also where where we would initialise xalanc::XalanTransformer if we were using it
   try {
      xercesc::XMLPlatformUtils::Initialize();
   } catch (xercesc::XMLException const & xercesInitException) {
      qCritical() << Q_FUNC_INFO << "Xerces XML Parser Initialisation Failed: " << xercesInitException.getMessage();
      return 1;
   }

   QApplication app(argc, argv);
   app.setOrganizationName("brewtarget");

   // Allows a different set of QTSettings while in debug mode.
   // Settings changed whilst debugging will not interfere with other installed instance of BT.
#ifdef QT_DEBUG
   app.setApplicationName("brewtarget-debug");
#else
   app.setApplicationName("brewtarget");
#endif

   app.setApplicationVersion(VERSIONSTRING);

   //
   // Check whether another instance of Brewtarget is running.  We want to avoid two instances running at the same time
   // because, at best, one of them will be locked out of the database (if using SQLite) and, at worst, race conditions
   // etc between the two instances could lead to data loss/corruption.
   //
   // Using QSharedMemory seems to be the standard way to do this in Qt according to various discussions on Stack
   // Overflow and Qt forums.  Essentially, we try to create one byte of cross-process shared memory with identifier
   // "Brewtarget".  If this fails, it means another process (ie another instance of Brewtarget) has already created
   // such shared memory (which gets automatically destroyed when the application exits).
   //
   // We want to allow the user to override this warning because, according to the Qt documentation, it is possible, on
   // Linux, that we get a "false positive".  Specifically, if the application crashed, then the shared memory will not
   // get cleaned up.  We do attempt to detect and rectify such cases, with the double-check below, but it still seems
   // wise to allow the user to override the warning if for any reason it is triggered incorrectly.
   //
   QSharedMemory sharedMemory("Brewtarget");
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
                                 QApplication::tr("Brewtarget is already running!"),
                                 QApplication::tr("Another instance of Brewtarget is already running.\n\n"
                                                  "Running two copies of the program at once may lead to data loss.\n\n"
                                                  "Press OK to quit."),
                                 QMessageBox::Ignore | QMessageBox::Ok,
                                 QMessageBox::Ok);
         if (buttonPressed == QMessageBox::Ok) {
            // We haven't yet called exec on QApplication, so I'm not sure we _need_ to call exit() here, but it doesn't
            // seem to hurt.
            app.exit();
            return EXIT_SUCCESS;
         }
      }
   }

   QCommandLineParser parser;
   parser.addHelpOption();
   parser.addVersionOption();

   const QCommandLineOption importFromXmlOption("from-xml", "Imports DB from XML in <file>", "file");
   const QCommandLineOption createBlankDBOption("create-blank", "Creates an empty database in <file>", "file");
   /*!
    * \brief Forces the application to a specific user directory.
    *
    * If this directory exists, it will replace the user directory taken
    * from QSettings.
    */
   const QCommandLineOption userDirectoryOption("user-dir", "Overwrite the directory used by the application with <directory>", "directory", QString());

   parser.addOption(importFromXmlOption);
   parser.addOption(createBlankDBOption);
   parser.addOption(userDirectoryOption);

   parser.process(app);

   if (parser.isSet(importFromXmlOption)) importFromXml(parser.value(importFromXmlOption));
   if (parser.isSet(createBlankDBOption)) createBlankDb(parser.value(createBlankDBOption));

   try
   {
      auto mainAppReturnValue = Brewtarget::run(parser.value(userDirectoryOption));

      //
      // Clean exit of Xerces XML tools
      // If we, in future, want to use XalanTransformer, this needs to be extended to:
      //    XalanTransformer::terminate();
      //    XMLPlatformUtils::Terminate();
      //    XalanTransformer::ICUCleanUp();
      //
      xercesc::XMLPlatformUtils::Terminate();

      return mainAppReturnValue;
   }
   catch (const QString &error)
   {
      QMessageBox::critical(0,
            QApplication::tr("Application terminates"),
            QApplication::tr("The application encountered a fatal error.\nError message:\n%1").arg(error));
   }
   catch (std::exception &exception)
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

/*!
 * \brief Imports the content of an xml file to the database.
 *
 * Use at your own risk.
 */
void importFromXml(const QString & filename) {

   QString errorMessage;
   QTextStream errorMessageAsStream{&errorMessage};
   if (!Database::instance().getBeerXml()->importFromXML(filename, errorMessageAsStream)) {
      qCritical() << "Unable to import" << filename << "Error: " << errorMessage;
      exit(1);
   }
    Database::dropInstance();
    Brewtarget::setOption("converted", QDate().currentDate().toString());
    exit(0);
}

//! \brief Creates a blank database using the given filename.
void createBlankDb(const QString & filename) {
    Database::createBlank(filename);
    exit(0);
}
