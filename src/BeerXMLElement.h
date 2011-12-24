/*
 * BeerXMLElement.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BEERXMLELEMENT_H
#define   _BEERXMLELEMENT_H

#include <QDomText>
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QObject>
#include <QMetaProperty>
#include <QVariant>
#include <QDateTime>
#include "database.h"
#include "brewtarget.h"
// For uintptr_t.
#if HAVE_STDINT_H
#   include <stdint.h>
#else
#   include "pstdint.h"
#endif

// Make uintptr_t available in QVariant.
Q_DECLARE_METATYPE( uintptr_t )

//class BeerXMLElement : public QObject
//{
//   Q_OBJECT
//   Q_CLASSINFO("version","1")
//   
//   friend class Database;
//   friend class SetterCommand;
//public:
//   BeerXMLElement() : _key(-1), _table(Database::NOTABLE) {};
//   BeerXMLElement( BeerXMLElement const& other ) : _key(other._key), _table(other._table){};
//   virtual ~BeerXMLElement(){};
//
//   Q_PROPERTY( bool deleted READ deleted )
//   Q_PROPERTY( int key READ key )
//   Q_PROPERTY( Database::DBTable table READ table )
//   
//   //! Convenience method to determine if we are deleted.
//   bool deleted(){ return get("deleted").toBool(); }
//   //! \returns our key in the table we are stored in.
//   int key(){ return _key; }
//   //! \returns the table we are stored in.
//   Database::DBTable table(){ return _table; }
//   //! \returns the BeerXML version of this element.
//   int version(){ return QString(metaObject()->classInfo(metaObject()->indexOfClassInfo("version")).value()).toInt(); }
//   //! Convenience method to get a meta property by name.
//   QMetaProperty metaProperty(const char* name){return metaObject()->property(metaObject()->indexOfProperty(name));}
//   //! Convenience method to get a meta property by name.
//   QMetaProperty metaProperty(QString const& name){return metaObject()->property(metaObject()->indexOfProperty(name.toStdString().c_str()));}
//   
//   // Move this to Database to convert to/from XML from/to SQLite tables.
//   //virtual void fromNode(const QDomNode& node) = 0; // Should initialize this element from the node.
//   
//   // Some static helpers to convert to/from text.
//   static double getDouble( const QDomText& textNode )
//   {
//      bool ok;
//      double ret;
//
//      QString text = textNode.nodeValue();
//
//      ret = text.toDouble(&ok);
//      if( !ok )
//         Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a number. Line %2").arg(text).arg(textNode.lineNumber()) );
//
//      return ret;
//   }
//   static bool getBool( const QDomText& textNode )
//   {
//      QString text = textNode.nodeValue();
//
//      if( text == "TRUE" )
//         return true;
//      else if( text == "FALSE" )
//         return false;
//      else
//         Brewtarget::log(Brewtarget::ERROR, QString("%1 is not a boolean value. Line %2").arg(text).arg(textNode.lineNumber()) );
//
//      return false;
//   }
//   static int getInt( const QDomText& textNode )
//   {
//      bool ok;
//      int ret;
//      QString text = textNode.nodeValue();
//
//      ret = text.toInt(&ok);
//      if( !ok )
//         Brewtarget::log(Brewtarget::ERROR, QString("%1 is not an integer. Line %2").arg(text).arg(textNode.lineNumber()) );
//
//      return ret;
//   }
//   //! Convert the string to a QDateTime according to Qt::ISODate.
//   static QDateTime getDateTime(QString const& str = "")
//   {
//      QDateTime temp;
//   
//      if ( str != "" && (temp = QDateTime::fromString(str, Qt::ISODate)).isValid() ) 
//         return temp;
//      else
//         return QDateTime::currentDateTime();
//   }
//   static QString text(bool val)
//   {
//      if( val )
//         return QString("TRUE");
//      else
//         return QString("FALSE");
//   }
//   static QString text(double val){return QString("%1").arg(val, 0, 'e', 5);}
//   static QString text(int val){return QString("%1").arg(val);}
//   static QString text(QDate const& val){return val.toString("dd-MM-yyyy");}
//   
//signals:
//   //! Passes the meta property that has changed about this object.
//   void changed(QMetaProperty, QVariant value = QVariant());
//   
//protected:
//   
//   //! The key of this ingredient in its table.
//   int _key;
//   //! The table where this ingredient is stored.
//   Database::DBTable _table;
//
//   /*!
//    * \param prop_name - A meta-property name
//    * \param col_name - The appropriate column in the table.
//    * Should do the following:
//    * 1) Set the appropriate value in the appropriate table row.
//    * 2) Call the NOTIFY method associated with \b prop_name if \b notify == true.
//    */
//   void set( const char* prop_name, const char* col_name, QVariant const& value, bool notify = true )
//   {
//      // Get the meta property.
//      int ndx = metaObject()->indexOfProperty(prop_name);
//   
//      // Should schedule an update of the appropriate entry in table,
//      // then use prop to emit its notification signal.
//      Database::instance().updateEntry( _table, _key, col_name, value, metaObject()->property(ndx), this, notify );
//   }
//   
//   /*!
//    * \param col_name - The database column of the attribute we want to get.
//    * Returns the value of the attribute specified by key/table/col_name.
//    */
//   QVariant get( const char* col_name ) const{return Database::instance().get( _table, _key, col_name );}
//   
//private:
//   
//};

class BeerXMLElement;

class BeerXMLElement : public QObject
{
   Q_OBJECT
   Q_CLASSINFO("version","1")
   
   friend class Database;
   friend class SetterCommand;
public:
   BeerXMLElement();
   BeerXMLElement( BeerXMLElement const& other );
   virtual ~BeerXMLElement(){};

   Q_PROPERTY( bool deleted READ deleted )
   Q_PROPERTY( int key READ key )
   Q_PROPERTY( Database::DBTable table READ table )
   
   //! Convenience method to determine if we are deleted.
   bool deleted();
   //! \returns our key in the table we are stored in.
   int key();
   //! \returns the table we are stored in.
   Database::DBTable table();
   //! \returns the BeerXML version of this element.
   int version();
   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(const char* name) const;
   //! Convenience method to get a meta property by name.
   QMetaProperty metaProperty(QString const& name) const;
   
   // Move this to Database to convert to/from XML from/to SQLite tables.
   /*
   virtual void fromNode(const QDomNode& node) = 0; // Should initialize this element from the node.
   */
   
   // Some static helpers to convert to/from text.
   static double getDouble( const QDomText& textNode );
   static bool getBool( const QDomText& textNode );
   static int getInt( const QDomText& textNode );
   static QString getString( QDomText const& textNode );
   static QDateTime getDateTime( QDomText const& textNode );
   //! Convert the string to a QDateTime according to Qt::ISODate.
   static QDateTime getDateTime(QString const& str = "");
   static QString text(bool val);
   static QString text(double val);
   static QString text(int val);
   //! Convert the date to string in Qt::ISODate format for storage NOT display.
   static QString text(QDate const& val);
   
   //! Use this to pass pointers around in QVariants.
   static inline QVariant qVariantFromPtr( BeerXMLElement* ptr )
   {
      // NOTE: weird way to cast ptr to a uintptr_t, but this is the only
      // way I can get it to work.
      uintptr_t addr = *(reinterpret_cast<uintptr_t*>(&ptr));
      return QVariant::fromValue<uintptr_t>(addr);
   }
   
   static inline BeerXMLElement* extractPtr( QVariant ptrVal )
   {
      uintptr_t addr = ptrVal.value<uintptr_t>();
      return reinterpret_cast<BeerXMLElement*>(addr);
   }
   
signals:
   //! Passes the meta property that has changed about this object.
   void changed(QMetaProperty, QVariant value = QVariant());
   
protected:
   
   //! The key of this ingredient in its table.
   int _key;
   //! The table where this ingredient is stored.
   Database::DBTable _table;

   /*!
    * \param prop_name - A meta-property name
    * \param col_name - The appropriate column in the table.
    * Should do the following:
    * 1) Set the appropriate value in the appropriate table row.
    * 2) Call the NOTIFY method associated with \b prop_name if \b notify == true.
    */
   void set( const char* prop_name, const char* col_name, QVariant const& value, bool notify = true );
   
   /*!
    * \param col_name - The database column of the attribute we want to get.
    * Returns the value of the attribute specified by key/table/col_name.
    */
   QVariant get( const char* col_name ) const;
   
private:
   
};


#endif   /* _BEERXMLELEMENT_H */

