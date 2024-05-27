/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * TimerMainDialog.h is part of Brewtarget, and is copyright the following authors 2009-2022:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
 *   • Philip Greggory Lee <rocketman768@gmail.com>
 *
 * Brewtarget is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Brewtarget is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 ╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌*/
#ifndef TIMERMAINDIALOG_H
#define TIMERMAINDIALOG_H
#pragma once

#include "ui_timerMainDialog.h"

#include <QDebug>
#include <QDialog>
#include <QStack>
#include <QString>
#include <QWidget>

class BoilTime;
class MainWindow;
class TimerWidget;
class TimerListDialog;

/*!
 * \class TimerMainDialog
 *
 * \brief Main boil timer, create timers individually or generate from recipe
 */
class TimerMainDialog : public QDialog, public Ui::TimerMainDialog {
   Q_OBJECT

public:
   TimerMainDialog(MainWindow* parent);
   ~TimerMainDialog();
   void removeTimer(TimerWidget* t);
   unsigned int getAlarmLimit();
   void hideTimers();
   void setTimerVisible(TimerWidget* t);
   void showTimers();

private slots:
   void on_addTimerButton_clicked();
   void on_startButton_clicked();
   void on_stopButton_clicked();
   void on_setBoilTimeBox_valueChanged(int t);
   void on_hideButton_clicked();
   void on_showButton_clicked();
   void on_resetButton_clicked();
   void on_loadRecipesButton_clicked();
   void on_cancelButton_clicked();
   void on_limitRingTimeCheckBox_clicked();
   void on_limitRingTimeSpinBox_valueChanged(int l);

   void decrementTimer();
   void timesUp();

private:
   void removeAllTimers();
   void resetTimers();
   void updateTime();
   QString timeToString(int t);
   void setRingLimits(bool l, unsigned int a);
   void sortTimers();
   TimerWidget* createNewTimer();
   void createTimer();
   void createTimer(QString n);
   void createTimer(QString n, int t);
   //Overload QDialog::reject()
   void reject();

   MainWindow* mainWindow; //To get currently selected recipe
   QList<TimerWidget*> * timers;
   TimerListDialog* timerWindow;
   BoilTime* boilTime;
   bool stopped;
   bool limitAlarmRing;
   unsigned int alarmLimit; // In seconds
};

#endif
