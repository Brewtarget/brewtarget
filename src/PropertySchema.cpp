/*
 * PropertySchema.cpp is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@gmail.com>
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

#include "brewtarget.h"
#include <QString>
#include "PropertySchema.h"

PropertySchema::PropertySchema(QString propName)
    : QObject(0),
    m_propName(propName),
    m_colNames(QHash<Brewtarget::DBTypes,QString>()),
    m_xmlName(QString()),
    m_colType(QString()),
    m_defaultValue(QVariant()),
    m_colSize(0),
    m_fkey(false),
    m_ftable(Brewtarget::NOTABLE)
{
}

PropertySchema::PropertySchema(QString propName, 
            QHash<Brewtarget::DBTypes,QString> colNames, 
            QString xmlName,
            QString colType, QVariant defaultValue, 
            int colSize,
            bool fKey,
            Brewtarget::DBTable fTable)
    : QObject(0),
    m_propName(propName),
    m_colNames(colNames),
    m_xmlName(xmlName),
    m_colType(colType),
    m_defaultValue(defaultValue),
    m_colSize(colSize),
    m_fkey(fKey),
    m_ftable(fTable)
{
}

const QHash<Brewtarget::DBTypes,QString> PropertySchema::colNames() const { return m_colNames; }
const QString PropertySchema::propName() const { return m_propName; }
const QString PropertySchema::colType() const { return m_colType; }
const QString PropertySchema::xmlName() const { return m_xmlName; }
const QVariant PropertySchema::defaultValue() const { return m_defaultValue; }
const int PropertySchema::colSize() const { return m_colSize; }
const bool PropertySchema::fKey() const { return m_fkey; }
const Brewtarget::DBTable PropertySchema::fTable() const { return m_ftable; }

const QString PropertySchema::colName(Brewtarget::DBTypes type) const
{
   QString retval = QString();

   if ( m_colNames.contains(type) ) {
      retval = m_colNames.value(type);
   }
   else {
      retval = m_colNames.value(Brewtarget::NODB);
   }

   return retval;
}

void PropertySchema::setColNames(QHash<Brewtarget::DBTypes, QString> names)
{
   QHash<Brewtarget::DBTypes, QString>::const_iterator i = names.constBegin();
   m_colNames.clear();

   while ( i != names.constEnd() ) {
      m_colNames.insert( i.key(), i.value());
   }
}

void PropertySchema::setColName(Brewtarget::DBTypes type, QString name)
{
   m_colNames.insert(type, name);
}

void PropertySchema::setXmlName(QString xmlName)
{
   m_xmlName = xmlName;
}

void PropertySchema::setColType(QString type)
{
   m_colType = type;
}

void PropertySchema::setDefaultValue(QVariant defVal)
{
   m_defaultValue = defVal;
}

void PropertySchema::setColSize(int size)
{
   m_colSize = size;
}

void PropertySchema::setFKey(bool fkey)
{
   m_fkey = fkey;
}

void PropertySchema::setFTable(Brewtarget::DBTable ftable) 
{
   m_ftable = ftable;
}

