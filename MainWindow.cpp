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

#include "FermentableEditor.h"
#include "MiscEditor.h"
#include "HopEditor.h"

#include "YeastTableModel.h"

#include "MiscTableModel.h"
#include <QtGui>

#include "style.h"
#include <QString>
#include <QFileDialog>

#include <iostream>
#include <fstream>

#include "recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "stringparsing.h"
#include "database.h"
#include "MiscTableWidget.h"
#include "YeastTableWidget.h"
#include "YeastDialog.h"
#include "BeerColorWidget.h"

const char* MainWindow::homedir =
#if defined(unix)
"/home";
#elif defined(windows)
"c:\\";
#elif defined(mac)
"/home";
#else
"";
#endif

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);

   // Null out the recipe
   recipeObs = 0;

   dialog_about = new AboutDialog(this);
   fermDialog = new FermentableDialog(this);
   fermEditor = new FermentableEditor(this);
   hopDialog = new HopDialog(this);
   hopEditor = new HopEditor(this);
   miscDialog = new MiscDialog(this);
   miscEditor = new MiscEditor(this);
   yeastDialog = new YeastDialog(this);

   // Set up the fileOpener dialog.
   fileOpener = new QFileDialog(this, tr("Open"), tr(homedir), tr(".xml"));
   fileOpener->setAcceptMode(QFileDialog::AcceptOpen);
   fileOpener->setFileMode(QFileDialog::ExistingFile);
   fileOpener->setViewMode(QFileDialog::List);

   // Set up the fileSaver dialog.
   fileSaver = new QFileDialog(this, tr("Save"), tr(homedir), tr(".xml") );
   fileSaver->setAcceptMode(QFileDialog::AcceptSave);
   fileSaver->setFileMode(QFileDialog::AnyFile);
   fileSaver->setViewMode(QFileDialog::List);

   // Set up and place the BeerColorWidget
   verticalLayout_lcd->insertWidget( 5, &beerColorWidget);

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
   yeastDialog->startObservingDB();
   
   if( db->getNumRecipes() > 0 )
      setRecipe(db->getRecipe(0));

   // Connect signals.
   connect( pushButton_exit, SIGNAL( clicked() ), this, SLOT( close() ));
   connect( pushButton_save, SIGNAL( clicked() ), this, SLOT( save() ));
   connect( pushButton_clear, SIGNAL( clicked() ), this, SLOT( clear() ));
   connect( recipeComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT(setRecipeByName(const QString&)) );
   connect( actionAbout_BrewTarget, SIGNAL( triggered() ), dialog_about, SLOT( show() ) );
   connect( actionNewRecipe, SIGNAL( triggered() ), this, SLOT( newRecipe() ) );
   connect( actionExportRecipe, SIGNAL( triggered() ), this, SLOT( exportRecipe() ) );
   connect( actionFermentables, SIGNAL( triggered() ), fermDialog, SLOT( show() ) );
   connect( actionHops, SIGNAL( triggered() ), hopDialog, SLOT( show() ) );
   connect( actionMiscs, SIGNAL( triggered() ), miscDialog, SLOT( show() ) );
   connect( actionYeasts, SIGNAL( triggered() ), yeastDialog, SLOT( show() ) );
   connect( lineEdit_name, SIGNAL( editingFinished() ), this, SLOT( updateRecipeName() ) );
   connect( lineEdit_batchSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBatchSize() ) );
   connect( lineEdit_boilSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBoilSize() ) );
   connect( lineEdit_efficiency, SIGNAL( editingFinished() ), this, SLOT( updateRecipeEfficiency() ) );
   connect( pushButton_addFerm, SIGNAL( clicked() ), fermDialog, SLOT( show() ) );
   connect( pushButton_addHop, SIGNAL( clicked() ), hopDialog, SLOT( show() ) );
   connect( pushButton_addMisc, SIGNAL( clicked() ), miscDialog, SLOT( show() ) );
   connect( pushButton_addYeast, SIGNAL( clicked() ), yeastDialog, SLOT( show() ) );
   connect( pushButton_removeFerm, SIGNAL( clicked() ), this, SLOT( removeSelectedFermentable() ) );
   connect( pushButton_removeHop, SIGNAL( clicked() ), this, SLOT( removeSelectedHop() ) );
   connect( pushButton_removeMisc, SIGNAL( clicked() ), this, SLOT( removeSelectedMisc() ) );
   connect( pushButton_removeYeast, SIGNAL( clicked() ), this, SLOT( removeSelectedYeast() ) );
   connect( pushButton_editFerm, SIGNAL(clicked()), this, SLOT( editSelectedFermentable() ) );
   connect( pushButton_editMisc, SIGNAL( clicked() ), this, SLOT( editSelectedMisc() ) );
   connect( pushButton_editHop, SIGNAL( clicked() ), this, SLOT( editSelectedHop() ) );
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
   lcdNumber_abv->display(doubleToStringPrec(recipeObs->getABV_pct(),1).c_str());
   lcdNumber_ibu->display(doubleToStringPrec(recipeObs->getIBU(),1).c_str());
   lcdNumber_srm->display(doubleToStringPrec(recipeObs->getColor_srm(),1).c_str());
   beerColorWidget.setColor( recipeObs->getSRMColor() );
}

void MainWindow::save()
{
   Database::savePersistent();
}

void MainWindow::clear()
{
   recipeObs->clear();
   setRecipe(recipeObs); // This will update tables and everything.
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

void MainWindow::addYeastToRecipe(Yeast* yeast)
{
   recipeObs->addYeast(yeast);
   yeastTable->getModel()->addYeast(yeast);
}

void MainWindow::exportRecipe()
{
   const char* filename;
   std::ofstream out;

   if( fileSaver->exec() )
      filename = fileSaver->selectedFiles()[0].toStdString().c_str();
   else
      return;

   out.open(filename, ios::trunc);

   out << "<?xml version=\"1.0\"?>" << std::endl;
   out << recipeObs->toXml();

   out.close();
}

void MainWindow::removeSelectedFermentable()
{
   QModelIndexList selected = fermentableTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Fermentable* ferm = fermentableTable->getModel()->getFermentable(row);
   fermentableTable->getModel()->removeFermentable(ferm);
   recipeObs->removeFermentable(ferm);
}

void MainWindow::editSelectedFermentable()
{
   QModelIndexList selected = fermentableTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Fermentable* ferm = fermentableTable->getModel()->getFermentable(row);
   fermEditor->setFermentable(ferm);
   fermEditor->show();
}

void MainWindow::editSelectedMisc()
{
   QModelIndexList selected = miscTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Misc* m = miscTable->getModel()->getMisc(row);
   miscEditor->setMisc(m);
   miscEditor->show();
}

void MainWindow::editSelectedHop()
{
   QModelIndexList selected = hopTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Hop* h = hopTable->getModel()->getHop(row);
   hopEditor->setHop(h);
   hopEditor->show();
}

void MainWindow::removeSelectedHop()
{
   QModelIndexList selected = hopTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Hop* hop = hopTable->getModel()->getHop(row);
   hopTable->getModel()->removeHop(hop);
   recipeObs->removeHop(hop);
}

void MainWindow::removeSelectedMisc()
{
   QModelIndexList selected = miscTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Misc* misc = miscTable->getModel()->getMisc(row);
   miscTable->getModel()->removeMisc(misc);
   recipeObs->removeMisc(misc);
}

void MainWindow::removeSelectedYeast()
{
   QModelIndexList selected = yeastTable->selectedIndexes();
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return;

   // Make sure only one row is selected.
   row = selected[0].row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return;
   }

   Yeast* yeast = yeastTable->getModel()->getYeast(row);
   yeastTable->getModel()->removeYeast(yeast);
   recipeObs->removeYeast(yeast);
}

void MainWindow::newRecipe()
{
   Recipe* recipe = new Recipe();
   std::string name = "New Recipe";

   // Set the following stuff so everything appears nice
   // and the calculations don't divide by zero... things like that.
   recipe->setName(name);
   recipe->setBatchSize_l(18.93); // 5 gallons
   recipe->setBoilSize_l(23.47);  // 6.2 gallons
   recipe->setEfficiency_pct(70.0);

   db->addRecipe(recipe);
   setRecipe(recipe);

   recipeComboBox->setIndexByRecipeName(name);
}
