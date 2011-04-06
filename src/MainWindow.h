/*
 * MainWindow.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2011.
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
#include <QVariant>
#include <QFileDialog>
#include <QPalette>
#include <QCloseEvent>
#include <QPrinter>
#include <QPrintDialog>
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
#include "HopEditor.h"
#include "YeastEditor.h"
#include "EquipmentEditor.h"
#include "StyleEditor.h"
#include "OptionDialog.h"
#include "MaltinessWidget.h"
#include "MashEditor.h"
#include "MashStepEditor.h"
#include "MashWizard.h"
#include "BrewDayScrollWidget.h"
#include "HtmlViewer.h"
#include "ScaleRecipeTool.h"
#include "RecipeFormatter.h"
#include "OgAdjuster.h"
#include "ConverterTool.h"
#include "TimerListDialog.h"
#include "MashComboBox.h"
#include "PrimingDialog.h"
#include "RecipeExtrasWidget.h"
#include "RefractoDialog.h"
#include "MashDesigner.h"
#include "PitchDialog.h"

class MainWindow : public QMainWindow, public Ui::mainWindow, public Observer
{
   Q_OBJECT

   friend class OptionDialog;
public:
   MainWindow(QWidget* parent=0);
   void setRecipe(Recipe* recipe);
   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   void forceRecipeUpdate(); // Should make the recipe call its hasChanged().

public slots:
   void save();
   void setRecipeByName(const QString& name);
   void setRecipeByIndex(const QModelIndex &index);
   void treeActivated(const QModelIndex &index);
   void clear();

   void updateRecipeName();
   void updateRecipeStyle();
   void updateRecipeEquipment(const QString&);
   void updateRecipeBatchSize();
   void updateRecipeBoilSize();
   void updateRecipeEfficiency();
   void updateRecipeStyle(const QString&);

   void addFermentableToRecipe(Fermentable* ferm);
   void removeSelectedFermentable();
   void editSelectedFermentable();
   void addHopToRecipe(Hop *hop);
   void removeSelectedHop();
   void editSelectedHop();
   void addMiscToRecipe(Misc* misc);
   void removeSelectedMisc();
   void editSelectedMisc();
   void addYeastToRecipe(Yeast* yeast);
   void removeSelectedYeast();
   void editSelectedYeast();

   void addMashStep();
   void removeSelectedMashStep();
   void editSelectedMashStep();
   void setMashByName(const QString& name);
   void saveMash();
	void removeMash();

   void newRecipe();
   void removeRecipe();
   void exportRecipe();
   void importRecipes();
   void copyRecipe();
   
   void printRecipe();
   void printBrewday();
   void printPreviewRecipe();
   void printPreviewBrewday();

   void backup(); // Backup the database.
   void restoreFromBackup(); // Restore the database.

   //void brewDayMode();

   void openDonateLink();

protected:
   virtual void closeEvent(QCloseEvent* event);

private:
   Recipe* recipeObs;
   AboutDialog* dialog_about;
   QFileDialog* fileOpener;
   QFileDialog* fileSaver;
   EquipmentEditor* equipEditor;
   FermentableDialog* fermDialog;
   FermentableEditor* fermEditor;
   HopDialog* hopDialog;
   HopEditor* hopEditor;
   MashEditor* mashEditor;
   MashStepEditor* mashStepEditor;
   MashWizard* mashWizard;
   MiscDialog* miscDialog;
   MiscEditor* miscEditor;
   StyleEditor* styleEditor;
   YeastDialog* yeastDialog;
   YeastEditor* yeastEditor;
   Database* db;
   OptionDialog* optionDialog;
   QPalette lcdPalette_old, lcdPalette_tooLow, lcdPalette_good, lcdPalette_tooHigh;
   MaltinessWidget* maltWidget;
   QDialog* brewDayDialog;
   HtmlViewer* htmlViewer;
   ScaleRecipeTool* recipeScaler;
   RecipeFormatter* recipeFormatter;
   OgAdjuster* ogAdjuster;
   ConverterTool* converterTool;
   TimerListDialog* timerListDialog;
   MashComboBox* mashComboBox;
   PrimingDialog* primingDialog;
   RefractoDialog* refractoDialog;
   MashDesigner* mashDesigner;
   PitchDialog* pitchDialog;
   QPrinter *printer;

   //! Currently highlighted fermentable in the fermentable table.
   Fermentable* selectedFermentable();
   //! Currently highlighted hop in the hop table.
   Hop* selectedHop();
   //! Currently highlighted misc in the misc table.
   Misc* selectedMisc();
   //! Currently highlighted yeast in the yeast table
   Yeast* selectedYeast();

   void setupToolbar();
   void showChanges(const QVariant& info = QVariant());
};

#endif	/* _MAINWINDOW_H */

