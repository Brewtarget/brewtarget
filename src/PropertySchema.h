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
   const QString colType() const;
   const QString xmlName() const;
   const QVariant defaultValue() const;
   const int colSize() const;

   // sets
   // NOTE: I am specifically not allowing the propName to be set. Do that
   // when you instantiate the object if you must
   void setColNames(QHash<Brewtarget::DBTypes,QString> names);
   void setColName(Brewtarget::DBTypes type,QString column);
   void setXmlName(QString xmlName);
   void setColType(QString type);
   void setDefaultValue(QVariant defVal);
   void setColSize(int size);

private:
   PropertySchema(QString propName);
   PropertySchema(QString propName, 
                  QHash<Brewtarget::DBTypes,QString> colNames, 
                  QString xmlName = QString(""),
                  QString colType = QString("double"), 
                  QVariant defaultValue = QVariant(0.0),
                  int colSize=0);

   QString propName_;
   QHash<Brewtarget::DBTypes,QString> colNames_;
   QString xmlName_;
   QString colType_;
   QVariant defaultValue_;
   int colSize_;

};

#endif // _PROPERTYSCHEMA_H
