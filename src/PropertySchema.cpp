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
    propName_(propName),
    colNames_(QHash<Brewtarget::DBTypes,QString>()),
    xmlName_(QString()),
    colType_(QString()),
    defaultValue_(QVariant()),
    colSize_(0)
{
}

PropertySchema::PropertySchema(QString propName, 
            QHash<Brewtarget::DBTypes,QString> colNames, 
            QString xmlName,
            QString colType, QVariant defaultValue, 
            int colSize)
    : QObject(0),
    propName_(propName),
    colNames_(colNames),
    xmlName_(xmlName),
    colType_(colType),
    defaultValue_(defaultValue),
    colSize_(colSize)
{
}

const QHash<Brewtarget::DBTypes,QString> PropertySchema::colNames() const { return colNames_; }
const QString PropertySchema::propName() const { return propName_; }
const QString PropertySchema::colType() const { return colType_; }
const QString PropertySchema::xmlName() const { return xmlName_; }
const QVariant PropertySchema::defaultValue() const { return defaultValue_; }
const int PropertySchema::colSize() const { return colSize_; }

const QString PropertySchema::colName(Brewtarget::DBTypes type) const
{
   QString retval = QString();

   if ( colNames_.contains(type) ) {
      retval = colNames_.value(type);
   }
   else {
      retval = colNames_.value(Brewtarget::NODB);
   }

   return retval;
}

void PropertySchema::setColNames(QHash<Brewtarget::DBTypes, QString> names)
{
   QHash<Brewtarget::DBTypes, QString>::const_iterator i = names.constBegin();
   colNames_.clear();

   while ( i != names.constEnd() ) {
      colNames_.insert( i.key(), i.value());
   }
}

void PropertySchema::setColName(Brewtarget::DBTypes type, QString name)
{
   colNames_.insert(type, name);
}

void PropertySchema::setXmlName(QString xmlName)
{
   xmlName_ = xmlName;
}

void PropertySchema::setColType(QString type)
{
   colType_ = type;
}

void PropertySchema::setDefaultValue(QVariant defVal)
{
   defaultValue_ = defVal;
}

void PropertySchema::setColSize(int size)
{
   colSize_ = size;
}
