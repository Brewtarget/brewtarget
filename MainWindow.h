/*
 * MainWindow.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _MAINWINDOW_H
#define	_MAINWINDOW_H

class MainWindow;

#include <QWidget>
#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include "ui_mainWindow.h"
#include "FermentableDialog.h"
#include "HopDialog.h"
#include "MiscDialog.h"
#include "YeastDialog.h"
#include "AboutDialog.h"
#include "observable.h"
#include "recipe.h"
#include "BeerColorWidget.h"
#include "FermentableEditor.h"
#include "MiscEditor.h"

class MainWindow : public QMainWindow, public Ui::mainWindow, public Observer
{
   Q_OBJECT

public:
   MainWindow(QWidget* parent=0);
   void setRecipe(Recipe* recipe);
   virtual void notify(Observable* notifier); // Inherited from Observer

   static const char* homedir;

public slots:
   void save();
   void setRecipeByName(const QString& name);
   void clear();

   void updateRecipeName();
   void updateRecipeStyle();
   void updateRecipeEquipment();
   void updateRecipeBatchSize();
   void updateRecipeBoilSize();
   void updateRecipeEfficiency();

   void addFermentableToRecipe(Fermentable* ferm);
   void removeSelectedFermentable();
   void editSelectedFermentable();
   void addHopToRecipe(Hop *hop);
   void removeSelectedHop();
   void addMiscToRecipe(Misc* misc);
   void removeSelectedMisc();
   void editSelectedMisc();
   void addYeastToRecipe(Yeast* yeast);
   void removeSelectedYeast();

   void newRecipe();
   void exportRecipe();

private:
   Recipe* recipeObs;
   AboutDialog* dialog_about;
   QFileDialog* fileOpener;
   QFileDialog* fileSaver;
   FermentableDialog* fermDialog;
   FermentableEditor* fermEditor;
   HopDialog* hopDialog;
   MiscDialog* miscDialog;
   MiscEditor* miscEditor;
   YeastDialog* yeastDialog;
   Database* db;
   BeerColorWidget beerColorWidget;
   
   void showChanges();
};

#endif	/* _MAINWINDOW_H */

