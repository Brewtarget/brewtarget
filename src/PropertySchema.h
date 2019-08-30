/*
 * PropertySchema.h is part of Brewtarget, and is Copyright the following
 * authors 2019-2024
 * - Mik Firestone <mikfire@fastmail.com>
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
#ifndef _PROPERTYSCHEMA_H
#define _PROPERTYSCHEMA_H

#include "brewtarget.h"
#include <QString>

// Needs the forward declaration
class TableSchema;

class PropertySchema : QObject
{

   friend TableSchema;

   Q_OBJECT
public:
   virtual ~PropertySchema() {}

   // gets
   const QString propName() const;
   const QHash<Brewtarget::DBTypes,QString> colNames() const;
   const QString colName(Brewtarget::DBTypes type) const;
   const QString xmlName() const;
   const QString colType() const;
   const QVariant defaultValue() const;
   int colSize() const;
   Brewtarget::DBTable fTable() const;

   // sets
   // NOTE: I am specifically not allowing the propName to be set. Do that
   // when you instantiate the object if you must
   void setColNames(QHash<Brewtarget::DBTypes,QString> names);
   void setColName(Brewtarget::DBTypes type,QString column);
   void setXmlName(QString xmlName);
   void setColType(QString type);
   void setDefaultValue(QVariant defVal);
   void setColSize(int size);
   void setFTable(Brewtarget::DBTable fTable);

private:
   PropertySchema(QString propName);
   PropertySchema(QString propName, 
                  QHash<Brewtarget::DBTypes,QString> colNames, 
                  QString xmlName = QString(""),
                  QString colType = QString("double"), 
                  QVariant defaultValue = QVariant(0.0),
                  int colSize = 0);

   PropertySchema(QString propName, 
                  QHash<Brewtarget::DBTypes,QString> colNames, 
                  Brewtarget::DBTable fTable);
   QString m_propName;
   QHash<Brewtarget::DBTypes,QString> m_colNames;
   QString m_xmlName;
   QString m_colType;
   QVariant m_defaultValue;
   int m_colSize;
   Brewtarget::DBTable m_ftable;

};

#endif // _PROPERTYSCHEMA_H
