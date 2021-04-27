/*
 * SimpleUndoableUpdate.h is part of Brewtarget, and is Copyright the following
 * authors 2020-2021
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
#ifndef SIMPLE_UNDOABLE_UPDATE_H
#define SIMPLE_UNDOABLE_UPDATE_H

#include <QMetaProperty>
#include <QMetaType>
#include <QString>
#include <QUndoCommand>
#include <QVariant>

/*!
 * \class SimpleUndoableUpdate
 * \author Matt Young
 *
 * \brief Each instance of this class is an undoable update to a 'simple' editable field of a recipe, style, etc.
 *        By simple, we mean that there is one of them and that it is non-relational (ie can be passed and set by value).
 *        The thing being updated needs to inherit from Q_OBJECT and the field being changed needs to have been
 *        declared as a Q_PROPERTY.
 */
class SimpleUndoableUpdate : public QUndoCommand
{
public:
   /*!
    * \param updatee The entity (eg recipe) we are updating
    * \param propertyName Which property we are updating - needs to have been declared as a Q_PROPERTY in the class header file
    * \param newValue The new value to assign
    * \param description Short text we can show on undo/redo menu to describe this update eg "Change Recipe Name"
    * \param parent This is for grouping updates together.  We don't currently use it.
    */
   SimpleUndoableUpdate(QObject & updatee,
                        char const * const propertyName,
                        QVariant newValue,
                        QString const & description,
                        QUndoCommand * parent = nullptr);

   ~SimpleUndoableUpdate();

   /*!
    * \brief Apply the update (including for the first time)
    */
   void redo();

   /*!
    * \brief Undo applying the update
    */
   void undo();

private:
   /*!
    * \brief Undo or redo applying the update
    * \param isUndo true for undo, false for redo
    * \return true if succeeded, false otherwise - not currently used but potentially useful for a derived class
    */
   bool undoOrRedo(bool const isUndo);

   QObject & updatee;
   char const * const propertyName;
   QVariant oldValue, newValue;
};

#endif /*SIMPLE_UNDOABLE_UPDATE_H*/
