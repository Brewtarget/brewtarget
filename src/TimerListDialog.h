/*
 * TimerListDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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
#ifndef TIMERLISTDIALOG_H
#define TIMERLISTDIALOG_H

class TimerListDialog;

#include "ui_timerListDialog.h"
#include <QDialog>
#include <QWidget>
#include "TimerDialog.h"
#include "boiltime.h"
#include "recipe.h"
#include "hop.h"
#include <QString>
#include <QDebug>
#include "MainWindow.h"
#include <QStack>

/*!
 * \class TimerListDialog
 * \author Aidan Roberts
 *
 * \brief Main boil timer, generate timers from recipe
 */
class TimerListDialog : public QDialog, public Ui::timerListDialog
{
   Q_OBJECT
   
public:
      TimerListDialog(MainWindow* parent);
      ~TimerListDialog();
      void removeTimer(TimerDialog* t);

      
private slots:
      void on_addTimerButton_clicked();
      void on_startButton_clicked();
      void on_stopButton_clicked();
      void on_setBoilTimeBox_valueChanged(int t);
      void decrementTimer();
      void on_hideButton_clicked();
      void on_showButton_clicked();
      void timesUp();
      void on_resetButton_clicked();
      void on_loadRecipesButton_clicked();
      void on_cancelButton_clicked();
      //Overload QDialog::reject()
      void reject();

private:
      MainWindow* mainWindow;
      QList<TimerDialog*> * timers;
      BoilTime* boilTime;
      QStack<int>* timerPositions;

      void removeAllTimers();
      void resetTimers();
      void setInitialTimerPosition();
      void updateTime();
      QString timeToString(int t);
      void positionNewTimer(TimerDialog* t);
};

#endif
