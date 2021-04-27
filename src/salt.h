/*
 * salt.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#include "model/NamedEntity.h"
namespace PropertyNames::Salt { static char const * const amount = "amount"; /* previously kpropAmount */ }
namespace PropertyNames::Salt { static char const * const amountIsWeight = "amountIsWeight"; /* previously kpropAmtIsWgt */ }
namespace PropertyNames::Salt { static char const * const type = "type"; /* previously kpropType */ }
namespace PropertyNames::Salt { static char const * const isAcid = "isAcid"; /* previously kpropIsAcid */ }
namespace PropertyNames::Salt { static char const * const percentAcid = "percentAcid"; /* previously kpropPctAcid */ }
namespace PropertyNames::Salt { static char const * const addTo = "addTo"; /* previously kpropAddTo */ }

namespace PropertyNames::Salt { static char const * const misc_id = "misc_id"; /* previously kcolMiscId */ }

/*!
 * \class Salt
 * \author Mik Firestone
 *
 * \brief Model for salt records in the database.
 */
class Salt : public NamedEntity
{
   Q_OBJECT
   Q_CLASSINFO("signal", "salts")

   friend class Database;
   friend class BeerXML;
   friend class WaterDialog;
   friend class SaltTableModel;
public:

   enum WhenToAdd {
      NEVER,
      MASH,
      SPARGE,
      RATIO,
      EQUAL
   };

   enum Types {
      NONE,
      CACL2,
      CACO3,
      CASO4,
      MGSO4,
      NACL,
      NAHCO3,
      LACTIC,
      H3PO4,
      ACIDMLT,
      numTypes
   };

   Q_ENUMS(WhenToAdd Types)

   Salt(QString name, bool cache = true);
   virtual ~Salt() {}

   // On a base or target profile, bicarbonate and alkalinity cannot both be used. I'm gonna have fun figuring that out
   //! \brief The amount of salt to be added (always a weight)
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount_l*/ )
   //! \brief When to add the salt (mash or sparge)
   Q_PROPERTY( Salt::WhenToAdd addTo READ addTo WRITE setAddTo /*NOTIFY changed*/ /*changedCalcium_ppm*/ )
   //! \brief What kind of salt this is
   Q_PROPERTY( Salt::Types type READ type WRITE setType /*NOTIFY changed*/ /*changedBicarbonate_ppm*/ )
   //! \brief Is this a weight (like CaCO3) or a volume (like H3PO3)
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight /*NOTIFY changed*/ /*changedAmountIsWeight*/ )
   //! \brief What percent is acid (used for lactic acid, H3PO4 and acid malts)
   Q_PROPERTY( double percentAcid READ percentAcid WRITE setPercentAcid /*NOTIFY changed*/ /*changedPercentAcid*/ )
   //! \brief Is this an acid or salt?
   Q_PROPERTY( bool isAcid READ isAcid WRITE setIsAcid /*NOTIFY changed*/ /*changedIsAcid*/ )
   //! \brief A link to the salt in the MISC table. Not sure I'm going to use this
   Q_PROPERTY( int miscId READ miscId /* WRITE setMiscId*/ /*NOTIFY changed*/ /*changedSulfate_ppm*/ )
   //! \brief To cache or not to cache
   Q_PROPERTY( bool cacheOnly READ cacheOnly WRITE setCacheOnly /*NOTIFY changed*/ /*changedSulfate_ppm*/ )

   double amount() const;
   Salt::WhenToAdd addTo() const;
   Salt::Types type() const;
   bool amountIsWeight() const;
   double percentAcid() const;
   bool isAcid() const;
   int miscId() const;
   bool cacheOnly() const;

   void setAmount( double var );
   void setAddTo( Salt::WhenToAdd var );
   void setType( Salt::Types var );
   void setAmountIsWeight( bool var );
   void setPercentAcid(double var);
   void setIsAcid( bool var );
   void setCacheOnly( bool var );

   static QString classNameStr();

   double Ca() const;
   double Cl() const;
   double CO3() const;
   double HCO3() const;
   double Mg() const;
   double Na() const;
   double SO4() const;

   // Salt objects do not have parents
   NamedEntity * getParent() { return nullptr; }
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

signals:

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
//   Salt(Brewtarget::DBTable table, int key);
   Salt(TableSchema* table, QSqlRecord rec, int t_key = -1);
   Salt(Salt & other );

   double m_amount;
   Salt::WhenToAdd m_add_to;
   Salt::Types m_type;
   bool m_amount_is_weight;
   double m_percent_acid;
   bool m_is_acid;
   int m_misc_id;
   bool m_cacheOnly;

};

Q_DECLARE_METATYPE( QList<Salt*> )

inline bool SaltPtrLt( Salt* lhs, Salt* rhs)
{
   return (lhs->type() < rhs->type() &&
           lhs->addTo() < rhs->addTo());
}

inline bool SaltPtrEq( Salt* lhs, Salt* rhs)
{
   return (lhs->type() == rhs->type() &&
           lhs->addTo() == rhs->addTo());

}

struct Salt_ptr_cmp
{
   bool operator()( Salt* lhs, Salt* rhs)
   {
      return ( lhs->type() < rhs->type() &&
           lhs->addTo() < rhs->addTo());
   }
};

struct Salt_ptr_equals
{
   bool operator()( Salt* lhs, Salt* rhs )
   {
      return ( lhs->type() == rhs->type() &&
           lhs->addTo() == rhs->addTo());
   }
};

#endif   /* _SALT_H */

