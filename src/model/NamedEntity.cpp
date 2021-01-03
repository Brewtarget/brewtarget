/*
 * model/NamedEntity.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
 * - Kregg K <gigatropolis@yahoo.com>
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

#include "model/NamedEntity.h"
#include <QDomElement>
#include <QDomNode>
#include <QMetaProperty>
#include "brewtarget.h"
#include "database.h"

static const char* kVersion = "version";

NamedEntity::NamedEntity(Brewtarget::DBTable table, int key, QString t_name, bool t_display, QString folder)
   : QObject(nullptr),
     _key(key),
     _table(table),
     parentKey(0),
     _valid(true),
     _folder(folder),
     _name(t_name),
     _display(t_display),
     _deleted(QVariant())
{
}

NamedEntity::NamedEntity(NamedEntity const& other)
   : QObject(nullptr),
     _key(other._key),
     _table(other._table),
     parentKey(other.parentKey),
     _valid(true),
     _folder(other._folder),
     _name(QString()),
     _display(other._display),
     _deleted(other._deleted)
{
}

bool NamedEntity::deleted() const
{
   return _deleted.toBool();
}

bool NamedEntity::display() const
{
   return _display.toBool();
}

// Sigh. New databases, more complexity
void NamedEntity::setDeleted(const bool var, bool cachedOnly)
{
   _deleted = var;
   if ( ! cachedOnly )
      setEasy(kpropDeleted, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
}

void NamedEntity::setDisplay(bool var, bool cachedOnly)
{
   _display = var;
   if ( ! cachedOnly )
      setEasy(kpropDisplay, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
}

QString NamedEntity::folder() const
{
   return _folder;
}

void NamedEntity::setFolder(const QString var, bool signal, bool cachedOnly)
{
   _folder = var;
   if ( ! cachedOnly )
      // set( kFolder, kFolder, var );
      setEasy( kpropFolder, var );
   // not sure if I should only signal when not caching?
   if ( signal )
      emit changedFolder(var);
}

QString NamedEntity::name() const
{
   return _name;
}

void NamedEntity::setName(const QString var, bool cachedOnly)
{

   _name = var;
   if ( ! cachedOnly ) {
      setEasy( kpropName, var );
      emit changedName(var);
   }
}

int NamedEntity::key() const
{
   return _key;
}

Brewtarget::DBTable NamedEntity::table() const
{
   return _table;
}

int NamedEntity::version() const
{
   return QString(metaObject()->classInfo(metaObject()->indexOfClassInfo(kVersion)).value()).toInt();
}

QMetaProperty NamedEntity::metaProperty(const char* name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name));
}

QMetaProperty NamedEntity::metaProperty(QString const& name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name.toStdString().c_str()));
}

// getVal =====================================================================
double NamedEntity::getDouble(const QDomText& textNode)
{
   bool ok;
   double ret;

   QString text = textNode.nodeValue();

   // ret = text.toDouble(&ok);
   ret = Brewtarget::toDouble(text,&ok);
   if( !ok )
      qCritical() << QString("NamedEntity::getDouble: %1 is not a number. Line %2").arg(text).arg(textNode.lineNumber());

   return ret;
}

bool NamedEntity::getBool(const QDomText& textNode)
{
   QString text = textNode.nodeValue();

   if( text == "TRUE" )
      return true;
   else if( text == "FALSE" )
      return false;
   else
      qCritical() << QString("NamedEntity::getBool: %1 is not a boolean value. Line %2").arg(text).arg(textNode.lineNumber());

   return false;
}

int NamedEntity::getInt(const QDomText& textNode)
{
   bool ok;
   int ret;
   QString text = textNode.nodeValue();

   ret = text.toInt(&ok);
   if( !ok )
      qCritical() << QString("NamedEntity::getInt: %1 is not an integer. Line %2").arg(text).arg(textNode.lineNumber());

   return ret;
}

QString NamedEntity::getString( QDomText const& textNode )
{
   return textNode.nodeValue();
}

QDateTime NamedEntity::getDateTime( QDomText const& textNode )
{
   bool ok = true;
   QDateTime ret;
   QString text = textNode.nodeValue();

   ret = QDateTime::fromString(text, Qt::ISODate);
   ok = ret.isValid();
   if( !ok )
      qCritical() << QString("NamedEntity::getDateTime: %1 is not a date. Line %2").arg(text).arg(textNode.lineNumber());

   return ret;
}

QDate NamedEntity::getDate( QDomText const& textNode )
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
      qCritical() << QString("NamedEntity::getDate: %1 is not an ISO date-time. Line %2").arg(text).arg(textNode.lineNumber());

   return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDateTime NamedEntity::getDateTime(QString const& str)
{
   QDateTime temp;

   if ( str != "" && (temp = QDateTime::fromString(str, Qt::ISODate)).isValid() )
      return temp;
   else
      return QDateTime::currentDateTime();
}

QString NamedEntity::text(bool val)
{
   if( val )
      return QString("TRUE");
   else
      return QString("FALSE");
}

QString NamedEntity::text(double val)
{
   return QString("%1").arg(val, 0, 'f', 8);
}

QString NamedEntity::text(int val)
{
   return QString("%1").arg(val);
}

QString NamedEntity::text(QDate const& val)
{
   return val.toString(Qt::ISODate);
}

void NamedEntity::setEasy(QString prop_name, QVariant value, bool notify)
{
   Database::instance().updateEntry(this,prop_name,value,notify);
}


QVariant NamedEntity::get( const QString& col_name ) const
{
   return Database::instance().get( _table, _key, col_name );
}

void NamedEntity::setInventory( const QVariant& value, int invKey, bool notify )
{
   Database::instance().setInventory( this, value, invKey, notify );
}

QVariant NamedEntity::getInventory( const QString& col_name ) const
{
   QVariant val = 0.0;
   val = Database::instance().getInventoryAmt(col_name, _table, _key);
   return val;
}

bool NamedEntity::isValid()
{
   return _valid;
}

void NamedEntity::invalidate()
{
   _valid = false;
}

void NamedEntity::setParent(NamedEntity const & parentNamedEntity)
{
   this->parentKey = parentNamedEntity._key;
   return;
}

QVariantMap NamedEntity::getColumnValueMap() const
{
   QVariantMap map;
   map.insert(kpropFolder, folder());
   map.insert(kpropName, name());
   return map;
}
