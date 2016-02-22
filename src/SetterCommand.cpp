/*
 * SetterCommand.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
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

#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QModelIndexList>
#include <QThread>
#include "SetterCommand.h"
#include "database.h"

SetterCommand::SetterCommand( Brewtarget::DBTable table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object, bool notify)
   : QUndoCommand(QString("Change %1 to %2").arg(col_name).arg(value.toString()))
{
   appendCommand( table, key, QString(col_name), value, prop, object, notify);
}

SetterCommand::~SetterCommand()
{
}

void SetterCommand::appendCommand( Brewtarget::DBTable table,
                  int key,
                  QString const& col_name,
                  QVariant value,
                  QMetaProperty prop,
                  BeerXMLElement* object,
                  bool n,
                  QVariant oldValue)
{
   tables.append(table);
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

   QList<Brewtarget::DBTable>::const_iterator tableIt, tableEnd;
   QList<QString>::const_iterator colNameIt;
   QList<int>::const_iterator keyIt;
   QList<QVariant>::const_iterator valueIt;
   tableIt = tables.constBegin();
   colNameIt = col_names.constBegin();
   keyIt = keys.constBegin();
   valueIt = values.constBegin();

   // Construct the statements.
   tableEnd = tables.constEnd();
   while( tableIt != tableEnd )
   {
      QSqlQuery q( Database::sqlDatabase() );
      q.setForwardOnly(true); // Helps with speed/memory.
      str = QString("UPDATE %1 SET %2=:value WHERE id=%3")
                .arg(Database::tableNames[*tableIt])
                .arg(*colNameIt)
                .arg(*keyIt);
      q.prepare(str);
      q.bindValue(":value",*valueIt);
      ret.append(q);

      ++tableIt;
      ++colNameIt;
      ++keyIt;
      ++valueIt;
   }

   return ret;
}

QList<QSqlQuery> SetterCommand::undoStatements()
{
   QList<QSqlQuery> ret;
   QString str;

   QList<Brewtarget::DBTable>::const_iterator tableIt, tableEnd;
   QList<QString>::const_iterator colNameIt;
   QList<int>::const_iterator keyIt;
   QList<QVariant>::const_iterator oldValueIt;
   tableIt = tables.constBegin();
   colNameIt = col_names.constBegin();
   keyIt = keys.begin();
   oldValueIt = oldValues.begin();

   // Construct the transaction string.
   tableEnd = tables.constEnd();
   while( tableIt != tableEnd )
   {
      QSqlQuery q( Database::sqlDatabase() );
      q.setForwardOnly(true);
      str = QString("UPDATE %1 SET %2 = :oldValue WHERE id=%3")
                .arg(Database::tableNames[*tableIt])
                .arg(*colNameIt)
                .arg(*keyIt);
      q.prepare(str);
      q.bindValue(":oldValue",*oldValueIt);
      ret.append(q);

      ++tableIt;
      ++colNameIt;
      ++keyIt;
      ++oldValueIt;
   }

   return ret;
}

void SetterCommand::oldValueTransaction()
{
   QList<QSqlQuery> queries;
   QList<QSqlQuery>::const_iterator qIt, qEnd;
   QString str;

   QList<Brewtarget::DBTable>::const_iterator tableIt, tableEnd;
   QList<QString>::const_iterator colNameIt;
   QList<int>::const_iterator keyIt;
   QList<QVariant>::const_iterator oldValueIt;
   tableIt = tables.constBegin();
   colNameIt = col_names.constBegin();
   keyIt = keys.begin();
   oldValueIt = oldValues.begin();

   tableEnd = tables.constEnd();
   QSqlQuery transBegin("BEGIN TRANSACTION", Database::sqlDatabase());
   while( tableIt != tableEnd )
   {
      QSqlQuery q( Database::sqlDatabase() );
      q.setForwardOnly(true);
      str = QString("SELECT %1 FROM %2 WHERE id=%3")
                .arg(*colNameIt)
                .arg(Database::tableNames[*tableIt])
                .arg(*keyIt);
      q.prepare(str);
      queries.append(q);
      q.exec();
      ++tableIt;
      ++colNameIt;
      ++keyIt;
   }
   QSqlQuery transCommit("COMMIT", Database::sqlDatabase());

   qEnd = queries.constEnd();
   oldValues.clear();
   for( qIt = queries.constBegin(); qIt != qEnd; ++qIt )
   {
      QSqlQuery q = *qIt;
      if( q.next() )
         oldValues.append(q.record().value(0));
      else if ( ! q.isValid() )
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

bool SetterCommand::sqlSuccess() { return _sqlSuccess; }

void SetterCommand::redo()
{
   int i, size;
   size = tables.size();
   if( size <= 0 )
      return;

   // Get the old values.
   oldValueTransaction();

   // Set the new values.
   QList<QSqlQuery> queries = setterStatements();

   try {
      foreach( QSqlQuery q, queries )
      {
         if( ! q.exec() )
         {
            QString e = QString("%1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery());
            q.finish();
            throw e;
         }

         q.finish();
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      _sqlSuccess = false;
      throw;
   }

   _sqlSuccess = true; 
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
   
   QList<QSqlQuery> queries = undoStatements();

   try {
      foreach( QSqlQuery q, queries )
      {
         if( ! q.exec() )
         {
            QString e = QString("%1.\n   \"%2\"").arg(q.lastError().text()).arg(q.lastQuery());
            q.finish();
            throw e;
         }
         q.finish();
      }
   }
   catch (QString e) {
      Brewtarget::logE( QString("%1 %2").arg(Q_FUNC_INFO).arg(e));
      _sqlSuccess = false;
   }

   _sqlSuccess = true; 
   // Emit signals.
   for( i = 0; i < size; ++i )
   {
      queries[i].finish();
      if( notify.at(i) )
         emit objects[i]->changed(props[i],oldValues[i]);
   }
}
