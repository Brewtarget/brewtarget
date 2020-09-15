/*
 * Ingredient.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020-2025
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

#include "ingredient.h"
#include <QDomElement>
#include <QDomNode>
#include <QMetaProperty>
#include "brewtarget.h"
#include "database.h"

static const char* kVersion = "version";

Ingredient::Ingredient(Brewtarget::DBTable table, int key, QString t_name, bool t_display, QString folder)
   : QObject(nullptr),
     _key(key),
     _table(table),
     _valid(true),
     _folder(folder),
     _name(t_name),
     _display(t_display),
     _deleted(QVariant())
{
}

Ingredient::Ingredient(Ingredient const& other)
   : QObject(nullptr),
     _key(other._key),
     _table(other._table),
     _valid(true),
     _folder(other._folder),
     _name(QString()),
     _display(other._display),
     _deleted(other._deleted)
{
}

bool Ingredient::deleted() const
{

   return _deleted.toBool();
}

bool Ingredient::display() const
{
   return _display.toBool();
}

// Sigh. New databases, more complexity
void Ingredient::setDeleted(const bool var, bool cachedOnly)
{
   _deleted = var;
   if ( ! cachedOnly )
      setEasy(kpropDeleted, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
}

void Ingredient::setDisplay(bool var, bool cachedOnly)
{
   _display = var;
   if ( ! cachedOnly )
      setEasy(kpropDisplay, var ? Brewtarget::dbTrue() : Brewtarget::dbFalse());
}

QString Ingredient::folder() const
{
   return _folder;
}

void Ingredient::setFolder(const QString var, bool signal, bool cachedOnly)
{
   _folder = var;
   if ( ! cachedOnly )
      // set( kFolder, kFolder, var );
      setEasy( kpropFolder, var );
   // not sure if I should only signal when not caching?
   if ( signal )
      emit changedFolder(var);
}

QString Ingredient::name() const
{
   return _name;
}

void Ingredient::setName(const QString var, bool cachedOnly)
{

   _name = var;
   if ( ! cachedOnly ) {
      setEasy( kpropName, var );
      emit changedName(var);
   }
}

int Ingredient::key() const
{
   return _key;
}

Brewtarget::DBTable Ingredient::table() const
{
   return _table;
}

int Ingredient::version() const
{
   return QString(metaObject()->classInfo(metaObject()->indexOfClassInfo(kVersion)).value()).toInt();
}

QMetaProperty Ingredient::metaProperty(const char* name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name));
}

QMetaProperty Ingredient::metaProperty(QString const& name) const
{
   return metaObject()->property(metaObject()->indexOfProperty(name.toStdString().c_str()));
}

// getVal =====================================================================
double Ingredient::getDouble(const QDomText& textNode)
{
   bool ok;
   double ret;

   QString text = textNode.nodeValue();

   // ret = text.toDouble(&ok);
   ret = Brewtarget::toDouble(text,&ok);
   if( !ok )
      Brewtarget::logE(QString("Ingredient::getDouble: %1 is not a number. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

bool Ingredient::getBool(const QDomText& textNode)
{
   QString text = textNode.nodeValue();

   if( text == "TRUE" )
      return true;
   else if( text == "FALSE" )
      return false;
   else
      Brewtarget::logE(QString("Ingredient::getBool: %1 is not a boolean value. Line %2").arg(text).arg(textNode.lineNumber()) );

   return false;
}

int Ingredient::getInt(const QDomText& textNode)
{
   bool ok;
   int ret;
   QString text = textNode.nodeValue();

   ret = text.toInt(&ok);
   if( !ok )
      Brewtarget::logE(QString("Ingredient::getInt: %1 is not an integer. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

QString Ingredient::getString( QDomText const& textNode )
{
   return textNode.nodeValue();
}

QDateTime Ingredient::getDateTime( QDomText const& textNode )
{
   bool ok = true;
   QDateTime ret;
   QString text = textNode.nodeValue();

   ret = QDateTime::fromString(text, Qt::ISODate);
   ok = ret.isValid();
   if( !ok )
      Brewtarget::logE(QString("Ingredient::getDateTime: %1 is not a date. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

QDate Ingredient::getDate( QDomText const& textNode )
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
      Brewtarget::logE(QString("Ingredient::getDate: %1 is not an ISO date-time. Line %2").arg(text).arg(textNode.lineNumber()) );

   return ret;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

QDateTime Ingredient::getDateTime(QString const& str)
{
   QDateTime temp;

   if ( str != "" && (temp = QDateTime::fromString(str, Qt::ISODate)).isValid() )
      return temp;
   else
      return QDateTime::currentDateTime();
}

QString Ingredient::text(bool val)
{
   if( val )
      return QString("TRUE");
   else
      return QString("FALSE");
}

QString Ingredient::text(double val)
{
   return QString("%1").arg(val, 0, 'f', 8);
}

QString Ingredient::text(int val)
{
   return QString("%1").arg(val);
}

QString Ingredient::text(QDate const& val)
{
   return val.toString(Qt::ISODate);
}

void Ingredient::setEasy(QString prop_name, QVariant value, bool notify)
{
   Database::instance().updateEntry(this,prop_name,value,notify);
}


QVariant Ingredient::get( const QString& col_name ) const
{
   return Database::instance().get( _table, _key, col_name );
}

void Ingredient::setInventory( const QVariant& value, int invKey, bool notify )
{
   Database::instance().setInventory( this, value, invKey, notify );
}

QVariant Ingredient::getInventory( const QString& col_name ) const
{
   QVariant val = 0.0;
   val = Database::instance().getInventoryAmt(col_name, _table, _key);
   return val;
}

bool Ingredient::isValid()
{
   return _valid;
}

void Ingredient::invalidate()
{
   _valid = false;
}

QVariantMap Ingredient::getColumnValueMap() const
{
   QVariantMap map;
   map.insert(kpropFolder, folder());
   map.insert(kpropName, name());
   return map;
}
