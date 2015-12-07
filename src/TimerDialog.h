/*
 * TimerListDialog.h is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
 * - Philip Greggory Lee <rocketman768@gmail.com>
 * - Aidan Roberts <aidanr67@gmail.com>
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

#ifndef TIMERDIALOG_H
#define TIMERDIALOG_H

class TimerDialog;

#include <QDialog>
#include "boiltime.h"
#include "ui_timerDialog.h"
#include "TimerListDialog.h"
#ifndef NO_QTMULTIMEDIA
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QPalette>
#endif

/*!
 * \class TimerDialog
 * \author Aidan Roberts
 *
 * \brief Individual boil addition timers
 */

class TimerDialog : public QDialog, public Ui::TimerDialog
{
    Q_OBJECT

public:
    TimerDialog(TimerListDialog *parent, BoilTime* bt = 0);
    ~TimerDialog();
    void setTime(int t);
    void setNote(QString n);
    void setBoil(BoilTime* bt);
    void reset();
    int getTime();
    QString getNote();
    void cancel();
    void hideTimer();
    void showTimer();
    void stopAlarm();
    void setAlarmLimits(bool l, unsigned int a);

private slots:
    void on_setSoundButton_clicked();
    void on_setTimeBox_valueChanged(int t);
    void decrementTime();
    void on_stopButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::TimerDialog *ui;
    TimerListDialog* mainTimer;
    QPalette paletteOld, paletteNew;
    bool oldColors;
    BoilTime* boilTime;
    bool started; //Used to automatically start timers if main timer is running
    bool stopped; //Used to flash LCDNumber if time has elapsed
    unsigned int time; /*This will be stored as time to addition, not addition time
                         ie. 50min for a 10min addition in a 60min boil - not 10min
                        */
    bool limitAlarmRing;
    unsigned int alarmRingLimit;
#ifndef NO_QTMULTIMEDIA
   QMediaPlayer* mediaPlayer;
   QMediaPlaylist* playlist;
#endif
   QByteArray timerPosition;

    void updateTime();
    void timesUp();
    QString timeToString(int t);
    void flash();
    void startAlarm();
    void setSound(QString s);
    void setDefualtAlarmSound();

};

#endif // TIMERDIALOG_H
