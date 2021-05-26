/*
 * model/NamedEntity.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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

#include <typeinfo>

#include <QDomElement>
#include <QDomNode>
#include <QMetaProperty>

#include "NamedEntity.h"
#include "brewtarget.h"
#include "database.h"

static const char* kVersion = "version";

// sometimes I just want to create a thing with a name and nothing else
NamedEntity::NamedEntity(Brewtarget::DBTable table, QString t_name, bool t_display)
   : QObject(nullptr),
     m_key(-1),
     m_table(table),
     parentKey(0),
     m_folder(QString()),
     m_name(t_name),
     m_display(t_display),
     m_deleted(QVariant())
{
}

// othertimes, we creating a thing from a record in the db
NamedEntity::NamedEntity(TableSchema* table, QSqlRecord rec, int t_key )
   : QObject(nullptr),
     parentKey(0),
     m_deleted(QVariant())
{
   if ( t_key == -1 ) {
      m_key = rec.value( table->keyName() ).toInt();
   }
   else {
      m_key = t_key;
   }
      
   m_folder  = rec.value( table->propertyToColumn(PropertyNames::NamedEntity::folder)  ).toString();
   m_name    = rec.value( table->propertyToColumn(PropertyNames::NamedEntity::name)    ).toString();
   m_display = rec.value( table->propertyToColumn(PropertyNames::NamedEntity::display) ).toBool();
   m_table   = table->dbTable();
}

// and finally sometimes we create a thing from other things
NamedEntity::NamedEntity(NamedEntity const& other)
   : QObject(nullptr),
     m_key(other.m_key),
     m_table(other.m_table),
     parentKey(other.parentKey),
     m_folder(other.m_folder),
     m_name(QString()),
     m_display(other.m_display),
     m_deleted(other.m_deleted)
{
   return;
}


QRegExp const & NamedEntity::getDuplicateNameNumberMatcher() {
   //
   // Note that, in the regexp, to match a bracket, we need to escape it, thus "\(" instead of "(".  However, we
   // must also escape the backslash so that the C++ compiler doesn't think we want a special character (such as
   // '\n') and barf a "unknown escape sequence" warning at us.  So "\\(" is needed in the string literal here to
   // pass "\(" to the regexp to match literal "(" (and similarly for close bracket).
   //
   static QRegExp const duplicateNameNumberMatcher{" *\\(([0-9]+)\\)$"};
   return duplicateNameNumberMatcher;
}


// See https://zpz.github.io/blog/overloading-equality-operator-in-cpp-class-hierarchy/ (and cross-references to
// http://www.gotw.ca/publications/mill18.htm) for good discussion on implementation of operator== in a class
// hierarchy.  Our implementation differs slightly for a couple of reasons:
//   - This class is already abstract so it's good to force subclasses to implement isEqualTo() by making it pure
//     virtual here.  (Of course that doesn't help us for sub-sub-classes etc, but it's better than nothing)
//   - We want to do the type comparison first, as this saves us repeating this test in each subclass
//
bool NamedEntity::operator==(NamedEntity const & other) const {
   // The first thing to do is check we are even comparing two objects of the same class.  A Hop is never equal to
   // a Recipe etc.
   if (typeid(*this) != typeid(other)) {
//      qDebug() << Q_FUNC_INFO << "No type id match (" << typeid(*this).name() << "/" << typeid(other).name() << ")";
      return false;
   }

   //
   // For the base class attributes, we deliberately don't compare m_key, parentKey, table or m_folder.  If we've read
   // in an object from a file and want to  see if it's the same as one in the database, then the DB-related info and
   // folder classification are not a helpful part of that comparison.  Similarly, we do not compare m_display and
   // m_deleted as they are more related to the UI than whether, in essence, two objects are the same.
   //
   if (this->m_name != other.m_name) {
//      qDebug() << Q_FUNC_INFO << "No name match (" << this->m_name << "/" << other.m_name << ")";
      //
      // If the names don't match, let's check it's not for a trivial reason.  Eg, if you have one Hop called
      // "Tettnang" and another called "Tettnang (1)" we wouldn't say they are different just because of the names.
      // So we want to strip off any number in brackets at the ends of the names and then compare again.
      //
      QRegExp const & duplicateNameNumberMatcher = NamedEntity::getDuplicateNameNumberMatcher();
      QString names[2] {this->m_name, other.m_name};
      for (auto ii = 0; ii < 2; ++ii) {
         int positionOfMatch = duplicateNameNumberMatcher.indexIn(names[ii]);
         if (positionOfMatch > -1) {
            // There's some integer in brackets at the end of the name.  Chop it off.
            names[ii].truncate(positionOfMatch);
         }
      }
//      qDebug() << Q_FUNC_INFO << "Adjusted names to " << names[0] << " & " << names[1];
      if (names[0] != names[1]) {
         return false;
      }
   }

   return this->isEqualTo(other);
}

bool NamedEntity::operator!=(NamedEntity const & other) const {
   // Don't reinvent the wheel '!=' should just be the opposite of '=='
   return !(*this == other);
}

bool NamedEntity::operator<(const NamedEntity & other) const { return (this->m_name < other.m_name); }
bool NamedEntity::operator>(const NamedEntity & other) const { return (this->m_name > other.m_name); }

bool NamedEntity::deleted() const
{
   return m_deleted.toBool();
}

bool NamedEntity::display() const
{
   return m_display.toBool();
}

// Sigh. New databases, more complexity
void NamedEntity::setDeleted(const bool var, bool cachedOnly)
{
   if ( cachedOnly || setEasy(PropertyNames::NamedEntity::deleted, Brewtarget::dbBoolean(var)) ) {
      m_deleted = var;
   }
}

void NamedEntity::setDisplay(bool var, bool cachedOnly)
{
   if ( cachedOnly || setEasy(PropertyNames::NamedEntity::display, Brewtarget::dbBoolean(var)) ) {
      m_display = var;
   }
}

QString NamedEntity::folder() const
{
   return m_folder;
}

void NamedEntity::setFolder(const QString var, bool signal, bool cachedOnly)
{
   if ( cachedOnly || setEasy( PropertyNames::NamedEntity::folder, var ) ) {
      m_folder = var;
   }

   if ( signal )
      emit changedFolder(var);
}

QString NamedEntity::name() const
{
   return m_name;
}

void NamedEntity::setName(const QString var, bool cachedOnly)
{

   m_name = var;
   if ( ! cachedOnly ) {
      setEasy( PropertyNames::NamedEntity::name, var );
      emit changedName(var);
   }
}

int NamedEntity::key() const
{
   return m_key;
}

Brewtarget::DBTable NamedEntity::table() const
{
   return m_table;
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

// Loathsomeness waits and dreams in the deep, and decay spreads over the tottering cities of men.
bool NamedEntity::setEasy(QString prop_name, QVariant value, bool notify, bool updateEntry)
{
   QString className = this->metaObject()->className();

   // you can change the recipe and the mash without trying to version
   if ( className == QStringLiteral("Recipe") ||
        className == QStringLiteral("Mash")   ||
        className == QStringLiteral("BrewNote") ||
        updateEntry ) {
      Database::instance().updateEntry(this,prop_name,value,notify);
      return true;
   }
   else {
      return Database::instance().modifyEntry(this, prop_name, value);
   }
}

void NamedEntity::signalCacheChange(QString propName, QVariant value)
{
   int idx = this->metaObject()->indexOfProperty(propName.toUtf8().data());
   QMetaProperty mProp = this->metaObject()->property(idx);
   emit changed(mProp,value);
}

QVariant NamedEntity::get( const QString& col_name ) const
{
   return Database::instance().get( m_table, m_key, col_name );
}

void NamedEntity::setInventory( const QVariant& value, int invKey, bool notify )
{
   Database::instance().setInventory( this, value, invKey, notify );
}

QVariant NamedEntity::getInventory() const
{
   QVariant val = 0.0;
   val = Database::instance().getInventoryAmt(m_table, m_key);
   return val;
}

void NamedEntity::setParent(NamedEntity const & parentNamedEntity)
{
   this->parentKey = parentNamedEntity.m_key;
   return;
}

QVariantMap NamedEntity::getColumnValueMap() const
{
   QVariantMap map;
   map.insert(PropertyNames::NamedEntity::folder, folder());
   map.insert(PropertyNames::NamedEntity::name, name());
   return map;
}
