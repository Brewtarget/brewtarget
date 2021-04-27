/*
 * ColorMethods.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "ColorMethods.h"
#include "brewtarget.h"
#include <cmath>
#include <QString>
#include <QObject>

ColorMethods::ColorMethods()
{
}

ColorMethods::~ColorMethods()
{
}

double ColorMethods::mcuToSrm(double mcu)
{
   switch( Brewtarget::colorFormula )
   {
      case Brewtarget::MOREY:
         return morey(mcu);
      case Brewtarget::DANIEL:
         return daniel(mcu);
      case Brewtarget::MOSHER:
         return mosher(mcu);
      default:
         qCritical() << QObject::tr("Invalid color formula type: %1").arg(Brewtarget::colorFormula);
         return morey(mcu);
   }
}

// I don't know where this is from.
double ColorMethods::morey(double mcu)
{
   return 1.4922 * pow( mcu, 0.6859 );
}

// From Palmer's "How to Brew"
double ColorMethods::daniel(double mcu)
{
   return 0.2 * mcu + 8.4;
}

// From Palmer's "How to Brew"
double ColorMethods::mosher(double mcu)
{
   return 0.3 * mcu + 4.7;
}
