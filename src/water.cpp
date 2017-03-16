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

/************* Columns *************/
const QString kName("name");
const QString kAmount("amount");
const QString kCalcium("calcium");
const QString kBiCarbonate("bicarbonate");
const QString kSulfate("sulfate");
const QString kChloride("chloride");
const QString kSodium("sodium");
const QString kMagnesium("magnesium");
const QString kPh("ph");
const QString kNotes("notes");

/************** Props **************/
const QString kNameProp("name");
const QString kAmountProp("amount_l");
const QString kCalciumProp("calcium_ppm");
const QString kBiCarbonateProp("bicarbonate_ppm");
const QString kSulfateProp("sulfate_ppm");
const QString kChlorideProp("chloride_ppm");
const QString kSodiumProp("sodium_ppm");
const QString kMagnesiumProp("magnesium_ppm");
const QString kPhProp("ph");
const QString kNotesProp("notes");


QHash<QString,QString> Water::tagToProp = Water::tagToPropHash();

QHash<QString,QString> Water::tagToPropHash()
{
   QHash<QString,QString> propHash;
   propHash["NAME"] = kNameProp;
   propHash["AMOUNT"] = kAmountProp;
   propHash["CALCIUM"] = kCalciumProp;
   propHash["BICARBONATE"] = kBiCarbonateProp;
   propHash["SULFATE"] = kSulfateProp;
   propHash["CHLORIDE"] = kChlorideProp;
   propHash["SODIUM"] = kSodiumProp;
   propHash["MAGNESIUM"] = kMagnesiumProp;
   propHash["PH"] = kPhProp;
   propHash["NOTES"] = kNotesProp;
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

QString Water::classNameStr()
{
   static const QString name("Water");
   return name;
}

Water::Water(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key)
{
}

//================================"SET" METHODS=================================
void Water::setAmount_l( double var )
{
   set(kAmountProp, kAmount, var);
}

void Water::setCalcium_ppm( double var )
{
   set(kCalciumProp, kCalcium, var);
}

void Water::setBicarbonate_ppm( double var )
{
   set(kBiCarbonateProp, kBiCarbonate, var);
}

void Water::setChloride_ppm( double var )
{
   set(kChlorideProp, kChloride, var);
}

void Water::setSodium_ppm( double var )
{
   set(kSodiumProp, kSodium, var);
}

void Water::setMagnesium_ppm( double var )
{
   set(kMagnesiumProp, kMagnesium, var);
}

void Water::setPh( double var )
{
   set(kPhProp, kPh, var);
}

void Water::setSulfate_ppm( double var )
{
   set(kSulfateProp, kSulfate, var);
}

void Water::setNotes( const QString &var )
{
   set(kNotesProp, kNotes, var);
}

//=========================="GET" METHODS=======================================
QString Water::notes() const
{
   return get(kNotes).toString();
}

double Water::sulfate_ppm() const
{
   return get(kSulfate).toDouble();
}

double Water::amount_l() const
{
   return get(kAmount).toDouble();
}

double Water::calcium_ppm() const
{
   return get(kCalcium).toDouble();
}

double Water::bicarbonate_ppm() const
{
   return get(kBiCarbonate).toDouble();
}

double Water::chloride_ppm() const
{
   return get(kChloride).toDouble();
}

double Water::sodium_ppm() const
{
   return get(kSodium).toDouble();
}

double Water::magnesium_ppm() const
{
   return get(kMagnesium).toDouble();
}

double Water::ph() const
{
   return get(kPh).toDouble();
}
