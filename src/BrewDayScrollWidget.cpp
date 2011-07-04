/*
 * BrewDayScrollWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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

#include "instruction.h"
#include "brewtarget.h"
#include "BrewDayScrollWidget.h"
#include <QListWidgetItem>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QVector>
#include <QDir>
#include "InstructionWidget.h"
#include "TimerWidget.h"

BrewDayScrollWidget::BrewDayScrollWidget(QWidget* parent) : QWidget(parent), Observer()
{
   setupUi(this);
   recObs = 0;

   connect( listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showInstruction(int)) );
   connect( plainTextEdit, SIGNAL(textChanged()), this, SLOT(saveInstruction()) );
   connect( pushButton_insert, SIGNAL(clicked()), this, SLOT(insertInstruction()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeSelectedInstruction()) );
   connect( pushButton_up, SIGNAL(clicked()), this, SLOT(pushInstructionUp()) );
   connect( pushButton_down, SIGNAL(clicked()), this, SLOT(pushInstructionDown()) );
   connect( pushButton_generateInstructions, SIGNAL(clicked()), this, SLOT(generateInstructions()) );

   doc = new QWebView();
}

void BrewDayScrollWidget::saveInstruction()
{
   // Need to disable notification to avoid a possible infinite loop.
   recObs->disableNotification();
   recObs->getInstruction( listWidget->currentRow() )->setDirections( plainTextEdit->toPlainText() );
   recObs->reenableNotification();
}

void BrewDayScrollWidget::showInstruction(int insNdx)
{
   if( recObs == 0 )
      return;

   int size = recObs->getNumInstructions();
   if( insNdx < 0 || insNdx >= size )
      return;

   plainTextEdit->setPlainText(recObs->getInstruction(insNdx)->getDirections());
}

void BrewDayScrollWidget::generateInstructions()
{
   if( recObs == 0 )
      return;

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
   recObs->removeInstruction(recObs->getInstruction(row));
}

void BrewDayScrollWidget::pushInstructionUp()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();
   if( row <= 0 )
      return;
   
   recObs->swapInstructions(row, row-1);
   listWidget->setCurrentRow(row-1);
}

void BrewDayScrollWidget::pushInstructionDown()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();
   if( row >= listWidget->count() )
      return;
   
   recObs->swapInstructions(row, row+1);
   listWidget->setCurrentRow(row+1);
}

QString BrewDayScrollWidget::getCSS() 
{
   if ( cssName == NULL )
       cssName = ":/css/brewday.css";

   QFile cssInput(cssName);
   QString css;

   if (cssInput.open(QFile::ReadOnly)) {
      QTextStream inStream(&cssInput);
      while ( ! inStream.atEnd() )
      {
         css += inStream.readLine();
      }
   }
   return css;
}

QString BrewDayScrollWidget::buildTitleTable(bool includeImage)
{
   QString header;
   QString body;

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   body += QString("<h1>%1</h1>").arg(recObs->getName());
   if ( includeImage )
      body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>")
         .arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg(recObs->getStyle()->getName());
   body += QString("<td class=\"right\">%1</td>")
         .arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   // second row:  boil time and efficiency.  
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Time"))
            .arg(Brewtarget::displayAmount(recObs->getBoilTime_min(),Units::minutes))
            .arg(tr("Efficiency"))
            .arg(Brewtarget::displayAmount(recObs->getEfficiency_pct(),0,0));

   // third row: pre-Boil Volume and Preboil Gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Boil Volume"))
            .arg(Brewtarget::displayAmount(recObs->getBoilSize_l(),Units::liters,2))
            .arg(tr("Preboil Gravity"))
            .arg(Brewtarget::displayOG(recObs->getBoilGrav()));

   // fourth row: Final volume and starting gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
            .arg(tr("Final Volume"))
            .arg(Brewtarget::displayAmount(recObs->getBatchSize_l(), Units::liters,2))
            .arg(tr("Starting Gravity"))
            .arg(Brewtarget::displayOG(recObs->getOg(), true));

   // fifth row: IBU and Final gravity
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("IBU"))
            .arg( recObs->getIBU(),0,'f',1)
            .arg(tr("Final Gravity"))
            .arg(Brewtarget::displayFG(recObs->getFg(), recObs->getOg(), true));

   // sixth row: ABV and estimate calories
   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2%</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
            .arg(tr("ABV"))
            .arg( recObs->getABV_pct(), 0, 'f', 1)
            .arg(tr("Estimated calories(per 12 oz)"))
            .arg( recObs->estimateCalories(), 0, 'f', 0);
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

   size = recObs->getNumInstructions();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QVector<QString> reagents;
      Instruction* ins = recObs->getInstruction(i);

      if (ins->getInterval())
         stepTime = Brewtarget::displayAmount(ins->getInterval(), Units::minutes, 0);
      else
         stepTime = "--";

      tmp = "";
      reagents = ins->getReagents();

      if ( ins->getName() == "Add grains")
      {
          Instruction* temp = recObs->getMashFermentable();
          reagents = temp->getReagents();
      }
      else if ( ins->getName() == "Heat water")
      {
         int mashSize = recObs->getMash()->getNumMashSteps();
         Instruction* temp = recObs->getMashWater(mashSize);
         reagents = temp->getReagents();

      }

      if ( reagents.size() > 1 )
      {
         tmp = QString("<ul>");
         for ( j = 0; j < reagents.size(); j++ )
         {
            tmp += QString("<li>%1</li>")
                   .arg(reagents[j]);
         }
         tmp += QString("</ul>");
      }
      else
      {
         tmp = reagents[0];
      }

      QString altTag = i % 2 ? "alt" : "norm";

      middle += QString("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
               .arg(altTag)
               .arg(stepTime)
               .arg(ins->getName())
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

bool BrewDayScrollWidget::loadComplete(bool ok) 
{
   doc->print(printer);
   disconnect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   return ok;
}

void BrewDayScrollWidget::print(QPrinter *mainPrinter, QPrintDialog* dialog,
      int action, QFile* outFile)
{
   QString pDoc;
//   QPrintDialog *dialog = new QPrintDialog(printer, this);

   if( recObs == 0 )
      return;

   /* Connect the webview's signal */
   if ( action == PRINT )
   {
      printer = mainPrinter;
      connect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );

      dialog->setWindowTitle(tr("Print Document"));
      if (dialog->exec() != QDialog::Accepted)
         return;
   }

   // Start building the document to be printed.  The HTML doesn't work with
   // the image since it is a compiled resource
   pDoc = buildTitleTable( action != HTML );
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += tr("<h2>Notes</h2>");
   if ( recObs->getNotes() != "" )
      pDoc += QString("%1").arg(recObs->getNotes());

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
}

void BrewDayScrollWidget::setRecipe(Recipe* rec)
{
   recObs = rec;
   setObserved(recObs);
   showChanges();
}

void BrewDayScrollWidget::insertInstruction()
{
   if( recObs == 0 )
      return;

   int pos = lineEdit_step->text().toInt();
   Instruction* ins = new Instruction();

   if( pos < 0 || pos > recObs->getNumInstructions() )
      pos = recObs->getNumInstructions();

   ins->setName(lineEdit_name->text());

   recObs->insertInstruction( ins, pos );
}

void BrewDayScrollWidget::notify(Observable* notifier, QVariant info)
{
   /*
   if( notifier != recObs || info.toInt() != Recipe::INSTRUCTION )
      return;
   */

   if( notifier == recObs && info.toInt() == Recipe::INSTRUCTION )
      showChanges();
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

   int i, size;
   size = recObs->getNumInstructions();

   for( i = 0; i < size; ++i )
   {
      QString text = tr("Step %1: %2").arg(i).arg(recObs->getInstruction(i)->getName());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if( size > 0 )
      listWidget->setCurrentRow(0);
   else
      listWidget->setCurrentRow(-1);
}
