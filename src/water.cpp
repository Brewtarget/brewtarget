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

// these are defined in the parent, but I need them here too
const QString kDeleted("deleted");
const QString kDisplay("display");
const QString kFolder("folder");
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
   : BeerXMLElement(table, key),
   m_amount_l(0.0),
   m_calcium_ppm(0.0),
   m_bicarbonate_ppm(0.0),
   m_sulfate_ppm(0.0),
   m_chloride_ppm(0.0),
   m_sodium_ppm(0.0),
   m_magnesium_ppm(0.0),
   m_ph(0.0),
   m_notes(QString()),
   m_cacheOnly(false)
{
}

Water::Water(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kName).toString(), rec.value(kDisplay).toBool()),
   m_amount_l(rec.value(kAmount).toDouble()),
   m_calcium_ppm(rec.value(kCalcium).toDouble()),
   m_bicarbonate_ppm(rec.value(kBiCarbonate).toDouble()),
   m_sulfate_ppm(rec.value(kSulfate).toDouble()),
   m_chloride_ppm(rec.value(kChloride).toDouble()),
   m_sodium_ppm(rec.value(kSodium).toDouble()),
   m_magnesium_ppm(rec.value(kMagnesium).toDouble()),
   m_ph(rec.value(kPh).toDouble()),
   m_notes(rec.value(kAmount).toString()),
   m_cacheOnly(false)
{
}

//================================"SET" METHODS=================================
void Water::setAmount_l( double var )
{
   m_amount_l = var;
   if ( ! m_cacheOnly ) {
      set(kAmountProp, kAmount, var);
   }
}

void Water::setCalcium_ppm( double var )
{
   m_calcium_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kCalciumProp, kCalcium, var);
   }
}

void Water::setBicarbonate_ppm( double var )
{
   m_bicarbonate_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kBiCarbonateProp, kBiCarbonate, var);
   }
}

void Water::setChloride_ppm( double var )
{
   m_chloride_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kChlorideProp, kChloride, var);
   }
}

void Water::setSodium_ppm( double var )
{
   m_sodium_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kSodiumProp, kSodium, var);
   }
}

void Water::setMagnesium_ppm( double var )
{
   m_magnesium_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kMagnesiumProp, kMagnesium, var);
   }
}

void Water::setPh( double var )
{
   m_ph = var;
   if ( ! m_cacheOnly ) {
      set(kPhProp, kPh, var);
   }
}

void Water::setSulfate_ppm( double var )
{
   m_sulfate_ppm = var;
   if ( ! m_cacheOnly ) {
      set(kSulfateProp, kSulfate, var);
   }
}

void Water::setNotes( const QString &var )
{
   m_notes = var;
   if ( ! m_cacheOnly ) {
      set(kNotesProp, kNotes, var);
   }
}

void Water::setCacheOnly(bool cache) { m_cacheOnly = cache; }
//=========================="GET" METHODS=======================================
QString Water::notes() const { return m_notes; }

double Water::sulfate_ppm() const { return m_sulfate_ppm; }

double Water::amount_l() const { return m_amount_l; }

double Water::calcium_ppm() const { return m_calcium_ppm; }

double Water::bicarbonate_ppm() const { return m_bicarbonate_ppm; }

double Water::chloride_ppm() const { return m_chloride_ppm; }

double Water::sodium_ppm() const { return m_sodium_ppm; }

double Water::magnesium_ppm() const { return m_magnesium_ppm; }

double Water::ph() const { return m_ph; }

bool Water::cacheOnly() const { return m_cacheOnly; }
