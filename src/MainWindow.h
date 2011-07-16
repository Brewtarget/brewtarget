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
#define   _MAINWINDOW_H

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
#include "BrewNoteWidget.h"

class MainWindow : public QMainWindow, public Ui::mainWindow, public Observer
{
   Q_OBJECT

   friend class OptionDialog;
public:
   MainWindow(QWidget* parent=0);
   void setRecipe(Recipe* recipe);
   virtual void notify(Observable* notifier, QVariant info = QVariant()); // Inherited from Observer
   void forceRecipeUpdate(); // Should make the recipe call its hasChanged().
   QFile* openForWrite(QString filterStr = "BeerXML files (*.xml)", QString defaultSuff = "xml");

   bool verifyImport(QString tag, QString name);
   bool verifyDelete(QString tab, QString name);

   void setBrewNoteByIndex(const QModelIndex &index);
   void setBrewNote(BrewNote* bNote);

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
   void exportRecipe();
   void importFiles();
   void copyRecipe();
   
   void deleteSelected();
   void copySelected();
   void exportSelected();

   void print();

   void backup(); // Backup the database.
   void restoreFromBackup(); // Restore the database.

   void contextMenu(const QPoint &point);
   void newBrewNote();
   void reBrewNote();

   void openDonateLink();

   void dragEnterEvent(QDragEnterEvent *event);
   void dropEvent( QDropEvent *event);

protected:
   virtual void closeEvent(QCloseEvent* event);

private:
   Recipe* recipeObs;
   AboutDialog* dialog_about;
   QFileDialog* fileOpener;
   QFileDialog* fileSaver;
   QList<QMenu*> contextMenus;
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

   QMultiHash<QString, BrewNoteWidget*> brewNotes;
   int confirmDelete;

   //! Currently highlighted fermentable in the fermentable table.
   Fermentable* selectedFermentable();
   //! Currently highlighted hop in the hop table.
   Hop* selectedHop();
   //! Currently highlighted misc in the misc table.
   Misc* selectedMisc();
   //! Currently highlighted yeast in the yeast table
   Yeast* selectedYeast();

   void setSelection(QModelIndex item);

   //! set the equipment based on a drop event
   void droppedRecipeEquipment(Equipment *kit);
   void setupToolbar();
   void setupContextMenu();

   void showChanges(const QVariant& info = QVariant());

   // Copy methods used by copySelected()
   void copyThis(Recipe *rec);
   void copyThis(Equipment *kit);
   void copyThis(Fermentable *ferm);
   void copyThis(Hop *hop);
   void copyThis(Misc *misc);
   void copyThis(Yeast *yeast);

};

#endif   /* _MAINWINDOW_H */

