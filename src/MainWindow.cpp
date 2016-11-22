/*
 * MainWindow.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - David Grundberg <individ@acc.umu.se>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Maxime Lavigne <duguigne@gmail.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - plut0nium
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

#include <QWidget>
#include <QMainWindow>
#include <QMessageBox>
#include <QToolButton>
#include <QSize>
#include <QtGui>
#include <QString>
#include <QFileDialog>
#include <QIcon>
#include <QPixmap>
#include <QList>
#include <QVector>
#include <QVBoxLayout>
#include <QDomDocument>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDomNodeList>
#include <QDomNode>
#include <QDomElement>
#include <QInputDialog>
#include <QLineEdit>
#include <QUrl>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QAction>
#include <QLinearGradient>
#include <QBrush>
#include <QPen>
#include <QDesktopWidget>

#include "Algorithms.h"
#include "MashStepEditor.h"
#include "MashStepTableModel.h"
#include "mash.h"
#include "MashEditor.h"
#include "brewtarget.h"
#include "FermentableEditor.h"
#include "MiscEditor.h"
#include "HopEditor.h"
#include "YeastEditor.h"
#include "YeastTableModel.h"
#include "MiscTableModel.h"
#include "style.h"
#include "recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "database.h"
#include "YeastDialog.h"
#include "config.h"
#include "unit.h"
#include "ScaleRecipeTool.h"
#include "HopTableModel.h"
#include "BtDigitWidget.h"
#include "FermentableTableModel.h"
#include "BrewNoteWidget.h"
#include "EquipmentEditor.h"
#include "FermentableDialog.h"
#include "HopDialog.h"
#include "InventoryFormatter.h"
#include "MashWizard.h"
#include "MiscDialog.h"
#include "StyleEditor.h"
#include "OptionDialog.h"
#include "OgAdjuster.h"
#include "ConverterTool.h"
#include "HydrometerTool.h"
#include "TimerMainDialog.h"
#include "RecipeFormatter.h"
#include "PrimingDialog.h"
#include "StrikeWaterDialog.h"
#include "RefractoDialog.h"
#include "MashDesigner.h"
#include "PitchDialog.h"
#include "fermentable.h"
#include "yeast.h"
#include "brewnote.h"
#include "equipment.h"
#include "FermentableTableModel.h"
#include "FermentableSortFilterProxyModel.h"
#include "HopTableModel.h"
#include "HopSortFilterProxyModel.h"
#include "MiscTableModel.h"
#include "MiscSortFilterProxyModel.h"
#include "YeastSortFilterProxyModel.h"
#include "EquipmentListModel.h"
#include "StyleListModel.h"
#include "MashListModel.h"
#include "StyleSortFilterProxyModel.h"
#include "NamedMashEditor.h"
#include "BtDatePopup.h"
#if defined(Q_OS_WIN)
   #include <windows.h>
#endif

#include <memory>

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);

   /* PLEASE DO NOT REMOVE.
    This code is left here, commented out, intentionally. The only way I can
    test internationalization is by forcing the locale manually. I am tired
    of having to figure this out every time I need to test.
    PLEASE DO NOT REMOVE.
   QLocale german(QLocale::German,QLocale::Germany);
   QLocale::setDefault(german);
   */


   // If the database doesn't load, we bail
   if (! Database::instance().loadSuccessful() )
      exit(1);

   // Set the window title.
   setWindowTitle( QString("Brewtarget - %1").arg(VERSIONSTRING) );

   // Null out the recipe
   recipeObs = 0;

   // Set up the printer
   printer = new QPrinter;
   printer->setPageSize(QPrinter::Letter);

   setupCSS();
   // initialize all of the dialog windows
   setupDialogs();
   // initialize the ranged sliders
   setupRanges();
   // the dialogs have to be setup before this is called
   setupComboBoxes();
   // do all the work to configure the tables models and their proxies
   setupTables();
   // Create the keyboard shortcuts
   setupShortCuts();
   // Once more with the context menus too
   setupContextMenu();
   // Breaks the naming convention, doesn't it?
   restoreSavedState();
   // Connect slots to triggered() signals
   setupTriggers();
   // Connect slots to clicked() signals
   setupClicks();
   // connect slots to activate() signals
   setupActivate();
   // connect signal/slots for labels
   setupLabels();
   // connect signal slots for the text editors
   setupTextEdit();
   // connect the remaining labels
   setupLabels();
   // and (finally) set up the drag/drop parts
   setupDrops();

   // No connections from the database yet? Oh FSM, that probably means I'm
   // doing it wrong again.
   connect( &(Database::instance()), SIGNAL( deletedSignal(BrewNote*)), this, SLOT( closeBrewNote(BrewNote*)));
}

// Setup the keyboard shortcuts
void MainWindow::setupShortCuts()
{
   actionNewRecipe->setShortcut(QKeySequence::New);
   actionCopy_Recipe->setShortcut(QKeySequence::Copy);
   actionDeleteSelected->setShortcut(QKeySequence::Delete);
}

// Any manipulation of CSS for the MainWindow should be in here
void MainWindow::setupCSS()
{
   // Different palettes for some text. This is all done via style sheets now.
   QColor wPalette = tabWidget_recipeView->palette().color(QPalette::Active,QPalette::Base);

   goodSS = QString( "QLineEdit:read-only { color: #008800; background: %1 }").arg(wPalette.name());
   lowSS  = QString( "QLineEdit:read-only { color: #0000D0; background: %1 }").arg(wPalette.name());
   highSS = QString( "QLineEdit:read-only { color: #D00000; background: %1 }").arg(wPalette.name());
   boldSS = QString( "QLineEdit:read-only { font: bold 12px; color: #000000; background: %1 }").arg(wPalette.name());

   // The bold style sheet doesn't change, so set it here once.
   lineEdit_boilSg->setStyleSheet(boldSS);
}

// Any dialogs should be initialized in here. That should include any initial
// configurations as well
void MainWindow::setupDialogs()
{
   dialog_about = new AboutDialog(this);
   equipEditor = new EquipmentEditor(this);
   singleEquipEditor = new EquipmentEditor(this, true);
   fermDialog = new FermentableDialog(this);
   fermEditor = new FermentableEditor(this);
   hopDialog = new HopDialog(this);
   hopEditor = new HopEditor(this);
   mashEditor = new MashEditor(this);
   mashStepEditor = new MashStepEditor(this);
   mashWizard = new MashWizard(this);
   miscDialog = new MiscDialog(this);
   miscEditor = new MiscEditor(this);
   styleEditor = new StyleEditor(this);
   singleStyleEditor = new StyleEditor(this,true);
   yeastDialog = new YeastDialog(this);
   yeastEditor = new YeastEditor(this);
   optionDialog = new OptionDialog(this);
   recipeScaler = new ScaleRecipeTool(this);
   recipeFormatter = new RecipeFormatter(this);
   ogAdjuster = new OgAdjuster(this);
   converterTool = new ConverterTool(this);
   hydrometerTool = new HydrometerTool(this);
   timerMainDialog = new TimerMainDialog(this);
   primingDialog = new PrimingDialog(this);
   strikeWaterDialog = new StrikeWaterDialog(this);
   refractoDialog = new RefractoDialog(this);
   mashDesigner = new MashDesigner(this);
   pitchDialog = new PitchDialog(this);
   btDatePopup = new BtDatePopup(this);

   // Set up the fileOpener dialog.
   fileOpener = new QFileDialog(this, tr("Open"), QDir::homePath(), tr("BeerXML files (*.xml)"));
   fileOpener->setAcceptMode(QFileDialog::AcceptOpen);
   fileOpener->setFileMode(QFileDialog::ExistingFiles);
   fileOpener->setViewMode(QFileDialog::List);

   // Set up the fileSaver dialog.
   fileSaver = new QFileDialog(this, tr("Save"), QDir::homePath(), tr("BeerXML files (*.xml)") );
   fileSaver->setAcceptMode(QFileDialog::AcceptSave);
   fileSaver->setFileMode(QFileDialog::AnyFile);
   fileSaver->setViewMode(QFileDialog::List);
   fileSaver->setDefaultSuffix(QString("xml"));

}

// Configures the range widgets for the bubbles
void MainWindow::setupRanges()
{
   styleRangeWidget_og->setRange(1.000, 1.120);
   styleRangeWidget_og->setPrecision(3);
   styleRangeWidget_og->setTickMarks(0.010, 2);

   styleRangeWidget_fg->setRange(1.000, 1.030);
   styleRangeWidget_fg->setPrecision(3);
   styleRangeWidget_fg->setTickMarks(0.010, 2);

   styleRangeWidget_abv->setRange(0.0, 15.0);
   styleRangeWidget_abv->setPrecision(1);
   styleRangeWidget_abv->setTickMarks(1, 2);

   styleRangeWidget_ibu->setRange(0.0, 120.0);
   styleRangeWidget_ibu->setPrecision(1);
   styleRangeWidget_ibu->setTickMarks(10, 2);

   const int srmMax = 50;
   styleRangeWidget_srm->setRange(0.0, static_cast<double>(srmMax));
   styleRangeWidget_srm->setPrecision(1);
   styleRangeWidget_srm->setTickMarks(10, 2);
   // Need to change appearance of color slider
   {
      // The styleRangeWidget_srm should display beer color in the background
      QLinearGradient grad( 0,0, 1,0 );
      grad.setCoordinateMode(QGradient::ObjectBoundingMode);
      for( int i=0; i <= srmMax; ++i )
      {
         double srm = i;
         grad.setColorAt( srm/static_cast<double>(srmMax), Algorithms::srmToColor(srm));
      }
      styleRangeWidget_srm->setBackgroundBrush(grad);

      // The styleRangeWidget_srm should display a "window" to show acceptable colors for the style
      styleRangeWidget_srm->setPreferredRangeBrush(QColor(0,0,0,0));
      styleRangeWidget_srm->setPreferredRangePen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

      // Half-height "tick" for color marker
      grad = QLinearGradient( 0,0, 0,1 );
      grad.setCoordinateMode(QGradient::ObjectBoundingMode);
      grad.setColorAt( 0, QColor(255,255,255,255) );
      grad.setColorAt( 0.49, QColor(255,255,255,255) );
      grad.setColorAt( 0.50, QColor(255,255,255,0) );
      grad.setColorAt( 1, QColor(255,255,255,0) );
      styleRangeWidget_srm->setMarkerBrush(grad);
   }
}

// Any new combo boxes, along with their list models, should be initialized
// here
void MainWindow::setupComboBoxes()
{
   // Set equipment combo box model.
   equipmentListModel = new EquipmentListModel(equipmentComboBox);
   equipmentComboBox->setModel(equipmentListModel);

   // Set the style combo box
   styleListModel = new StyleListModel(styleComboBox);
   styleProxyModel = new StyleSortFilterProxyModel(styleComboBox);
   styleProxyModel->setDynamicSortFilter(true);
   styleProxyModel->setSourceModel(styleListModel);
   styleComboBox->setModel(styleProxyModel);

   // Set the mash combo box
   mashListModel =  new MashListModel(mashComboBox);
   mashComboBox->setModel(mashListModel);

   // Nothing to say.
   namedMashEditor = new NamedMashEditor(this, mashStepEditor);
   // I don't think this is used yet
   singleNamedMashEditor = new NamedMashEditor(this,mashStepEditor,true);

}

// Anything creating new tables models, filter proxies and configuring the two
// should go in here
void MainWindow::setupTables()
{
   // Set table models.
   // Fermentables
   fermTableModel = new FermentableTableModel(fermentableTable);
   fermTableProxy = new FermentableSortFilterProxyModel(fermentableTable,false);
   fermTableProxy->setSourceModel(fermTableModel);
   fermentableTable->setItemDelegate(new FermentableItemDelegate(fermentableTable));
   fermentableTable->setModel(fermTableProxy);
   // Make the fermentable table show grain percentages in row headers.
   fermTableModel->setDisplayPercentages(true);

   // Hops
   hopTableModel = new HopTableModel(hopTable);
   hopTableProxy = new HopSortFilterProxyModel(hopTable, false);
   hopTableProxy->setSourceModel(hopTableModel);
   hopTable->setItemDelegate(new HopItemDelegate(hopTable));
   hopTable->setModel(hopTableProxy);
   // Hop table show IBUs in row headers.
   hopTableModel->setShowIBUs(true);

   // Misc
   miscTableModel = new MiscTableModel(miscTable);
   miscTableProxy = new MiscSortFilterProxyModel(miscTable,false);
   miscTableProxy->setSourceModel(miscTableModel);
   miscTable->setItemDelegate(new MiscItemDelegate(miscTable));
   miscTable->setModel(miscTableProxy);

   // Yeast
   yeastTableModel = new YeastTableModel(yeastTable);
   yeastTableProxy = new YeastSortFilterProxyModel(yeastTable,false);
   yeastTableProxy->setSourceModel(yeastTableModel);
   yeastTable->setItemDelegate(new YeastItemDelegate(yeastTable));
   yeastTable->setModel(yeastTableProxy);

   // Mashes
   mashStepTableModel = new MashStepTableModel(mashStepTableWidget);
   mashStepTableWidget->setItemDelegate(new MashStepItemDelegate());
   mashStepTableWidget->setModel(mashStepTableModel);

   // Enable sorting in the main tables.
   fermentableTable->horizontalHeader()->setSortIndicator( FERMAMOUNTCOL, Qt::DescendingOrder );
   fermentableTable->setSortingEnabled(true);
   fermTableProxy->setDynamicSortFilter(true);
   hopTable->horizontalHeader()->setSortIndicator( HOPTIMECOL, Qt::DescendingOrder );
   hopTable->setSortingEnabled(true);
   hopTableProxy->setDynamicSortFilter(true);
   miscTable->horizontalHeader()->setSortIndicator( MISCUSECOL, Qt::DescendingOrder );
   miscTable->setSortingEnabled(true);
   miscTableProxy->setDynamicSortFilter(true);
   yeastTable->horizontalHeader()->setSortIndicator( YEASTNAMECOL, Qt::DescendingOrder );
   yeastTable->setSortingEnabled(true);
   yeastTableProxy->setDynamicSortFilter(true);
}

// Anything resulting in a restoreState() should go in here
void MainWindow::restoreSavedState()
{
   QDesktopWidget *desktop = QApplication::desktop();

   // If we saved a size the last time we ran, use it
   if ( Brewtarget::hasOption("geometry"))
   {
      restoreGeometry(Brewtarget::option("geometry").toByteArray());
      restoreState(Brewtarget::option("windowState").toByteArray());
   }
   else
   {
      // otherwise, guess a reasonable size at 1/4 of the screen.
      int width = desktop->width();
      int height = desktop->height();

      this->resize(width/2,height/2);
   }

   // If we saved the selected recipe name the last time we ran, select it and show it.
   if (Brewtarget::hasOption("recipeKey"))
   {
      int key = Brewtarget::option("recipeKey").toInt();
      recipeObs = Database::instance().recipe( key );
      QModelIndex rIdx = treeView_recipe->findElement(recipeObs);

      setRecipe(recipeObs);
      setTreeSelection(rIdx);
   }
   else
   {
      QList<Recipe*> recs = Database::instance().recipes();
      if( recs.size() > 0 )
         setRecipe( recs[0] );
   }

   //UI restore state
   if (Brewtarget::hasOption("MainWindow/splitter_vertical_State"))
      splitter_vertical->restoreState(Brewtarget::option("MainWindow/splitter_vertical_State").toByteArray());
   if (Brewtarget::hasOption("MainWindow/splitter_horizontal_State"))
      splitter_horizontal->restoreState(Brewtarget::option("MainWindow/splitter_horizontal_State").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_recipe_headerState"))
      treeView_recipe->header()->restoreState(Brewtarget::option("MainWindow/treeView_recipe_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_style_headerState"))
      treeView_style->header()->restoreState(Brewtarget::option("MainWindow/treeView_style_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_equip_headerState"))
      treeView_equip->header()->restoreState(Brewtarget::option("MainWindow/treeView_equip_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_ferm_headerState"))
      treeView_ferm->header()->restoreState(Brewtarget::option("MainWindow/treeView_ferm_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_hops_headerState"))
      treeView_hops->header()->restoreState(Brewtarget::option("MainWindow/treeView_hops_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_misc_headerState"))
      treeView_misc->header()->restoreState(Brewtarget::option("MainWindow/treeView_misc_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/treeView_yeast_headerState"))
      treeView_yeast->header()->restoreState(Brewtarget::option("MainWindow/treeView_yeast_headerState").toByteArray());
   if (Brewtarget::hasOption("MainWindow/mashStepTableWidget_headerState"))
      mashStepTableWidget->horizontalHeader()->restoreState(Brewtarget::option("MainWindow/mashStepTableWidget_headerState").toByteArray());
}

// anything with a SIGNAL of triggered() should go in here.
void MainWindow::setupTriggers()
{
   // actions
   connect( actionExit, SIGNAL( triggered() ), this, SLOT( close() ) );
   connect( actionAbout_BrewTarget, SIGNAL( triggered() ), dialog_about, SLOT( show() ) );
   connect( actionNewRecipe, SIGNAL( triggered() ), this, SLOT( newRecipe() ) );
   connect( actionImport_Recipes, SIGNAL( triggered() ), this, SLOT( importFiles() ) );
   connect( actionExportRecipe, SIGNAL( triggered() ), this, SLOT( exportRecipe() ) );
   connect( actionEquipments, SIGNAL( triggered() ), equipEditor, SLOT( show() ) );
   connect( actionMashs, SIGNAL( triggered() ), namedMashEditor, SLOT( show() ) );
   connect( actionStyles, SIGNAL( triggered() ), styleEditor, SLOT( show() ) );
   connect( actionFermentables, SIGNAL( triggered() ), fermDialog, SLOT( show() ) );
   connect( actionHops, SIGNAL( triggered() ), hopDialog, SLOT( show() ) );
   connect( actionMiscs, SIGNAL( triggered() ), miscDialog, SLOT( show() ) );
   connect( actionYeasts, SIGNAL( triggered() ), yeastDialog, SLOT( show() ) );
   connect( actionOptions, SIGNAL( triggered() ), optionDialog, SLOT( show() ) );
   connect( actionManual, SIGNAL( triggered() ), this, SLOT( openManual() ) );
   connect( actionScale_Recipe, SIGNAL( triggered() ), recipeScaler, SLOT( show() ) );
   connect( action_recipeToTextClipboard, SIGNAL( triggered() ), recipeFormatter, SLOT( toTextClipboard() ) );
   connect( actionConvert_Units, SIGNAL( triggered() ), converterTool, SLOT( show() ) );
   connect( actionHydrometer_Temp_Adjustment, SIGNAL( triggered() ), hydrometerTool, SLOT( show() ) );
   connect( actionOG_Correction_Help, SIGNAL( triggered() ), ogAdjuster, SLOT( show() ) );
   connect( actionCopy_Recipe, SIGNAL( triggered() ), this, SLOT( copyRecipe() ) );
   connect( actionPriming_Calculator, SIGNAL( triggered() ), primingDialog, SLOT( show() ) );
   connect( actionStrikeWater_Calculator, SIGNAL( triggered() ), strikeWaterDialog, SLOT( show() ) );
   connect( actionRefractometer_Tools, SIGNAL( triggered() ), refractoDialog, SLOT( show() ) );
   connect( actionPitch_Rate_Calculator, SIGNAL(triggered()), this, SLOT(showPitchDialog()));
   connect( actionMergeDatabases, SIGNAL(triggered()), this, SLOT(updateDatabase()) );
   connect( actionTimers, SIGNAL(triggered()), timerMainDialog, SLOT(show()) );
   connect( actionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()) );

   // postgresql cannot backup or restore yet. I would like to find some way
   // around this, but for now just disable
   if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
      actionBackup_Database->setEnabled(false);
      actionRestore_Database->setEnabled(false);
      label_Brewtarget->setToolTip( recipeFormatter->getLabelToolTip());
   }
   else {
      connect( actionBackup_Database, SIGNAL( triggered() ), this, SLOT( backup() ) );
      connect( actionRestore_Database, SIGNAL( triggered() ), this, SLOT( restoreFromBackup() ) );
   }
   // Printing signals/slots.
   // Refactoring is good.  It's like a rye saison fermenting away
   connect(actionRecipePrint, &QAction::triggered, [this]() {
      print([this](QPrinter* printer) {
         recipeFormatter->print(
               printer,  RecipeFormatter::PRINT);
      });
   });
   connect(actionRecipePreview, &QAction::triggered, [this]() {
      recipeFormatter->print(printer, RecipeFormatter::PREVIEW);
   });
   connect(actionRecipeHTML, &QAction::triggered, this, [this]() {
      exportHTML([this](QFile* file) {
         recipeFormatter->print(printer, RecipeFormatter::HTML, file);
      });
   });
   connect(actionRecipeBBCode, &QAction::triggered, [this]() {
      QApplication::clipboard()->setText(recipeFormatter->getBBCodeFormat());
   });
   connect(actionBrewdayPrint, &QAction::triggered, [this]() {
      print([this](QPrinter* printer) {
         brewDayScrollWidget->print(
               printer,  BrewDayScrollWidget::PRINT);
      });
   });
   connect(actionBrewdayPreview, &QAction::triggered, [this]() {
      brewDayScrollWidget->print(printer, RecipeFormatter::PREVIEW);
   });
   connect(actionBrewdayHTML, &QAction::triggered, this, [this]() {
      exportHTML([this](QFile* file) {
         brewDayScrollWidget->print(
               printer,  BrewDayScrollWidget::PRINT);
      });
   });
   connect(actionInventoryPrint, &QAction::triggered, [this]() {
      print(
            [](QPrinter* printer) { InventoryFormatter::print(printer); });
   });
   connect(actionInventoryPreview, &QAction::triggered,
         []() { InventoryFormatter::printPreview(); });
   connect(actionInventoryHTML, &QAction::triggered, [this]() {
      exportHTML(
            [](QFile* file) { InventoryFormatter::exportHTML(file); });
   });
}

// anything with a SIGNAL of clicked() should go in here.
void MainWindow::setupClicks()
{
   connect( equipmentButton, SIGNAL( clicked() ), this, SLOT(showEquipmentEditor()));
   connect( styleButton, SIGNAL( clicked() ), this, SLOT(showStyleEditor()) );
   connect( mashButton, SIGNAL( clicked() ), mashEditor, SLOT( showEditor() ) );
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
   connect( pushButton_editYeast, SIGNAL( clicked() ), this, SLOT( editSelectedYeast() ) );
   connect( pushButton_editMash, SIGNAL( clicked() ), mashEditor, SLOT( showEditor() ) );
   connect( pushButton_addMashStep, SIGNAL( clicked() ), this, SLOT(addMashStep()) );
   connect( pushButton_removeMashStep, SIGNAL( clicked() ), this, SLOT(removeSelectedMashStep()) );
   connect( pushButton_editMashStep, SIGNAL( clicked() ), this, SLOT(editSelectedMashStep()) );
   connect( pushButton_mashWizard, SIGNAL( clicked() ), mashWizard, SLOT( show() ) );
   connect( pushButton_saveMash, SIGNAL( clicked() ), this, SLOT( saveMash() ) );
   connect( pushButton_mashDes, SIGNAL( clicked() ), mashDesigner, SLOT( show() ) );
   connect( pushButton_mashUp, SIGNAL( clicked() ), this, SLOT( moveSelectedMashStepUp() ) );
   connect( pushButton_mashDown, SIGNAL( clicked() ), this, SLOT( moveSelectedMashStepDown() ) );
   connect( pushButton_mashRemove, SIGNAL( clicked() ), this, SLOT( removeMash() ) );
}

// anything with a SIGNAL of activated() should go in here.
void MainWindow::setupActivate()
{
   connect( equipmentComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeEquipment()) );
   connect( styleComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeStyle()) );
   connect( mashComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeMash()) );
}

// anything with either an editingFinished() or a textModified() should go in
// here
void MainWindow::setupTextEdit()
{
   connect( lineEdit_name, SIGNAL( editingFinished() ), this, SLOT( updateRecipeName() ) );
   connect( lineEdit_batchSize, SIGNAL( textModified() ), this, SLOT( updateRecipeBatchSize() ) );
   connect( lineEdit_boilSize, SIGNAL( textModified() ), this, SLOT( updateRecipeBoilSize() ) );
   connect( lineEdit_boilTime, SIGNAL( textModified() ), this, SLOT( updateRecipeBoilTime() ) );
   connect( lineEdit_efficiency, SIGNAL( textModified() ), this, SLOT( updateRecipeEfficiency() ) );
}

// anything using a BtLabel::labelChanged signal should go in here
void MainWindow::setupLabels()
{
   // These are the sliders. I need to consider these harder, but small steps
   connect(oGLabel,       &BtLabel::labelChanged,
           this,          &MainWindow::redisplayLabel);
   connect(fGLabel,       &BtLabel::labelChanged,
           this,          &MainWindow::redisplayLabel);
   connect(colorSRMLabel, &BtLabel::labelChanged,
           this,          &MainWindow::redisplayLabel);
}

// anything with a BtTabWidget::set* signal should go in here
void MainWindow::setupDrops()
{
   // drag and drop. maybe
   connect( tabWidget_recipeView,  &BtTabWidget::setRecipe,
            this,                  &MainWindow::setRecipe);
   connect( tabWidget_recipeView,  &BtTabWidget::setEquipment,
            this,                  &MainWindow::droppedRecipeEquipment);
   connect( tabWidget_recipeView,  &BtTabWidget::setStyle,
            this,                  &MainWindow::droppedRecipeStyle);
   connect( tabWidget_ingredients, &BtTabWidget::setFermentables,
            this,                  &MainWindow::droppedRecipeFermentable);
   connect( tabWidget_ingredients, &BtTabWidget::setHops,
            this,                  &MainWindow::droppedRecipeHop);
   connect( tabWidget_ingredients, &BtTabWidget::setMiscs,
            this,                  &MainWindow::droppedRecipeMisc);
   connect( tabWidget_ingredients, &BtTabWidget::setYeasts,
            this,                  &MainWindow::droppedRecipeYeast);
}

void MainWindow::deleteSelected()
{
   QModelIndexList selected;
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   // This happens after startup when nothing is selected
   if (!active)
      return;

   active->deleteSelected(active->selectionModel()->selectedRows());

   // This should be fixed to find the first nonfolder object in the tree
   QModelIndex first = active->first();
   if ( first.isValid() )
   {
      if (active->type(first) == BtTreeItem::RECIPE)
         setRecipe(treeView_recipe->recipe(first));
      setTreeSelection(first);
   }

}

void MainWindow::treeActivated(const QModelIndex &index)
{
   Equipment *kit;
   Fermentable *ferm;
   Hop* h;
   Misc *m;
   Yeast *y;
   Style *s;

   QObject* calledBy = sender();
   BtTreeView* active;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == 0 )
      return;

   active = qobject_cast<BtTreeView*>(calledBy);

   // If the sender cannot be morphed into a BtTreeView object
   if ( active == 0 )
      return;

   switch( active->type(index))
   {
      case BtTreeItem::RECIPE:
         setRecipe(treeView_recipe->recipe(index));
         break;
      case BtTreeItem::EQUIPMENT:
         kit = active->equipment(index);
         if ( kit )
         {
            singleEquipEditor->setEquipment(kit);
            singleEquipEditor->show();
         }
         break;
      case BtTreeItem::FERMENTABLE:
         ferm = active->fermentable(index);
         if ( ferm )
         {
            fermEditor->setFermentable(ferm);
            fermEditor->show();
         }
         break;
      case BtTreeItem::HOP:
         h = active->hop(index);
         if (h)
         {
            hopEditor->setHop(h);
            hopEditor->show();
         }
         break;
      case BtTreeItem::MISC:
         m = active->misc(index);
         if (m)
         {
            miscEditor->setMisc(m);
            miscEditor->show();
         }
         break;
      case BtTreeItem::STYLE:
         s = active->style(index);
         if ( s )
         {
            singleStyleEditor->setStyle(s);
            singleStyleEditor->show();
         }
         break;
      case BtTreeItem::YEAST:
         y = active->yeast(index);
         if (y)
         {
            yeastEditor->setYeast(y);
            yeastEditor->show();
         }
         break;
      case BtTreeItem::BREWNOTE:
         setBrewNoteByIndex(index);
         break;
      case BtTreeItem::FOLDER:  // default behavior is fine, but no warning
         break;
      default:
         Brewtarget::logW(QString("MainWindow::treeActivated Unknown type %1.").arg(treeView_recipe->type(index)));
   }
   treeView_recipe->setCurrentIndex(index);
}

void MainWindow::setBrewNoteByIndex(const QModelIndex &index)
{
   BrewNoteWidget* ni;

   BrewNote* bNote = treeView_recipe->brewNote(index);

   if ( ! bNote )
      return;
   // HERE
   // This is some clean up work. REMOVE FROM HERE TO THERE
   if ( bNote->projPoints() < 15 )
   {
      double pnts = bNote->projPoints();
      bNote->setProjPoints(pnts);
   }
   if ( bNote->effIntoBK_pct() < 10 )
   {
      bNote->calculateEffIntoBK_pct();
      bNote->calculateBrewHouseEff_pct();
   }
   // THERE

   Recipe* parent  = Database::instance().getParentRecipe(bNote);
   // I think this means a brew note for a different recipe has been selected.
   // We need to select that recipe, which will clear the current tabs
   if (  parent != recipeObs )
      setRecipe(parent);

   ni = findBrewNoteWidget(bNote);
   if ( ! ni )
   {
      ni = new BrewNoteWidget(tabWidget_recipeView);
      ni->setBrewNote(bNote);
   }

   tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
   tabWidget_recipeView->setCurrentWidget(ni);

}

BrewNoteWidget* MainWindow::findBrewNoteWidget(BrewNote* b)
{
   for (int i = 0; i < tabWidget_recipeView->count(); ++i)
   {
      if (tabWidget_recipeView->widget(i)->objectName() == "BrewNoteWidget")
      {
         BrewNoteWidget* ni = qobject_cast<BrewNoteWidget*>(tabWidget_recipeView->widget(i));
         if ( ni->isBrewNote(b) )
            return ni;
      }
   }
   return 0;
}

void MainWindow::setBrewNote(BrewNote* bNote)
{
   QString tabname;
   BrewNoteWidget* ni = findBrewNoteWidget(bNote);

   if ( ni )
   {
      tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   ni = new BrewNoteWidget(tabWidget_recipeView);
   ni->setBrewNote(bNote);

   tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
   tabWidget_recipeView->setCurrentWidget(ni);
}

// Can handle null recipes.
void MainWindow::setRecipe(Recipe* recipe)
{
   int tabs = 0;
   // Don't like void pointers.
   if( recipe == 0 )
      return;

   // Make sure this MainWindow is paying attention...
   if( recipeObs )
      disconnect( recipeObs, 0, this, 0 );
   recipeObs = recipe;

   recStyle = recipe->style();
   recEquip = recipe->equipment();

   if( recStyle )
   {
      styleRangeWidget_og->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, "og", Brewtarget::DENSITY ));
      styleRangeWidget_fg->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, "fg", Brewtarget::DENSITY ));

      styleRangeWidget_abv->setPreferredRange(recStyle->abvMin_pct(), recStyle->abvMax_pct());
      styleRangeWidget_ibu->setPreferredRange(recStyle->ibuMin(), recStyle->ibuMax());

      styleRangeWidget_srm->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, "color_srm", Brewtarget::COLOR ));
   }

   // Reset all previous recipe shit.
   fermTableModel->observeRecipe(recipe);
   hopTableModel->observeRecipe(recipe);
   miscTableModel->observeRecipe(recipe);
   yeastTableModel->observeRecipe(recipe);
   mashStepTableModel->setMash(recipeObs->mash());

   // Clean out any brew notes
   tabWidget_recipeView->setCurrentIndex(0);
   // Start closing from the right (highest index) down. Anything else dumps
   // core in the most unpleasant of fashions
   tabs = tabWidget_recipeView->count() - 1;
   for (int i = tabs; i >= 0; --i)
   {
      if (tabWidget_recipeView->widget(i)->objectName() == "BrewNoteWidget")
         tabWidget_recipeView->removeTab(i);
   }

   // Tell some of our other widgets to observe the new recipe.
   mashWizard->setRecipe(recipe);
   brewDayScrollWidget->setRecipe(recipe);
   equipmentListModel->observeRecipe(recipe);
   recipeFormatter->setRecipe(recipe);
   ogAdjuster->setRecipe(recipe);
   recipeExtrasWidget->setRecipe(recipe);
   mashDesigner->setRecipe(recipe);
   equipmentButton->setRecipe(recipe);
   singleEquipEditor->setEquipment(recEquip);
   styleButton->setRecipe(recipe);
   singleStyleEditor->setStyle(recStyle);

   mashEditor->setMash(recipeObs->mash());
   mashEditor->setEquipment(recEquip);

   mashButton->setMash(recipeObs->mash());
   recipeScaler->setRecipe(recipeObs);

   // If you don't connect this late, every previous set of an attribute
   // causes this signal to be slotted, which then causes showChanges() to be
   // called.
   connect( recipeObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   showChanges();
}

void MainWindow::changed(QMetaProperty prop, QVariant value)
{
   QString propName(prop.name());

   if( propName == "equipment" )
   {
      Equipment* newRecEquip = qobject_cast<Equipment*>(BeerXMLElement::extractPtr(value));
      recEquip = newRecEquip;

      singleEquipEditor->setEquipment(recEquip);
   }
   else if( propName == "style" )
   {
      //recStyle = recipeObs->style();
      recStyle = qobject_cast<Style*>(BeerXMLElement::extractPtr(value));
      singleStyleEditor->setStyle(recStyle);

   }

   showChanges(&prop);
}

void MainWindow::updateDensitySlider(QString attribute, RangedSlider* slider, double max)
{
   Unit::unitDisplay dispUnit = (Unit::unitDisplay)Brewtarget::option(attribute, Unit::noUnit, "tab_recipe", Brewtarget::UNIT).toInt();

   if ( dispUnit == Unit::noUnit )
      dispUnit = Brewtarget::densityUnit == Brewtarget::PLATO ? Unit::displayPlato : Unit::displaySg;

   slider->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, attribute, Brewtarget::DENSITY));
   slider->setRange(         Brewtarget::displayRange(tab_recipe, attribute, 1.000, max, Brewtarget::DENSITY ));

   if ( dispUnit == Unit::displayPlato )
   {
      slider->setPrecision(1);
      slider->setTickMarks(2,5);
   }
   else
   {
      slider->setPrecision(3);
      slider->setTickMarks(0.010, 2);
   }
}

void MainWindow::updateColorSlider(QString attribute, RangedSlider* slider)
{
   Unit::unitDisplay dispUnit = (Unit::unitDisplay)Brewtarget::option(attribute, Unit::noUnit, "tab_recipe", Brewtarget::UNIT).toInt();

   if ( dispUnit == Unit::noUnit )
      dispUnit = Brewtarget::colorUnit == Brewtarget::SRM ? Unit::displaySrm : Unit::displayEbc;

   slider->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, attribute,Brewtarget::COLOR));
   slider->setRange(Brewtarget::displayRange(tab_recipe, attribute, 1, 44, Brewtarget::COLOR) );
   slider->setTickMarks( dispUnit == Unit::displaySrm ? 10 : 40, 2);

}

void MainWindow::showChanges(QMetaProperty* prop)
{
   if( recipeObs == 0 )
      return;

   bool updateAll = (prop == 0);
   QString propName;

   if( prop )
   {
      propName = prop->name();
   }

   // May St. Stevens preserve me
   lineEdit_name->setText(recipeObs->name());
   lineEdit_batchSize->setText(recipeObs);
   lineEdit_boilSize->setText(recipeObs);
   lineEdit_efficiency->setText(recipeObs);
   lineEdit_boilTime->setText(recipeObs);
   lineEdit_name->setCursorPosition(0);
   lineEdit_batchSize->setCursorPosition(0);
   lineEdit_boilSize->setCursorPosition(0);
   lineEdit_efficiency->setCursorPosition(0);
   lineEdit_boilTime->setCursorPosition(0);

   lineEdit_calcBatchSize->setText(recipeObs);
   lineEdit_calcBoilSize->setText(recipeObs);

   // Color manipulation

   if( 0.95*recipeObs->batchSize_l() <= recipeObs->finalVolume_l() && recipeObs->finalVolume_l() <= 1.05*recipeObs->batchSize_l() )
      lineEdit_calcBatchSize->setStyleSheet(goodSS);
   else if( recipeObs->finalVolume_l() < 0.95*recipeObs->batchSize_l() )
      lineEdit_calcBatchSize->setStyleSheet(lowSS);
   else
      lineEdit_calcBatchSize->setStyleSheet(highSS);

   if( 0.95*recipeObs->boilSize_l() <= recipeObs->boilVolume_l() && recipeObs->boilVolume_l() <= 1.05*recipeObs->boilSize_l() )
      lineEdit_calcBoilSize->setStyleSheet(goodSS);
   else if( recipeObs->boilVolume_l() < 0.95* recipeObs->boilSize_l() )
      lineEdit_calcBoilSize->setStyleSheet(lowSS);
   else
      lineEdit_calcBoilSize->setStyleSheet(highSS);

   lineEdit_boilSg->setText(recipeObs);

   updateDensitySlider("og", styleRangeWidget_og, 1.120);
   styleRangeWidget_og->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"og",Units::sp_grav,0));

   updateDensitySlider("fg", styleRangeWidget_fg, 1.03);
   styleRangeWidget_fg->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"fg",Units::sp_grav,0));

   styleRangeWidget_abv->setValue(recipeObs->ABV_pct());
   styleRangeWidget_ibu->setValue(recipeObs->IBU());

   /* Colors need the same basic treatment as gravity */
   updateColorSlider("color_srm", styleRangeWidget_srm);
   styleRangeWidget_srm->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"color_srm",Units::srm,0));

   ibuGuSlider->setValue(recipeObs->IBU()/((recipeObs->og()-1)*1000));

   label_calories->setText( QString("%1").arg( Brewtarget::getVolumeUnitSystem() == SI ? recipeObs->calories33cl() : recipeObs->calories12oz(),0,'f',0) );

   // See if we need to change the mash in the table.
   if( (updateAll && recipeObs->mash()) ||
       (propName == "mash" && recipeObs->mash()) )
   {
      mashStepTableModel->setMash(recipeObs->mash());
   }

   // Not sure about this, but I am annoyed that modifying the hop usage
   // modifiers isn't automatically updating my display
   if ( updateAll ) {
     recipeObs->acceptHopChange( recipeObs->metaProperty("hops"), QVariant());
     hopTableProxy->invalidate();
   }
}

void MainWindow::updateRecipeName()
{
   if( recipeObs == 0 || ! lineEdit_name->isModified())
      return;

   recipeObs->setName(lineEdit_name->text());
}

void MainWindow::updateRecipeStyle()
{
   if( recipeObs == 0 )
      return;

   QModelIndex proxyIndex( styleProxyModel->index(styleComboBox->currentIndex(),0) );
   QModelIndex sourceIndex( styleProxyModel->mapToSource(proxyIndex) );
   Style* selected = styleListModel->at(sourceIndex.row());
   if( selected )
   {
      Database::instance().addToRecipe( recipeObs, selected );

      styleRangeWidget_og->setPreferredRange( Brewtarget::displayRange(selected, tab_recipe, "og", Brewtarget::DENSITY ));
      styleRangeWidget_fg->setPreferredRange( Brewtarget::displayRange(selected, tab_recipe, "fg", Brewtarget::DENSITY ));

      styleRangeWidget_abv->setPreferredRange(selected->abvMin_pct(), selected->abvMax_pct());
      styleRangeWidget_ibu->setPreferredRange(selected->ibuMin(), selected->ibuMax());
      styleRangeWidget_srm->setPreferredRange(Brewtarget::displayRange(selected, tab_recipe, "color_srm", Brewtarget::COLOR));
   }
}

void MainWindow::updateRecipeMash()
{
   if( recipeObs == 0 )
      return;

   Mash* selected = mashListModel->at(mashComboBox->currentIndex());
   if( selected )
   {
      Database::instance().addToRecipe( recipeObs, selected );
      mashEditor->setMash(recipeObs->mash());
      mashButton->setMash(recipeObs->mash());
   }
}

void MainWindow::updateRecipeEquipment()
{
  droppedRecipeEquipment(equipmentListModel->at(equipmentComboBox->currentIndex()));
}

void MainWindow::droppedRecipeEquipment(Equipment *kit)
{
   if( recipeObs == 0 )
      return;

   // equip may be null.
   if( kit == 0 )
      return;

   // Notice that we are using a copy from the database.
   Database::instance().addToRecipe(recipeObs,kit);
   equipmentButton->setEquipment(kit);

   // Keep the mash tun weight and specific heat up to date.
   Mash* m = recipeObs->mash();
   if( m )
   {
      m->setTunWeight_kg( kit->tunWeight_kg() );
      m->setTunSpecificHeat_calGC( kit->tunSpecificHeat_calGC() );
   }

   if( QMessageBox::question(this,
                             tr("Equipment request"),
                             tr("Would you like to set the batch size, boil size and time to that requested by the equipment?"),
                             QMessageBox::Yes,
                             QMessageBox::No)
        == QMessageBox::Yes
     )
   {
      recipeObs->setBatchSize_l( kit->batchSize_l() );
      recipeObs->setBoilSize_l( kit->boilSize_l() );
      recipeObs->setBoilTime_min( kit->boilTime_min() );
      mashEditor->setEquipment(kit);
   }
}

void MainWindow::droppedRecipeStyle(Style* style)
{
   if ( ! recipeObs )
      return;
   Database::instance().addToRecipe( recipeObs, style);
   styleButton->setStyle( style );
}

// Well, aint this a kick in the pants. Apparently I can't template a slot
void MainWindow::droppedRecipeFermentable(QList<Fermentable*>ferms)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != fermentableTab )
      tabWidget_ingredients->setCurrentWidget(fermentableTab);
   Database::instance().addToRecipe(recipeObs, ferms);
}

void MainWindow::droppedRecipeHop(QList<Hop*>hops)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != hopsTab )
      tabWidget_ingredients->setCurrentWidget(hopsTab);
   Database::instance().addToRecipe(recipeObs, hops);
}

void MainWindow::droppedRecipeMisc(QList<Misc*>miscs)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != miscTab )
      tabWidget_ingredients->setCurrentWidget(miscTab);
   Database::instance().addToRecipe(recipeObs, miscs);
}

void MainWindow::droppedRecipeYeast(QList<Yeast*>yeasts)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != yeastTab )
      tabWidget_ingredients->setCurrentWidget(yeastTab);
   Database::instance().addToRecipe(recipeObs, yeasts);
}

void MainWindow::updateRecipeBatchSize()
{
   if( recipeObs == 0 )
      return;

   recipeObs->setBatchSize_l( lineEdit_batchSize->toSI() );
}

void MainWindow::updateRecipeBoilSize()
{
   if( recipeObs == 0 )
      return;

   recipeObs->setBoilSize_l( lineEdit_boilSize->toSI() );
}

void MainWindow::updateRecipeBoilTime()
{
   double boilTime = 0.0;
   Equipment* kit;

   if( recipeObs == 0 )
      return;

   kit = recipeObs->equipment();
   boilTime = Brewtarget::qStringToSI( lineEdit_boilTime->text(),Units::minutes );

   // Here, we rely on a signal/slot connection to propagate the equipment
   // changes to recipeObs->boilTime_min and maybe recipeObs->boilSize_l
   // NOTE: This works because kit is the recipe's equipment, not the generic
   // equipment in the recipe drop down.
   if( kit )
      kit->setBoilTime_min(boilTime);
   else
      recipeObs->setBoilTime_min(boilTime);
}

void MainWindow::updateRecipeEfficiency()
{
   if( recipeObs == 0 )
      return;

   recipeObs->setEfficiency_pct( lineEdit_efficiency->toSI() );
}

void MainWindow::addFermentableToRecipe(Fermentable* ferm)
{
   recipeObs->addFermentable(ferm);
   fermTableModel->addFermentable(ferm);
}

void MainWindow::addHopToRecipe(Hop *hop)
{
   recipeObs->addHop(hop);
   hopTableModel->addHop(hop);
}

void MainWindow::addMiscToRecipe(Misc* misc)
{
   recipeObs->addMisc(misc);
   miscTableModel->addMisc(misc);
}

void MainWindow::addYeastToRecipe(Yeast* yeast)
{
   recipeObs->addYeast(yeast);
   yeastTableModel->addYeast(yeast);
}

void MainWindow::exportRecipe()
{
   QFile* outFile;
   QDomDocument doc;

   if( recipeObs == 0 )
      return;

   outFile = openForWrite();
   if ( ! outFile )
      return;

   QTextStream out(outFile);

   QString xmlHead = QString("version=\"1.0\" encoding=\"ISO-8859-1\"");

   // Create the headers to make other BeerXML parsers happy
   QDomProcessingInstruction inst = doc.createProcessingInstruction("xml", xmlHead);
   QDomComment beerxml = doc.createComment("BeerXML generated by brewtarget");

   doc.appendChild(inst);
   doc.appendChild(beerxml);

   QDomElement recipes = doc.createElement("RECIPES"); // The root element.
   doc.appendChild(recipes);
   Database::instance().toXml( recipeObs, doc, recipes );

   // QString::toLatin1 returns an ISO 8859-1 stream.
   out << doc.toString().toLatin1();

   outFile->close();
   delete outFile;
}

Recipe* MainWindow::currentRecipe()
{
   return recipeObs;
}

Fermentable* MainWindow::selectedFermentable()
{
   QModelIndexList selected = fermentableTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return 0;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return 0;
   }

   modelIndex = fermTableProxy->mapToSource(viewIndex);
   Fermentable* ferm = fermTableModel->getFermentable(modelIndex.row());

   return ferm;
}

Hop* MainWindow::selectedHop()
{
   QModelIndexList selected = hopTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return 0;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return 0;
   }

   modelIndex = hopTableProxy->mapToSource(viewIndex);

   Hop* h = hopTableModel->getHop(modelIndex.row());

   return h;
}

Misc* MainWindow::selectedMisc()
{
   QModelIndexList selected = miscTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return 0;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return 0;
   }

   modelIndex = miscTableProxy->mapToSource(viewIndex);

   Misc* m = miscTableModel->getMisc(modelIndex.row());

   return m;
}

Yeast* MainWindow::selectedYeast()
{
   QModelIndexList selected = yeastTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return 0;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return 0;
   }

   modelIndex = yeastTableProxy->mapToSource(viewIndex);

   Yeast* y = yeastTableModel->getYeast(modelIndex.row());

   return y;
}


void MainWindow::removeSelectedFermentable()
{
    QModelIndexList selected = fermentableTable->selectionModel()->selectedIndexes();
    QModelIndex viewIndex, modelIndex;
    QList<Fermentable *> itemsToRemove;
    int size, i;

    size = selected.size();

    if( size == 0 )
       return;

    for(int i = 0; i < size; i++)
    {
        viewIndex = selected.at(i);
        modelIndex = fermTableProxy->mapToSource(viewIndex);

        itemsToRemove.append(fermTableModel->getFermentable(modelIndex.row()));
    }

    for(i = 0; i < itemsToRemove.size(); i++)
    {
        fermTableModel->removeFermentable(itemsToRemove.at(i));
        recipeObs->remove(itemsToRemove.at(i));
    }
}


void MainWindow::editSelectedFermentable()
{
   Fermentable* f = selectedFermentable();
   if( f == 0 )
      return;

   fermEditor->setFermentable(f);
   fermEditor->show();
}

void MainWindow::editSelectedMisc()
{
   Misc* m = selectedMisc();
   if( m == 0 )
      return;

   miscEditor->setMisc(m);
   miscEditor->show();
}

void MainWindow::editSelectedHop()
{
   Hop* h = selectedHop();
   if( h == 0 )
      return;

   hopEditor->setHop(h);
   hopEditor->show();
}

void MainWindow::editSelectedYeast()
{
   Yeast* y = selectedYeast();
   if( y == 0 )
      return;

   yeastEditor->setYeast(y);
   yeastEditor->show();
}

void MainWindow::removeSelectedHop()
{
    QModelIndexList selected = hopTable->selectionModel()->selectedIndexes();
    QModelIndex modelIndex, viewIndex;
    QList<Hop *> itemsToRemove;
    int size, i;

    size = selected.size();

    if( size == 0 )
       return;

    for(int i = 0; i < size; i++)
    {
        viewIndex = selected.at(i);
        modelIndex = hopTableProxy->mapToSource(viewIndex);

        itemsToRemove.append(hopTableModel->getHop(modelIndex.row()));
    }

    for(i = 0; i < itemsToRemove.size(); i++)
    {
        hopTableModel->removeHop(itemsToRemove.at(i));
        recipeObs->remove(itemsToRemove.at(i));
    }

}


void MainWindow::removeSelectedMisc()
{
    QModelIndexList selected = miscTable->selectionModel()->selectedIndexes();
    QModelIndex modelIndex, viewIndex;
    QList<Misc *> itemsToRemove;
    int size, i;

    size = selected.size();

    if( size == 0 )
       return;

    for(int i = 0; i < size; i++)
    {
        viewIndex = selected.at(i);
        modelIndex = miscTableProxy->mapToSource(viewIndex);

        itemsToRemove.append(miscTableModel->getMisc(modelIndex.row()));
    }

    for(i = 0; i < itemsToRemove.size(); i++)
    {
       miscTableModel->removeMisc(itemsToRemove.at(i));
       recipeObs->remove(itemsToRemove.at(i));
    }
}

void MainWindow::removeSelectedYeast()
{
    QModelIndexList selected = yeastTable->selectionModel()->selectedIndexes();
    QModelIndex modelIndex, viewIndex;
    QList<Yeast *> itemsToRemove;
    int size, i;

    size = selected.size();

    if( size == 0 )
       return;

    for(int i = 0; i < size; i++)
    {
        viewIndex = selected.at(i);
        modelIndex = yeastTableProxy->mapToSource(viewIndex);

        itemsToRemove.append(yeastTableModel->getYeast(modelIndex.row()));
    }

    for(i = 0; i < itemsToRemove.size(); i++)
    {
       yeastTableModel->removeYeast(itemsToRemove.at(i));
       recipeObs->remove(itemsToRemove.at(i));
    }
}

void MainWindow::newRecipe()
{
   QString name = QInputDialog::getText(this, tr("Recipe name"),
                                          tr("Recipe name:"));
   QVariant defEquipKey = Brewtarget::option("defaultEquipmentKey", -1);
   QObject* selection = sender();

   if( name.isEmpty() )
      return;

   Recipe* newRec = Database::instance().newRecipe();

   // bad things happened -- let somebody know
   if ( ! newRec ) {
      QMessageBox::warning(this,tr("Error copying recipe"),
                           tr("An error was returned while creating %1").arg(name));
      return;
   }
   // Set the following stuff so everything appears nice
   // and the calculations don't divide by zero... things like that.
   newRec->setName(name);
   newRec->setBatchSize_l(18.93); // 5 gallons
   newRec->setBoilSize_l(23.47);  // 6.2 gallons
   newRec->setEfficiency_pct(70.0);

   if ( defEquipKey != -1 )
   {
      Equipment* e = Database::instance().equipment(defEquipKey.toInt());
      if ( e )
      {
         Database::instance().addToRecipe(newRec, e);
         newRec->setBatchSize_l( e->batchSize_l() );
         newRec->setBoilSize_l( e->boilSize_l() );
         newRec->setBoilTime_min( e->boilTime_min() );
      }
   }

   // a new recipe will be put in a folder if you right click on a recipe or
   // folder. Otherwise, it goes into the main window?
   if ( selection ) {
      BtTreeView* sent = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
      if ( sent )
      {
         QModelIndexList indexes = sent->selectionModel()->selectedRows();
         // This is a little weird. There is an edge case where nothing is
         // selected and you click the big blue + button.
         if ( indexes.size() > 0 )
         {
            if ( sent->type(indexes.at(0)) == BtTreeItem::RECIPE )
            {
               Recipe* foo = sent->recipe(indexes.at(0));

               if ( foo && ! foo->folder().isEmpty())
                  newRec->setFolder( foo->folder() );
            }
            else if ( sent->type(indexes.at(0)) == BtTreeItem::FOLDER )
            {
               BtFolder* foo = sent->folder(indexes.at(0));
               if ( foo )
                  newRec->setFolder( foo->fullPath() );
            }
         }
      }
   }
   setTreeSelection(treeView_recipe->findElement(newRec));
   setRecipe(newRec);
}

void MainWindow::newFolder()
{
   QString dPath;
   QModelIndexList indexes;
   QModelIndex starter;
   // get the currently active tree
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   if (! active )
      return;

   indexes = active->selectionModel()->selectedRows();
   starter = indexes.at(0);

   // Where to start from
   dPath = active->folderName(starter);

   QString name = QInputDialog::getText(this, tr("Folder name"),
                                          tr("Folder name:"),
                                           QLineEdit::Normal, dPath);
   // User clicks cancel
   if (name.isEmpty())
      return;
   // Do some input validation here.

   // Nice little builtin to collapse leading and following white space
   name = name.simplified();
   if ( name.isEmpty() )
   {
      QMessageBox::critical( this, tr("Bad Name"),
                             tr("A folder name must have at least one non-whitespace character in it"));
      return;
   }

   if ( name.split("/", QString::SkipEmptyParts).isEmpty() )
   {
      QMessageBox::critical( this, tr("Bad Name"), tr("A folder name must have at least one non-/ character in it"));
      return;
   }
   active->addFolder(name);
}

void MainWindow::renameFolder()
{
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   BtFolder* victim;
   QModelIndexList indexes;
   QModelIndex starter;

   // If the sender cannot be morphed into a BtTreeView object
   if ( active == 0 )
      return;

   // I don't think I can figure out what the behavior will be if you select
   // many items
   indexes = active->selectionModel()->selectedRows();
   starter = indexes.at(0);

   // The item to be renamed
   // Don't rename anything other than a folder
   if ( active->type(starter) != BtTreeItem::FOLDER )
      return;

   victim = active->folder(starter);
   QString newName = QInputDialog::getText(this, tr("Folder name"), tr("Folder name:"),
                                           QLineEdit::Normal, victim->name());

   // User clicks cancel
   if (newName.isEmpty())
      return;
   // Do some input validation here.

   // Nice little builtin to collapse leading and following white space
   newName = newName.simplified();
   if ( newName.isEmpty() )
   {
      QMessageBox::critical( this, tr("Bad Name"),
                             tr("A folder name must have at least one non-whitespace character in it"));
      return;
   }

   if ( newName.split("/", QString::SkipEmptyParts).isEmpty() )
   {
      QMessageBox::critical( this, tr("Bad Name"), tr("A folder name must have at least one non-/ character in it"));
      return;
   }
   newName = victim->path() % "/" % newName;

   // Delgate this work to the tree.
   active->renameFolder(victim,newName);
}

void MainWindow::setTreeSelection(QModelIndex item)
{
   BtTreeView *active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   if (! item.isValid())
      return;

   if ( active == 0 )
      active = qobject_cast<BtTreeView*>(treeView_recipe);

   // Couldn't cast the active item to a BtTreeView
   if ( active == 0 )
      return;

   QModelIndex parent = active->parent(item);

   active->setCurrentIndex(item);
   if ( active->type(parent) == BtTreeItem::FOLDER && !
         active->isExpanded(parent) )
      active->setExpanded(parent,true);
   active->scrollTo(item,QAbstractItemView::PositionAtCenter);

}
// reduces the inventory by the selected recipes
void MainWindow::reduceInventory(){

   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
      QModelIndex bIndex;

      foreach(QModelIndex selected, indexes)
      {
         Recipe*   rec   = treeView_recipe->recipe(selected);
         if( rec == 0 ){
            //try the parent recipe
            rec = treeView_recipe->recipe(treeView_recipe->parent(selected));
            if( rec == 0 ){
               continue;
            }
         }

         // Make sure everything is properly set and selected
         if( rec != recipeObs )
            setRecipe(rec);

      int i = 0;
      //reduce fermentables
      QList<Fermentable*> flist = rec->fermentables();
      if(flist.size() > 0){
         for( i = 0; static_cast<int>(i) < flist.size(); ++i )
         {
            double newVal=flist[i]->inventory() - flist[i]->amount_kg();
            newVal = (newVal < 0) ? 0 : newVal;
            flist[i]->setInventoryAmount(newVal);
         }
      }

      //reduce misc
      QList<Misc*> mlist = rec->miscs();
      if(mlist.size() > 0){
         for( i = 0; static_cast<int>(i) < mlist.size(); ++i )
         {
            double newVal=mlist[i]->inventory() - mlist[i]->amount();
            newVal = (newVal < 0) ? 0 : newVal;
            mlist[i]->setInventoryAmount(newVal);
         }
      }
      //reduce hops
      QList<Hop*> hlist = rec->hops();
      if(hlist.size() > 0){
         for( i = 0; static_cast<int>(i) < hlist.size(); ++i )
         {
            double newVal = hlist[i]->inventory() - hlist[i]->amount_kg();
            newVal = (newVal < 0) ? 0 : newVal;
            hlist[i]->setInventoryAmount(newVal);
         }
      }
      //reduce yeast
      QList<Yeast*> ylist = rec->yeasts();
      if(ylist.size() > 0){
         for( i = 0; static_cast<int>(i) < ylist.size(); ++i )
         {
            //Yeast inventory is done by quanta not amount
            int newVal = ylist[i]->inventory() - 1;
            newVal = (newVal < 0) ? 0 : newVal;
            ylist[i]->setInventoryQuanta(newVal);
         }
      }
   }

}

// Need to make sure the recipe tree is active, I think
void MainWindow::newBrewNote()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   QModelIndex bIndex;

   foreach(QModelIndex selected, indexes)
   {
      Recipe*   rec   = treeView_recipe->recipe(selected);
      QModelIndex newItem;

      if( rec == 0 )
         continue;

      // Make sure everything is properly set and selected
      if( rec != recipeObs )
         setRecipe(rec);

      BrewNote* bNote = Database::instance().newBrewNote(rec);
      bNote->populateNote(rec);
      bNote->setBrewDate();

      setBrewNote(bNote);

      bIndex = treeView_recipe->findElement(bNote);
      if ( bIndex.isValid() )
         setTreeSelection(bIndex);
   }
}

void MainWindow::reBrewNote()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   foreach(QModelIndex selected, indexes)
   {
      BrewNote* old   = treeView_recipe->brewNote(selected);
      Recipe* rec     = treeView_recipe->recipe(treeView_recipe->parent(selected));

      if (! old || ! rec)
         return;

      BrewNote* bNote = Database::instance().newBrewNote(old);
      bNote->setBrewDate();

      if (rec != recipeObs)
         setRecipe(rec);

      setBrewNote(bNote);

      setTreeSelection(treeView_recipe->findElement(bNote));
   }
}

void MainWindow::brewItHelper()
{
   newBrewNote();
   reduceInventory();
}

void MainWindow::brewAgainHelper()
{
   reBrewNote();
   reduceInventory();
}

void MainWindow::backup()
{
   QString dir = QFileDialog::getExistingDirectory(this, tr("Backup Database"));

   bool success = Database::backupToDir(dir);

   if( ! success )
      QMessageBox::warning( this, tr("Oops!"), tr("Could not copy the files for some reason."));
}

void MainWindow::restoreFromBackup()
{
   if( QMessageBox::question(
          this,
          tr("A Warning"),
          tr("This will obliterate your current set of recipes and ingredients. Do you want to continue?"),
          QMessageBox::Yes, QMessageBox::No
       )
       == QMessageBox::No
   )
   {
      return;
   }

   QString restoreDbFile = QFileDialog::getOpenFileName(this, tr("Choose File"), "", tr("SQLite (*.sqlite)"));
   bool success = Database::restoreFromFile(restoreDbFile);

   if( ! success )
      QMessageBox::warning( this, tr("Oops!"), tr("For some reason, the operation failed.") );
   else
      QMessageBox::information(this, tr("Restart"), tr("Please restart Brewtarget."));
   //TODO: do this without requiring restarting :)
}

// Imports all the recipes from a file into the database.
void MainWindow::importFiles()
{
   if ( ! fileOpener->exec() )
      return;

   foreach( QString filename, fileOpener->selectedFiles() )
   {
      if ( ! Database::instance().importFromXML(filename) )
         importMsg();
   }

   showChanges();
}

bool MainWindow::verifyImport(QString tag, QString name)
{
   return QMessageBox::question(this, tr("Import %1?").arg(tag), tr("Import %1?").arg(name),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::addMashStep()
{
   Mash* mash;
   if( recipeObs != 0 && recipeObs->mash() != 0 )
   {
      mash = recipeObs->mash();
   }
   else
   {
      QMessageBox::information(this, tr("No mash"), tr("Trying to add a mash step without a mash. Please create a mash first.") );
      return;
   }

   MashStep* step = Database::instance().newMashStep(mash);
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
}

void MainWindow::removeSelectedMashStep()
{
   Mash* mash = recipeObs == 0 ? 0 : recipeObs->mash();
   if( mash == 0 )
      return;

   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
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

   MashStep* step = mashStepTableModel->getMashStep(row);
   Database::instance().removeFrom(mash,step);
}

void MainWindow::moveSelectedMashStepUp()
{
   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
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

   // Make sure we can actually move it up.
   if( row < 1 )
      return;

   MashStep* m1 = mashStepTableModel->getMashStep(row);
   MashStep* m2 = mashStepTableModel->getMashStep(row-1);
   Database::instance().swapMashStepOrder(m1,m2);
}

void MainWindow::moveSelectedMashStepDown()
{
   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
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

   // Make sure it's not the last row so we can move it down.
   if( row >= mashStepTableModel->rowCount() - 1 )
      return;

   MashStep* m1 = mashStepTableModel->getMashStep(row);
   MashStep* m2 = mashStepTableModel->getMashStep(row+1);
   Database::instance().swapMashStepOrder(m1,m2);
}

void MainWindow::editSelectedMashStep()
{
   if( !recipeObs || !recipeObs->mash() )
      return;

   QModelIndexList selected = mashStepTableWidget->selectionModel()->selectedIndexes();
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

   MashStep* step = mashStepTableModel->getMashStep(row);
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
}

void MainWindow::removeMash()
{
   Mash *m = mashButton->mash();

   if( m == 0)
      return;
   //due to way this is designed, we can't have a NULL mash, so
   //we need to remove all the mash steps and then remove the mash
   //from the database.
   //remove from db

   m->removeAllMashSteps();
   Database::instance().remove(m);

   Mash* defaultMash = Database::instance().newMash(recipeObs);
   mashStepTableModel->setMash(defaultMash);

   //remove from combobox handled automatically by qt
   mashButton->setMash(defaultMash);

}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
   Brewtarget::saveSystemOptions();
   Brewtarget::setOption("geometry", saveGeometry());
   Brewtarget::setOption("windowState", saveState());
   if ( recipeObs )
      Brewtarget::setOption("recipeKey", recipeObs->key());

   //UI save state
   Brewtarget::setOption("MainWindow/splitter_vertical_State", splitter_vertical->saveState());
   Brewtarget::setOption("MainWindow/splitter_horizontal_State", splitter_horizontal->saveState());
   Brewtarget::setOption("MainWindow/treeView_recipe_headerState", treeView_recipe->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_style_headerState", treeView_style->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_equip_headerState", treeView_equip->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_ferm_headerState", treeView_ferm->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_hops_headerState", treeView_hops->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_misc_headerState", treeView_misc->header()->saveState());
   Brewtarget::setOption("MainWindow/treeView_yeast_headerState", treeView_yeast->header()->saveState());
   Brewtarget::setOption("MainWindow/mashStepTableWidget_headerState", mashStepTableWidget->horizontalHeader()->saveState());

   // After unloading the database, can't make any more queries to it, so first
   // make the main window disappear so that redraw events won't inadvertently
   // cause any more queries.
   setVisible(false);

}

void MainWindow::copyRecipe()
{
   QString name = QInputDialog::getText( this, tr("Copy Recipe"), tr("Enter a unique name for the copy.") );

   if( name.isEmpty() )
      return;

   Recipe* newRec = Database::instance().newRecipe(recipeObs); // Create a deep copy.
   if ( newRec )
      newRec->setName(name);
}

void MainWindow::setMashToCurrentlySelected()
{
   if( recipeObs == 0 )
      return;

   Mash* selected = mashListModel->at(mashComboBox->currentIndex());
   if( selected )
   {
      Database::instance().newMash(selected);
      mashButton->setMash(selected);
   }
}

void MainWindow::saveMash()
{
   if( recipeObs == 0 || recipeObs->mash() == 0 )
      return;

   Mash* mash = recipeObs->mash();
   // Ensure the mash has a name.
   if( mash->name() == "" )
   {
      QMessageBox::information( this, tr("Oops!"), tr("Please give your mash a name before saving.") );
      return;
   }

   // NOTE: should NOT displace recipeObs' current mash.
   Mash* newMash = Database::instance().newMash(mash, false);
   // NOTE: need to set the display to true for the saved, named mash to work
   newMash->setDisplay(true);
   mashButton->setMash(newMash);

}

void MainWindow::openManual()
{
   // TODO: open language-dependent manual when we have more than the English version
   QDesktopServices::openUrl(QUrl::fromLocalFile(Brewtarget::getDataDir().filePath("manual-en.pdf")));
}

void MainWindow::print(std::function<void(QPrinter* printer)> functor)
{
   if (!functor)
   {
      Brewtarget::logE("The print function is called with an empty functor");
   }

   QPrintDialog dialogue(printer, this);
   dialogue.setWindowTitle(tr("Print Document"));
   if (dialogue.exec() == QDialog::Accepted)
   {
      functor(printer);
   }
}

void MainWindow::exportHTML(std::function<void(QFile* file)> functor)
{
   if (!functor)
   {
      Brewtarget::logE(
            "The export HTML function is called with an empty functor");
   }

   std::unique_ptr<QFile> file{
         openForWrite(tr("HTML files (*.html)"), QString("html"))};
   if (file)
   {
      functor(file.get());
   }
}

// We build the menus at start up time.  This just needs to exec the proper
// menu.
void MainWindow::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   BtTreeView* active;
   QModelIndex selected;
   QMenu* tempMenu;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == 0 )
      return;

   active = qobject_cast<BtTreeView*>(calledBy);

   // If the sender cannot be morphed into a BtTreeView object
   if ( active == 0 )
      return;

   selected = active->indexAt(point);
   if (! selected.isValid())
      return;

   tempMenu = active->contextMenu(selected);

   if (tempMenu)
      tempMenu->exec(active->mapToGlobal(point));
}

void MainWindow::setupContextMenu()
{

   treeView_recipe->setupContextMenu(this,this);
   treeView_equip->setupContextMenu(this,equipEditor);

   treeView_ferm->setupContextMenu(this,fermDialog);
   treeView_hops->setupContextMenu(this,hopDialog);
   treeView_misc->setupContextMenu(this,miscDialog);
   treeView_style->setupContextMenu(this,singleStyleEditor);
   treeView_yeast->setupContextMenu(this,yeastDialog);

   // TreeView for clicks, both double and right
   connect( treeView_recipe, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_recipe, SIGNAL(customContextMenuRequested( const QPoint& )), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_equip, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_equip, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_ferm, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_ferm, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_hops, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_hops, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_misc, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_misc, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_yeast, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_yeast, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

   connect( treeView_style, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(treeActivated(const QModelIndex &)));
   connect( treeView_style, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(contextMenu(const QPoint &)));

}

void MainWindow::copySelected()
{
   QModelIndexList selected;
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   active->copySelected(active->selectionModel()->selectedRows());
}

QFile* MainWindow::openForWrite( QString filterStr, QString defaultSuff)
{
   QFile* outFile = new QFile();

   fileSaver->setNameFilter( filterStr );
   fileSaver->setDefaultSuffix( defaultSuff );

   if( fileSaver->exec() )
   {
      QString filename = fileSaver->selectedFiles()[0];
      outFile->setFileName(filename);

      if( ! outFile->open(QIODevice::WriteOnly | QIODevice::Truncate) )
      {
         Brewtarget::logW(QString("MainWindow::openForWrite Could not open %1 for writing.").arg(filename));
         outFile = 0;
      }
   }
   else
     outFile = 0;

   return outFile;
}

void MainWindow::exportSelectedHtml() {
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   QModelIndexList selected;
   QList <Recipe*> targets;
   QFile* outFile;

   if ( active == 0 )
      return;

   // this only works for recipes
   if ( active != treeView_recipe ) {
       return;
   }

   // get the targeted file
   outFile = openForWrite(tr("HTML files (*.html)"), QString("html"));
   if ( !outFile )
      return;

   // Get the selected recipes and throw them into a list
   selected = active->selectionModel()->selectedRows();
   if( selected.count() == 0 )
      return;

   foreach( QModelIndex ndx, selected)
      targets.append( treeView_recipe->recipe(ndx) );

   // and write it all
   QTextStream out(outFile);
   out << recipeFormatter->getHTMLFormat(targets);
   outFile->close();
}

void MainWindow::exportSelected()
{
   BtTreeView* active = qobject_cast<BtTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   QModelIndexList selected;
   QList<QModelIndex>::const_iterator at,end;
   QDomDocument doc;
   QFile* outFile;
   QDomElement root,dbase,recipe;
   bool didRecipe = false;


   if ( active == 0 )
      return;

   selected = active->selectionModel()->selectedRows();
   if( selected.count() == 0 )
      return;

   outFile = openForWrite();
   if ( !outFile )
      return;

   QTextStream out(outFile);

   QString xmlHead = QString("version=\"1.0\" encoding=\"%1\"").arg(QTextCodec::codecForLocale()->name().data());

   // Create the headers to make other BeerXML parsers happy
   QDomProcessingInstruction inst = doc.createProcessingInstruction("xml", xmlHead);
   QDomComment beerxml = doc.createComment("BeerXML generated by brewtarget");

   doc.appendChild(inst);
   doc.appendChild(beerxml);

   // We need to handle the recipes separate from the normal database
   // elements.  All recipes live under the RECIPES tag, whereas the
   // equipment, hops, etc. go under DATABASE.
   dbase = doc.createElement("DATABASE");
   recipe = doc.createElement("RECIPES");

   for(at = selected.begin(),end = selected.end(); at < end; ++at)
   {
      QModelIndex selection = *at;
      int type = active->type(selection);

      switch(type)
      {
         case BtTreeItem::RECIPE:
            Database::instance().toXml( treeView_recipe->recipe(selection), doc, recipe);
            didRecipe = true;
            break;
         case BtTreeItem::EQUIPMENT:
            Database::instance().toXml( treeView_equip->equipment(selection), doc, dbase);
            break;
         case BtTreeItem::FERMENTABLE:
            Database::instance().toXml( treeView_ferm->fermentable(selection), doc, dbase);
            break;
         case BtTreeItem::HOP:
            Database::instance().toXml( treeView_hops->hop(selection), doc, dbase);
            break;
         case BtTreeItem::MISC:
            Database::instance().toXml( treeView_misc->misc(selection), doc, dbase);
            break;
         case BtTreeItem::STYLE:
            Database::instance().toXml( treeView_style->style(selection), doc, dbase);
            break;
         case BtTreeItem::YEAST:
            Database::instance().toXml( treeView_yeast->yeast(selection), doc, dbase);
            break;
      }
   }

   if ( didRecipe )
      doc.appendChild(recipe);
   else
      doc.appendChild(dbase);

   out << doc.toString();

   outFile->close();
   delete outFile;
}

void MainWindow::updateDatabase()
{
   QString otherDb;
   QMessageBox::StandardButton but;

   // Tell user what's about to happen.
   but = QMessageBox::question( this,
                          tr("Database Update"),
                          tr("You are about to update the current database with another one. "
                             "This may make changes to (but will not delete) some of your ingredients. "
                             "It will not modify any of your recipes. "
                             "Continue?"),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::No );
   if( but == QMessageBox::No )
      return;

   // Select the db to merge with.
   otherDb = QFileDialog::getOpenFileName( this,
                                           tr("Select Database File"),
                                           Brewtarget::getUserDataDir().canonicalPath(),
                                           tr("Brewtarget Database (*.sqlite)") );

   // Merge.
   Database::instance().updateDatabase( otherDb );
}

void MainWindow::finishCheckingVersion()
{
   QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
   if( reply == 0 )
      return;

   QString remoteVersion(reply->readAll());

   // If there is an error, just return.
   if( reply->error() != QNetworkReply::NoError )
      return;

   // If the remote version is newer...
   if( !remoteVersion.startsWith(VERSIONSTRING) )
   {
      // ...and the user wants to download the new version...
      if( QMessageBox::information(this,
                                   QObject::tr("New Version"),
                                   QObject::tr("Version %1 is now available. Download it?").arg(remoteVersion),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::Yes) == QMessageBox::Yes )
      {
         // ...take them to the website.
         QDesktopServices::openUrl(QUrl("http://www.brewtarget.org/download.html"));
      }
      else // ... and the user does NOT want to download the new version...
      {
         // ... and they want us to stop bothering them...
         if( QMessageBox::question(this,
                                   QObject::tr("New Version"),
                                   QObject::tr("Stop bothering you about new versions?"),
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::Yes) == QMessageBox::Yes)
         {
            // ... tell brewtarget to stop bothering the user about the new version.
            Brewtarget::checkVersion = false;
         }
      }
   }
   else // The current version is newest so...
   {
      // ...tell brewtarget to bother users about future new versions.
      // This means that when a user downloads the new version, this
      // variable will always get reset to true.
      Brewtarget::checkVersion = true;
   }
}

void MainWindow::redisplayLabel(Unit::unitDisplay oldUnit, Unit::unitScale oldScale)
{
   // There is a lot of magic going on in the showChanges(). I can either
   // duplicate that magic or I can just call showChanges().
   showChanges();
}

void MainWindow::showPitchDialog()
{
   // First, copy the current recipe og and volume.
   if( recipeObs )
   {
      pitchDialog->setWortVolume_l( recipeObs->finalVolume_l() );
      pitchDialog->setWortDensity( recipeObs->og() );
      pitchDialog->calculate();
   }

   pitchDialog->show();
}

void MainWindow::showEquipmentEditor()
{
   if ( ! recipeObs->equipment() )
   {
      QMessageBox::warning( this, tr("No equipment"), tr("You must select or define an equipment profile first."));
   }
   else
   {
      singleEquipEditor->show();
   }
}

void MainWindow::showStyleEditor()
{
   if ( ! recipeObs->style() )
   {
      QMessageBox::warning( this, tr("No style"), tr("You must select a style first."));
   }
   else
   {
      singleStyleEditor->show();
   }
}

void MainWindow::convertedMsg()
{

   QMessageBox msgBox;
   msgBox.setText( tr("The database has been converted/upgraded."));
   msgBox.setInformativeText( tr("The original XML files can be found in ") + Brewtarget::getUserDataDir().canonicalPath() + "obsolete");
   msgBox.exec();

}

void MainWindow::importMsg()
{
   QMessageBox msgBox;
   msgBox.setText( tr("The import contained invalid beerXML. It has been imported, but please make certain it makes sense."));
   msgBox.exec();
}

void MainWindow::changeBrewDate()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   QDateTime newDate;

   foreach(QModelIndex selected, indexes)
   {
      BrewNote* target = treeView_recipe->brewNote(selected);

      // No idea how this could happen, but I've seen stranger things
      if ( ! target )
         continue;

      // Pop the calendar, get the date.
      if ( btDatePopup->exec() == QDialog::Accepted )
      {
         newDate = btDatePopup->selectedDate();
         target->setBrewDate(newDate);

         // If this note is open in a tab
         BrewNoteWidget* ni = findBrewNoteWidget(target);
         if ( ni )
         {
            tabWidget_recipeView->setTabText(tabWidget_recipeView->indexOf(ni), target->brewDate_short());
            return;
         }
      }
   }
}

void MainWindow::fixBrewNote()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   QDateTime newDate;

   foreach(QModelIndex selected, indexes)
   {
      BrewNote* target = treeView_recipe->brewNote(selected);

      // No idea how this could happen, but I've seen stranger things
      if ( ! target )
         continue;

      Recipe* noteParent = treeView_recipe->recipe( treeView_recipe->parent(selected));

      if ( ! noteParent )
         continue;

      target->recalculateEff(noteParent);
   }
}

void MainWindow::updateStatus(const QString status) {
   if( statusBar() )
      statusBar()->showMessage(status, 3000);
}

void MainWindow::closeBrewNote(BrewNote* b)
{
   Recipe* parent = Database::instance().getParentRecipe(b);

   // If this isn't the focused recipe, do nothing because there are no tabs
   // to close.
   if ( parent != recipeObs )
      return;

   BrewNoteWidget* ni = findBrewNoteWidget(b);

   if ( ni )
      tabWidget_recipeView->removeTab( tabWidget_recipeView->indexOf(ni));

   return;

}
