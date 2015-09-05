/*
 * water.cpp is part of Brewtarget, and is Copyright the following
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

#include <QVector>
#include "water.h"
#include "brewtarget.h"
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include "water.h"
#include "brewtarget.h"

QHash<QString,QString> Water::tagToProp = Water::tagToPropHash();

QHash<QString,QString> Water::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = "name";
   propHash["AMOUNT"] = "amount_l";
   propHash["CALCIUM"] = "calcium_ppm";
   propHash["BICARBONATE"] = "bicarbonate_ppm";
   propHash["SULFATE"] = "sulfate_ppm";
   propHash["CHLORIDE"] = "chloride_ppm";
   propHash["SODIUM"] = "sodium_ppm";
   propHash["MAGNESIUM"] = "magnesium_ppm";
   propHash["PH"] = "ph";
   propHash["NOTES"] = "notes";
   return propHash;
}

bool operator<(Water &w1, Water &w2)
{
   return w1.name() < w2.name();
}

bool operator==(Water &w1, Water &w2)
{
   return w1.name() == w2.name();
}

/*
void Water::setDefaults()
{
   name = "";
   amount_l = 0.0;
   calcium_ppm = 0.0;
   bicarbonate_ppm = 0.0;
   chloride_ppm = 0.0;
   sodium_ppm = 0.0;
   magnesium_ppm = 0.0;
   ph = 7.0;
   notes = "";
}
*/

Water::Water()
   : BeerXMLElement()
{
}

//================================"SET" METHODS=================================
void Water::setName( const QString &var )
{
   set("name", "name", var);
   emit changedName(var);
}

void Water::setAmount_l( double var )
{
   set("amount_l", "amount", var);
}

void Water::setCalcium_ppm( double var )
{
   set("calcium_ppm", "calcium", var);
}

void Water::setBicarbonate_ppm( double var )
{
   set("bicarbonate_ppm", "bicarbonate", var);
}

void Water::setChloride_ppm( double var )
{
   set("chloride_ppm", "chloride", var);
}

void Water::setSodium_ppm( double var )
{
   set("sodium_ppm", "sodium", var);
}

void Water::setMagnesium_ppm( double var )
{
   set("magnesium_ppm", "magnesium", var);
}

void Water::setPh( double var )
{
   set("ph", "ph", var);
}

void Water::setSulfate_ppm( double var )
{
   set("sulfate_ppm", "sulfate", var);
}

void Water::setNotes( const QString &var )
{
   set("notes", "notes", var);
}

//=========================="GET" METHODS=======================================
QString Water::name() const { return get("name").toString(); }
QString Water::notes() const { return get("notes").toString(); }

double Water::sulfate_ppm() const { return get("sulfate").toDouble(); }
double Water::amount_l() const { return get("amount").toDouble(); }
double Water::calcium_ppm() const { return get("calcium").toDouble(); }
double Water::bicarbonate_ppm() const { return get("bicarbonate").toDouble(); }
double Water::chloride_ppm() const { return get("chloride").toDouble(); }
double Water::sodium_ppm() const { return get("sodium").toDouble(); }
double Water::magnesium_ppm() const { return get("magnesium").toDouble(); }
double Water::ph() const { return get("ph").toDouble(); }
