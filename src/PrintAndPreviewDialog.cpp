
/*
 * PrintAndPreviewDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2021
 * - Mattias Måhl <mattias@kejsarsten.com>
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
#include "PrintAndPreviewDialog.h"
#include <QDebug>
#include <QPainter>
#include <QFont>
#include <QMessageBox>
#include <QPrinterInfo>
#include <QList>
#include <QSizePolicy>




PrintAndPreviewDialog::PrintAndPreviewDialog ( MainWindow *parent)
   : QDialog(parent)
{
   setupUi(this);

   _parent = parent;

   _printer = new QPrinter();
   previewWidget = new QPrintPreviewWidget( _printer , this);
   recipeFormatter = new RecipeFormatter(this);

   collectRecipe();
   collectPrinterInfo();
   setupConnections();
   setPrintingControls();
   setupPreviewWidget();
}

void PrintAndPreviewDialog::showEvent(QShowEvent *e) {
   setVisible(true);
   previewWidget->updatePreview();
}

void PrintAndPreviewDialog::collectRecipe() {
   selectedRecipe = _parent->currentRecipe();
   recipeFormatter->setRecipe(selectedRecipe);
}

void PrintAndPreviewDialog::collectPrinterInfo() {
   //Getting the list of available printers on the system and adding them to the combobox for user selection.
   comboBox_PrinterSelector->addItems(QStringList(QPrinterInfo().availablePrinterNames()));
   //setting the systems default printer as the selected printer.
   comboBox_PrinterSelector->setCurrentText(QPrinterInfo().defaultPrinterName());

   CollectSupportedPageSizes();

}

void PrintAndPreviewDialog::CollectSupportedPageSizes() {
   PageSizeMap.clear();

   QPrinterInfo printerInfo(*_printer);
   QList<QPageSize> supportedPageSizeList(printerInfo.supportedPageSizes());
   foreach(QPageSize pageSize, supportedPageSizeList) {
      PageSizeMap.insert(pageSize.name(), pageSize);
      comboBox_PaperFormatSelector->addItem(pageSize.name());
   }
}

void PrintAndPreviewDialog::setupConnections() {
   //Preview windows connections for drawing the previewed document. connects to the printer.
   connect(previewWidget, &QPrintPreviewWidget::paintRequested, this, &PrintAndPreviewDialog::printDocument);

   //Radiobuttons connections to update settings for output.
   connect(buttonGroup, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), [=](QAbstractButton *button, bool checked) { outputRadioButtonsClicked (); });

   //Radiobuttons connections to update the orientation on the document.
   connect(buttonGroup_2, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), [=](QAbstractButton *button, bool checked) { orientationRadioButtonsClicked(); });

   //updates the selelected printer.
   connect(comboBox_PrinterSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index){
      selectedPrinterChanged(index);
   });

   //updates the selected Paper size
   connect(comboBox_PaperFormatSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
      selectedPaperChanged(index);
   });

   connect(Button_Cancel, &QPushButton::clicked, [this]() {
      setVisible(false);
   });

   //This will Printout the document and close the dialog
   connect(Button_Print, &QPushButton::clicked, [this]() {
      previewWidget->print();
      setVisible(false);
   });

}

void PrintAndPreviewDialog::resetAndClose(bool checked) {
   setVisible(false);
}

void PrintAndPreviewDialog::selectedPaperChanged(int index) {
   if (PageSizeMap.empty()) {
      return;
   }
   QString key(comboBox_PaperFormatSelector->currentText());
   QPageSize page(PageSizeMap.value(key));
   //_printer->setPageSize(page);
   _printer->pageLayout().setPageSize(page);
   previewWidget->update();
   //previewWidget->updatePreview();
}

void PrintAndPreviewDialog::selectedPrinterChanged(int index) {
   _printer->setPrinterName(comboBox_PrinterSelector->itemText(index));
}

void PrintAndPreviewDialog::outputRadioButtonsClicked() {
   outputSelection = (radioButton_OutputPaper->isChecked()) ? PAPER : ((radioButton_OutputPDF->isChecked()) ? PDF : HTML);
   setPrintingControls();
}

void PrintAndPreviewDialog::orientationRadioButtonsClicked() {
   _printer->setPageOrientation((radioButton_Protrait->isChecked()) ? QPageLayout::Orientation::Portrait : QPageLayout::Orientation::Landscape);
   previewWidget->updatePreview();
}

void PrintAndPreviewDialog::setPrintingControls() {
   //First off, The Printer selector either needs to be disabled when you output to PDF or HTML, or you set the outout selector to say like "QT PDF exporter" or something.
   //for now, let's disable it.
   comboBox_PrinterSelector->setEnabled    ( radioButton_OutputPaper->isChecked() );

   //Lets set the Papersize selector to be the inverse of radiobutton_OutputHTML.
   comboBox_PaperFormatSelector->setEnabled( ! radioButton_OutputHTML->isChecked() );


   //if HTML is selected the orientation and papersize is not relavant so lets set them to the inverse of radiobutton_OutputHTML.
   groupBox_Orientation->setEnabled        ( ! radioButton_OutputHTML->isChecked() );
   groupBox_PrinterSettings->setEnabled    ( ! radioButton_OutputHTML->isChecked() );

   Button_Print->setText((radioButton_OutputPaper->isChecked()) ? "Print" : "Save as");
}

void PrintAndPreviewDialog::setupPreviewWidget() {

   PrintAndPreviewDialog::verticalLayout_PrintPreviewWidget->addWidget ( previewWidget );

   previewWidget->setFont(QFont("Arial", 18, QFont::Bold));
   previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   previewWidget->setZoomMode(QPrintPreviewWidget::FitInView);
   previewWidget->show();
}

void PrintAndPreviewDialog::printDocument(QPrinter * printer)
{
   if ( _parent->currentRecipe() == nullptr)
      return;
   recipeFormatter->setRecipe(_parent->currentRecipe());
   using namespace nBtPage;
   //Setting up a blank page for drawing.
   BtPage page(printer);
   // adding the Recipe name as a title.
   PageText *recipeText = page.addChildObject(
      new PageText (
         _parent->currentRecipe()->name(),
         QFont("Arial", 18, QFont::Bold)
      ),
      QPoint(0,0)
      );

   PageText *brewerText = page.addChildObject(
      new PageText (
         _parent->currentRecipe()->brewer(),
         QFont("Arial", 10)
      ));
   brewerText->placeRelationalTo(recipeText, PlacingFlags::BELOW, 20, 0);

   //Adding the Brewtarget logo.
   PageImage *img = page.addChildObject<PageImage>(
      new PageImage (
         QPoint(),
         QImage(":/images/title.svg")
      ));
   img->setImageSize(280, 60);
   img->setDPI(printer->resolution());
   img->placeOnPage(printer, PlacingFlags::TOP | PlacingFlags::RIGHT);

   //Statistics table with beer data.
   PageTable *statTable = page.addChildObject(
      new PageTable (
         QString("Beer details"),                  /* table header text */
         recipeFormatter->buildStatList(),  /* Table data including column headers.*/
         QPoint(40, 85)                           /* position of text/table*/
      ));
   statTable->rowPadding=0;
   statTable->columnHeaders.at(1)->ColumnWidth=200;

   PageTable *fermTable = page.addChildObject(
      new PageTable (
         QString("Fermentables"),                  // table header text
         recipeFormatter->buildFermentableList()  // Table data including column headers.
      ));
   fermTable->placeRelationalTo(statTable, PlacingFlags::BELOW, 0, 20);

   // Create the HopsTable
   PageTable *hopsTable = page.addChildObject(
      new PageTable (
         QString("Hops"),                    // table header text
         recipeFormatter->buildHopsList()    // Table data including column headers.
      ));
   hopsTable->setColumnAlignment(1, Qt::AlignRight);
   hopsTable->placeRelationalTo(fermTable, PlacingFlags::BELOW, 0, 20);

   // Create the MiscTable
   PageTable *miscTable = page.addChildObject(
      new PageTable (
         QString("Misc"),                    // table header text
         recipeFormatter->buildMiscList()    // Table data including column headers.
      ));
   miscTable->placeRelationalTo(hopsTable, PlacingFlags::BELOW, 0, 20);

   // Create the Yeast Table
   PageTable *yeastTable = page.addChildObject(
      new PageTable (
         QString("Yeast"),                   // table header text
         recipeFormatter->buildYeastList()   // Table data including column headers.
      ));
   yeastTable->placeRelationalTo(miscTable, PlacingFlags::BELOW, 0, 20);

   PageTable *mashTable = page.addChildObject(
      new PageTable (
         QString("Mash"),
         recipeFormatter->buildMashList()
      ));
   mashTable->placeRelationalTo(yeastTable, PlacingFlags::BELOW, 0, 20);
   //Render the Page onto the painter/printer for preview/printing.

   page.renderPage();
}