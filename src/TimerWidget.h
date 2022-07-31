/*
 * TimerWidget.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2022:
 * - Aidan Roberts <aidanr67@gmail.com>
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
#ifndef TIMERWIDGET_H
#define TIMERWIDGET_H
#pragma once

#include <QDialog>
#include <QSoundEffect>
#include <QPalette>

#include "ui_timerWidget.h"

#include "boiltime.h"

class TimerMainDialog;

/*!
 * \class TimerWidget
 *
 * \brief Individual boil addition timers
 */
class TimerWidget : public QDialog, public Ui::timerWidget {
   Q_OBJECT

public:
   TimerWidget(TimerMainDialog *parent, BoilTime* bt = 0);
   ~TimerWidget();
   void setTime(int t);
   void setNote(QString n);
   void setBoil(BoilTime* bt);
   void reset();
   int getTime();
   QString getNote();
   void cancel();
   void stopAlarm();
   void setAlarmLimits(bool l, unsigned int a);

private slots:
   // The names of most of these slots need to correspond with UI elements in ui/timerWidget.ui
   void on_setSoundButton_clicked();
   void on_setTimeBox_valueChanged(int t);
   void decrementTime();
   void on_stopButton_clicked();
   void on_cancelButton_clicked();
   void on_playButton_clicked();

private:
//   Ui::timerWidget *ui;
   TimerMainDialog* mainTimer;
   QPalette paletteOld, paletteNew;
   bool oldColors;
   BoilTime* boilTime;
   bool started; // Used to automatically start timers if main timer is running
   bool stopped; // Used to flash LCDNumber if time has elapsed
   /**
    * This will be stored as time to addition, not addition time ie. 50min for a 10min addition in a 60min boil - not 10min
    */
   unsigned int time;
   bool limitAlarmRing;
   unsigned int alarmRingLimit;
   QSoundEffect* soundPlayer;

   void updateTime();
   void timesUp();
   void flash();
   void startAlarm(bool loop = true);
   void setSound(QString s);
   void setDefualtAlarmSound();
   void reject();

};

#endif
