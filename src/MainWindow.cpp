/*
 * MainWindow.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "BeerColorWidget.h"
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
#include "MashWizard.h"
#include "MiscDialog.h"
#include "StyleEditor.h"
#include "OptionDialog.h"
#include "HtmlViewer.h"
#include "OgAdjuster.h"
#include "ConverterTool.h"
#include "TimerListDialog.h"
#include "RecipeFormatter.h"
#include "PrimingDialog.h"
#include "RefractoDialog.h"
#include "MashDesigner.h"
#include "PitchDialog.h"
#include "MaltinessWidget.h"
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

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
{
   // Need to call this to get all the widgets added (I think).
   setupUi(this);

   QDesktopWidget *desktop = QApplication::desktop();

   // Ensure database initializes.
   Database::instance();

   // Set the window title.
   setWindowTitle( QString("Brewtarget - %1").arg(VERSIONSTRING) );
  
   // If we converted from XML, pop a dialog telling the user where they can
   // find their backups.
   if (Database::instance().isConverted())
      convertedMsg(); 
   
   // Different palettes for some text.
   lcdPalette_old = lcdNumber_og->palette();
   lcdPalette_tooLow = QPalette(lcdPalette_old);
   lcdPalette_tooLow.setColor(QPalette::Active, QPalette::WindowText, QColor::fromRgb(0, 0, 208));
   lcdPalette_good = QPalette(lcdPalette_old);
   lcdPalette_good.setColor(QPalette::Active, QPalette::WindowText, QColor::fromRgb(0, 128, 0));
   lcdPalette_tooHigh = QPalette(lcdPalette_old);
   lcdPalette_tooHigh.setColor(QPalette::Active, QPalette::WindowText, QColor::fromRgb(208, 0, 0));

   // Set constant colors for some lcds.
   lcdNumber_ogLow->setConstantColor(BtDigitWidget::LOW);
   lcdNumber_ogHigh->setConstantColor(BtDigitWidget::HIGH);
   lcdNumber_fgLow->setConstantColor(BtDigitWidget::LOW);
   lcdNumber_fgHigh->setConstantColor(BtDigitWidget::HIGH);
   lcdNumber_abvLow->setConstantColor(BtDigitWidget::LOW);
   lcdNumber_abvHigh->setConstantColor(BtDigitWidget::HIGH);
   lcdNumber_ibuLow->setConstantColor(BtDigitWidget::LOW);
   lcdNumber_ibuHigh->setConstantColor(BtDigitWidget::HIGH);
   lcdNumber_srmLow->setConstantColor(BtDigitWidget::LOW);
   lcdNumber_srmHigh->setConstantColor(BtDigitWidget::HIGH);
   lcdNumber_boilSG->setConstantColor(BtDigitWidget::BLACK);
   lcdNumber_ibugu->setConstantColor(BtDigitWidget::BLACK);
   lcdNumber_calories->setConstantColor(BtDigitWidget::BLACK);

   // Null out the recipe
   recipeObs = 0;
   
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
   htmlViewer = new HtmlViewer(this);
   recipeScaler = new ScaleRecipeTool(this);
   recipeFormatter = new RecipeFormatter(this);
   ogAdjuster = new OgAdjuster(this);
   converterTool = new ConverterTool(this);
   timerListDialog = new TimerListDialog(this);
   primingDialog = new PrimingDialog(this);
   refractoDialog = new RefractoDialog(this);
   mashDesigner = new MashDesigner(this);
   pitchDialog = new PitchDialog(this);

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
   
   // Create the keyboard shortcuts
   setupShortCuts();

   // Set up the printer
   printer = new QPrinter;
   printer->setPageSize(QPrinter::Letter);

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

   // And test out the maltiness widget.
   maltWidget = new MaltinessWidget(tabWidget_recipeView);
   verticalLayout_beerColor->insertWidget( 1, maltWidget );

   // Set up HtmlViewer to view documentation.
   htmlViewer->setHtml(Brewtarget::getDocDir() + "index.html");

   // Do some magic on the splitter widget to keep the tree from expanding
   splitter_2->setStretchFactor(0,0);
   splitter_2->setStretchFactor(1,1);

   // Once more with the context menus too
   setupContextMenu();

   // clear out the brewnotes 
   brewNotes.clear();

   // If we saved a size the last time we ran, use it
   if ( Brewtarget::btSettings.contains("geometry"))
   {
      restoreGeometry(Brewtarget::btSettings.value("geometry").toByteArray());
      restoreState(Brewtarget::btSettings.value("windowState").toByteArray());
   }
   else
   {
      // otherwise, guess a reasonable size at 1/4 of the screen.
      int width = desktop->width();
      int height = desktop->height();

      this->resize(width/2,height/2);
   }

   // If we saved the selected recipe name the last time we ran, select it and show it.
   if (Brewtarget::btSettings.contains("recipeKey"))
   {
      int key = Brewtarget::btSettings.value("recipeKey").toInt();
      recipeObs = Database::instance().recipe( key );

      setRecipe(recipeObs);
      setTreeSelection(treeView_recipe->findRecipe(recipeObs));
   }     
   else
   {
      QList<Recipe*> recs = Database::instance().recipes();
      if( recs.size() > 0 )
         setRecipe( recs[0] );
   }

   // Connect signals.
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
   connect( actionManual, SIGNAL( triggered() ), htmlViewer, SLOT( show() ) );
   connect( actionScale_Recipe, SIGNAL( triggered() ), recipeScaler, SLOT( show() ) );
   connect( action_recipeToTextClipboard, SIGNAL( triggered() ), recipeFormatter, SLOT( toTextClipboard() ) );
   connect( actionConvert_Units, SIGNAL( triggered() ), converterTool, SLOT( show() ) );
   connect( actionOG_Correction_Help, SIGNAL( triggered() ), ogAdjuster, SLOT( show() ) );
   connect( actionBackup_Database, SIGNAL( triggered() ), this, SLOT( backup() ) );
   connect( actionRestore_Database, SIGNAL( triggered() ), this, SLOT( restoreFromBackup() ) );
   connect( actionCopy_Recipe, SIGNAL( triggered() ), this, SLOT( copyRecipe() ) );
   connect( actionPriming_Calculator, SIGNAL( triggered() ), primingDialog, SLOT( show() ) );
   connect( actionRefractometer_Tools, SIGNAL( triggered() ), refractoDialog, SLOT( show() ) );
   connect( actionPitch_Rate_Calculator, SIGNAL(triggered()), this, SLOT(showPitchDialog()));
   connect( actionMergeDatabases, SIGNAL(triggered()), this, SLOT(updateDatabase()) );
   connect( actionTimers, SIGNAL(triggered()), timerListDialog, SLOT(show()) );
   connect( actionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()) );
   //connect( actionSave, SIGNAL(triggered()), this, SLOT(save()) );
   connect( actionDonate, SIGNAL( triggered() ), this, SLOT( openDonateLink() ) );

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

   // Printing signals/slots.
   // Refactoring is good.  It's like a rye saison fermenting away
   connect( actionRecipePrint, SIGNAL(triggered()), this, SLOT(print()));
   connect( actionRecipePreview, SIGNAL(triggered()), this, SLOT(print()));
   connect( actionRecipeHTML, SIGNAL(triggered()), this, SLOT(print()));
   connect( actionBrewdayPrint, SIGNAL(triggered()), this, SLOT(print()));
   connect( actionBrewdayPreview, SIGNAL(triggered()), this, SLOT(print()));
   connect( actionBrewdayHTML, SIGNAL(triggered()), this, SLOT(print()));

   // Connect up all the labels. I really need to find a better way.
   connect(targetBatchSizeLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(calculatedBatchSizeLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(targetBoilSizeLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(calculatedBoilSizeLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(oGLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(boilSgLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(fGLabel, SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));
   connect(colorSRMLabel,SIGNAL(labelChanged(QString)), this, SLOT(redisplayLabel(QString)));

   // Those are the easy ones. Let's see what we can do with the tables. First one wires the cells, second wires (I think) the header
   // per-cell has been disabled
   // connect(fermentableTable, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(fermentableCellSignal(const QPoint&)));
   QHeaderView* headerView = fermentableTable->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(fermentableContextMenu(const QPoint&)));

   headerView = hopTable->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(hopContextMenu(const QPoint&)));

   headerView = miscTable->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(miscContextMenu(const QPoint&)));

   headerView = yeastTable->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(yeastContextMenu(const QPoint&)));

   headerView = mashStepTableWidget->horizontalHeader();
   headerView->setContextMenuPolicy(Qt::CustomContextMenu);
   connect(headerView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(mashStepContextMenu(const QPoint&)));

   connect( dialog_about->pushButton_donate, SIGNAL(clicked()), this, SLOT(openDonateLink()) );

   connect( equipmentComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeEquipment()) );
   connect( equipmentButton, SIGNAL( clicked() ), this, SLOT(showEquipmentEditor()));

   connect( styleComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeStyle()) );
   connect( styleButton, SIGNAL( clicked() ), this, SLOT(showStyleEditor()) );

   connect( mashComboBox, SIGNAL( activated(int) ), this, SLOT(updateRecipeMash()) );
   connect( mashButton, SIGNAL( clicked() ), mashEditor, SLOT( showEditor() ) );

   connect( lineEdit_name, SIGNAL( editingFinished() ), this, SLOT( updateRecipeName() ) );
   connect( lineEdit_batchSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBatchSize() ) );
   connect( lineEdit_boilSize, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBoilSize() ) );
   connect( lineEdit_boilTime, SIGNAL( editingFinished() ), this, SLOT( updateRecipeBoilTime() ) );
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

void MainWindow::setupShortCuts()
{
   actionNewRecipe->setShortcut(QKeySequence::New);
   actionCopy_Recipe->setShortcut(QKeySequence::Copy);
   actionSave->setShortcut(QKeySequence::Save);
   actionDeleteSelected->setShortcut(QKeySequence::Delete);
}

void MainWindow::deleteSelected()
{
   QModelIndexList selected; 
   BrewTargetTreeView* active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   QModelIndex first, top;
   QList<QModelIndex>::const_iterator at,end;
   QList<Recipe*> deadRec;
   QList<Equipment*> deadKit;
   QList<Fermentable*> deadFerm;
   QList<Hop*> deadHop;
   QList<Misc*> deadMisc;
   QList<Yeast*> deadYeast;
   QList<BrewNote*> deadNote;

   if ( active == 0 )
      return;

   selected = active->selectionModel()->selectedRows();

   confirmDelete = QMessageBox::NoButton;
   // Get the dead things first.  Deleting as we process the list doesn't work, because the
   // delete updates the database and the indices get recalculated.
   for(at = selected.begin(),end = selected.end();at < end;++at)
   {
      switch(active->getType(*at))
      {
         case BrewTargetTreeItem::RECIPE:
            if ( *at != active->findRecipe(0) && verifyDelete("Recipe",active->getRecipe(*at)->name()))
               deadRec.append(active->getRecipe(*at));
            break;
         case BrewTargetTreeItem::EQUIPMENT:
            if (*at != active->findEquipment(0) && verifyDelete("Equipment",active->getEquipment(*at)->name()))
               deadKit.append(active->getEquipment(*at));
            break;
         case BrewTargetTreeItem::FERMENTABLE:
            if (*at != active->findFermentable(0) && verifyDelete("Fermentable",active->getFermentable(*at)->name()))
               deadFerm.append(active->getFermentable(*at));
            break;
         case BrewTargetTreeItem::HOP:
            if (*at != active->findHop(0) && verifyDelete("Hop",active->getHop(*at)->name()))
               deadHop.append(active->getHop(*at));
            break;
         case BrewTargetTreeItem::MISC:
            if (*at != active->findMisc(0) && verifyDelete("Misc",active->getMisc(*at)->name()))
               deadMisc.append(active->getMisc(*at));
            break;
         case BrewTargetTreeItem::YEAST:
            if (*at != active->findYeast(0) && verifyDelete("Yeast",active->getYeast(*at)->name()))
               deadYeast.append(active->getYeast(*at));
            break;
         case BrewTargetTreeItem::BREWNOTE:
            if (verifyDelete("BrewNote",active->getBrewNote(*at)->brewDate_short()))
               deadNote.append(active->getBrewNote(*at));
            break;
         default:
            Brewtarget::log(Brewtarget::WARNING, QString("MainWindow::deleteSelected Unknown type: %1").arg(treeView_recipe->getType(*at)));
      }
      if ( confirmDelete == QMessageBox::Cancel )
         return;
   }

   // Deleting brewnotes is kind of annoying, actually.  But do it before you
   // delete recipes.  Unpleasant things will happen... I really want to
   // isolate this so it looks as clean as the others do.
   for (int i = 0; i < deadNote.count(); ++i)
   {
      BrewNoteWidget* ni = brewNotes.value(deadNote.at(i)->key());
      Recipe* rec = Database::instance().getParentRecipe(deadNote.at(i));
      int numtab = tabWidget_recipeView->indexOf(ni);  

      // remove it from the recipe and from our internal tracking.
      rec->removeBrewNote(deadNote.at(i));
      brewNotes.remove(deadNote.at(i)->key());

      if ( numtab > 2 )
         tabWidget_recipeView->removeTab(numtab);
   }

   Database::instance().removeRecipe(deadRec);
   Database::instance().removeEquipment(deadKit);
   Database::instance().removeFermentable(deadFerm);
   Database::instance().removeHop(deadHop);
   Database::instance().removeMisc(deadMisc);
   Database::instance().removeYeast(deadYeast);

   first = active->getFirst();
   if ( first.isValid() )
   {
      if (active->getType(first) == BrewTargetTreeItem::RECIPE)
         setRecipeByIndex(first);
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

   QObject* calledBy = sender();
   BrewTargetTreeView* active;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == 0 )
      return;

   active = qobject_cast<BrewTargetTreeView*>(calledBy);

   // If the sender cannot be morphed into a BrewTargetTreeView object
   if ( active == 0 )
      return;

   switch( active->getType(index))
   {
      case BrewTargetTreeItem::RECIPE:
         setRecipeByIndex(index);
         break;
      case BrewTargetTreeItem::EQUIPMENT:
         kit = active->getEquipment(index);
         if ( kit )
         {
            singleEquipEditor->setEquipment(kit);
            singleEquipEditor->show();
         }
         break;
      case BrewTargetTreeItem::FERMENTABLE:
         ferm = active->getFermentable(index);
         if ( ferm )
         {
            fermEditor->setFermentable(ferm);
            fermEditor->show();
         }
         break;
      case BrewTargetTreeItem::HOP:
         h = active->getHop(index);
         if (h)
         {
            hopEditor->setHop(h);
            hopEditor->show();
         }
         break;
      case BrewTargetTreeItem::MISC:
         m = active->getMisc(index);
         if (m)
         {
            miscEditor->setMisc(m);
            miscEditor->show();
         }
         break;
      case BrewTargetTreeItem::YEAST:
         y = active->getYeast(index);
         if (y)
         {
            yeastEditor->setYeast(y);
            yeastEditor->show();
         }
         break;
      case BrewTargetTreeItem::BREWNOTE:
         setBrewNoteByIndex(index);
         break;
      default:
         Brewtarget::log(Brewtarget::WARNING, QString("MainWindow::treeActivated Unknown type %1.").arg(treeView_recipe->getType(index)));
   }
   treeView_recipe->setCurrentIndex(index);
}

void MainWindow::setBrewNoteByIndex(const QModelIndex &index)
{
   BrewNoteWidget* ni;

   BrewNote* bNote = treeView_recipe->getBrewNote(index);

   if ( ! bNote )
      return;

   Recipe* parent  = Database::instance().getParentRecipe(bNote);
   // I think this means a brew note for a different recipe has been selected.
   // We need to select that recipe, which will clear the current tabs
   if (  parent != recipeObs )
   {
      setRecipe(parent);
   }
   else if (brewNotes.contains(bNote->key()))
   {
      tabWidget_recipeView->setCurrentWidget(brewNotes.value(bNote->key()));
      return;
   }

   ni = new BrewNoteWidget(tabWidget_recipeView);
   ni->setBrewNote(bNote);

   tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
   brewNotes.insert(bNote->key(), ni);
   tabWidget_recipeView->setCurrentWidget(ni);

}

void MainWindow::setBrewNote(BrewNote* bNote)
{
   QString tabname;
   BrewNoteWidget* ni;

   if (brewNotes.contains(bNote->key()))
   {
      ni = brewNotes.value(bNote->key());
      tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   ni = new BrewNoteWidget(tabWidget_recipeView);
   ni->setBrewNote(bNote);

   brewNotes.insert(bNote->key(), ni);
   tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
   tabWidget_recipeView->setCurrentWidget(ni);
}

void MainWindow::setRecipeByIndex(const QModelIndex &index)
{
   Recipe *rec = treeView_recipe->getRecipe(index);
   if( rec )
      setRecipe(rec);
}

// Can handle null recipes.
void MainWindow::setRecipe(Recipe* recipe)
{
   // Don't like void pointers.
   if( recipe == 0 )
      return;

   int startTab;
   QHashIterator<int,BrewNoteWidget*> b(brewNotes);

   // Make sure this MainWindow is paying attention...
   if( recipeObs )
      disconnect( recipeObs, 0, this, 0 );
   recipeObs = recipe;
   
   recStyle = recipe->style();
   recEquip = recipe->equipment();
   
   // BeerXML is stupid and has reduntant fields.
   // Reset all previous recipe shit.
   fermTableModel->observeRecipe(recipe);
   hopTableModel->observeRecipe(recipe);
   miscTableModel->observeRecipe(recipe);
   yeastTableModel->observeRecipe(recipe);
   mashStepTableModel->setMash(recipeObs->mash());

   // Clean out any brew notes
   tabWidget_recipeView->setCurrentIndex(0);
   startTab = tabWidget_recipeView->count() - brewNotes.size();

   while( b.hasNext() )
   {
      b.next();
      tabWidget_recipeView->removeTab(startTab);
   }
   brewNotes.clear();
  
   // Tell some of our other widgets to observe the new recipe.
   mashWizard->setRecipe(recipe);
   brewDayScrollWidget->setRecipe(recipe);
   //recipeStyleNameButton->setRecipe(recipe);
   equipmentListModel->observeRecipe(recipe);
   maltWidget->observeRecipe(recipe);
   beerColorWidget->setRecipe(recipe);
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
   
   lineEdit_name->setText(recipeObs->name());
   lineEdit_batchSize->setText(Brewtarget::displayAmount(recipeObs, tab_recipe, "batchSize_l", Units::liters));
   lineEdit_boilSize->setText(Brewtarget::displayAmount(recipeObs, tab_recipe, "boilSize_l", Units::liters));
   lineEdit_efficiency->setText(Brewtarget::displayAmount(recipeObs, tab_recipe, "efficiency_pct", 0,0));
   lineEdit_boilTime->setText(Brewtarget::displayAmount(recipeObs, tab_recipe, "boilTime_min", Units::minutes));
   lineEdit_name->setCursorPosition(0);
   lineEdit_batchSize->setCursorPosition(0);
   lineEdit_boilSize->setCursorPosition(0);
   lineEdit_efficiency->setCursorPosition(0);
   lineEdit_boilTime->setCursorPosition(0);
   
   label_calcBatchSize->setText(Brewtarget::displayAmount(recipeObs,tab_recipe, "finalVolume_l", Units::liters));
   label_calcBoilSize->setText(Brewtarget::displayAmount(recipeObs, tab_recipe, "boilVolume_l", Units::liters));
   
   // Color manipulation
   if( 0.95*recipeObs->batchSize_l() <= recipeObs->finalVolume_l() && recipeObs->finalVolume_l() <= 1.05*recipeObs->batchSize_l() )
      label_calcBatchSize->setPalette(lcdPalette_good);
   else if( recipeObs->finalVolume_l() < 0.95*recipeObs->batchSize_l() )
      label_calcBatchSize->setPalette(lcdPalette_tooLow);
   else
      label_calcBatchSize->setPalette(lcdPalette_tooHigh);
   if( 0.95*recipeObs->boilSize_l() <= recipeObs->boilVolume_l() && recipeObs->boilVolume_l() <= 1.05*recipeObs->boilSize_l() )
      label_calcBoilSize->setPalette(lcdPalette_good);
   else if( recipeObs->boilVolume_l() < 0.95* recipeObs->boilSize_l() )
      label_calcBoilSize->setPalette(lcdPalette_tooLow);
   else
      label_calcBoilSize->setPalette(lcdPalette_tooHigh);

   QPair<QString, BeerXMLElement*> fg("fg",recipeObs);
   QPair<QString, BeerXMLElement*> og("og", recipeObs);
   
   // Want to do some manipulation based on selected style.
   if( recStyle != 0 )
   {
      lcdNumber_ogLow->display(Brewtarget::displayOG(recStyle, tab_recipe, "ogMin",false));
      lcdNumber_og->setLowLim(Brewtarget::displayOG(recStyle, tab_recipe, "ogMin",false).toDouble());
      
      lcdNumber_ogHigh->display(Brewtarget::displayOG(recStyle,tab_recipe, "ogMax",false));
      lcdNumber_og->setHighLim(Brewtarget::displayOG(recStyle,tab_recipe, "ogMax",false).toDouble());
      
      // 
      fg.first = "fgMin";
      fg.second = recStyle;
      lcdNumber_fgLow->display(Brewtarget::displayFG(fg,og,tab_recipe,false));
      lcdNumber_fg->setLowLim(Brewtarget::displayFG(fg,og,tab_recipe,false).toDouble());

      fg.first = "fgMax";
      lcdNumber_fgHigh->display(Brewtarget::displayFG(fg,og,tab_recipe,false));
      lcdNumber_fg->setHighLim(Brewtarget::displayFG(fg,og,tab_recipe,false).toDouble());

      lcdNumber_abvLow->display(recStyle->abvMin_pct(), 1);
      lcdNumber_abvHigh->display(recStyle->abvMax_pct(), 1);
      lcdNumber_abv->setLowLim(recStyle->abvMin_pct());
      lcdNumber_abv->setHighLim(recStyle->abvMax_pct());

      lcdNumber_ibuLow->display(recStyle->ibuMin(), 1);
      lcdNumber_ibuHigh->display(recStyle->ibuMax(), 1);
      lcdNumber_ibu->setLowLim(recStyle->ibuMin());
      lcdNumber_ibu->setHighLim(recStyle->ibuMax());

      lcdNumber_srmLow->display(Brewtarget::displayColor(recStyle, tab_recipe, "colorMin_srm", false));
      lcdNumber_srmHigh->display(Brewtarget::displayColor(recStyle, tab_recipe, "colorMax_srm", false));
      lcdNumber_srm->setLowLim(Brewtarget::displayColor(recStyle, tab_recipe, "colorMin_srm", false).toDouble());
      lcdNumber_srm->setHighLim(Brewtarget::displayColor(recStyle, tab_recipe, "colorMax_srm", false).toDouble());
   }

   lcdNumber_og->display(Brewtarget::displayOG(recipeObs,tab_recipe,"og",false));
   lcdNumber_boilSG->display(Brewtarget::displayOG(recipeObs,tab_recipe,"boilGrav",false));

   lcdNumber_fg->display(Brewtarget::displayFG(recipeObs->fg(), recipeObs->og()));

   lcdNumber_abv->display(recipeObs->ABV_pct(), 1);
   lcdNumber_ibu->display(recipeObs->IBU(), 1);
   lcdNumber_srm->display(Brewtarget::displayColor(recipeObs,tab_recipe,"color_srm",false));
   lcdNumber_ibugu->display(recipeObs->IBU()/((recipeObs->og()-1)*1000), 2);
   lcdNumber_calories->display( recipeObs->calories(), 0);

   // See if we need to change the mash in the table.
   if( (updateAll && recipeObs->mash()) ||
       (propName == "mash" &&
       recipeObs->mash()) )
   {
      mashStepTableModel->setMash(recipeObs->mash());
   }
}

void MainWindow::updateRecipeName()
{
   if( recipeObs == 0 )
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
   if( recipeObs == 0 )
      return;

   // equip may be null.
   Equipment* equip = equipmentListModel->at(equipmentComboBox->currentIndex());
   if( equip == 0 )
      return;

   // Notice that we are using a copy from the database.
   Database::instance().addToRecipe(recipeObs,equip);
   equipmentButton->setEquipment(equip);

   // Keep the mash tun weight and specific heat up to date.
   Mash* m = recipeObs->mash();
   if( m )
   {
      m->setTunWeight_kg( equip->tunWeight_kg() );
      m->setTunSpecificHeat_calGC( equip->tunSpecificHeat_calGC() );
   }
   
   if( QMessageBox::question(this, tr("Equipment request"),
                             tr("Would you like to set the batch and boil size to that requested by the equipment?"),
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
   {
      recipeObs->setBatchSize_l( equip->batchSize_l() );
      recipeObs->setBoilSize_l( equip->boilSize_l() );
      recipeObs->setBoilTime_min( equip->boilTime_min() );
      mashEditor->setEquipment(equip);
   }
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

void MainWindow::updateRecipeBatchSize()
{
   unitDisplay dispUnit;
   if( recipeObs == 0 )
      return;
   
   dispUnit  = (unitDisplay)Brewtarget::option("batchsize_L", noUnit,tab_recipe,Brewtarget::UNIT).toInt();
   recipeObs->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text(),dispUnit) );
}

void MainWindow::updateRecipeBoilSize()
{
   unitDisplay dispUnit;
   if( recipeObs == 0 )
      return;
 
   dispUnit  = (unitDisplay)Brewtarget::option("boilsize_L", noUnit,tab_recipe,Brewtarget::UNIT).toInt();
   recipeObs->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text(), dispUnit) );
}

void MainWindow::updateRecipeBoilTime()
{
   double boilTime = 0.0;
   Equipment* kit;

   if( recipeObs == 0 )
      return;
 
   kit = recipeObs->equipment();
   boilTime = Brewtarget::timeQStringToSI( lineEdit_boilTime->text() );
   
   // Here, we rely on a signa/slot connection to propagate the equipment
   // changes to recipeObs->boilTime_min and maybe recipeObs->boilSize_l
   if( kit )
      kit->setBoilTime_min(boilTime);
   else
      recipeObs->setBoilTime_min(boilTime);
}

void MainWindow::updateRecipeEfficiency()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setEfficiency_pct( lineEdit_efficiency->text().toDouble() );
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
   Fermentable* f = selectedFermentable();
   if( f == 0 )
      return;

   fermTableModel->removeFermentable(f);
   recipeObs->removeFermentable(f);
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
   Hop* hop = selectedHop();
   if( hop == 0 )
      return;

   hopTableModel->removeHop(hop);
   recipeObs->removeHop(hop);
}

void MainWindow::removeSelectedMisc()
{
   Misc* misc = selectedMisc();
   if( misc == 0 )
      return;

   miscTableModel->removeMisc(misc);
   recipeObs->removeMisc(misc);
}

void MainWindow::removeSelectedYeast()
{
   Yeast* yeast = selectedYeast();
   if( yeast == 0 )
      return;

   yeastTableModel->removeYeast(yeast);
   recipeObs->removeYeast(yeast);
}

void MainWindow::newRecipe()
{
   QString name = QInputDialog::getText(this, tr("Recipe name"),
                                          tr("Recipe name:"));
   QVariant defEquipKey = Brewtarget::option("defaultEquipmentKey", -1); 
   if( name.isEmpty() )
      return;

   Recipe* newRec = Database::instance().newRecipe();

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

   setTreeSelection(treeView_recipe->findRecipe(newRec));
   setRecipe(newRec);
}

void MainWindow::setTreeSelection(QModelIndex item)
{
   BrewTargetTreeView *active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   if (! item.isValid())
      return;

   if ( active == 0 )
      active = qobject_cast<BrewTargetTreeView*>(treeView_recipe);

   // Couldn't cast the active item to a brwetargettreeview
   if ( active == 0 )
      return;

   QModelIndex parent = active->getParent(item);

   active->setCurrentIndex(item);
   active->scrollTo(item,QAbstractItemView::PositionAtCenter);
//   treeView_recipe->expand(parent);
   
}

// Need to make sure the recipe tree is active, I think
void MainWindow::newBrewNote()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   QModelIndex bIndex;

   foreach(QModelIndex selected, indexes)
   {
      Recipe*   rec   = treeView_recipe->getRecipe(selected);
      QModelIndex newItem;

      if( rec == 0 )
         continue;


      // Make sure everything is properly set and selected
      if( rec != recipeObs )
         setRecipe(rec);

//      BrewNote* bNote = rec->addBrewNote();
      BrewNote* bNote = Database::instance().newBrewNote(rec);
      bNote->populateNote(rec);
      bNote->setBrewDate();

      setBrewNote(bNote);

      bIndex = treeView_recipe->findBrewNote(bNote);
      if ( bIndex.isValid() )
         setTreeSelection(bIndex);
   }
}
  
void MainWindow::reBrewNote()
{
   QModelIndexList indexes = treeView_recipe->selectionModel()->selectedRows();
   foreach(QModelIndex selected, indexes)
   {
      BrewNote* old   = treeView_recipe->getBrewNote(selected);
      Recipe* rec     = treeView_recipe->getRecipe(treeView_recipe->getParent(selected));

      if (! old || ! rec)
         return;

//      BrewNote* bNote = rec->addBrewNote(old);
      BrewNote* bNote = Database::instance().newBrewNote(old);
      bNote->setBrewDate();

      if (rec != recipeObs)
         setRecipe(rec);

      setBrewNote(bNote);

      setTreeSelection(treeView_recipe->findBrewNote(bNote));
   }
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
      Database::instance().importFromXML(filename);
   
   showChanges();
}

bool MainWindow::verifyImport(QString tag, QString name)
{
   return QMessageBox::question(this, tr("Import %1?").arg(tag), tr("Import %1?").arg(name),
                                QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

bool MainWindow::verifyDelete(QString tag, QString name)
{
   if ( confirmDelete == QMessageBox::YesToAll )
      return true;

   confirmDelete = QMessageBox::question(this, tr("Delete %1").arg(tag), tr("Delete %1 %2?").arg(tag).arg(name),
                                  QMessageBox::Yes | QMessageBox::YesToAll | QMessageBox::No | QMessageBox::Cancel,
                                  QMessageBox::No);

   return (confirmDelete == QMessageBox::Yes || confirmDelete ==  QMessageBox::YesToAll); 
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
   if( ! ( recipeObs || recipeObs->mash() ) )
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
   Database::instance().removeMash(m);
   
   Mash* defaultMash = Database::instance().newMash(recipeObs);
   mashStepTableModel->setMash(defaultMash);
   
   //remove from combobox handled automatically by qt
   mashButton->setMash(defaultMash);

}

void MainWindow::closeEvent(QCloseEvent* /*event*/)
{
   // TODO: anything to do here?
   /*
   if( QMessageBox::question(this, tr("Save database?"),
                             tr("Do you want to save the changes made? If not, you will lose anything you changed in this session."),
                             QMessageBox::Yes,
                             QMessageBox::No)
       == QMessageBox::Yes )
   {
      Database::savePersistent();
   }
   */
   
   Brewtarget::savePersistentOptions();

   Brewtarget::btSettings.setValue("geometry", saveGeometry());
   Brewtarget::btSettings.setValue("windowState", saveState());
   if ( recipeObs )
      Brewtarget::btSettings.setValue("recipeKey", recipeObs->key());
   setVisible(false);
}

void MainWindow::copyRecipe()
{
   QString name = QInputDialog::getText( this, tr("Copy Recipe"), tr("Enter a unique name for the copy.") );
   
   if( name.isEmpty() )
      return;
   
   Recipe* newRec = Database::instance().newRecipe(recipeObs); // Create a deep copy.
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

void MainWindow::openDonateLink()
{
   QDesktopServices::openUrl(QUrl("http://sourceforge.net/project/project_donations.php?group_id=249733"));
}

// One print function to rule them all. Now we just need to make the menuing
// system make sense
void MainWindow::print()
{
   QObject* selection = sender();

   if ( selection == actionRecipePrint || selection == actionBrewdayPrint )
   {
      QPrintDialog printerDialog(printer, this);
      selection == actionRecipePrint ?  recipeFormatter->print( printer, &printerDialog, RecipeFormatter::PRINT) :
                                        brewDayScrollWidget->print( printer, &printerDialog, BrewDayScrollWidget::PRINT);
   }
   else if ( selection == actionRecipePreview )
   {
      recipeFormatter->print(printer, 0, RecipeFormatter::PREVIEW);
   }
   else if ( selection == actionBrewdayPreview )
   {
      brewDayScrollWidget->print(printer, 0, RecipeFormatter::PREVIEW);
   }
   else if ( selection == actionRecipeHTML || selection == actionBrewdayHTML)
   {
      QFile* outfile = openForWrite(tr("HTML files (*.html)"), QString("html"));

      if (! outfile )
         return;
      selection == actionRecipeHTML ? recipeFormatter->print(printer, 0, RecipeFormatter::HTML, outfile) : 
                                      brewDayScrollWidget->print(printer, 0, BrewDayScrollWidget::HTML, outfile);
      delete outfile;
   }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
   if (event->mimeData()->hasFormat("application/x-brewtarget"))
      event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
   QModelIndexList indexes;
   QWidget *last = 0;
   QString name;
   BrewTargetTreeView* active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
   int type;

   // Check that the recipe isn't a null pointer.
   if (recipeObs == 0)
      return;

   // If the sender cannot be morphed into a BrewTargetTreeView object
   if ( active == 0 )
      return;

   if (! event->mimeData()->hasFormat("application/x-brewtarget"))
      return;

   indexes = active->selectionModel()->selectedRows();

   foreach(QModelIndex index, indexes)
   {
      if ( index.isValid() )
      {
         type = active->getType(index);
         switch(type)
         {
            case BrewTargetTreeItem::RECIPE:
               setRecipeByIndex(index);
               break;
            case BrewTargetTreeItem::EQUIPMENT:
               //equipmentComboBox->setCurrentIndex(equipmentListModel->indexOf(active->getEquipment(index)));
               //NOTE: is the following right?
               //equipmentComboBox->setText(active->getEquipment(index)->name());
               droppedRecipeEquipment(active->getEquipment(index));
               break;
            // NOTE: addToRecipe() calls the appropriate new* under the covers. Calling it twice caused some odd problems
            case BrewTargetTreeItem::FERMENTABLE:
               Database::instance().addToRecipe( recipeObs, active->getFermentable(index) );
               last = fermentableTab;
               break;
            case BrewTargetTreeItem::HOP:
               Database::instance().addToRecipe( recipeObs, active->getHop(index));
               last = hopsTab;
               break;
            case BrewTargetTreeItem::MISC:
               Database::instance().addToRecipe( recipeObs, active->getMisc(index) );
               last = miscTab;
               break;
            case BrewTargetTreeItem::YEAST:
               Database::instance().addToRecipe( recipeObs, active->getYeast(index) );
               last = yeastTab;
               break;
            case BrewTargetTreeItem::BREWNOTE:
               setBrewNoteByIndex(index);
               break;
         }
         event->accept();
      }
   }
   if (last)
      tabWidget->setCurrentWidget(last);
}

// We build the menus at start up time.  This just needs to exec the proper
// menu.  
void MainWindow::contextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   BrewTargetTreeView* active;
   QModelIndex selected;
   QMenu* tempMenu;

   // Not sure how this could happen, but better safe the sigsegv'd
   if ( calledBy == 0 )
      return;

   active = qobject_cast<BrewTargetTreeView*>(calledBy);

   // If the sender cannot be morphed into a BrewTargetTreeView object
   if ( active == 0 )
      return;

   selected = active->indexAt(point);
   if (! selected.isValid())
      return;

   tempMenu = active->getContextMenu(selected);

   if (tempMenu)
      tempMenu->exec(active->mapToGlobal(point));
}

void MainWindow::setupContextMenu()
{
   QMenu *sMenu = new QMenu(this);

   // Set up the "new" submenu
   sMenu->setTitle(tr("New"));
   sMenu->addAction(tr("Recipe"), this, SLOT(newRecipe()));
   sMenu->addAction(tr("Equipment"), equipEditor, SLOT(newEquipment()));
   sMenu->addAction(tr("Fermentable"), fermDialog, SLOT(newFermentable()));
   sMenu->addAction(tr("Hop"), hopDialog, SLOT(newHop()));
   sMenu->addAction(tr("Miscellaneous"), miscDialog, SLOT(newMisc()));
   sMenu->addAction(tr("Yeast"), yeastDialog, SLOT(newYeast()));

   treeView_recipe->setupContextMenu(this,this,sMenu,BrewTargetTreeItem::RECIPE);
   treeView_equip->setupContextMenu(this,equipEditor,sMenu,BrewTargetTreeItem::EQUIPMENT);

   treeView_ferm->setupContextMenu(this,fermDialog,sMenu,BrewTargetTreeItem::FERMENTABLE);
   treeView_hops->setupContextMenu(this,hopDialog,sMenu,BrewTargetTreeItem::HOP);
   treeView_misc->setupContextMenu(this,miscDialog,sMenu,BrewTargetTreeItem::MISC);
   treeView_yeast->setupContextMenu(this,yeastDialog,sMenu,BrewTargetTreeItem::YEAST);


}

void MainWindow::copyThis(Recipe *rec)
{
   QString name = QInputDialog::getText( this, tr("Copy %1").arg(rec->name()), tr("Enter a unique name for the copy of %1.").arg(rec->name()) );

   if( name.isEmpty() )
      return;

   Recipe* newRec = Database::instance().newRecipe(rec); // Create a deep copy.
   newRec->setName(name);
}

void MainWindow::copyThis(Equipment *kit)
{
   QString name = QInputDialog::getText( this, tr("Copy Equipment"), tr("Enter a unique name for the copy of %1.").arg(kit->name()) );

   if( name.isEmpty() )
      return;

   Equipment* newKit = Database::instance().newEquipment(kit); // Create a deep copy.
   newKit->setName(name);
}

void MainWindow::copyThis(Fermentable *ferm)
{
   QString name = QInputDialog::getText( this, tr("Copy Fermentable"), tr("Enter a unique name for the copy of %1.").arg(ferm->name()) );

   if( name.isEmpty() )
      return;

   Fermentable* newFerm = Database::instance().newFermentable(ferm); // Create a deep copy.
   newFerm->setName(name);
}

void MainWindow::copyThis(Hop *hop)
{
   QString name = QInputDialog::getText( this, tr("Copy Hop"), tr("Enter a unique name for the copy of %1.").arg(hop->name()));

   if( name.isEmpty() )
      return;

   Hop* newHop = Database::instance().newHop(hop); // Create a deep copy.
   newHop->setName(name);
}

void MainWindow::copyThis(Misc *misc)
{
   QString name = QInputDialog::getText( this, tr("Copy Miscellaneous"), tr("Enter a unique name for the copy of %1.").arg(misc->name()) );

   if( name.isEmpty() )
      return;

   Misc* newMisc = Database::instance().newMisc(misc); // Create a deep copy.
   newMisc->setName(name);
}

void MainWindow::copyThis(Yeast *yeast)
{
   QString name = QInputDialog::getText( this, tr("Copy Yeast"), tr("Enter a unique name for the copy of %1.").arg(yeast->name()) );

   if( name.isEmpty() )
      return;

   Yeast* newYeast = Database::instance().newYeast(yeast); // Create a deep copy.
   newYeast->setName(name);
}

void MainWindow::copySelected()
{
   BrewTargetTreeView* active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   QList<QModelIndex>::const_iterator at, end;
   QModelIndex above;
   QList<Recipe*> copyRec;
   QList<Equipment*> copyKit;
   QList<Fermentable*> copyFerm;
   QList<Hop*> copyHop;
   QList<Misc*> copyMisc;
   QList<Yeast*> copyYeast;

   if ( active == 0 )
      return;

   const QModelIndexList selected = active->selectionModel()->selectedRows();

   // We need to process them all before we get the names, because adding new things does mess
   // up the indexes.  This ... is not gonna be pretty.
   for(at = selected.begin(),end = selected.end();at < end;++at)
   {
      switch(active->getType(*at))
      {
         case BrewTargetTreeItem::RECIPE:
            if ( *at == active->findRecipe(0) )
               continue;
            copyRec.append(active->getRecipe(*at));
            break;
         case BrewTargetTreeItem::EQUIPMENT:
            if ( *at == active->findEquipment(0) )
               continue;
            copyKit.append(active->getEquipment(*at));
            break;
         case BrewTargetTreeItem::FERMENTABLE:
            if ( *at == active->findFermentable(0) )
               continue;
            copyFerm.append(active->getFermentable(*at));
            break;
         case BrewTargetTreeItem::HOP:
            if ( *at == active->findHop(0) )
               continue;
            copyHop.append(active->getHop(*at));
            break;
         case BrewTargetTreeItem::MISC:
            if ( *at == active->findMisc(0) )
               continue;
            copyMisc.append(active->getMisc(*at));
            break;
         case BrewTargetTreeItem::YEAST:
            if ( *at == active->findYeast(0) )
               continue;
            copyYeast.append(active->getYeast(*at));
            break;
            // No Brewnote, because it just doesn't make sense
         default:
            Brewtarget::log(Brewtarget::WARNING, QString("MainWindow::copySelected Unknown type: %1").arg(active->getType(*at)));
      }
   }

   for(int i = 0; i < copyRec.count(); ++i)
      copyThis(copyRec.at(i));
   for(int i = 0; i < copyKit.count(); ++i)
      copyThis(copyKit.at(i));
   for(int i = 0; i < copyFerm.count(); ++i)
      copyThis(copyFerm.at(i));
   for(int i = 0; i < copyMisc.count(); ++i)
      copyThis(copyMisc.at(i));
   for(int i = 0; i < copyYeast.count(); ++i)
      copyThis(copyYeast.at(i));


   above = active->getFirst();
   if ( active->getType(above) == BrewTargetTreeItem::RECIPE )
      setRecipeByIndex(above);
   setTreeSelection(above);
}

QFile* MainWindow::openForWrite( QString filterStr, QString defaultSuff)
{
   const char* filename;
   QFile* outFile = new QFile();

   fileSaver->setFilter( filterStr );
   fileSaver->setDefaultSuffix( defaultSuff );

   if( fileSaver->exec() )
   {
      filename = fileSaver->selectedFiles()[0].toAscii();
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

void MainWindow::exportSelected()
{
   BrewTargetTreeView* active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());
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
      int type = active->getType(selection);

      switch(type)
      {
         case BrewTargetTreeItem::RECIPE:
            Database::instance().toXml( treeView_recipe->getRecipe(selection), doc, recipe);
            didRecipe = true;
            break;
         case BrewTargetTreeItem::EQUIPMENT:
            Database::instance().toXml( treeView_equip->getEquipment(selection), doc, dbase);
            break;
         case BrewTargetTreeItem::FERMENTABLE:
            Database::instance().toXml( treeView_ferm->getFermentable(selection), doc, dbase);
            break;
         case BrewTargetTreeItem::HOP:
            Database::instance().toXml( treeView_hops->getHop(selection), doc, dbase);
            break;
         case BrewTargetTreeItem::MISC:
            Database::instance().toXml( treeView_misc->getMisc(selection), doc, dbase);
            break;
         case BrewTargetTreeItem::YEAST:
            Database::instance().toXml( treeView_yeast->getYeast(selection), doc, dbase);
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
                                           Brewtarget::getUserDataDir(),
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

void MainWindow::redisplayLabel(QString field)
{
   // There is a lot of magic going on in the showChanges(). I can either
   // duplicate that magic or I can just call showChanges().
   showChanges();
}

void MainWindow::fermentableContextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   unitDisplay currentUnit;
   unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = fermTableModel->displayUnit(selected);
   currentScale = fermTableModel->displayScale(selected);
   
   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case FERMAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(this,currentUnit, currentScale); 
         break;
      case FERMCOLORCOL:
         menu = Brewtarget::setupColorMenu(this,currentUnit); 
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
      fermTableModel->setDisplayUnit(selected,(unitDisplay)invoked->data().toInt());
   else
      fermTableModel->setDisplayScale(selected,(unitScale)invoked->data().toInt());

   showChanges();
}

void MainWindow::hopContextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   unitDisplay currentUnit;
   unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = hopTableModel->displayUnit(selected);
   currentScale = hopTableModel->displayScale(selected);
   
   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case HOPAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(this,currentUnit, currentScale); 
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
      hopTableModel->setDisplayUnit(selected,(unitDisplay)invoked->data().toInt());
   else
      hopTableModel->setDisplayScale(selected,(unitScale)invoked->data().toInt());

   showChanges();
}

void MainWindow::mashStepContextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   unitDisplay currentUnit;
   unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = mashStepTableModel->displayUnit(selected);
   currentScale = mashStepTableModel->displayScale(selected);
   
   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case MASHSTEPAMOUNTCOL:
         menu = Brewtarget::setupVolumeMenu(this,currentUnit, currentScale); 
         break;
      case MASHSTEPTEMPCOL:
      case MASHSTEPTARGETTEMPCOL:
         menu = Brewtarget::setupTemperatureMenu(this,currentUnit);
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   QWidget* pMenu = invoked->parentWidget();
   if ( pMenu == menu )
      mashStepTableModel->setDisplayUnit(selected,(unitDisplay)invoked->data().toInt());
   else
      mashStepTableModel->setDisplayScale(selected,(unitScale)invoked->data().toInt());

   showChanges();
}

void MainWindow::miscContextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   unitDisplay currentUnit;
   unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = miscTableModel->displayUnit(selected);
   currentScale = miscTableModel->displayScale(selected);
   
   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case MISCAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(this,currentUnit, currentScale, false); 
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   miscTableModel->setDisplayUnit(selected,(unitDisplay)invoked->data().toInt());

   showChanges();
}

void MainWindow::yeastContextMenu(const QPoint &point)
{
   QObject* calledBy = sender();
   QHeaderView* hView = qobject_cast<QHeaderView*>(calledBy);

   int selected = hView->logicalIndexAt(point);
   unitDisplay currentUnit;
   unitScale  currentScale;

   // Since we need to call generateVolumeMenu() two different ways, we need
   // to figure out the currentUnit and Scale here

   currentUnit  = yeastTableModel->displayUnit(selected);
   currentScale = yeastTableModel->displayScale(selected);
   
   QMenu* menu;
   QAction* invoked;

   switch(selected)
   {
      case YEASTAMOUNTCOL:
         menu = Brewtarget::setupMassMenu(this,currentUnit, currentScale, false); 
         break;
      default:
         return;
   }

   invoked = menu->exec(hView->mapToGlobal(point));
   if ( invoked == 0 )
      return;

   yeastTableModel->setDisplayUnit(selected,(unitDisplay)invoked->data().toInt());

   showChanges();
}

void MainWindow::showPitchDialog()
{
   // First, copy the current recipe og and volume.
   if( recipeObs )
   {
      pitchDialog->setWortVolume_l( recipeObs->finalVolume_l() );
      pitchDialog->setWortGravity( recipeObs->og() );
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
   QDir dir(Brewtarget::getUserDataDir());

   msgBox.setText( tr("The database has been converted/upgraded."));
   msgBox.setInformativeText( tr("The original XML files can be found in ") + Brewtarget::getUserDataDir() + "obsolete");
   msgBox.exec();

}
   
