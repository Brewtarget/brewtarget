/*
 * yeast.h is part of Brewtarget, and is Copyright the following
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

#ifndef _YEAST_H
#define _YEAST_H

#include "ingredient.h"
#include <QString>
#include <QStringList>

// Forward declarations.
class Yeast;
bool operator<(Yeast &y1, Yeast &y2);
bool operator==(Yeast &y1, Yeast &y2);

/*!
 * \class Yeast
 * \author Philip G. Lee
 *
 * \brief Model for yeast records in the database.
 */
class Yeast : public Ingredient
{
   Q_OBJECT
   Q_CLASSINFO("signal", "yeasts")

   friend class Database;
   friend class BeerXML;
   friend class YeastDialog;
public:
   //! \brief What beverage the yeast is for.
   enum Type {Ale, Lager, Wheat, Wine, Champagne};
   //! \brief What form the yeast comes in.
   enum Form {Liquid, Dry, Slant, Culture};
   //! \brief How flocculant the strain is.
   enum Flocculation {Low, Medium, High, Very_High}; // NOTE: BeerXML expects a space in "Very High", but not possible with enum. What to do?
   Q_ENUMS( Type Form Flocculation )

   virtual ~Yeast() {}

   //! \brief The \c Type.
   Q_PROPERTY( Type type READ type WRITE setType /*NOTIFY changed*/ /*changedType*/ )
   //! \brief The \c Type string.
   Q_PROPERTY( QString typeString READ typeString )
   //! \brief The translated \c Type string.
   Q_PROPERTY( QString typeStringTr READ typeStringTr )
   //! \brief The \c Form.
   Q_PROPERTY( Form form READ form WRITE setForm /*NOTIFY changed*/ /*changedForm*/ )
   //! \brief The \c Form string.
   Q_PROPERTY( QString formString READ formString )
   //! \brief The translated \c Form string.
   Q_PROPERTY( QString formStringTr READ formStringTr )
   //! \brief The amount in either liters or kg depending on \c amountIsWeight().
   Q_PROPERTY( double amount READ amount WRITE setAmount /*NOTIFY changed*/ /*changedAmount*/ )
   //! \brief The amount in inventory in either liters or kg depending on \c amountIsWeight().
   Q_PROPERTY( double inventory READ inventory WRITE setInventoryQuanta /*NOTIFY changed*/ /*changedInventory*/ )
   //! \brief The inventory id
   Q_PROPERTY( double inventoryId READ inventoryId WRITE setInventoryId /*NOTIFY changed*/ /*changedInventory*/ )
   //! \brief Whether the \c amount() is weight (kg) or volume (liters).
   Q_PROPERTY( bool amountIsWeight READ amountIsWeight WRITE setAmountIsWeight /*NOTIFY changed*/ /*changedAmountIsWeight*/ )
   //! \brief The lab from which it came.
   Q_PROPERTY( QString laboratory READ laboratory WRITE setLaboratory /*NOTIFY changed*/ /*changedLaboratory*/ )
   //! \brief The product ID.
   Q_PROPERTY( QString productID READ productID WRITE setProductID /*NOTIFY changed*/ /*changedProductID*/ )
   //! \brief The minimum fermenting temperature.
   Q_PROPERTY( double minTemperature_c READ minTemperature_c WRITE setMinTemperature_c /*NOTIFY changed*/ /*changedMinTemperature_c*/ )
   //! \brief The maximum fermenting temperature.
   Q_PROPERTY( double maxTemperature_c READ maxTemperature_c WRITE setMaxTemperature_c /*NOTIFY changed*/ /*changedMaxTemperature_c*/ )
   //! \brief The \c Flocculation.
   Q_PROPERTY( Flocculation flocculation READ flocculation WRITE setFlocculation /*NOTIFY changed*/ /*changedFlocculation*/ )
   //! \brief The \c Flocculation string.
   Q_PROPERTY( QString flocculationString READ flocculationString )
   //! \brief The translated \c Flocculation string.
   Q_PROPERTY( QString flocculationStringTr READ flocculationStringTr )
   //! \brief The apparent attenuation in percent.
   Q_PROPERTY( double attenuation_pct READ attenuation_pct WRITE setAttenuation_pct /*NOTIFY changed*/ /*changedAttenuation_pct*/ )
   //! \brief The notes.
   Q_PROPERTY( QString notes READ notes WRITE setNotes /*NOTIFY changed*/ /*changedNotes*/ )
   //! \brief What styles the strain is best for.
   Q_PROPERTY( QString bestFor READ bestFor WRITE setBestFor /*NOTIFY changed*/ /*changedBestFor*/ )
   //! \brief The number of times recultured.
   Q_PROPERTY( int timesCultured READ timesCultured WRITE setTimesCultured /*NOTIFY changed*/ /*changedTimesCultured*/ )
   //! \brief The maximum recommended number of reculturings.
   Q_PROPERTY( int maxReuse READ maxReuse WRITE setMaxReuse /*NOTIFY changed*/ /*changedMaxReuse*/ )
   //! \brief Whether the yeast is added to secondary or primary.
   Q_PROPERTY( bool addToSecondary READ addToSecondary WRITE setAddToSecondary /*NOTIFY changed*/ /*changedAddToSecondary*/ )

   // Setters
   void setType( Type t);
   void setForm( Form f);
   void setAmount( double var);
   void setInventoryQuanta(int var);
   void setAmountIsWeight( bool var);
   void setLaboratory( const QString& var);
   void setProductID( const QString& var);
   void setMinTemperature_c( double var);
   void setMaxTemperature_c( double var);
   void setFlocculation( Flocculation f);
   void setAttenuation_pct( double var);
   void setNotes( const QString& var);
   void setBestFor( const QString& var);
   void setTimesCultured( int var);
   void setMaxReuse( int var);
   void setAddToSecondary( bool var);
   void setCacheOnly( bool cache);
   void setInventoryId( int key);

   // Getters
   Type type() const;
   const QString typeString() const;
   const QString typeStringTr() const;
   Form form() const;
   const QString formString() const;
   const QString formStringTr() const;
   double amount() const;
   int inventory();
   int inventoryId() const;
   bool amountIsWeight() const;
   QString laboratory() const;
   QString productID() const;
   double minTemperature_c() const;
   double maxTemperature_c() const;
   Flocculation flocculation() const;
   const QString flocculationString() const;
   const QString flocculationStringTr() const;
   double attenuation_pct() const;
   QString notes() const;
   QString bestFor() const;
   int timesCultured() const;
   int maxReuse() const;
   bool addToSecondary() const;
   bool cacheOnly() const;

   static QString classNameStr();

signals:

private:
   Yeast(Brewtarget::DBTable table, int key);
   Yeast(Brewtarget::DBTable table, int key, QSqlRecord rec);
   Yeast(QString name, bool cache = true);
   Yeast(Yeast & other);

   QString m_typeString;
   Type m_type;
   QString m_formString;
   Form m_form;
   QString m_flocculationString;
   Flocculation m_flocculation;
   double m_amount;
   bool m_amountIsWeight;
   QString m_laboratory;
   QString m_productID;
   double m_minTemperature_c;
   double m_maxTemperature_c;
   double m_attenuation_pct;
   QString m_notes;
   QString m_bestFor;
   int m_timesCultured;
   int m_maxReuse;
   bool m_addToSecondary;
   int m_inventory;
   int m_inventory_id;
   bool m_cacheOnly;

   static QStringList types;
   static QStringList forms;
   static QStringList flocculations;

   // Methods
   bool isValidType(const QString& str) const;
   bool isValidForm(const QString& str) const;
   bool isValidFlocculation(const QString& str) const;

   static QHash<QString,QString> tagToProp;
   static QHash<QString,QString> tagToPropHash();
};

Q_DECLARE_METATYPE( QList<Yeast*> )

inline bool YeastPtrLt( Yeast* lhs, Yeast* rhs)
{
   return *lhs < *rhs;
}

inline bool YeastPtrEq( Yeast* lhs, Yeast* rhs)
{
   return *lhs == *rhs;
}

struct Yeast_ptr_cmp
{
   bool operator()( Yeast* lhs, Yeast* rhs)
   {
      return *lhs < *rhs;
   }
};

struct Yeast_ptr_equals
{
   bool operator()( Yeast* lhs, Yeast* rhs )
   {
      return *lhs == *rhs;
   }
};

#endif   /* _YEAST_H */

