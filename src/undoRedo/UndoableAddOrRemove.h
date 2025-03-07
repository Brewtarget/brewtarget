/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * undoRedo/UndoableAddOrRemove.h is part of Brewtarget, and is copyright the following authors 2020-2024:
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
#ifndef UNDOREDO_UNDOABLEADDORREMOVE_H
#define UNDOREDO_UNDOABLEADDORREMOVE_H
#pragma once

#include <memory>
#include <type_traits>

#include <QDebug>
#include <QMetaType>
#include <QString>
#include <QUndoCommand>

#include "database/ObjectStoreWrapper.h"
#include "Logging.h"

class MainWindow;

/*!
 * \class UndoableAddOrRemove
 *
 * \brief Each instance of this class is a non-trivial undoable addition to, or removal from, a recipe etc.
 *
 *        Note that in the majority of cases, \c BB and \c UU are the same class.  The distinction exists to allow for
 *        the fact that a member function on \c UU is actually inherited from \c BB.  Eg, when the caller supplies
 *        \c &Mash::addStep as a "doer" parameter we actually receive &StepOwnerBase<Mash, MashStep>::addStep
 *        (because Mash inherits from StepOwnerBase<Mash, MashStep>).  Fortunately, callers don't have to worry about
 *        this as the compiler works everything out for us.
 *
 *        TBD: As of C++20 I think we can replace the slightly cumbersome \c std::enable_if_t syntax with concepts, but
 *             I haven't yet got my head around the exact syntax to do so!
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
class UndoableAddOrRemove : public QUndoCommand {
public:
   // NB: Constructors are all explicit as we don't want to construct with implicit conversions
   /*!
    * \param updatee The object we are updating - eg recipe
    * \param doer The method on the updatee to do the addition or removal.
    *             This should return the object that needs to be passed in to the \c undoer method.
    *             For certain classes - eg Fermentable, Hop - adding an object actually adds a new object which is
    *             the "child" of the one passed in.  Thus adding a Fermentable to a recipe returns the new copy
    *             ("child") Fermentable that was created; removing a Fermentable from a recipe returns its original
    *             "parent" Fermentable.
    * \param whatToAddOrRemove The (subclass of) NamedEntity object we're adding or removing - eg Fermentable.  This
    *                          must be a copy of the shared pointer from the relevant ObjectStore because, if the
    *                          object gets deleted (eg removing a MashStep from a Mash) then we will be the only ones
    *                          holding a copy of its shared pointer unless and until that action is undone.
    * \param undoer The method on the updatee to undo the addition or removal
    * \param doCallback The method on MainWindow to call after doing/redoing the change - typically to update
    *                   other display elements.  If null, no callback is made.
    * \param undoCallback The method on MainWindow to call after undoing the change - typically to update
    *                     other display elements.  If null, no callback is made.
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Style"
    * \param parent This is for grouping updates together.
    */
   explicit UndoableAddOrRemove(UU & updatee,
                                std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                std::shared_ptr<VV> whatToAddOrRemove,
                                std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                void (MainWindow::*doCallback)(std::shared_ptr<VV>),
                                void (MainWindow::*undoCallback)(std::shared_ptr<VV>),
                                QString const & description,
                                QUndoCommand * parent = nullptr) :
      QUndoCommand(parent),
      updatee(updatee),
      doer(doer),
      whatToAddOrRemove(whatToAddOrRemove),
      undoer(undoer),
      doCallback(doCallback),
      undoCallback(undoCallback),
      everDone(false) {
      // Uncomment this block if you need to diagnose problems that result in hitting the asserts below
//      if (!whatToAddOrRemove || whatToAddOrRemove->key() <= 0) {
//         qCritical().noquote() << Q_FUNC_INFO << Logging::getStackTrace();
//      }
      // It's a coding error to add or remove either a null pointer...
      Q_ASSERT(whatToAddOrRemove);
///      // ...or something that is not yet stored in its corresponding ObjectStore (ie does not yet have a DB ID)
///      Q_ASSERT(whatToAddOrRemove->key() > 0);

      // Parent class handles storing description and making it accessible to the undo stack etc - we just have to give
      // it the text.
      this->setText(description);

      return;
   }

   //! Copy constructor OK
   explicit UndoableAddOrRemove(UndoableAddOrRemove const &) = default;
   //! Move constructor OK
   explicit UndoableAddOrRemove(UndoableAddOrRemove &&) = default;

   ~UndoableAddOrRemove() = default;

   /*!
    * \brief Apply the update (including for the first time)
    */
   void redo() {
      QUndoCommand::redo();
      this->undoOrRedo(false);
      return;
   }

   /*!
    * \brief Undo applying the update
    */
   void undo() {
      QUndoCommand::undo();
      this->undoOrRedo(true);
      return;
   }

private:
   /*!
    * \brief Undo or redo applying the update
    * \param isUndo true for undo, false for redo
    */
   void undoOrRedo(bool const isUndo) {
      //
      // This function works on the assumption that Add and Remove both return "what was changed".
      //
      // If the action is Add, and the thing we are adding is of a type that gets copied, then the doer is going to
      // return the copy that was created and added, which is what we'll want to remove if we undo.  If we then redo
      // with this object, it will get stored in the DB with a new ID (and the object's key will be updated
      // accordingly).  The same object (with now modified key) is returned and it's what we'll need if we want to undo
      // again.
      //
      // If the action is Remove, then we'll get back a pointer to what we removed.  When we undo, passing this to added
      // will cause it to be stored in the DB with a new ID.
      //
      if (!isUndo) {
         qDebug() <<
            Q_FUNC_INFO << (this->everDone ? "Redo" : "Do" ) << this->text() << "for " <<
            this->whatToAddOrRemove->metaObject()->className() << "#" << this->whatToAddOrRemove->key() << "(" <<
            this->whatToAddOrRemove->name() << ")";
         // Normally leave the next line commented out as it generates a lot of logging
//         qDebug().noquote() << Q_FUNC_INFO << Logging::getStackTrace();

         this->whatToAddOrRemove = (this->updatee.*(this->doer))(this->whatToAddOrRemove);
         qDebug() <<
            Q_FUNC_INFO << (this->everDone ? "Redo" : "Do" ) << "Returned " <<
            this->whatToAddOrRemove->metaObject()->className() << "#" << this->whatToAddOrRemove->key() << "(" <<
            this->whatToAddOrRemove->name() << ")";

         if (this->doCallback != nullptr) {
            (MainWindow::instance().*(this->doCallback))(this->whatToAddOrRemove);
         }

         // In this implementation "Do" and "Redo" are identical, but it's nonetheless useful for debugging purposes to
         // be able to distinguish the two cases.
         this->everDone = true;
      } else {
         qDebug() <<
            Q_FUNC_INFO << "Undo" << this->text() << "for " << this->whatToAddOrRemove->metaObject()->className() <<
            "#" << this->whatToAddOrRemove->key() << "(" << this->whatToAddOrRemove->name() << ")";

         this->whatToAddOrRemove = (this->updatee.*(this->undoer))(this->whatToAddOrRemove);
         qDebug() <<
            Q_FUNC_INFO << "Undo Returned " << this->whatToAddOrRemove->metaObject()->className() << "#" <<
            this->whatToAddOrRemove->key() << "(" << this->whatToAddOrRemove->name() << ")";

         if (this->undoCallback != nullptr) {
            (MainWindow::instance().*(this->undoCallback))(this->whatToAddOrRemove);
         }
      }

      return;
   }

   //! No assignment operator
   UndoableAddOrRemove & operator=(UndoableAddOrRemove const &) = delete;
   //! No move assignment
   UndoableAddOrRemove & operator=(UndoableAddOrRemove &&) = delete;

   UU & updatee;
   std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>);
   std::shared_ptr<VV> whatToAddOrRemove;
   std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>);
   void (MainWindow::*doCallback)(std::shared_ptr<VV>);
   void (MainWindow::*undoCallback)(std::shared_ptr<VV>);
   bool everDone;
};

/*!
 * \brief Helper function that allows UndoableAddOrRemove to be instantiated with automatic template argument deduction.
 *
 *        (I thought this might not be necessary with the introduction of Class Template Argument Deduction in C++17,
 *        but I think I must be missing something.)
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemove<BB, UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                         std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                         std::shared_ptr<VV> whatToAddOrRemove,
                                                         std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                         void (MainWindow::*doCallback)(std::shared_ptr<VV>),
                                                         void (MainWindow::*undoCallback)(std::shared_ptr<VV>),
                                                         QString const & description,
                                                         QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                              doer,
                                              whatToAddOrRemove,
                                              undoer,
                                              doCallback,
                                              undoCallback,
                                              description,
                                              parent);
}

/*!
 * \brief Raw pointer version of \c newUndoableAddOrRemove above
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemove<BB, UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                         std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                         VV * const whatToAddOrRemove,
                                                         std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                         void (MainWindow::*doCallback)(std::shared_ptr<VV>),
                                                         void (MainWindow::*undoCallback)(std::shared_ptr<VV>),
                                                         QString const & description,
                                                         QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                              doer,
                                              ObjectStoreWrapper::getSharedFromRaw(whatToAddOrRemove),
                                              undoer,
                                              doCallback,
                                              undoCallback,
                                              description,
                                              parent);
}

/*!
 * \brief Helper function that allows UndoableAddOrRemove to be instantiated with automatic template argument deduction.
 *
 *        This is useful when there are no callbacks, otherwise caller has to do a static cast on null pointer
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemove<BB, UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                         std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                         std::shared_ptr<VV> whatToAddOrRemove,
                                                         std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                         QString const & description,
                                                         QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                              doer,
                                              whatToAddOrRemove,
                                              undoer,
                                              static_cast<void (MainWindow::*)(std::shared_ptr<VV>)>(nullptr),
                                              static_cast<void (MainWindow::*)(std::shared_ptr<VV>)>(nullptr),
                                              description,
                                              parent);
}

/*!
 * \brief Raw pointer version of \c newUndoableAddOrRemove above
 */
template<class BB, class UU, class VV, std::enable_if_t<std::is_base_of_v<BB, UU>, bool> = true>
UndoableAddOrRemove<BB, UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                         std::shared_ptr<VV> (BB::*doer)(std::shared_ptr<VV>),
                                                         VV * const whatToAddOrRemove,
                                                         std::shared_ptr<VV> (BB::*undoer)(std::shared_ptr<VV>),
                                                         QString const & description,
                                                         QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<BB, UU, VV>(updatee,
                                              doer,
                                              ObjectStoreWrapper::getSharedFromRaw(whatToAddOrRemove),
                                              undoer,
                                              static_cast<void (MainWindow::*)(std::shared_ptr<VV>)>(nullptr),
                                              static_cast<void (MainWindow::*)(std::shared_ptr<VV>)>(nullptr),
                                              description,
                                              parent);
}

#endif
