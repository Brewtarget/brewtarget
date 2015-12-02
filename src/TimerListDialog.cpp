/*
 * TimerListDialog.cpp is part of Brewtarget, and is Copyright the following
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

#include "TimerListDialog.h"

TimerListDialog::TimerListDialog(QWidget* parent) : QDialog(parent)
{
   setupUi(this);  
   boilTime->setBoilTime(setBoilTimeBox->value() * 60); //default 60mins
   timers = new QList<TimerDialog*>();
   connect(boilTime, SIGNAL(BoilTimeChanged()), this, SLOT(decrementTimer()));
   connect(boilTime, SIGNAL(timesUp()), this, SLOT(timesUp()));
   updateTime();
   stopButton->setEnabled(false);
   resetButton->setEnabled(false);
}

TimerListDialog::~TimerListDialog()
{
}

void TimerListDialog::on_addTimerButton_clicked()
{
   TimerDialog* newTimer = new TimerDialog(this, boilTime);
   timers->append(newTimer);
   newTimer->show();
}

void TimerListDialog::on_startButton_clicked()
{
    boilTime->startTimer();
    stopButton->setEnabled(true);
    resetButton->setEnabled(true);

}

void TimerListDialog::on_stopButton_clicked()
{
    boilTime->stopTimer();
    stopButton->setEnabled(false);
}

void TimerListDialog::on_resetButton_clicked()
{
    // Reset boil time to defined boil time
    boilTime->setBoilTime(setBoilTimeBox->value() * 60);
    updateTime();
    foreach (TimerDialog* t, *timers)
        t->reset();
    resetButton->setEnabled(false);
}

void TimerListDialog::on_setBoilTimeBox_valueChanged(int t)
{
    boilTime->setBoilTime(t * 60);
    updateTime();
}

void TimerListDialog::decrementTimer()
{
    if(!resetButton->isEnabled())
        resetButton->setEnabled(true);
    updateTime();
}

void TimerListDialog::updateTime()
{
   unsigned int time = boilTime->getTime();
   timeLCD->display(timeToString(time));
}

QString TimerListDialog::timeToString(int t)
{
    unsigned int seconds = t;
    unsigned int minutes = 0;
    unsigned int hours = 0;
   if (t == 0)
       return "00:00:00";
   if (t > 59){
       seconds = t%60;
       minutes = t/60;
       if (minutes > 59){
          hours = minutes/60;
          minutes = minutes%60;
       }
   }
   QString secStr, minStr, hourStr;
   if (seconds < 10)
       secStr = "0" + QString::number(seconds);
   else
       secStr = QString::number(seconds);
   if (minutes <10)
       minStr = "0" + QString::number(minutes);
   else
       minStr = QString::number(minutes);
   if (hours < 10)
       hourStr = "0" + QString::number(hours);
   else
       hourStr = QString::number(hours);
   return hourStr + ":" + minStr + ":" + secStr;
}


void TimerListDialog::on_hideButton_clicked()
{
    foreach (TimerDialog* t, *timers) {
        if (!t->isHidden())
            t->hide();
    }
}

void TimerListDialog::on_showButton_clicked()
{
    foreach (TimerDialog* t, *timers) {
        if (t->isHidden())
            t->show();
    }
}

void TimerListDialog::timesUp()
{
    //Do something cool
}
