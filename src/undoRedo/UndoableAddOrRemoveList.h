/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * undoRedo/UndoableAddOrRemoveList.h is part of Brewtarget, and is copyright the following authors 2021-2025:
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
 =====================================================================================================================*/
#ifndef UNDOREDO_UNDOABLEADDORREMOVELIST_H
#define UNDOREDO_UNDOABLEADDORREMOVELIST_H
#pragma once

#include <QList>
#include <QString>
#include <QUndoCommand>
#include <QVariant>

#include "undoRedo/UndoableAddOrRemove.h"

/*!
 * \class UndoableAddOrRemoveList
 *
 * \brief A version of \c that handles adding/removing lists of things to/from a recipe etc.
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
class UndoableAddOrRemoveList : public QUndoCommand {
public:
   /*!
    * \param updatee The object we are updating - eg recipe
    * \param doer The method on the updatee to do the addition or removal.
    *             This should return the object that needs to be passed in to the \c undoer method.
    *             For certain classes - eg Fermentable, Hop - adding an object actually adds a new object which is
    *             the "child" of the one passed in.  Thus adding a Fermentable to a recipe returns the new copy
    *             ("child") Fermentable that was created; removing a Fermentable from a recipe returns its original
    *             "parent" Fermentable.
    * \param listToAddOrRemove The list of things we're adding or removing - eg fermentable
    * \param undoer The method on the updatee to undo the addition or removal
    * \param doCallback The free function (usually in \c Undoable.h) to call after doing/redoing the change - typically
    *                   calling in to \c MainWindow to update other display elements.  If \c null, no callback is made.
    * \param undoCallback The free function (usually in \c Undoable.h) to call after undoing the change - typically
    *                     calling in to \c MainWindow to update other display elements.  If \c null, no callback is
    *                     made.
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Style"
    * \param parent This is for grouping updates together.
    */
   UndoableAddOrRemoveList(UU & updatee,
                           std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                           QList<VV *> listToAddOrRemove,
                           std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                           void (*doCallback)(std::shared_ptr<VV>),
                           void (*undoCallback)(std::shared_ptr<VV>),
                           QString const & description,
                           QUndoCommand * parent = nullptr) : QUndoCommand(parent) {
      // Parent class handles storing description and making it accessible to the undo stack etc - we just have to give
      // it the text.
      this->setText(description);

      //
      // We don't need to do anything in this class other than create an UndoableAddOrRemove object for each item in
      // listToAddOrRemove.  Setting the parent field on a QUndoCommand makes that parent the owner, responsible for
      // invoking, deleting, etc.  WE don't have to do anything more as the base class (QUndoCommand) does all the
      // work.
      //
      for (auto ii : listToAddOrRemove) {
         // Doesn't matter what description we pass in to these child objects as it will never be seen.  Might as well
         // give them the same one as the parent/grouping object.
         new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                             doer,
                                             ObjectStoreWrapper::getSharedFromRaw(ii),
                                             undoer,
                                             doCallback,
                                             undoCallback,
                                             description,
                                             this);
      }

      return;
   }

   /**
    * \brief Alternate constructor for when we have a list of shared pointers
    */
   UndoableAddOrRemoveList(UU & updatee,
                           std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                           QList<std::shared_ptr<VV>> listToAddOrRemove,
                           std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                           void (*doCallback)(std::shared_ptr<VV>),
                           void (*undoCallback)(std::shared_ptr<VV>),
                           QString const & description,
                           QUndoCommand * parent = nullptr) : QUndoCommand(parent) {
      this->setText(description);
      for (auto ii : listToAddOrRemove) {
         new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                             doer,
                                             ii,
                                             undoer,
                                             doCallback,
                                             undoCallback,
                                             description,
                                             this);
      }

      return;
   }

   ~UndoableAddOrRemoveList() = default;

private:
};


/*!
 * \brief Helper function that allows UndoableAddOrRemoveList to be instantiated with automatic template argument deduction.
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemoveList<BB, UU, VV> * newUndoableAddOrRemoveList(UU & updatee,
                                                                 std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                                 QList<VV *> listToAddOrRemove,
                                                                 std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                                 void (*doCallback)(std::shared_ptr<VV>),
                                                                 void (*undoCallback)(std::shared_ptr<VV>),
                                                                 QString const & description,
                                                                 QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemoveList<BB, UU, VV>(updatee,
                                                  doer,
                                                  listToAddOrRemove,
                                                  undoer,
                                                  doCallback,
                                                  undoCallback,
                                                  description,
                                                  parent);
}

template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemoveList<BB, UU, VV> * newUndoableAddOrRemoveList(UU & updatee,
                                                                 std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                                 QList<std::shared_ptr<VV>> listToAddOrRemove,
                                                                 std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                                 void (*doCallback)(std::shared_ptr<VV>),
                                                                 void (*undoCallback)(std::shared_ptr<VV>),
                                                                 QString const & description,
                                                                 QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemoveList<BB, UU, VV>(updatee,
                                                  doer,
                                                  listToAddOrRemove,
                                                  undoer,
                                                  doCallback,
                                                  undoCallback,
                                                  description,
                                                  parent);
}

/*!
 * \brief Helper function that allows UndoableAddOrRemoveList to be instantiated with automatic template argument deduction.
 *
 *        This is useful when there are no callbacks, otherwise caller has to do a static cast on null pointer
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemoveList<BB, UU, VV> * newUndoableAddOrRemoveList(UU & updatee,
                                                                 std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                                 QList<VV *> listToAddOrRemove,
                                                                 std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                                 QString const & description,
                                                                 QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemoveList<BB, UU, VV>(updatee,
                                                  doer,
                                                  listToAddOrRemove,
                                                  undoer,
                                                  static_cast<void (*)(std::shared_ptr<VV>)>(nullptr),
                                                  static_cast<void (*)(std::shared_ptr<VV>)>(nullptr),
                                                  description,
                                                  parent);
}

template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemoveList<BB, UU, VV> * newUndoableAddOrRemoveList(UU & updatee,
                                                                 std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                                 QList<std::shared_ptr<VV>> listToAddOrRemove,
                                                                 std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                                 QString const & description,
                                                                 QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemoveList<BB, UU, VV>(updatee,
                                                  doer,
                                                  listToAddOrRemove,
                                                  undoer,
                                                  static_cast<void (*)(std::shared_ptr<VV>)>(nullptr),
                                                  static_cast<void (*)(std::shared_ptr<VV>)>(nullptr),
                                                  description,
                                                  parent);
}

#endif
