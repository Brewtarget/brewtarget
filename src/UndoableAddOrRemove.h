/*
 * UndoableAddOrRemove.h is part of Brewtarget, and is Copyright the following
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
#ifndef UNDOABLE_ADD_OR_REMOVE_H
#define UNDOABLE_ADD_OR_REMOVE_H

#include "brewtarget.h" // For logging
#include <QMetaType>
#include <QString>
#include <QUndoCommand>
#include <QVariant>
#include "model/Recipe.h"
#include "model/Style.h"
#include "StyleButton.h"

/*!
 * \class UndoableAddOrRemove
 * \author Matt Young
 *
 * \brief Each instance of this class is a non-trivial undoable addition to, or removal from, a recipe etc.
 */
template<class UU, class VV>
class UndoableAddOrRemove : public QUndoCommand
{
public:
   /*!
    * \param updatee The object we are updating - eg recipe
    * \param doer The method on the updatee to do the addition or removal.
    *             This should return the object that needs to be passed in to the \c undoer method.
    *             For certain classes - eg Fermentable, Hop - adding an object actually adds a new object which is
    *             the "child" of the one passed in.  Thus adding a Fermentable to a recipe returns the new copy
    *             ("child") Fermentable that was created; removing a Fermentable from a recipe returns its original
    *             "parent" Fermentable.
    * \param whatToAddOrRemove The thing we're adding or removing - eg fermentable
    * \param undoer The method on the updatee to undo the addition or removal
    * \param doCallback The method on MainWindow to call after doing/redoing the change - typically to update
    *                   other display elements.  If null, no callback is made.
    * \param undoCallback The method on MainWindow to call after undoing the change - typically to update
    *                     other display elements.  If null, no callback is made.
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Style"
    * \param parent This is for grouping updates together.
    */
   UndoableAddOrRemove(UU & updatee,
                       VV * (UU::*doer)(VV *),
                       VV * oneToAddOrRemove,
                       VV * (UU::*undoer)(VV *),
                       void (MainWindow::*doCallback)(VV *),
                       void (MainWindow::*undoCallback)(VV *),
                       QString const & description,
                       QUndoCommand * parent = nullptr)
   : QUndoCommand(parent),
     updatee(updatee),
     doer(doer),
     whatToAddOrRemove(oneToAddOrRemove),
     undoer(undoer),
     doCallback(doCallback),
     undoCallback(undoCallback),
     everDone(false)
   {
      this->setText(description);
      return;
   }
   /*!
    * This is the list version of the previous constructor. Some actions --
    * like dropping hops from the tree on a recipe -- work on lists  of
    * objects. We need this constructor because casting sucks. 
    * \param updatee See above
    * \param doer The method on the updatee to do the addition or removal.
    *             This should return a QList of objects that need to be passed in to the \c undoer method.
    * \param listToAddOrRemove The list of things we're adding or removing - eg fermentables
    * \param undoer See above
    * \param doCallback See above
    * \param undoCallback See above
    * \param description See above
    * \param parent See above
    */
   UndoableAddOrRemove(UU & updatee,
                       QList<VV *> (UU::*doer)(QList<VV *>),
                       QList<VV *> listToAddOrRemove,
                       QList<VV *> (UU::*undoer)(QList<VV *>),
                       void (MainWindow::*doCallback)(QList<VV *>),
                       void (MainWindow::*undoCallback)(QList<VV *>),
                       QString const & description,
                       QUndoCommand * parent = nullptr)
   : QUndoCommand(parent),
     updatee(updatee),
     list_doer(doer),
     list_whatToAddOrRemove(listToAddOrRemove),
     list_undoer(undoer),
     list_doCallback(doCallback),
     list_undoCallback(undoCallback),
     everDone(false)
   {
      // Parent class handles storing description and making it accessible to the undo stack etc - we just have to give
      // it the text.
      this->setText(description);
      return;
   }

   ~UndoableAddOrRemove()
   {
      return;
   }

   /*!
    * \brief Apply the update (including for the first time)
    */
   void redo()
   {
      QUndoCommand::redo();

      this->whatToAddOrRemove != nullptr ? this->undoOrRedo(false) : this->listUndoOrRedo(false);
      return;
   }

   /*!
    * \brief Undo applying the update
    */
   void undo()
   {
      QUndoCommand::undo();
      this->whatToAddOrRemove != nullptr ? this->undoOrRedo(true) : this->listUndoOrRedo(true);
      return;
   }

private:
   /*!
    * \brief Undo or redo applying the update
    * \param isUndo true for undo, false for redo
    */
   void undoOrRedo(bool const isUndo)
   {
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
        qDebug() << QString("%1: %2 \"%3\" for #%4")
                        .arg(Q_FUNC_INFO)
                        .arg(this->everDone ? "Redo" : "Do" )
                        .arg(this->text())
                        .arg(this->whatToAddOrRemove->key());

         this->whatToAddOrRemove = (this->updatee.*(this->doer))(whatToAddOrRemove);

         qDebug() << QString("%1: %2 Returned #%3")
                        .arg(Q_FUNC_INFO)
                        .arg(this->everDone ? "Redo" : "Do" )
                        .arg(this->whatToAddOrRemove->key());

         if (this->doCallback != nullptr) {
            (Brewtarget::mainWindow()->*(this->doCallback))(whatToAddOrRemove);
         }

         // In this implementation "Do" and "Redo" are identical, but it's nonetheless useful for debugging purposes to
         // be able to distinguish the two cases.
         this->everDone = true;
      } else {
         qDebug() << QString("%1: Undo \"%2\" for #%3")
            .arg(Q_FUNC_INFO)
            .arg(this->text())
            .arg(this->whatToAddOrRemove->key());

         this->whatToAddOrRemove = (this->updatee.*(this->undoer))(whatToAddOrRemove);

         qDebug() << QString("%1: Undo Returned #%2")
                        .arg(Q_FUNC_INFO)
                        .arg(this->whatToAddOrRemove->key());

         if (this->undoCallback != nullptr) {
            (Brewtarget::mainWindow()->*(this->undoCallback))(whatToAddOrRemove);
         }
      }

      return;
   }

   void listUndoOrRedo(bool const isUndo)
   {
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
      QList<VV*> results;

      if (!isUndo) {
         qDebug() << QString("%1: %2 \"%3\" for #%4")
                        .arg(Q_FUNC_INFO)
                        .arg(this->everDone ? "Redo" : "Do" )
                        .arg(this->text())
                        .arg(this->list_whatToAddOrRemove.size());

         // results.append((this->updatee.*(this->list_doer))(list_whatToAddOrRemove));
         this->list_whatToAddOrRemove = (this->updatee.*(this->list_doer))(list_whatToAddOrRemove);

         qDebug() << QString("%1: %2 Returned #%3")
                        .arg(Q_FUNC_INFO)
                        .arg(this->everDone ? "Redo" : "Do" )
                        .arg(this->list_whatToAddOrRemove.size());

         if (this->doCallback != nullptr) {
            (Brewtarget::mainWindow()->*(this->list_doCallback))(list_whatToAddOrRemove);
         }


         // In this implementation "Do" and "Redo" are identical, but it's nonetheless useful for debugging purposes to
         // be able to distinguish the two cases.
         this->everDone = true;
      } else {
         qDebug() << QString("%1: Undo \"%2\" for #%3")
            .arg(Q_FUNC_INFO)
            .arg(this->text())
            .arg(this->list_whatToAddOrRemove.size());

         // results.append((this->updatee.*(this->list_undoer))(list_whatToAddOrRemove));
         this->list_whatToAddOrRemove = (this->updatee.*(this->list_undoer))(list_whatToAddOrRemove);

         qDebug() << QString("%1: Undo Returned #%2")
                        .arg(Q_FUNC_INFO)
                        .arg(this->list_whatToAddOrRemove.size());

         if (this->undoCallback != nullptr) {
            (Brewtarget::mainWindow()->*(this->list_undoCallback))(list_whatToAddOrRemove);
         }
      }
      this->list_whatToAddOrRemove.append(results);

      return;
   }
   UU & updatee;
   // singletons
   VV * (UU::*doer)(VV *);
   VV * whatToAddOrRemove;
   VV * (UU::*undoer)(VV *);
   void (MainWindow::*doCallback)(VV *);
   void (MainWindow::*undoCallback)(VV *);

   // lists
   QList<VV *> (UU::*list_doer)(QList<VV *>);
   QList<VV *> list_whatToAddOrRemove;
   QList<VV *> (UU::*list_undoer)(QList<VV *>);
   void (MainWindow::*list_doCallback)(QList<VV *>);
   void (MainWindow::*list_undoCallback)(QList<VV *>);

   bool everDone;
};


/*!
 * \brief Helper function that allows UndoableAddOrRemove to be instantiated with automatic template argument deduction.
 *
 *        (I thought this might not be necessary with the introduction of Class Template Argument Deduction in C++17,
 *        but I think I must be missing something.)
 */
template<class UU, class VV> UndoableAddOrRemove<UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                                                  VV * (UU::*doer)(VV *),
                                                                                  VV * oneToAddOrRemove,
                                                                                  VV * (UU::*undoer)(VV *),
                                                                                  void (MainWindow::*doCallback)(VV *),
                                                                                  void (MainWindow::*undoCallback)(VV *),
                                                                                  QString const & description,
                                                                                  QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<UU, VV>(updatee,
                                          doer,
                                          oneToAddOrRemove,
                                          undoer,
                                          doCallback,
                                          undoCallback,
                                          description,
                                          parent);
}

template<class UU, class VV> UndoableAddOrRemove<UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                                                  QList<VV *> (UU::*doer)(QList<VV *>),
                                                                                  QList<VV *> whatToAddOrRemove,
                                                                                  QList<VV *> (UU::*undoer)(QList<VV *>),
                                                                                  void (MainWindow::*doCallback)(QList<VV *>),
                                                                                  void (MainWindow::*undoCallback)(QList<VV *>),
                                                                                  QString const & description,
                                                                                  QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<UU, VV>(updatee,
                                          doer,
                                          whatToAddOrRemove,
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
template<class UU, class VV> UndoableAddOrRemove<UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                                                  VV * (UU::*doer)(VV *),
                                                                                  VV * oneToAddOrRemove,
                                                                                  VV * (UU::*undoer)(VV *),
                                                                                  QString const & description,
                                                                                  QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<UU, VV>(updatee,
                                          doer,
                                          oneToAddOrRemove,
                                          undoer,
                                          static_cast<void (MainWindow::*)(VV *)>(nullptr),
                                          static_cast<void (MainWindow::*)(VV *)>(nullptr),
                                          description,
                                          parent);
}

template<class UU, class VV> UndoableAddOrRemove<UU, VV> * newUndoableAddOrRemove(UU & updatee,
                                                                                  QList<VV *> (UU::*doer)(QList<VV *>),
                                                                                  QList<VV *> whatToAddOrRemove,
                                                                                  QList<VV *> (UU::*undoer)(QList<VV *>),
                                                                                  QString const & description,
                                                                                  QUndoCommand * parent = nullptr) {
   return new UndoableAddOrRemove<UU, VV>(updatee,
                                          doer,
                                          whatToAddOrRemove,
                                          undoer,
                                          static_cast<void (MainWindow::*)(QList<VV *>)>(nullptr),
                                          static_cast<void (MainWindow::*)(QList<VV *>)>(nullptr),
                                          description,
                                          parent);
}

#endif /*UNDOABLE_ADD_OR_REMOVE_H*/
