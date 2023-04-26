/*
 * model/Misc.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2023
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
#pragma once

#include <QString>
#include <QSqlRecord>

#include "model/NamedEntityWithInventory.h"

//======================================================================================================================
//========================================== Start of property name constants ==========================================
#define AddPropertyName(property) namespace PropertyNames::Misc { BtStringConst const property{#property}; }
AddPropertyName(amount        )
AddPropertyName(amountIsWeight)
AddPropertyName(amountType    )
AddPropertyName(notes         )
AddPropertyName(time          )
AddPropertyName(typeString    )
AddPropertyName(type          )
AddPropertyName(useFor        )
AddPropertyName(useString     )
AddPropertyName(use           )
#undef AddPropertyName
//=========================================== End of property name constants ===========================================
//======================================================================================================================


/*!
 * \class Misc
 *
 * \brief Model for a misc record in the database.
 */
class Misc : public NamedEntityWithInventory {
   Q_OBJECT
   Q_CLASSINFO("signal", "miscs")

public:

   //! \brief The type of ingredient.
   enum class Type {Spice, Fining, Water_Agent, Herb, Flavor, Other};
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Type)

   //! \brief Where the ingredient is used.
   enum class Use { Boil, Mash, Primary, Secondary, Bottling };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(Use)

   //! \brief What is the type of amount.
   enum class AmountType { Weight, Volume };
   // This allows us to store the above enum class in a QVariant
   Q_ENUM(AmountType)

   /**
    * \brief Mapping of names to types for the Qt properties of this class.  See \c NamedEntity::typeLookup for more
    *        info.
    */
   static TypeLookup const typeLookup;

   Misc(QString name = "");
   Misc(NamedParameterBundle const & namedParameterBundle);
   Misc(Misc const & other);

   virtual ~Misc();

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

   //! \brief The amount in inventory in either kg or L, depending on \c amountIsWeight().
   virtual void setInventoryAmount( double var );
   void setTime( double var );
   void setAmountIsWeight( bool var );
   void setUseFor( const QString &var );
   void setNotes( const QString &var );

   // Get
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
   //! \brief The amount in inventory in either kg or L, depending on \c amountIsWeight().
   virtual double inventory() const;
   double time() const;
   bool amountIsWeight() const;
   QString useFor() const;
   QString notes() const;

   virtual Recipe * getOwningRecipe();

signals:

   //! \brief Emitted when \c name() changes.
   // Declared in Base Class NamedEntity, should not be overloaded
   //void changedName(QString);

protected:
   virtual bool isEqualTo(NamedEntity const & other) const;
   virtual ObjectStore & getObjectStoreTypedInstance() const;

private:
   Type m_type;
   Use  m_use;  // Primarily valid in "Use Of" instance
   double m_time;
   double m_amount;
   bool   m_amountIsWeight;
   QString m_useFor;
   QString m_notes;

   bool isValidType( const QString &var );
   bool isValidUse( const QString &var );
};

Q_DECLARE_METATYPE( QList<Misc*> )

#endif
