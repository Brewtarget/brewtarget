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
#include "TimerWidget.h"
#include "boiltime.h"
#include <QTimer>
#include <QThread>
#include <QString>
#include <QDebug>

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
      TimerListDialog(QWidget* parent=0);
      ~TimerListDialog();

      
private slots:
      void on_addTimerButton_clicked();
      void on_startButton_clicked();
      void on_stopButton_clicked();
      void on_Reset_clicked();
      void on_setBoilTimeBox_valueChanged(int t);
      void decrementTimer();

private:
      QList<TimerWidget*> * timers;
      BoilTime* boilTime = new BoilTime(this);
      QTimer* timer;
      QThread* timerThread;

      void updateTime();
      QString timeToString(int t);
      void placeNewTimerWidget(TimerWidget* tw);
};

#endif
