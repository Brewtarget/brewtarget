/*
 * InstructionWidget.cpp is part of Brewtarget, and is Copyright the following
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
#include "InstructionWidget.h"
#include "TimerWidget.h"

InstructionWidget::InstructionWidget(QWidget* parent) :
   QWidget(parent), insObs(0)
{
   setupUi(this);
   timer->setVisible(false);

   connect( checkBox_completed, &QCheckBox::stateChanged, this, &InstructionWidget::setCompleted );
   connect( timer, SIGNAL(timerSet(QString)), this, SLOT(setTimerValue(QString)) );
   connect( textEdit, &QTextEdit::textChanged, this, &InstructionWidget::setDirections );
}

InstructionWidget::~InstructionWidget()
{
}

QSize InstructionWidget::sizeHint() const
{
   return QSize(0,0);
}

void InstructionWidget::setInstruction(Instruction* ins)
{
   if( insObs )
      disconnect( insObs, 0, this, 0 );
   
   insObs = ins;
   if( insObs )
      connect( insObs, SIGNAL(changed(QMetaProperty,QVariant)), this, SLOT(changed(QMetaProperty,QVariant)) );

   showChanges();
}

void InstructionWidget::showChanges()
{
   if( insObs == 0 )
      return;

   textEdit->setPlainText(insObs->directions());
   checkBox_showTimer->setCheckState( insObs->hasTimer() ? Qt::Checked : Qt::Unchecked );
   checkBox_completed->setCheckState( insObs->completed() ? Qt::Checked : Qt::Unchecked );
}

void InstructionWidget::changed(QMetaProperty prop, QVariant /*val*/)
{
   if( sender() != insObs )
      return;
   
   showChanges();
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

/*void InstructionWidget::setHasTimer()
{
   if( insObs == 0 )
      return;

   insObs->setHasTimer( (checkBox_showTimer->checkState() == Qt::Checked)? true : false );

   if( insObs->hasTimer() )
   {
      timer->setTimer(insObs->timerValue());
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
}*/

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
