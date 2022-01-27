/*
 * InstructionWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2021
 * - Jeff Bailey <skydvr38@verizon.net>
 * - Matt Young <mfsy@yahoo.com>
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
#ifndef INSTRUCTIONWIDGET_H
#define INSTRUCTIONWIDGET_H
#pragma once

#include "ui_instructionWidget.h"
#include <QWidget>
#include <QSize>
#include <QMetaProperty>
#include <QVariant>

// Forward declarations.
class TimerWidget;
class Instruction;

/*!
 * \class InstructionWidget
 *
 * \brief View/controller widget that views/edits recipe instructions.
 */
class InstructionWidget : public QWidget, public Ui::instructionWidget
{
   Q_OBJECT
public:
   InstructionWidget(QWidget* parent=0);
   virtual ~InstructionWidget();
   //! View/edit the given instruction.
   void setInstruction(Instruction* ins);

   virtual QSize sizeHint() const; // From QWidget

public slots:
   void setDirections();
   void setTimerValue(QString value);
   void setCompleted();

   void changed(QMetaProperty,QVariant);
private:
   void showChanges();
   void makeEverythingInactive();

   Instruction* insObs;
   TimerWidget* timer;
};

#endif   /* _INGREDIENTWIDGET_H */
