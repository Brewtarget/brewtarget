/*
 * model/Misc.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
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
#ifndef MODEL_MISC_H
#define MODEL_MISC_H

#include <QString>

#include "model/NamedEntity.h"

namespace PropertyNames::Misc { static char const * const amount = "amount"; /* previously kpropAmount */ }
namespace PropertyNames::Misc { static char const * const amountIsWeight = "amountIsWeight"; /* previously kpropAmtIsWgt */ }
namespace PropertyNames::Misc { static char const * const inventory = "inventory"; /* previously kpropInventory */ }
namespace PropertyNames::Misc { static char const * const inventory_id = "inventory_id"; /* previously kpropInventoryId */ }
namespace PropertyNames::Misc { static char const * const useString = "useString"; /* previously kpropUseString */ }
namespace PropertyNames::Misc { static char const * const use = "use"; /* previously kpropUse */ }
namespace PropertyNames::Misc { static char const * const typeString = "typeString"; /* previously kpropTypeString */ }
namespace PropertyNames::Misc { static char const * const type = "type"; /* previously kpropType */ }
namespace PropertyNames::Misc { static char const * const notes = "notes"; /* previously kpropNotes */ }
namespace PropertyNames::Misc { static char const * const time = "time"; /* previously kpropMiscTime */ }
namespace PropertyNames::Misc { static char const * const useFor = "useFor"; /* previously kpropUseFor */ }

/*!
 * \class Misc
 *
 * \brief Model for a misc record in the database.
 */
class Misc : public NamedEntity
{
   Q_OBJECT
   Q_CLASSINFO("signal", "miscs")

   friend class Database;
   friend class BeerXML;
   friend class MiscDialog;
public:

   //! \brief The type of ingredient.
   enum Type {Spice, Fining, Water_Agent, Herb, Flavor, Other};
   //! \brief Where the ingredient is used.
   enum Use { Boil, Mash, Primary, Secondary, Bottling };
   //! \brief What is the type of amount.
   enum AmountType { AmountType_Weight, AmountType_Volume };
   Q_ENUMS( Type Use AmountType )

   Misc(QString name, bool cache = true);
   virtual ~Misc() {}

   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The  \c Type string.
   Q_PROPERTY( QString typeString READ typeString /*NOTIFY changed*/ STORED false )
   //! \brief The translated \c Type string.
   Q_PROPERTY( QString typeStringTr READ typeStringTr /*NOTIFY changed*/ STORED false )
   //! \brief The \c Use.
   Q_PROPERTY( Use use READ use WRITE setUse /*NOTIFY changed*/ /*changedUse*/ )
   //! \brief The \c Use string.
   Q_PROPERTY( QString useString READ useString /*NOTIFY changed*/ /*changedUse*/ STORED false )
   //! \brief The translated \c Use string.
   Q_PROPERTY( QString useStringTr READ useStringTr /*NOTIFY changed*/ /*changedUse*/ STORED false )
   //! \brief The \c Amount type.
   Q_PROPERTY( AmountType amountType READ amountType WRITE setAmountType /*NOTIFY changed*/ /*changedAmountType*/ )
   //! \brief The \c Amount type string.
   Q_PROPERTY( QString amountTypeString READ amountTypeString /*NOTIFY changed*/ /*changedAmountType*/ STORED false )
   //! \brief The translated \c Use string.
   Q_PROPERTY( QString amountTypeStringTr READ amountTypeStringTr /*NOTIFY changed*/ /*changedAmountType*/ STORED false )
   //! \brief The time used in minutes.
   // .:TBD:. (MY 2020-01-03) This property name seems inconsistent with Hop (where we use time_min)
   Q_PROPERTY( double time READ time WRITE setTime /*NOTIFY changed*/ /*changedTime*/ )
   //! \brief The amount in either kg or L, depending on \c amountIsWeight().
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount*/ )
   //! \brief The amount in inventory in either kg or L, depending on \c amountIsWeight().
   Q_PROPERTY( double inventory READ inventory WRITE setInventoryAmount /*NOTIFY changed*/ /*changedAmount*/ )
   //! \brief The inventory id.
   Q_PROPERTY( double inventoryId READ inventoryId WRITE setInventoryId /*NOTIFY changed*/ /*changedAmount*/ )
   //! \brief Whether the amount is weight (kg), or volume (L).
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight /*NOTIFY changed*/ /*changedAmountIsWeight*/ )
   //! \brief What to use it for.
   Q_PROPERTY( QString useFor READ useFor WRITE setUseFor /*NOTIFY changed*/ /*changedUseFor*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )

   // Set
   void setType( Type t );
   void setUse( Use u );
   void setAmountType( AmountType t );
   void setAmount( double var );
   void setInventoryAmount( double var );
   void setTime( double var );
   void setAmountIsWeight( bool var );
   void setUseFor( const QString &var );
   void setNotes( const QString &var );
   void setCacheOnly( bool cache );
   void setInventoryId( int key );

   // Get
//   QString name() const;
   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;
   Use use() const;
   const QString useString() const;
   const QString useStringTr() const;
   AmountType amountType() const;
   const QString amountTypeString() const;
   const QString amountTypeStringTr() const;
   double amount() const;
   double inventory();
   int inventoryId() const;
   double time() const;
   bool amountIsWeight() const;
   QString useFor() const;
   QString notes() const;
   bool cacheOnly() const;

   static QString classNameStr();

   NamedEntity * getParent();
   virtual int insertInDatabase();
   virtual void removeFromDatabase();

signals:

   //! \brief Emitted when \c name() changes.
   // Declared in Base Class BeerXMLElement, should not be overloaded
   //void changedName(QString);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;

private:
   Misc(TableSchema* table, QSqlRecord rec, int t_key = -1);
   Misc(Misc & other);

   QString m_typeString;
   Type m_type;
   QString m_useString;
   Use m_use;
   double m_time;
   double m_amount;
   bool m_amountIsWeight;
   QString m_useFor;
   QString m_notes;
   double m_inventory;
   int m_inventory_id;
   bool m_cacheOnly;

   bool isValidType( const QString &var );
   bool isValidUse( const QString &var );

   static QStringList types;
   static QStringList uses;
   static QStringList amountTypes;
};

Q_DECLARE_METATYPE( QList<Misc*> )
/*
inline bool MiscPtrLt( Misc* lhs, Misc* rhs)
{
   return lhs->name() < rhs->name();
}

inline bool MiscPtrEq( Misc* lhs, Misc* rhs)
{
   return lhs->name() == rhs->name();
}

struct Misc_ptr_cmp
{
   bool operator()( Misc* lhs, Misc* rhs)
   {
      return lhs->name() < rhs->name();
   }
};

struct Misc_ptr_equals
{
   bool operator()( Misc* lhs, Misc* rhs )
   {
      return lhs->name() == rhs->name();
   }
};
*/
#endif   /* _MISC_H */
