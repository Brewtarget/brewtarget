/*
 * BeerXMLElement.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Kregg K <gigatropolis@yahoo.com>
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

#include "BeerXMLElement.h"
#include <QDomElement>
#include <QDomNode>
#include <QMetaProperty>
#include "brewtarget.h"
#include "database.h"

static const QString kFolder("folder");
static const QString kName("name");
static const QString kDeleted("deleted");
static const QString kDisplay("display");

static const char* kVersion = "version";

BeerXMLElement::BeerXMLElement(Brewtarget::DBTable table, int key)
   : QObject(0),
     _key(key),
     _table(table),
     _folder(QString()),
     _name(QString()),
     _display(QVariant()),
     _deleted(QVariant())
{
   _valid = true;
}

BeerXMLElement::BeerXMLElement(BeerXMLElement const& other)
   : QObject(0),
   _key(other._key),
   _table(other._table),
   _folder(QString()),
   _name(QString()),
   _display(QVariant()),
   _deleted(QVariant())
{
   _valid = true;
}

bool BeerXMLElement::deleted() const
{

   if ( ! _deleted.isValid() )
      _deleted = get(kDeleted);

   return _deleted.toBool();
}

bool BeerXMLElement::display() const
{
   if ( ! _display.isValid() )
      _display = get(kDisplay);

   return _display.toBool();
}

// Sigh. New databases, more complexity
void BeerXMLElement::setDeleted(const bool var)
{
   set(kDeleted, kDeleted, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
   _deleted = var;
}

void BeerXMLElement::setDisplay(bool var)
{
   set(kDisplay, kDisplay, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
   _display = var;
}

QString BeerXMLElement::folder() const
{
   if ( _folder.isEmpty() )
      _folder = get(kFolder).toString();

   return _folder;
}

void BeerXMLElement::setFolder(const QString var, bool signal)
{
   set( kFolder, kFolder, var );
   _folder = var;
   if ( signal )
      emit changedFolder(var);
}

QString BeerXMLElement::name() const
{
   if ( _name.isEmpty() )
      _name = get(kName).toString();

   return _name;
}

void BeerXMLElement::setName(const QString var)
{
   set( kName, kName, var );
   _name = var;
   emit changedName(var);
}

int BeerXMLElement::key() const
{
   return _key;
}

Brewtarget::DBTable BeerXMLElement::table() const
{
   return _table;
}


int BeerXMLElement::version() const
{
   return QString(metaObject()->classInfo(metaObject()->indexOfClassInfo(kVersion)).value()).toInt();
}

QMetaProperty BeerXMLElement::metaProperty(const char* name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name));
}

QMetaProperty BeerXMLElement::metaProperty(QString const& name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name.toStdString().c_str()));
}

// getVal =====================================================================
double BeerXMLElement::getDouble(const QDomText& textNode)
{
   bool ok;
   double ret;

   QString text = textNode.nodeValue();

   // ret = text.toDouble(&ok);
   ret = Brewtarget::toDouble(text,&ok);
   if( !ok )
      Brewtarget::logE(QString("BeerXMLElement::getDouble: %1 is not a number. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

bool BeerXMLElement::getBool(const QDomText& textNode)
{
   QString text = textNode.nodeValue();

   if( text == "TRUE" )
      return true;
   else if( text == "FALSE" )
      return false;
   else
      Brewtarget::logE(QString("BeerXMLElement::getBool: %1 is not a boolean value. Line %2").arg(text).arg(textNode.lineNumber()) );

   return false;
}

int BeerXMLElement::getInt(const QDomText& textNode)
{
   bool ok;
   int ret;
   QString text = textNode.nodeValue();

   ret = text.toInt(&ok);
   if( !ok )
      Brewtarget::logE(QString("BeerXMLElement::getInt: %1 is not an integer. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

QString BeerXMLElement::getString( QDomText const& textNode )
{
   return textNode.nodeValue();
}

QDateTime BeerXMLElement::getDateTime( QDomText const& textNode )
{
   bool ok = true;
   QDateTime ret;
   QString text = textNode.nodeValue();

   ret = QDateTime::fromString(text, Qt::ISODate);
   ok = ret.isValid();
   if( !ok )
      Brewtarget::logE(QString("BeerXMLElement::getDateTime: %1 is not a date. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

QDate BeerXMLElement::getDate( QDomText const& textNode )
{
   bool ok = true;
   QDate ret;
   QString text = textNode.nodeValue();

   ret = QDate::fromString(text, "M/d/yyyy");
   ok = ret.isValid();
   // Dates have some odd inconsistencies.
   if( !ok )
   {
      ret = QDate::fromString(text,"d/M/yyyy");
      ok = ret.isValid();
   }

   if ( !ok )
      Brewtarget::logE(QString("BeerXMLElement::getDate: %1 is not an ISO date-time. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDateTime BeerXMLElement::getDateTime(QString const& str)
{
   QDateTime temp;

   if ( str != "" && (temp = QDateTime::fromString(str, Qt::ISODate)).isValid() )
      return temp;
   else
      return QDateTime::currentDateTime();
}

QString BeerXMLElement::text(bool val)
{
   if( val )
      return QString("TRUE");
   else
      return QString("FALSE");
}

QString BeerXMLElement::text(double val)
{
   return QString("%1").arg(val, 0, 'f', 8);
}

QString BeerXMLElement::text(int val)
{
   return QString("%1").arg(val);
}

QString BeerXMLElement::text(QDate const& val)
{
   return val.toString(Qt::ISODate);
}

void BeerXMLElement::set( const char* prop_name, const char* col_name, QVariant const& value, bool notify )
{
   if (prop_name != NULL && col_name != NULL) {
    // Get the meta property.
    int ndx = metaObject()->indexOfProperty(prop_name);

    // Should schedule an update of the appropriate entry in table,
    // then use prop to emit its notification signal.
    Database::instance().updateEntry( _table, _key, col_name, value, metaObject()->property(ndx), this, notify );
   }
}

void BeerXMLElement::set(const QString &prop_name, const QString &col_name, const QVariant &value, bool notify)
{
   set(prop_name.toUtf8().constData(), col_name.toUtf8().constData(), value, notify);
}

QVariant BeerXMLElement::get( const char* col_name ) const
{
   return Database::instance().get( _table, _key, col_name );
}

QVariant BeerXMLElement::get( const QString& col_name ) const
{
   return get(col_name.toUtf8().constData());
}

void BeerXMLElement::setInventory( const char* prop_name, const char* col_name, QVariant const& value, bool notify )
{
    // Get the meta property.
    int ndx = metaObject()->indexOfProperty(prop_name);

    int invkey = Database::instance().getInventoryID(_table, _key);
    Brewtarget::DBTable invtable = Database::instance().getInventoryTable(_table);
    if(invkey == 0){ //no inventory row in the database so lets make one
      Database::instance().newInventory(_table,_key);
      invkey = Database::instance().getInventoryID(_table, _key);
    }
    Database::instance().updateEntry( invtable, invkey, col_name, value, metaObject()->property(ndx), this, notify );
}

void BeerXMLElement::setInventory( const QString& prop_name, const QString& col_name, QVariant const& value, bool notify )
{
   setInventory(prop_name.toUtf8().constData(), col_name.toUtf8().constData(), value, notify);
}

QVariant BeerXMLElement::getInventory( const char* col_name ) const
{
   int invkey = Database::instance().getInventoryID(_table, _key);
   Brewtarget::DBTable invtable = Database::instance().getInventoryTable(_table);
   QVariant val = 0.0;
   if(invkey != 0){
      val = Database::instance().get( invtable , invkey, col_name );
   }
   return val;
}

QVariant BeerXMLElement::getInventory( const QString& col_name ) const
{
   return getInventory(col_name.toUtf8().constData());
}

bool BeerXMLElement::isValid()
{
   return _valid;
}

void BeerXMLElement::invalidate()
{
   _valid = false;
}

QVariantMap BeerXMLElement::getColumnValueMap() const
{
   QVariantMap map;
   map.insert(kFolder, folder());
   map.insert(kName, name());
   return map;
}
