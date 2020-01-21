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

#include "salt.h"
#include "brewtarget.h"
#include "TableSchemaConst.h"
#include "SaltSchema.h"

bool operator<(Salt &w1, Salt &w2)
{
   return w1.name() < w2.name();
}

bool operator==(Salt &w1, Salt &w2)
{
   return w1.name() == w2.name();
}

QString Salt::classNameStr()
{
   static const QString name("Salt");
   return name;
}

Salt::Salt(Brewtarget::DBTable table, int key)
   : BeerXMLElement(table, key),
   m_amount(0.0),
   m_add_to(NEVER),
   m_type(NONE),
   m_misc_id(-1),
   m_cacheOnly(false)
{
}

Salt::Salt(QString name, bool cache)
   : BeerXMLElement(Brewtarget::SALTTABLE, -1, name, true),
   m_amount(0.0),
   m_add_to(NEVER),
   m_type(NONE),
   m_misc_id(-1),
   m_cacheOnly(cache)
{
}

Salt::Salt(Salt const& other)
   : BeerXMLElement(Brewtarget::SALTTABLE, -1, other.name(), true),
   m_amount(other.m_amount),
   m_add_to(other.m_add_to),
   m_type(other.m_type),
   m_misc_id(other.m_misc_id),
   m_cacheOnly(other.m_cacheOnly)
{
}

Salt::Salt(Brewtarget::DBTable table, int key, QSqlRecord rec)
   : BeerXMLElement(table, key, rec.value(kcolName).toString(), rec.value(kcolDisplay).toBool()),
   m_amount(rec.value(kcolAmount).toDouble()),
   m_add_to(static_cast<Salt::WhenToAdd>(rec.value(kcolSaltAddTo).toInt())),
   m_type(static_cast<Salt::Types>(rec.value(kcolSaltType).toInt())),
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
   if ( var < NEVER || var > SPARGE ) {
      return;
   }

   m_add_to = var;
   if ( ! m_cacheOnly ) {
      setEasy(kpropAddTo, var);
   }
}

void Salt::setType(Salt::Types type)
{
   if ( type < NONE || type > NAHCO3 ) {
      return;
   }

   m_type = type;
   if ( ! m_cacheOnly ) {
      setEasy(kpropType, type);
   }
}

void Salt::setCacheOnly(bool cache) { m_cacheOnly = cache; }

//=========================="GET" METHODS=======================================
double Salt::amount() const { return m_amount; }
Salt::WhenToAdd Salt::addTo() const { return m_add_to; }
Salt::Types Salt::type() const { return m_type; }
bool Salt::cacheOnly() const { return m_cacheOnly; }
int Salt::miscId() const { return m_misc_id; }
