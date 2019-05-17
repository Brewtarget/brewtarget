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

Water::Water(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key)
{
   _amount_l = rec.value(kAmount).toDouble();
   _calcium_ppm = rec.value(kCalcium).toDouble();
   _bicarbonate_ppm = rec.value(kBiCarbonate).toDouble();
   _sulfate_ppm = rec.value(kSulfate).toDouble();
   _chloride_ppm = rec.value(kChloride).toDouble();
   _sodium_ppm = rec.value(kSodium).toDouble();
   _magnesium_ppm = rec.value(kMagnesium).toDouble();
   _ph = rec.value(kPh).toDouble();
   _notes = rec.value(kAmount).toString();
}

//================================"SET" METHODS=================================
void Water::setAmount_l( double var )
{
   _amount_l = var;
   set(kAmountProp, kAmount, var);
}

void Water::setCalcium_ppm( double var )
{
   _calcium_ppm = var;
   set(kCalciumProp, kCalcium, var);
}

void Water::setBicarbonate_ppm( double var )
{
   _bicarbonate_ppm = var;
   set(kBiCarbonateProp, kBiCarbonate, var);
}

void Water::setChloride_ppm( double var )
{
   _chloride_ppm = var;
   set(kChlorideProp, kChloride, var);
}

void Water::setSodium_ppm( double var )
{
   _sodium_ppm = var;
   set(kSodiumProp, kSodium, var);
}

void Water::setMagnesium_ppm( double var )
{
   _magnesium_ppm = var;
   set(kMagnesiumProp, kMagnesium, var);
}

void Water::setPh( double var )
{
   _ph = var;
   set(kPhProp, kPh, var);
}

void Water::setSulfate_ppm( double var )
{
   _sulfate_ppm = var;
   set(kSulfateProp, kSulfate, var);
}

void Water::setNotes( const QString &var )
{
   _notes = var;
   set(kNotesProp, kNotes, var);
}

//=========================="GET" METHODS=======================================
QString Water::notes() const { return _notes; }

double Water::sulfate_ppm() const { return _sulfate_ppm; }

double Water::amount_l() const { return _amount_l; }

double Water::calcium_ppm() const { return _calcium_ppm; }

double Water::bicarbonate_ppm() const { return _bicarbonate_ppm; }

double Water::chloride_ppm() const { return _chloride_ppm; }

double Water::sodium_ppm() const { return _sodium_ppm; }

double Water::magnesium_ppm() const { return _magnesium_ppm; }

double Water::ph() const { return _ph; }
