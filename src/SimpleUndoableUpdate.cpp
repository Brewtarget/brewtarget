/*
 * SimpleUndoableUpdate.cpp is part of Brewtarget, and is Copyright the following
 * authors 2020
 * - Matt Young <mfsy@yahoo.com>
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
#include "SimpleUndoableUpdate.h"
#include "brewtarget.h" // For logging

SimpleUndoableUpdate::SimpleUndoableUpdate(QObject & updatee,
                                           char const * const propertyName,
                                           QVariant newValue,
                                           QString const & description,
                                           QUndoCommand * parent)
   : QUndoCommand(parent), updatee(updatee), propertyName(propertyName), newValue(newValue)
{
   this->oldValue = this->updatee.property(this->propertyName);
   Q_ASSERT(this->oldValue.isValid() && "Trying to update non-existent property");

   this->setText(description);
   return;
}

SimpleUndoableUpdate::~SimpleUndoableUpdate()
{
   return;
}

void SimpleUndoableUpdate::redo()
{
   QUndoCommand::redo();
   this->undoOrRedo(false);
   return;
}

void SimpleUndoableUpdate::undo()
{
   QUndoCommand::undo();
   this->undoOrRedo(true);
   return;
}

bool SimpleUndoableUpdate::undoOrRedo(bool const isUndo)
{
   // This is where we call the setter for propertyName on updatee, via the magic of the Qt Property System
   bool success = this->updatee.setProperty(this->propertyName, isUndo ? this->oldValue : this->newValue);

   // It's a coding error if we tried to update a non-existent property
   Q_ASSERT(success && "Trying to update non-existent property");
   if (!success)
   {
      qCritical() << QString("Could not %1 update of %2 property %3").arg(isUndo ? "undo" : "redo").arg(this->updatee.metaObject()->className()).arg(propertyName);
   }
   return success;
}
