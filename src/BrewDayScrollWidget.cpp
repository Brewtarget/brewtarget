/*
 * BrewDayScrollWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "instruction.h"
#include "brewtarget.h"
#include "BrewDayScrollWidget.h"
#include "database.h"
#include "Html.h"
#include <QListWidgetItem>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QVector>
#include <QDir>
#include "InstructionWidget.h"
#include "TimerWidget.h"
#include "style.h"
#include "equipment.h"
#include "mash.h"

BrewDayScrollWidget::BrewDayScrollWidget(QWidget* parent)
   : QWidget(parent), doc(new QTextBrowser())
{
   setupUi(this);
   setObjectName("BrewDayScrollWidget");
   recObs = 0;

   connect( listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showInstruction(int)) );
   // connect( plainTextEdit, SIGNAL(textChanged()), this, SLOT(saveInstruction()) );
   connect(btTextEdit,SIGNAL(textModified()), this, SLOT(saveInstruction()));
   connect( pushButton_insert, SIGNAL(clicked()), this, SLOT(insertInstruction()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeSelectedInstruction()) );
   connect( pushButton_up, SIGNAL(clicked()), this, SLOT(pushInstructionUp()) );
   connect( pushButton_down, SIGNAL(clicked()), this, SLOT(pushInstructionDown()) );
   connect( pushButton_generateInstructions, SIGNAL(clicked()), this, SLOT(generateInstructions()) );
}

void BrewDayScrollWidget::saveInstruction()
{
   recObs->instructions()[ listWidget->currentRow() ]->setDirections( btTextEdit->toPlainText() );
}

void BrewDayScrollWidget::showInstruction(int insNdx)
{
   if( recObs == 0 )
      return;

   int size = recIns.size();
   if( insNdx < 0 || insNdx >= size )
      return;

   // Block signals to avoid setPlainText() from triggering saveInstruction().
   btTextEdit->setPlainText((recIns[insNdx])->directions());
}

void BrewDayScrollWidget::generateInstructions()
{
   if( recObs == 0 )
      return;

   if(!btTextEdit->isEnabled())
      btTextEdit->setEnabled(true);

   recObs->generateInstructions();
}

QSize BrewDayScrollWidget::sizeHint() const
{
   return QSize(0,0);
}

void BrewDayScrollWidget::removeSelectedInstruction()
{
   if( recObs == 0 )
      return;

   int row = listWidget->currentRow();
   if( row < 0 )
      return;
   recObs->remove(recIns[row]);

   if(recIns.isEmpty())
   {
      btTextEdit->clear();
      btTextEdit->setEnabled(false);
   }
}

void BrewDayScrollWidget::pushInstructionUp()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();
   if( row <= 0 )
      return;
   
   recObs->swapInstructions(recIns[row], recIns[row-1]);
   listWidget->setCurrentRow(row-1);
}

void BrewDayScrollWidget::pushInstructionDown()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();

   if( row >= listWidget->count() - 1 || row < 0 )
      return;
   
   recObs->swapInstructions(recIns[row], recIns[row+1]);
   listWidget->setCurrentRow(row+1);
}

bool BrewDayScrollWidget::loadComplete(bool ok) 
{
   doc->print(printer);
   disconnect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   return ok;
}

void BrewDayScrollWidget::print(QPrinter *mainPrinter,
      int action, QFile* outFile)
{
   QString pDoc;

   if( recObs == 0 )
      return;

   /* Connect the webview's signal */
   if ( action == PRINT )
   {
      printer = mainPrinter;
   }

   // Start building the document to be printed.  The HTML doesn't work with
   // the image since it is a compiled resource
   pDoc = buildTitleTable( action != HTML );
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += tr("<h2>Notes</h2>");
   if ( recObs->notes() != "" )
      pDoc += QString("<div id=\"customNote\">%1</div>\n").arg(recObs->notes());

   pDoc += "</body></html>";

   doc->setHtml(pDoc);
   if ( action == PREVIEW )
      doc->show();
   else if ( action == HTML )
   {
      QTextStream out(outFile);
      out << pDoc;
      outFile->close();
   }
   else
   {
       loadComplete(true);
   }
}

void BrewDayScrollWidget::setRecipe(Recipe* rec)
{
   // Disconnect old notifier.
   if( recObs )
      disconnect( recObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(acceptChanges(QMetaProperty,QVariant)) );
   
   recObs = rec;
   connect( recObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(acceptChanges(QMetaProperty,QVariant)) );
   
   recIns = recObs->instructions();
   foreach( Instruction* ins, recIns )
         connect( ins, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(acceptInsChanges(QMetaProperty,QVariant)) );
   
   btTextEdit->clear();
   if(recIns.isEmpty())
      btTextEdit->setEnabled(false);
   else
      btTextEdit->setEnabled(true);

   showChanges();
}

void BrewDayScrollWidget::insertInstruction()
{
   if( recObs == 0 )
      return;

   if(!btTextEdit->isEnabled())
      btTextEdit->setEnabled(true);

   int pos = 0;
   if(lineEdit_step->text().isEmpty())
      pos = listWidget->count() + 1;
   else
   {
      pos = lineEdit_step->text().toInt();
      lineEdit_step->clear();
   }
   Instruction* ins = Database::instance().newInstruction(recObs);

   pos = qBound(1, pos, recIns.size());

   ins->setName(lineEdit_name->text());
   lineEdit_name->clear();

   recObs->insertInstruction( ins, pos );
   listWidget->setCurrentRow(pos-1);
}

void BrewDayScrollWidget::acceptChanges(QMetaProperty prop, QVariant /*value*/)
{
   if( recObs && QString(prop.name()) == "instructions" )
   {
      // An instruction has been added or deleted, so update internal list.
      foreach( Instruction* ins, recIns )
         disconnect( ins, 0, this, 0 );
      recIns = recObs->instructions(); // Already sorted by instruction numbers.
      foreach( Instruction* ins, recIns )
         connect( ins, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(acceptInsChanges(QMetaProperty,QVariant)) );
      showChanges();
   }
}

void BrewDayScrollWidget::acceptInsChanges(QMetaProperty prop, QVariant /*value*/)
{
   QString propName = prop.name();
   
   if( propName == "instructionNumber" )
   {
      // The order changed, so resort our internal list.
      qSort( recIns.begin(), recIns.end(), insPtrLtByNumber );
      showChanges();
   }
   else if( propName == "directions" )
   {
      // This will make the displayed text directions update.
      listWidget->setCurrentRow( listWidget->currentRow() );
   }
}

void BrewDayScrollWidget::clear()
{
   listWidget->clear();
}

void BrewDayScrollWidget::showChanges()
{
   clear();
   if( recObs == 0 )
      return;

   repopulateListWidget();
}

void BrewDayScrollWidget::repopulateListWidget()
{
   listWidget->clear();

   if( recObs == 0 )
      return;
   
   foreach( Instruction* ins, recIns )
   {
      //QString text = tr("Step %1: %2").arg(i).arg(ins->name());
      QString text = tr("Step %1: %2").arg(ins->instructionNumber()).arg(ins->name());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if( recIns.size() > 0 )
      listWidget->setCurrentRow(0);
   else
      listWidget->setCurrentRow(-1);
}

static QString styleName(Style* style)
{
   if ( ! style )
   {
      return "unknown";
   }
   else
   {
      return style->name();
   }
}

static QString boilTime(Equipment* equipment)
{
   if ( ! equipment )
   {
      return "unknown";
   }
   else
   {
      return Brewtarget::displayAmount(equipment->boilTime_min(), "tab_recipe", "boilTime_min", Units::minutes);
   }
}

QString BrewDayScrollWidget::buildTitleTable(bool includeImage)
{
   QString header;
   QString body;

   // Do the style sheet first
   if (cssName == NULL)
      cssName = ":/css/brewday.css";

   header = Html::createHeader(BrewDayScrollWidget::tr("Brewday"), cssName);

   body = QString("<h1>%1</h1>").arg(recObs->name());
   if ( includeImage )
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");

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
            .arg(Brewtarget::displayAmount(recObs->efficiency_pct(),0,0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Brewtarget::displayAmount(recObs->boilVolume_l(), "tab_recipe", "boilVolume_l", Units::liters,2))
            .arg(tr("Preboil Gravity"))
            .arg(Brewtarget::displayAmount(recObs->boilGrav(), "tab_recipe", "og", Units::sp_grav, 3));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Brewtarget::displayAmount(recObs->finalVolume_l(), "tab_recipe", "finalVolume_l", Units::liters,2))
            .arg(tr("Starting Gravity"))
            .arg(Brewtarget::displayAmount(recObs->og(), "tab_recipe", "og", Units::sp_grav, 3));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( Brewtarget::displayAmount(recObs->IBU(),0,1))
            .arg(tr("Final Gravity"))
            .arg(Brewtarget::displayAmount(recObs->fg(), "tab_recipe", "fg", Units::sp_grav, 3));

   // sixth row: ABV and estimate calories
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg( Brewtarget::displayAmount(recObs->ABV_pct(),0,1) )
            .arg( Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
            .arg( Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? recObs->calories33cl() : recObs->calories12oz(),0,0) );

   body += "</table>";

   return header + body;

}

QString BrewDayScrollWidget::buildInstructionTable()
{
   QString middle;
   int i, j, size;

   middle += QString("<h2>%1</h2>").arg(tr("Instructions"));
   middle += QString("<table id=\"steps\">");
   middle += QString("<tr><th class=\"check\">%1</th><th class=\"time\">%2</th><th class=\"step\">%3</th></tr>")
         .arg(tr("Completed"))
         .arg(tr("Time"))
         .arg(tr("Step"));

   QList<Instruction*> instructions = recObs->instructions();
   QList<MashStep*> mashSteps = recObs->mash()->mashSteps();
   size = instructions.size();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QList<QString> reagents;
      
      Instruction* ins = instructions[i];

      if (ins->interval())
         stepTime = Brewtarget::displayAmount(ins->interval(), Units::minutes, 0);
      else
         stepTime = "--";

      tmp = "";

      // TODO: comparing ins->name() with these untranslated strings means this
      // doesn't work in other languages. Find a better way.
      if ( ins->name() == tr("Add grains") )
         reagents = recObs->getReagents( recObs->fermentables() );
      else if ( ins->name() == tr("Heat water") )
         reagents = recObs->getReagents( recObs->mash()->mashSteps() );
      else 
         reagents = ins->reagents();

      if ( reagents.size() > 1 )
      {
         tmp = QString("<ul>");
         for ( j = 0; j < reagents.size(); j++ )
         {
            tmp += QString("<li>%1</li>")
                   .arg(reagents.at(j));
         }
         tmp += QString("</ul>");
      }
      else if ( reagents.size() == 1 )
      {
         tmp = reagents.at(0);
      }
      else
      {
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

QString BrewDayScrollWidget::buildFooterTable()
{
   QString bottom;

   bottom = QString("<table id=\"notes\">");
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
