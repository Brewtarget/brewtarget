/*
 * SetterCommand.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QModelIndexList>
#include "SetterCommand.h"

SetterCommand::SetterCommand( QSqlRelationalTableModel* table, const char* key_name, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify)
   : QUndoCommand(QString("Change %1 to %2").arg(col_name).arg(value.toString()))
{
   appendCommand( table, key_name, key, col_name, value, prop, object, notify );
}

SetterCommand::~SetterCommand()
{
}

void SetterCommand::appendCommand( QSqlRelationalTableModel* table,
                  QString const& key_name,
                  int key,
                  QString const& col_name,
                  QVariant value,
                  QMetaProperty prop,
                  BeerXMLElement* object,
                  bool n,
                  QVariant oldValue)
{
   tables.append(table);
   key_names.append(key_name);
   keys.append(key);
   col_names.append(col_name);
   values.append(value);
   props.append(prop);
   objects.append(object);
   notify.append(n);
   
   oldValues.append(oldValue);
}

QList<QSqlQuery> SetterCommand::setterStatements()
{
   QList<QSqlQuery> ret;
   QString str;
   
   int i, size;
   size = tables.size();
   if( size <= 0 )
      return ret;
   
   // Construct the statements.
   for( i = 0; i < size; ++i )
   {
      QSqlQuery q( tables[i]->database() );
      str = QString("UPDATE `%1` SET `%2`= :value WHERE `%3`='%4'")
                .arg(tables.at(i)->tableName())
                .arg(col_names.at(i))
                .arg(key_names.at(i))
                .arg(keys.at(i));
      q.setForwardOnly(true); // Helps with speed/memory.
      q.prepare(str);
      q.bindValue(":value",values[i]);
      ret.append(q);
      
      //qDebug() << "Value: " << values[i].toString();
      //QVariant check = q.boundValues()[0];
      //qDebug() << "Bound Value: " << check.toString();
   }
   
   return ret;
}

QList<QSqlQuery> SetterCommand::undoStatements()
{
   QList<QSqlQuery> ret;
   QString str;
   int i, size;
   
   size = tables.size();
   if( size <= 0 )
      return ret;
   
   // Construct the transaction string.
   for( i = 0; i < size; ++i )
   {
      QSqlQuery q( tables[i]->database() );
      str = QString("UPDATE `%1` SET `%2` = :oldValue WHERE `%3`='%4'")
                .arg(tables.at(i)->tableName())
                .arg(col_names.at(i))
                .arg(key_names.at(i))
                .arg(keys.at(i));
      q.setForwardOnly(true);
      q.prepare(str);
      q.bindValue(":oldValue",oldValues[i]);
      ret.append(q);
   }
   
   return ret;
}

void SetterCommand::oldValueTransaction()
{
   QList<QSqlQuery> queries;
   QString str;
   
   int i, size;
   size = tables.size();
   if( size <= 0 )
      return;
   
   tables[0]->database().transaction();
   for( i = 0; i < size; ++i )
   {
      QSqlQuery q( tables[i]->database() );
      str = QString("SELECT `%1` FROM `%2` WHERE `%3`='%4'")
                .arg(col_names.at(i))
                .arg(tables.at(i)->tableName())
                .arg(key_names.at(i))
                .arg(keys.at(i));
      q.setForwardOnly(true);
      q.prepare(str);
      queries.append(q);
      q.exec();
   }
   tables[0]->database().commit();
   
   for( i = 0; i < size; ++i )
   {
      QSqlQuery q = queries[i];
      if( q.next() )
         oldValues[i] = q.record().value(0);
      else
         Brewtarget::logE( QString("SetterCommand::oldValueTransaction: %1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery()) );
   }
}

int SetterCommand::id() const
{
   // NOTE: should return an id unique to this class.
   // If there are two commands in a stack with same id,
   // they they may be merged with mergeWith() by the stack.
   
   return 0;
}

int SetterCommand::size() const
{
   return tables.size();
}

bool SetterCommand::mergeWith( const QUndoCommand* command )
{
   //SetterCommand* other = qobject_cast<SetterCommand*>(command);
   // NOTE: just gotta pray that you can do this cast?
   const SetterCommand* other = reinterpret_cast<const SetterCommand*>(command);
   if( other == 0 )
      return false;
   
   int i, size;
   size = other->size();
   for( i = 0; i < size; ++i )
   {
      appendCommand(
         other->tables[i],
         other->key_names[i],
         other->keys[i],
         other->col_names[i],
         other->values[i],
         other->props[i],
         other->objects[i],
         other->notify[i],
         other->oldValues[i]
      );
   }
   
   return true;
}

void SetterCommand::redo()
{   
   int i, size;
   size = tables.size();
   if( size <= 0 )
      return;
   
   // Get the old values.
   oldValueTransaction();
   
   // Set the new values.
   tables[0]->database().transaction();
   QList<QSqlQuery> queries = setterStatements();
   size = queries.size();
   for( i = 0; i < size; ++i )
   {
      if( ! queries[i].exec() )
         Brewtarget::logE( QString("SetterCommand::redo: %1.\n   \"%2\"").arg(queries[i].lastError().text()).arg(queries[i].lastQuery()) );
   }
   tables[0]->database().commit();

   // Emit signals.
   for( i = 0; i < size; ++i )
   {
      queries[i].finish();
      if( notify.at(i) )
         emit objects[i]->changed(props[i],values[i]);
   }
}

void SetterCommand::undo()
{
   
   int i, size;
   size = tables.size();
   
   // Set back the old values.
   tables[0]->database().transaction();
   QList<QSqlQuery> queries = undoStatements();
   size = queries.size();
   for( i = 0; i < size; ++i )
   {
      if( ! queries[i].exec() )
         Brewtarget::logE( QString("SetterCommand::undo: %1.\n   \"%2\"").arg(queries[i].lastError().text()).arg(queries[i].lastQuery()) );
   }
   tables[0]->database().commit();
   
   // Emit signals.
   for( i = 0; i < size; ++i )
   {
      queries[i].finish();
      if( notify.at(i) )
         emit objects[i]->changed(props[i],oldValues[i]);
   }
}
