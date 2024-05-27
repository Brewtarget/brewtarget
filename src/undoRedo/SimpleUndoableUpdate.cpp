/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * undoRedo/SimpleUndoableUpdate.cpp is part of Brewtarget, and is copyright the following authors 2020-2023:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "undoRedo/SimpleUndoableUpdate.h"

#include <QDebug>

#include "Logging.h"

SimpleUndoableUpdate::SimpleUndoableUpdate(NamedEntity & updatee,
                                           TypeInfo const & typeInfo,
                                           QVariant newValue,
                                           QString const & description,
                                           QUndoCommand * parent) :
   SimpleUndoableUpdate(updatee,
                        typeInfo.propertyName,
                        typeInfo,
                        newValue,
                        description,
                        parent) {
   return;
}


SimpleUndoableUpdate::SimpleUndoableUpdate(NamedEntity & updatee,
                                           PropertyPath const propertyPath,
                                           TypeInfo const & typeInfo,
                                           QVariant newValue,
                                           QString const & description,
                                           QUndoCommand * parent) :
   QUndoCommand  {parent},
   m_updatee     {updatee},
   m_propertyPath{propertyPath},
   m_typeInfo    {typeInfo},
   m_oldValue    {m_propertyPath.getValue(m_updatee)},
   m_newValue    {newValue} {
   // Uncomment this log message if the assert below is tripping, as it will usually help find the bug quickly
//   qDebug().noquote() <<
//      Q_FUNC_INFO << this->m_updatee.metaObject()->className() << "#" << this->m_updatee.key() << "; Property path:" <<
//      this->m_propertyPath << "; Type Info:" << this->m_typeInfo << "; New Value:" << this->m_newValue << "; Old Value:" <<
//      this->m_oldValue << "; Stack trace:" << Logging::getStackTrace();
   Q_ASSERT(this->m_oldValue.isValid() && "Trying to update non-existent property");

   this->setText(description);
   return;
}

SimpleUndoableUpdate::~SimpleUndoableUpdate() {
   return;
}

void SimpleUndoableUpdate::redo() {
   QUndoCommand::redo();
   this->undoOrRedo(false);
   return;
}

void SimpleUndoableUpdate::undo() {
   QUndoCommand::undo();
   this->undoOrRedo(true);
   return;
}

bool SimpleUndoableUpdate::undoOrRedo(bool const isUndo) {
   // This is where we call the setter for propertyName on updatee, via the magic of the Qt Property System
   bool success = this->m_propertyPath.setValue(this->m_updatee, isUndo ? this->m_oldValue : this->m_newValue);

   // It's a coding error if we tried to update a non-existent property
   if (!success) {
      qCritical() <<
         Q_FUNC_INFO << "Could not" << (isUndo ? "undo" : "(re)do") << " update of " <<
         this->m_updatee.metaObject()->className() << "propertyPath" << this->m_propertyPath << "with" <<
         (isUndo ? this->m_oldValue : this->m_newValue) << "(" << this->m_typeInfo << ")";
      qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
   }
   Q_ASSERT(success && "Trying to update non-existent property");
   return success;
}
