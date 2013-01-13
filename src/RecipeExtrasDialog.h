/*
* RecipeExtrasDialog.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2010-2013.
*
* Brewtarget is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* Brewtarget is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RECIPEEXTRASDIALOG_H
#define RECIPEEXTRASDIALOG_H

class RecipeExtrasDialog;

#include <QDialog>
#include <QWidget>
#include <QMetaProperty>
#include <QVariant>
#include "ui_recipeExtrasDialog.h"

// Forward declarations.
class Recipe;

/*!
 * \class RecipeExtrasDialog
 * \author Philip G. Lee
 *
 * \brief View/controller dialog for editing "extra" fields of the recipe.
 */
class RecipeExtrasDialog : public QDialog, public Ui::recipeExtrasDialog
{
   Q_OBJECT

public:
   RecipeExtrasDialog(QWidget* parent=0);
   virtual ~RecipeExtrasDialog() {}
   //! Set the recipe to view/edit.
   void setRecipe(Recipe* rec);

public slots:
   void updateBrewer();
   void updateBrewerAsst();
   void updateTasteRating();
   void updatePrimaryAge();
   void updatePrimaryTemp();
   void updateSecondaryAge();
   void updateSecondaryTemp();
   void updateTertiaryAge();
   void updateTertiaryTemp();
   void updateAge();
   void updateAgeTemp();
   void updateDate();
   void updateCarbonation();
   void updateTasteNotes();
   void updateNotes();

   void saveAndQuit();
   void changed(QMetaProperty, QVariant);

private:
   Recipe* recObs;

   void showChanges(QMetaProperty* prop = 0);
};

#endif // RECIPEEXTRASDIALOG_H
