/*
 * main.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Maxime Lavigne <duguigne@gmail.com>
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
#include "config.h"
#include "brewtarget.h"
#include "database.h"

void importFromXml(const QString & filename);
void createBlankDb(const QString & filename);

int main(int argc, char **argv)
{  
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
      return Brewtarget::run(parser.value(userDirectoryOption));
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
    Database::instance().importFromXML(filename);
    Database::dropInstance();
    Brewtarget::setOption("converted", QDate().currentDate().toString());
    exit(0);
}

//! \brief Creates a blank database using the given filename.
void createBlankDb(const QString & filename) {
    Database::createBlank(filename);
    exit(0);
}
