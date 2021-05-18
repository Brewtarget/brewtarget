/*
 * IbuMethods.cpp is part of Brewtarget, and is Copyright the following
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

#include "IbuMethods.h"
#include <cmath>
#include "Algorithms.h"
#include "brewtarget.h"
#include <QString>
#include <QObject>

IbuMethods::IbuMethods()
{
}

IbuMethods::~IbuMethods()
{
}

double IbuMethods::getIbus(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes)
{
   switch( Brewtarget::ibuFormula )
   {
      case Brewtarget::TINSETH:
         return tinseth(AArating, hops_grams, finalVolume_liters, wort_grav, minutes);
         break;
      case Brewtarget::RAGER:
         return rager(AArating, hops_grams, finalVolume_liters, wort_grav, minutes);
         break;
      case Brewtarget::NOONAN:
         return noonan(AArating, hops_grams, finalVolume_liters, wort_grav, minutes);
         break;
      default:
         qCritical() << QObject::tr("Unrecognized IBU formula type. %1").arg(Brewtarget::ibuFormula);
         return tinseth(AArating, hops_grams, finalVolume_liters, wort_grav, minutes);
         break;
   }
}

// These are collected from http://www.realbeer.com/hops/FAQ.html

double IbuMethods::tinseth(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes)
{
   return ((AArating * hops_grams * 1000) / finalVolume_liters) * ((1.0 - exp(-0.04 * minutes))/4.15) * (1.65 * pow(0.000125, (wort_grav - 1)));
}

double IbuMethods::rager(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes)
{
   double utilization = (18.11 + 13.86*tanh((minutes-31.32)/18.17)) / 100.0;

   double gravityFactor = (wort_grav > 1.050)? (wort_grav - 1.050)/0.2 : 0.0;

   return (hops_grams*utilization*AArating*1000)/(finalVolume_liters*(1+gravityFactor));
}

double IbuMethods::noonan(double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes)
{
    double volumeFactor = (Units::us_gallons.toSI(5.0))/ finalVolume_liters;
    double hopsFactor = hops_grams/ (Units::ounces.toSI(1.0) * 1000.0);
    static Polynomial p(Polynomial() << 0.7000029428 << -0.08868853463 << 0.02720809386 << -0.002340415323 << 0.00009925450081 << -0.000002102006144 << 0.00000002132644293 << -0.00000000008229488217);

    //using 60 minutes as a general table
    double utilizationFactorTable[4][2] =  {
                     {1.050, 1},
                     {1.065, 0.9286},
                     {1.085, 0.8571},
                     {1.100, 0.75}
                    };

    double utilizationFactor;

    if(wort_grav <= utilizationFactorTable[0][0])
    {
        utilizationFactor = utilizationFactorTable[0][1];
    }
    else if(wort_grav <= utilizationFactorTable[1][0])
    {
        utilizationFactor = utilizationFactorTable[1][1];
    }
    else if(wort_grav <= utilizationFactorTable[2][0])
    {
        utilizationFactor = utilizationFactorTable[2][1];
    }
    else
    {
        utilizationFactor = utilizationFactorTable[3][1];
    }

    return(volumeFactor * ( hopsFactor * (100 * AArating) * p.eval(minutes) ) * utilizationFactor);
}
