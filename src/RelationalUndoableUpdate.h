/*
 * RelationalUndoableUpdate.h is part of Brewtarget, and is Copyright the following
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
#ifndef RELATIONAL_UNDOABLE_UPDATE_H
#define RELATIONAL_UNDOABLE_UPDATE_H

#include "brewtarget.h" // For logging
#include <QMetaType>
#include <QString>
#include <QUndoCommand>
#include <QVariant>
#include "model/Recipe.h"
#include "model/Style.h"
#include "StyleButton.h"

/*!
 * \class RelationalUndoableUpdate
 * \author Matt Young
 *
 * \brief Each instance of this class is a non-trivial undoable update to, eg, a recipe that cannot be represented with
 *        SimpleUndoableUpdate - eg because we're adding a link to another object.
 */
template<class UU, class VV>
class RelationalUndoableUpdate : public QUndoCommand
{
public:
   /*!
    * \param updatee The object we are updating
    * \param setter The setter method on the updatee - cannot be null!
    * \param oldValue The current value.
    *                 (Looked at passing the getter instead of the current value, as it makes the call look a bit more
    *                 elegant - and the caller is almost certainly going to have to call the getter anyway.  However,
    *                 not all getters are const functions - eg because of lazy loading from DB etc - so it's simpler to
    *                 have the caller just give us the current value.)
    * \param newValue The new value to assign
    * \param callback The method on MainWindow to call after doing/undoing/redoing the change - typically to update
    *                 other display elements.  If null, no callback is made.
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Style"
    * \param parent This is for grouping updates together.
    */
   RelationalUndoableUpdate(UU & updatee,
                            void (UU::*setter)(VV *),
                            VV * oldValue,
                            VV * newValue,
                            void (MainWindow::*callback)(void),
                            QString const & description,
                            QUndoCommand * parent = nullptr)
   : QUndoCommand(parent), updatee(updatee), setter(setter), oldValue(oldValue), newValue(newValue), callback(callback)
   {
      // Parent class handles storing description and making it accessible to the undo stack etc - we just have to give
      // it the text.
      this->setText(description);
      return;
   }

   ~RelationalUndoableUpdate()
   {
      return;
   }

   /*!
    * \brief Apply the update (including for the first time)
    */
   void redo()
   {
      QUndoCommand::redo();
      this->undoOrRedo(false);
      return;
   }

   /*!
    * \brief Undo applying the update
    */
   void undo()
   {
      QUndoCommand::undo();
      this->undoOrRedo(true);
      return;
   }

private:
   /*!
    * \brief Undo or redo applying the update
    * \param isUndo true for undo, false for redo
    */
   void undoOrRedo(bool const isUndo)
   {
      (this->updatee.*(this->setter))(isUndo ? this->oldValue : this->newValue);
      if (this->callback != nullptr) {
         (Brewtarget::mainWindow()->*(this->callback))();
      }
      return;
   }

   UU & updatee;
   void (UU::*setter)(VV *);
   VV * oldValue;
   VV * newValue;
   void (MainWindow::*callback)(void);
};


/*!
 * \brief Helper function that allows RelationalUndoableUpdate to be instantiated with automatic template argument
 *        deduction.
 *
 *        (I thought this might not be necessary with the introduction of Class Template Argument Deduction in C++17,
 *        but I think I must be missing something.)
 */
template<class UU, class VV> RelationalUndoableUpdate<UU, VV> * newRelationalUndoableUpdate(UU & updatee,
                                                                                            void (UU::*setter)(VV *),
                                                                                            VV * oldValue,
                                                                                            VV * newValue,
                                                                                            void (MainWindow::*callback)(void),
                                                                                            QString const & description,
                                                                                            QUndoCommand * parent = nullptr) {
   return new RelationalUndoableUpdate<UU, VV>(updatee, setter, oldValue, newValue, callback, description, parent);
}



#endif /*RELATIONAL_UNDOABLE_UPDATE_H*/
