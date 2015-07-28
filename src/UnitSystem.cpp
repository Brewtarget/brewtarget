/*
 * UnitSystem.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2015
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
#include "brewtarget.h"
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
   double amt = 0.0;
   Unit* u = defUnit;
   Unit* found = 0;

   // make sure we can parse the string
   if (amtUnit.indexIn(qstr) == -1)
   {
      return 0.0;
   }

   amt = Brewtarget::toDouble( amtUnit.cap(1), "UnitSystem::qstringToSI()");

   QString unit = amtUnit.cap(2);

   // Look first in this unit system. If you can't find it here, find it
   // globally. I *think* this finally has all the weird magic right. If the
   // field is marked as "Imperial" and you enter "3 qt" you get 3 imperial
   // qts, 3.6 US qts, 3.41L. If you enter 3L, you get 2.64 imperial qts,
   // 3.17 US qt. If you mean 3 US qt, you are SOL unless you mark the field
   // as US Customary.
   found = qstringToUnit().value(unit);
   if ( ! found )
      found = Unit::getUnit(unit,false);

   // If the calling method isn't overriding the search and we actually found
   // something, use it
   if ( ! force && found )
   {
      u = found;
   }

   // It is possible for u to be NULL at this point, so make sure we handle
   // that case
   if ( u == 0 )
   {
      return -1.0;
   }

   return u->toSI(amt);
}

QString UnitSystem::displayAmount( double amount, Unit* units, int precision, Unit::unitScale scale )
{
   // If the precision is not specified, we take the default one
   if( precision < 0)
   {
      precision = this->precision;
   }

   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || units->getUnitType() != _type)
      return QString("%L1").arg(amount, fieldWidth, format, precision);

   // We really shouldn't ever reference something that could be null until
   // after we have verified it isn't.
   double SIAmount    = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   Unit* last = 0;

   // Don't loop if the 'without' key is defined
   if ( scaleToUnit().contains(Unit::scaleWithout) )
      scale = Unit::scaleWithout;

   // If a specific scale is provided, just use that and don't loop.
   if ( scaleToUnit().contains(scale) )
   {
      Unit* bob = scaleToUnit().value(scale);
      return QString("%L1 %2").arg(bob->fromSI(SIAmount), fieldWidth, format, precision).arg(bob->getUnitName());
   }

   // scaleToUnit() is a QMap which means we loop in the  order in which the
   // items were inserted. Order counts, and this map has to be
   // created from smallest to largest scale (e.g., mg, g, kg).
   QMap<Unit::unitScale, Unit*>::const_iterator it;
   for( it = scaleToUnit().begin(); it != scaleToUnit().end(); ++it)
   {
      Unit* bob = it.value();
      double boundary = bob->boundary();

      // This is a nice bit of work, if I may say so myself. If we've been
      // through the loop at least once already, and the boundary condition is
      // met, use the Unit* from the last loop.
      if ( last && absSIAmount < bob->toSI(boundary) )
         return QString("%L1 %2").arg(last->fromSI(SIAmount), fieldWidth, format, precision).arg(last->getUnitName());

      // If we get all the way through the map, this will be the largest unit
      // available
      last = bob;
   }

   // If we get here, use the largest unit available
   if( last )
      return QString("%L1 %2").arg(last->fromSI(SIAmount), fieldWidth, format, precision).arg(last->getUnitName());
   else
      return QString("nounit"); // Should never happen, so be obvious if it does

}

double UnitSystem::amountDisplay( double amount, Unit* units, Unit::unitScale scale )
{
   // Special cases. Make sure the unit isn't null and that we're
   // dealing with volume.
   if( units == 0 || units->getUnitType() != _type)
      return amount;

   double SIAmount = units->toSI( amount );
   double absSIAmount = qAbs(SIAmount);
   Unit* last = 0;

   // Short circuit if the 'without' key is defined
   if ( scaleToUnit().contains(Unit::scaleWithout) )
      scale = Unit::scaleWithout;

   if ( scaleToUnit().contains(scale) )
   {
      Unit* bob = scaleToUnit().value(scale);
      return bob->fromSI(SIAmount);
   }

   QMap<Unit::unitScale, Unit*>::const_iterator it;
   for( it = scaleToUnit().begin(); it != scaleToUnit().end(); ++it)
   {
      Unit* bob = it.value();
      double boundary = bob->boundary();

      if ( last && absSIAmount < bob->toSI(boundary) )
         return last->fromSI(SIAmount);

      last = bob;
   }
   // If we get here, use the largest unit available
   if( last )
      return last->fromSI(SIAmount);
   else
      return -42.42; // Should never happen, so be obvious if it does

}

Unit* UnitSystem::scaleUnit(Unit::unitScale scale) { return scaleToUnit().contains(scale) ?  scaleToUnit().value(scale) : 0; }
