/*
 * main.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QApplication>
#include <QStringList>
#include <QHash>
#include "config.h"
#include "brewtarget.h"
#include "database.h"

// TODO: replace with real parsing (Qt5)
void parseArgs(QApplication const& app)
{
   int i;
   QStringList args(app.arguments());
   QHash< QString, QString > optionValue;
   
   // Parse the args into option/value pairs
   for( i = 0; i < args.size(); ++i )
   {
      QString option(args.at(i));
      QString value;
      
      // All options start with '-'
      if( !option.startsWith("-") )
         continue;
      
      if( i+1 < args.size() )
      {
         // If the arg following the current one is not an option, it is a
         // value.
         if( !args.at(i+1).startsWith("-") )
         {
            value = args.at(i+1);
            ++i;
         }
      }
      
      optionValue.insert(option, value);
   }
   
   // --from-xml
   if( optionValue.contains("--from-xml") )
   {
      Database::instance().importFromXML(optionValue["--from-xml"]);
      Database::dropInstance();
      // If you know enough to run --from-xml, I am going to assume you know
      // enough to do it right
      Brewtarget::setOption("converted", QDate().currentDate().toString());
      exit(0);
   }
}

int main(int argc, char **argv)
{  
   QApplication app(argc, argv);
   app.setOrganizationName("brewtarget");
   app.setApplicationName("brewtarget");
   app.setApplicationVersion(VERSIONSTRING);

   parseArgs(app);
   
   return Brewtarget::run();
}
