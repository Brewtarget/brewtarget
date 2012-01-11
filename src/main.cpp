/*
 * main.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include "brewtarget.h"
#include "config.h"
#include "database.h"

#include <QMetaProperty>

// Need this for changed(QMetaProperty,QVariant) to be emitted across threads.
Q_DECLARE_METATYPE( QMetaProperty )

int main(int argc, char **argv)
{
   QApplication app(argc, argv);
   app.setApplicationName("brewtarget");
   app.setApplicationVersion(VERSIONSTRING);
   app.setOrganizationName("Philip G. Lee");
   Brewtarget::setApp(app);

   // Need this for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   
   // TODO: make a command-line parser class.
   QStringList args(app.arguments());
   int i = args.indexOf("--from-xml");
   if( i >= 0 )
   {
      Database::instance().importFromXML(args.at(i+1));
      return 0;
   }
   
   return Brewtarget::run();
}