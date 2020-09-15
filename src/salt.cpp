/*
 * salt.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - mik fml firestone
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
#include <QDomElement>
#include <QDomText>
#include <QObject>
#include <QDebug>

#include "salt.h"
#include "brewtarget.h"
#include "TableSchemaConst.h"
#include "SaltSchema.h"

bool operator<(const Salt &s1, const Salt &s2)
{
   return s1.m_add_to < s2.m_add_to;
}

bool operator==(const Salt &s1, const Salt &s2)
{
   return s1.m_add_to == s2.m_add_to;
}

QString Salt::classNameStr()
{
   static const QString name("Salt");
   return name;
}

Salt::Salt(Brewtarget::DBTable table, int key)
   : Ingredient(table, key),
   m_amount(0.0),
   m_add_to(NEVER),
   m_type(NONE),
   m_amount_is_weight(true),
   m_percent_acid(0.0),
   m_is_acid(false),
   m_misc_id(-1),
   m_cacheOnly(false)
{
}

Salt::Salt(QString name, bool cache)
   : Ingredient(Brewtarget::SALTTABLE, -1, name, true),
   m_amount(0.0),
   m_add_to(NEVER),
   m_type(NONE),
   m_amount_is_weight(true),
   m_percent_acid(0.0),
   m_is_acid(false),
   m_misc_id(-1),
   m_cacheOnly(cache)
{
}

Salt::Salt(Salt & other)
   : Ingredient(Brewtarget::SALTTABLE, -1, other.name(), true),
   m_amount(other.m_amount),
   m_add_to(other.m_add_to),
   m_type(other.m_type),
   m_amount_is_weight(other.m_amount_is_weight),
   m_percent_acid(other.m_percent_acid),
   m_is_acid(other.m_is_acid),
   m_misc_id(other.m_misc_id),
   m_cacheOnly(other.m_cacheOnly)
{
}

Salt::Salt(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : Ingredient(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool()),
   m_amount(rec.value(kcolAmount).toDouble()),
   m_add_to(static_cast<Salt::WhenToAdd>(rec.value(kcolSaltAddTo).toInt())),
   m_type(static_cast<Salt::Types>(rec.value(kcolSaltType).toInt())),
   m_amount_is_weight(rec.value(kcolSaltAmtIsWgt).toBool()),
   m_percent_acid(rec.value(kcolSaltPctAcid).toDouble()),
   m_is_acid(rec.value(kcolSaltIsAcid).toBool()),
   m_misc_id(rec.value(kcolMiscId).toInt()),
   m_cacheOnly(false)
{
}

//================================"SET" METHODS=================================
void Salt::setAmount( double var )
{
   m_amount = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAmount, var);
   }
}

void Salt::setAddTo( Salt::WhenToAdd var )
{
   if ( var < NEVER || var > EQUAL ) {
      return;
   }

   m_add_to = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAddTo, var);
   }
}

// This may come to haunt me, but I am setting the isAcid flag and the
// amount_is_weight flags here.
void Salt::setType(Salt::Types type)
{
   if ( type < NONE || type > ACIDMLT ) {
      return;
   }

   m_type = type;
   m_is_acid = (type > NAHCO3);
   m_amount_is_weight = ! (type == LACTIC || type == H3PO4);

   if ( ! m_cacheOnly ) {
      setEasy(kpropType, type);
      setEasy(kpropIsAcid, m_is_acid);
      setEasy(kpropAmtIsWgt, m_amount_is_weight);
   }
}

void Salt::setAmountIsWeight( bool var )
{
   m_amount_is_weight = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAmtIsWgt, var);
   }
}

void Salt::setIsAcid( bool var )
{
   m_is_acid = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropIsAcid, var);
   }
}

void Salt::setPercentAcid(double var)
{
   m_percent_acid = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropPctAcid, var);
   }
}
void Salt::setCacheOnly(bool cache) { m_cacheOnly = cache; }

//=========================="GET" METHODS=======================================
double Salt::amount() const { return m_amount; }
Salt::WhenToAdd Salt::addTo() const { return m_add_to; }
Salt::Types Salt::type() const { return m_type; }
bool Salt::cacheOnly() const { return m_cacheOnly; }
int Salt::miscId() const { return m_misc_id; }
bool Salt::isAcid() const { return m_is_acid; }
bool Salt::amountIsWeight() const { return m_amount_is_weight; }
double Salt::percentAcid() const { return m_percent_acid; }

//====== maths ===========
// All of these the per gram, per liter
// these values are taken from Bru'n Water's execellent water knowledge page
// https://sites.google.com/site/brunwater/water-knowledge
// the numbers are derived by dividing the molecular weight of the ion by the
// molecular weight of the molecule in grams and then multiplying by 1000 to
// mg
// eg:
//    NaHCO3 84 g/mol
//       Na provides    23 g/mol
//       HCO3 provides  61 g/mol (ish)
//     So 1 g of NaHCO3 in 1L of water provides 1000*(61/84) = 726 ppm HCO3
//
// the magic 1000 is here because masses are stored as kg. We need it in grams
// for this part
double Salt::Ca() const
{
   if ( m_add_to == Salt::NEVER ) {
      return 0.0;
   }

   switch (m_type) {
      case Salt::CACL2: return 272.0 * m_amount * 1000.0;
      case Salt::CACO3: return 200.0 * m_amount * 1000.0;
      case Salt::CASO4: return 232.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::Cl() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::CACL2: return 483 * m_amount * 1000.0;
      case Salt::NACL: return 607 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::CO3() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::CACO3 ? 610.0  * m_amount * 1000.0: 0.0;
}

double Salt::HCO3() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::NAHCO3 ? 726.0 * m_amount * 1000.0: 0.0;
}

double Salt::Mg() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   return m_type == Salt::MGSO4 ? 99.0 * m_amount * 1000.0: 0.0;
}

double Salt::Na() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::NACL: return 393.0 * m_amount * 1000.0;
      case Salt::NAHCO3: return 274.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}

double Salt::SO4() const
{
   if ( m_add_to == Salt::NEVER )
      return 0.0;
   switch (m_type) {
      case Salt::CASO4: return 558.0 * m_amount * 1000.0;
      case Salt::MGSO4: return 389.0 * m_amount * 1000.0;
      default: return 0.0;
   }
}
