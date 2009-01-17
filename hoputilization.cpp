/*
    Copyright Philip Greggory Lee (rocketman768@gmail.com), 2008-2009.
    
    hoputilization.cpp is part of Brewtarget.
    
    Brewtarget is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Brewtarget is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Brewtarget.  If not, see <http://www.gnu.org/licenses/>.
*/

// Uses a method outlined at http://www.realbeer.com/hops/research.html

#include <cmath>

double BoilTimeFactor( double minutes )
{
   return (1.0 - exp(-0.04 * minutes))/4.15;
}

double BignessFactor( double wort_grav )
{
   return 1.65 * pow(0.000125, (wort_grav - 1));
}

double AlphaAcidUtilization( double wort_grav, double minutes )
{
   return BoilTimeFactor(minutes) * BignessFactor(wort_grav);
}

double MaxAAConcentration_mgPerLiter(double AArating, double hops_grams, double finalVolume_liters)
{
   return (AArating * hops_grams * 1000) / finalVolume_liters;
}

double IBU( double AArating, double hops_grams, double finalVolume_liters, double wort_grav, double minutes)
{
   return MaxAAConcentration_mgPerLiter(AArating, hops_grams, finalVolume_liters) * AlphaAcidUtilization(wort_grav, minutes);
}

