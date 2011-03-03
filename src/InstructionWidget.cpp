/*
 * InstructionWidget.cpp is part of Brewtarget, and is Copyright Philip G. Lee
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
#include "InstructionWidget.h"
#include <iostream>

InstructionWidget::InstructionWidget(QWidget* parent) : QWidget(parent)
{
   setupUi(this);

   insObs = 0;
   timer = new TimerWidget(this);
   timer->setVisible(false);

   connect( checkBox_showTimer, SIGNAL(stateChanged(int)), this, SLOT(setHasTimer()) );
   connect( checkBox_completed, SIGNAL(stateChanged(int)), this, SLOT(setCompleted()) );
   connect( timer, SIGNAL(timerSet(QString)), this, SLOT(setTimerValue(QString)) );
   connect( textEdit, SIGNAL(textChanged()), this, SLOT(setDirections()) );
}

InstructionWidget::~InstructionWidget()
{
   setObserved(0);
   delete timer;
}

QSize InstructionWidget::sizeHint() const
{
   return QSize(0,0);
}

void InstructionWidget::setInstruction(Instruction* ins)
{
   insObs = ins;
   setObserved(insObs);
   showChanges();
}

void InstructionWidget::showChanges()
{
   if( insObs == 0 )
      return;

   textEdit->setPlainText(insObs->getDirections());
   checkBox_showTimer->setCheckState( insObs->getHasTimer() ? Qt::Checked : Qt::Unchecked );
   checkBox_completed->setCheckState( insObs->getCompleted() ? Qt::Checked : Qt::Unchecked );
}

void InstructionWidget::notify(Observable* /*notifier*/, QVariant /*info*/)
{
   // Do nothing since we should be the only ones changing the observed.
}

void InstructionWidget::setCompleted()
{
   if( insObs == 0 )
      return;

   bool completed = (checkBox_completed->checkState() == Qt::Checked)? true : false;
   insObs->setCompleted( completed );

   // Want to inactivate certain things sometimes.
   if( completed )
   {
      // Gray out everything except checkBox_completed.
      textEdit->setEnabled(false);
      checkBox_showTimer->setEnabled(false);
      timer->setEnabled(false);
   }
   else
   {
      textEdit->setEnabled(true);
      checkBox_showTimer->setEnabled(true);
      timer->setEnabled(true);
   }
}

void InstructionWidget::setHasTimer()
{
   if( insObs == 0 )
      return;

   insObs->setHasTimer( (checkBox_showTimer->checkState() == Qt::Checked)? true : false );

   if( insObs->getHasTimer() )
   {
      timer->setTimer(insObs->getTimerValue());
      verticalLayout->insertWidget(1,timer);
      timer->setVisible(true);
      verticalLayout->update(); // Shouldn't have to do this, but if I don't,
                                // then, the layout screws up.
   }
   else
   {
      verticalLayout->removeWidget(timer);
      timer->setVisible(false);
      verticalLayout->update();
   }
}

void InstructionWidget::setTimerValue(QString value)
{
   if( insObs == 0 )
      return;

   insObs->setTimerValue(value);
}

void InstructionWidget::setDirections()
{
   if( insObs == 0 )
      return;

   insObs->setDirections(textEdit->toPlainText());
}
