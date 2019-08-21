/*
 * TableSchema.h is part of Brewtarget, and is Copyright the following
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
#ifndef _TABLESCHEMA_H
#define _TABLESCHEMA_H

#include "PropertySchema.h"
#include "brewtarget.h"
#include "database.h"
#include <QString>

class DatabaseSchema;

class TableSchema : QObject
{

   friend DatabaseSchema;
   friend Database;

   Q_OBJECT
public:

   const QString tableName();
   const Brewtarget::DBTable dbTable();
   const QMap<QString, PropertySchema*> properties() const;

   const PropertySchema* property(QString prop) const;

   // returns all of the column names
   const QStringList propertyToColumn(QString prop) const;
   // returns just one for the database type at hand
   const QString propertyToColumn(QString prop, Brewtarget::DBTypes type) const;

   // get the type name for this column
   const QString propertyColumnType(QString prop) const;
   // get teh XML tag for this column
   const QString propertyToXml(QString prop) const;
   // get the default value for this column
   const QVariant propertyColumnDefault(QString prop) const;
   // get the column size of the property's column
   const int propertyColumnSize(QString prop) const;

   // given an XML tag, get the associated property name
   const QString xmlToProperty(QString xmlName) const;

   const QStringList allPropertyNames() const;
   const QStringList allColumnNames(Brewtarget::DBTypes type = Brewtarget::NODB) const;
private:

   // You can create this either via tableName or DBTable
   TableSchema(QString tablename);
   TableSchema(Brewtarget::DBTable dbTable);

   QString tableName_;
   Brewtarget::DBTable dbTable_;
   QMap<QString,PropertySchema*> properties_;

   QMap<QString,PropertySchema*> defineTable(Brewtarget::DBTable table);
   QMap<QString,PropertySchema*> defineStyleTable();
   QMap<QString,PropertySchema*> defineEquipmentTable();
   QMap<QString,PropertySchema*> defineFermentableTable();
   QMap<QString,PropertySchema*> defineHopTable();
   QMap<QString,PropertySchema*> defineMashTable();
   QMap<QString,PropertySchema*> defineMashstepTable();
   QMap<QString,PropertySchema*> defineMiscTable();
   QMap<QString,PropertySchema*> defineRecipeTable();
   QMap<QString,PropertySchema*> defineYeastTable();
};

#endif
