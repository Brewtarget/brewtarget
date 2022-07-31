/*
 * TimerWidget.cpp is part of Brewtarget, and is Copyright the following
 * authors 2009-2022
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
#include "TimerWidget.h"

#include <QChar>
#include <QDir>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSpacerItem>
#include <QUrl>
#include <QVBoxLayout>

#include "brewtarget.h"
#include "TimerMainDialog.h"
#include "utils/TimerUtils.h"

TimerWidget::TimerWidget(TimerMainDialog *parent, BoilTime* bt) :
   QDialog{parent},
//   ui{new Ui::timerWidget},
   mainTimer{parent},
   paletteOld{},
   paletteNew{},
   oldColors{true},
   boilTime{bt},
   started{true}, //default is started, this means timers will start as soon as main timer is started
   stopped{false},
   time{0},
   limitAlarmRing{false},
   alarmRingLimit{5},
   //
   // In theory, we could create a QMediaPlayer with the QMediaPlayer::LowLatency flag, meaning "The player is expected
   // to be used with simple audio formats, but playback should start without significant delay. Such playback service
   // can be used for beeps, ringtones, etc."  However, it seems from this bug report
   // https://bugreports.qt.io/browse/QTBUG-72685 that "QMediaPlayer::LowLatency is very old, never implemented, never
   // worked".  There seem to be no plans to fix this and the suggestion is to use QSoundEffect instead.  The good news
   // is that this is slightly simpler and we don't have to create a QMediaPlaylist.  The bad news is that QSoundEffect
   // supports fewer media types than QMediaPlayer.  On Linux for instance, QMediaPlayer will happily play Ogg files but
   // QSoundEffect will only play wav files.
   //
   soundPlayer{new QSoundEffect{this}} {
   qDebug() <<
      Q_FUNC_INFO << "QSoundEffect supports the following formats:" << QSoundEffect::supportedMimeTypes().join(", ");
   this->setupUi(this);

   //Default all timers to Boil time
   this->time = boilTime->getTime();
   this->updateTime();
   this->setDefualtAlarmSound();
   this->stopButton->setEnabled(false);

   this->soundPlayer->setVolume(1.0);

   this->paletteOld = timeLCD->palette();
   this->paletteNew = QPalette(paletteOld);
   // Swap colors.
   this->paletteNew.setColor(QPalette::Active, QPalette::WindowText, paletteOld.color(QPalette::Active, QPalette::Window));
   this->paletteNew.setColor(QPalette::Active, QPalette::Window, paletteOld.color(QPalette::Active, QPalette::WindowText));

   this->retranslateUi(this);

   //Connections
   connect(boilTime, &BoilTime::BoilTimeChanged, this, &TimerWidget::decrementTime);
   connect(boilTime, &BoilTime::timesUp, this, &TimerWidget::decrementTime);
   return;
}

TimerWidget::~TimerWidget() {
//   delete ui;
   return;
}

void TimerWidget::setTime(int t) {
   //Special case if timer auto created from recipe
   if (this->setTimeBox->value() != t/60) {
      this->setTimeBox->setValue(t/60);
   }

   this->time = boilTime->getTime() - t;
   //Reset timer to run again with new time
   this->stopped = false;
   this->started = true;
   this->updateTime();
   return;
}

void TimerWidget::setNote(QString n) {
   // Append if notes exist, new note if not
   if (this->noteEdit->text() == tr("Notes...") || noteEdit->text() == "") {
      this->noteEdit->setText(n);
   } else {
      this->noteEdit->setText(noteEdit->text() + tr(" and ") + n);
   }
   return;
}

void TimerWidget::setBoil(BoilTime *bt) {
   this->boilTime = bt;
   return;
}

int TimerWidget::getTime() {
   /*invert to return addition time not time to addition.
   getTime() and getNote() are used for timer checks when
   generating timers from recipes
   */
   return this->boilTime->getTime() - this->time;
}

QString TimerWidget::getNote() {
   return this->noteEdit->text();
}

void TimerWidget::updateTime() {
   this->timeLCD->display(TimerUtils::timeToString(this->time));
   return;
}

void TimerWidget::decrementTime() {
   if (this->started) {
      // show timers a minute before they go off
      if (time == 60 && this->isHidden()) {
         this->show();
      } else if (time == 0) {
         timesUp();
      } else {
         time = time - 1;
         updateTime();
      }
   }
   return;
}

void TimerWidget::on_setSoundButton_clicked() {
   //Taken form old brewtarget timers
   QDir soundsDir(Brewtarget::getDataDir().canonicalPath() + "/sounds");
   //QDir soundsDir = QString("%1sounds/").arg(Brewtarget::getDataDir());
   QString soundFile =
      QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
                                   tr("Open Sound"), soundsDir.exists() ? soundsDir.canonicalPath() : "",
                                   tr("Audio Files (*.wav)"));

   if (soundFile.isNull()) {
      this->setDefualtAlarmSound();
      return;
   }

   this->setSound(soundFile);
   // Indicate a sound is loaded
   this->setSoundButton->setCheckable(true);
   this->setSoundButton->setChecked(true);
   return;
}

void TimerWidget::setDefualtAlarmSound() {
   QDir soundsDir(Brewtarget::getDataDir().canonicalPath() + "/sounds");
   QString soundFile = soundsDir.absoluteFilePath("beep.wav");
   this->setSound(soundFile);
   return;
}

void TimerWidget::setSound(QString s) {
   QUrl source{QUrl::fromLocalFile(s)};
   this->soundPlayer->setSource(source);
   qDebug() <<
      Q_FUNC_INFO << "Setting alarm sound to" << s << "=" << source << "; sound player status =" <<
      static_cast<int>(this->soundPlayer->status());
   return;
}

void TimerWidget::on_setTimeBox_valueChanged(int t) {
   if (t * 60 > boilTime->getTime()) {
      QMessageBox::warning(this, tr("Error"), tr("Addition time cannot be longer than remaining boil time"));
      time = 0;
   } else {
      this->setTime(t * 60);
   }
   return;
}

void TimerWidget::timesUp() {
   if (limitAlarmRing && alarmRingLimit == 0) {
      this->stopAlarm();
   }
   if (!stopped) {
      this->mainTimer->showTimers();
      this->mainTimer->setTimerVisible(this);
      this->startAlarm();
      this->stopped = true;
   } else {
      this->flash();
   }
   if (this->limitAlarmRing) {
      this->alarmRingLimit--;
   }
   return;
}

void TimerWidget::flash() {
   this->oldColors = !this->oldColors;

   if (this->oldColors) {
      this->timeLCD->setPalette(this->paletteOld);
   } else {
      this->timeLCD->setPalette(this->paletteNew);
   }

   this->timeLCD->repaint();
   return;
}

void TimerWidget::reset() {
   if (this->setTimeBox->value() * 60 > this->boilTime->getTime()) {
      this->time = 0;
   } else {
      this->time = this->boilTime->getTime() - (this->setTimeBox->value() * 60);
   }
   this->stopped = false;
   this->soundPlayer->stop();
   this->updateTime();
   this->alarmRingLimit = this->mainTimer->getAlarmLimit();
   return;
}

void TimerWidget::startAlarm(bool loop) {
   qDebug() <<
      Q_FUNC_INFO << "About to play" << this->soundPlayer->source() << "alarm" << (loop ? "in loop" : "once") <<
      ".  Sound player status is" << static_cast<int>(this->soundPlayer->status());
   this->soundPlayer->setLoopCount(loop ? QSoundEffect::Infinite : 1);
   this->soundPlayer->play();
   if (this->soundPlayer->status() == QSoundEffect::Error || this->soundPlayer->status() == QSoundEffect::Null) {
      qWarning() <<
         Q_FUNC_INFO << "Unable to play timer alarm sound.  Sound player status = " << static_cast<int>(this->soundPlayer->status());
   }

   if (loop) {
      this->stopButton->setEnabled(true);
   }
   return;
}

void TimerWidget::on_stopButton_clicked() {
   this->stopAlarm();
   return;
}

void TimerWidget::stopAlarm() {
   this->soundPlayer->stop();
   this->stopButton->setEnabled(false);
   // Stop timer until new time set
   this->started = false;
   return;
}

void TimerWidget::on_cancelButton_clicked() {
   cancel();
   return;
}

void TimerWidget::cancel() {
   // Deletes this object and removes it from timers list
   this->mainTimer->removeTimer(this);
   return;
}

void TimerWidget::on_playButton_clicked() {
   // Play the alarm sound - eg if you want to check (a) that it works and (b) what it sounds like
   this->startAlarm(false);
   return;
}

void TimerWidget::setAlarmLimits(bool l, unsigned int a) {
   this->limitAlarmRing = l;
   this->alarmRingLimit = a;
   return;
}

void TimerWidget::reject() {
   // Escape hides timer window
   this->mainTimer->hideTimers();
   return;
}
