
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
#include "btpage/Page.h"

PrintAndPreviewDialog::PrintAndPreviewDialog ( MainWindow *parent)
   : QDialog(parent)
{
   setupUi(this);

   _parent = parent;

   _printer = new QPrinter(QPrinter::HighResolution);

   QPageLayout layout = _printer->pageLayout();
   layout.setUnits(QPageLayout::Point);
   QMarginsF margins = layout.margins();

   int margin = 20;
   margins.setBottom( margin );
   margins.setTop( margin );
   margins.setLeft( margin );
   margins.setRight( margin );
   layout.setMargins(margins);
   _printer->setPageLayout(layout);

   previewWidget = new QPrintPreviewWidget( _printer , this);
   recipeFormatter = new RecipeFormatter(this);
   brewDayFormatter = new BrewDayFormatter(this);
   htmlDocument = new QTextBrowser(this);

   checkBox_Recipe->setChecked(true);
   checkBox_Recipe->setEnabled(false);

   collectRecipe();
   collectPrinterInfo();
   setupConnections();
   setPrintingControls();
   setupPreviewWidgets();
}

void PrintAndPreviewDialog::showEvent(QShowEvent *e) {
   setVisible(true);
   collectRecipe();

   int index = comboBox_PaperFormatSelector->findText(_printer->pageLayout().pageSize().name());
   comboBox_PaperFormatSelector->setCurrentIndex(index);
   comboBox_PaperFormatSelector->repaint();
   previewWidget->updatePreview();
}

void PrintAndPreviewDialog::collectRecipe() {
   selectedRecipe = _parent->currentRecipe();
   recipeFormatter->setRecipe(selectedRecipe);
   brewDayFormatter->setRecipe(selectedRecipe);
   label_CurrentRecipe->setText((selectedRecipe != nullptr) ? selectedRecipe->name() : "NULL");
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
      handlePrinting();
   });

   //Connect the Recipe and BrewInstructions checkboxes to update the preview when toggled
   connect(checkBox_Recipe, &QCheckBox::toggled, this, &PrintAndPreviewDialog::checkBoxRecipe_toggle);
   connect(checkBox_BrewdayInstructions, &QCheckBox::toggled, this, &PrintAndPreviewDialog::checkBoxBrewday_toggle);

}

/**
 * @brief handles the Recipe checkbox toggle
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxRecipe_toggle(bool checked)
{
   updatePreview();
}

/**
 * @brief handles the breDay checkbox toggle action
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxBrewday_toggle(bool checked)
{
   //first off, we always need to have something to export/print, so if brewday instructions is unchecked the recipe will automatically be selected.
   checkBox_Recipe->setChecked( (checkBox_BrewdayInstructions->isChecked()) ? checkBox_Recipe->isChecked() : true );

   // We enable the Recipe checkbox only if the brewday checkbox is checked.
   checkBox_Recipe->setEnabled( checkBox_BrewdayInstructions->isChecked() );

   //update the view accordingly
   updatePreview();
}

/**
 * @brief Handles the printing. sends to the selected output format.
 *
 */
void PrintAndPreviewDialog::handlePrinting() {
   //make it short if we are printing to paper.
   if (radioButton_OutputPaper->isChecked())
   {
      previewWidget->print();
   }
   // if we are not sending to printer we need to save a file.
   QString fileDialogFilter = (radioButton_OutputPDF->isChecked()) ? "PDF (*.pdf)" : "HTML (*.html)";
   QString filename = QFileDialog::getSaveFileName(
      this,
      (radioButton_OutputPDF->isChecked()) ? "Save PDF" : "Save HTML",
      QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
      fileDialogFilter
      );
   qDebug() << Q_FUNC_INFO << "Filename to save: " << filename;
   if (radioButton_OutputPDF->isChecked())
   {
      _printer->setOutputFormat(QPrinter::PdfFormat);
      _printer->setOutputFileName(filename);
      previewWidget->print();
   }
   else
   {
      QFile file = QFile(filename);
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      {
         qWarning() << Q_FUNC_INFO << tr("File %1 filname << could not be saved").arg(filename);
         QMessageBox msgbox(this);
         msgbox.setWindowTitle(tr("Error saving file"));
         msgbox.setText(tr("Could not open the file %1 for writing! please try again with a new filename or diretory").arg(filename));
         msgbox.exec();
         return;
      }
      QTextStream ts(&file);
      ts << htmlDocument->document()->toHtml();
      file.close();
   }

   //Closing down the dialog.
   setVisible(false);
}

/**
 * @brief updates the current view with the changed data. depanding on selected output.
 *
 * @param checked
 */
void PrintAndPreviewDialog::updatePreview() {
   if ( ! radioButton_OutputHTML->isChecked())
   {
      previewWidget->updatePreview();
   }
   else
   {
      bool chkRec = checkBox_Recipe->isChecked();
      bool chkBDI = checkBox_BrewdayInstructions->isChecked();
      QString pDoc = "";
      if (chkRec) {
         pDoc = recipeFormatter->getHTMLFormat(checkBox_BrewdayInstructions->isChecked());
      }
      if ( chkBDI && !chkRec ) {
         pDoc = brewDayFormatter->buildHTML();
      }
      htmlDocument->setHtml(pDoc);
      (radioButton_OutputHTML->isChecked()) ? htmlDocument->show() : htmlDocument->hide();
      (radioButton_OutputPaper->isChecked() || radioButton_OutputPDF->isChecked()) ? previewWidget->show() : previewWidget->hide();
   }
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

   //Only set this as "print" if we are printing to paper, else it is set to "save as"
   Button_Print->setText((radioButton_OutputPaper->isChecked()) ? "Print" : "Save as");

   //setting up printer accordingly
   if (radioButton_OutputPaper->isChecked())
   {
      _printer->setOutputFormat(QPrinter::OutputFormat::NativeFormat);
      _printer->setPrinterName(comboBox_PrinterSelector->currentText());
   }
   else if (radioButton_OutputPDF->isChecked())
   {
      _printer->setOutputFormat(QPrinter::OutputFormat::PdfFormat);
   }

   //setting up views correctly.
   updatePreview();

}

void PrintAndPreviewDialog::setupPreviewWidgets()
{
   //Setting up the Document preview for Paper and PDF
   PrintAndPreviewDialog::verticalLayout_PrintPreviewWidget->addWidget ( previewWidget );
   previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   previewWidget->setZoomMode(QPrintPreviewWidget::FitInView);
   previewWidget->show();

   //setting up the Document preview for HTML, we're hiding this to begin with.
   verticalLayout_PrintPreviewWidget->addWidget( htmlDocument );
   htmlDocument->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   htmlDocument->hide();
}

/**
 * @brief Slot: printDocument
 * prints out the document to the preview window.
 * to print hardcopy or PDF file use previewWidget->print()
 *
 * @param printer PagedPaintingDevice (QT) Printer to use for printout.
 */
void PrintAndPreviewDialog::printDocument(QPrinter * printer)
{
   if ( _parent->currentRecipe() == nullptr)
      return;
   recipeFormatter->setRecipe(_parent->currentRecipe());
   using namespace BtPage;
   //Setting up a blank page for drawing.
   Page page(printer);
   /*
   all of the below code to generate the printout will be subject to change and refactoring when I get around to
   making the template editor for printouts where you can save your templates and use them or share them
   with other BT users.
   */
   if ( checkBox_Recipe->isChecked())
   {
      renderHeader(page);
      renderRecipe(page);
   }
   if ( checkBox_Recipe->isChecked() && checkBox_BrewdayInstructions->isChecked())
      page.addChildObject(new PageBreak(&page));
   if (checkBox_BrewdayInstructions->isChecked())
   {
      renderHeader(page);
      renderBrewdayInstructions(page);
   }
   //Render the Page onto the painter/printer for preview/printing.
   page.renderPage();
}

/**
 * @brief Reders the Recipe data onto the page object.
 * @author Mattias Måhl
 * @param page Page object to render content to.
 */
void PrintAndPreviewDialog::renderRecipe(BtPage::Page &page)
{
   using namespace BtPage;

   //Statistics table with beer data.
   PageTable *statTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Beer details")),
         recipeFormatter->buildStatList()
      ));
   statTable->columnHeadersFont = statTable->Font;
   statTable->rowPadding=0;
   statTable->columnHeaders.at(1)->ColumnWidth=200;
   statTable->setPositionMM(10, 20);

   PageTable *fermTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Fermentables")),
         recipeFormatter->buildFermentableList()
      ));
   page.placeRelationalToMM(fermTable, statTable, BtPage::BELOW, 0, 5);

   // Create the HopsTable
   PageTable *hopsTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Hops")),
         recipeFormatter->buildHopsList()
      ));
   hopsTable->setColumnAlignment(1, Qt::AlignRight);
   page.placeRelationalToMM(hopsTable, fermTable, BtPage::BELOW, 0, 5);

   // Create the MiscTable
   PageTable *miscTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Misc")),
         recipeFormatter->buildMiscList()
      ));
   page.placeRelationalToMM(miscTable, hopsTable, BtPage::BELOW, 0, 5);

   // Create the Yeast Table
   PageTable *yeastTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Yeast")),
         recipeFormatter->buildYeastList()
      ));
   page.placeRelationalToMM(yeastTable, miscTable, BtPage::BELOW, 0, 5);

   PageTable *mashTable = page.addChildObject(
      new PageTable (
         &page,
         QString(tr("Mash")),
         recipeFormatter->buildMashList()
      ));
   page.placeRelationalToMM(mashTable, yeastTable, BtPage::BELOW, 0, 5);

   PageText *notesHeader = page.addChildObject(
      new PageText (
         &page,
         QString(tr("Notes")),
         QFont("Arial", 12, QFont::Bold)
      ));
   page.placeRelationalToMM(notesHeader, mashTable, BtPage::BELOW, 0, 5);

   PageText *notesText = page.addChildObject(
      new PageText (
         &page,
         recipeFormatter->buildNotesString(),
         QFont("Arial", 10)
      ));
   page.placeRelationalToMM(notesText, notesHeader, BtPage::BELOW);

   PageText *tasteNotesHeader = page.addChildObject(
      new PageText (
         &page,
         QString(tr("Taste notes")),
         QFont("Arial", 12, QFont::Bold)
      ));
   page.placeRelationalToMM(tasteNotesHeader, notesText, BtPage::BELOW, 0, 5);

   PageText *tasteNotesHeaderText = page.addChildObject(
      new PageText (
         &page,
         recipeFormatter->buildTasteNotesString(),
         QFont("Arial", 10)
      ));
   page.placeRelationalToMM(tasteNotesHeaderText, tasteNotesHeader, BtPage::BELOW);
}

/**
 * @brief Renders the instructions for the recipe on the page supplied in arguments.
 * @author Mattias Måhl
 * @param page Page object to render content to.
 */
void PrintAndPreviewDialog::renderBrewdayInstructions(BtPage::Page &page)
{
   brewDayFormatter->setRecipe(_parent->currentRecipe());
   using namespace BtPage;
   PageTable *statsTable = page.addChildObject(new PageTable (
      &page,
      _parent->currentRecipe()->name(),
      brewDayFormatter->buildTitleList()
   ));
   statsTable->setPositionMM(10,20);

   PageTable *instructionsTable = page.addChildObject(new PageTable (
      &page,
      QString("Instructions"),
      brewDayFormatter->buildInstructionList()
   ));
   page.placeRelationalToMM(instructionsTable, statsTable, BtPage::BELOW, 0, 5);
}

/**
 * @brief Reders the Recipe data onto the page object.
 * @author Mattias Måhl
 * @param page Page object to render content to.
 */
void PrintAndPreviewDialog::renderHeader(BtPage::Page &page)
{
   using namespace BtPage;
   // adding the Recipe name as a title.
   PageText *recipeText = page.addChildObject(
      new PageText (
         &page,
         _parent->currentRecipe()->name(),
         QFont("Arial", 18, QFont::Bold)
      ),
      QPoint(0,0)
      );
   // adding Brewers name
   PageText *brewerText = page.addChildObject(
      new PageText (
         &page,
         _parent->currentRecipe()->brewer(),
         QFont("Arial", 10)
      ));
   page.placeRelationalToMM(brewerText, recipeText, BtPage::BELOW, 2, 0);

   //Adding the Brewtarget logo.
   PageImage *img = page.addChildObject(
      new PageImage (
         &page,
         QPoint(),
         QImage(":/images/title.svg")
      ));
   img->setImageSizeMM(100, 20);
   page.placeOnPage(img, BtPage::TOP | BtPage::RIGHT);
}