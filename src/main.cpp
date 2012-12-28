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
#include <QList>
#include <QSharedMemory>
#include <iostream>
#include "brewtarget.h"
#include "config.h"
#include "database.h"

#include <QMetaProperty>
class BrewNote;
class Equipment;
class Fermentable;
class Hop;
class Instruction;
class Mash;
class Misc;
class Style;
class Yeast;

// Need this for changed(QMetaProperty,QVariant) to be emitted across threads.
Q_DECLARE_METATYPE( QMetaProperty )
Q_DECLARE_METATYPE( Equipment* )
Q_DECLARE_METATYPE( Mash* )
Q_DECLARE_METATYPE( Style* )
Q_DECLARE_METATYPE( Brewtarget::DBTable )
Q_DECLARE_METATYPE( QList<BrewNote*> )
Q_DECLARE_METATYPE( QList<Hop*> )
Q_DECLARE_METATYPE( QList<Instruction*> )
Q_DECLARE_METATYPE( QList<Fermentable*> )
Q_DECLARE_METATYPE( QList<Misc*> )
Q_DECLARE_METATYPE( QList<Yeast*> )
Q_DECLARE_METATYPE( QList<Water*> )

int main(int argc, char **argv)
{
   // This uses shared memory to detect other instances.
   QSharedMemory mem("brewtargetCheck");
   if( !mem.create(1) )
   {
      std::cerr << "Another instance of brewtarget is already runnning." << std::endl;
      return 1;
   }
   
   QApplication app(argc, argv);
   app.setApplicationName("brewtarget");
   app.setApplicationVersion(VERSIONSTRING);
   app.setOrganizationName("Philip G. Lee");

   // Need this for changed(QMetaProperty,QVariant) to be emitted across threads.
   qRegisterMetaType<QMetaProperty>();
   qRegisterMetaType<Equipment*>();
   qRegisterMetaType<Mash*>();
   qRegisterMetaType<Style*>();
   qRegisterMetaType<Brewtarget::DBTable>();
   qRegisterMetaType< QList<BrewNote*> >();
   qRegisterMetaType< QList<Hop*> >();
   qRegisterMetaType< QList<Instruction*> >();
   qRegisterMetaType< QList<Fermentable*> >();
   qRegisterMetaType< QList<Misc*> >();
   qRegisterMetaType< QList<Yeast*> >();
   qRegisterMetaType< QList<Water*> >();

   // TODO: make a command-line parser class.
   QStringList args(app.arguments());
   int i = args.indexOf("--from-xml");
   if( i >= 0 )
   {
      Database::instance().importFromXML(args.at(i+1));
      Database::dropInstance();
      // If you know enough to run --from-xml, I am going to assume you know
      // enough to do it right
      Brewtarget::setOption("converted", QDate().currentDate().toString());
      return 0;
   }
   
   return Brewtarget::run();
}
