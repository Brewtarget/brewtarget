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
#include <QString>
#include <QFileDialog>

#include <iostream>

#include "recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "stringparsing.h"
#include "database.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);
   
   // Null out the recipe
   recipeObs = 0;

   dialog_about = new AboutDialog(this);

   // Set up the fileOpener dialog.
   fileOpener = new QFileDialog(this, tr("Open"),
                                #if defined(unix)
                                tr("~/"),
                                #elif defined(windows)
                                tr("c:\\"),
                                #elif defined(mac)
                                tr("~/"),
                                #else
                                tr(""),
                                #endif
                                tr("xml")
                                );
   fileOpener->setAcceptMode(QFileDialog::AcceptOpen);
   fileOpener->setFileMode(QFileDialog::ExistingFile);
   fileOpener->setViewMode(QFileDialog::List);

   // Setup some of the widgets.
   if( Database::isInitialized() )
   {
      unsigned int size, i;

      // Set up the recipeComboBox widget.
      size = Database::recipes.size();
      for( i = 0; i < size; ++i )
         recipeComboBox->addRecipe(Database::recipes[i]);
      recipeComboBox->repopulateList();
      if( size > 0 )
         setRecipe(Database::recipes[0]);
      
      size = Database::styles.size();
      for( i = 0; i < size; ++i )
         comboBox_style->addItem(tr(Database::styles[i]->getName().c_str()));
      size = Database::equipments.size();
      for( i = 0; i < size; ++i )
         comboBox_equipment->addItem(tr(Database::equipments[i]->getName().c_str()));
   }
   else
      std::cerr << "MainWindow warning: the database was not initialized." << std::endl;

   // Connect signals.
   connect( pushButton_exit, SIGNAL( clicked() ), this, SLOT( close() ));
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ));
   connect( recipeComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT(setRecipeByName(const QString&)) );
   connect( actionAbout_BrewTarget, SIGNAL( triggered() ), dialog_about, SLOT( show() ) );
   connect( lineEdit_name, SIGNAL( editingFinished() ), this, SLOT( updateRecipeName() ) );
   connect( lineEdit_batchSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBatchSize() ) );
   connect( lineEdit_boilSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBoilSize() ) );
   connect( lineEdit_efficiency, SIGNAL( editingFinished() ), this, SLOT( updateRecipeEfficiency() ) );
}

void MainWindow::setRecipeByName(const QString& name)
{
   if(  ! Database::isInitialized() )
      return;

   unsigned int i, size;

   size = Database::recipes.size();
   for( i = 0; i < size; ++i )
      if( Database::recipes[i]->getName() == name.toStdString() )
         setRecipe(Database::recipes[i]);
}

void MainWindow::setRecipe(Recipe* recipe)
{
   // Don't like void pointers.
   if( recipe == 0 )
      return;

   unsigned int i;
   Fermentable *ferm;
   Hop *hop;
   Misc *misc;
   Yeast *yeast;

   // First, remove any previous recipe shit.
   fermentableTable->getModel()->removeAll();
   hopTable->getModel()->removeAll();
   miscTable->getModel()->removeAll();
   yeastTable->getModel()->removeAll();

   // Make sure this MainWindow is paying attention...
   recipeObs = recipe;
   setObserved(recipeObs); // Automatically removes the previous observer.

   // Make sure the fermentableTable is paying attention...
   for( i = 0; i < recipeObs->getNumFermentables(); ++i )
   {
      ferm = recipeObs->getFermentable(i);
      fermentableTable->getModel()->addFermentable(ferm);
   }

   for( i = 0; i < recipeObs->getNumHops(); ++i )
   {
      hop = recipeObs->getHop(i);
      hopTable->getModel()->addHop(hop);
   }

   for( i = 0; i < recipeObs->getNumMiscs(); ++i )
   {
      misc = recipeObs->getMisc(i);
      miscTable->getModel()->addMisc(misc);
   }

   for( i = 0; i < recipeObs->getNumYeasts(); ++i )
   {
      yeast = recipeObs->getYeast(i);
      yeastTable->getModel()->addYeast(yeast);
   }

   showChanges();
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
   Database::savePersistent();
}

void MainWindow::clear()
{
   // TODO: this method should clear the recipe I guess.
}

void MainWindow::updateRecipeName()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setName(lineEdit_name->text().toStdString());
}

void MainWindow::updateRecipeStyle()
{
   if( recipeObs == 0 )
      return;
}

void MainWindow::updateRecipeEquipment()
{
   if( recipeObs == 0 )
      return;
}

void MainWindow::updateRecipeBatchSize()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setBatchSize_l( parseDouble(lineEdit_batchSize->text().toStdString()) );
}

void MainWindow::updateRecipeBoilSize()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setBoilSize_l( parseDouble(lineEdit_boilSize->text().toStdString()) );
}

void MainWindow::updateRecipeEfficiency()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setEfficiency_pct( parseDouble(lineEdit_efficiency->text().toStdString()) );
}
