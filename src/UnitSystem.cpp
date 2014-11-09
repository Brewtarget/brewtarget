/*
 * UnitSystem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#include "UnitSystem.h"
#include <QRegExp>
#include <QString>
#include <QLocale>
#include <QDebug>
#include "unit.h"

const int UnitSystem::fieldWidth = 0;
const char UnitSystem::format = 'f';
const int UnitSystem::precision = 3;

UnitSystem::UnitSystem()
{
   // Make sure we get the right decimal point (. or ,) and the right grouping
   // separator (, or .). Some locales write 1.000,10 and other write
   // 1,000.10. We need to catch both
   QString decimal = QRegExp::escape( QLocale::system().decimalPoint());
   QString grouping = QRegExp::escape(QLocale::system().groupSeparator());

   amtUnit.setPattern("((?:\\d+" + grouping + ")?\\d+(?:" + decimal + "\\d+)?|" + decimal + "\\d+)\\s*(\\w+)?");
   amtUnit.setCaseSensitivity(Qt::CaseInsensitive);
}

double UnitSystem::qstringToSI(QString qstr, Unit* defUnit, bool force)
{
   bool convOk = true;
   double amt = 0.0;
   Unit* u = defUnit;
   Unit* found = 0;

   if (amtUnit.indexIn(qstr) == -1)
   {
      return 0.0;
   }

   amt = QLocale().toDouble(amtUnit.cap(1), &convOk);
   if( !convOk )
   {
      amt = QLocale::c().toDouble(amtUnit.cap(1));
   }
   
   QString unit = amtUnit.cap(2);

   found = Unit::getUnit(unit,false);

   if ( ! force && found )
   {
      u = found;
   }

   if ( u == 0 )
   {
      return -1.0;
   }

   return u->toSI(amt);
}

QString UnitSystem::displayAmount( double amount, Unit* units, unitScale scale )
{
   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   Unit* last = 0;

   QString ret;

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || units->getUnitType() != _type)
      return QString("%L1").arg(amount, fieldWidth, format, precision);

   if ( scaleToUnit.empty() )
      loadMap();

   if ( scaleToUnit.contains(scale) )
   {
      Unit* bob = scaleToUnit.value(scale);
      return QString("%L1 %2").arg(bob->fromSI(SIAmount), fieldWidth, format, precision).arg(bob->getUnitName());
   }

   foreach( unitScale key, scaleToUnit.keys() )
   {
      Unit* bob = scaleToUnit.value(key);
      double boundary = bob->boundary();

      if ( last && absSIAmount < bob->toSI(boundary) )
         return QString("%L1 %2").arg(last->fromSI(SIAmount), fieldWidth, format, precision).arg(last->getUnitName());

      last = bob;
   }
   // If we get here, use the largest unit available
   return QString("%L1 %2").arg(last->fromSI(SIAmount), fieldWidth, format, precision).arg(last->getUnitName());

}
