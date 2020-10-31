/*
 * RecipeUndoableUpdate.h is part of Brewtarget, and is Copyright the following
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
#ifndef RECIPE_UNDOABLE_UPDATE_H
#define RECIPE_UNDOABLE_UPDATE_H

#include "brewtarget.h" // For logging
#include "recipe.h"
#include <QMetaType>
#include <QString>
#include <QUndoCommand>
#include <QVariant>

/*!
 * \class RecipeUndoableUpdate
 * \author Matt Young
 *
 * \brief Each instance of this class is a single undoable update to a recipe
 *
 * .:TODO:. See if we could template this class to handle updates to other objects...
 */
class RecipeUndoableUpdate : public QUndoCommand
{
public:
   /*!
    * \param recipe The recipe we are updating
    * \param propertyName Which property we are updating - needs to have been declared as a Q_PROPERTY in the class header file
    * \param newValue The new value to assign
    * \param parent This is for grouping updates together.  We don't currently use it.
    */
   RecipeUndoableUpdate(Recipe & recipe,
                        char const * const propertyName,
                        QVariant newValue,
                        QUndoCommand * parent = nullptr)

   : QUndoCommand(parent), recipe(recipe), propertyName(propertyName), newValue(newValue)
   {
      int propertyIndex = recipe.metaObject()->indexOfProperty(propertyName);
      if ( propertyIndex < 0 )
      {
         // Getting here means a coding error at the caller.
         throw QString("Trying to update undeclared property \"%1\" of recipe").arg(propertyName);
      }

      this->metaProperty = recipe.metaObject()->property(propertyIndex);

      // For a simple property, we just store the old value.  For a list of objects, where the update is adding or removing something
      // from the list, it's a bit more complicated.  .:TODO:. IMPLEMENT THIS!
      QVariant::Type propertyType = this->metaProperty.type();
      Brewtarget::logD(QString("Recipe property %1 is %2 (%3)").arg(propertyName).arg(this->metaProperty.typeName()).arg(propertyType));
      this->oldValue = this->metaProperty.read(&recipe);
   }

   ~RecipeUndoableUpdate()
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
   void undoOrRedo(bool const isUndo)
   {
      bool success = this->metaProperty.write(&recipe, isUndo ? this->oldValue : this->newValue);
      if (!success)
      {
         Brewtarget::logE(QString("Could not %1 update of recipe property %2").arg(isUndo ? "undo" : "redo").arg(propertyName));
      }
      return;
   }

   Recipe & recipe;
   char const * const propertyName;
   QVariant oldValue, newValue;
   QMetaProperty metaProperty;
};

#endif /*RECIPE_UNDOABLE_UPDATE_H*/
