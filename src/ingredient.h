/*
 * Ingredient.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
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

#ifndef _INGREDIENT_H
#define _INGREDIENT_H

#include <QDomText>
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QObject>
#include <QMetaProperty>
#include <QVariant>
#include <QDateTime>
#include <QSqlRecord>
#include "brewtarget.h"
// For uintptr_t.
#if HAVE_STDINT_H
#   include <stdint.h>
#else
#   include "pstdint.h"
#endif

// Make uintptr_t available in QVariant.
Q_DECLARE_METATYPE( uintptr_t )

class Ingredient;

/*!
 * \class Ingredient
 * \author Philip G. Lee
 *
 * \brief The base class for our database items.
 */
class Ingredient : public QObject
{
   Q_OBJECT
   Q_CLASSINFO("version","1")

   friend class Database;
   friend class BeerXML;
public:
   Ingredient(Brewtarget::DBTable table, int key, QString t_name = QString(),
                  bool t_display = false, QString folder = QString());
   Ingredient( Ingredient const& other );

   // Everything that inherits from BeerXML has a name, delete, display and a folder
   Q_PROPERTY( QString name   READ name WRITE setName )
   Q_PROPERTY( bool deleted   READ deleted WRITE setDeleted )
   Q_PROPERTY( bool display   READ display WRITE setDisplay )
   Q_PROPERTY( QString folder READ folder WRITE setFolder )

   Q_PROPERTY( int key READ key )
   Q_PROPERTY( Brewtarget::DBTable table READ table )

   //! Convenience method to determine if we are deleted or displayed
   bool deleted() const;
   bool display() const;
   //! Access to the folder attribute.
   QString folder() const;
   //! Access to the name attribute.
   QString name() const;

   //! And ways to set those flags
   void setDeleted(const bool var, bool cachedOnly = false);
   void setDisplay(const bool var, bool cachedOnly = false);
   //! and a way to set the folder
   virtual void setFolder(const QString var, bool signal=true, bool cachedOnly = false);

   //!
   void setName(const QString var, bool cachedOnly = false);

   //! \returns our key in the table we are stored in.
   int key() const;
   //! \returns the table we are stored in.
   Brewtarget::DBTable table() const;
   //! \returns the BeerXML version of this element.
   int version() const;
   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(const char* name) const;
   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(QString const& name) const;

   // Some static helpers to convert to/from text.
   static double getDouble( const QDomText& textNode );
   static bool getBool( const QDomText& textNode );
   static int getInt( const QDomText& textNode );
   static QString getString( QDomText const& textNode );
   static QDateTime getDateTime( QDomText const& textNode );
   static QDate getDate( QDomText const& textNode );
   //! Convert the string to a QDateTime according to Qt::ISODate.
   static QDateTime getDateTime(QString const& str = "");
   static QDate getDate(QString const& str = "");
   static QString text(bool val);
   static QString text(double val);
   static QString text(int val);
   //! Convert the date to string in Qt::ISODate format for storage NOT display.
   static QString text(QDate const& val);

   //! Use this to pass pointers around in QVariants.
   static inline QVariant qVariantFromPtr( Ingredient* ptr )
   {
      uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
      return QVariant::fromValue<uintptr_t>(addr);
   }

   static inline Ingredient* extractPtr( QVariant ptrVal )
   {
      uintptr_t addr = ptrVal.value<uintptr_t>();
      return reinterpret_cast<Ingredient*>(addr);
   }

   bool isValid();
   void invalidate();

signals:
   /*!
    * Passes the meta property that has changed about this object.
    * NOTE: when subclassing, be \em extra careful not to create a method with
    * the same signature. Otherwise, everything will silently break.
    */
   void changed(QMetaProperty, QVariant value = QVariant());
   void changedFolder(QString);
   void changedName(QString);

protected:

   //! The key of this ingredient in its table.
   int _key;
   //! The table where this ingredient is stored.
   Brewtarget::DBTable _table;

   /*!
    * \param prop_name A meta-property name
    * \param col_name The appropriate column in the table.
    * \param value the new value
    * \param notify true to call NOTIFY method associated with \c prop_name
    * Should do the following:
    * 1) Set the appropriate value in the appropriate table row.
    * 2) Call the NOTIFY method associated with \c prop_name if \c notify == true.
    */
   /*
   void set( const char* prop_name, const char* col_name, QVariant const& value, bool notify = true );
   void set( const QString& prop_name, const QString& col_name, const QVariant& value, bool notify = true );
   */
   void setEasy( QString prop_name, QVariant value, bool notify = true );

   /*!
    * \param col_name - The database column of the attribute we want to get.
    * Returns the value of the attribute specified by key/table/col_name.
    */
   QVariant get( const QString& col_name ) const;

   void setInventory( const QVariant& value, int invKey = 0, bool notify=true );
   QVariant getInventory( const QString& col_name ) const;

   QVariantMap getColumnValueMap() const;

private:
   /*!
    * \param valid - Indicates if the beerXML element was valid. There is a problem with importing invalid
    * XML. I'm hoping this helps fix it
    */
  bool _valid;
  mutable QString _folder;
  mutable QString _name;
  mutable QVariant _display;
  mutable QVariant _deleted;

};


#endif   /* _BEERXMLELEMENT_H */

