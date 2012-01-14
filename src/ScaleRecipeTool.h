/*
* ScaleRecipeTool.h is part of Brewtarget, and is Copyright Philip G. Lee
* (rocketman768@gmail.com), 2009-2012.
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

#ifndef SCALE_RECIPE_TOOL_H
#define SCALE_RECIPE_TOOL_H

class ScaleRecipeTool;

#include "ui_scaleRecipeTool.h"
#include <QDialog>
#include <QWidget>
#include <QAbstractButton>
#include <QButtonGroup>

// Forward declarations
class Recipe;

/*!
 * \class ScaleRecipeTool
 * \author Philip G. Lee
 *
 * \brief Controller class that scales a recipe's ingredients.
 */
class ScaleRecipeTool : public QDialog, public Ui::scaleRecipeTool
{
   Q_OBJECT
   
public:
   ScaleRecipeTool(QWidget* parent=0);
   void setRecipe(Recipe* rec);
   
public slots:
   void show();
   
private slots:
   void scale();
   void scaleByVolume();
   void scaleByEfficiency();
   void scaleGroupButtonPressed(QAbstractButton* button);

private:
   Recipe* recObs;
   QButtonGroup scaleGroup;
};

#endif /*SCALE_RECIPE_TOOL_H*/
