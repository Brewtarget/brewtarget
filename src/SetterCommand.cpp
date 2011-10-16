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

SetterCommand::SetterCommand( QSqlRelationalTableModel* table, int key, const char* col_name, QVariant value, QMetaProperty prop, BeerXMLElement* object)
   : QUndoCommand(QString("Change %1 to %2").arg(col_name).arg(value)),
     table(table), key(key), prop(prop), col_name(col_name), value(value), object(object)
{
}

SetterCommand::~SetterCommand()
{
}

void SetterCommand::redo()
{
   // Get the current filter.
   QString filter = table->filter();
   
   // Makes the only visible row the one that has our key.
   table->setFilter( QString("key=%1").arg(key) );
   
   // Record the old data for undo.
   oldValue = table->record(0).value( col_name );
   
   // Change the data.
   table->record(0).setValue( col_name, value );
   
   // Unset the filter.
   table->setFilter(filter);
   table->select();
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   emit object->changed(prop,value);
}

void SetterCommand::undo()
{
   // Get the current filter.
   QString filter = table->filter();
   
   // Makes the only visible row the one that has our key.
   table->setFilter( QString("key=%1").arg(key) );
   
   // Change the data back
   table->record(0).setValue( col_name, oldValue );
   
   // Unset the filter.
   table->setFilter(filter);
   table->select();
   
   // Emit the notifier.
   //prop.notifySignal().invoke( object, Q_ARG(QMetaProperty, prop), Q_ARG(QVariant, value) );
   emit object->changed(prop,value);
}