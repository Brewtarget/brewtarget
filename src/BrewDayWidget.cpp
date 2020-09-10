/*
 * BrewDayWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Kregg K <gigatropolis@yahoo.com>
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

#include <QListWidgetItem>
#include <QPrinter>
#include <QPrintDialog>
#include <QDate>
#include <QVector>
#include <QDir>
#include <QDebug>
#include "database.h"
#include "InstructionWidget.h"
#include "TimerWidget.h"
#include "instruction.h"
#include "brewtarget.h"
#include "BrewDayWidget.h"
#include "recipe.h"
#include "style.h"

// NOTE: QPrinter has no parent? Will it get destroyed properly?
BrewDayWidget::BrewDayWidget(QWidget* parent) :
   QWidget(parent), recObs(0), printer(new QPrinter()), doc(new QTextBrowser(this))
{
   setupUi(this);

   setObjectName("BrewDayWidget");
   // HAVE to do this since apparently the stackedWidget NEEDS at least 1
   // widget at all times.
   stackedWidget->insertWidget(0, new InstructionWidget(stackedWidget) );
   stackedWidget->widget(0)->setVisible(false);
   stackedWidget->removeWidget(stackedWidget->widget(1));

   connect( listWidget, &QListWidget::currentRowChanged, stackedWidget, &QStackedWidget::setCurrentIndex );
   connect( pushButton_insert, &QAbstractButton::clicked, this, &BrewDayWidget::insertInstruction );
   connect( pushButton_remove, &QAbstractButton::clicked, this, &BrewDayWidget::removeSelectedInstruction );
   connect( pushButton_up, &QAbstractButton::clicked, this, &BrewDayWidget::pushInstructionUp );
   connect( pushButton_down, &QAbstractButton::clicked, this, &BrewDayWidget::pushInstructionDown );


   // Set up the printer stuff
   printer->setPageSize(QPrinter::Letter);

   // populate the drop down list

}

QSize BrewDayWidget::sizeHint() const
{
   return QSize(0,0);
}

void BrewDayWidget::insertInstruction()
{
   if( recObs == 0 )
      return;

   int pos = lineEdit_step->text().toInt();
   int size = recObs->instructions().size();
   if( pos < 0 || pos > size )
      pos = size;

   Instruction* ins = Database::instance().newInstruction(recObs);
   ins->setName(lineEdit_name->text());

   // TODO: figure out how to do ordering of ingredients.
   recObs->insertInstruction( ins, pos );
   //listWidget->insertItem(pos, ins->text(false));
   repopulateListWidget();
}

void BrewDayWidget::removeSelectedInstruction()
{
   if( recObs == 0 )
      return;

   int row = listWidget->currentRow();
   if( row < 0 )
      return;
   listWidget->takeItem(row);
   repopulateListWidget();
   recObs->remove(recObs->instructions()[row]);
}

void BrewDayWidget::pushInstructionUp()
{
   if( recObs == 0 )
      return;
   
   QList<Instruction*> ins = recObs->instructions();
   int row = listWidget->currentRow();
   if( row <= 0 )
      return;
   
   recObs->swapInstructions(ins[row], ins[row-1]);
   QString instrStep = listWidget->item(row)->text();
   listWidget->insertItem(row, listWidget->item(row-1)->text());
   listWidget->insertItem(row-1, instrStep);
   listWidget->setCurrentRow(row-1);
   //repopulateListWidget();
}

void BrewDayWidget::pushInstructionDown()
{
   if( recObs == 0 )
      return;
   
   QList<Instruction*> ins = recObs->instructions();
   int row = listWidget->currentRow();
   if( row >= listWidget->count() )
      return;
   
   recObs->swapInstructions(ins[row], ins[row+1]);
  QString instrStep = listWidget->item(row+1)->text();
   listWidget->insertItem(row+1, listWidget->item(row)->text());
  listWidget->insertItem(row, instrStep);
   listWidget->setCurrentRow(row+1);
  // repopulateListWidget();
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
   body += QString("<h1>%1</h1>").arg(recObs->name());
   body += QString("<img src=\"%1\" />").arg("qrc:/images/title.svg");

   // Build the top table
   // Build the first row: Style and Date
   body += "<table id=\"title\">";
   body += QString("<tr><td class=\"left\">%1</td>").arg(tr("Style"));
   body += QString("<td class=\"value\">%1</td>")
           .arg(recObs->style()->name());
   body += QString("<td class=\"right\">%1</td>").arg(tr("Date"));
   body += QString("<td class=\"value\">%1</td></tr>")
           .arg(QDate::currentDate().toString());

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Volume"))
           .arg(Brewtarget::displayAmount(recObs->boilSize_l(),Units::liters,2))
           .arg(tr("Preboil Gravity"))
           .arg(Brewtarget::displayAmount(recObs->boilGrav(), Units::sp_grav, 3));

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Final Volume"))
           .arg(Brewtarget::displayAmount(recObs->batchSize_l(), Units::liters,2))
           .arg(tr("Starting Gravity"))
           .arg(Brewtarget::displayAmount(recObs->og(), Units::sp_grav, 3));

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</td></tr>")
           .arg(tr("Boil Time"))
           .arg(Brewtarget::displayAmount(recObs->boilTime_min(),Units::minutes))
           .arg(tr("IBU"))
           .arg(Brewtarget::displayAmount(recObs->IBU(),0,1));

   body += QString("<tr><td class=\"left\">%1</td><td class=\"value\">%2</td><td class=\"right\">%3</td><td class=\"value\">%4</tr>")
           .arg(tr("Predicted Efficiency"))
           .arg(Brewtarget::displayAmount(recObs->efficiency_pct(),0,0))
           .arg(Brewtarget::getVolumeUnitSystem() == SI ? tr("Estimated calories (per 33 cl)") : tr("Estimated calories (per 12 oz)"))
           .arg(Brewtarget::displayAmount(Brewtarget::getVolumeUnitSystem() == SI ? recObs->calories33cl() : recObs->calories12oz(),0,0));

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

   QList<Instruction*> instructions = recObs->instructions();
   size = instructions.size();
   for( i = 0; i < size; ++i )
   {
      QString stepTime, tmp;
      QList<QString> reagents;

      if(instructions[i]->interval())
         stepTime = Brewtarget::displayAmount(instructions[i]->interval(), Units::minutes, 0);
      else
         stepTime = "--";

      tmp = "";
      reagents = instructions[i]->reagents();
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
               .arg(instructions[i]->name())
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
   return ok;
}

void BrewDayWidget::pushInstructionPrint()
{
   QString pDoc;
   QPrintDialog *dialog = new QPrintDialog(printer, this);

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
   loadComplete(true);
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
   if( recObs )
      disconnect( recObs, 0, this, 0 );
   
   recObs = rec;
   if( recObs )
      connect( recObs, &Ingredient::changed, this, &BrewDayWidget::changed );
   
   showChanges();
}


void BrewDayWidget::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() == recObs &&
       QString(prop.name()) == "instructions")
      showChanges();
}

void BrewDayWidget::clear()
{
   listWidget->clear();

   while( stackedWidget->count() > 0 )
   {
      InstructionWidget* iw = (InstructionWidget*)stackedWidget->widget(0);
      stackedWidget->removeWidget(iw);
         iw->deleteLater();
   }

   stackedWidget->setCurrentIndex(0);
}

void BrewDayWidget::showChanges()
{
   clear();
   if( recObs == 0 )
      return;

   int i, size;
   InstructionWidget* iw;
   QList<Instruction*> instructions = recObs->instructions();
   size = instructions.size();

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

      iw->setInstruction(instructions[i]);
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
   QList<Instruction*> instructions = recObs->instructions();
   size = instructions.size();

   for( i = 0; i < size; ++i )
   {
      QString text = tr("Step %1: %2").arg(i).arg(instructions[i]->name());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if( size > 0 )
      listWidget->setCurrentRow(0);
   else
      listWidget->setCurrentRow(-1);
   this->setUpdatesEnabled(true);
   listWidget->update();
}
