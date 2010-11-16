/*
 * BrewDayWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
 * (rocketman768@gmail.com), 2009.
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
   connect( pushButton_print, SIGNAL(clicked()), this, SLOT(pushInstructionPrint()) );
   connect( pushButton_preview, SIGNAL(clicked()), this, SLOT(pushInstructionPreview()) );
   connect( comboBox_template, SIGNAL(currentIndexChanged(QString)), this, SLOT(comboSetCSS(QString)) );

   // Set up the printer stuff
   doc = new QWebView();
   printer = new QPrinter;
   printer->setPageSize(QPrinter::Letter);

   // populate the drop down list
   populateComboBox(comboBox_template);
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

void BrewDayWidget::comboSetCSS(const QString name) 
{
   cssName = QString(":/css/%1").arg(name);
}

QString BrewDayWidget::getCSS() 
{
   if ( cssName == NULL )
       cssName = tr(":/css/brewday.css");

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
   body += tr("<h1>%1</h1>").arg(recObs->getName().c_str());
   body += tr("<img src=\"%1\" />").arg("qrc:/images/title.svg");

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += tr("<tr><td class=\"left\">Style</td>");
   body += tr("<td class=\"value\">%1</td>")
           .arg(recObs->getStyle()->getName().c_str());
   body += tr("<td class=\"right\">Date</td>");
   body += tr("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   body += tr("<tr><td class=\"left\">Boil Volume</td><td class=\"value\">%1</td><td class=\"right\">Preboil Gravity</td><td class=\"value\">%2</td></tr>")
           .arg(Brewtarget::displayAmount(recObs->getBoilSize_l(),Units::liters,2))
           .arg(Brewtarget::displayOG(recObs->getBoilGrav()));

   body += tr("<tr><td class=\"left\">Final Volume</td><td class=\"value\">%1</td><td class=\"right\">Starting Gravity</td><td class=\"value\">%2</td></tr>")
           .arg(Brewtarget::displayAmount(recObs->getBatchSize_l(), Units::liters,2))
           .arg(Brewtarget::displayOG(recObs->getOg()));

   body += tr("<tr><td class=\"left\">Boil Time</td><td class=\"value\">%1</td><td class=\"right\">IBU</td><td class=\"value\">%2</td></tr>")
           .arg(Brewtarget::displayAmount(recObs->getBoilTime_min(),Units::minutes))
           .arg(recObs->getIBU(),0,'f',1);

   body += tr("<tr><td class=\"left\">Predicted Efficiency</td><td class=\"value\">%1</td><td class=\"right\">Estimated calories(per 12 oz )</td><td class=\"value\">%2</tr>")
           .arg(Brewtarget::displayAmount(recObs->getEfficiency_pct(),0,0))
           .arg(Brewtarget::displayAmount(recObs->estimateCalories(),0,0));

   body += "</table>";

   return header + body;

}

QString BrewDayWidget::buildInstructionTable()
{
   QString middle;
   int i, j, size;

   middle += "<h2>Instructions</h2>";
   middle += "<table id=\"steps\">";
   middle += tr("<tr><th class=\"check\">Completed</th><th class=\"time\">Time</th><th class=\"step\">Step</th></tr>");

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
         tmp = tr("<ul>");
         for ( j = 0; j < reagents.size(); j++ ) 
         {
            tmp += tr("<li>%1</li>")
                   .arg(reagents[j]);
         }
         tmp += tr("</ul>");
      }
      else {
         tmp = reagents[0];
      }

      QString altTag = i % 2 ? "alt" : "norm";

      middle += tr("<tr class=\"%1\"><td class=\"check\"></td><td class=\"time\">%2</td><td align=\"step\">%3 : %4</td></tr>")
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
   bottom += tr("<tr><td class=\"left\">Actual PreBoil Volume:</td><td class=\"value\"></td><td class=\"right\">Actual PreBoil Gravity:</td><td class=\"value\"></td></tr>");

   bottom += tr("<tr><td class=\"left\">PostBoil Volume:</td><td class=\"value\"></td><td class=\"right\">PostBoil Gravity:</td><td class=\"value\"></td></tr>");

   bottom += tr("<tr><td class=\"left\">Volume into fermenter:</td><td class=\"value\"></tr>");
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

   pDoc += tr("<h2>Notes</h2>");
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

   pDoc += tr("<h2>Notes</h2>");
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


void BrewDayWidget::populateComboBox(QComboBox *comboBox_template)
{
   QDir css(":/css");
   QFileInfoList cssList;
   QFileInfo fileInfo;
   int i;

   css.setFilter(QDir::Files);
   cssList = css.entryInfoList();

   for ( i = 0; i < cssList.size(); i++) 
   {  
      fileInfo = cssList.at(i);
      comboBox_template->addItem(tr("%1").arg(fileInfo.fileName()));
   }

   comboBox_template->setEditable(false);
}
