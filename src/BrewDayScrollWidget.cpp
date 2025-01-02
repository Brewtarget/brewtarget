/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * BrewDayScrollWidget.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Carles Muñoz Gorriz <carlesmu@internautas.org>
 *   • Daniel Pettersson <pettson81@gmail.com>
 *   • Greg Greenaae <ggreenaae@gmail.com>
 *   • Mark de Wever <koraq@xs4all.nl>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Mik Firestone <mikfire@gmail.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *   • Théophane Martin <theophane.m@gmail.com>
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
#include "BrewDayScrollWidget.h"

#include <QDate>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPrinter>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "Html.h"
///#include "InstructionWidget.h"
#include "measurement/Measurement.h"
#include "measurement/UnitSystem.h"
#include "model/Equipment.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Style.h"
#include "PersistentSettings.h"
#include "TimerWidget.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_BrewDayScrollWidget.cpp"
#endif

namespace {
   QString styleName(Style const * style) {
      if (!style) {
         return "unknown";
      }

      return style->name();
   }

   QString boilTime(Equipment const * equipment) {
      if (!equipment) {
         return "unknown";
      }

      return Measurement::displayAmount(Measurement::Amount{equipment->boilTime_min().value_or(Equipment::default_boilTime_mins), Measurement::Units::minutes});
   }
}


BrewDayScrollWidget::BrewDayScrollWidget(QWidget* parent) : QWidget{parent},
                                                            m_recObs{nullptr} {
   this->setupUi(this);
   this->setObjectName("BrewDayScrollWidget");

   connect(this->listWidget,                      &QListWidget::currentRowChanged, this, &BrewDayScrollWidget::showInstruction          );
   connect(this->btTextEdit,                      &BtTextEdit::textModified,       this, &BrewDayScrollWidget::saveInstruction          );
   connect(this->pushButton_insert,               &QAbstractButton::clicked,       this, &BrewDayScrollWidget::insertInstruction        );
   connect(this->pushButton_remove,               &QAbstractButton::clicked,       this, &BrewDayScrollWidget::removeSelectedInstruction);
   connect(this->pushButton_up,                   &QAbstractButton::clicked,       this, &BrewDayScrollWidget::pushInstructionUp        );
   connect(this->pushButton_down,                 &QAbstractButton::clicked,       this, &BrewDayScrollWidget::pushInstructionDown      );
   connect(this->pushButton_generateInstructions, &QAbstractButton::clicked,       this, &BrewDayScrollWidget::generateInstructions     );

   return;
}

BrewDayScrollWidget::~BrewDayScrollWidget() = default;

void BrewDayScrollWidget::saveInstruction() {
   this->m_recObs->instructions()[ listWidget->currentRow() ]->setDirections( btTextEdit->toPlainText() );
   return;
}

void BrewDayScrollWidget::showInstruction(int insNdx) {
   if (!this->m_recObs) {
      return;
   }

   int size = m_recIns.size();
   if (insNdx < 0 || insNdx >= size) {
      return;
   }

   // Block signals to avoid setPlainText() from triggering saveInstruction().
   btTextEdit->setPlainText((m_recIns[insNdx])->directions());
   return;
}

void BrewDayScrollWidget::generateInstructions() {
   if (!this->m_recObs) {
      return;
   }

   if (!btTextEdit->isEnabled()) {
      btTextEdit->setEnabled(true);
   }

   //
   // If the Recipe already has instructions, then they will get erased when we call generateInstructions.  In case the
   // user has done manual edits to the existing instructions, we should get confirmation to proceed.
   //
   // TODO: Would be neat to make this an undoable action
   //
   bool proceed = true;
   if (this->m_recObs->instructions().size() > 0) {
      proceed = QMessageBox::Yes == QMessageBox::question(
         this,
         tr("Overwrite Existing Instructions"),
         tr("Generating instructions will overwrite the existing ones.  This is not undoable.  Do you want to proceed?"),
         QMessageBox::Yes,
         QMessageBox::No
      );
   }

   if (proceed) {
      this->m_recObs->generateInstructions();
   }
   return;
}

QSize BrewDayScrollWidget::sizeHint() const {
   return QSize(0,0);
}

void BrewDayScrollWidget::removeSelectedInstruction() {
   if (this->m_recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row < 0) {
      return;
   }
   this->m_recObs->m_instructions.remove(this->m_recIns[row]);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->m_recObs);

   if (this->m_recIns.isEmpty()) {
      btTextEdit->clear();
      btTextEdit->setEnabled(false);
   } else {
      if (row > this->m_recIns.size()) {
         row = this->m_recIns.size();
      }
      listWidget->setCurrentRow(row);
   }

   return;
}

void BrewDayScrollWidget::pushInstructionUp() {
   if (this->m_recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row <= 0) {
      return;
   }

   this->m_recObs->m_instructions.swap(*this->m_recIns[row], *this->m_recIns[row-1]);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->m_recObs);

   listWidget->setCurrentRow(row-1);
   return;
}

void BrewDayScrollWidget::pushInstructionDown() {
   if (this->m_recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row >= listWidget->count() - 1 || row < 0) {
      return;
   }

   this->m_recObs->m_instructions.swap(*this->m_recIns[row], *this->m_recIns[row+1]);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->m_recObs);

   listWidget->setCurrentRow(row+1);
   return;
}

bool BrewDayScrollWidget::loadComplete(bool ok) {
   this->m_doc->print(this->m_printer);
   return ok;
}

void BrewDayScrollWidget::print(QPrinter *mainPrinter, int action, QFile* outFile) {
   if (this->m_recObs == nullptr) {
      return;
   }

   // Connect the webview's signal
   if (action == PRINT) {
      this->m_printer = mainPrinter;
   }

   // Start building the document to be printed.  The HTML doesn't work with
   // the image since it is a compiled resource
   QString pDoc = buildTitleTable(action != HTML);
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += tr("<h2>Notes</h2>");
   if (this->m_recObs->notes() != "" )
      pDoc += QString("<div id=\"customNote\">%1</div>\n").arg(m_recObs->notes());

   pDoc += "</body></html>";

   this->m_doc->setHtml(pDoc);
   if (action == PREVIEW) {
      this->m_doc->show();
   } else if ( action == HTML ) {
      QTextStream out(outFile);
      out << pDoc;
      outFile->close();
   } else {
       this->loadComplete(true);
   }
   return;
}

void BrewDayScrollWidget::setRecipe(Recipe* rec) {
   // Disconnect old notifier.
   if (this->m_recObs) {
      disconnect(this->m_recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges );
   }

   this->m_recObs = rec;
   connect(this->m_recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges);

   m_recIns = this->m_recObs->m_instructions.items();
   for (auto ins : m_recIns) {
      connect(ins.get(), &Instruction::changed, this, &BrewDayScrollWidget::acceptInsChanges);
   }

   btTextEdit->clear();
   if (m_recIns.isEmpty()) {
      btTextEdit->setEnabled(false);
   } else {
      btTextEdit->setEnabled(true);
   }

   showChanges();
   return;
}

void BrewDayScrollWidget::insertInstruction() {
   if (this->m_recObs == nullptr) {
      return;
   }

   if (!btTextEdit->isEnabled()) {
      btTextEdit->setEnabled(true);
   }

   int pos = 0;
   if (lineEdit_step->text().isEmpty()) {
      pos = listWidget->count() + 1;
   } else {
      pos = lineEdit_step->text().toInt();
      lineEdit_step->clear();
   }

   qDebug() << Q_FUNC_INFO << "Inserting instruction '" << lineEdit_name->text() << "' at posistion" << pos;
   auto instruction = std::make_shared<Instruction>();
   instruction->setName(lineEdit_name->text());
   ObjectStoreWrapper::insert(instruction);
   lineEdit_name->clear();

   pos = qBound(1, pos, this->m_recIns.size());
   this->m_recObs->m_instructions.insert(instruction, pos);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->m_recObs);

   listWidget->setCurrentRow(pos - 1);
   return;
}

void BrewDayScrollWidget::acceptChanges(QMetaProperty prop, QVariant /*value*/) {
   if (m_recObs && QString(prop.name()) == PropertyNames::Recipe::instructions) {
      // An instruction has been added or deleted, so update internal list.
      for (auto ins : m_recIns ) {
         disconnect(ins.get(), nullptr, this, nullptr);
      }
      m_recIns = this->m_recObs->m_instructions.items(); // Already sorted by instruction numbers.
      for (auto ins : m_recIns ) {
         connect(ins.get(), &Instruction::changed, this, &BrewDayScrollWidget::acceptInsChanges);
      }
      showChanges();
   }
   return;
}

void BrewDayScrollWidget::acceptInsChanges(QMetaProperty prop, QVariant /*value*/) {
   QString propName = prop.name();
   if (propName == "instructionNumber") {
      // The order changed, so resort our internal list.
      std::sort(m_recIns.begin(), m_recIns.end());
      showChanges();
   } else if (propName == PropertyNames::Instruction::directions) {
      // This will make the displayed text directions update.
      listWidget->setCurrentRow(listWidget->currentRow());
   }
   return;
}

void BrewDayScrollWidget::clear() {
   listWidget->clear();
   return;
}

void BrewDayScrollWidget::showChanges() {
   this->clear();
   if (this->m_recObs == nullptr) {
      return;
   }

   this->repopulateListWidget();
   return;
}

void BrewDayScrollWidget::repopulateListWidget() {
   this->listWidget->clear();

   if (this->m_recObs == nullptr) {
      return;
   }

   for (auto ins : this->m_recIns ) {
      //QString text = tr("Step %1: %2").arg(i).arg(ins->name());
      QString text = tr("Step %1: %2").arg(ins->stepNumber()).arg(ins->name());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if (this->m_recIns.size() > 0 ) {
      this->listWidget->setCurrentRow(0);
   } else {
      this->listWidget->setCurrentRow(-1);
   }
   return;
}

QString BrewDayScrollWidget::buildTitleTable(bool includeImage) {
   // Do the style sheet first
   if (this->m_cssName == nullptr) {
      this->m_cssName = ":/css/brewday.css";
   }

   QString header = Html::createHeader(BrewDayScrollWidget::tr("Brewday"), m_cssName);

   QString body = QString("<h1>%1</h1>").arg(m_recObs->name());
   if (includeImage) {
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");
   }

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>")
         .arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg(styleName(m_recObs->style().get()));
   body += QString("<td class=\"right\">%1</td>")
         .arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   // second row:  boil time and efficiency.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Time"))
            .arg(boilTime(m_recObs->equipment().get()))
            .arg(tr("Efficiency"))
            .arg(Measurement::displayQuantity(m_recObs->efficiency_pct(), 0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Measurement::displayAmount(Measurement::Amount{m_recObs->boilVolume_l(), Measurement::Units::liters}, 2))
            .arg(tr("Preboil Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{m_recObs->boilGrav(), Measurement::Units::specificGravity}, 3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Measurement::displayAmount(Measurement::Amount{m_recObs->finalVolume_l(), Measurement::Units::liters}, 2))
            .arg(tr("Starting Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{m_recObs->og(), Measurement::Units::specificGravity}, 3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( Measurement::displayQuantity(m_recObs->IBU(), 1))
            .arg(tr("Final Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{m_recObs->fg(), Measurement::Units::specificGravity}, 3));

   // sixth row: ABV and estimate calories
   bool metricVolume = (
      Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume) ==
      Measurement::UnitSystems::volume_Metric
   );
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg(Measurement::displayQuantity(m_recObs->ABV_pct(), 1) )
            .arg(metricVolume ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg(Measurement::displayQuantity(metricVolume ? this->m_recObs->caloriesPer33cl() : this->m_recObs->caloriesPerUs12oz(), 0) );

   body += "</table>";

   return header + body;
}

QString BrewDayScrollWidget::buildInstructionTable() {
   QString middle = QString("<h2>%1</h2>").arg(tr("Instructions"));
   middle += QString("<table id=\"steps\">");
   middle += QString("<tr><th class=\"check\">%1</th><th class=\"time\">%2</th><th class=\"step\">%3</th></tr>")
         .arg(tr("Completed"))
         .arg(tr("Time"))
         .arg(tr("Step"));

   auto instructions = this->m_recObs->m_instructions.items();
   auto mashSteps = this->m_recObs->mash()->mashSteps();
   int size = instructions.size();
   for (int i = 0; i < size; ++i ) {

      auto ins = instructions[i];

      QString stepTime;
      if (ins->interval() > 0.0 ) {
         stepTime = Measurement::displayAmount(Measurement::Amount{ins->interval(), Measurement::Units::minutes}, 0);
      } else {
         stepTime = "--";
      }

      QString tmp = "";

      // TODO: comparing ins->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      QList<QString> reagents;
      if ( ins->name() == tr("Add grains") ) {
         reagents = this->m_recObs->getReagents( this->m_recObs->fermentableAdditions() );
      } else if ( ins->name() == tr("Heat water") ) {
         reagents = this->m_recObs->getReagents( this->m_recObs->mash()->mashSteps() );
      } else {
         reagents = ins->reagents();
      }

      if ( reagents.size() > 1 ) {
         tmp = QString("<ul>");
         for (int j = 0; j < reagents.size(); j++ ) {
            tmp += QString("<li>%1</li>")
                   .arg(reagents.at(j));
         }
         tmp += QString("</ul>");
      } else if ( reagents.size() == 1 ) {
         tmp = reagents.at(0);
      } else {
         tmp = ins->directions();
      }

      QString altTag = i % 2 ? "alt" : "norm";

      middle += QString("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
               .arg(altTag)
               .arg(stepTime)
               .arg(ins->name())
               .arg(tmp);
   }
   middle += "</table>";

   return middle;
}

QString BrewDayScrollWidget::buildFooterTable() {
   QString bottom = QString("<table id=\"notes\">");
   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></td><td class=\"right\">%2:</td><td class=\"value\"></td></tr>")
         .arg(tr("Actual PreBoil Volume"))
         .arg(tr("Actual PreBoil Gravity"));

   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></td><td class=\"right\">%2:</td><td class=\"value\"></td></tr>")
         .arg(tr("PostBoil Volume"))
         .arg(tr("PostBoil Gravity"));

   bottom += QString("<tr><td class=\"left\">%1:</td><td class=\"value\"></tr>")
         .arg(tr("Volume into fermenter"));
   bottom += "</table>";

   return bottom;
}
