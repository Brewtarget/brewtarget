/*
 * MainWindow.h is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009-2013.
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
#define _MAINWINDOW_H

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
#include <QTimer>
#include "ui_mainWindow.h"

// Forward Declarations
class FermentableDialog;
class HopDialog;
class MiscDialog;
class YeastDialog;
class AboutDialog;
class Recipe;
class BeerColorWidget;
class FermentableEditor;
class MiscEditor;
class HopEditor;
class YeastEditor;
class EquipmentEditor;
class StyleEditor;
class OptionDialog;
class MaltinessWidget;
class MashEditor;
class MashStepEditor;
class MashWizard;
class BrewDayScrollWidget;
class HtmlViewer;
class ScaleRecipeTool;
class RecipeFormatter;
class OgAdjuster;
class ConverterTool;
class TimerListDialog;
class PrimingDialog;
class RecipeExtrasWidget;
class RefractoDialog;
class MashDesigner;
class MashListModel;
class PitchDialog;
class BrewNoteWidget;
class FermentableTableModel;
class FermentableSortFilterProxyModel;
class HopTableModel;
class HopSortFilterProxyModel;
class MiscTableModel;
class MiscSortFilterProxyModel;
class YeastTableModel;
class YeastSortFilterProxyModel;
class MashStepTableModel;
class EquipmentListModel;
class StyleListModel;
class StyleSortFilterProxyModel;
class NamedMashEditor;
class BtDatePopup;

/*!
 * \class MainWindow
 * \author Philip G. Lee
 *
 * \brief Brewtarget's main window. This is a view/controller class.
 */
class MainWindow : public QMainWindow, public Ui::mainWindow
{
   Q_OBJECT

   friend class OptionDialog;
public:
   MainWindow(QWidget* parent=0);
   virtual ~MainWindow() {}
   //! \brief View the given recipe.
   void setRecipe(Recipe* recipe);
   //! \brief Get the currently observed recipe.
   Recipe* currentRecipe();
   //! \brief Display a file dialog for writing xml files.
   QFile* openForWrite(QString filterStr = "BeerXML files (*.xml)", QString defaultSuff = "xml");

   bool verifyImport(QString tag, QString name);
   bool verifyDelete(QString tab, QString name);

   void setBrewNoteByIndex(const QModelIndex &index);
   void setBrewNote(BrewNote* bNote);

public slots:
   
   //! \brief Accepts Recipe changes, and takes appropriate action to show the changes.
   void changed(QMetaProperty,QVariant);
   
   void setRecipeByIndex(const QModelIndex &index);
   void treeActivated(const QModelIndex &index);

   //! \brief Update Recipe name to that given by the relevant widget.
   void updateRecipeName();
   //! \brief Update Recipe Style to that given by the relevant widget.
   void updateRecipeStyle();
   //! \brief Update Recipe Equipment to that given by the relevant widget.
   void updateRecipeEquipment();
   //! \brief Update Recipe batch size to that given by the relevant widget.
   void updateRecipeBatchSize();
   //! \brief Update Recipe boil size to that given by the relevant widget.
   void updateRecipeBoilSize();
   //! \brief Update Recipe boil time to that given by the relevant widget.
   void updateRecipeBoilTime();
   //! \brief Update Recipe efficiency to that given by the relevant widget.
   void updateRecipeEfficiency();
   //! \brief Update Recipe's mash
   void updateRecipeMash();

   //! \brief Add given Fermentable to the Recipe.
   void addFermentableToRecipe(Fermentable* ferm);
   //! \brief Remove selected Fermentable from the Recipe.
   void removeSelectedFermentable();
   //! \brief Edit selected Fermentable.
   void editSelectedFermentable();

   //! \brief Show the pitch dialog.
   void showPitchDialog();
   
   //! \brief Add given Hop to the Recipe.
   void addHopToRecipe(Hop *hop);
   //! \brief Remove selected Hop from the Recipe.
   void removeSelectedHop();
   //! \brief Edit selected Hop.
   void editSelectedHop();

   //! \brief Add given Misc to the Recipe.
   void addMiscToRecipe(Misc* misc);
   //! \brief Remove selected Misc from the Recipe.
   void removeSelectedMisc();
   //! \brief Edit selected Misc.
   void editSelectedMisc();

   //! \brief Add given Yeast to the Recipe.
   void addYeastToRecipe(Yeast* yeast);
   //! \brief Remove selected Yeast from the Recipe.
   void removeSelectedYeast();
   //! \brief Edit selected Yeast
   void editSelectedYeast();

   //! \brief Add a new mash step to the recipe.
   void addMashStep();
   //! \brief Move currently selected mash step down.
   void moveSelectedMashStepUp();
   //! \brief Move currently selected mash step up.
   void moveSelectedMashStepDown();
   //! \brief Remove currently selected mash step.
   void removeSelectedMashStep();
   //! \brief Edit currently selected mash step.
   void editSelectedMashStep();
   //! \brief Set the current recipe's mash to the one selected in the mash combo box.
   void setMashToCurrentlySelected();
   //! \brief Save the current recipe's mash to be used in other recipes.
   void saveMash();
   //! \brief Remove the current mash from the recipe, and replace with a blank one.
   void removeMash();

   //! \brief Create a new recipe in the database.
   void newRecipe();
   //! \brief Export current recipe to BeerXML.
   void exportRecipe();
   //! \brief Display file selection dialog and import BeerXML files.
   void importFiles();
   //! \brief Create a duplicate of the current recipe.
   void copyRecipe();
   
   void deleteSelected();
   void copySelected();
   void exportSelected();

   //! \brief Prints the right thing, depending on the signal sender.
   void print();
   //! \brief saves the database, which will have some interesting
   //implications later
   void save();
   //! \brief Backup the database.
   void backup();
   //! \brief Restore the database.
   void restoreFromBackup();

   //! \brief draws a context menu, the exact nature of which depends on which
   //tree is focused
   void contextMenu(const QPoint &point);
   //! \brief creates a new brewnote
   void newBrewNote();
   //! \brief copies an existing brewnote to a new brewday
   void reBrewNote();
   void changeBrewDate();
   void fixBrewNote();

   //! \brief Open the default browser to Brewtarget's donation page.
   void openDonateLink();

   //! \brief Merges two database files.
   void updateDatabase();
  
   //! \brief decides if we accept the drop event
   void dragEnterEvent(QDragEnterEvent *event);
   //! \brief handles the actual drop event
   void dropEvent(QDropEvent *event);

   //! \brief Catches a QNetworkReply signal and gets info about any new version available.
   void finishCheckingVersion();

   void redisplayLabel(QString field);

   void showEquipmentEditor();
   void showStyleEditor();

protected:
   virtual void closeEvent(QCloseEvent* event);

private slots:
   /*!
    * \brief Make the widgets in the window update changes.
    * 
    * Updates all the widgets with info about the currently
    * selected Recipe, except for the tables.
    * 
    * \param prop Not yet used. Will indicate which Recipe property has changed.
    */
   void showChanges(QMetaProperty* prop = 0);
   
   //! \brief Displays custom Fermentable context menu.
   void fermentableContextMenu(const QPoint& point);
   //! \brief Displays custom Hop context menu.
   void hopContextMenu(const QPoint& point);
   //! \brief Displays custom MashStep context menu.
   void mashStepContextMenu(const QPoint& point);
   //! \brief Displays custom Misc context menu.
   void miscContextMenu(const QPoint& point);
   //! \brief Displays custom Yeast context menu.
   void yeastContextMenu(const QPoint& point);
   
private:
   Recipe* recipeObs;
   Style* recStyle;
   Equipment* recEquip;
   
   AboutDialog* dialog_about;
   QFileDialog* fileOpener;
   QFileDialog* fileSaver;
   QList<QMenu*> contextMenus;
   EquipmentEditor* equipEditor;
   EquipmentEditor* singleEquipEditor;
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
   StyleEditor* singleStyleEditor;
   YeastDialog* yeastDialog;
   YeastEditor* yeastEditor;
   OptionDialog* optionDialog;
   MaltinessWidget* maltWidget;
   QDialog* brewDayDialog;
   HtmlViewer* htmlViewer;
   ScaleRecipeTool* recipeScaler;
   RecipeFormatter* recipeFormatter;
   OgAdjuster* ogAdjuster;
   ConverterTool* converterTool;
   TimerListDialog* timerListDialog;
   PrimingDialog* primingDialog;
   RefractoDialog* refractoDialog;
   MashDesigner* mashDesigner;
   PitchDialog* pitchDialog;
   QPrinter *printer;

   FermentableTableModel* fermTableModel;
   FermentableSortFilterProxyModel* fermTableProxy;
   HopTableModel* hopTableModel;
   HopSortFilterProxyModel* hopTableProxy;
   MiscTableModel* miscTableModel;
   MiscSortFilterProxyModel* miscTableProxy;
   YeastTableModel* yeastTableModel;
   YeastSortFilterProxyModel* yeastTableProxy;
   MashStepTableModel* mashStepTableModel;
   EquipmentListModel* equipmentListModel;
   MashListModel* mashListModel;
   StyleListModel* styleListModel;
   StyleSortFilterProxyModel* styleProxyModel;
  
   NamedMashEditor* namedMashEditor;
   NamedMashEditor* singleNamedMashEditor;

   BtDatePopup* btDatePopup;
   QHash<int, BrewNoteWidget*> brewNotes;
   int confirmDelete;

   //! \brief Currently highlighted fermentable in the fermentable table.
   Fermentable* selectedFermentable();
   //! \brief Currently highlighted hop in the hop table.
   Hop* selectedHop();
   //! \brief Currently highlighted misc in the misc table.
   Misc* selectedMisc();
   //! \brief Currently highlighted yeast in the yeast table
   Yeast* selectedYeast();

   //! \brief Scroll to the given \c item in the currently visible item tree.
   void setTreeSelection(QModelIndex item);

   //! \brief Set the equipment based on a drop event
   void droppedRecipeEquipment(Equipment *kit);
   //! \brief Set the keyboard shortcuts.
   void setupShortCuts();
   //! \brief Set the context menus.
   void setupContextMenu();

   // Copy methods used by copySelected()
   void copyThis(Recipe *rec);
   void copyThis(Equipment *kit);
   void copyThis(Fermentable *ferm);
   void copyThis(Hop *hop);
   void copyThis(Misc *misc);
   void copyThis(Style *style);
   void copyThis(Yeast *yeast);

   void convertedMsg(); 
};

#endif   /* _MAINWINDOW_H */

