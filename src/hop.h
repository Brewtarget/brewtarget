/*
 * hop.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Jeff Bailey <skydvr38@verizon.net>
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

#ifndef _HOP_H
#define _HOP_H

#include <QString>
#include <QStringList>
#include "ingredient.h"

// Forward declarations.
class Hop;
class HopException;
bool operator<( Hop &h1, Hop &h2 );
bool operator==( Hop &h1, Hop &h2 );

/*!
 * \class Hop
 * \author Philip G. Lee
 *
 * \brief Model class for a hop record in the database.
 */
class Hop : public Ingredient
{
   Q_OBJECT
   Q_CLASSINFO("signal", "hops")

   friend class Database;
   friend class BeerXML;
   friend class HopDialog;
public:

   //! \brief The type of hop, meaning for what properties it is used.
   enum Type {Bittering, Aroma, Both};
   //! \brief The form of the hop.
   enum Form {Leaf, Pellet, Plug};
   //! \brief The way the hop is used.
   enum Use {Mash, First_Wort, Boil, UseAroma, Dry_Hop }; // NOTE: way bad. We have a duplicate enum (Aroma), and BeerXML expects a space for "Dry Hop" and "First Wort". Damn. Damn damn.
   Q_ENUMS( Type Form Use )

   virtual ~Hop() {}

   //! \brief The percent alpha.
   Q_PROPERTY( double alpha_pct READ alpha_pct WRITE setAlpha_pct /*NOTIFY changed*/ /*changedAlpha_pct*/ )
   //! \brief The amount in kg.
   Q_PROPERTY( double amount_kg READ amount_kg WRITE setAmount_kg /*NOTIFY changed*/ /*changedAmount_kg*/ )
   //! \brief The amount in inventory in kg.
   Q_PROPERTY( double inventory READ inventory WRITE setInventoryAmount /*NOTIFY changed*/ /*changedInventory*/ )
   //! \brief The inventory ID -- needed for signal processing. This is pretty much readonly
   Q_PROPERTY( double inventoryId READ inventoryId WRITE setInventoryId /*NOTIFY changed*/ /*changedInventory*/ )
   //! \brief The \c Use.
   Q_PROPERTY( Use use READ use WRITE setUse /*NOTIFY changed*/ /*changedUse*/ )
   //! \brief The untranslated \c Use string.
   Q_PROPERTY( QString useString READ useString /* WRITE setUse NOTIFY changed*/ /*changedUse*/ )
   //! \brief The time in minutes that the hop is used.
   Q_PROPERTY( double time_min READ time_min WRITE setTime_min /*NOTIFY changed*/ /*changedTime_min*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The untranslated string type
   Q_PROPERTY( QString typeString READ typeString /* WRITE setType NOTIFY changed*/ /*changedType*/ )
   //! \brief The \c Form.
   Q_PROPERTY( Form form READ form WRITE setForm /*NOTIFY changed*/ /*changedForm*/ )
   //! \brief The untranslated \c Form string.
   Q_PROPERTY( QString formString READ formString /* WRITE setForm NOTIFY changed*/ /*changedForm*/ )
   //! \brief The percent of beta acids.
   Q_PROPERTY( double beta_pct READ beta_pct WRITE setBeta_pct /*NOTIFY changed*/ /*changedBeta_pct*/ )
   //! \brief The hop stability index in percent.
   Q_PROPERTY( double hsi_pct READ hsi_pct WRITE setHsi_pct /*NOTIFY changed*/ /*changedHsi_pct*/ )
   //! \brief The origin.
   Q_PROPERTY( QString origin READ origin WRITE setOrigin /*NOTIFY changed*/ /*changedOrigin*/ )
   //! \brief The list of substitutes.
   Q_PROPERTY( QString substitutes READ substitutes WRITE setSubstitutes /*NOTIFY changed*/ /*changedSubstitutes*/ )
   //! \brief Humulene as a percentage of total hop oil.
   Q_PROPERTY( double humulene_pct READ humulene_pct WRITE setHumulene_pct /*NOTIFY changed*/ /*changedHumulene_pct*/ )
   //! \brief Caryophyllene as a percentage of total hop oil.
   Q_PROPERTY( double caryophyllene_pct READ caryophyllene_pct WRITE setCaryophyllene_pct /*NOTIFY changed*/ /*changedCaryophyllene_pct*/ )
   //! \brief Cohumulone as a percentage of total hop oil.
   Q_PROPERTY( double cohumulone_pct READ cohumulone_pct WRITE setCohumulone_pct /*NOTIFY changed*/ /*changedCohumulone_pct*/ )
   //! \brief Myrcene as a percentage of total hop oil.
   Q_PROPERTY( double myrcene_pct READ myrcene_pct WRITE setMyrcene_pct /*NOTIFY changed*/ /*changedMyrcene_pct*/ )

   double alpha_pct() const;
   double amount_kg() const;
   double inventory();
   int inventoryId() const;
   // Use in enumerated, untranslated and translated versions
   Use use() const;
   const QString useString() const;
   const QString useStringTr() const;

   double time_min() const;
   const QString notes() const;

   // Type in enumerated, untranslated and translated versions
   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;

   // Form in enumerated, untranslated and translated versions
   Form form() const;
   const QString formString() const;
   const QString formStringTr() const;

   double beta_pct() const;
   double hsi_pct() const;
   const QString origin() const;
   const QString substitutes() const;
   double humulene_pct() const;
   double caryophyllene_pct() const;
   double cohumulone_pct() const;
   double myrcene_pct() const;
   bool cacheOnly() const;

   //set
   void setAlpha_pct( double num);
   void setAmount_kg( double num);
   void setInventoryAmount( double num);
   void setUse( Use u);
   void setTime_min( double num);

   void setNotes( const QString& str);
   void setType( Type t);
   void setForm( Form f);
   void setBeta_pct( double num);
   void setHsi_pct( double num);
   void setOrigin( const QString& str);
   void setSubstitutes( const QString& str);
   void setHumulene_pct( double num);
   void setCaryophyllene_pct( double num);
   void setCohumulone_pct( double num);
   void setMyrcene_pct( double num);
   void setCacheOnly(bool cache);
   void setInventoryId(int key);

   static QString classNameStr();
signals:

private:
   Hop(Brewtarget::DBTable table, int key);
   Hop(Brewtarget::DBTable table, int key, QSqlRecord rec);
   Hop(QString name, bool cache = true);
   Hop( Hop & other );

   QString m_useStr;
   Use m_use;
   QString m_typeStr;
   Type m_type;
   QString m_formStr;
   Form m_form;
   double m_alpha_pct;
   double m_amount_kg;
   double m_time_min;
   QString m_notes;
   double m_beta_pct;
   double m_hsi_pct;
   QString m_origin;
   QString m_substitutes;
   double m_humulene_pct;
   double m_caryophyllene_pct;
   double m_cohumulone_pct;
   double m_myrcene_pct;
   double m_inventory;
   int m_inventory_id;
   bool m_cacheOnly;

   void setDefaults();

   static bool isValidUse(const QString& str);
   static bool isValidType(const QString& str);
   static bool isValidForm(const QString& str);

   static QStringList uses;
   static QStringList types;
   static QStringList forms;

   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

Q_DECLARE_METATYPE( QList<Hop*> )

inline bool HopPtrLt( Hop* lhs, Hop* rhs)
{
   return *lhs < *rhs;
}

inline bool HopPtrEq( Hop* lhs, Hop* rhs)
{
   return *lhs == *rhs;
}

inline bool hopLessThanByTime(const Hop* lhs, const Hop* rhs)
{
   if ( lhs->use() == rhs->use() )
   {
      if ( lhs->time_min() == rhs->time_min() )
         return lhs->name() < rhs->name();

      return lhs->time_min() > rhs->time_min();
   }
   return lhs->use() < rhs->use();
}

struct Hop_ptr_cmp
{
   bool operator()( Hop* lhs, Hop* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Hop_ptr_equals
{
   bool operator()( Hop* lhs, Hop* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif // _HOP_H
