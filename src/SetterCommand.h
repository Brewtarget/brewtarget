/*
 * SetterCommand.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2012.
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

class SetterCommand;

#include <QList>
#include <QVariant>
#include <QUndoCommand>
#include <QMetaProperty>
#include <QSqlRelationalTableModel>
#include "BeerXMLElement.h"

/*!
 * \class SetterCommand
 * \author Philip G. Lee
 *
 * A command that updates entries in an SQL table pertaining to a BeerXMLElement.
 */
class SetterCommand : public QUndoCommand
{
public:
   /*! A setter command that will set the entry specified by (table,key,col_name)
    * to value, and call prop's notify() method when done.
    */
   SetterCommand( QSqlRelationalTableModel* table,
                  const char* key_name,
                  int key,
                  const char* col_name,
                  QVariant value,
                  QMetaProperty prop,
                  BeerXMLElement* object,
                  bool notify=true);
   virtual ~SetterCommand();
   
   //! Reimplemented from QUndoCommand.
   virtual int id() const;
   //! Reimplemented from QUndoCommand.
   virtual bool mergeWith( const QUndoCommand* command );
   
   //! Reimplemented from QUndoCommand. Executes the command.
   virtual void redo();
   //! Reimplemented from QUndoCommand. Undoes the command.
   virtual void undo();
   
private:
   QList<QSqlRelationalTableModel*> tables;
   QList<QString> key_names;
   QList<int> keys;
   QList<QMetaProperty> props;
   QList<QString> col_names;
   QList<QVariant> values;
   QList<QVariant> oldValues;
   QList<BeerXMLElement*> objects;
   QList<bool> notify;
   
   //! Append a command to us.
   void appendCommand( QSqlRelationalTableModel* table,
                  QString const& key_name,
                  int key,
                  QString const& col_name,
                  QVariant value,
                  QMetaProperty prop,
                  BeerXMLElement* object,
                  bool notify,
                  QVariant oldValue = QVariant());

   //! \returns query statements for setting the values.
   QList<QSqlQuery> setterStatements();
   //! After execution, oldValues[] should be populated.
   void oldValueTransaction();
   //! \returns an unexecuted query for the transaction to rollback the values.
   QList<QSqlQuery> undoStatements();
   
   //! \returns how many commands we have.
   int size() const;
};