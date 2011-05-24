/*
 * UnitSystem.cpp is part of Brewtarget, written by Mik Firestone
 * (mikfire@gmail.com) and is Copyright Philip G. Lee
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

#include "UnitSystem.h"
#include <QRegExp>
#include <QString>

QMap<QString, Unit*> UnitSystem::nameToUnit;
QRegExp UnitSystem::amtUnit;

UnitSystem::UnitSystem()
{
   amtUnit.setPattern("(\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);
}

UnitSystem::~UnitSystem()
{
}

double UnitSystem::qstringToSI(QString qstr, Unit* defUnit)
{
   
   double amt = 0.0;
   Unit* u = defUnit;

   if (amtUnit.indexIn(qstr) == -1)
      return 0.0;

   amt = amtUnit.cap(1).toDouble();
   QString unit = amtUnit.cap(2);

   if ( unit.size() > 0 && getUnit(unit) )
      u = getUnit(unit);

   return u->toSI(amt);
}

Unit* UnitSystem::getUnit(const QString& name)
{
   if( nameToUnit.count(name) < 1 )
      return 0;
   else
      return nameToUnit[name];
}

