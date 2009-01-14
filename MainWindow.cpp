/*
 * MainWindow.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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

#include <QWidget>
#include <QMainWindow>
#include <QtGui>

#include "recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "stringparsing.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);
   
   // Null out the recipe
   recipeObs = 0;

   dialog_about = new AboutDialog(this);

   // Connect some signals.
   connect( pushButton_exit, SIGNAL( clicked() ), this, SLOT( close() ));
   connect( actionAbout_BrewTarget, SIGNAL( triggered() ), dialog_about, SLOT( show() ) );
}

void MainWindow::setRecipe(Recipe* recipe)
{
   // Don't like void pointers.
   if( recipe == 0 )
      return;

   recipeObs = recipe;
   setObserved(recipeObs);
}

void MainWindow::notify(Observable* notifier)
{
   // Make sure the notifier is our observed recipe
   if( notifier != recipeObs )
      return;

   showChanges();
}

void MainWindow::showChanges()
{
   if( recipeObs == 0 )
      return;

   recipeObs->recalculate();
   // TODO: fill in this method to change the widgets to reflect
   // the current recipe.
   lineEdit_name->setText(recipeObs->getName().c_str());
   lineEdit_batchSize->setText(doubleToString(recipeObs->getBatchSize_l()).c_str());
   lineEdit_boilSize->setText(doubleToString(recipeObs->getBoilSize_l()).c_str());
   lineEdit_efficiency->setText(doubleToString(recipeObs->getEfficiency_pct()).c_str());

   lcdNumber_og->display(recipeObs->getOg());
   lcdNumber_fg->display(recipeObs->getFg());
   lcdNumber_abv->display(recipeObs->getABV_pct());
   lcdNumber_srm->display(recipeObs->getColor_srm());
}

void MainWindow::save()
{
   // TODO: make this method write the recipe (and maybe other stuff)
   // out to the xml files.
}

void MainWindow::clear()
{
   // TODO: this method should clear the recipe I guess.
}
