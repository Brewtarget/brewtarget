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
#include "BrewDayWidget.h"
#include <QListWidgetItem>
#include "InstructionWidget.h"
#include "TimerWidget.h"

BrewDayWidget::BrewDayWidget(QWidget* parent) : QWidget(parent), Observer()
{
   setupUi(this);

   recObs = 0;

   // Stick some timers at the top.
   horizontalLayout_timers->insertWidget(0, new TimerWidget());
   horizontalLayout_timers->insertWidget(1, new TimerWidget());

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
   if( notifier != recObs || info.toInt() != Recipe::INSTRUCTION )
      return;

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
      QString text = QString("Step %1: %2").arg(i).arg(recObs->getInstruction(i)->getName());
      listWidget->addItem(new QListWidgetItem(text));
   }

   if( size > 0 )
      listWidget->setCurrentRow(0);
   else
      listWidget->setCurrentRow(-1);
}