/*
 * InstructionWidget.h is part of Brewtarget, and is Copyright Philip G. Lee
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

#ifndef _INGREDIENTWIDGET_H
#define	_INGREDIENTWIDGET_H

class InstructionWidget;

#include "ui_instructionWidget.h"
#include "observable.h"
#include "instruction.h"
#include <QWidget>
#include <QSize>
#include "TimerWidget.h"

class InstructionWidget : public QWidget, public Ui::instructionWidget, public Observer
{
   Q_OBJECT
public:
   InstructionWidget(QWidget* parent=0);
   ~InstructionWidget();
   void setInstruction(Instruction* ins);

   virtual QSize sizeHint() const; // From QWidget
   virtual void notify(Observable *notifier, QVariant info); // From Observer

public slots:
   void setDirections();
   void setHasTimer();
   void setTimerValue(QString value);
   void setCompleted();

private:
   void showChanges();
   void makeEverythingInactive();
   
   Instruction* insObs;
   TimerWidget* timer;
};

#endif	/* _INGREDIENTWIDGET_H */

