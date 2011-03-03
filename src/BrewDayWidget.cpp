/*
 * BrewDayWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "BrewDayWidget.h"
#include <QListWidgetItem>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QVector>
#include <QDir>
#include "InstructionWidget.h"
#include "TimerWidget.h"

BrewDayWidget::BrewDayWidget(QWidget* parent) : QWidget(parent), Observer()
{
   setupUi(this);
   recObs = 0;

   // HAVE to do this since apparently the stackedWidget NEEDS at least 1
   // widget at all times.
   stackedWidget->insertWidget(0, new InstructionWidget(stackedWidget) );
   stackedWidget->widget(0)->setVisible(false);
   stackedWidget->removeWidget(stackedWidget->widget(1));

   connect( listWidget, SIGNAL(currentRowChanged(int)), stackedWidget, SLOT(setCurrentIndex(int)) );
   connect( pushButton_insert, SIGNAL(clicked()), this, SLOT(insertInstruction()) );
   connect( pushButton_remove, SIGNAL(clicked()), this, SLOT(removeSelectedInstruction()) );
   connect( pushButton_up, SIGNAL(clicked()), this, SLOT(pushInstructionUp()) );
   connect( pushButton_down, SIGNAL(clicked()), this, SLOT(pushInstructionDown()) );


   // Set up the printer stuff
   doc = new QWebView();
   printer = new QPrinter;
   printer->setPageSize(QPrinter::Letter);

   // populate the drop down list

}

QSize BrewDayWidget::sizeHint() const
{
   return QSize(0,0);
}

void BrewDayWidget::removeSelectedInstruction()
{
   if( recObs == 0 )
      return;

   int row = listWidget->currentRow();
   if( row < 0 )
      return;
   recObs->removeInstruction(recObs->getInstruction(row));
}

void BrewDayWidget::pushInstructionUp()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();
   if( row <= 0 )
      return;
   
   recObs->swapInstructions(row, row-1);
   listWidget->setCurrentRow(row-1);
}

void BrewDayWidget::pushInstructionDown()
{
   if( recObs == 0 )
      return;
   
   int row = listWidget->currentRow();
   if( row >= listWidget->count() )
      return;
   
   recObs->swapInstructions(row, row+1);
   listWidget->setCurrentRow(row+1);
}

QString BrewDayWidget::getCSS() 
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

QString BrewDayWidget::buildTitleTable()
{
   QString header;
   QString body;

   // Do the style sheet first
   header = "<html><head><style type=\"text/css\">";
   header += getCSS();
   header += "</style></head>";

   body   = "<body>";
   body += QString("<h1>%1</h1>").arg(recObs->getName());
   body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>").arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg(recObs->getStyle()->getName());
   body += QString("<td class=\"right\">%1</td>").arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Volume"))
           .arg(Brewtarget::displayAmount(recObs->getBoilSize_l(),Units::liters,2))
           .arg(tr("Preboil Gravity"))
           .arg(Brewtarget::displayOG(recObs->getBoilGrav()));

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Final Volume"))
           .arg(Brewtarget::displayAmount(recObs->getBatchSize_l(), Units::liters,2))
           .arg(tr("Starting Gravity"))
           .arg(Brewtarget::displayOG(recObs->getOg()));

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Time"))
           .arg(Brewtarget::displayAmount(recObs->getBoilTime_min(),Units::minutes))
           .arg(tr("IBU"))
           .arg(recObs->getIBU(),0,'f',1);

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
           .arg(tr("Predicted Efficiency"))
           .arg(Brewtarget::displayAmount(recObs->getEfficiency_pct(),0,0))
           .arg(tr("Estimated calories (per 12 oz)"))
           .arg(Brewtarget::displayAmount(recObs->estimateCalories(),0,0));

   body += "</table>";

   return header + body;

}

QString BrewDayWidget::buildInstructionTable()
{
   QString middle;
   int i, j, size;

   middle += QString("<h2>%1</h2>").arg(tr("Instructions"));
   middle += "<table id=\"steps\">";
   middle += QString("<tr><th class=\"check\">%1</th><th class=\"time\">%2</th><th class=\"step\">%3</th></tr>")
             .arg(tr("Completed"))
             .arg(tr("Time"))
             .arg(tr("Step"));

   size = recObs->getNumInstructions();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QVector<QString> reagents;

      if (recObs->getInstruction(i)->getInterval())
         stepTime = Brewtarget::displayAmount(recObs->getInstruction(i)->getInterval(), Units::minutes, 0);
      else
         stepTime = "--";

      tmp = "";
      reagents = recObs->getInstruction(i)->getReagents();
      if ( reagents.size() > 1 ) {
         tmp = "<ul>";
         for ( j = 0; j < reagents.size(); j++ ) 
         {
            tmp += QString("<li>%1</li>")
                   .arg(reagents[j]);
         }
         tmp += "</ul>";
      }
      else {
         tmp = reagents[0];
      }

      QString altTag = i % 2 ? "alt" : "norm";

      middle += QString("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
               .arg(altTag)
               .arg(stepTime)
               .arg(recObs->getInstruction(i)->getName())
               .arg(tmp);
   }
   middle += "</table>";

   return middle;
}

QString BrewDayWidget::buildFooterTable()
{
   QString bottom;

   bottom = "<table id=\"notes\">";
   bottom += QString("<tr><td class=\"left\">%1</td><td class=\"value\"></td><td class=\"right\">%2</td><td class=\"value\"></td></tr>")
             .arg(tr("Actual Pre-boil Volume:"))
             .arg(tr("Actual Pre-boil Gravity:"));

   bottom += QString("<tr><td class=\"left\">%1</td><td class=\"value\"></td><td class=\"right\">%2</td><td class=\"value\"></td></tr>")
             .arg(tr("Post-boil Volume:"))
             .arg(tr("Post-boil Gravity:"));

   bottom += QString("<tr><td class=\"left\">%1</td><td class=\"value\"></tr>")
             .arg(tr("Volume in fermenter:"));
   bottom += "</table>";

   return bottom;
}

bool BrewDayWidget::loadComplete(bool ok) 
{
   doc->print(printer);
   disconnect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );
   return ok;
}

void BrewDayWidget::pushInstructionPrint()
{
   QString pDoc;
   QPrintDialog *dialog = new QPrintDialog(printer, this);

   /* Instantiate the Webview and then connect its signal */
   connect( doc, SIGNAL(loadFinished(bool)), this, SLOT(loadComplete(bool)) );

   dialog->setWindowTitle(tr("Print Document"));
   if (dialog->exec() != QDialog::Accepted)
      return;

   if( recObs == 0 )
      return;

   // Start building the document to be printed.  I think.
   pDoc = buildTitleTable();
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += QString("<h2>%1</h2>").arg(tr("Notes"));
   pDoc += "</body></html>";

   doc->setHtml(pDoc);
}

void BrewDayWidget::pushInstructionPreview()
{
   QString pDoc;

   if( recObs == 0 )
      return;

   // Start building the document to be printed.  I think.
   pDoc = buildTitleTable();
   pDoc += buildInstructionTable();
   pDoc += buildFooterTable();

   pDoc += QString("<h2>%1</h2>").arg(tr("Notes"));
   pDoc += "</body></html>";

   doc->setHtml(pDoc);
   doc->show();
}

void BrewDayWidget::setRecipe(Recipe* rec)
{
   recObs = rec;
   setObserved(recObs);
   showChanges();
}

void BrewDayWidget::insertInstruction()
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

void BrewDayWidget::notify(Observable* notifier, QVariant info)
{
   /*
   if( notifier != recObs || info.toInt() != Recipe::INSTRUCTION )
      return;
   */

   if( notifier == recObs && info.toInt() == Recipe::INSTRUCTION )
      showChanges();
}

void BrewDayWidget::clear()
{
   listWidget->clear();

   while( stackedWidget->count() > 0 )
   {
      InstructionWidget* iw = (InstructionWidget*)stackedWidget->widget(0);
      stackedWidget->removeWidget(iw);
      delete iw;
   }

   stackedWidget->setCurrentIndex(0);
}

void BrewDayWidget::showChanges()
{
   clear();
   if( recObs == 0 )
   {
      //clear();
      return;
   }

   int i, size;
   InstructionWidget* iw;
   size = recObs->getNumInstructions();

   for( i = 0; i < size; ++i )
   {
      if(stackedWidget->widget(i) == 0)
      {
         iw = new InstructionWidget(stackedWidget);
         stackedWidget->addWidget(iw);
      }
      else
      {
         iw = (InstructionWidget*)stackedWidget->widget(i);
         iw->setVisible(true);
      }

      iw->setInstruction(recObs->getInstruction(i));
   }

   stackedWidget->update(); // Whatever, I give up.
   repopulateListWidget();
}

void BrewDayWidget::repopulateListWidget()
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
