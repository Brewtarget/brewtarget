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
#include "recipe.h"
#include "style.h"
#include "StyleButton.h"

/*!
 * \class RelationalUndoableUpdate
 * \author Matt Young
 *
 * \brief Each instance of this class is a non-trivial undoable update to a recipe that cannot be represented with
 *        SimpleUndoableUpdate - eg because we're adding a link to another object.
 */
template<class UU, class NV>
class RelationalUndoableUpdate : public QUndoCommand
{
public:
   /*!
    * \param updatee The object we are updating
    * \param setter The setter method on the updatee
    * \param newValue The new value to assign
    * \param callback The method on MainWindow to call after doing/undoing/redoing the change - typically to update other display elements
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Style"
    * \param parent This is for grouping updates together.  We don't currently use it.
    */
   RelationalUndoableUpdate(UU & updatee,
                            void (UU::*setter)(NV *),
                            NV * newValue,
                            void (MainWindow::*callback)(void),
                            QString const & description,
                            QUndoCommand * parent = nullptr)
   : QUndoCommand(nullptr), updatee(updatee), setter(setter), newValue(newValue), callback(callback)
   {
      this->oldValue = updatee.style();
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
      this->undoOrRedo(false);
      return;
   }

   /*!
    * \brief Undo applying the update
    */
   void undo()
   {
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
      (Brewtarget::mainWindow()->*(this->callback))();
      return;
   }

   UU & updatee;
   void (UU::*setter)(NV *);
   NV * newValue;
   NV * oldValue;
   void (MainWindow::*callback)(void);
};


/*!
 * \brief Helper function that allows RelationalUndoableUpdate to be instantiated with automatic template argument
 *        deduction.
 *
 *        (I thought this might not be necessary with the introduction of Class Template Argument Deduction in C++17,
 *        but I think I must be missing something.)
 */
template<class UU, class NV> RelationalUndoableUpdate<UU, NV> * newRelationalUndoableUpdate(UU & updatee,
                                                                                            void (UU::*setter)(NV *),
                                                                                            NV * newValue,
                                                                                            void (MainWindow::*callback)(void),
                                                                                            QString const & description,
                                                                                            QUndoCommand * parent = nullptr) {
   return new RelationalUndoableUpdate<UU, NV>(updatee, setter, newValue, callback, description, parent);
}



#endif /*RELATIONAL_UNDOABLE_UPDATE_H*/
