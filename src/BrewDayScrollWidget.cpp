/*
 * BrewDayScrollWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
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

#include "brewtarget.h"
#include "database/ObjectStoreWrapper.h"
#include "Html.h"
#include "InstructionWidget.h"
#include "model/Equipment.h"
#include "model/Instruction.h"
#include "model/Mash.h"
#include "model/Style.h"
#include "PersistentSettings.h"
#include "TimerWidget.h"

namespace {
   QString styleName(Style* style) {
      if ( ! style ) {
         return "unknown";
      } else {
         return style->name();
      }
   }

   QString boilTime(Equipment* equipment) {
      if ( ! equipment ) {
         return "unknown";
      } else {
         return Brewtarget::displayAmount(equipment->boilTime_min(),
                                       PersistentSettings::Sections::tab_recipe,
                                       PropertyNames::Recipe::boilTime_min,
                                       &Units::minutes);
      }
   }
}


BrewDayScrollWidget::BrewDayScrollWidget(QWidget* parent) : QWidget{parent},
                                                            recObs{nullptr},
                                                            btPrintPreview{new BtPrintPreview(this)} {
   this->setupUi(this);
   this->setObjectName("BrewDayScrollWidget");

   connect( listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showInstruction(int)) );
   connect(btTextEdit,SIGNAL(textModified()), this, SLOT(saveInstruction()));
   connect( pushButton_insert, SIGNAL(clicked()), this, SLOT(insertInstruction()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeSelectedInstruction()) );
   connect( pushButton_up, SIGNAL(clicked()), this, SLOT(pushInstructionUp()) );
   connect( pushButton_down, SIGNAL(clicked()), this, SLOT(pushInstructionDown()) );
   connect( pushButton_generateInstructions, SIGNAL(clicked()), this, SLOT(generateInstructions()) );
}

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
  this->recObs->remove(ObjectStoreWrapper::getSharedFromRaw(recIns[row]));

   if(recIns.isEmpty()) {
      btTextEdit->clear();
      btTextEdit->setEnabled(false);
   }
}

void BrewDayScrollWidget::pushInstructionUp() {
   if (this->recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();
   if (row <= 0) {
      return;
   }

  this->recObs->swapInstructions(recIns[row], recIns[row-1]);
   listWidget->setCurrentRow(row-1);
}

void BrewDayScrollWidget::pushInstructionDown() {
   if (this->recObs == nullptr) {
      return;
   }

   int row = listWidget->currentRow();

   if (row >= listWidget->count() - 1 || row < 0) {
      return;
   }

  this->recObs->swapInstructions(recIns[row], recIns[row+1]);
   listWidget->setCurrentRow(row+1);
}


void BrewDayScrollWidget::buildHtml(bool includeImage) {
   // Caller's responsibility to have checked this first
   Q_ASSERT(this->recObs != nullptr);

   // Start building the document to be printed.  The HTML doesn't work with
   // the image since it is a compiled resource
   QString pDoc = buildTitleTable(includeImage);
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += tr("<h2>Notes</h2>");
   if (this->recObs->notes() != "" )
      pDoc += QString("<div id=\"customNote\">%1</div>\n").arg(recObs->notes());

   pDoc += "</body></html>";

   this->btPrintPreview->setContent(pDoc);
   return;
}

void BrewDayScrollWidget::printPreview() {
   if (this->recObs == nullptr) {
      return;
   }
   this->buildHtml(true);
   this->btPrintPreview->show();
   return;
}

void BrewDayScrollWidget::print(QPrinter* printer) {
   if (this->recObs == nullptr) {
      return;
   }
   this->buildHtml(true);
   this->btPrintPreview->print(printer);
   return;
}

void BrewDayScrollWidget::exportHtml(QFile* file) {
   if (this->recObs == nullptr) {
      return;
   }
   this->buildHtml(false);
   this->btPrintPreview->exportHtml(file);
   return;
}

/*
void BrewDayScrollWidget::print(QPrinter *printer,
      int action, QFile* outFile)
{
   if(this->recObs == nullptr )
      return;


   this->doc->setHtml(pDoc);
   if ( action == PREVIEW ) {
      this->doc->adjustSize();
      this->doc->show();
   } else if ( action == HTML ) {
      QTextStream out(outFile);
      out << pDoc;
      outFile->close();
   }
   else
   {
       this->doc->print(printer);
   }
}
*/
void BrewDayScrollWidget::setRecipe(Recipe* rec) {
   // Disconnect old notifier.
   if (recObs) {
      disconnect(this->recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges );
   }

  this->recObs = rec;
   connect(this->recObs, &Recipe::changed, this, &BrewDayScrollWidget::acceptChanges );

   recIns =this->recObs->instructions();
   foreach( Instruction* ins, recIns )
         connect( ins, &Instruction::changed, this, &BrewDayScrollWidget::acceptInsChanges );

   btTextEdit->clear();
   if (recIns.isEmpty()) {
      btTextEdit->setEnabled(false);
   } else {
      btTextEdit->setEnabled(true);
   }

   showChanges();
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
   auto ins = std::make_shared<Instruction>();
   ins->setName(lineEdit_name->text());
   ObjectStoreWrapper::insert(ins);
   lineEdit_name->clear();

   pos = qBound(1, pos, this->recIns.size());
   this->recObs->insertInstruction(ins.get(), pos);
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
}

void BrewDayScrollWidget::acceptInsChanges(QMetaProperty prop, QVariant /*value*/) {
   QString propName = prop.name();
   if (propName == "instructionNumber") {
      // The order changed, so resort our internal list.
      std::sort(recIns.begin(), recIns.end(), insPtrLtByNumber);
      showChanges();
   } else if (propName == PropertyNames::Instruction::directions) {
      // This will make the displayed text directions update.
      listWidget->setCurrentRow( listWidget->currentRow() );
   }
}

void BrewDayScrollWidget::clear() {
   listWidget->clear();
}

void BrewDayScrollWidget::showChanges() {
   this->clear();
   if (this->recObs == nullptr) {
      return;
   }

   repopulateListWidget();
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
   if ( includeImage ) {
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
            .arg(Brewtarget::displayAmount(recObs->efficiency_pct(),nullptr,0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Brewtarget::displayAmount(recObs->boilVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::boilVolume_l, &Units::liters,2))
            .arg(tr("Preboil Gravity"))
            .arg(Brewtarget::displayAmount(recObs->boilGrav(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::og, &Units::sp_grav, 3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Brewtarget::displayAmount(recObs->finalVolume_l(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::finalVolume_l, &Units::liters,2))
            .arg(tr("Starting Gravity"))
            .arg(Brewtarget::displayAmount(recObs->og(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::og, &Units::sp_grav, 3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( Brewtarget::displayAmount(recObs->IBU(),nullptr,1))
            .arg(tr("Final Gravity"))
            .arg(Brewtarget::displayAmount(recObs->fg(), PersistentSettings::Sections::tab_recipe, PropertyNames::Recipe::fg, &Units::sp_grav, 3));

   // sixth row: ABV and estimate calories
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg( Brewtarget::displayAmount(recObs->ABV_pct(),nullptr,1) )
            .arg( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ?this->recObs->calories33cl() :this->recObs->calories12oz(),nullptr,0) );

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
   QList<MashStep*> mashSteps = this->recObs->mash()->mashSteps();
   int size = instructions.size();
   for(int i = 0; i < size; ++i ) {

      Instruction* ins = instructions[i];

      QString stepTime;
      if (ins->interval() > 0.0 ) {
         stepTime = Brewtarget::displayAmount(ins->interval(), &Units::minutes, 0);
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
