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

class BeerXMLElement;

#include <QDomText>
#include <QDomNode>
#include <QDomDocument>
#include <QString>
#include <QObject>
#include <QMetaProperty>
#include <QVariant>
#include "database.h"

class BeerXMLElement : public QObject
{
   Q_OBJECT
   Q_CLASSINFO("version","1")
   
   friend class Database;
public:
   BeerXMLElement();
   BeerXMLElement( BeerXMLElement const& other );
   virtual ~BeerXMLElement() {}

   Q_PROPERTY( bool deleted READ deleted )
   Q_PROPERTY( int key READ key )
   Q_PROPERTY( Database::DBTable table READ table )
   
   bool deleted(){ return get("deleted").toBool(); }
   int key(){ return _key; }
   Database::DBTable table(){ return _table; }
   int version(){ return QString(metaObject()->classInfo(metaObject()->indexOfClassInfo("version")).value()).toInt(); }
   
   // There should be Database::createClone(BeerXMLElement&) that does this.
   //void deepCopy( BeerXMLElement* other ); // Constructs a deep copy of this element.
   
   // Move this to Database to convert to/from XML from/to SQLite tables.
   /*
   virtual void fromNode(const QDomNode& node) = 0; // Should initialize this element from the node.
   */
   virtual void toXml(QDomDocument& doc, QDomNode& parent) = 0;
   
   // Some static helpers to convert to/from text.
   static double getDouble( const QDomText& textNode );
   static bool getBool( const QDomText& textNode );
   static int getInt( const QDomText& textNode );
   static QString text(bool val);
   static QString text(double val);
   static QString text(int val);
   
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
    * 2) Call the NOTIFY method associated with \b prop_name.
    */
   void set( const char* prop_name, const char* col_name, QVariant const& value );
   
   /*!
    * \param col_name - The database column of the attribute we want to get.
    * Returns the value of the attribute specified by key/table/col_name.
    */
   QVariant get( const char* col_name );
   
private:
   
};

#endif   /* _BEERXMLELEMENT_H */

