/*
 * MainWindow.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - A.J. Drobnich <aj.drobnich@gmail.com>
 * - Dan Cavanagh <dan@dancavanagh.com>
 * - David Grundberg <individ@acc.umu.se>
 * - Kregg K <gigatropolis@yahoo.com>
 * - Matt Young <mfsy@yahoo.com>
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
#include "BtTabWidget.h"
#include "MashStepEditor.h"
#include "MashStepTableModel.h"
#include "model/Mash.h"
#include "MashEditor.h"
#include "brewtarget.h"
#include "FermentableEditor.h"
#include "MiscEditor.h"
#include "HopEditor.h"
#include "YeastEditor.h"
#include "YeastTableModel.h"
#include "MiscTableModel.h"
#include "model/Style.h"
#include "model/Recipe.h"
#include "MainWindow.h"
#include "AboutDialog.h"
#include "database.h"
#include "YeastDialog.h"
#include "config.h"
#include "Unit.h"
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
#include "AlcoholTool.h"
#include "TimerMainDialog.h"
#include "RecipeFormatter.h"
#include "PrimingDialog.h"
#include "PrintAndPreviewDialog.h"
#include "StrikeWaterDialog.h"
#include "RefractoDialog.h"
#include "MashDesigner.h"
#include "PitchDialog.h"
#include "model/Fermentable.h"
#include "model/Yeast.h"
#include "model/BrewNote.h"
#include "model/Equipment.h"
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
#include "WaterDialog.h"
#include "WaterListModel.h"
#include "WaterEditor.h"
#include "xml/BeerXml.h"
#include "RelationalUndoableUpdate.h"
#include "UndoableAddOrRemove.h"
#include "BtHorizontalTabs.h"
#include "AncestorDialog.h"

#if defined(Q_OS_WIN)
   #include <windows.h>
#endif

#include <memory>

// This private implementation class holds all private non-virtual members of MainWindow
class MainWindow::impl {
public:

   impl() : fileOpenDirectory{QDir::homePath()} {
      return;
   }

   ~impl() = default;

   /**
    * @brief Import recipes, hops, equipment, etc from files specified by the user.  (Currently this is just BeerXML,
    *        but in future could well be other formats too.
    *
    * @param mainWindow Back pointer to MainWindow class
    */
   void importFromFiles(MainWindow * mainWindow) {
      // Since we can only be called from an instance of MainWindow, it should be impossible for it to give us null as
      // the pointer to itself!
      Q_ASSERT(mainWindow != nullptr);

      // Set up the fileOpener dialog.  In previous versions of the code, this was created once and reused every time
      // we want to open a file.  The advantage of that is that, on subsequent uses, the file dialog is going to open
      // wherever you navigated to when you last opened a file.  However, as at 2020-12-30, there is a known bug in Qt
      // (https://bugreports.qt.io/browse/QTBUG-88971) which means you cannot make a QFileDialog "forget" previous
      // files you have selected with it.  So each time you you show it, the subsequent list returned from
      // selectedFiles() is actually all files _ever_ selected with this dialog object.  (The bug report is a bit bare
      // bones, but https://forum.qt.io/topic/121235/qfiledialog-has-memory has more detail.)
      //
      // Our workaround is to use a new QFileDialog each time, and manually keep track of the current directory
      QFileDialog fileOpener{mainWindow,
                             tr("Open"),
                             this->fileOpenDirectory,
                             tr("BeerXML files (*.xml)")};
      fileOpener.setAcceptMode(QFileDialog::AcceptOpen);
      fileOpener.setFileMode(QFileDialog::ExistingFiles);
      fileOpener.setViewMode(QFileDialog::List);

      if ( ! fileOpener.exec() ) {
         // User clicked cancel, so nothing more to do
         return;
      }

      qDebug() << Q_FUNC_INFO << "Importing " << fileOpener.selectedFiles().length() << " files";
      qDebug() << Q_FUNC_INFO << "Directory " << fileOpener.directory();
      this->fileOpenDirectory = fileOpener.directory().canonicalPath();

      foreach( QString filename, fileOpener.selectedFiles() ) {
         //
         // I guess if the user were importing a lot of files in one go, it might be annoying to have a separate result
         // message for each one, but TBD whether that's much of a use case.  For now, we keep things simple.
         //
         qDebug() << Q_FUNC_INFO << "Importing " << filename;
         QString userMessage;
         QTextStream userMessageAsStream{&userMessage};
         bool succeeded = Database::instance().getBeerXml()->importFromXML(filename, userMessageAsStream);
         qDebug() << Q_FUNC_INFO << "Import " << (succeeded ? "succeeded" : "failed");
         this->importMsg(filename, succeeded, userMessage);
      }

      mainWindow->showChanges();

      return;
   }

   /**
    * \brief Show a success/failure message to the user after we attempted to import one or more BeerXML files
    */
   void importMsg(QString const & fileName, bool succeeded, QString const & userMessage) {
      // This will allow us to drop the directory path to the file, as it is often long and makes the message box a
      // "wall of text" that will put a lot of users off.
      QFileInfo fileInfo(fileName);


      QString messageBoxTitle{succeeded ? tr("Success!") : tr("ERROR")};
      QString messageBoxText;
      if (succeeded) {
         // The userMessage parameter will tell how many files were imported and/or skipped (as duplicates)
         messageBoxText = QString(
            tr("Successfully read \"%1\"\n\n%2").arg(fileInfo.fileName()).arg(userMessage)
         );
      } else {
         messageBoxText = QString(
            tr("Unable to import data from \"%1\"\n\n"
               "%2\n\n"
               "Log file may contain more details.").arg(fileInfo.fileName()).arg(userMessage)
         );
      }
      qDebug() << Q_FUNC_INFO << "Message box text : " << messageBoxText;
      QMessageBox msgBox{succeeded ? QMessageBox::Information : QMessageBox::Critical,
                         messageBoxTitle,
                         messageBoxText};
      msgBox.exec();
      return;
   }

private:
   QFileDialog* fileOpener;

   QString fileOpenDirectory;
};


MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent), pimpl{ new impl{} }
{
   qDebug() << Q_FUNC_INFO;

   undoStack = new QUndoStack(this);

   // Need to call this parent class method to get all the widgets added (I think).
   this->setupUi(this);

   // Stop things looking ridiculously tiny on high DPI displays
   this->setSizesInPixelsBasedOnDpi();

   // Horizontal tabs, please
   tabWidget_Trees->tabBar()->setStyle(new BtHorizontalTabs);

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
   recipeObs = nullptr;

   // Set up the printer
   printer = new QPrinter;
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   printer->setPageSize(QPrinter::Letter);
#else
   printer->setPageSize(QPageSize(QPageSize::Letter));
#endif
   return;
}

void MainWindow::init() {
   qDebug() << Q_FUNC_INFO;
   this->setupCSS();
   // initialize all of the dialog windows
   this->setupDialogs();
   // initialize the ranged sliders
   this->setupRanges();
   // the dialogs have to be setup before this is called
   this->setupComboBoxes();
   // do all the work to configure the tables models and their proxies
   this->setupTables();
   // Create the keyboard shortcuts
   this->setupShortCuts();
   // Once more with the context menus too
   this->setupContextMenu();
   // do all the work for checkboxes (just one right now)
   this->setUpStateChanges();

   // This sets up things that might have been 'remembered' (ie stored in the config file) from a previous run of the
   // program - eg window size, which is stored in MainWindow::closeEvent().
   // Breaks the naming convention, doesn't it?
   this->restoreSavedState();

   // Connect menu item slots to triggered() signals
   this->setupTriggers();
   // Connect pushbutton slots to clicked() signals
   this->setupClicks();
   // connect combobox slots to activate() signals
   this->setupActivate();
   // connect signal slots for the line edits
   this->setupTextEdit();
   // connect the remaining labels
   this->setupLabels();
   // set up the drag/drop parts
   this->setupDrops();

   // I do not like this connection here.
   connect( ancestorDialog, &AncestorDialog::ancestoryChanged, treeView_recipe->model(), &BtTreeModel::versionedRecipe);
   connect( optionDialog, &OptionDialog::showAllAncestors, treeView_recipe->model(), &BtTreeModel::catchAncestors);
   connect( treeView_recipe, &BtTreeView::recipeSpawn, this, &MainWindow::versionedRecipe );
   // No connections from the database yet? Oh FSM, that probably means I'm
   // doing it wrong again.
   connect( &(Database::instance()), qOverload<BrewNote*>(&Database::deletedSignal), this, &MainWindow::closeBrewNote);

   // setup the pretty tool tip. It doesn't really belong anywhere, so here it
   // is
   label_Brewtarget->setToolTip( recipeFormatter->getLabelToolTip());

   qDebug() << Q_FUNC_INFO << "MainWindow initialisation complete";
   return;
}


// See https://herbsutter.com/gotw/_100/ for why we need to explicitly define the destructor here (and not in the header file)
MainWindow::~MainWindow() = default;


void MainWindow::setSizesInPixelsBasedOnDpi()
{
   //
   // Default icon sizes are fine for low DPI monitors, but need changing on high-DPI systems.
   //
   // Fortunately, the icons are already SVGs, so we don't need to do anything more complicated than tell Qt what size
   // in pixels to render them.
   //
   // For the moment, we assume we don't need to change the icon size after set-up.  (In theory, it would be nice
   // to detect, on a multi-monitor system, whether we have moved from a high DPI to a low DPI screen or vice versa.
   // See https://doc.qt.io/qt-5/qdesktopwidget.html#screen-geometry for more on this.
   // But, for now, TBD how important a use case that is.  Perhaps a future enhancement...)
   //
   // Low DPI monitors are 72 or 96 DPI typically.  High DPI monitors can be 168 DPI (as reported by logicalDpiX(),
   // logicalDpiX()).  Default toolbar icon size of 22×22 looks fine on low DPI monitor.  So it seems 1/4-inch is a
   // good width and height for these icons.  Therefore divide DPI by 4 to get icon size.
   //
   auto dpiX = this->logicalDpiX();
   auto dpiY = this->logicalDpiY();
   qDebug() << QString("Logical DPI: %1,%2.  Physical DPI: %3,%4")
      .arg(dpiX)
      .arg(dpiY)
      .arg(this->physicalDpiX())
      .arg(this->physicalDpiY());
   auto defaultToolBarIconSize = this->toolBar->iconSize();
   qDebug() << QString("Default toolbar icon size: %1,%2")
      .arg(defaultToolBarIconSize.width())
      .arg(defaultToolBarIconSize.height());
   this->toolBar->setIconSize(QSize(dpiX/4,dpiY/4));

   //
   // Historically, tab icon sizes were, by default, smaller (16×16), but it seems more logical for them to be the same
   // size as the toolbar ones.
   //
   auto defaultTabIconSize = this->tabWidget_Trees->iconSize();
   qDebug() << QString("Default tab icon size: %1,%2")
      .arg(defaultTabIconSize.width())
      .arg(defaultTabIconSize.height());
   this->tabWidget_Trees->setIconSize(QSize(dpiX/4,dpiY/4));

   //
   // Default logo size is 100×30 pixels, which is actually the wrong aspect ratio for the underlying image (currently
   // 265 × 66 - ie aspect ratio of 4.015:1).
   //
   // Setting height to be 1/3 inch seems plausible for the default size, but looks a bit wrong in practice.  Using 1/2
   // height looks better.  Then width 265/66 × height.  (Note that we actually put the fraction in double literals to
   // avoid premature rounding.)
   //
   // This is a bit more work to implement because its a PNG image in a QLabel object
   //
   qDebug() << QString("Logo default size: %1,%2").arg(this->label_Brewtarget->width()).arg(this->label_Brewtarget->height());
   this->label_Brewtarget->setScaledContents(true);
   this->label_Brewtarget->setFixedSize((265.0/66.0) * dpiX/2,  // width = 265/66 × height = 265/66 × half an inch = (265/66) × (dpiX/2)
                                        dpiY/2);                // height = half an inch = dpiY/2
   qDebug() << QString("Logo new size: %1,%2").arg(this->label_Brewtarget->width()).arg(this->label_Brewtarget->height());

   return;
}


// Setup the keyboard shortcuts
void MainWindow::setupShortCuts()
{
   actionNewRecipe->setShortcut(QKeySequence::New);
   actionCopy_Recipe->setShortcut(QKeySequence::Copy);
   actionDeleteSelected->setShortcut(QKeySequence::Delete);
   actionUndo->setShortcut(QKeySequence::Undo);
   actionRedo->setShortcut(QKeySequence::Redo);
}

void MainWindow::setUpStateChanges() 
{
   connect( checkBox_locked, &QCheckBox::stateChanged, this, &MainWindow::lockRecipe );
}

// Any manipulation of CSS for the MainWindow should be in here
void MainWindow::setupCSS()
{
   // Different palettes for some text. This is all done via style sheets now.
   QColor wPalette = tabWidget_recipeView->palette().color(QPalette::Active,QPalette::Base);

   //
   // NB: Using pixels for font sizes in Qt is bad because, given the significant variations in pixels-per-inch (aka
   // dots-per-inch / DPI) between "normal" and "high DPI" displays, a size specified in pixels will most likely be
   // dramatically wrong on some displays.  The simple solution is instead to use points (which are device independent)
   // to specify font size.
   //
   goodSS = QString( "QLineEdit:read-only { color: #008800; background: %1 }").arg(wPalette.name());
   lowSS  = QString( "QLineEdit:read-only { color: #0000D0; background: %1 }").arg(wPalette.name());
   highSS = QString( "QLineEdit:read-only { color: #D00000; background: %1 }").arg(wPalette.name());
   boldSS = QString( "QLineEdit:read-only { font: bold 10pt; color: #000000; background: %1 }").arg(wPalette.name());

   // The bold style sheet doesn't change, so set it here once.
   lineEdit_boilSg->setStyleSheet(boldSS);

   // Disabled fields should change color, but not become unreadable. Mucking
   // with the css seems the most reasonable way to do that.
   QString tabDisabled = QString("QWidget:disabled { color: #000000; background: #F0F0F0 }");
   tab_recipe->setStyleSheet(tabDisabled);
   tabWidget_ingredients->setStyleSheet(tabDisabled);

}

// Most dialogs are initialized in here. That should include any initial
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
   printAndPreviewDialog = new PrintAndPreviewDialog(this);
   ogAdjuster = new OgAdjuster(this);
   converterTool = new ConverterTool(this);
   hydrometerTool = new HydrometerTool(this);
   alcoholTool = new AlcoholTool(this);
   timerMainDialog = new TimerMainDialog(this);
   primingDialog = new PrimingDialog(this);
   strikeWaterDialog = new StrikeWaterDialog(this);
   refractoDialog = new RefractoDialog(this);
   mashDesigner = new MashDesigner(this);
   pitchDialog = new PitchDialog(this);
   btDatePopup = new BtDatePopup(this);

   waterDialog = new WaterDialog(this);
   waterEditor = new WaterEditor(this);

   ancestorDialog = new AncestorDialog(this);

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

   // definitely cheating, but I don't feel like making a whole subclass just to support this
   // or the next.
   rangeWidget_batchsize->setRange(0, recipeObs == nullptr ? 19.0 : recipeObs->batchSize_l());
   rangeWidget_batchsize->setPrecision(1);
   rangeWidget_batchsize->setTickMarks(2,5);

   rangeWidget_batchsize->setBackgroundBrush(QColor(255,255,255));
   rangeWidget_batchsize->setPreferredRangeBrush(QColor(55,138,251));
   rangeWidget_batchsize->setMarkerBrush(QBrush(Qt::NoBrush));

   rangeWidget_boilsize->setRange(0, recipeObs == nullptr? 24.0 : recipeObs->boilVolume_l());
   rangeWidget_boilsize->setPrecision(1);
   rangeWidget_boilsize->setTickMarks(2,5);

   rangeWidget_boilsize->setBackgroundBrush(QColor(255,255,255));
   rangeWidget_boilsize->setPreferredRangeBrush(QColor(55,138,251));
   rangeWidget_boilsize->setMarkerBrush(QBrush(Qt::NoBrush));

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
   styleProxyModel->setSortLocaleAware(true);
   styleProxyModel->setSourceModel(styleListModel);
   styleProxyModel->sort(0);
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
   // Double clicking the name column pops up an edit dialog for the selected item
   connect( fermentableTable, &QTableView::doubleClicked, this, [&](const QModelIndex &idx) {
       if (idx.column() == 0)
           MainWindow::editSelectedFermentable();
   });

   // Hops
   hopTableModel = new HopTableModel(hopTable);
   hopTableProxy = new HopSortFilterProxyModel(hopTable, false);
   hopTableProxy->setSourceModel(hopTableModel);
   hopTable->setItemDelegate(new HopItemDelegate(hopTable));
   hopTable->setModel(hopTableProxy);
   // Hop table show IBUs in row headers.
   hopTableModel->setShowIBUs(true);
   connect( hopTable, &QTableView::doubleClicked, this, [&](const QModelIndex &idx) {
       if (idx.column() == 0)
           MainWindow::editSelectedHop();
   });

   // Misc
   miscTableModel = new MiscTableModel(miscTable);
   miscTableProxy = new MiscSortFilterProxyModel(miscTable,false);
   miscTableProxy->setSourceModel(miscTableModel);
   miscTable->setItemDelegate(new MiscItemDelegate(miscTable));
   miscTable->setModel(miscTableProxy);
   connect( miscTable, &QTableView::doubleClicked, this, [&](const QModelIndex &idx) {
       if (idx.column() == 0)
           MainWindow::editSelectedMisc();
   });

   // Yeast
   yeastTableModel = new YeastTableModel(yeastTable);
   yeastTableProxy = new YeastSortFilterProxyModel(yeastTable,false);
   yeastTableProxy->setSourceModel(yeastTableModel);
   yeastTable->setItemDelegate(new YeastItemDelegate(yeastTable));
   yeastTable->setModel(yeastTableProxy);
   connect( yeastTable, &QTableView::doubleClicked, this, [&](const QModelIndex &idx) {
       if (idx.column() == 0)
           MainWindow::editSelectedYeast();
   });

   // Mashes
   mashStepTableModel = new MashStepTableModel(mashStepTableWidget);
   mashStepTableWidget->setItemDelegate(new MashStepItemDelegate());
   mashStepTableWidget->setModel(mashStepTableModel);
   connect( mashStepTableWidget, &QTableView::doubleClicked, this, [&](const QModelIndex &idx) {
       if (idx.column() == 0)
           MainWindow::editSelectedMashStep();
   });

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

   // If we saved a size the last time we ran, use it
   if ( Brewtarget::hasOption("geometry"))
   {
      restoreGeometry(Brewtarget::option("geometry").toByteArray());
      restoreState(Brewtarget::option("windowState").toByteArray());
   }
   else
   {
      // otherwise, guess a reasonable size at 1/4 of the screen.
      QDesktopWidget *desktop = QApplication::desktop();
      int width = desktop->width();
      int height = desktop->height();
      this->resize(width/2,height/2);

      // Or we could do the same in one line:
      // this->resize(QDesktopWidget().availableGeometry(this).size() * 0.5);
   }

   // If we saved the selected recipe name the last time we ran, select it and show it.
   int key = -1;
   if (Brewtarget::hasOption("recipeKey")) {
      key = Brewtarget::option("recipeKey").toInt();
   }
   else if ( Database::instance().numberOfRecipes() > 0 ) {
      key = 0;
   }

   if ( key > -1 ) {
      recipeObs = Database::instance().recipe( key );
      QModelIndex rIdx = treeView_recipe->findElement(recipeObs);

      setRecipe(recipeObs);
      setTreeSelection(rIdx);
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

// menu items with a SIGNAL of triggered() should go in here.
void MainWindow::setupTriggers()
{
   // Connect actions defined in *.ui files to methods in code
   connect( actionExit, &QAction::triggered, this, &QWidget::close );                                                   // > File > Exit
   connect( actionAbout_BrewTarget, &QAction::triggered, dialog_about, &QWidget::show );                                // > About > About Brewtarget
   connect( actionNewRecipe, &QAction::triggered, this, &MainWindow::newRecipe );                                       // > File > New Recipe
   connect( actionImportFromXml, &QAction::triggered, this, &MainWindow::importFiles );                                // > File > Import Recipes
   connect( actionExportToXml, &QAction::triggered, this, &MainWindow::exportRecipe );                                 // > File > Export Recipes
   connect( actionUndo, &QAction::triggered, this, &MainWindow::editUndo );                                             // > Edit > Undo
   connect( actionRedo, &QAction::triggered, this, &MainWindow::editRedo );                                             // > Edit > Redo
   setUndoRedoEnable();
   connect( actionEquipments, &QAction::triggered, equipEditor, &QWidget::show );                                       // > View > Equipments
   connect( actionMashs, &QAction::triggered, namedMashEditor, &QWidget::show );                                        // > View > Mashs
   connect( actionStyles, &QAction::triggered, styleEditor, &QWidget::show );                                           // > View > Styles
   connect( actionFermentables, &QAction::triggered, fermDialog, &QWidget::show );                                      // > View > Fermentables
   connect( actionHops, &QAction::triggered, hopDialog, &QWidget::show );                                               // > View > Hops
   connect( actionMiscs, &QAction::triggered, miscDialog, &QWidget::show );                                             // > View > Miscs
   connect( actionYeasts, &QAction::triggered, yeastDialog, &QWidget::show );                                           // > View > Yeasts
   connect( actionOptions, &QAction::triggered, optionDialog, &OptionDialog::show );                                    // > Tools > Options
   connect( actionManual, &QAction::triggered, this, &MainWindow::openManual );                                         // > About > Manual
   connect( actionScale_Recipe, &QAction::triggered, recipeScaler, &QWidget::show );                                    // > Tools > Scale Recipe
   connect( action_recipeToTextClipboard, &QAction::triggered, recipeFormatter, &RecipeFormatter::toTextClipboard );    // > Tools > Recipe to Clipboard as Text
   connect( actionConvert_Units, &QAction::triggered, converterTool, &QWidget::show );                                  // > Tools > Convert Units
   connect( actionHydrometer_Temp_Adjustment, &QAction::triggered, hydrometerTool, &QWidget::show );                    // > Tools > Hydrometer Temp Adjustment
   connect( actionAlcohol_Percentage_Tool, &QAction::triggered, alcoholTool, &QWidget::show );                          // > Tools > Alcohol
   connect( actionOG_Correction_Help, &QAction::triggered, ogAdjuster, &QWidget::show );                                // > Tools > OG Correction Help
   connect( actionCopy_Recipe, &QAction::triggered, this, &MainWindow::copyRecipe );                                    // > File > Copy Recipe
   connect( actionPriming_Calculator, &QAction::triggered, primingDialog, &QWidget::show );                             // > Tools > Priming Calculator
   connect( actionStrikeWater_Calculator, &QAction::triggered, strikeWaterDialog, &QWidget::show );                     // > Tools > Strike Water Calculator
   connect( actionRefractometer_Tools, &QAction::triggered, refractoDialog, &QWidget::show );                           // > Tools > Refractometer Tools
   connect( actionPitch_Rate_Calculator, &QAction::triggered, this, &MainWindow::showPitchDialog);                      // > Tools > Pitch Rate Calculator
   connect( actionMergeDatabases, &QAction::triggered, this, &MainWindow::updateDatabase );                             // > File > Database > Merge
   connect( actionTimers, &QAction::triggered, timerMainDialog, &QWidget::show );                                       // > Tools > Timers
   connect( actionDeleteSelected, &QAction::triggered, this, &MainWindow::deleteSelected );
   connect( actionWater_Chemistry, &QAction::triggered, this, &MainWindow::popChemistry);                               // > Tools > Water Chemistry
   connect( actionAncestors, &QAction::triggered, this, &MainWindow::setAncestor);                                      // > Tools > Ancestors
   connect( action_brewit, &QAction::triggered, this, &MainWindow::brewItHelper );
   //One Dialog to rule them all, at least all printing and export.
   connect( actionPrint, &QAction::triggered, printAndPreviewDialog, &QWidget::show);                                   // > File > Print and Preview

   // postgresql cannot backup or restore yet. I would like to find some way
   // around this, but for now just disable
   if ( Brewtarget::dbType() == Brewtarget::PGSQL ) {
      actionBackup_Database->setEnabled(false);                                                                         // > File > Database > Backup
      actionRestore_Database->setEnabled(false);                                                                        // > File > Database > Restore
   }
   else {
      connect( actionBackup_Database, &QAction::triggered, this, &MainWindow::backup );                                 // > File > Database > Backup
      connect( actionRestore_Database, &QAction::triggered, this, &MainWindow::restoreFromBackup );                     // > File > Database > Restore
   }
}

// pushbuttons with a SIGNAL of clicked() should go in here.
void MainWindow::setupClicks()
{
   connect( equipmentButton, &QAbstractButton::clicked, this, &MainWindow::showEquipmentEditor);
   connect( styleButton, &QAbstractButton::clicked, this, &MainWindow::showStyleEditor );
   connect( mashButton, &QAbstractButton::clicked, mashEditor, &MashEditor::showEditor );
   connect( pushButton_addFerm, &QAbstractButton::clicked, fermDialog, &QWidget::show );
   connect( pushButton_addHop, &QAbstractButton::clicked, hopDialog, &QWidget::show );
   connect( pushButton_addMisc, &QAbstractButton::clicked, miscDialog, &QWidget::show );
   connect( pushButton_addYeast, &QAbstractButton::clicked, yeastDialog, &QWidget::show );
   connect( pushButton_removeFerm, &QAbstractButton::clicked, this, &MainWindow::removeSelectedFermentable );
   connect( pushButton_removeHop, &QAbstractButton::clicked, this, &MainWindow::removeSelectedHop );
   connect( pushButton_removeMisc, &QAbstractButton::clicked, this, &MainWindow::removeSelectedMisc );
   connect( pushButton_removeYeast, &QAbstractButton::clicked, this, &MainWindow::removeSelectedYeast );
   connect( pushButton_editFerm, &QAbstractButton::clicked, this, &MainWindow::editSelectedFermentable );
   connect( pushButton_editMisc, &QAbstractButton::clicked, this, &MainWindow::editSelectedMisc );
   connect( pushButton_editHop, &QAbstractButton::clicked, this, &MainWindow::editSelectedHop );
   connect( pushButton_editYeast, &QAbstractButton::clicked, this, &MainWindow::editSelectedYeast );
   connect( pushButton_editMash, &QAbstractButton::clicked, mashEditor, &MashEditor::showEditor );
   connect( pushButton_addMashStep, &QAbstractButton::clicked, this, &MainWindow::addMashStep );
   connect( pushButton_removeMashStep, &QAbstractButton::clicked, this, &MainWindow::removeSelectedMashStep );
   connect( pushButton_editMashStep, &QAbstractButton::clicked, this, &MainWindow::editSelectedMashStep );
   connect( pushButton_mashWizard, &QAbstractButton::clicked, mashWizard, &MashWizard::show );
   connect( pushButton_saveMash, &QAbstractButton::clicked, this, &MainWindow::saveMash );
   connect( pushButton_mashDes, &QAbstractButton::clicked, mashDesigner, &MashDesigner::show );
   connect( pushButton_mashUp, &QAbstractButton::clicked, this, &MainWindow::moveSelectedMashStepUp );
   connect( pushButton_mashDown, &QAbstractButton::clicked, this, &MainWindow::moveSelectedMashStepDown );
   connect( pushButton_mashRemove, &QAbstractButton::clicked, this, &MainWindow::removeMash );
   return;
}

// comboBoxes with a SIGNAL of activated() should go in here.
void MainWindow::setupActivate()
{
   connect( equipmentComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeEquipment()) );
   connect( styleComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeStyle()) );
   connect( mashComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeMash()) );
}

// lineEdits with either an editingFinished() or a textModified() should go in
// here
void MainWindow::setupTextEdit()
{
   connect( lineEdit_name, &QLineEdit::editingFinished, this, &MainWindow::updateRecipeName );
   connect( lineEdit_batchSize, &BtLineEdit::textModified, this, &MainWindow::updateRecipeBatchSize );
   connect( lineEdit_boilSize, &BtLineEdit::textModified, this, &MainWindow::updateRecipeBoilSize );
   connect( lineEdit_boilTime, &BtLineEdit::textModified, this, &MainWindow::updateRecipeBoilTime );
   connect( lineEdit_efficiency, &BtLineEdit::textModified, this, &MainWindow::updateRecipeEfficiency );
}

// anything using a BtLabel::labelChanged signal should go in here
void MainWindow::setupLabels()
{
   // These are the sliders. I need to consider these harder, but small steps
   connect(oGLabel,       &BtLabel::labelChanged, this, &MainWindow::redisplayLabel);
   connect(fGLabel,       &BtLabel::labelChanged, this, &MainWindow::redisplayLabel);
   connect(colorSRMLabel, &BtLabel::labelChanged, this, &MainWindow::redisplayLabel);
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

   QModelIndex start = active->selectionModel()->selectedRows().first();
   active->deleteSelected(active->selectionModel()->selectedRows());

   if ( ! start.isValid() ) {
      start = active->first();
   }


   if ( start.isValid() ) {
      if (active->type(start) == BtTreeItem::RECIPE)
         setRecipe(treeView_recipe->recipe(start));
      setTreeSelection(start);
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
   Water *w;

   QObject* calledBy = sender();
   BtTreeView* active;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == nullptr )
      return;

   active = qobject_cast<BtTreeView*>(calledBy);

   // If the sender cannot be morphed into a BtTreeView object
   if ( active == nullptr )
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
      case BtTreeItem::WATER:
         w = active->water(index);
         if (w)
         {
            waterEditor->setWater(w);
            waterEditor->show();
         }
         break;
      default:
         qWarning() << QString("MainWindow::treeActivated Unknown type %1.").arg(treeView_recipe->type(index));
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
   QModelIndex pNdx = treeView_recipe->parent(index);

   // this gets complex. Versioning means we can't just clear the open
   // brewnote tabs out.
   if ( parent != this->recipeObs ) {
      if ( ! this->recipeObs->isMyAncestor(parent) ) {
         setRecipe(parent);
      }
      else if ( treeView_recipe->ancestorsAreShowing(pNdx) ) {
         tabWidget_recipeView->setCurrentIndex(0);
         // Start closing from the right (highest index) down. Anything else dumps
         // core in the most unpleasant of fashions
         int tabs = tabWidget_recipeView->count() - 1;
         for (int i = tabs; i >= 0; --i) {
            if (tabWidget_recipeView->widget(i)->objectName() == "BrewNoteWidget")
               tabWidget_recipeView->removeTab(i);
         }
         setRecipe(parent);
      }
   }

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
   return nullptr;
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

void MainWindow::setAncestor()
{
   Recipe* rec;
   if ( this->recipeObs ) {
      rec = this->recipeObs;
      
   }
   else {
      QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
      rec = treeView_recipe->recipe(indexes[0]);
   }

   ancestorDialog->setAncestor(rec);
   ancestorDialog->show();
}

// Can handle null recipes.
void MainWindow::setRecipe(Recipe* recipe)
{
   int tabs = 0;
   // Don't like void pointers.
   if( recipe == nullptr )
      return;

   // Make sure this MainWindow is paying attention...
   if( recipeObs )
      disconnect( recipeObs, nullptr, this, nullptr );
   recipeObs = recipe;

   this->recStyle = recipe->style();
   recEquip = recipe->equipment();
   this->displayRangesEtcForCurrentRecipeStyle();

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
   for (int i = tabs; i >= 0; --i) {
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
   singleStyleEditor->setStyle(recipe->style());

   mashEditor->setMash(recipeObs->mash());
   mashEditor->setRecipe(recipeObs);

   mashButton->setMash(recipeObs->mash());
   recipeScaler->setRecipe(recipeObs);

   // Set the locked flag as required
   checkBox_locked->setCheckState( recipe->locked() ? Qt::Checked : Qt::Unchecked );
   lockRecipe( recipe->locked() ? Qt::Checked : Qt::Unchecked );

   // Here's the fun part. If the recipe is locked and display is false, then
   // you have said "show versions" and we will not all the recipe to be
   // unlocked. Hmmm. Skeptical Mik is skeptical
   if ( recipe->locked() && ! recipe->display() ) {
      checkBox_locked->setEnabled( false );
   }
   else {
      checkBox_locked->setEnabled( true );
   }

   checkBox_locked->setCheckState( recipe->locked() ? Qt::Checked : Qt::Unchecked );
   lockRecipe(recipe->locked() ? Qt::Checked : Qt::Unchecked );
   // changes in how the data is loaded means we may not have fired all the signals we should have
   // this makes sure the signals are fired. This is likely a 5kg hammer driving a finishing nail.
   recipe->recalcAll();

   // If you don't connect this late, every previous set of an attribute
   // causes this signal to be slotted, which then causes showChanges() to be
   // called.
   connect( recipeObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   showChanges();
}

// When a recipe is locked, many fields need to be disabled.
void MainWindow::lockRecipe(int state)
{
   if ( this->recipeObs == nullptr )
      return;

   // If I am locking a recipe (lock == true ), I want to disable fields
   // (enable == false). If I am unlocking (lock == false), I want to enable
   // fields (enable == true). This just makes that easy
   bool lockIt = state == Qt::Checked;
   bool enabled = ! lockIt;

   // Lock/unlock the recipe, then disable/enable the fields. I am leaving the
   // name field as editable. I may regret that, but if you are defining an
   // inheritance tree, you may want to remove strings from the ancestoral
   // names
   this->recipeObs->setLocked(lockIt);

   // I could disable tab_recipe, but would not prevent you from unlocking the
   // recipe because that field would also be disabled
   qWidget_styleBox->setEnabled(enabled);
   qWidget_equipmentBox->setEnabled(enabled);
   lineEdit_batchSize->setEnabled(enabled);
   lineEdit_boilSize->setEnabled(enabled);
   lineEdit_efficiency->setEnabled(enabled);
   lineEdit_boilTime->setEnabled(enabled);

   // locked recipes cannot be deleted
   actionDeleteSelected->setEnabled(enabled);
   treeView_recipe->enableDelete(enabled);

   treeView_recipe->setDragDropMode( lockIt ? QAbstractItemView::NoDragDrop : QAbstractItemView::DragDrop);
   tabWidget_ingredients->setAcceptDrops( enabled );

   // Onto the tables. Four lines each to disable edits, drag/drop and deletes
   fermentableTable->setEnabled(enabled);
   pushButton_addFerm->setEnabled(enabled);
   pushButton_removeFerm->setEnabled(enabled);
   pushButton_editFerm->setEnabled(enabled);

   hopTable->setEnabled(enabled);
   pushButton_addHop->setEnabled(enabled);
   pushButton_removeHop->setEnabled(enabled);
   pushButton_editHop->setEnabled(enabled);

   miscTable->setEnabled(enabled);
   pushButton_addMisc->setEnabled(enabled);
   pushButton_removeMisc->setEnabled(enabled);
   pushButton_editMisc->setEnabled(enabled);

   yeastTable->setEnabled(enabled);
   pushButton_addYeast->setEnabled(enabled);
   pushButton_removeYeast->setEnabled(enabled);
   pushButton_editYeast->setEnabled(enabled);

   fermDialog->pushButton_addToRecipe->setEnabled(enabled);
   hopDialog->pushButton_addToRecipe->setEnabled(enabled);
   miscDialog->pushButton_addToRecipe->setEnabled(enabled);
   yeastDialog->pushButton_addToRecipe->setEnabled(enabled);
   // mashes still need dealing with
   //

}

void MainWindow::changed(QMetaProperty prop, QVariant value)
{
   QString propName(prop.name());

   if( propName == "equipment" )
   {
      Equipment* newRecEquip = qobject_cast<Equipment*>(NamedEntity::extractPtr(value));
      recEquip = newRecEquip;

      singleEquipEditor->setEquipment(recEquip);
   }
   else if( propName == "style" )
   {
      //recStyle = recipeObs->style();
      recStyle = qobject_cast<Style*>(NamedEntity::extractPtr(value));
      singleStyleEditor->setStyle(recStyle);

   }

   showChanges(&prop);
}

void MainWindow::updateDensitySlider(QString attribute, RangedSlider* slider, double max)
{
   Unit::unitDisplay dispUnit = static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, Unit::noUnit, "tab_recipe", Brewtarget::UNIT).toInt());

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
   Unit::unitDisplay dispUnit = static_cast<Unit::unitDisplay>(Brewtarget::option(attribute, Unit::noUnit, "tab_recipe", Brewtarget::UNIT).toInt());

   if ( dispUnit == Unit::noUnit )
      dispUnit = Brewtarget::colorUnit == Brewtarget::SRM ? Unit::displaySrm : Unit::displayEbc;

   slider->setPreferredRange(Brewtarget::displayRange(recStyle, tab_recipe, attribute,Brewtarget::COLOR));
   slider->setRange(Brewtarget::displayRange(tab_recipe, attribute, 1, 44, Brewtarget::COLOR) );
   slider->setTickMarks( dispUnit == Unit::displaySrm ? 10 : 40, 2);

}

void MainWindow::showChanges(QMetaProperty* prop)
{
   if( recipeObs == nullptr )
      return;

   bool updateAll = (prop == nullptr);
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
/*
   lineEdit_calcBatchSize->setText(recipeObs);
   lineEdit_calcBoilSize->setText(recipeObs);
*/

   // Color manipulation
/*
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
*/
   lineEdit_boilSg->setText(recipeObs);

   updateDensitySlider("og", styleRangeWidget_og, 1.120);
   styleRangeWidget_og->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"og",&Units::sp_grav,0));

   updateDensitySlider("fg", styleRangeWidget_fg, 1.03);
   styleRangeWidget_fg->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"fg",&Units::sp_grav,0));

   styleRangeWidget_abv->setValue(recipeObs->ABV_pct());
   styleRangeWidget_ibu->setValue(recipeObs->IBU());

   rangeWidget_batchsize->setRange(0, Brewtarget::amountDisplay(recipeObs,tab_recipe,"batchSize_l", &Units::liters,0));
   rangeWidget_batchsize->setPreferredRange(0, Brewtarget::amountDisplay(recipeObs,tab_recipe,"finalVolume_l", &Units::liters,0));
   rangeWidget_batchsize->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"finalVolume_l", &Units::liters,0));

   rangeWidget_boilsize->setRange(0, Brewtarget::amountDisplay(recipeObs,tab_recipe,"boilSize_l", &Units::liters,0));
   rangeWidget_boilsize->setPreferredRange(0, Brewtarget::amountDisplay(recipeObs,tab_recipe,"boilVolume_l", &Units::liters,0));
   rangeWidget_boilsize->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"boilVolume_l", &Units::liters,0));

   /* Colors need the same basic treatment as gravity */
   updateColorSlider("color_srm", styleRangeWidget_srm);
   styleRangeWidget_srm->setValue(Brewtarget::amountDisplay(recipeObs,tab_recipe,"color_srm",&Units::srm,0));

   // In some, incomplete, recipes, OG is approximately 1.000, which then makes GU close to 0 and thus IBU/GU insanely
   // large.  Besides being meaningless, such a large number takes up a lot of space.  So, where gravity units are
   // below 1, we just show IBU on the IBU/GU slider.
   auto gravityUnits = (recipeObs->og()-1)*1000;
   if (gravityUnits < 1) {
      gravityUnits = 1;
   }
   ibuGuSlider->setValue(recipeObs->IBU()/gravityUnits);

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
   if( recipeObs == nullptr || ! lineEdit_name->isModified())
      return;

   this->doOrRedoUpdate(*this->recipeObs, PropertyNames::NamedEntity::name, lineEdit_name->text(), tr("Change Recipe Name"));
}

void MainWindow::displayRangesEtcForCurrentRecipeStyle()
{
   if ( this->recipeObs == nullptr ) {
      return;
   }

   Style * style = this->recipeObs->style();
   if ( style == nullptr ) {
      return;
   }

   styleRangeWidget_og->setPreferredRange( Brewtarget::displayRange(style, tab_recipe, "og", Brewtarget::DENSITY ));
   styleRangeWidget_fg->setPreferredRange( Brewtarget::displayRange(style, tab_recipe, "fg", Brewtarget::DENSITY ));

   styleRangeWidget_abv->setPreferredRange(style->abvMin_pct(), style->abvMax_pct());
   styleRangeWidget_ibu->setPreferredRange(style->ibuMin(), style->ibuMax());
   styleRangeWidget_srm->setPreferredRange(Brewtarget::displayRange(style, tab_recipe, "color_srm", Brewtarget::COLOR));

   this->styleButton->setStyle(style);

   return;
}

void MainWindow::updateRecipeStyle()
{
   if( recipeObs == nullptr )
      return;

   QModelIndex proxyIndex( styleProxyModel->index(styleComboBox->currentIndex(),0) );
   QModelIndex sourceIndex( styleProxyModel->mapToSource(proxyIndex) );
   Style* selected = styleListModel->at(sourceIndex.row());
   if( selected )
   {
      this->doOrRedoUpdate(
         newRelationalUndoableUpdate(*this->recipeObs,
                                     &Recipe::setStyle,
                                     this->recipeObs->style(),
                                     selected,
                                     &MainWindow::displayRangesEtcForCurrentRecipeStyle,
                                     tr("Change Recipe Style"))
      );
   }
}

void MainWindow::updateRecipeMash()
{
   if( recipeObs == nullptr )
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

void MainWindow::updateEquipmentButton()
{
   if (this->recipeObs != nullptr) {
      this->equipmentButton->setEquipment(this->recipeObs->equipment());
   }
   return;
}

void MainWindow::droppedRecipeEquipment(Equipment *kit)
{
   if( recipeObs == nullptr )
      return;

   // equip may be null.
   if( kit == nullptr )
      return;

   // We need to hang on to this QUndoCommand pointer because there might be other updates linked to it - see below
   auto equipmentUpdate = newRelationalUndoableUpdate(*this->recipeObs,
                                                      &Recipe::setEquipment,
                                                      this->recipeObs->equipment(),
                                                      kit,
                                                      &MainWindow::updateEquipmentButton,
                                                      tr("Change Recipe Kit"));

   // Keep the mash tun weight and specific heat up to date.
   Mash* m = recipeObs->mash();
   if( m )
   {
      new SimpleUndoableUpdate(*m, "tunWeight_kg", kit->tunWeight_kg(), tr("Change Tun Weight"), equipmentUpdate);
      new SimpleUndoableUpdate(*m, "tunSpecificHeat_calGC", kit->tunSpecificHeat_calGC(), tr("Change Tun Specific Heat"), equipmentUpdate);
   }

   if( QMessageBox::question(this,
                             tr("Equipment request"),
                             tr("Would you like to set the batch size, boil size and time to that requested by the equipment?"),
                             QMessageBox::Yes,
                             QMessageBox::No)
        == QMessageBox::Yes
     )
   {
      // If we do update batch size etc as a result of the equipment change, then we want those updates to undo/redo
      // if and when the equipment change is undone/redone.  Setting the parent field on a QUndoCommand makes that
      // parent the owner, responsible for invoking, deleting, etc.  Technically the descriptions of these subcommands
      // won't ever be seen by the user, but there's no harm in setting them.
      // (The previous call here to mashEditor->setRecipe() was a roundabout way of calling setTunWeight_kg() and
      // setTunSpecificHeat_calGC() on the mash.)
      new SimpleUndoableUpdate(*this->recipeObs, "batchSize_l", kit->batchSize_l(), tr("Change Batch Size"), equipmentUpdate);
      new SimpleUndoableUpdate(*this->recipeObs, "boilSize_l", kit->boilSize_l(), tr("Change Boil Size"), equipmentUpdate);
      new SimpleUndoableUpdate(*this->recipeObs, "boilTime_min", kit->boilTime_min(), tr("Change Boil Time"), equipmentUpdate);
   }

   // This will do the equipment update and any related updates - see above
   this->doOrRedoUpdate(equipmentUpdate);
}

// This isn't called when we think it is...!
void MainWindow::droppedRecipeStyle(Style* style)
{
   qDebug() << "MainWindow::droppedRecipeStyle";

   if ( ! recipeObs )
      return;
   // When the style is changed, we also need to update what is shown on the Style button
   qDebug() << "MainWindow::droppedRecipeStyle - do or redo";
   this->doOrRedoUpdate(
      newRelationalUndoableUpdate(*this->recipeObs,
                                  &Recipe::setStyle,
                                  this->recipeObs->style(),
                                  style,
                                  &MainWindow::displayRangesEtcForCurrentRecipeStyle,
                                  tr("Change Recipe Style"))
   );

   return;
}

// Well, aint this a kick in the pants. Apparently I can't template a slot
void MainWindow::droppedRecipeFermentable(QList<Fermentable*>ferms)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != fermentableTab )
      tabWidget_ingredients->setCurrentWidget(fermentableTab);
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Fermentable>,
                             ferms,
                             &Recipe::remove<Fermentable>,
                             tr("Drop fermentables on a recipe"))
   );
}

void MainWindow::droppedRecipeHop(QList<Hop*>hops)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != hopsTab )
      tabWidget_ingredients->setCurrentWidget(hopsTab);
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Hop>,
                             hops,
                             &Recipe::remove<Hop>,
                             tr("Drop hops on a recipe"))
   );
}

void MainWindow::droppedRecipeMisc(QList<Misc*>miscs)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != miscTab )
      tabWidget_ingredients->setCurrentWidget(miscTab);
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Misc>,
                             miscs,
                             &Recipe::remove<Misc>,
                             tr("Drop misc on a recipe"))
   );
}

void MainWindow::droppedRecipeYeast(QList<Yeast*>yeasts)
{
   if ( ! recipeObs )
      return;

   if ( tabWidget_ingredients->currentWidget() != yeastTab )
      tabWidget_ingredients->setCurrentWidget(yeastTab);
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Yeast>,
                             yeasts,
                             &Recipe::remove<Yeast>,
                             tr("Drop yeast on a recipe"))
   );
}

void MainWindow::updateRecipeBatchSize()
{
   if( recipeObs == nullptr )
      return;

   this->doOrRedoUpdate(*this->recipeObs, "batchSize_l", lineEdit_batchSize->toSI(), tr("Change Batch Size"));
}

void MainWindow::updateRecipeBoilSize()
{
   if( recipeObs == nullptr )
      return;

   this->doOrRedoUpdate(*this->recipeObs, "boilSize_l", lineEdit_boilSize->toSI(), tr("Change Boil Size"));
}

void MainWindow::updateRecipeBoilTime()
{
   double boilTime = 0.0;
   Equipment* kit;

   if( recipeObs == nullptr )
      return;

   kit = recipeObs->equipment();
   boilTime = Brewtarget::qStringToSI( lineEdit_boilTime->text(),&Units::minutes );

   // Here, we rely on a signal/slot connection to propagate the equipment
   // changes to recipeObs->boilTime_min and maybe recipeObs->boilSize_l
   // NOTE: This works because kit is the recipe's equipment, not the generic
   // equipment in the recipe drop down.
   if( kit )
      this->doOrRedoUpdate(*kit, "boilTime_min", boilTime, tr("Change Boil Time"));
   else
      this->doOrRedoUpdate(*this->recipeObs, "boilTime_min", boilTime, tr("Change Boil Time"));

   return;
}

void MainWindow::updateRecipeEfficiency()
{
   if( recipeObs == nullptr )
      return;

   this->doOrRedoUpdate(*this->recipeObs, PropertyNames::Recipe::efficiency_pct, lineEdit_efficiency->toSI(), tr("Change Recipe Efficiency"));
   return;
}

void MainWindow::addFermentableToRecipe(Fermentable* ferm)
{
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Fermentable>,
                             ferm,
                             &Recipe::remove<Fermentable>,
                             tr("Add fermentable to recipe"))
   );
   // We don't need to call fermTableModel->addFermentable(ferm) here because the change to the recipe will already have
   // triggered the necessary updates to fermTableModel.
}

void MainWindow::addHopToRecipe(Hop *hop)
{
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Hop>,
                             hop,
                             &Recipe::remove<Hop>,
                             tr("Add hop to recipe"))
   );
   // We don't need to call hopTableModel->addHop(hop) here because the change to the recipe will already have
   // triggered the necessary updates to hopTableModel.
}

void MainWindow::addMiscToRecipe(Misc* misc)
{
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Misc>,
                             misc,
                             &Recipe::remove<Misc>,
                             tr("Add misc to recipe"))
   );
   // We don't need to call miscTableModel->addMisc(misc) here because the change to the recipe will already have
   // triggered the necessary updates to miscTableModel.
}

void MainWindow::addYeastToRecipe(Yeast* yeast)
{
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs,
                             &Recipe::add<Yeast>,
                             yeast,
                             &Recipe::remove<Yeast>,
                             tr("Add yeast to recipe"))
   );
   // We don't need to call yeastTableModel->addYeast(yeast) here because the change to the recipe will already have
   // triggered the necessary updates to yeastTableModel.
}

void MainWindow::postAddMashStepToMash(MashStep * mashStep) {
   this->mashStepTableModel->addMashStep(mashStep);
   return;
}

void MainWindow::addMashStepToMash(MashStep * mashStep)
{
   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs->mash(),
                             &Mash::addMashStep,
                             mashStep,
                             &Mash::removeMashStep,
                             &MainWindow::postAddMashStepToMash,
                             static_cast<void (MainWindow::*)(MashStep *)>(nullptr),
                             tr("Add mash step to recipe"))
   );
   // .:TBD:. (MY 2020-11-24) For some reason that I didn't work out yet, mashStepTableModel is not plumbed in quite
   //         the same as the other table models, hence the need to add the callback to update it (otherwise Redo of
   //         add MashStep will not update the display correctly).  This works, but is a bit of a hack.  At some
   //         point it would be good to bring MashStepTableModel closer into line with similar classes - perhaps
   //         even by pulling out some of the common/similar code into a base class or template...
}

/**
 * .:TODO:. (MY 2020-12-30) See also MainWindow::exportSelected().  I wonder if we could share more code between that
 *                          function and this one.
 */
void MainWindow::exportRecipe()
{
   QFile* outFile;
   QDomDocument doc;
   BeerXML* bxml = Database::instance().getBeerXml();

   if( recipeObs == nullptr )
      return;

   outFile = openForWrite();
   if ( ! outFile )
      return;

   QTextStream out(outFile);

   QString xmlHead = QString("version=\"1.0\" encoding=\"ISO-8859-1\"");

   // Create the headers to make other BeerXML parsers happy
   QDomProcessingInstruction inst = doc.createProcessingInstruction("xml", xmlHead);
   QDomComment beerxml = doc.createComment(QString(" BeerXML generated by Brewtarget %1 ").arg(VERSIONSTRING));

   doc.appendChild(inst);
   doc.appendChild(beerxml);

   QDomElement recipes = doc.createElement("RECIPES"); // The root element.
   doc.appendChild(recipes);
   bxml->toXml( recipeObs, doc, recipes );

   // QString::toLatin1 returns an ISO 8859-1 stream.
   out << doc.toString().toLatin1();

   outFile->close();
   delete outFile;
}

Recipe* MainWindow::currentRecipe()
{
   return recipeObs;
}

void MainWindow::setUndoRedoEnable()
{
   Q_ASSERT(this->undoStack != 0);
   actionUndo->setEnabled(this->undoStack->canUndo());
   actionRedo->setEnabled(this->undoStack->canRedo());

   actionUndo->setText(QString(tr("Undo %1").arg(this->undoStack->undoText())));
   actionRedo->setText(QString(tr("Redo %1").arg(this->undoStack->redoText())));

   return;
}

void MainWindow::doOrRedoUpdate(QUndoCommand * update)
{
   Q_ASSERT(this->undoStack != nullptr);
   Q_ASSERT(update != nullptr);
   this->undoStack->push(update);
   this->setUndoRedoEnable();
   return;
}

void MainWindow::doOrRedoUpdate(QObject & updatee,
                                char const * const propertyName,
                                QVariant newValue,
                                QString const & description,
                                QUndoCommand * parent) {
///   qDebug() << Q_FUNC_INFO << "Updating" << propertyName << "on" << updatee.metaObject()->className();
///   qDebug() << Q_FUNC_INFO << "this=" << static_cast<void *>(this);
   this->doOrRedoUpdate(new SimpleUndoableUpdate(updatee, propertyName, newValue, description));
   return;
}

// For undo/redo, we use Qt's Undo framework
void MainWindow::editUndo()
{
   Q_ASSERT(this->undoStack != 0);
   if ( !this->undoStack->canUndo() ) {
      qDebug() << "Undo called but nothing to undo";
   } else {
      this->undoStack->undo();
   }

   setUndoRedoEnable();
   return;
}

void MainWindow::editRedo()
{
   Q_ASSERT(this->undoStack != 0);
   if ( !this->undoStack->canRedo() ) {
      qDebug() << "Redo called but nothing to redo";
   } else {
      this->undoStack->redo();
   }

   setUndoRedoEnable();
   return;
}

Fermentable* MainWindow::selectedFermentable()
{
   QModelIndexList selected = fermentableTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return nullptr;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return nullptr;
   }

   modelIndex = fermTableProxy->mapToSource(viewIndex);
   Fermentable* ferm = fermTableModel->getFermentable(static_cast<unsigned int>(modelIndex.row()));

   return ferm;
}

Hop* MainWindow::selectedHop()
{
   QModelIndexList selected = hopTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return nullptr;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return nullptr;
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
      return nullptr;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return nullptr;
   }

   modelIndex = miscTableProxy->mapToSource(viewIndex);

   Misc* m = miscTableModel->getMisc(static_cast<unsigned int>(modelIndex.row()));

   return m;
}

Yeast* MainWindow::selectedYeast()
{
   QModelIndexList selected = yeastTable->selectionModel()->selectedIndexes();
   QModelIndex modelIndex, viewIndex;
   int row, size, i;

   size = selected.size();
   if( size == 0 )
      return nullptr;

   // Make sure only one row is selected.
   viewIndex = selected[0];
   row = viewIndex.row();
   for( i = 1; i < size; ++i )
   {
      if( selected[i].row() != row )
         return nullptr;
   }

   modelIndex = yeastTableProxy->mapToSource(viewIndex);

   Yeast* y = yeastTableModel->getYeast(static_cast<unsigned int>(modelIndex.row()));

   return y;
}

void MainWindow::removeHop(Hop * itemToRemove) {
   this->hopTableModel->removeHop(itemToRemove);
   return;
}

void MainWindow::removeFermentable(Fermentable * itemToRemove) {
   this->fermTableModel->removeFermentable(itemToRemove);
   return;
}

void MainWindow::removeMisc(Misc * itemToRemove) {
   this->miscTableModel->removeMisc(itemToRemove);
   return;
}

void MainWindow::removeYeast(Yeast * itemToRemove) {
   this->yeastTableModel->removeYeast(itemToRemove);
   return;
}

void MainWindow::removeMashStep(MashStep * itemToRemove) {
   this->mashStepTableModel->removeMashStep(itemToRemove);
   return;
}

void MainWindow::removeSelectedFermentable()
{

   QModelIndexList selected = fermentableTable->selectionModel()->selectedIndexes();
   QModelIndex viewIndex, modelIndex;
   QList<Fermentable *> itemsToRemove;
   int size, i;

   size = selected.size();

   qDebug() << QString("MainWindow::removeSelectedFermentable() %1 items selected to remove").arg(size);

   if( size == 0 )
      return;

   for(int i = 0; i < size; i++)
   {
      viewIndex = selected.at(i);
      modelIndex = fermTableProxy->mapToSource(viewIndex);

      itemsToRemove.append(fermTableModel->getFermentable(static_cast<unsigned int>(modelIndex.row())));
   }

   for(i = 0; i < itemsToRemove.size(); i++)
   {
      this->doOrRedoUpdate(
         newUndoableAddOrRemove(*this->recipeObs,
                                &Recipe::remove<Fermentable>,
                                itemsToRemove.at(i),
                                &Recipe::add<Fermentable>,
                                &MainWindow::removeFermentable,
                                static_cast<void (MainWindow::*)(Fermentable *)>(nullptr),
                                tr("Remove fermentable from recipe"))
      );
    }

    return;
}

void MainWindow::editSelectedFermentable()
{
   Fermentable* f = selectedFermentable();
   if( f == nullptr )
      return;

   fermEditor->setFermentable(f);
   fermEditor->show();
}

void MainWindow::editSelectedMisc()
{
   Misc* m = selectedMisc();
   if( m == nullptr )
      return;

   miscEditor->setMisc(m);
   miscEditor->show();
}

void MainWindow::editSelectedHop()
{
   Hop* h = selectedHop();
   if( h == nullptr )
      return;

   hopEditor->setHop(h);
   hopEditor->show();
}

void MainWindow::editSelectedYeast()
{
   Yeast* y = selectedYeast();
   if( y == nullptr )
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
      this->doOrRedoUpdate(
         newUndoableAddOrRemove(*this->recipeObs,
                                 &Recipe::remove<Hop>,
                                 itemsToRemove.at(i),
                                 &Recipe::add<Hop>,
                                 &MainWindow::removeHop,
                                 static_cast<void (MainWindow::*)(Hop *)>(nullptr),
                                 tr("Remove hop from recipe"))
      );
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

      itemsToRemove.append(miscTableModel->getMisc(static_cast<unsigned int>(modelIndex.row())));
   }

   for(i = 0; i < itemsToRemove.size(); i++)
   {
      this->doOrRedoUpdate(
         newUndoableAddOrRemove(*this->recipeObs,
                                 &Recipe::remove<Misc>,
                                 itemsToRemove.at(i),
                                 &Recipe::add<Misc>,
                                 &MainWindow::removeMisc,
                                 static_cast<void (MainWindow::*)(Misc *)>(nullptr),
                                 tr("Remove misc from recipe"))
      );
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

      itemsToRemove.append(yeastTableModel->getYeast(static_cast<unsigned int>(modelIndex.row())));
   }

   for(i = 0; i < itemsToRemove.size(); i++)
   {
      this->doOrRedoUpdate(
         newUndoableAddOrRemove(*this->recipeObs,
                                 &Recipe::remove<Yeast>,
                                 itemsToRemove.at(i),
                                 &Recipe::add<Yeast>,
                                 &MainWindow::removeYeast,
                                 static_cast<void (MainWindow::*)(Yeast *)>(nullptr),
                                 tr("Remove yeast from recipe"))
      );
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

   Recipe* newRec = new Recipe(name);

   // bad things happened -- let somebody know
   if ( ! newRec ) {
      QMessageBox::warning(this,tr("Error creating recipe"),
                           tr("An error was returned while creating %1").arg(name));
      return;
   }
   // Set the following stuff so everything appears nice
   // and the calculations don't divide by zero... things like that.
   newRec->setBatchSize_l(18.93); // 5 gallons
   newRec->setBoilSize_l(23.47);  // 6.2 gallons
   newRec->setEfficiency_pct(70.0);

   // we need a valid key, so insert the recipe before we add equipment
   if ( defEquipKey != -1 )
   {
      Equipment *e = Database::instance().equipment(defEquipKey.toInt());
      // I really want to do this before we've written the object to the
      // database
      if ( e )
      {
         newRec->setBatchSize_l( e->batchSize_l() );
         newRec->setBoilSize_l( e->boilSize_l() );
         newRec->setBoilTime_min( e->boilTime_min() );
         Database::instance().addToRecipe(newRec, e);
      }
   }

   newRec->insertInDatabase();

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

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QString::SplitBehavior skip = QString::SkipEmptyParts;
#else
   Qt::SplitBehaviorFlags skip = Qt::SkipEmptyParts;
#endif
   if ( name.split("/", skip).isEmpty() )
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
   if ( active == nullptr )
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

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QString::SplitBehavior skip = QString::SkipEmptyParts;
#else
   Qt::SplitBehaviorFlags skip = Qt::SkipEmptyParts;
#endif
   if ( newName.split("/", skip).isEmpty() )
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

   if ( active == nullptr )
      active = qobject_cast<BtTreeView*>(treeView_recipe);

   // Couldn't cast the active item to a BtTreeView
   if ( active == nullptr )
      return;

   QModelIndex parent = active->parent(item);

   active->setCurrentIndex(item);
   if ( active->type(parent) == BtTreeItem::FOLDER && ! active->isExpanded(parent) )
      active->setExpanded(parent,true);
   active->scrollTo(item,QAbstractItemView::PositionAtCenter);

}
// reduces the inventory by the selected recipes
void MainWindow::reduceInventory(){

   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();

   foreach(QModelIndex selected, indexes) {
      Recipe* rec   = treeView_recipe->recipe(selected);
      if ( rec == nullptr ) {
         //try the parent recipe
         rec = treeView_recipe->recipe(treeView_recipe->parent(selected));
         if ( rec == nullptr ) {
            continue;
         }
      }

      // Make sure everything is properly set and selected
      if( rec != recipeObs )
         setRecipe(rec);

      int i = 0;
      //reduce fermentables
      QList<Fermentable*> flist = rec->fermentables();
      if ( flist.size() > 0 ){
         for( i = 0; static_cast<int>(i) < flist.size(); ++i ) {
            double newVal=flist[i]->inventory() - flist[i]->amount_kg();
            newVal = (newVal < 0) ? 0 : newVal;
            flist[i]->setInventoryAmount(newVal);
         }
      }

      //reduce misc
      QList<Misc*> mlist = rec->miscs();
      if ( mlist.size() > 0 ) {
         for( i = 0; static_cast<int>(i) < mlist.size(); ++i ) {
            double newVal=mlist[i]->inventory() - mlist[i]->amount();
            newVal = (newVal < 0) ? 0 : newVal;
            mlist[i]->setInventoryAmount(newVal);
         }
      }
      //reduce hops
      QList<Hop*> hlist = rec->hops();
      if( hlist.size() > 0 ) {
         for( i = 0; static_cast<int>(i) < hlist.size(); ++i ) {
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

   foreach(QModelIndex selected, indexes) {
      Recipe*   rec   = treeView_recipe->recipe(selected);

      if( rec == nullptr )
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
   // NB: QDir does all the necessary magic of translating '/' to whatever current platform's directory separator is
   QString defaultBackupFileName = QDir::currentPath() + "/" + Database::getDefaultBackupFileName();
   QString backupFileName = QFileDialog::getSaveFileName(this, tr("Backup Database"), defaultBackupFileName);
   qDebug() << QString("Database backup filename \"%1\"").arg(backupFileName);

   // If the filename returned from the dialog is empty, it means the user clicked cancel, so we should stop trying to do the backup
   if (!backupFileName.isEmpty())
   {
      bool success = Database::backupToFile(backupFileName);

      if( ! success )
         QMessageBox::warning( this, tr("Oops!"), tr("Could not copy the files for some reason."));
   }
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

// Imports all the recipes, hops, equipment or whatever from a BeerXML file into the database.
void MainWindow::importFiles()
{
   this->pimpl->importFromFiles(this);
}

bool MainWindow::verifyImport(QString tag, QString name)
{
   return QMessageBox::question(this, tr("Import %1?").arg(tag), tr("Import %1?").arg(name),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::addMashStep()
{
   Mash* mash;
   if( recipeObs != nullptr && recipeObs->mash() != nullptr )
   {
      mash = recipeObs->mash();
   }
   else
   {
      QMessageBox::information(this, tr("No mash"), tr("Trying to add a mash step without a mash. Please create a mash first.") );
      return;
   }

   MashStep* step = new MashStep("", true);
   step->setMash(mash);
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
}

void MainWindow::removeSelectedMashStep()
{
   Mash* mash = recipeObs == nullptr ? nullptr : recipeObs->mash();
   if( mash == nullptr )
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

   MashStep* step = mashStepTableModel->getMashStep(static_cast<unsigned int>(row));

   this->doOrRedoUpdate(
      newUndoableAddOrRemove(*this->recipeObs->mash(),
                              &Mash::removeMashStep,
                              step,
                              &Mash::addMashStep,
                              &MainWindow::removeMashStep,
                              &MainWindow::postAddMashStepToMash,
                              tr("Remove mash step"))
   );

   return;
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

   MashStep* m1 = mashStepTableModel->getMashStep(static_cast<unsigned int>(row));
   MashStep* m2 = mashStepTableModel->getMashStep(static_cast<unsigned int>(row-1));
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

   MashStep* m1 = mashStepTableModel->getMashStep(static_cast<unsigned int>(row));
   MashStep* m2 = mashStepTableModel->getMashStep(static_cast<unsigned int>(row+1));
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

   MashStep* step = mashStepTableModel->getMashStep(static_cast<unsigned int>(row));
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
}

void MainWindow::removeMash()
{
   Mash *m = mashButton->mash();

   if( m == nullptr)
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
   if( recipeObs == nullptr )
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
   if( recipeObs == nullptr || recipeObs->mash() == nullptr )
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

// We build the menus at start up time.  This just needs to exec the proper
// menu.
void MainWindow::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   BtTreeView* active;
   QModelIndex selected;
   QMenu* tempMenu;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == nullptr )
      return;

   active = qobject_cast<BtTreeView*>(calledBy);

   // If the sender cannot be morphed into a BtTreeView object
   if ( active == nullptr )
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
   treeView_equip->setupContextMenu(this,singleEquipEditor);

   treeView_ferm->setupContextMenu(this,fermDialog);
   treeView_hops->setupContextMenu(this,hopDialog);
   treeView_misc->setupContextMenu(this,miscDialog);
   treeView_style->setupContextMenu(this,singleStyleEditor);
   treeView_yeast->setupContextMenu(this,yeastDialog);
   treeView_water->setupContextMenu(this,waterEditor);

   // TreeView for clicks, both double and right
   connect( treeView_recipe, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_recipe, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_equip, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_equip, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_ferm, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_ferm, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_hops, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_hops, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_misc, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_misc, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_yeast, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_yeast, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_style, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_style, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);

   connect( treeView_water, &QAbstractItemView::doubleClicked, this, &MainWindow::treeActivated);
   connect( treeView_water, &QWidget::customContextMenuRequested, this, &MainWindow::contextMenu);
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
         qWarning() << QString("MainWindow::openForWrite Could not open %1 for writing.").arg(filename);
         outFile = nullptr;
      }
   }
   else
     outFile = nullptr;

   return outFile;
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
   BeerXML* bxml = Database::instance().getBeerXml();


   if ( active == nullptr )
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
   QDomComment beerxml = doc.createComment(QString(" BeerXML generated by Brewtarget %1 ").arg(VERSIONSTRING));

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
            bxml->toXml( treeView_recipe->recipe(selection), doc, recipe);
            didRecipe = true;
            break;
         case BtTreeItem::EQUIPMENT:
            bxml->toXml( treeView_equip->equipment(selection), doc, dbase);
            break;
         case BtTreeItem::FERMENTABLE:
            bxml->toXml( treeView_ferm->fermentable(selection), doc, dbase);
            break;
         case BtTreeItem::HOP:
            bxml->toXml( treeView_hops->hop(selection), doc, dbase);
            break;
         case BtTreeItem::MISC:
            bxml->toXml( treeView_misc->misc(selection), doc, dbase);
            break;
         case BtTreeItem::STYLE:
            bxml->toXml( treeView_style->style(selection), doc, dbase);
            break;
         case BtTreeItem::YEAST:
            bxml->toXml( treeView_yeast->yeast(selection), doc, dbase);
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
   if( reply == nullptr )
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

void MainWindow::redisplayLabel()
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
   if ( recipeObs && ! recipeObs->equipment() )
   {
      QMessageBox::warning( this, tr("No equipment"), tr("You must select or define an equipment profile first."));
   }
   else
   {
      singleEquipEditor->setEquipment(recipeObs->equipment());
      singleEquipEditor->show();
   }
}

void MainWindow::showStyleEditor()
{
   if ( recipeObs && ! recipeObs->style() )
   {
      QMessageBox::warning( this, tr("No style"), tr("You must select a style first."));
   }
   else
   {
      singleStyleEditor->setStyle(recipeObs->style());
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

void MainWindow::versionedRecipe(Recipe* descendant)
{
   QModelIndex ndx = treeView_recipe->findElement(descendant);
   setRecipe(descendant);
   treeView_recipe->setCurrentIndex(ndx);
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

void MainWindow::popChemistry()
{
   bool allow = false;

   if ( recipeObs ) {

      Mash* eMash = recipeObs->mash();
      if ( eMash && eMash->mashSteps().size() > 0 ) {
         allow = true;
      }
   }

   // late binding for the win?
   if (allow ) {
      waterDialog->setRecipe(recipeObs);
      waterDialog->show();
   }
   else {
      QMessageBox::warning( this, tr("No Mash"), tr("You must define a mash first."));
   }
}
