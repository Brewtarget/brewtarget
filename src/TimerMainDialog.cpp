/*╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌
 * TimerMainDialog.cpp is part of Brewtarget, and is copyright the following authors 2009-2024:
 *   • Aidan Roberts <aidanr67@gmail.com>
 *   • Brian Rower <brian.rower@gmail.com>
 *   • Matt Young <mfsy@yahoo.com>
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
#include "TimerMainDialog.h"

#include <QMessageBox>
#include <QToolTip>

#include "boiltime.h"
#include "MainWindow.h"
#include "measurement/Unit.h"
#include "measurement/Measurement.h"
#include "model/Boil.h"
#include "model/RecipeAdditionHop.h"
#include "TimerListDialog.h"
#include "TimerWidget.h"

#ifdef BUILDING_WITH_CMAKE
   // Explicitly doing this include reduces potential problems with AUTOMOC when compiling with CMake
   #include "moc_TimerMainDialog.cpp"
#endif

TimerMainDialog::TimerMainDialog(MainWindow* parent) :
   QDialog{parent},
   mainWindow{parent},
   timers{new QList<TimerWidget*>()},
   stopped{false},
   limitAlarmRing{false},
   alarmLimit{5} {
   this->setupUi(this);

   this->boilTime = new BoilTime(this);
   this->boilTime->setBoilTime(setBoilTimeBox->value() * 60); //default 60mins
   this->timerWindow = new TimerListDialog(this, timers);
   this->updateTime();

   //Connections
   connect(boilTime, &BoilTime::BoilTimeChanged, this, &TimerMainDialog::decrementTimer);
   connect(boilTime, &BoilTime::timesUp, this, &TimerMainDialog::timesUp);

   this->retranslateUi(this);
   return;
}

TimerMainDialog::~TimerMainDialog() = default;

void TimerMainDialog::on_addTimerButton_clicked() {
   this->createTimer();
   return;
}

TimerWidget* TimerMainDialog::createNewTimer() {
   TimerWidget* newTimer = new TimerWidget(this, boilTime);
   timers->append(newTimer);
   newTimer->setAlarmLimits(limitAlarmRing, alarmLimit);
   return newTimer;
}

void TimerMainDialog::createTimer() {
   TimerWidget* newTimer = createNewTimer();
   sortTimers();
   showTimers();
   timerWindow->setTimerVisible(newTimer);
   return;
}

void TimerMainDialog::createTimer(QString n) {
   TimerWidget* newTimer = createNewTimer();
   newTimer->setNote(n);
   sortTimers();
   showTimers();
   timerWindow->setTimerVisible(newTimer);
   return;
}

void TimerMainDialog::createTimer(QString n, int t) {
   TimerWidget* newTimer = createNewTimer();
   newTimer->setNote(n);
   newTimer->setTime(t);
   sortTimers();
   showTimers();
   timerWindow->setTimerVisible(newTimer);
   return;
}

void TimerMainDialog::showTimers() {
   if (timers->isEmpty()) {
      if (!timerWindow->isHidden()) {
         timerWindow->hide();
      }
   } else {
      int x = timerWindow->x();
      int y = timerWindow->y();
      this->timerWindow->setAttribute(Qt::WA_DeleteOnClose);
      this->timerWindow->close();

      this->timerWindow = new TimerListDialog(this, timers);
      this->timerWindow->move(x, y);
      this->timerWindow->show();
   }
   return;
}

void TimerMainDialog::on_startButton_clicked() {
   if (!this->boilTime->isStarted()) {
      this->boilTime->startTimer();
   }
   return;
}

void TimerMainDialog::on_stopButton_clicked() {
   if (this->boilTime->isStarted()) {
      this->boilTime->stopTimer();
   }
   return;
}

void TimerMainDialog::on_resetButton_clicked() {
   this->resetTimers();
   return;
}

void TimerMainDialog::resetTimers() {
   // Reset boil time to defined boil time
   this->boilTime->setBoilTime(setBoilTimeBox->value() * 60);
   this->updateTime();
   // Reset all children timers
   if (!this->timers->isEmpty()) {
      for (TimerWidget* t : *this->timers) {
         t->reset();
      }
   }
   return;
}

void TimerMainDialog::on_setBoilTimeBox_valueChanged(int t) {
   this->boilTime->setBoilTime(t * 60);
   this->resetTimers();
   this->stopped = false;
   return;
}

void TimerMainDialog::decrementTimer() {
   // Main timer uses boilTimer which decrements then
   // triigers this function, so there is nothing to
   // do here but show the change.
   this->updateTime();
   return;
}

void TimerMainDialog::updateTime() {
   unsigned int time = boilTime->getTime();
   this->timeLCD->display(timeToString(time));
   return;
}

QString TimerMainDialog::timeToString(int t) {
   if (t == 0) {
      return "00:00:00";
   }

   unsigned int seconds = t;
   unsigned int minutes = 0;
   unsigned int hours = 0;
   if (t > 59) {
      seconds = t%60;
      minutes = t/60;
      if (minutes > 59) {
         hours = minutes/60;
         minutes = minutes%60;
      }
   }
   QString secStr, minStr, hourStr;
   if (seconds < 10) {
       secStr = "0" + QString::number(seconds);
   } else {
       secStr = QString::number(seconds);
   }
   if (minutes <10) {
       minStr = "0" + QString::number(minutes);
   } else {
       minStr = QString::number(minutes);
   }
   if (hours < 10) {
       hourStr = "0" + QString::number(hours);
   } else {
       hourStr = QString::number(hours);
   }
   return hourStr + ":" + minStr + ":" + secStr;
}


void TimerMainDialog::on_hideButton_clicked() {
   this->hideTimers();
   return;
}

void TimerMainDialog::hideTimers() {
   if (!this->timerWindow->isHidden()) {
      this->timerWindow->hide();
   }
   return;
}

void TimerMainDialog::on_showButton_clicked() {
   if (!this->timers->isEmpty()) {
      if (this->timerWindow->isHidden()) {
         this->timerWindow->show();
      }
   }
   else {
      QMessageBox::warning(this, tr("No Timers"), tr("There are currently no timers to show."));
   }
   return;
}

void TimerMainDialog::timesUp() {
   // If there are no knockout timers generate a timer for this
   if (!this->stopped) {
      bool isKnockOutTimer = false;
      QString note = tr("KNOCKOUT");
      for (TimerWidget* t : *this->timers) {
         if (t->getTime() == 0) {
            isKnockOutTimer = true;
            t->setNote(note); //update existing timers note
         }
      }
      if (!isKnockOutTimer) {
         this->createTimer(note);
      }
      stopped = true;
   }
   return;
}

void TimerMainDialog::on_loadRecipesButton_clicked() {
   // Load current recipes
   if (!this->timers->isEmpty()) {
      QMessageBox mb;
      mb.setText(tr("Active Timers"));
      mb.setInformativeText(tr("You currently have active timers, would you like to replace them or add to them?"));
      QAbstractButton *replace =  mb.addButton(tr("Replace"), QMessageBox::YesRole);
      QAbstractButton *add = mb.addButton(tr("Add"), QMessageBox::NoRole);
      add->setFocus();
      mb.setIcon(QMessageBox::Question);
      mb.exec();
      if (mb.clickedButton() == replace) {
         removeAllTimers();
      }
   }
   Recipe * recipe = mainWindow->currentRecipe();
   this->setBoilTimeBox->setValue(recipe->boil() ? recipe->boil()->boilTime_mins() : 0.0);
   bool timerFound = false;
   int duplicates = 0;
   int timersGenerated = 0;
   for (auto hopAddition : recipe->hopAdditions()) {
      if (hopAddition->stage() == RecipeAddition::Stage::Boil &&
          hopAddition->addAtTime_mins()) {
         QString note = tr("%1 of %2").arg(Measurement::displayAmount(hopAddition->amount())).arg(hopAddition->hop()->name());
         int addAtTime_seconds = *hopAddition->addAtTime_mins() * 60;
         for (TimerWidget * td : *this->timers) {
            if (td->getTime() == addAtTime_seconds) {
               if (!td->getNote().contains(note, Qt::CaseInsensitive)) {
                  td->setNote(note); // append note to existing timer
               } else {
                  ++duplicates;
               }
               timerFound = true;
            }
         }
         if (!timerFound) {
            createTimer(note, addAtTime_seconds);
            ++timersGenerated;
         }
         timerFound = false;
      }
   }

   if (duplicates > 0) {
      QString timerText;
      if (duplicates == 1) {
         timerText = tr("%1 hop addition is already timed and has been ignored.").arg(duplicates);
      } else {
         timerText = tr("%1 hop additions are already timed and have been ignored.").arg(duplicates);
      }

      QMessageBox::warning(this, tr("Duplicate Timers Ignored"), timerText, QMessageBox::Ok);
   }
   if (timersGenerated == 0 && duplicates == 0) {
      QMessageBox::warning(this,
                           tr("No Addition Timers"),
                           tr("There are no boil addition, no timers generated."),
                           QMessageBox::Ok);
   }
   return;
}

void TimerMainDialog::on_cancelButton_clicked() {
   this->removeAllTimers();
   return;
}

void TimerMainDialog::removeAllTimers() {
   qDeleteAll(*this->timers);
   this->timers->clear();
   this->timerWindow->close();
   return;
}

void TimerMainDialog::removeTimer(TimerWidget *t) {
   for (int i = 0; i < this->timers->count(); i++) {
      if (this->timers->at(i) == t) {
         delete(this->timers->at(i));
         this->timers->removeAt(i);
      }
   }
   this->showTimers();
   return;
}

void TimerMainDialog::reject() {
   // Escape resets MainTimer if timer has completed
   if (boilTime->isCompleted()) {
      boilTime->stopTimer();
      removeAllTimers();
      resetTimers();
      this->hide();
   } else {
      this->hide();
   }
   return;
}

void TimerMainDialog::on_limitRingTimeCheckBox_clicked() {
   if (limitRingTimeCheckBox->isChecked()) {
      limitAlarmRing = true;
      limitRingTimeSpinBox->setEnabled(true);
   }
   if (!limitRingTimeCheckBox->isChecked()) {
      limitAlarmRing = false;
      limitRingTimeSpinBox->setEnabled(false);
   }
   setRingLimits(limitAlarmRing, alarmLimit);
   return;
}

void TimerMainDialog::on_limitRingTimeSpinBox_valueChanged(int limit) {
   this->alarmLimit = limit;
   this->setRingLimits(this->limitAlarmRing, alarmLimit);
   return;
}

// .:TODO:. I think this function needs refactoring given that it doesn't do anything with either of its parameters!
void TimerMainDialog::setRingLimits([[maybe_unused]] bool limit,
                                    [[maybe_unused]] unsigned int a) {
   for (TimerWidget* t : *this->timers) {
      t->setAlarmLimits(limitAlarmRing, alarmLimit);
   }
   return;
}

unsigned int TimerMainDialog::getAlarmLimit() {
   return alarmLimit;
}

void TimerMainDialog::setTimerVisible(TimerWidget *t) {
   timerWindow->setTimerVisible(t);
   return;
}

void TimerMainDialog::sortTimers() {
   if (!this->timers->isEmpty()) {
      QList<TimerWidget*>* sortedTimers = new QList<TimerWidget*>;
      TimerWidget* biggest = this->timers->front();
      while (!this->timers->isEmpty()) {
         for (TimerWidget* t : *this->timers) {
            if (t->getTime() > biggest->getTime()) {
               biggest = t;
            }
         }
         sortedTimers->append(biggest);
         this->timers->removeOne(biggest);
         if (!this->timers->isEmpty()) {
            biggest = this->timers->front();
         }
      }
      this->timers = sortedTimers;
   }
   return;
}
