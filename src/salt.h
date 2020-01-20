/*
 * salt.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#ifndef _SALT_H
#define _SALT_H

#include <QString>
#include "BeerXMLElement.h"

// Forward declarations.
class Salt;
bool operator<(Salt &w1, Salt &w2);
bool operator==(Salt &w1, Salt &w2);

/*!
 * \class Salt
 * \author Philip G. Lee
 *
 * \brief Model for salt records in the database.
 */
class Salt : public BeerXMLElement
{
   Q_OBJECT
   Q_CLASSINFO("signal", "salts")

   friend class Database;
   friend class WaterDialog;
public:

   virtual ~Salt() {}

   enum WhenToAdd {
      NEVER,
      MASH,
      SPARGE
   };
   Q_ENUM(WhenToAdd)

   enum Types {
      NONE,
      CACL2,
      CACO3,
      CASO4,
      MGSO4,
      NACL,
      NAHCO3
   };
   Q_ENUM(Types)

   // On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out
   //! \brief The amount of salt to be added (always a weight)
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount_l*/ )
   //! \brief When to add the salt (mash or sparge)
   Q_PROPERTY( Salt::WhenToAdd addTo READ addTo WRITE setAddTo /*NOTIFY changed*/ /*changedCalcium_ppm*/ )
   //! \brief What kind of salt this is
   Q_PROPERTY( Salt::Types type READ type WRITE setType /*NOTIFY changed*/ /*changedBicarbonate_ppm*/ )
   //! \brief A link to the salt in the MISC table. Not sure I'm going to use this
   Q_PROPERTY( int miscId READ miscId /* WRITE setMiscId*/ /*NOTIFY changed*/ /*changedSulfate_ppm*/ )
   //! \brief To cache or not to cache
   Q_PROPERTY( bool cacheOnly READ cacheOnly WRITE setCacheOnly /*NOTIFY changed*/ /*changedSulfate_ppm*/ )

   double amount() const;
   Salt::WhenToAdd addTo() const;
   Salt::Types type() const;
   int miscId() const;
   bool cacheOnly() const;

   void setAmount( double var );
   void setAddTo( Salt::WhenToAdd var );
   void setType( Salt::Types var );
   void setCacheOnly( bool var );

   static QString classNameStr();

signals:

   //! \brief Emitted when \c name() changes.
   void changedName(QString);

private:
   Salt(Brewtarget::DBTable table, int key);
   Salt(Brewtarget::DBTable table, int key, QSqlRecord rec);
   Salt(Salt const& other, bool cache = true);
   Salt(QString name, bool cache = true);

   double m_amount;
   Salt::WhenToAdd m_add_to;
   Salt::Types m_type;
   int m_misc_id;
   bool m_cacheOnly;

};

Q_DECLARE_METATYPE( QList<Salt*> )

inline bool SaltPtrLt( Salt* lhs, Salt* rhs)
{
   return *lhs < *rhs;
}

inline bool SaltPtrEq( Salt* lhs, Salt* rhs)
{
   return *lhs == *rhs;
}

struct Salt_ptr_cmp
{
   bool operator()( Salt* lhs, Salt* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Salt_ptr_equals
{
   bool operator()( Salt* lhs, Salt* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif   /* _SALT_H */

