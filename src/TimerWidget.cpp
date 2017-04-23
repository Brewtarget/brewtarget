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

#include "TimerWidget.h"
#include <QChar>
#include <QFileDialog>
#include <QDir>
#include <QUrl>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include "brewtarget.h"
#include <QMessageBox>

TimerWidget::TimerWidget(TimerMainDialog *parent, BoilTime* bt) :
    QDialog(parent),
    ui(new Ui::timerWidget),
    mainTimer(parent),
    paletteOld(),
    paletteNew(),
    oldColors(true),
    boilTime(bt),
    started(true), //default is started, this means timers will start as soon as main timer is started
    stopped(false),
    time(0),
    limitAlarmRing(false),
    alarmRingLimit(5)
#ifndef NO_QTMULTIMEDIA
         , mediaPlayer(new QMediaPlayer(this))
         , playlist(new QMediaPlaylist(mediaPlayer))
#endif
{
    setupUi(this);

    //Default all timers to Boil time
    time = boilTime->getTime();
    updateTime();
    setDefualtAlarmSound();
    stopButton->setEnabled(false);

    // Taken from old brewtarget timers
#ifndef NO_QTMULTIMEDIA
   playlist->setPlaybackMode(QMediaPlaylist::Loop);
   mediaPlayer->setVolume(100);
   mediaPlayer->setPlaylist(playlist);
#endif
    paletteOld = timeLCD->palette();
    paletteNew = QPalette(paletteOld);
    // Swap colors.
    paletteNew.setColor(QPalette::Active, QPalette::WindowText, paletteOld.color(QPalette::Active, QPalette::Window));
    paletteNew.setColor(QPalette::Active, QPalette::Window, paletteOld.color(QPalette::Active, QPalette::WindowText));

    retranslateUi(this);

    //Connections
    connect(boilTime, &BoilTime::BoilTimeChanged, this, &TimerWidget::decrementTime);
    connect(boilTime, &BoilTime::timesUp, this, &TimerWidget::decrementTime);
}

TimerWidget::~TimerWidget()
{
    delete ui;
}

void TimerWidget::setTime(int t)
{
    //Special case if timer auto created from recipe
    if (setTimeBox->value() != t/60)
        setTimeBox->setValue(t/60);

    time = boilTime->getTime() - t;
    //Reset timer to run again with new time
    if (stopped)
        stopped = false;
    if (!started)
        started = true;
    updateTime();
}

void TimerWidget::setNote(QString n)
{
    //Append if notes exist, new note if not
    if (noteEdit->text() == tr("Notes...") || noteEdit->text() == "")
        noteEdit->setText(n);
    else
        noteEdit->setText(noteEdit->text() + tr(" and ") + n);
}

void TimerWidget::setBoil(BoilTime *bt)
{
    boilTime = bt;
}

int TimerWidget::getTime()
{
    /*invert to return addition time not time to addition.
    getTime() and getNote() are used for timer checks when
    generating timers from recipes
    */
    return boilTime->getTime() - time;
}

QString TimerWidget::getNote()
{
    return noteEdit->text();
}

void TimerWidget::updateTime()
{
    timeLCD->display(timeToString(time));
}

QString TimerWidget::timeToString(int t)
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

void TimerWidget::decrementTime()
{
    if (started) {
        if (time == 60 && this->isHidden()) //show timers a minute before they go off
            this->show();
        else if (time == 0)
            timesUp();
        else {
            time = time - 1;
            updateTime();
        }
    }
}

void TimerWidget::on_setSoundButton_clicked()
{
    //Taken form old brewtarget timers
    QDir soundsDir(Brewtarget::getDataDir().canonicalPath() + "/sounds");
    //QDir soundsDir = QString("%1sounds/").arg(Brewtarget::getDataDir());
    QString soundFile = QFileDialog::getOpenFileName( qobject_cast<QWidget*>(this), tr("Open Sound"), soundsDir.exists() ? soundsDir.canonicalPath() : "", tr("Audio Files (*.wav *.ogg *.mp3 *.aiff)") );

    if( soundFile.isNull() )
    {
       setDefualtAlarmSound();
       return;
    }
    setSound(soundFile);
    // Indicate a sound is loaded
    setSoundButton->setCheckable(true);
    setSoundButton->setChecked(true);
}

void TimerWidget::setDefualtAlarmSound()
{
    QDir soundsDir(Brewtarget::getDataDir().canonicalPath() + "/sounds");
    QString soundFile = soundsDir.absoluteFilePath("beep.ogg");
    setSound(soundFile);
}

void TimerWidget::setSound(QString s)
{
    //Taken from old brewtarget timers
#ifndef NO_QTMULTIMEDIA
   if( !playlist->clear() )
      Brewtarget::logW(playlist->errorString());
   if( !playlist->addMedia(QUrl::fromLocalFile(s)) )
      Brewtarget::logW(playlist->errorString());
   playlist->setCurrentIndex(0);
#endif
}

void TimerWidget::on_setTimeBox_valueChanged(int t)
{
    if (t*60 > boilTime->getTime()) {
        QMessageBox::warning(this, tr("Error"), tr("Addition time cannot be longer than remaining boil time"));
        time = 0;
    } else {
        setTime(t*60);
    }
}

void TimerWidget::timesUp()
{
    if (limitAlarmRing && alarmRingLimit == 0)
        stopAlarm();
    if (!stopped) {
        mainTimer->showTimers();
        mainTimer->setTimerVisible(this);
        startAlarm();
        stopped = true;
    }
    else
        flash();
    if (limitAlarmRing)
        alarmRingLimit--;
}

void TimerWidget::flash()
{
    oldColors = ! oldColors;

    if( oldColors )
       timeLCD->setPalette(paletteOld);
    else
       timeLCD->setPalette(paletteNew);

    timeLCD->repaint();
}

void TimerWidget::reset()
{
    if (setTimeBox->value() * 60 > boilTime->getTime())
        time = 0;
    else
        time = boilTime->getTime() - (setTimeBox->value() * 60);
    if (stopped)
        stopped = false;
#ifndef NO_QTMULTIMEDIA
    mediaPlayer->stop();
#endif
    updateTime();
    alarmRingLimit = mainTimer->getAlarmLimit();
}

void TimerWidget::startAlarm()
{
#ifndef NO_QTMULTIMEDIA
   mediaPlayer->play();
#endif
   stopButton->setEnabled(true);
}

void TimerWidget::on_stopButton_clicked()
{
    stopAlarm();
}

void TimerWidget::stopAlarm()
{
#ifndef NO_QTMULTIMEDIA
    mediaPlayer->stop();
#endif
    stopButton->setEnabled(false);
    //Stop timer until new time set
    started = false;
}

void TimerWidget::on_cancelButton_clicked()
{
    cancel();
}

void TimerWidget::cancel()
{
    //Deletes this object and removes it from timers list
    mainTimer->removeTimer(this);
}

void TimerWidget::setAlarmLimits(bool l, unsigned int a)
{
    limitAlarmRing = l;
    alarmRingLimit = a;
}

void TimerWidget::reject()
{
    //Escape hides timer window
    mainTimer->hideTimers();
}
