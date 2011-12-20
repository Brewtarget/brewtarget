/*
 * MainWindow.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include <iostream>
#include <fstream>
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
#include "MiscTableWidget.h"
#include "YeastTableWidget.h"
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
#include "MashComboBox.h"
#include "PrimingDialog.h"
#include "RefractoDialog.h"
#include "MashDesigner.h"
#include "PitchDialog.h"
#include "MaltinessWidget.h"
#include "fermentable.h"
#include "yeast.h"
#include "brewnote.h"
#include "equipment.h"

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

   // Make the fermentable table show grain percentages in row headers.
   fermentableTable->getModel()->setDisplayPercentages(true);
   // Hop table show IBUs in row headers.
   hopTable->getModel()->setShowIBUs(true);
   
   dialog_about = new AboutDialog(this);
   equipEditor = new EquipmentEditor(this);
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
   yeastDialog = new YeastDialog(this);
   yeastEditor = new YeastEditor(this);
   optionDialog = new OptionDialog(this);
   htmlViewer = new HtmlViewer(this);
   recipeScaler = new ScaleRecipeTool(this);
   recipeFormatter = new RecipeFormatter();
   ogAdjuster = new OgAdjuster();
   converterTool = new ConverterTool();
   timerListDialog = new TimerListDialog(this);
   mashComboBox = new MashComboBox(this);
   primingDialog = new PrimingDialog(this);
   refractoDialog = new RefractoDialog(this);
   mashDesigner = new MashDesigner(this);
   pitchDialog = new PitchDialog(this);

   // Enable sorting in the main tables.
   fermentableTable->setSortingEnabled(true);
   fermentableTable->sortByColumn( FERMAMOUNTCOL, Qt::DescendingOrder );
   hopTable->setSortingEnabled(true);
   hopTable->sortByColumn( HOPTIMECOL, Qt::DescendingOrder );
   miscTable->setSortingEnabled(true);
   miscTable->sortByColumn( MISCUSECOL, Qt::DescendingOrder );
   yeastTable->setSortingEnabled(true);
   yeastTable->sortByColumn( YEASTNAMECOL, Qt::DescendingOrder );

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
   verticalLayout_beerColor->insertWidget( -1, maltWidget );
   horizontalLayout_mash->insertWidget( 1, mashComboBox );

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
   QSettings settings("brewtarget");
   if ( settings.contains("geometry"))
   {
      restoreGeometry(settings.value("geometry").toByteArray());
      restoreState(settings.value("windowState").toByteArray());
   }
   else
   {
      // otherwise, guess a reasonable size at 1/4 of the screen.
      int width = desktop->width();
      int height = desktop->height();

      this->resize(width/2,height/2);
   }

   // If we saved the selected recipe name the last time we ran, select it and show it.
   if (settings.contains("recipeKey"))
   {
      int key = settings.value("recipeKey").toInt();
      recipeObs = Database::instance().recipe( key );

      setRecipe(recipeObs);
      setSelection(treeView_recipe->findRecipe(recipeObs));
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
   connect( actionPitch_Rate_Calculator, SIGNAL(triggered()), pitchDialog, SLOT(show()));
   connect( actionMergeDatabases, SIGNAL(triggered()), this, SLOT(mergeDatabases()) );
   connect( actionTimers, SIGNAL(triggered()), timerListDialog, SLOT(show()) );
   connect( actionDeleteSelected, SIGNAL(triggered()), this, SLOT(deleteSelected()) );
   connect( actionSave, SIGNAL(triggered()), this, SLOT(save()) );
   connect( actionClearRecipe, SIGNAL(triggered()), this, SLOT(clear()) );
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

   connect( equipmentComboBox, SIGNAL( activated(const QString&) ), this, SLOT(updateRecipeEquipment(const QString&)) );
   connect( mashComboBox, SIGNAL( currentIndexChanged(const QString&) ), this, SLOT(setMashToCurrentlySelected()) );
   //connect( styleComboBox, SIGNAL( activated(const QString&) ), this, SLOT(updateRecipeStyle(const QString&)) );
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
   connect( pushButton_editYeast, SIGNAL( clicked() ), this, SLOT( editSelectedYeast() ) );
   connect( pushButton_editMash, SIGNAL( clicked() ), mashEditor, SLOT( showEditor() ) );
   connect( pushButton_addMashStep, SIGNAL( clicked() ), this, SLOT(addMashStep()) );
   connect( pushButton_removeMashStep, SIGNAL( clicked() ), this, SLOT(removeSelectedMashStep()) );
   connect( pushButton_editMashStep, SIGNAL( clicked() ), this, SLOT(editSelectedMashStep()) );
   connect( pushButton_mashWizard, SIGNAL( clicked() ), mashWizard, SLOT( show() ) );
   connect( pushButton_saveMash, SIGNAL( clicked() ), this, SLOT( saveMash() ) );
   connect( pushButton_mashDes, SIGNAL( clicked() ), mashDesigner, SLOT( show() ) );
   connect( pushButton_mashUp, SIGNAL( clicked() ), mashStepTableWidget, SLOT( moveSelectedStepUp() ) );
   connect( pushButton_mashDown, SIGNAL( clicked() ), mashStepTableWidget, SLOT( moveSelectedStepDown() ) );
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
            Brewtarget::log(Brewtarget::WARNING, QObject::tr("MainWindow::deleteSelected Unknown type: %1").arg(treeView_recipe->getType(*at)));
      }
      if ( confirmDelete == QMessageBox::Cancel )
         return;
   }

   // Deleting brewnotes is kind of annoying, actually.  But do it before you
   // delete recipes.  Unpleasant things will happen... I really want to
   // isolate this so it looks as clean as the others do.
   for (int i = 0; i < deadNote.count(); ++i)
   {
      BrewNoteWidget* ni = brewNotes.value(deadNote.at(i)->brewDate_str());
      Recipe* rec = Database::instance().getParentRecipe(deadNote.at(i));
      int numtab = tabWidget_recipeView->indexOf(ni);

      rec->removeBrewNote(deadNote.at(i));
      tabWidget_recipeView->removeTab(numtab);
   }

   Database::instance().removeRecipe(deadRec);
   Database::instance().removeEquipment(deadKit);
   Database::instance().removeFermentable(deadFerm);
   Database::instance().removeHop(deadHop);
   Database::instance().removeMisc(deadMisc);
   Database::instance().removeYeast(deadYeast);

   // This is wrong. Since we have split the trees out, the selection should
   // be set to the top element of the active tree. I am just not sure how to
   // do it
   first = active->getFirst();
   if ( active->getType(first) == BrewTargetTreeItem::RECIPE)
      setRecipeByIndex(first);

   setSelection(first);
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
            equipEditor->setEquipment(kit);
            equipEditor->show();
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
         Brewtarget::log(Brewtarget::WARNING, tr("MainWindow::treeActivated Unknown type %1.").arg(treeView_recipe->getType(index)));
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
   else if (brewNotes.contains(bNote->brewDate_str()))
   {
      tabWidget_recipeView->setCurrentWidget(brewNotes.value(bNote->brewDate_str()));
      return;
   }

   ni = new BrewNoteWidget(tabWidget_recipeView);
   ni->setBrewNote(bNote);

   tabWidget_recipeView->addTab(ni,bNote->brewDate_short());
   brewNotes.insert(bNote->brewDate_str(), ni);
   tabWidget_recipeView->setCurrentWidget(ni);

}

void MainWindow::setBrewNote(BrewNote* bNote)
{
   QString tabname;
   BrewNoteWidget* ni;

   if (brewNotes.contains(bNote->brewDate_str()))
   {
      ni = brewNotes.value(bNote->brewDate_str());
      tabWidget_recipeView->setCurrentWidget(ni);
      return;
   }

   ni = new BrewNoteWidget(tabWidget_recipeView);
   ni->setBrewNote(bNote);

   brewNotes.insert(bNote->brewDate_str(), ni);
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
   Style* style;
   Equipment* equip;
   QHashIterator<QString,BrewNoteWidget*> b(brewNotes);

   // Make sure this MainWindow is paying attention...
   if( recipeObs )
      disconnect( recipeObs, 0, this, 0 );
   recipeObs = recipe;
   connect( recipeObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );
   
   style = recipe->style();
   equip = recipe->equipment();

   // Reset all previous recipe shit.
   fermentableTable->getModel()->observeRecipe(recipe);
   hopTable->getModel()->observeRecipe(recipe);
   miscTable->getModel()->observeRecipe(recipe);
   yeastTable->getModel()->observeRecipe(recipe);
   if( recipeObs->mash() != 0 )
      mashStepTableWidget->getModel()->setMash(recipeObs->mash());

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
   //styleComboBox->observeRecipe(recipe);
   //recipeStyleNameButton->setRecipe(recipe);
   equipmentComboBox->observeRecipe(recipe);
   maltWidget->observeRecipe(recipe);
   beerColorWidget->setRecipe(recipe);
   recipeFormatter->setRecipe(recipe);
   ogAdjuster->setRecipe(recipe);
   recipeExtrasWidget->setRecipe(recipe);
   mashDesigner->setRecipe(recipe);

   mashEditor->setMash(recipeObs->mash());
   recipeScaler->setRecipe(recipeObs);

   showChanges();
}

void MainWindow::changed(QMetaProperty prop, QVariant /*value*/)
{
   // Make sure the notifier is our observed recipe
   if( sender() != recipeObs )
      return;

   showChanges(&prop);
}

// This method should update all the widgets in the window (except the tables)
// to reflect the currently observed recipe.
void MainWindow::showChanges(QMetaProperty* prop)
{
   if( recipeObs == 0 )
      return;

   bool updateAll = (prop == 0);
   QString propName;
   //QVariant propVal;
   if( prop )
   {
      propName = prop->name();
      //propVal = prop->read( /*What here?*/ );
   }
   //recipeObs->recalculate();

   lineEdit_name->setText(recipeObs->name());
   lineEdit_name->setCursorPosition(0);
   lineEdit_batchSize->setText( Brewtarget::displayAmount(recipeObs->batchSize_l(), Units::liters) );
   lineEdit_boilSize->setText( Brewtarget::displayAmount(recipeObs->boilSize_l(), Units::liters) );
   lineEdit_efficiency->setText( Brewtarget::displayAmount(recipeObs->efficiency_pct(), 0) );
   
   label_calcBatchSize->setText( Brewtarget::displayAmount(recipeObs->finalVolume_l(), Units::liters) );
   label_calcBoilSize->setText( Brewtarget::displayAmount(recipeObs->boilVolume_l(), Units::liters) );
   
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

   lcdNumber_og->display(Brewtarget::displayOG(recipeObs->og()));
   lcdNumber_boilSG->display(Brewtarget::displayOG(recipeObs->boilGrav()));
   lcdNumber_fg->display(Brewtarget::displayFG(recipeObs->fg(), recipeObs->og()));
   lcdNumber_abv->display(recipeObs->ABV_pct(), 1);
   lcdNumber_ibu->display(recipeObs->IBU(), 1);
   lcdNumber_srm->display(Brewtarget::displayColor(recipeObs->color_srm(),false));
   lcdNumber_ibugu->display(recipeObs->IBU()/((recipeObs->og()-1)*1000), 2);
   lcdNumber_calories->display( recipeObs->calories(), 0);

   // Want to do some manipulation based on selected style.
   Style* recipeStyle = recipeObs->style();
   if( recipeStyle != 0 )
   {
      lcdNumber_ogLow->display(Brewtarget::displayOG(recipeStyle->ogMin()));
      lcdNumber_ogHigh->display(Brewtarget::displayOG(recipeStyle->ogMax()));
      lcdNumber_og->setLowLim(Brewtarget::displayOG(recipeStyle->ogMin()).toDouble());
      lcdNumber_og->setHighLim(Brewtarget::displayOG(recipeStyle->ogMax()).toDouble());

      lcdNumber_fgLow->display(Brewtarget::displayFG(recipeStyle->fgMin(), recipeObs->og()));
      lcdNumber_fgHigh->display(Brewtarget::displayFG(recipeStyle->fgMax(), recipeObs->og()));
      lcdNumber_fg->setLowLim(Brewtarget::displayFG(recipeStyle->fgMin(), recipeObs->og()).toDouble());
      lcdNumber_fg->setHighLim(Brewtarget::displayFG(recipeStyle->fgMax(), recipeObs->og()).toDouble());

      lcdNumber_abvLow->display(recipeStyle->abvMin_pct(), 1);
      lcdNumber_abvHigh->display(recipeStyle->abvMax_pct(), 1);
      lcdNumber_abv->setLowLim(recipeStyle->abvMin_pct());
      lcdNumber_abv->setHighLim(recipeStyle->abvMax_pct());

      lcdNumber_ibuLow->display(recipeStyle->ibuMin(), 1);
      lcdNumber_ibuHigh->display(recipeStyle->ibuMax(), 1);
      lcdNumber_ibu->setLowLim(recipeStyle->ibuMin());
      lcdNumber_ibu->setHighLim(recipeStyle->ibuMax());

      lcdNumber_srmLow->display(Brewtarget::displayColor(recipeStyle->colorMin_srm(), false));
      lcdNumber_srmHigh->display(Brewtarget::displayColor(recipeStyle->colorMax_srm(), false));
      lcdNumber_srm->setLowLim(Brewtarget::displayColor(recipeStyle->colorMin_srm(), false).toDouble());
      lcdNumber_srm->setHighLim(Brewtarget::displayColor(recipeStyle->colorMax_srm(), false).toDouble());
   }

   // See if we need to change the mash in the table.
   if( (updateAll && recipeObs->mash()) ||
       (propName == "mash" &&
       recipeObs->mash()) )
   {
      mashStepTableWidget->getModel()->setMash(recipeObs->mash());
   }
}

void MainWindow::save()
{
   // TODO: seems like we don't need this at all?
   //Database::savePersistent();
}

void MainWindow::clear()
{
   if( QMessageBox::question(this, tr("Sure about that?"),
                             tr("You are about to obliterate the recipe. Is that ok?"),
                             QMessageBox::Ok,
                             QMessageBox::No)
       == QMessageBox::Ok )
   {
      recipeObs->clear();
      setRecipe(recipeObs); // This will update tables and everything.
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
}

void MainWindow::updateRecipeEquipment(const QString& /*equipmentName*/)
{
   if( recipeObs == 0 )
      return;

   // equip may be null.
   Equipment* equip = equipmentComboBox->getSelected();
   if( equip == 0 )
      return;

   // Notice that we are using a copy from the database.
   Database::instance().addToRecipe(recipeObs,equip);

   if( QMessageBox::question(this,
                             tr("Equipment request"),
                             tr("Would you like to set the batch and boil size to that requested by the equipment?"),
                             QMessageBox::Yes,
                             QMessageBox::No)
        == QMessageBox::Yes
     )
   {
      if( recipeObs )
      {
         recipeObs->setBatchSize_l( equip->batchSize_l() );
         recipeObs->setBoilSize_l( equip->boilSize_l() );
         recipeObs->setBoilTime_min( equip->boilTime_min() );
      }
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

   if( QMessageBox::question(this,
                             tr("Equipment request"),
                             tr("Would you like to set the batch and boil size to that requested by the equipment?"),
                             QMessageBox::Yes,
                             QMessageBox::No)
        == QMessageBox::Yes
     )
   {
      if( recipeObs )
      {
         recipeObs->setBatchSize_l( kit->batchSize_l() );
         recipeObs->setBoilSize_l( kit->boilSize_l() );
      }
   }
}

void MainWindow::updateRecipeBatchSize()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setBatchSize_l( Brewtarget::volQStringToSI(lineEdit_batchSize->text()) );
}

void MainWindow::updateRecipeBoilSize()
{
   if( recipeObs == 0 )
      return;
   
   recipeObs->setBoilSize_l( Brewtarget::volQStringToSI(lineEdit_boilSize->text()) );
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
}

Recipe* MainWindow::currentRecipe()
{
   return recipeObs;
}

Fermentable* MainWindow::selectedFermentable()
{
   QModelIndexList selected = fermentableTable->selectedIndexes();
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

   modelIndex = fermentableTable->getProxy()->mapToSource(viewIndex);

   Fermentable* ferm = fermentableTable->getModel()->getFermentable(modelIndex.row());

   return ferm;
}

Hop* MainWindow::selectedHop()
{
   QModelIndexList selected = hopTable->selectedIndexes();
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

   modelIndex = hopTable->getProxy()->mapToSource(viewIndex);

   Hop* h = hopTable->getModel()->getHop(modelIndex.row());

   return h;
}

Misc* MainWindow::selectedMisc()
{
   QModelIndexList selected = miscTable->selectedIndexes();
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

   modelIndex = miscTable->getProxy()->mapToSource(viewIndex);

   Misc* m = miscTable->getModel()->getMisc(modelIndex.row());

   return m;
}

Yeast* MainWindow::selectedYeast()
{
   QModelIndexList selected = yeastTable->selectedIndexes();
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

   modelIndex = yeastTable->getProxy()->mapToSource(viewIndex);

   Yeast* y = yeastTable->getModel()->getYeast(modelIndex.row());

   return y;
}

void MainWindow::removeSelectedFermentable()
{
   Fermentable* f = selectedFermentable();
   if( f == 0 )
      return;

   fermentableTable->getModel()->removeFermentable(f);
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

   hopTable->getModel()->removeHop(hop);
   recipeObs->removeHop(hop);
}

void MainWindow::removeSelectedMisc()
{
   Misc* misc = selectedMisc();
   if( misc == 0 )
      return;

   miscTable->getModel()->removeMisc(misc);
   recipeObs->removeMisc(misc);
}

void MainWindow::removeSelectedYeast()
{
   Yeast* yeast = selectedYeast();
   if( yeast == 0 )
      return;

   yeastTable->getModel()->removeYeast(yeast);
   recipeObs->removeYeast(yeast);
}

void MainWindow::newRecipe()
{
   QString name = QInputDialog::getText(this, tr("Recipe name"),
                                          tr("Recipe name:"));
   if( name.isEmpty() )
      return;

   Recipe* newRec = Database::instance().newRecipe();

   // Set the following stuff so everything appears nice
   // and the calculations don't divide by zero... things like that.
   newRec->setName(name);
   newRec->setBatchSize_l(18.93); // 5 gallons
   newRec->setBoilSize_l(23.47);  // 6.2 gallons
   newRec->setEfficiency_pct(70.0);

   setSelection(treeView_recipe->findRecipe(newRec));
   setRecipe(newRec);
}

void MainWindow::setSelection(QModelIndex item)
{
   BrewTargetTreeView *active = qobject_cast<BrewTargetTreeView*>(tabWidget_Trees->currentWidget()->focusWidget());

   if (! item.isValid())
      return;

   if ( active == 0 )
      active = qobject_cast<BrewTargetTreeView*>(treeView_recipe);

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

      BrewNote* bNote = Database::instance().newBrewNote(rec);
      // Make sure everything is properly set and selected
      if( rec != recipeObs )
         setRecipe(rec);

      setBrewNote(bNote);

      bIndex = treeView_recipe->findBrewNote(bNote);
      if ( bIndex.isValid() )
         setSelection(bIndex);
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

      BrewNote* bNote = Database::instance().newBrewNote(old);
      bNote->setBrewDate();

      if (rec != recipeObs)
         setRecipe(rec);
      //recipeObs->addBrewNote(bNote);
      setBrewNote(bNote);

      setSelection(treeView_recipe->findBrewNote(bNote));
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
   if( QMessageBox::question( this, tr("A Warning"),
         tr("This will obliterate your current set of recipes and ingredients. Do you want to continue?"),QMessageBox::Yes, QMessageBox::No )
       == QMessageBox::No
      )
   {
      return;
   }
   
   QString dir = QFileDialog::getExistingDirectory(this, tr("Restore Database"));
   
   bool success = Database::restoreFromDir(dir);
   
   if( ! success )
      QMessageBox::warning( this, tr("Oops!"), tr("For some reason, the operation failed.") );
}

// Imports all the recipes from a file into the database.
void MainWindow::importFiles()
{
   if ( ! fileOpener->exec() )
      return;
   
   foreach( QString filename, fileOpener->selectedFiles() )
      Database::instance().importFromXML(filename);
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
   Mash* mash;
   if( recipeObs && recipeObs->mash() )
   {
      mash = recipeObs->mash();
   }
   else
   {
      return;
   }
   
   QModelIndexList selected = mashStepTableWidget->selectedIndexes();
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

   MashStep* step = mashStepTableWidget->getModel()->getMashStep(row);
   Database::instance().removeFrom(mash,step);
}

void MainWindow::editSelectedMashStep()
{
   Mash* mash;
   if( recipeObs && recipeObs->mash() )
   {
      mash = recipeObs->mash();
   }
   else
   {
      return;
   }

   QModelIndexList selected = mashStepTableWidget->selectedIndexes();
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

   MashStep* step = mashStepTableWidget->getModel()->getMashStep(row);
   mashStepEditor->setMashStep(step);
   mashStepEditor->setVisible(true);
}

void MainWindow::removeMash()
{

   if( mashComboBox->currentIndex() == -1)
      return;
   //due to way this is designed, we can't have a NULL mash, so
   //we need to remove all the mash steps and then remove the mash
   //from the database.
   //remove from db
   Mash *m = mashComboBox->getSelectedMash();
   m->removeAllMashSteps();
   Database::instance().removeMash(m);
   
   Mash* defaultMash = Database::instance().newMash(recipeObs);
   mashStepTableWidget->getModel()->setMash(defaultMash);
   
   //remove from combobox handled automatically by qt
   mashComboBox->setIndex( -1 );
   //recipeObs->forceNotify();
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
   QSettings settings("brewtarget");
   settings.setValue("geometry", saveGeometry());
   settings.setValue("windowState", saveState());
   if ( recipeObs )
      settings.setValue("recipeKey", recipeObs->key());
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
   
   Mash* mash = mashComboBox->getSelectedMash();
   
   if( mash == 0 )
      return;

   // NOTE: Should displace mash as the mash associated with recipeObs.
   Database::instance().newMash(mash);
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
   mashComboBox->setIndexByMash(newMash);
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
               equipmentComboBox->setIndexByEquipment(active->getEquipment(index));
               droppedRecipeEquipment(active->getEquipment(index));
               break;
            case BrewTargetTreeItem::FERMENTABLE:
               Database::instance().addToRecipe( recipeObs, Database::instance().newFermentable(active->getFermentable(index)) );
               last = fermentableTab;
               break;
            case BrewTargetTreeItem::HOP:
               Database::instance().addToRecipe( recipeObs, Database::instance().newHop(active->getHop(index)));
               last = hopsTab;
               break;
            case BrewTargetTreeItem::MISC:
               Database::instance().addToRecipe( recipeObs, Database::instance().newMisc(active->getMisc(index)) );
               last = miscTab;
               break;
            case BrewTargetTreeItem::YEAST:
               Database::instance().addToRecipe( recipeObs, Database::instance().newYeast(active->getYeast(index)) );
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

// Set up the context menus.  This is much prettier now that I moved the
// tree-specific pieces into the treeview objects.
void MainWindow::setupContextMenu()
{
   QMenu *sMenu = new QMenu();

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
   above = active->getFirst();

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
            Brewtarget::log(Brewtarget::WARNING, QObject::tr("MainWindow::copySelected Unknown type: %1").arg(active->getType(*at)));
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


   if ( active->getType(above) == BrewTargetTreeItem::RECIPE )
      setRecipeByIndex(above);
   setSelection(above);
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
         Brewtarget::log(Brewtarget::WARNING, tr("MainWindow::openForWrite Could not open %1 for writing.").arg(filename));
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

   for(int i=0; i < selected.count(); ++i)
   {
      QModelIndex selection = selected.value(i);
      int type = treeView_recipe->getType(selection);

      switch(type)
      {
         case BrewTargetTreeItem::RECIPE:
            Database::instance().toXml( treeView_recipe->getRecipe(selection), doc, recipe);
            didRecipe = true;
            break;
         case BrewTargetTreeItem::EQUIPMENT:
            Database::instance().toXml( treeView_recipe->getEquipment(selection), doc, recipe);
            break;
         case BrewTargetTreeItem::HOP:
            Database::instance().toXml( treeView_recipe->getHop(selection), doc, recipe);
            break;
         case BrewTargetTreeItem::MISC:
            Database::instance().toXml( treeView_recipe->getMisc(selection), doc, recipe);
            break;
         case BrewTargetTreeItem::YEAST:
            Database::instance().toXml( treeView_recipe->getYeast(selection), doc, recipe);
            break;
      }
   }

   if ( didRecipe )
      doc.appendChild(recipe);
   else
      doc.appendChild(dbase);

   out << doc.toString();
   
   outFile->close();
}

void MainWindow::mergeDatabases()
{
   // TODO: implement.
   /*
   QString otherDb;
   QFile dbFile, otherDbFile;
   QDomDocument dbDoc, otherDbDoc;
   QString err;
   int line, col;
   QMessageBox::StandardButton but;
   
   // Tell user what's about to happen.
   but = QMessageBox::question( this,
                          tr("Database Merge"),
                          tr("You are about to merge another database into the current one. Continue?"),
                          QMessageBox::Yes | QMessageBox::No,
                          QMessageBox::No );
   if( but == QMessageBox::No )
      return;
   
   // Select the db to merge with.
   otherDb = QFileDialog::getOpenFileName( this,
                                           tr("Select Database File"),
                                           Brewtarget::getUserDataDir(),
                                           tr("BeerXML File (*.xml)") );

   // Try to open the file.
   otherDbFile.setFileName(otherDb);
   if( ! otherDbFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.").arg(otherDbFile.fileName()));
      return;
   }
   if( ! otherDbDoc.setContent(&otherDbFile, false, &err, &line, &col) )
     Brewtarget::logW( QString("Bad document formatting in %1 %2:%3. %4").arg(otherDbFile.fileName()).arg(line).arg(col).arg(err) );
   
   // NOTE: What to do here?
   // Save db.
   //Database::savePersistent();
   
   // Open db.
   dbFile.setFileName(Database::getDbFileName());
   if( ! dbFile.open(QIODevice::ReadOnly) )
   {
      Brewtarget::logE(QString("Could not open %1 for reading.").arg(dbFile.fileName()));
      return;
   }
   dbDoc.setContent(&dbFile, false, &err, &line, &col);
   dbFile.close();
   
   // Merge.
   Database::mergeBeerXMLDBDocs( dbDoc, otherDbDoc );
   
   // Write changes.
   if( ! dbFile.open(QIODevice::Truncate | QIODevice::WriteOnly) )
   {
      Brewtarget::logE(QString("Could not open %1 for writing.").arg(dbFile.fileName()));
      return;
   }
   QTextStream dbOut(&dbFile);
   dbOut.setCodec( "UTF-8" );
   //dbOut <<  "<?xml version=\"1.0\" encoding=\"" << dbOut.codec()->name() << "\"?>\n";
   dbOut << dbDoc.toString();
   
   // Reload db. Probably safer just to ask the user to restart brewtarget.
   QMessageBox::information(this,
                            tr("Database Merged"),
                            tr("Database successfully merged. Please restart Brewtarget NOW and changes will appear."));
   

   dbFile.close();
   otherDbFile.close();
   */
}
