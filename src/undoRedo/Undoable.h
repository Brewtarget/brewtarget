/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * undoRedo/Undoable.h is part of Brewtarget, and is copyright the following authors 2020-2025:
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
#ifndef UNDOREDO_UNDOABLE_H
#define UNDOREDO_UNDOABLE_H
#pragma once

#include <memory>

#include <QString>
#include <QUndoCommand>
#include <QUndoStack>

#include "model/NamedEntity.h"
#include "undoRedo/SimpleUndoableUpdate.h"
#include "utils/TypeLookup.h"

namespace Undoable {
   /**
    * \brief Returns the main undo-redo stack for the program
    */
   QUndoStack & getStack();

   /**
    * \brief Doing updates via this function makes them undoable (and redoable).  This is the most generic version
    *        which requires the caller to construct a QUndoCommand.
    */
   void doOrRedoUpdate(QUndoCommand * update);

   //! \brief Doing updates via this method makes them undoable (and redoable).  This is the simplified version
   //         which suffices for modifications to most individual non-relational attributes.
   template<typename T>
   void doOrRedoUpdate(NamedEntity & updatee,
                       TypeInfo const & typeInfo,
                       T newValue,
                       QString const & description,
                       [[maybe_unused]] QUndoCommand * parent = nullptr) {
      doOrRedoUpdate(new SimpleUndoableUpdate(updatee, typeInfo, newValue, description));
      return;
   }

   /**
    * \brief This version of \c doOrRedoUpdate is needed when updating a property that has (or might have) a non-trivial
    *        \c PropertyPath
    */
   template<typename T>
   void doOrRedoUpdate(NamedEntity & updatee,
                       PropertyPath const & propertyPath,
                       TypeInfo const & typeInfo,
                       T newValue,
                       QString const & description,
                       [[maybe_unused]] QUndoCommand * parent = nullptr) {
      doOrRedoUpdate(new SimpleUndoableUpdate(updatee, propertyPath, typeInfo, newValue, description));
      return;
   }

   /**
    * \brief Actually add the new mash step to (the mash of) the recipe (in an undoable way).  Same for boil steps,
    *        fermentation steps, inventory changes, etc.
    */
   template<class ItemOwnerClass, class ItemClass>
   void addEnumeratedItemToOwner(ItemOwnerClass & itemOwner, std::shared_ptr<ItemClass> item);
   template<class ItemOwnerClass, class ItemClass>
   void addEnumeratedItemToOwner(std::shared_ptr<ItemOwnerClass> itemOwner, std::shared_ptr<ItemClass> item) {
      addEnumeratedItemToOwner(*itemOwner, item);
      return;
   }
}

#endif
