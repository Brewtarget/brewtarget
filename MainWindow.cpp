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

#include "MiscTableModel.h"
#include <QtGui>

#include "style.h"
#include <QString>
#include <QFileDialog>

#include <iostream>

#include "recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "stringparsing.h"
#include "database.h"
#include "MiscTableWidget.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);
   
   // Null out the recipe
   recipeObs = 0;

   dialog_about = new AboutDialog(this);
   fermDialog = new FermentableDialog(this);
   hopDialog = new HopDialog(this);
   miscDialog = new MiscDialog(this);

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

   if( Database::isInitialized() )
      db = Database::getDatabase();
   else
   {
      Database::initialize();
      db = Database::getDatabase();
   }

   // Setup some of the widgets.
   recipeComboBox->startObservingDB();
   fermDialog->startObservingDB();
   hopDialog->startObservingDB();
   miscDialog->startObservingDB();
   
   if( db->getNumRecipes() > 0 )
      setRecipe(db->getRecipe(0));

   // Connect signals.
   connect( pushButton_exit, SIGNAL( clicked() ), this, SLOT( close() ));
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ));
   connect( recipeComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT(setRecipeByName(const QString&)) );
   connect( actionAbout_BrewTarget, SIGNAL( triggered() ), dialog_about, SLOT( show() ) );
   connect( lineEdit_name, SIGNAL( editingFinished() ), this, SLOT( updateRecipeName() ) );
   connect( lineEdit_batchSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBatchSize() ) );
   connect( lineEdit_boilSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBoilSize() ) );
   connect( lineEdit_efficiency, SIGNAL( editingFinished() ), this, SLOT( updateRecipeEfficiency() ) );
   connect( pushButton_addFerm, SIGNAL( clicked() ), fermDialog, SLOT( show() ) );
   connect( pushButton_addHop, SIGNAL( clicked() ), hopDialog, SLOT( show() ) );
   connect( pushButton_addMisc, SIGNAL( clicked() ), miscDialog, SLOT( show() ) );
}

void MainWindow::setRecipeByName(const QString& name)
{
   if(  ! Database::isInitialized() )
      return;

   unsigned int i, size;

   size = db->getNumRecipes();
   for( i = 0; i < size; ++i )
      if( db->getRecipe(i)->getName() == name.toStdString() )
         setRecipe(db->getRecipe(i));
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

// This method should update all the widgets in the window (except the tables)
// to reflect the currently observed recipe.
void MainWindow::showChanges()
{
   if( recipeObs == 0 )
      return;

   recipeObs->recalculate();

   lineEdit_name->setText(recipeObs->getName().c_str());
   lineEdit_batchSize->setText(doubleToString(recipeObs->getBatchSize_l()).c_str());
   lineEdit_boilSize->setText(doubleToString(recipeObs->getBoilSize_l()).c_str());
   lineEdit_efficiency->setText(doubleToString(recipeObs->getEfficiency_pct()).c_str());

   pushButton_style->setText(tr(recipeObs->getStyle()->getName().c_str()));
   pushButton_style->adjustSize();
   
   // Recipe's equipment is optional, so might be null.
   Equipment* equip = recipeObs->getEquipment();
   if( equip )
      pushButton_equipment->setText(tr(equip->getName().c_str()));
   else
      pushButton_equipment->setText(tr("No equipment"));
   pushButton_equipment->adjustSize();

   lcdNumber_og->display(doubleToStringPrec(recipeObs->getOg(), 3).c_str());
   lcdNumber_fg->display(doubleToStringPrec(recipeObs->getFg(), 3).c_str());
   lcdNumber_abv->display(recipeObs->getABV_pct());
   lcdNumber_ibu->display(recipeObs->getIBU());
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

void MainWindow::addFermentableToRecipe(Fermentable* ferm)
{
   recipeObs->addFermentable(ferm);
   fermentableTable->getModel()->addFermentable(ferm);
}

void MainWindow::addHopToRecipe(Hop *hop)
{
   recipeObs->addHop(hop);
   hopTable->getModel()->addHop(hop);
}

void MainWindow::addMiscToRecipe(Misc* misc)
{
   recipeObs->addMisc(misc);
   miscTable->getModel()->addMisc(misc);
}
