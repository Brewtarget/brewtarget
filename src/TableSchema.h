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
   Brewtarget::DBTable dbTable();
   const QMap<QString, PropertySchema*> properties() const;
   const QMap<QString, PropertySchema*> foreignKeys() const;

   const PropertySchema* property(QString prop) const;

   // returns all of the column names
   const QStringList propertyToColumn(QString prop) const;

   // returns just one for the database type at hand
   const QString propertyToColumn(QString prop, Brewtarget::DBTypes type) const;

   // get the type name for this column
   const QString propertyColumnType(QString prop) const;
   // get the XML tag for this column
   const QString propertyToXml(QString prop) const;
   // get the default value for this column
   const QVariant propertyColumnDefault(QString prop) const;
   // get the column size of the property's column
   int propertyColumnSize(QString prop) const;

   // given an XML tag, get the associated property name
   const QString xmlToProperty(QString xmlName) const;

   const QStringList allPropertyNames() const;
   const QStringList allColumnNames(Brewtarget::DBTypes type = Brewtarget::NODB) const;

   const  QStringList allForeignKeyNames() const;
   const QStringList allForeignKeyColumnNames(Brewtarget::DBTypes type = Brewtarget::NODB) const;

private:

   // I only allow table schema to be made with a DBTable constant
   // It saves a lot of work, and I think the name to constant
   // mapping doesn't belong here -- it belongs in DatabaseSchema
   TableSchema(Brewtarget::DBTable dbTable);

   QString m_tableName;
   Brewtarget::DBTable m_dbTable;

   QMap<QString,PropertySchema*> m_properties;
   QMap<QString,PropertySchema*> m_foreignKeys;

   void defineTable();
   void defineStyleTable();
   void defineEquipmentTable();
   void defineFermentableTable();
   void defineHopTable();
   void defineMashTable();
   void defineMashstepTable();
   void defineMiscTable();
   void defineRecipeTable();
   void defineYeastTable();
   void defineBrewnoteTable();

   // and we can get away with one method for the child tables
   void defineChildTable(Brewtarget::DBTable table);

   // and one method for all the in_recipe tables
   void defineInRecipeTable(QString childIdx, Brewtarget::DBTable table);

   // one method for all the bt_tables
   void defineBtTable(QString childIdx, Brewtarget::DBTable table);

   // Inventory tables are strange and I didn't feel quite comfortable trying to make one
   // method for all of them;
   void defineFermInventoryTable();
   void defineHopInventoryTable();
   void defineMiscInventoryTable();
   void defineYeastInventoryTable();

};

#endif
