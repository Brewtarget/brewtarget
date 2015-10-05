/*
 * main.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
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
#include "config.h"
#include "brewtarget.h"
#include "database.h"

void importFromXml(const QString & optionValue);
void createBlankDb(const QString & optionValue);

int main(int argc, char **argv)
{  
   QApplication app(argc, argv);
   app.setOrganizationName("brewtarget");
   app.setApplicationName("brewtarget");
   app.setApplicationVersion(VERSIONSTRING);

   QCommandLineParser parser;
   parser.addHelpOption();
   parser.addVersionOption();

   const QCommandLineOption importFromXmlOption("from-xml", "Imports DB from XML", "file");
   const QCommandLineOption createBlankDBOption("create-blank", "Creates a blank DB", "file");

   parser.addOption(importFromXmlOption);
   parser.addOption(createBlankDBOption);

   parser.process(app);

   if (parser.isSet(importFromXmlOption)) importFromXml(parser.value(importFromXmlOption));
   if (parser.isSet(createBlankDBOption)) createBlankDb(parser.value(createBlankDBOption));
   
   return Brewtarget::run();
}

void importFromXml(const QString & optionValue) {
    Database::instance().importFromXML(optionValue);
    Database::dropInstance();
    // If you know enough to run --from-xml, I am going to assume you know
    // enough to do it right
    Brewtarget::setOption("converted", QDate().currentDate().toString());
    exit(0);
}

void createBlankDb(const QString & optionValue) {
    Database::createBlank(optionValue);
    exit(0);
}
