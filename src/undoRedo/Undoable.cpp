/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * undoRedo/Undoable.cpp is part of Brewktarget, and is copyright the following authors 2020-2025:
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
#include "undoRedo/Undoable.h"

#include "MainWindow.h"
#include "undoRedo/UndoableAddOrRemove.h"

QUndoStack & Undoable::getStack() {
   // Meyers singleton
   static QUndoStack undoStack;
   return undoStack;
}

void Undoable::doOrRedoUpdate(QUndoCommand * update) {
   // Caller's responsibility to provide us a valid update
   Q_ASSERT(update);

   QUndoStack & undoStack { Undoable::getStack() };
   undoStack.push(update);

   MainWindow::instance().setUndoRedoEnable();
   return;

}

template<class StepOwnerClass, class StepClass>
void Undoable::addStepToStepOwner(StepOwnerClass & stepOwner, std::shared_ptr<StepClass> step) {
   qDebug() << Q_FUNC_INFO;
   //
   // Mash/Boil/Fermentation Steps are a bit different from most other NamedEntity objects in that they don't really
   // have an independent existence.  Taking Mash as an example, if you ask a Mash to remove a MashStep then it will
   // also tell the ObjectStore to delete it, but, when we're adding a MashStep to a Mash it's easier (for eg the
   // implementation of undo/redo) if we add it to the ObjectStore before we call Mash::addMashStep().
   //
   // However, normally, at this point, the new step will already have been added to the DB by
   // EditorBase::doSaveAndClose.  So we are just belt-and-braces here checking whether it needs to be added.
   //
   if (step->key() < 0) {
      qWarning() << Q_FUNC_INFO << step->metaObject()->className() << "unexpectedly not in DB, so inserting it now.";
      ObjectStoreWrapper::insert(step);
   }

   Undoable::doOrRedoUpdate(
      newUndoableAddOrRemove(stepOwner,
                             &StepOwnerClass::add,
                             step,
                             &StepOwnerClass::remove,
                             QObject::tr("Add step to %1").arg(StepOwnerClass::localisedName()))
   );
   // We don't need to do anything further here.  The change to the mash/boil/ferementation will already have triggered
   // the necessary updates to the corresponding MashStepTableModel/BoilStepTableModel/etc.
   return;
}

// Instantiate the above so that it can be called from StepEditorBase etc.
template void Undoable::addStepToStepOwner(Boil         & stepOwner, std::shared_ptr<        BoilStep> step);
template void Undoable::addStepToStepOwner(Mash         & stepOwner, std::shared_ptr<        MashStep> step);
template void Undoable::addStepToStepOwner(Fermentation & stepOwner, std::shared_ptr<FermentationStep> step);
