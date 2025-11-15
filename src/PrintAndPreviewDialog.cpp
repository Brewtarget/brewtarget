/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * PrintAndPreviewDialog.cpp is part of Brewtarget, and is copyright the following authors 2021-2024:
 *   • Mattias Måhl <mattias@kejsarsten.com>
 *   • Matt Young <mfsy@yahoo.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#include "PrintAndPreviewDialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QFont>
#include <QList>
#include <QMessageBox>
#include <QPainter>
#include <QPrinterInfo>
#include <QSizePolicy>
#include <QStandardPaths>
#include <QTextBrowser>

#include "StockFormatter.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_PrintAndPreviewDialog.cpp"
#endif

/**
 * @brief Construct a new Print And Preview Dialog:: Print And Preview Dialog object
 *
 * @param parent
 */
PrintAndPreviewDialog::PrintAndPreviewDialog(MainWindow * parent) : QDialog(parent) {
   setupUi(this);

   mainWindow = parent;

   printer = new QPrinter(QPrinter::HighResolution);

   QPageLayout layout = printer->pageLayout();
   layout.setUnits(QPageLayout::Point);
   QMarginsF margins = layout.margins();

   int margin = 20;
   margins.setBottom( margin );
   margins.setTop( margin );
   margins.setLeft( margin );
   margins.setRight( margin );
   layout.setMargins(margins);
   printer->setPageLayout(layout);

   previewWidget = new QPrintPreviewWidget( printer , this);
   recipeFormatter = new RecipeFormatter(this);
   brewDayFormatter = new BrewDayFormatter(this);
   htmlDocument = new QTextBrowser(this);

   checkBox_Recipe->setChecked(true);
   checkBox_Recipe->setEnabled(false);

   //
   // Set the default split between the controls on the left and the preview on the right.  It is the relative, not
   // absolute, values that matter here as, per the documentation "The overall size of the splitter widget is not
   // affected [by the parameters to setSizes()]. Instead, any additional/missing space is distributed amongst the
   // widgets according to the relative weight of the sizes."
   //
   // .:TODO:. We should probably also do something clever like size the dialog relative to the current size of
   // MainWindow and/or remember its size when it is closed.  (At the moment it comes up a bit small on high DPI
   // displays.)
   //
   QList<int> const horizontalSplits{10, 50};
   this->uiSplitter->setSizes(horizontalSplits);

   collectRecipe();
   collectPrinterInfo();
   setupConnections();
   setPrintingControls();
   setupPreviewWidgets();
   return;
}

/**
 * @brief Destroy the Print And Preview Dialog:: Print And Preview Dialog object
 *
 */
PrintAndPreviewDialog::~PrintAndPreviewDialog() = default;

void PrintAndPreviewDialog::showEvent(QShowEvent * event) {
   //
   // In older versions of the code, we called setVisible(true) here without incident.  But, since Qt6, this results in
   // a stack overflow on Windows (because calling setVisible(true) triggers sending of a show event, which causes this
   // function to be called again).  The correct thing here is to call the base class handler.
   //
   this->QDialog::showEvent(event);
   this->collectRecipe();
   this->currentlySelectedPageSize = printer->pageLayout().pageSize();
   int index = comboBox_PaperFormatSelector->findText(currentlySelectedPageSize.name());
   this->comboBox_PaperFormatSelector->setCurrentIndex(index);
   this->comboBox_PaperFormatSelector->repaint();
   this->updatePreview();
   return;
}

/**
 * @brief Gets the Recipe from MainWindow and sets it to the respective formatters.
 */
void PrintAndPreviewDialog::collectRecipe() {
   selectedRecipe = mainWindow->currentRecipe();
   recipeFormatter->setRecipe(selectedRecipe);
   brewDayFormatter->setRecipe(selectedRecipe);
   label_CurrentRecipe->setText((selectedRecipe != nullptr) ? selectedRecipe->name() : "NULL");
   return;
}

/**
 * @brief Collects the available printers on the computer and saves the list to the comboBox.
 */
void PrintAndPreviewDialog::collectPrinterInfo() {
   //Getting the list of available printers on the system and adding them to the combobox for user selection.
   comboBox_PrinterSelector->addItems(QStringList(QPrinterInfo().availablePrinterNames()));
   //If there are no printers installed on the system we disable the Paper output and select the PDF as default.
   if (comboBox_PrinterSelector->count() == 0) {
      radioButton_OutputPDF->setChecked(true);
      radioButton_OutputPaper->setEnabled(false);
   }
   //setting the systems default printer as the selected printer.
   comboBox_PrinterSelector->setCurrentText(QPrinterInfo().defaultPrinterName());

   collectSupportedPageSizes();
   return;
}

/**
 * @brief Collect the supported Paper sizes from the selected printer.
 */
void PrintAndPreviewDialog::collectSupportedPageSizes() {
   PageSizeMap.clear();
   QPrinterInfo printerInfo(*printer);
   QList<QPageSize> supportedPageSizeList;
   if (radioButton_OutputPaper->isChecked()) {
      supportedPageSizeList = printerInfo.supportedPageSizes();
   } else if (radioButton_OutputPDF->isChecked()) {
      qDebug() << "generating a list of page sizes as there is no printer intalled on the system";
      supportedPageSizeList = generatePageSizeList();
   }
   for (QPageSize pageSize : supportedPageSizeList) {
      PageSizeMap.insert(pageSize.name(), pageSize);
      comboBox_PaperFormatSelector->addItem(pageSize.name());
   }
   if (comboBox_PaperFormatSelector->findText(currentlySelectedPageSize.name(), Qt::MatchFlag::MatchContains) > -1) {
      comboBox_PaperFormatSelector->setCurrentText(currentlySelectedPageSize.name());
   } else {
      currentlySelectedPageSize = QPageSize((QPageSize::PageSizeId)0);
      comboBox_PaperFormatSelector->setCurrentIndex(0);
   }
   return;
}

QList<QPageSize> PrintAndPreviewDialog::generatePageSizeList() {
   QList<QPageSize> result;

   for (int itor = 0; itor < QPageSize::LastPageSize; itor++) {
      QPageSize key((QPageSize::PageSizeId)itor);
      result.append(key);
   }
   return result;
}

/**
 * @brief Connects all the signals to their respective function.
 */
void PrintAndPreviewDialog::setupConnections() {
   // Preview windows connections for drawing the previewed document. connects to the printer.
   connect(previewWidget, &QPrintPreviewWidget::paintRequested, this, &PrintAndPreviewDialog::printDocument);

   // Radiobuttons connections to update settings for output.
   connect(buttonGroup,   QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &PrintAndPreviewDialog::outputRadioButtonsClicked);

   // Radiobuttons connections to update the orientation on the document.
   connect(buttonGroup_2, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, &PrintAndPreviewDialog::orientationRadioButtonsClicked);

   // Updates the selected printer.
   connect(comboBox_PrinterSelector,     QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PrintAndPreviewDialog::selectedPrinterChanged);

   // Updates the selected Paper size
   connect(comboBox_PaperFormatSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PrintAndPreviewDialog::selectedPaperChanged);

   connect(Button_Cancel, &QPushButton::clicked, [this]() {
      setVisible(false);
   });

   // This will print out the document and close the dialog
   connect(Button_Print,                   &QPushButton::clicked,       this, &PrintAndPreviewDialog::handlePrinting);

   // Connect the Recipe and BrewInstructions checkboxes to update the preview when toggled
   connect(checkBox_Recipe,                &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxRecipe_toggle);
   connect(checkBox_BrewdayInstructions,   &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxBrewday_toggle);

   connect(checkBox_inventoryAll,          &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxInventoryAll_toggle);
   connect(checkBox_inventoryFermentables, &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxInventoryIngredient_toggle);
   connect(checkBox_inventoryHops,         &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxInventoryIngredient_toggle);
   connect(checkBox_inventoryMicellaneous, &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxInventoryIngredient_toggle);
   connect(checkBox_inventoryYeast,        &QCheckBox::toggled,         this, &PrintAndPreviewDialog::checkBoxInventoryIngredient_toggle);

   connect(verticalTabWidget,              &QTabWidget::currentChanged, this, &PrintAndPreviewDialog::verticalTabWidget_currentChanged);
   return;
}

/**
 * @brief handles the Recipe checkbox toggle
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxRecipe_toggle([[maybe_unused]] bool checked) {
   updatePreview();
   return;
}

/**
 * @brief handles the brew day checkbox toggle action
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxBrewday_toggle([[maybe_unused]] bool checked) {
   // First off, we always need to have something to export/print, so if brewday instructions is unchecked the recipe
   // will automatically be selected.
   checkBox_Recipe->setChecked( (checkBox_BrewdayInstructions->isChecked()) ? checkBox_Recipe->isChecked() : true );

   // We enable the Recipe checkbox only if the brewday checkbox is checked.
   checkBox_Recipe->setEnabled( checkBox_BrewdayInstructions->isChecked() );

   // Update the view accordingly
   updatePreview();
   return;
}

/**
 * @brief handle the Inventory All checkBox signal.
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxInventoryAll_toggle(bool checked) {
   checkBox_inventoryFermentables->setEnabled(!checked);
   checkBox_inventoryHops->setEnabled(!checked);
   checkBox_inventoryYeast->setEnabled(!checked);
   checkBox_inventoryMicellaneous->setEnabled(!checked);
   checkBox_inventoryFermentables->setChecked(true);
   checkBox_inventoryHops->setChecked(true);
   checkBox_inventoryYeast->setChecked(true);
   checkBox_inventoryMicellaneous->setChecked(true);
   updatePreview();
   return;
}

/**
 * @brief handles the Ingredient checkboxes toggled signal.
 *
 * @param checked
 */
void PrintAndPreviewDialog::checkBoxInventoryIngredient_toggle([[maybe_unused]] bool checked) {
   updatePreview();
   return;
}

/**
 * @brief Handels the Vertical TabWidget 'currentChanged" signal.
 *
 * @param index
 */
void PrintAndPreviewDialog::verticalTabWidget_currentChanged([[maybe_unused]] int index) {
   updatePreview();
   return;
}

/**
 * @brief Handles the printing. sends to the selected output format.
 *
 */
void PrintAndPreviewDialog::handlePrinting() {
   // Make it short if we are printing to paper.
   if (radioButton_OutputPaper->isChecked()) {
      previewWidget->print();
   } else {
      // if we are not sending to printer we need to save a file.
      QString fileDialogFilter = (radioButton_OutputPDF->isChecked()) ? "PDF (*.pdf)" : "HTML (*.html)";
      QString filename = QFileDialog::getSaveFileName(
         this,
         (radioButton_OutputPDF->isChecked()) ? "Save PDF" : "Save HTML",
         QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
         fileDialogFilter
         );
      qDebug() << Q_FUNC_INFO << "Filename to save: " << filename;
      if (radioButton_OutputPDF->isChecked()) {
         printer->setOutputFormat(QPrinter::PdfFormat);
         printer->setOutputFileName(filename);
         previewWidget->print();
      } else {
         QFile file(filename);
         if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
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
   }
   // Closing down the dialog.
   setVisible(false);
   return;
}

/**
 * @brief updates the current view with the changed data. depanding on selected output.
 *
 * @param checked
 */
void PrintAndPreviewDialog::updatePreview() {
   if (!radioButton_OutputHTML->isChecked()) {
      previewWidget->updatePreview();
   } else {
      QString pDoc = "";

      if (verticalTabWidget->currentIndex() == 0) {
         bool chkRec = checkBox_Recipe->isChecked();
         bool chkBDI = checkBox_BrewdayInstructions->isChecked();
         if (chkRec) {
            pDoc = recipeFormatter->getHtmlFormat();
         }
         if ( chkBDI && !chkRec ) {
            pDoc += brewDayFormatter->buildHtml();
         }
      } else if (verticalTabWidget->currentIndex() == 1) {
         StockFormatter::HtmlGenerationFlags flags;
         if (checkBox_inventoryFermentables->isChecked()) { flags |= StockFormatter::HtmlGenerationFlag::FERMENTABLES ; }
         if (checkBox_inventoryHops->isChecked()        ) { flags |= StockFormatter::HtmlGenerationFlag::HOPS         ; }
         if (checkBox_inventoryYeast->isChecked()       ) { flags |= StockFormatter::HtmlGenerationFlag::YEAST        ; }
         if (checkBox_inventoryMicellaneous->isChecked()) { flags |= StockFormatter::HtmlGenerationFlag::MISCELLANEOUS; }

         pDoc = StockFormatter::createStockHtml(flags);
      }
      // adding the generated HTML to the QTexBrowser.
      htmlDocument->setHtml(pDoc);
   }

   // choose what displaywidget that should be showing depending on users choice.
   if (radioButton_OutputHTML->isChecked()) {
      htmlDocument->show();
   } else {
      htmlDocument->hide();
   }
   if (radioButton_OutputPaper->isChecked() || radioButton_OutputPDF->isChecked()) {
      previewWidget->show();
   } else {
      previewWidget->hide();
   }

   return;
}

/**
 * @brief Closes the Dialog
 *
 * @param checked
 */
void PrintAndPreviewDialog::resetAndClose([[maybe_unused]] bool checked) {
   setVisible(false);
   return;
}

/**
 * @brief Sets the selected paper and updates the view.
 *
 * @param index
 */
void PrintAndPreviewDialog::selectedPaperChanged([[maybe_unused]] int index) {
   if (PageSizeMap.empty()) {
      return;
   }
   QString key(comboBox_PaperFormatSelector->currentText());
   currentlySelectedPageSize = PageSizeMap.value(key);
   //_printer->setPageSize(page);
   printer->pageLayout().setPageSize(currentlySelectedPageSize);
   updatePreview();
   return;
}

/**
 * @brief updates the selected printer and sets appropriate values to Papersize
 *
 * @param index
 */
void PrintAndPreviewDialog::selectedPrinterChanged(int index) {
   printer->setPrinterName(comboBox_PrinterSelector->itemText(index));
   collectSupportedPageSizes();
   return;
}

/**
 * @brief handles the output radio buttons signal.
 */
void PrintAndPreviewDialog::outputRadioButtonsClicked() {
   setPrintingControls();
   return;
}

/**
 * @brief Handles the Orientation Radio buttons
 */
void PrintAndPreviewDialog::orientationRadioButtonsClicked() {
   printer->setPageOrientation((radioButton_Protrait->isChecked()) ? QPageLayout::Orientation::Portrait : QPageLayout::Orientation::Landscape);
   previewWidget->updatePreview();
   return;
}

/**
 * @brief Sets the Printing settings and enables/disables controls accordingly.
 */
void PrintAndPreviewDialog::setPrintingControls() {
   // First off, the printer selector either needs to be disabled when you output to PDF or HTML, or you set the output
   // selector to say like "QT PDF exporter" or something.
   // For now, let's disable it.
   comboBox_PrinterSelector->setEnabled    ( radioButton_OutputPaper->isChecked() );

   // Let's set the Papersize selector to be the inverse of radiobutton_OutputHTML.
   comboBox_PaperFormatSelector->setEnabled( ! radioButton_OutputHTML->isChecked() );


   // If HTML is selected the orientation and papersize is not relavant so lets set them to the inverse of
   // radiobutton_OutputHTML.
   groupBox_Orientation->setEnabled        ( ! radioButton_OutputHTML->isChecked() );
   groupBox_PrinterSettings->setEnabled    ( ! radioButton_OutputHTML->isChecked() );

   // Only set this as "print" if we are printing to paper, else it is set to "save as"
   Button_Print->setText((radioButton_OutputPaper->isChecked()) ? "Print" : "Save as");

   // Setting up printer accordingly
   if (radioButton_OutputPaper->isChecked()) {
      printer->setOutputFormat(QPrinter::OutputFormat::NativeFormat);
      printer->setPrinterName(comboBox_PrinterSelector->currentText());
   } else if (radioButton_OutputPDF->isChecked()) {
      printer->setOutputFormat(QPrinter::OutputFormat::PdfFormat);
   }
   collectSupportedPageSizes();

   // Setting up views correctly.
   updatePreview();

   return;
}

/**
 * @brief Creates the preview widgets and sets the QPrintPreviewWidget as default viewer.
 *        This is changed according to when output is changed.
 */
void PrintAndPreviewDialog::setupPreviewWidgets() {
   // Setting up the Document preview for Paper and PDF
   PrintAndPreviewDialog::verticalLayout_PrintPreviewWidget->addWidget ( previewWidget );
   previewWidget->setMinimumWidth(300);
   previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   previewWidget->setZoomMode(QPrintPreviewWidget::FitInView);
   previewWidget->show();

   // Setting up the Document preview for HTML, we're hiding this to begin with.
   verticalLayout_PrintPreviewWidget->addWidget( htmlDocument );
   htmlDocument->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   htmlDocument->hide();

   return;
}

/**
 * @brief Slot: printDocument
 * prints out the document to the preview window.
 * to print hardcopy or PDF file use previewWidget->print()
 *
 * @param printer PagedPaintingDevice (QT) Printer to use for printout.
 */
void PrintAndPreviewDialog::printDocument(QPrinter * printer) {
   if ( mainWindow->currentRecipe() == nullptr) {
      return;
   }
   recipeFormatter->setRecipe(mainWindow->currentRecipe());
   // Setting up a blank page for drawing.
   QTextBrowser textBrowser;
   QString hDoc = "";
   // If we are watching the Recipe tab we should print recipe stuff.
   if (Ui_BtPrintAndPreview::verticalTabWidget->currentIndex() == 0) {
      //
      // TODO: All of the below code to generate the printout will be subject to change and refactoring when I get
      //       around to making the template editor for printouts where you can save your templates and use them or
      //       share them with other BT users.
      //
      hDoc += recipeFormatter->buildHtmlHeader();
      if (checkBox_Recipe->isChecked()) {
         hDoc += recipeFormatter->getHtmlFormat();
      }

      if (checkBox_BrewdayInstructions->isChecked()) {
         hDoc += brewDayFormatter->buildInstructionHtml();
      }

      hDoc += recipeFormatter->buildHtmlFooter();
   } else if (verticalTabWidget->currentIndex() == 1) {
      StockFormatter::HtmlGenerationFlags flags;
      if (checkBox_inventoryFermentables->isChecked()) { flags |= StockFormatter::HtmlGenerationFlag::FERMENTABLES ; }
      if (checkBox_inventoryHops->isChecked()        ) { flags |= StockFormatter::HtmlGenerationFlag::HOPS         ; }
      if (checkBox_inventoryYeast->isChecked()       ) { flags |= StockFormatter::HtmlGenerationFlag::YEAST        ; }
      if (checkBox_inventoryMicellaneous->isChecked()) { flags |= StockFormatter::HtmlGenerationFlag::MISCELLANEOUS; }

      hDoc += StockFormatter::createStockHtml(flags);
   }

   // Render the page onto the painter/printer for preview/printing.
   textBrowser.setHtml(hDoc);
   textBrowser.print(printer);

   return;
}
