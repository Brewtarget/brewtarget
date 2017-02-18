/*
 * TimerMainDialog.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2014
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

#include "TimerMainDialog.h"
#include <QMessageBox>
#include <QToolTip>
#include "unit.h"

TimerMainDialog::TimerMainDialog(MainWindow* parent) : QDialog(parent),
    mainWindow(parent),
    stopped(false),
    limitAlarmRing(false),
    alarmLimit(5) //default 5 seconds
{
   setupUi(this);

   boilTime = new BoilTime(this);
   boilTime->setBoilTime(setBoilTimeBox->value() * 60); //default 60mins
   timers = new QList<TimerWidget*>();
   timerWindow = new TimerListDialog(this, timers);
   updateTime();

   //Connections
   connect(boilTime, &BoilTime::BoilTimeChanged, this, &TimerMainDialog::decrementTimer);
   connect(boilTime, &BoilTime::timesUp, this, &TimerMainDialog::timesUp);

   retranslateUi(this);
}

TimerMainDialog::~TimerMainDialog()
{
}

void TimerMainDialog::on_addTimerButton_clicked()
{
    createTimer();
}

TimerWidget* TimerMainDialog::createNewTimer()
{
    TimerWidget* newTimer = new TimerWidget(this, boilTime);
    timers->append(newTimer);
    newTimer->setAlarmLimits(limitAlarmRing, alarmLimit);
    return newTimer;
}

void TimerMainDialog::createTimer()
{
    TimerWidget* newTimer = createNewTimer();
    sortTimers();
    showTimers();
    timerWindow->setTimerVisible(newTimer);
}

void TimerMainDialog::createTimer(QString n)
{
    TimerWidget* newTimer = createNewTimer();
    newTimer->setNote(n);
    sortTimers();
    showTimers();
    timerWindow->setTimerVisible(newTimer);
}

void TimerMainDialog::createTimer(QString n, int t)
{
    TimerWidget* newTimer = createNewTimer();
    newTimer->setNote(n);
    newTimer->setTime(t);
    sortTimers();
    showTimers();
    timerWindow->setTimerVisible(newTimer);
}

void TimerMainDialog::showTimers()
{
    if (timers->isEmpty()) {
        if (!timerWindow->isHidden())
            timerWindow->hide();
    }
    else {
        int x = timerWindow->x();
        int y = timerWindow->y();
        timerWindow->setAttribute(Qt::WA_DeleteOnClose);
        timerWindow->close();

        timerWindow = new TimerListDialog(this, timers);
        timerWindow->move(x, y);
        timerWindow->show();
    }
}

void TimerMainDialog::on_startButton_clicked()
{
    if (!boilTime->isStarted())
        boilTime->startTimer();
}

void TimerMainDialog::on_stopButton_clicked()
{
    if (boilTime->isStarted())
        boilTime->stopTimer();
}

void TimerMainDialog::on_resetButton_clicked()
{
    resetTimers();
}

void TimerMainDialog::resetTimers()
{
    // Reset boil time to defined boil time
    boilTime->setBoilTime(setBoilTimeBox->value() * 60);
    updateTime();
    // Reset all children timers
    if (!timers->isEmpty())
        foreach (TimerWidget* t, *timers)
            t->reset();
}

void TimerMainDialog::on_setBoilTimeBox_valueChanged(int t)
{
    boilTime->setBoilTime(t * 60);
    resetTimers();
    stopped = false;
}

void TimerMainDialog::decrementTimer()
{
    /*Main timer uses boilTimer which decrements then
     * triigers this function, so there is nothing to
     * do here but show the change.
     */
    updateTime();
}

void TimerMainDialog::updateTime()
{
   unsigned int time = boilTime->getTime();
   timeLCD->display(timeToString(time));
}

QString TimerMainDialog::timeToString(int t)
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


void TimerMainDialog::on_hideButton_clicked()
{
    hideTimers();
}

void TimerMainDialog::hideTimers()
{
    if (!timerWindow->isHidden())
        timerWindow->hide();
}

void TimerMainDialog::on_showButton_clicked()
{
    if (!timers->isEmpty()) {
        if (timerWindow->isHidden())
            timerWindow->show();
    }
    else
            QMessageBox::warning(this, tr("No Timers"), tr("There are currently no timers to show."));
}

void TimerMainDialog::timesUp()
{
    //If there are no knockout timers generate a timer for this
    if (!stopped) {
        bool isKnockOutTimer = false;
        QString note = tr("KNOCKOUT");
        foreach (TimerWidget* t, *timers) {
           if (t->getTime() == 0) {
               isKnockOutTimer = true;
               t->setNote(note); //update existing timers note
            }
        }
        if (!isKnockOutTimer)
            createTimer(note);
        stopped = true;
    }
}

void TimerMainDialog::on_loadRecipesButton_clicked()
{
    //Load current recipes
    if (!timers->isEmpty()){
        QMessageBox mb;
        mb.setText(tr("Active Timers"));
        mb.setInformativeText(tr("You currently have active timers, would you like to replace them or add to them?"));
        QAbstractButton *replace =  mb.addButton(tr("Replace"), QMessageBox::YesRole);
        QAbstractButton *add = mb.addButton(tr("Add"), QMessageBox::NoRole);
        add->setFocus();
        mb.setIcon(QMessageBox::Question);
        mb.exec();
        if (mb.clickedButton() == replace)
            removeAllTimers();
    }
    Recipe* recipe = mainWindow->currentRecipe();
    setBoilTimeBox->setValue(recipe->boilTime_min());
    bool timerFound = false;
    int duplicates = 0;
    int timersGenerated = 0;
    bool duplicatesFound = false;
    QList<Hop*> hops;
    QString note;
    hops = recipe->hops();
    foreach (Hop* h, hops) {
        if (h->use() == 2) { //2 = Boil addition -- Hop::Use enum
            note = tr("%1 of %2").arg(Brewtarget::displayAmount(h->amount_kg(), "TimerNote", "hop_amount", Units::kilograms)).arg(h->name());
            int newTime = h->time_min() * 60;
            foreach (TimerWidget* td, *timers) {
                if (td->getTime() == newTime){
                    if (!td->getNote().contains(note, Qt::CaseInsensitive))
                        td->setNote(note); //append note to existing timer
                    else {
                        duplicates++;
                        if (!duplicatesFound)
                            duplicatesFound = true;
                    }
                    timerFound = true;
                }
            }
            if (!timerFound) {
                createTimer(note, h->time_min()*60);
                timersGenerated++;
            }
        timerFound = false;
        }
    }

    if (duplicatesFound) {
        QString timerText;
        if (duplicates == 1)
            timerText = tr("%1 hop addition is already timed and has been ignored.").arg(duplicates);
        else
            timerText = tr("%1 hop additions are already timed and have been ignored.").arg(duplicates);

        QMessageBox::warning(this, tr("Duplicate Timers Ignored"), timerText, QMessageBox::Ok);
    }
    if (timersGenerated == 0 && !duplicatesFound)
        QMessageBox::warning(this, tr("No Addition Timers"), tr("There are no boil addition, no timers generated."), QMessageBox::Ok);
}

void TimerMainDialog::on_cancelButton_clicked()
{
    removeAllTimers();
}

void TimerMainDialog::removeAllTimers()
{
    qDeleteAll(*timers);
    timers->clear();
    timerWindow->close();
}

void TimerMainDialog::removeTimer(TimerWidget *t)
{
    for (int i = 0; i < timers->count(); i++) {
        if (timers->at(i) == t) {
            delete(timers->at(i));
            timers->removeAt(i);
        }
    }
    showTimers();
}

void TimerMainDialog::reject()
{
    //Escape resets MainTimer if timer has completed
    if (boilTime->isCompleted()) {
        boilTime->stopTimer();
        removeAllTimers();
        resetTimers();
        this->hide();
    }
    else
       this->hide();
}

void TimerMainDialog::on_limitRingTimeCheckBox_clicked()
{
    if (limitRingTimeCheckBox->isChecked()) {
        limitAlarmRing = true;
        limitRingTimeSpinBox->setEnabled(true);
    }
    if (!limitRingTimeCheckBox->isChecked()) {
        limitAlarmRing = false;
        limitRingTimeSpinBox->setEnabled(false);
    }
    setRingLimits(limitAlarmRing, alarmLimit);
}

void TimerMainDialog::on_limitRingTimeSpinBox_valueChanged(int l)
{
    alarmLimit = l;
    setRingLimits(limitAlarmRing, alarmLimit);

}

void TimerMainDialog::setRingLimits(bool l, unsigned int a)
{
    foreach (TimerWidget* t, *timers) {
       t->setAlarmLimits(limitAlarmRing, alarmLimit);
    }
}

unsigned int TimerMainDialog::getAlarmLimit()
{
    return alarmLimit;
}

void TimerMainDialog::setTimerVisible(TimerWidget *t)
{
    timerWindow->setTimerVisible(t);
}

void TimerMainDialog::sortTimers()
{
    if (!timers->isEmpty()) {
        QList<TimerWidget*>* sortedTimers = new QList<TimerWidget*>;
        TimerWidget* biggest = timers->front();
        while (!timers->isEmpty()) {
            foreach (TimerWidget* t, *timers) {
                  if (t->getTime() > biggest->getTime())
                      biggest = t;
            }
            sortedTimers->append(biggest);
            timers->removeOne(biggest);
            if (!timers->isEmpty())
                biggest = timers->front();
        }
        timers = sortedTimers;
    }
}

