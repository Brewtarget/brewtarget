 /*
 * MainWindow.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2020
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Samuel Östling <MrOstling@gmail.com>
 * - Ted Wright
 *
 * Brewtarget is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Brewtarget is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

class MainWindow;

#include <memory> // For PImpl

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
#include <QUndoStack>
#include "ui_mainWindow.h"
#include "SimpleUndoableUpdate.h"

#include <functional>

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
class MashEditor;
class MashStepEditor;
class MashWizard;
class BrewDayScrollWidget;
class HtmlViewer;
class ScaleRecipeTool;
class RecipeFormatter;
class PrintAndPreviewDialog;
class OgAdjuster;
class ConverterTool;
class HydrometerTool;
class AlcoholTool;
class TimerMainDialog;
class PrimingDialog;
class StrikeWaterDialog;
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
class AncestorDialog;

class WaterDialog;
class WaterListModel;
class WaterEditor;

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
   MainWindow(QWidget* parent=nullptr);
   virtual ~MainWindow();

   /**
    * \brief This needs to be called immediately after the constructor.  It does the remaining initialisation of the
    *        object.  This function cannot be called from the constructor as, in certain circumstances, it will invoke
    *        code that calls Brewtarget::mainWindow() which returns a pointer to the MainWindow and therefore needs the
    *        MainWindow constructor to have returned!
    */
   void init();

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

   void treeActivated(const QModelIndex &index);
   //! \brief View the given recipe.
   void setRecipe(Recipe* recipe);

   //! \brief Update Recipe name to that given by the relevant widget.
   void updateRecipeName();
   //! \brief Redisplay the OG, FG, etc ranges for the style of the current recipe.  Needs to be called whenever the recipe style is changed.
   void displayRangesEtcForCurrentRecipeStyle();
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

   //! \brief Update the main windows statusbar.
   void updateStatus(const QString status);

   //! \brief Close a brewnote tab if we must
   void closeBrewNote(BrewNote*);
   //! \brief Add given Fermentable to the Recipe.
   void addFermentableToRecipe(Fermentable* ferm);
   //! \brief Remove selected Fermentable(s) from the Recipe.
   void removeSelectedFermentable();
   //! \brief Edit selected Fermentable.
   void editSelectedFermentable();

   //! \brief Show the pitch dialog.
   void showPitchDialog();

   //! \brief Add given Hop to the Recipe.
   void addHopToRecipe(Hop *hop);
   //! \brief Remove selected Hop(s) from the Recipe.
   void removeSelectedHop();
   //! \brief Edit selected Hop.
   void editSelectedHop();

   //! \brief Add given Misc to the Recipe.
   void addMiscToRecipe(Misc* misc);
   //! \brief Remove selected Misc(s) from the Recipe.
   void removeSelectedMisc();
   //! \brief Edit selected Misc.
   void editSelectedMisc();

   //! \brief Add given Yeast to the Recipe.
   void addYeastToRecipe(Yeast* yeast);
   //! \brief Remove selected Yeast(s) from the Recipe.
   void removeSelectedYeast();
   //! \brief Edit selected Yeast
   void editSelectedYeast();

   //! \brief Invoke the pop-up Window to add a new mash step to (the mash of) the recipe.
   void addMashStep();
   //! \brief Actually add the new mash step to (the mash of) the recipe (in an undoable way).
   void addMashStepToMash(MashStep*);
   //! \brief Update the display once a mash step is added.
   void postAddMashStepToMash(MashStep * mashStep);
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

   //! \brief Implements "> Edit > Undo"
   void editUndo();
   //! \brief Implements "> Edit > Redo"
   void editRedo();

   //! \brief Create a new folder
   void newFolder();
   void renameFolder();

   void deleteSelected();
   void copySelected();
   void exportSelected();

   //! \brief Backup the database.
   void backup();
   //! \brief Restore the database.
   void restoreFromBackup();

   //! \brief makes sure we can do water chemistry before we show the window
   void popChemistry();

   //! \brief draws a context menu, the exact nature of which depends on which
   //tree is focused
   void contextMenu(const QPoint &point);
   //! \brief creates a new brewnote
   void newBrewNote();
   //! \brief copies an existing brewnote to a new brewday
   void reBrewNote();
   void brewItHelper();
   void brewAgainHelper();
   void reduceInventory();
   void changeBrewDate();
   void fixBrewNote();

   //! \brief Open the default browser to view Brewtarget manual.
    void openManual();

   //! \brief Merges two database files.
   void updateDatabase();

   //! \brief Catches a QNetworkReply signal and gets info about any new version available.
   void finishCheckingVersion();

   void redisplayLabel();

   void showEquipmentEditor();
   void showStyleEditor();

   void updateEquipmentButton();

   //! \brief Set all the things based on a drop event
   void droppedRecipeEquipment(Equipment *kit);
   void droppedRecipeStyle(Style *style);
   void droppedRecipeFermentable(QList<Fermentable*>ferms);
   void droppedRecipeHop(QList<Hop*>hops);
   void droppedRecipeMisc(QList<Misc*>miscs);
   void droppedRecipeYeast(QList<Yeast*>yeasts);

   void versionedRecipe(Recipe* descendant);
   //! \brief Doing updates via this method makes them undoable (and redoable).  This is the most generic version
   //         which requires the caller to construct a QUndoCommand.
   void doOrRedoUpdate(QUndoCommand * update);
   //! \brief to lock or not was never the question before now.
   void lockRecipe(int state);
   //! \brief prepopulate the ancestorDialog when the menu is selected
   void setAncestor();

public:
   //! \brief Doing updates via this method makes them undoable (and redoable).  This is the simplified version
   //         which suffices for modifications to most individual non-relational attributes.
   void doOrRedoUpdate(QObject & updatee,
                       char const * const propertyName,
                       QVariant newValue,
                       QString const & description,
                       QUndoCommand * parent = nullptr);

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
   void showChanges(QMetaProperty* prop = nullptr);

   //! \brief Set whether undo / redo commands are enabled
   void setUndoRedoEnable();

private:
   // Private implementation details - see https://herbsutter.com/gotw/_100/
   class impl;
   std::unique_ptr<impl> pimpl;

   void removeHop(Hop * itemToRemove);
   void removeFermentable(Fermentable * itemToRemove);
   void removeMisc(Misc * itemToRemove);
   void removeYeast(Yeast * itemToRemove);
   void removeMashStep(MashStep * itemToRemove);
//   void removeWater(Water * itemToRemove);
//   void removeSalt(Salt * itemToRemove);

   Recipe* recipeObs;
   // TBD: (MY 2020-11-24) Not sure whether we need to store recipe style (since it ought to be available from the
   //      recipe) or whether this is just for convenience.
   Style* recStyle;
   Equipment* recEquip;

   QString highSS, lowSS, goodSS, boldSS; // Palette replacements

   AboutDialog* dialog_about;
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
   QDialog* brewDayDialog;
   ScaleRecipeTool* recipeScaler;
   RecipeFormatter* recipeFormatter;
   PrintAndPreviewDialog* printAndPreviewDialog;
   OgAdjuster* ogAdjuster;
   ConverterTool* converterTool;
   HydrometerTool* hydrometerTool;
   AlcoholTool* alcoholTool;
   TimerMainDialog* timerMainDialog;
   PrimingDialog* primingDialog;
   StrikeWaterDialog* strikeWaterDialog;
   RefractoDialog* refractoDialog;
   MashDesigner* mashDesigner;
   PitchDialog* pitchDialog;
   QPrinter *printer;

   WaterDialog* waterDialog;
   WaterEditor* waterEditor;

   AncestorDialog* ancestorDialog;
   // all things tables should go here.
   FermentableTableModel* fermTableModel;
   HopTableModel* hopTableModel;
   MashStepTableModel* mashStepTableModel;
   MiscTableModel* miscTableModel;
   YeastTableModel* yeastTableModel;

   // all things lists should go here
   EquipmentListModel* equipmentListModel;
   MashListModel* mashListModel;
   StyleListModel* styleListModel;
//   WaterListModel* waterListModel;  Appears to be unused...

   // all things sort/filter proxy go here
   FermentableSortFilterProxyModel* fermTableProxy;
   HopSortFilterProxyModel* hopTableProxy;
   MiscSortFilterProxyModel* miscTableProxy;
   StyleSortFilterProxyModel* styleProxyModel;
   YeastSortFilterProxyModel* yeastTableProxy;

   NamedMashEditor* namedMashEditor;
   NamedMashEditor* singleNamedMashEditor;

   BtDatePopup* btDatePopup;
   int confirmDelete;

   // Undo / Redo, using the Qt Undo framework
   QUndoStack * undoStack = nullptr;

   //! \brief Fix pixel dimensions according to dots-per-inch (DPI) of screen we're on.
   void setSizesInPixelsBasedOnDpi();

   //! \brief Currently highlighted fermentable in the fermentable table.
   Fermentable* selectedFermentable();
   //! \brief Currently highlighted hop in the hop table.
   Hop* selectedHop();
   //! \brief Currently highlighted misc in the misc table.
   Misc* selectedMisc();
   //! \brief Currently highlighted yeast in the yeast table
   Yeast* selectedYeast();

   //! \brief Find an open brewnote tab, if it is open
   BrewNoteWidget* findBrewNoteWidget(BrewNote* b);

   //! \brief Scroll to the given \c item in the currently visible item tree.
   void setTreeSelection(QModelIndex item);

   //! \brief Set the keyboard shortcuts.
   void setupShortCuts();
   //! \brief Set the BtTreeView context menus.
   void setupContextMenu();
   //! \brief Create the CSS strings
   void setupCSS();
   //! \brief Create the dialogs, including the file dialogs
   void setupDialogs();
   //! \brief Configure the range sliders
   void setupRanges();
   //! \brief Configure combo boxes and their list models
   void setupComboBoxes();
   //! \brief Configure the tables and their proxies
   void setupTables();
   //! \brief Restore any saved states
   void restoreSavedState();
   //! \brief Connect the signal/slots for actions
   void setupTriggers();
   //! \brief Connect signal/slots for buttons
   void setupClicks();
   //! \brief Connect signal/slots for combo boxes
   void setupActivate();
   //! \brief Connect signal/slots for combo boxes
   void setupLabels();
   //! \brief Connect signal/slots for text edits
   void setupTextEdit();
   //! \brief Connect signal/slots drag/drop
   void setupDrops();
   //! \brief Connect signal/slots for check boxes
   void setUpStateChanges();


   void updateDensitySlider(QString attribute, RangedSlider* slider, double max);
   void updateColorSlider(QString attribute, RangedSlider* slider);

   void convertedMsg();

};

#endif   /* _MAINWINDOW_H */

