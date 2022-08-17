/*
 * BrewDayScrollWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
 * - Matt Young <mfsy@yahoo.com>
 * - Mik Firestone <mikfire@gmail.com>
 * - Philip Greggory Lee <rocketman768@gmail.com>
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
#include "BrewDayScrollWidget.h"

#include <QDate>
#include <QListWidgetItem>
#include <QPrinter>
#include <QVector>

#include "database/ObjectStoreWrapper.h"
#include "Html.h"
#include "InstructionWidget.h"
#include "measurement/Measurement.h"
#include "measurement/UnitSystem.h"
#include "model/Equipment.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Style.h"
#include "PersistentSettings.h"
#include "TimerWidget.h"

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

      return Measurement::displayAmount(Measurement::Amount{equipment->boilTime_min(), Measurement::Units::minutes},
                                        PersistentSettings::Sections::tab_recipe,
                                        PropertyNames::Recipe::boilTime_min);
   }
}


BrewDayScrollWidget::BrewDayScrollWidget(QWidget* parent) : QWidget{parent},
                                                            recObs{nullptr} {
   this->setupUi(this);
   this->setObjectName("BrewDayScrollWidget");

   connect(listWidget,                      &QListWidget::currentRowChanged, this, &BrewDayScrollWidget::showInstruction          );
   connect(btTextEdit,                      SIGNAL(textModified()),          this, SLOT(saveInstruction())                        );
//   connect(btTextEdit,                      &BtLineEdit::textModified,       this, &BrewDayScrollWidget::saveInstruction          );
   connect(pushButton_insert,               &QAbstractButton::clicked,       this, &BrewDayScrollWidget::insertInstruction        );
   connect(pushButton_remove,               &QAbstractButton::clicked,       this, &BrewDayScrollWidget::removeSelectedInstruction);
   connect(pushButton_up,                   &QAbstractButton::clicked,       this, &BrewDayScrollWidget::pushInstructionUp        );
   connect(pushButton_down,                 &QAbstractButton::clicked,       this, &BrewDayScrollWidget::pushInstructionDown      );
   connect(pushButton_generateInstructions, &QAbstractButton::clicked,       this, &BrewDayScrollWidget::generateInstructions     );

   return;
}

BrewDayScrollWidget::~BrewDayScrollWidget() = default;

void BrewDayScrollWidget::saveInstruction() {
  this->recObs->instructions()[ listWidget->currentRow() ]->setDirections( btTextEdit->toPlainText() );
}

void BrewDayScrollWidget::showInstruction(int insNdx) {
   if (this->recObs == nullptr) {
      return;
   }

   int size = recIns.size();
   if (insNdx < 0 || insNdx >= size) {
      return;
   }

   // Block signals to avoid setPlainText() from triggering saveInstruction().
   btTextEdit->setPlainText((recIns[insNdx])->directions());
}

void BrewDayScrollWidget::generateInstructions() {
   if (this->recObs == nullptr) {
      return;
   }

   if (!btTextEdit->isEnabled()) {
      btTextEdit->setEnabled(true);
   }

  this->recObs->generateInstructions();
}

QSize BrewDayScrollWidget::sizeHint() const {
   return QSize(0,0);
}

void BrewDayScrollWidget::removeSelectedInstruction() {
   if (this->recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row < 0) {
      return;
   }
   this->recObs->remove(ObjectStoreWrapper::getSharedFromRaw(this->recIns[row]));

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->recObs);

   if (this->recIns.isEmpty()) {
      btTextEdit->clear();
      btTextEdit->setEnabled(false);
   } else {
      if (row > this->recIns.size()) {
         row = this->recIns.size();
      }
      listWidget->setCurrentRow(row);
   }

   return;
}

void BrewDayScrollWidget::pushInstructionUp() {
   if (this->recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row <= 0) {
      return;
   }

   this->recObs->swapInstructions(this->recIns[row], this->recIns[row-1]);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->recObs);

   listWidget->setCurrentRow(row-1);
   return;
}

void BrewDayScrollWidget::pushInstructionDown() {
   if (this->recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row >= listWidget->count() - 1 || row < 0) {
      return;
   }

   this->recObs->swapInstructions(this->recIns[row], this->recIns[row+1]);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->recObs);

   listWidget->setCurrentRow(row+1);
   return;
}

bool BrewDayScrollWidget::loadComplete(bool ok) {
   this->doc->print(this->printer);
   return ok;
}

void BrewDayScrollWidget::print(QPrinter *mainPrinter, int action, QFile* outFile) {
   if (this->recObs == nullptr) {
      return;
   }

   // Connect the webview's signal
   if (action == PRINT) {
      this->printer = mainPrinter;
   }

   // Start building the document to be printed.  The HTML doesn't work with
   // the image since it is a compiled resource
   QString pDoc = buildTitleTable(action != HTML);
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += tr("<h2>Notes</h2>");
   if (this->recObs->notes() != "" )
      pDoc += QString("<div id=\"customNote\">%1</div>\n").arg(recObs->notes());

   pDoc += "</body></html>";

   this->doc->setHtml(pDoc);
   if (action == PREVIEW) {
      this->doc->show();
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
   if (this->recObs) {
      disconnect(this->recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges );
   }

   this->recObs = rec;
   connect(this->recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges);

   recIns = this->recObs->instructions();
   for (Instruction* ins : recIns) {
      connect(ins, &Instruction::changed, this, &BrewDayScrollWidget::acceptInsChanges);
   }

   btTextEdit->clear();
   if (recIns.isEmpty()) {
      btTextEdit->setEnabled(false);
   } else {
      btTextEdit->setEnabled(true);
   }

   showChanges();
   return;
}

void BrewDayScrollWidget::insertInstruction() {
   if (this->recObs == nullptr) {
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
   auto ins = std::make_shared<Instruction>();
   ins->setName(lineEdit_name->text());
   ObjectStoreWrapper::insert(ins);
   lineEdit_name->clear();

   pos = qBound(1, pos, this->recIns.size());
   this->recObs->insertInstruction(*ins.get(), pos);

   // After updating the model, this is the simplest way to update the display
   this->setRecipe(this->recObs);

   listWidget->setCurrentRow(pos-1);
   return;
}

void BrewDayScrollWidget::acceptChanges(QMetaProperty prop, QVariant /*value*/) {
   if (recObs && QString(prop.name()) == "instructions") {
      // An instruction has been added or deleted, so update internal list.
      foreach( Instruction* ins, recIns ) {
         disconnect(ins, nullptr, this, nullptr);
      }
      recIns =this->recObs->instructions(); // Already sorted by instruction numbers.
      foreach( Instruction* ins, recIns ) {
         connect(ins, &Instruction::changed, this, &BrewDayScrollWidget::acceptInsChanges);
      }
      showChanges();
   }
   return;
}

void BrewDayScrollWidget::acceptInsChanges(QMetaProperty prop, QVariant /*value*/) {
   QString propName = prop.name();
   if (propName == "instructionNumber") {
      // The order changed, so resort our internal list.
      std::sort(recIns.begin(), recIns.end(), insPtrLtByNumber);
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
   if (this->recObs == nullptr) {
      return;
   }

   repopulateListWidget();
   return;
}

void BrewDayScrollWidget::repopulateListWidget() {
   this->listWidget->clear();

   if (this->recObs == nullptr) {
      return;
   }

   foreach( Instruction* ins, this->recIns ) {
      //QString text = tr("Step %1: %2").arg(i).arg(ins->name());
      QString text = tr("Step %1: %2").arg(ins->instructionNumber()).arg(ins->name());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if (this->recIns.size() > 0 ) {
      this->listWidget->setCurrentRow(0);
   } else {
      this->listWidget->setCurrentRow(-1);
   }
   return;
}

QString BrewDayScrollWidget::buildTitleTable(bool includeImage) {
   // Do the style sheet first
   if (this->cssName == nullptr) {
      this->cssName = ":/css/brewday.css";
   }

   QString header = Html::createHeader(BrewDayScrollWidget::tr("Brewday"), cssName);

   QString body = QString("<h1>%1</h1>").arg(recObs->name());
   if (includeImage) {
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");
   }

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>")
         .arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg(styleName(recObs->style()));
   body += QString("<td class=\"right\">%1</td>")
         .arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   // second row:  boil time and efficiency.
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Time"))
            .arg(boilTime(recObs->equipment()))
            .arg(tr("Efficiency"))
            .arg(Measurement::displayQuantity(recObs->efficiency_pct(), 0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Measurement::displayAmount(Measurement::Amount{recObs->boilVolume_l(), Measurement::Units::liters},
                                            PersistentSettings::Sections::tab_recipe,
                                            PropertyNames::Recipe::boilVolume_l,
                                            2))
            .arg(tr("Preboil Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{recObs->boilGrav(), Measurement::Units::sp_grav},
                                            PersistentSettings::Sections::tab_recipe,
                                            PropertyNames::Recipe::og,
                                            3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Measurement::displayAmount(Measurement::Amount{recObs->finalVolume_l(), Measurement::Units::liters},
                                            PersistentSettings::Sections::tab_recipe,
                                            PropertyNames::Recipe::finalVolume_l,
                                            2))
            .arg(tr("Starting Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{recObs->og(), Measurement::Units::sp_grav},
                                            PersistentSettings::Sections::tab_recipe,
                                            PropertyNames::Recipe::og,
                                            3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( Measurement::displayQuantity(recObs->IBU(), 1))
            .arg(tr("Final Gravity"))
            .arg(Measurement::displayAmount(Measurement::Amount{recObs->fg(), Measurement::Units::sp_grav},
                                            PersistentSettings::Sections::tab_recipe,
                                            PropertyNames::Recipe::fg,
                                            3));

   // sixth row: ABV and estimate calories
   bool metricVolume = (
      Measurement::getDisplayUnitSystem(Measurement::PhysicalQuantity::Volume) ==
      Measurement::UnitSystems::volume_Metric
   );
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg(Measurement::displayQuantity(recObs->ABV_pct(), 1) )
            .arg(metricVolume ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg(Measurement::displayQuantity(metricVolume ? this->recObs->calories33cl() : this->recObs->calories12oz(), 0) );

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

   QList<Instruction*> instructions = this->recObs->instructions();
   auto mashSteps = this->recObs->mash()->mashSteps();
   int size = instructions.size();
   for (int i = 0; i < size; ++i ) {

      Instruction* ins = instructions[i];

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
         reagents = this->recObs->getReagents( this->recObs->fermentables() );
      } else if ( ins->name() == tr("Heat water") ) {
         reagents = this->recObs->getReagents( this->recObs->mash()->mashSteps() );
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
