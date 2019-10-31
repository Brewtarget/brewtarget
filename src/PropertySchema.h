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

struct dbProp {
   QString m_propName;
   QString m_colName;
   QString m_xmlName;
   QString m_constraint;
   QString m_colType;
   QVariant m_defaultValue;
   int m_colSize;
   Brewtarget::DBTable m_ftable;
};

class PropertySchema : QObject
{

   friend class TableSchema;

   Q_OBJECT
public:
   virtual ~PropertySchema() {}

   // since I've removed all the things from the initializer, we need two
   // methods to actually add properties
   void addProperty(QString propName,
                   Brewtarget::DBTypes dbType = Brewtarget::ALLDB,
                   QString colName = QString(),
                   QString xmlName = QString(),
                   QString colType = QString(),
                   QVariant defaultValue = QVariant(),
                   int colSize = 0,
                   QString constraint = QString() );

   void addForeignKey(QString propName,
                      Brewtarget::DBTypes dbType = Brewtarget::ALLDB,
                      QString colName = QString(),
                      Brewtarget::DBTable fTable = Brewtarget::NOTABLE);

   // this may get revisited later, but if you either do not include the
   // dbType in the call or you send ALLDB, then you will get the default
   const QString propName(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   const QString colName(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   const QString xmlName(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   const QString constraint(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   const QString colType(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   const QVariant defaultValue(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   int colSize(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;
   Brewtarget::DBTable fTable(Brewtarget::DBTypes dbType = Brewtarget::ALLDB) const;

   // sets
   // NOTE: I am specifically not allowing the propName to be set. Do that
   // when you call addProperty or addForeignKey. I may
   void setColName(QString column, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setXmlName(QString xmlName, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setConstraint(QString constraint, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setColType(QString colType, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setDefaultValue(QVariant defVal, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setColSize(int size, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);
   void setFTable(Brewtarget::DBTable fTable, Brewtarget::DBTypes dbType = Brewtarget::ALLDB);

private:
   PropertySchema();
   // if you use this constructor, it will default to ALLDB
   PropertySchema( QString propName,
                   QString colName,
                   QString xmlName,
                   QString colType,
                   QVariant defVal,
                   QString constraint = QString(""),
                   int colSize = 0
   );

   PropertySchema( QString propName,
                   QString colName,
                   QString colType,
                   Brewtarget::DBTable fTable
   );

   // any given property has at least one element in this list and possible as
   // many as we support. Using a vector allows me to prepopulate
   QVector<dbProp*> m_properties;
};

#endif // _PROPERTYSCHEMA_H
