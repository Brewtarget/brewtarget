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
#include <QString>

class TableSchema : QObject
{

   Q_OBJECT

   friend class DatabaseSchemaHelper;
   friend class DatabaseSchema;
   friend class Database;
   friend class BeerXML;

public:

   enum TableType {
      BASE,
      INV,
      CHILD,
      INREC,
      BT,
      META
   };

   //!brief get this TableSchema's name
   const QString tableName() const;
   //!brief get the Brewtarget class associated with this table, if any
   const QString className() const;
   //!brief get the Brewtarget::DBTable for this TableSchema
   Brewtarget::DBTable dbTable() const;
   //!brief get the Brewtarget::DBTable for this TableSchema's child table, if any
   Brewtarget::DBTable childTable() const;
   //!brief get the Brewtarget::DBTable for this TableSchema's _in_recipe table, if any
   Brewtarget::DBTable inRecTable() const;
   //!brief get the Brewtarget::DBTable for this TableSchema's _inventory table, if any
   Brewtarget::DBTable invTable() const;
   //!brief get the Brewtarget::DBTable for this TableSchema's bt_ table, if any
   Brewtarget::DBTable btTable() const;
   //!brief get all the properties in this schema as a map <name,PropertySchema>
   const QMap<QString, PropertySchema*> properties() const;
   //!brief get all the foreign keys in this schema as a map <name,PropertySchema>
   const QMap<QString, PropertySchema*> foreignKeys() const;
   //!brief get the PropertySchema for the unique id
   const PropertySchema* key() const;

   // Things to do for properties

   //!brief Get the property object. Try not to use this?
   const PropertySchema* property(QString prop) const;
   //!brief some properties may be named differently (like inventory v quanta)
   const QString propertyName(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get the database column name for this property
   const QString propertyToColumn(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get the database column type
   const QString propertyColumnType(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get the XML tag for this column
   const QString propertyToXml(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get the default value for this column
   const QVariant propertyColumnDefault(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get the column size of the property's column
   int propertyColumnSize(QString prop, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief given an XML tag, get the associated property name
   const QString xmlToProperty(QString xmlName, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief returns the property to be used for the increment/decrement triggers
   const QString triggerProperty() const;

   //!brief get all the property names
   const QStringList allPropertyNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get all the database column names
   const QStringList allColumnNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get keys for the properties
   const QStringList allProperties() const;

   // things to do on foreign keys

   //!brief get a specific foreign key column name
   const QString foreignKeyToColumn(QString fkey, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief a lot of tables have one foreign key. This is a nice shortcut for that
   const QString foreignKeyToColumn(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   //!brief which table does this foreign key point to
   Brewtarget::DBTable foreignTable(QString fkey, Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief a lot of tables have one foreign key. This is a nice shortcut for that
   Brewtarget::DBTable foreignTable(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

   //!brief get all the foreign key property names
   const QStringList allForeignKeyNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get all the foreign key column names
   const QStringList allForeignKeyColumnNames(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;
   //!brief get keys for the foreign keys
   const QStringList allForeignKeys() const;

   //!brief Use this to get the not recipe_id index from an inrec table
   const QString inRecIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the child_id index from a children table
   const QString childIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the recipe_id from a inrec table
   const QString recipeIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief Use this to get the parent_id from a child table
   const QString parentIndexName(Brewtarget::DBTypes type = Brewtarget::ALLDB);

   // Not sure these belong here yet, but maybe

   //!brief generates a properly formatted CREATE TABLE
   const QString generateCreateTable(Brewtarget::DBTypes type = Brewtarget::ALLDB, QString tmpName = QString("") );

   //!brief generates a properly formatted UPDATE, and binds the proper ID key
   const QString generateUpdateRow(int key, Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief generates a properly formatted UPDATE, without binding the ID
   const QString generateUpdateRow(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief generate an INSERT into a table, including all foreign keys
   const QString generateInsertRow(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief generate an INSERT into a table, ignoring all of the foreign keys
   const QString generateInsertProperties(Brewtarget::DBTypes type = Brewtarget::ALLDB);
   //!brief generate a CREATE temp table, needed when dropping columns in SQLite
   const QString generateCopyTable( QString dest, Brewtarget::DBTypes type = Brewtarget::ALLDB);

   //!brief generates a decrementing trigger, based on database type
   const QString generateDecrementTrigger(Brewtarget::DBTypes type);
   //!brief generates an incrementing trigger, based on database type
   const QString generateIncrementTrigger(Brewtarget::DBTypes type);

   //!brief returns true if this table is an inventory table
   bool isInventoryTable();
   //!brief returns true if this table is a base table
   bool isBaseTable();
   //!brief returns true if this table is a _child table
   bool isChildTable();
   //!brief returns true if this table is an _in_rec table
   bool isInRecTable();
   //!brief returns true if this table is an bt_ table
   bool isBtTable();
   //!brief returns true if this table is a table about the database
   bool isMetaTable();

   // convenience for the name of the key (eg, id) field in the db
   const QString keyName(Brewtarget::DBTypes type = Brewtarget::ALLDB) const;

private:

   // I only allow table schema to be made with a DBTable constant
   // It saves a lot of work, and I think the name to constant
   // mapping doesn't belong here -- it belongs in DatabaseSchema
   TableSchema(Brewtarget::DBTable dbTable);

   QString m_tableName;
   QString m_className;
   Brewtarget::DBTable m_dbTable;
   TableType m_type;

   // these are only set by the base tables
   Brewtarget::DBTable m_childTable;
   Brewtarget::DBTable m_inRecTable;
   Brewtarget::DBTable m_invTable;
   Brewtarget::DBTable m_btTable;

   QString m_trigger;

   PropertySchema* m_key;
   QMap<QString,PropertySchema*> m_properties;
   QMap<QString,PropertySchema*> m_foreignKeys;
   // It all depends on the call I want to make. I can require the type on
   // every call to a TableSchema object which is dull, repetitive and makes
   // some already difficult to read calls harder to read. Or I can cache the
   // default in the table and use that if ALLDB is sent, which breaks the
   // metaphor.
   Brewtarget::DBTypes m_defType;

   // getter only. But this is private because only my dearest,
   // closest friends can do this
   Brewtarget::DBTypes defType() const;

   void defineTable();
   void defineStyleTable();
   void defineEquipmentTable();
   void defineFermentableTable();
   void defineHopTable();
   void defineInstructionTable();
   void defineMashTable();
   void defineMashstepTable();
   void defineMiscTable();
   void defineRecipeTable();
   void defineYeastTable();
   void defineWaterTable();
   void defineSaltTable();
   void defineBrewnoteTable();
   void defineSettingsTable();

   // and we can get away with one method for the child tables
   void defineChildTable(Brewtarget::DBTable table);

   // and almost one method for all the in_recipe tables
   void defineInRecipeTable(QString childIdx, Brewtarget::DBTable table);
   // Instructions in recipe actually carry information. Sigh.
   void defineInstructionInRecipeTable( QString childIdx, Brewtarget::DBTable table);

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
